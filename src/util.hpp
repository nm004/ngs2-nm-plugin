/*
 * NINJA GAIDEN Master Collection NM Plugin by Nozomi Miyamori
 * is marked with CC0 1.0. This file is a part of NINJA GAIDEN
 * Master Collection NM Plugin.
 */

#ifndef NGMC_NM_UTIL_HPP
#define NGMC_NM_UTIL_HPP

#define WIN32_LEAN_AND_MEAN

#include "distormx.h"
#include <windows.h>
#include <type_traits>
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>


namespace nm::util::detail {

PIMAGE_NT_HEADERS64
get_nt_headers ()
{
  auto base_addr {reinterpret_cast<uintptr_t> (GetModuleHandle (nullptr))};
  auto pehdr_ofs {*reinterpret_cast<uint32_t *> (base_addr + 0x3c)};
  return reinterpret_cast<PIMAGE_NT_HEADERS64> (base_addr + pehdr_ofs);
}

} // namespace nm::util::detail

namespace nm {

// This is currently defined by the size of code.
enum class ImageId : unsigned {
  // for America, Europe and other than East Asia
  NGS1SteamAE = 0x91aa00,

  // for Japan and East Asia
  NGS1SteamJP = 0,

  // for America, Europe and other than East Asia
  NGS2SteamAE = 0x179c400,

  // for Japan and East Asia
  NGS2SteamJP = 0x179c000,
};

inline const ImageId image_id {util::detail::get_nt_headers ()->OptionalHeader.SizeOfCode};
inline const uintptr_t base_of_image {reinterpret_cast<uintptr_t> (GetModuleHandle (nullptr))};

template <typename... T>
constexpr std::array<uint8_t, (sizeof (T) + ... )>
concat(T &&... x) noexcept
{
  std::array<uint8_t, (sizeof (T) + ... )> result;
  std::size_t index {0};
  ((std::copy_n(std::bit_cast<std::array<uint8_t, sizeof (T)>> (x).begin (),
                sizeof (T), result.begin () + index),
    index += sizeof (T)),...);
  return result;
}

template <size_t N>
using Bytes = std::array<uint8_t, N>;

template <typename... T>
constexpr Bytes<sizeof...(T)>
make_bytes(T &&... x) noexcept
{
  return concat(static_cast<uint8_t> (x)...);
}

template <typename T>
class Patch {
public:
  static_assert (std::is_trivially_copyable<T>::value == true,
		 "content type must be memcpy-able.");

  Patch () = delete;
  Patch (const Patch &) = delete;
  Patch & operator= (const Patch &) = delete;

  constexpr
  Patch (uintptr_t base, uintptr_t rva, const T &content)
    : m_dst {std::bit_cast<void *> (base + rva)},
      m_content {content}
  {
    m_buf = new T;
    BOOL r {ReadProcessMemory (GetCurrentProcess (), m_dst, m_buf, sizeof (T), nullptr)};
    assert(r);
    write (&m_content);
  }

  constexpr
  Patch (uintptr_t rva, const T &content)
    : Patch {base_of_image, rva, content} {}

  constexpr
  ~Patch ()
  {
    write (m_buf);
    delete m_buf;
    m_buf = nullptr;
  }

  constexpr const T *
  old_content () const { return m_buf; }

private:
  // This forcibly overwrites the memory.
  void
  write (const T *src) const
  {
    DWORD flOldProtect;
    VirtualProtect (m_dst, sizeof (T), PAGE_EXECUTE_READWRITE, &flOldProtect);
    BOOL r {WriteProcessMemory (GetCurrentProcess (), m_dst, src, sizeof (T), nullptr)};
    assert(r);
    VirtualProtect (m_dst, sizeof (T), flOldProtect, &flOldProtect);
  }

  void *m_dst;
  T m_content;
  std::remove_const<T>::type *m_buf {nullptr};
};

class CallOffsetPatch : public Patch<uint32_t>
{
public:
  CallOffsetPatch (uintptr_t call_rva, uintptr_t callee_rva)
    : Patch<uint32_t> {call_rva+1, static_cast<uint32_t> (callee_rva - (call_rva + 5))} {}
};

// This does not provide trampoline() function.
// Hooking is done by direct jump.
template <typename T>
class SimpleInlineHook {
public:
  SimpleInlineHook () = delete;
  SimpleInlineHook (const SimpleInlineHook &) = delete;
  SimpleInlineHook & operator= (const SimpleInlineHook &) = delete;

  constexpr
  SimpleInlineHook (uintptr_t base, uintptr_t target_func, uintptr_t callback_func)
    : patch {base, target_func,
	    // mov rax, imm
	    // jmp rax
	    concat(make_bytes (0x48, 0xb8), callback_func,
		   make_bytes (0xff, 0xe0))} {}

  constexpr
  SimpleInlineHook (T target_func, T callback_func)
    : SimpleInlineHook {0, reinterpret_cast<uintptr_t> (target_func), reinterpret_cast<uintptr_t> (callback_func)} {}

  constexpr
  SimpleInlineHook (uintptr_t target_func, T callback_func)
    : SimpleInlineHook {base_of_image, target_func, reinterpret_cast<uintptr_t> (callback_func)} {}

private:
  Patch<Bytes<2 + sizeof (uintptr_t) + 2>> patch;
};

template <typename T>
class InlineHook {
public:
  InlineHook () = delete;
  InlineHook (const InlineHook &) = delete;
  InlineHook & operator= (const InlineHook &) = delete;

  constexpr
  InlineHook (uintptr_t base, uintptr_t target_func, T callback_func)
    : m_dst {reinterpret_cast<T> (base + target_func)}, m_cb {callback_func}
  {
    int r {distormx_hook (reinterpret_cast<void **> (&m_dst), reinterpret_cast<void *> (m_cb))};
    assert(r);
  }

  constexpr
  InlineHook (T target_func, T callback_func)
    : InlineHook {0, target_func, callback_func} {}

  constexpr
  InlineHook (uintptr_t target_func, T callback_func)
    : InlineHook {base_of_image, target_func, callback_func} {}

  constexpr
  ~InlineHook ()
  {
    distormx_unhook (reinterpret_cast<void *> (m_dst));
  }

  constexpr
  T trampoline () const { return m_dst; }

private:
  T m_dst;
  T m_cb;
};

template <typename T>
class VFPHook {
  Patch<uintptr_t> patch;
public:
  VFPHook () = delete;
  VFPHook (const VFPHook &) = delete;
  VFPHook & operator= (const VFPHook &) = delete;

  constexpr
  VFPHook (uintptr_t target_vfp, T callback_func)
    : patch {target_vfp, reinterpret_cast<uintptr_t> (callback_func)} {}

  constexpr
  T trampoline () const { return reinterpret_cast<T> (*patch.old_content ()); }
};

} // namespace nm

#endif

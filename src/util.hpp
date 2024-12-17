/*
 * NINJA GAIDEN Master Collection NM Plugin by Nozomi Miyamori
 * is marked with CC0 1.0. This file is a part of NINJA GAIDEN
 * Master Collection NM Plugin.
 */

#ifndef NGMC_NM_UTIL_HPP
#define NGMC_NM_UTIL_HPP

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <type_traits>
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <cstddef>

namespace nm::util::detail {

PIMAGE_NT_HEADERS64
get_nt_headers ()
{
  auto base {reinterpret_cast<uintptr_t> (GetModuleHandle (nullptr))};
  auto pehdr_ofs {*reinterpret_cast<uint32_t *> (base + 0x3c)};
  return reinterpret_cast<PIMAGE_NT_HEADERS64> (base + pehdr_ofs);
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

template <size_t N>
using Bytes = std::array<std::byte, N>;

constexpr auto
make_bytes(auto &&... x) noexcept
{
  return Bytes<sizeof... (x)> { static_cast<std::byte> (x)... };
}

constexpr auto
concat(auto &&... x) noexcept
{
  using namespace std;

  Bytes<(sizeof (x) + ...)> result;
  size_t index {0};
  ((copy_n(bit_cast<Bytes<sizeof (x)>> (x).begin (), sizeof (x), result.begin () + index),
    index += sizeof (x)), ...);
  return result;
}

template <typename T>
class Patch {
public:
  static_assert (std::is_trivially_copyable<T>::value, "content type must be memcpy-able.");

  Patch () = delete;
  Patch (const Patch &) = delete;
  Patch & operator= (const Patch &) = delete;

  constexpr
  Patch (uintptr_t base, uintptr_t rva, const T &content)
    : m_dst {reinterpret_cast<void *> (base + rva)}
  {
    using namespace std;

    if constexpr (is_array<T>::value)
      {
	copy_n (content, size (content), m_content);
	m_buf = new T[size (content)];
      }
    else
      {
        m_content = content;
	m_buf = new T;
      }
    BOOL r {ReadProcessMemory (GetCurrentProcess (), m_dst, m_buf, sizeof (T), nullptr)};
    assert (r);
    write (m_content);
  }

  constexpr
  Patch (uintptr_t rva, const T &content)
    : Patch {base_of_image, rva, content} {}

  constexpr
  ~Patch ()
  {
    write (*m_buf);
    if constexpr (std::is_array<T>::value)
	delete[] m_buf;
    else
	delete m_buf;
  }

  constexpr const T &
  old_content () const { return *m_buf; }

  constexpr const uintptr_t
  get_dest_va () const { return reinterpret_cast<uintptr_t>(m_dst); }

private:
  // This forcibly overwrites the memory.
  void
  write (const T &src) const
  {
    DWORD flOldProtect;
    VirtualProtect (m_dst, sizeof (T), PAGE_EXECUTE_READWRITE, &flOldProtect);
    BOOL r {WriteProcessMemory (GetCurrentProcess (), m_dst, &src, sizeof (T), nullptr)};
    assert(r);
    VirtualProtect (m_dst, sizeof (T), flOldProtect, &flOldProtect);
  }

  void *m_dst;
  T m_content;
  std::remove_const<T>::type *m_buf;
};

// This does not provide call() function.
// Hooking is done by direct jump.
template <typename T>
class SimpleInlineHook {
  static_assert (std::is_function<T>::value, "type T must be function type.");

public:
  SimpleInlineHook () = delete;
  SimpleInlineHook (const SimpleInlineHook &) = delete;
  SimpleInlineHook & operator= (const SimpleInlineHook &) = delete;

  constexpr
  SimpleInlineHook (uintptr_t base, uintptr_t target_func, uintptr_t callback_func)
    : m_patch {base, target_func,
	    // mov rax, imm
	    // jmp rax
	    concat(make_bytes (0x48, 0xb8), callback_func,
		   make_bytes (0xff, 0xe0))} {}

  constexpr
  SimpleInlineHook (T* func_rva, T* callback_func)
    : SimpleInlineHook {0, reinterpret_cast<uintptr_t> (func_rva), reinterpret_cast<uintptr_t> (callback_func)} {}

  constexpr
  SimpleInlineHook (uintptr_t func_rva, T* callback_func)
    : SimpleInlineHook {base_of_image, func_rva, reinterpret_cast<uintptr_t> (callback_func)} {}

private:
  Patch<Bytes<2 + sizeof (uintptr_t) + 2>> m_patch;
};

template <typename T>
class VFPHook {
  static_assert (std::is_function<T>::value, "type T must be function type.");

public:
  VFPHook () = delete;
  VFPHook (const VFPHook &) = delete;
  VFPHook & operator= (const VFPHook &) = delete;

  constexpr
  VFPHook (uintptr_t vfp_rva, T* callback_func)
    : m_patch {base_of_image, vfp_rva, reinterpret_cast<uintptr_t> (callback_func)} {}

  constexpr auto
  call (auto &&... args) const
    {
      auto f = reinterpret_cast<T *> (m_patch.old_content ());
      return f (std::forward<decltype (args)> (args)...);
    }

private:
  Patch<uintptr_t> m_patch;
};

} // namespace nm

#endif

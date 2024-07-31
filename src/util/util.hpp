/*
 * NINJA GAIDEN Master Collection NM Plugin by Nozomi Miyamori
 * is marked with CC0 1.0. This file is a part of NINJA GAIDEN
 * Master Collection NM Plugin.
 */

#ifndef NGMC_NM_UTIL_HPP
#define NGMC_NM_UTIL_HPP

#include "distormx.h"
#include <windef.h>
#include <memoryapi.h>
#include <processthreadsapi.h>
#include <type_traits>
#include <array>
#include <bit>
#include <cassert>
#include <algorithm>
#include <cstdint>

namespace util {
  template <typename T>
  class Patch;

  class CallOffsetPatch;

  // This does not provide trampoline() function and hook
  // is done by direct jump.
  class SimpleInlineHook;

  template <typename T = void (*)()>
  class InlineHook;

  template <typename T = void (*)()>
  class VFPHook;

  template <typename... T>
  constexpr std::array<uint8_t, (sizeof(T) + ... )>
  concat(T &&... x) noexcept;

  template <typename... T>
  constexpr std::array<uint8_t, sizeof...(T)>
  make_bytes(T &&... x) noexcept;

  // This is currently defined by the size of code.
  enum class IMAGE_ID : unsigned {
    // for America, Europe and other than East Asia
    NGS1_STEAM_AE = 0x91aa00,

    // for Japan and East Asia
    NGS1_STEAM_JP = 0,

    // for America, Europe and other than East Asia
    NGS2_STEAM_AE = 0x179c400,

    // for Japan and East Asia
    NGS2_STEAM_JP = 0x179c000,
  } extern const image_id;

  extern const uintptr_t base_of_image;
}

template <typename... T>
constexpr std::array<uint8_t, (sizeof(T) + ... )>
util::concat(T &&... x) noexcept
{
  std::array<uint8_t, (sizeof(T) + ... )> result;
  std::size_t index{};
  ((std::copy_n(std::bit_cast<std::array<uint8_t, sizeof(T)>>(x).begin(),
                sizeof(T), result.begin() + index),
    index += sizeof(T)),...);
  return result;
}

template <typename... T>
constexpr std::array<uint8_t, sizeof...(T)>
util::make_bytes(T &&... x) noexcept
{
  return concat(static_cast<uint8_t>(x)...);
}

template <typename T>
class util::Patch {
public:
  static_assert (std::is_trivially_copyable<T>::value == true,
		 "content type must be memcpy() able.");

  constexpr
  Patch (uintptr_t base, uintptr_t rva, const T &content)
    : m_dst{reinterpret_cast<void *>(base + rva)},
      m_rva{rva},
      m_content{content}
  {}

  constexpr
  Patch (uintptr_t rva, const T &content)
    : Patch {base_of_image, rva, content}
  {}

  constexpr
  ~Patch () { revert (); }

  void
  apply ();

  void
  revert ();

  constexpr const T *
  old_content () const { return m_buf; }

  constexpr const uintptr_t
  rva () const { return m_rva; }

private:
  // This forcibly overwrites the memory.
  void write (const T *src) const;

  uintptr_t m_rva;
  void *m_dst;
  T m_content;
  std::remove_const<T>::type *m_buf = nullptr;
};

template <typename T>
void
util::Patch<T>::apply ()
{
  m_buf = new T;
  BOOL r = ReadProcessMemory (GetCurrentProcess (), m_dst,
			      m_buf, sizeof(T), nullptr);
  assert(r);
  write (&m_content);
}

template <typename T>
void
util::Patch<T>::revert ()
{
  write (m_buf);
  delete m_buf;
  m_buf = nullptr;
}

template <typename T>
void
util::Patch<T>::write (const T *src) const
{
  DWORD flOldProtect;
  VirtualProtect (m_dst, sizeof(T), PAGE_EXECUTE_READWRITE, &flOldProtect);
  BOOL r = WriteProcessMemory (GetCurrentProcess (), m_dst, src, sizeof(T), nullptr);
  assert(r);
  VirtualProtect (m_dst, sizeof(T), flOldProtect, &flOldProtect);
}

class util::CallOffsetPatch : public Patch<uint32_t>
{
public:
  CallOffsetPatch (uintptr_t call_rva, uintptr_t callee_rva)
    : Patch<uint32_t>{call_rva+1, static_cast<uint32_t>(callee_rva - (call_rva + 5))} {}
};

class util::SimpleInlineHook {
  static constexpr auto mov_rax_imm = make_bytes(0x48, 0xb8);
  static constexpr auto jmp_rax = make_bytes(0xff, 0xe0);
  Patch<decltype(concat(mov_rax_imm, uintptr_t{0}, jmp_rax))> patch;
public:
  constexpr
  SimpleInlineHook (uintptr_t base, uintptr_t target_func, uintptr_t callback_func)
    : patch{base, target_func, concat(mov_rax_imm, callback_func, jmp_rax)}
  {}

  constexpr
  SimpleInlineHook (uintptr_t base, auto target_func, auto callback_func)
    : SimpleInlineHook{base,
		       reinterpret_cast<uintptr_t>(target_func),
		       reinterpret_cast<uintptr_t>(callback_func)}
  {}

  constexpr
  SimpleInlineHook (int target_func, auto callback_func)
    : SimpleInlineHook{base_of_image,
		       static_cast<uintptr_t>(target_func),
		       reinterpret_cast<uintptr_t>(callback_func)}
  {}

  constexpr
  SimpleInlineHook (auto target_func, auto callback_func)
    : SimpleInlineHook{base_of_image,
		       reinterpret_cast<uintptr_t>(target_func),
		       reinterpret_cast<uintptr_t>(callback_func)}
  {}

  ~SimpleInlineHook () { detach (); }

  void
  attach () { patch.apply (); }

  void
  detach () { patch.revert (); }
};

template <typename T>
class util::InlineHook {
public:
  constexpr
  InlineHook (uintptr_t base, void *target_func, const void *callback_func)
    : m_dst {reinterpret_cast<void *>(base + reinterpret_cast<uintptr_t>(target_func))},
      m_cb {callback_func}
  {}

  constexpr
  InlineHook (void *target_func, const void *callback_func)
    : InlineHook {base_of_image, target_func, callback_func}
  {}

  constexpr
  InlineHook (auto target_func, auto callback_func)
    : InlineHook {reinterpret_cast<void *>(target_func),
	    	  reinterpret_cast<const void *>(callback_func)}
  {}

  constexpr
  ~InlineHook () { detach (); }

  void
  attach ();

  void
  detach () { distormx_unhook (&m_dst); }

  constexpr
  T trampoline () const { return reinterpret_cast<T>(m_dst); }

private:
  void *m_dst;
  const void *m_cb;
};

template <typename T>
void
util::InlineHook<T>::attach ()
{
  int r = distormx_hook (&m_dst, const_cast<void *>(m_cb));
  assert(r);
}

template <typename T>
class util::VFPHook {
  Patch<uintptr_t> patch;
public:
  constexpr
  VFPHook (uintptr_t target_vfp, auto callback_func)
    : patch{target_vfp, reinterpret_cast<uintptr_t>(callback_func)}
  {}

  ~VFPHook () { detach (); }

  void
  attach () { patch.apply (); }

  void
  detach () { patch.revert (); }

  constexpr
  T trampoline () const { return reinterpret_cast<T>(*patch.old_content ()); }
};

#endif

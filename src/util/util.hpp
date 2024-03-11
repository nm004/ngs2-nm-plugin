/*
 * NGS2 NM Plugin by Nozomi Miyamori is marked with CC0 1.0.
 * This file is a part of NGS2 NM Plugin.
 *
 * This module has some utility functions and constants.
 */

#ifndef NGS2_NM_UTIL_HPP
#define NGS2_NM_UTIL_HPP

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
  extern const uintptr_t base_of_image;

  template <typename... T>
  constexpr auto
  concat(T &&... x) noexcept;

  template <typename... T>
  constexpr auto
  make_bytes(T &&... x) noexcept;

  template <typename T>
  class Patch;

  class CallOffsetPatch;

  template<typename T = void (*)()>
  class Hook {
  public:
    virtual void attach () = 0;
    virtual void detach () = 0;
    virtual T trampoline () = 0;
  };

  template <typename T = void (*)()>
  class InlineHook;

  template <typename T = void (*)()>
  class VFPHook;

  namespace ngs2 {
    // These are currently defined by the sizes of the program text.
    enum class IMAGE_ID {
      // for America, Europe and other than East Asia
      STEAM_AE = 0x179c400,

      // for Japan and East Asia
      STEAM_JP = 0x179c000,
    } extern const image_id;
  }
}

template <typename... T>
constexpr auto
util::concat(T &&... x) noexcept
{
  std::array<uint8_t, (sizeof(x) + ... )> result;
  std::size_t index{};
  ((std::copy_n(std::bit_cast<std::array<uint8_t, sizeof(x)>>(x).begin(),
                sizeof(x), result.begin() + index),
    index += sizeof(x)),...);
  return result;
}

template <typename... T>
constexpr auto
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
  Patch (uintptr_t base, uintptr_t rva, const T &content);

  constexpr
  Patch (uintptr_t rva, const T &content)
    : Patch {base_of_image, rva, content} {}

  constexpr
  ~Patch ();

  void
  apply ();

  void
  revert () { write (m_buf); }

  constexpr const T&
  old_content () { return m_buf; }

  constexpr const uintptr_t
  rva () { return m_rva; }

private:
  // This forcibly overwrites the memory.
  void write (const T& src);

  DWORD m_flOldProtect;
  const uintptr_t m_rva;
  void *m_dst;
  const T m_content;
  std::remove_const<T>::type m_buf;
};

class util::CallOffsetPatch : public Patch<uint32_t>
{
public:
  CallOffsetPatch (uintptr_t call_rva, uintptr_t callee_rva)
    : Patch<uint32_t>{call_rva+1, static_cast<uint32_t>(callee_rva - (call_rva + 5))} {}
};

template <typename T>
constexpr
util::Patch<T>::Patch (uintptr_t base, uintptr_t rva, const T &content)
  : m_dst{reinterpret_cast<void *>(base + rva)},
    m_rva{rva},
    m_content{content}
{
  VirtualProtect (m_dst, sizeof(m_content), PAGE_EXECUTE_READWRITE, &m_flOldProtect);
}

template <typename T>
constexpr
util::Patch<T>::~Patch ()
{
  revert ();
  VirtualProtect (m_dst, sizeof(m_content), m_flOldProtect, &m_flOldProtect);
}

template <typename T>
void
util::Patch<T>::apply ()
{
  BOOL r = ReadProcessMemory (GetCurrentProcess (), m_dst, &m_buf, sizeof(m_buf), NULL);
  assert(r);
  write (m_content);
}

template <typename T>
void
util::Patch<T>::write (const T& src)
{
  BOOL r = WriteProcessMemory (GetCurrentProcess (), m_dst, &src, sizeof(src), NULL);
  assert(r);
}

template <typename T>
class util::InlineHook : public Hook<T> {
public:
  constexpr
  InlineHook (uintptr_t base, void *target_func, const void *callback_func)
    : m_dst {reinterpret_cast<void *>(base + reinterpret_cast<uintptr_t>(target_func))},
      m_cb {callback_func} {}

  constexpr
  InlineHook (uintptr_t base, auto target_func, auto callback_func)
    : InlineHook {base,
	    	  reinterpret_cast<void *>(target_func),
	    	  reinterpret_cast<const void *>(callback_func)} {}

  constexpr
  InlineHook (void *target_func, const void *callback_func)
    : InlineHook {base_of_image, target_func, callback_func} {}

  constexpr
  InlineHook (auto target_func, auto callback_func)
    : InlineHook {reinterpret_cast<void *>(target_func),
	    	  reinterpret_cast<const void *>(callback_func)} {}

  constexpr
  ~InlineHook () { detach (); }

  void
  attach () override;

  void
  detach () override { distormx_unhook (&m_dst); }

  constexpr
  T trampoline () override { return reinterpret_cast<T>(m_dst); }

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
class util::VFPHook : public Hook<T>, Patch<uintptr_t> {
public:
  constexpr
  VFPHook (uintptr_t target_vfp, auto callback_func)
    : Patch<uintptr_t> {target_vfp, reinterpret_cast<uintptr_t>(callback_func)} {}

  void
  attach () override { Patch<uintptr_t>::apply (); }

  void
  detach () override { Patch<uintptr_t>::revert (); }

  constexpr
  T trampoline () { return reinterpret_cast<T>(Patch<uintptr_t>::old_content ()); }
};


#endif

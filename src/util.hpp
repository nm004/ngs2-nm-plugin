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

namespace nm::util::detail
{

PIMAGE_NT_HEADERS64
get_nt_headers ()
{
  auto base {reinterpret_cast<uintptr_t> (GetModuleHandle (nullptr))};
  auto pehdr_ofs {*reinterpret_cast<uint32_t *> (base + 0x3c)};
  return reinterpret_cast<PIMAGE_NT_HEADERS64> (base + pehdr_ofs);
}

} // namespace nm::util::detail

namespace nm
{

// This is currently defined by the size of code.
enum class ImageId : unsigned
{
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
class Patch
{
public:
  static_assert (std::is_trivially_copyable<T>::value, "content type must be memcpy-able.");

  Patch () = delete;
  Patch (const Patch &) = delete;
  Patch & operator= (const Patch &) = delete;

  constexpr
  Patch (void *va, const T &content)
    : m_va {reinterpret_cast <T *> (va)}
  {
    using namespace std;
    if constexpr (std::is_array<T>::value)
      copy_n (*m_va, size (content), m_old_content);
    else
      m_old_content = *m_va;
    write (content);
  }

  constexpr
  Patch (uintptr_t rva, const T &content)
    : Patch {reinterpret_cast <void *> (base_of_image + rva), content} {}

  constexpr
  ~Patch ()
  {
    if (m_va)
      write (m_old_content);
  }

  Patch&
  operator= (Patch&& other) noexcept
  {
    m_va = other.m_va;
    if constexpr (std::is_array<T>::value)
      copy_n (*m_va, size (m_old_content), m_old_content);
    else
      m_old_content = other.m_old_content;
    other.m_va = nullptr;
    return *this;
  }

  constexpr const T &
  get_old_content () const { return m_old_content; }

  constexpr const T *
  get_dest_va () const { return m_va; }

private:
  // This forcibly overwrites the content at m_va.
  void
  write (const T &src) const
  {
    using namespace std;

    DWORD flOldProtect;
    VirtualProtect (m_va, sizeof (src), PAGE_EXECUTE_READWRITE, &flOldProtect);
    if constexpr (std::is_array<T>::value)
      copy_n (src, size (src), *m_va);
    else
      *m_va = src;
    VirtualProtect (m_va, sizeof (src), flOldProtect, &flOldProtect);
  }

  std::remove_const<T>::type *m_va;
  std::remove_const<T>::type m_old_content;
};

// Hooking is done by direct jump.
template <typename T>
class SimpleInlineHook
{
  static_assert (std::is_function<T>::value, "type T must be function type.");

public:
  SimpleInlineHook () = delete;
  SimpleInlineHook (const SimpleInlineHook &) = delete;
  SimpleInlineHook & operator= (const SimpleInlineHook &) = delete;

  constexpr
  SimpleInlineHook (Bytes<16> *target_func, uintptr_t callback_func)
    // mov rax, imm; jmp rax; padding
    : m_patch {target_func, concat(make_bytes (0x48, 0xb8), callback_func, make_bytes (0xff, 0xe0), 0x00)} {}

  constexpr
  SimpleInlineHook (T *target_func, T *callback_func)
    : SimpleInlineHook {reinterpret_cast <Bytes <16> *> (target_func), reinterpret_cast <uintptr_t> (callback_func)} {}

  constexpr
  SimpleInlineHook (uintptr_t func_rva, T *callback_func)
    : SimpleInlineHook {reinterpret_cast <Bytes <16> *> (base_of_image + func_rva), reinterpret_cast<uintptr_t> (callback_func)} {}

  // This is not thread-safe (but who cares :).
  constexpr auto
  call (auto &&... args)
    {
      using namespace std;

      auto va = m_patch.get_dest_va ();
      Bytes <16> code;
      copy (va->begin (), va->end (), code.begin ());
      m_patch.~Patch ();

      if constexpr (std::is_same <void, typename invoke_result <T, decltype (args)...>::type>::value)
	{
	  reinterpret_cast <T *> (va) (forward<decltype (args)> (args)...);
	  m_patch = move (Patch {const_cast <Bytes <16> *> (va), code});
	}
      else
	{
	  auto r = reinterpret_cast <T *> (va) (forward<decltype (args)> (args)...);
	  m_patch = move (Patch {const_cast <Bytes <16> *> (va), code});
	  return r;
	}
    }

private:
  Patch<Bytes <16>> m_patch;
};

template <typename T>
class VFPHook
{
  static_assert (std::is_function<T>::value, "type T must be function type.");

public:
  VFPHook () = delete;
  VFPHook (const VFPHook &) = delete;
  VFPHook & operator= (const VFPHook &) = delete;

  constexpr
  VFPHook (uintptr_t vfp_rva, T* callback_func)
    : m_patch {reinterpret_cast <void *> (base_of_image + vfp_rva), reinterpret_cast<uintptr_t> (callback_func)} {}

  constexpr auto
  call (auto &&... args) const
    {
      auto f = reinterpret_cast<T *> (m_patch.get_old_content ());
      return f (std::forward<decltype (args)> (args)...);
    }

private:
  Patch<uintptr_t> m_patch;
};

} // namespace nm

#endif

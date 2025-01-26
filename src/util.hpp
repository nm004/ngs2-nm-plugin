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

    VirtualProtect (m_va, sizeof (T), PAGE_EXECUTE_READWRITE, &m_flOldProtect);
    write (content);
  }

  constexpr
  Patch (uintptr_t rva, const T &content)
    : Patch {reinterpret_cast <void *> (base_of_image + rva), content} {}

  constexpr
  ~Patch ()
  {
    if (m_va)
    {
      write (m_old_content);
      VirtualProtect (m_va, sizeof (T), m_flOldProtect, &m_flOldProtect);
    }
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
  void
  write (const T &src) const
  {
    using namespace std;

    if constexpr (is_array<T>::value)
      copy_n (src, size (src), *m_va);
    else
      *m_va = src;
  }

  DWORD m_flOldProtect;
  std::remove_const<T>::type *m_va;
  std::remove_const<T>::type m_old_content;
};

// Hooking is done by direct jump.
template <typename F = void ()>
class SimpleInlineHook
{
  using InjectionCode = Bytes <12>;

public:
  static_assert (std::is_function<F>::value, "type F must be function type.");

  static constexpr size_t INJECTION_CODE_SIZE = sizeof (InjectionCode);

  SimpleInlineHook () = delete;
  SimpleInlineHook (const SimpleInlineHook &) = delete;
  SimpleInlineHook & operator= (const SimpleInlineHook &) = delete;

  constexpr
  SimpleInlineHook (F *target_func, uintptr_t callback_func)
    : m_callback_func {callback_func},
      m_patch {reinterpret_cast <InjectionCode *> (target_func), make_injection_code ()} {}

  constexpr
  SimpleInlineHook (F *target_func, F *callback_func)
    : SimpleInlineHook {target_func, reinterpret_cast <uintptr_t> (callback_func)} {}

  constexpr
  SimpleInlineHook (uintptr_t func_rva, F *callback_func)
    : SimpleInlineHook {reinterpret_cast <F *> (base_of_image + func_rva), reinterpret_cast<uintptr_t> (callback_func)} {}

  // Callback is done by writing the original code back.
  // This is not thread-safe (but who cares :).
  constexpr auto
  call (auto &&... args)
  {
    using namespace std;

    auto va = const_cast <InjectionCode *> (m_patch.get_dest_va ());
    m_patch.~Patch ();

    if constexpr (is_same <void, typename invoke_result <F, decltype (args)...>::type>::value)
      {
	reinterpret_cast <F *> (va) (forward<decltype (args)> (args)...);
	m_patch = move (Patch {va, make_injection_code ()});
      }
    else
      {
	auto r = reinterpret_cast <F *> (va) (forward<decltype (args)> (args)...);
	m_patch = move (Patch {va, make_injection_code ()});
	return r;
      }
  }

  constexpr auto
  call_direct (auto &&... args)
  {
    uintptr_t va = reinterpret_cast <uintptr_t> (m_patch.get_dest_va ()) + INJECTION_CODE_SIZE;
    return reinterpret_cast <F *> (va) (std::forward<decltype (args)> (args)...);
  }

private:
  constexpr auto
  make_injection_code () const
  {
    // mov rax, imm; jmp rax;
    return concat(make_bytes (0x48, 0xb8), m_callback_func, make_bytes (0xff, 0xe0));
  }

  const uintptr_t m_callback_func;
  Patch <InjectionCode> m_patch;
};

template <typename F = void ()>
class VFPHook
{
public:
  static_assert (std::is_function<F>::value, "type F must be function type.");

  VFPHook () = delete;
  VFPHook (const VFPHook &) = delete;
  VFPHook & operator= (const VFPHook &) = delete;

  constexpr
  VFPHook (uintptr_t vfp_rva, F* callback_func)
    : m_patch {reinterpret_cast <void *> (base_of_image + vfp_rva), reinterpret_cast<uintptr_t> (callback_func)} {}

  constexpr auto
  call (auto &&... args) const
  {
    auto f = reinterpret_cast<F *> (m_patch.get_old_content ());
    return f (std::forward<decltype (args)> (args)...);
  }

private:
  Patch<uintptr_t> m_patch;
};

} // namespace nm

#endif

/*
 * NGS2 NM Plugin by Nozomi Miyamori is marked with CC0 1.0.
 * This file is a part of NGS2 NM Plugin.
 */

#ifndef NGS2_NM_UTIL_HPP
#define NGS2_NM_UTIL_HPP

#define WIN32_LEAN_AND_MEAN
#include <windef.h>
#include <cstdint>
#include <cassert>

namespace ngs2::nm::util {
  // This forcibly overwrites the content of addr with lpBuffer.
  void
  WriteMemory (SIZE_T addr, LPCVOID lpBuffer, SIZE_T nSize);

  template <typename T> constexpr void
  WriteMemory (SIZE_T addr, const T &v) { return WriteMemory (addr, &v, sizeof(T)); }

  // This finds the address that holds the same pattern.
  uintptr_t
  VAof (const uint8_t *arr, size_t n);

  // This is a wrapper for VAof() that prints out the declaration of the varibale.
  // We should use this for development purpose only.
  uintptr_t
  VAof (const uint8_t *arr, size_t n, const char *name);

  template <size_t N> constexpr uintptr_t
  VAof (const uint8_t (&arr)[N]) { return VAof (&arr, N); }

  enum class NGS2_BINARY_KIND {
    STEAM_JP, // for Japan and East Asia
    STEAM_AE, // for America, Europe and other than East Asia
    UNKNOWN,
  };

  extern const NGS2_BINARY_KIND binary_kind;
  extern const uintptr_t base_of_image;
  extern const uintptr_t start_of_code;
  extern const uintptr_t start_of_data;
}

#endif

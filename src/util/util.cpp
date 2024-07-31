/*
 * NINJA GAIDEN Master Collection NM Plugin by Nozomi Miyamori
 * is marked with CC0 1.0. This file is a part of NINJA GAIDEN
 * Master Collection NM Plugin.
 */

#define WIN32_LEAN_AND_MEAN

#include "util.hpp"
#include <windef.h>
#include <winnt.h>
#include <psapi.h>
#include <libloaderapi.h>
#include <processthreadsapi.h>
#include <cstdint>

namespace {
  namespace detail {
    uintptr_t
    get_base_of_image ();

    PIMAGE_NT_HEADERS64
    get_nt_headers ();
  }
}

namespace util {
  const uintptr_t base_of_image = detail::get_base_of_image ();
  const IMAGE_ID image_id = IMAGE_ID{detail::get_nt_headers ()->OptionalHeader.SizeOfCode};
}

uintptr_t
detail::get_base_of_image ()
{
  MODULEINFO moduleInfo;
  GetModuleInformation(GetCurrentProcess (),
		       GetModuleHandle (nullptr),
		       &moduleInfo,
		       sizeof (moduleInfo));
  return reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);
}

PIMAGE_NT_HEADERS64
detail::get_nt_headers ()
{
  auto base_addr = get_base_of_image ();
  auto pehdr_ofs = *reinterpret_cast<uint32_t *>(base_addr + 0x3c);
  return reinterpret_cast<PIMAGE_NT_HEADERS64>(base_addr + pehdr_ofs);
}

/*
 * NGS2 NM Plugin by Nozomi Miyamori is marked with CC0 1.0.
 * This file is a part of NGS2 NM Plugin.
 */

#define WIN32_LEAN_AND_MEAN

#include "util.hpp"
#include <windef.h>
#include <winnt.h>
#include <psapi.h>
#include <libloaderapi.h>
#include <processthreadsapi.h>
#include <cstdint>

namespace util {
  inline namespace {
    uintptr_t
    get_base_of_image ();

    PIMAGE_NT_HEADERS64
    get_nt_headers ();
  }

  const uintptr_t base_of_image = get_base_of_image ();
  namespace ngs2 {
    const IMAGE_ID image_id = static_cast<util::ngs2::IMAGE_ID>
      (util::get_nt_headers ()->OptionalHeader.SizeOfCode);
  }
}

uintptr_t
util::get_base_of_image ()
{
  MODULEINFO moduleInfo;
  GetModuleInformation(GetCurrentProcess (),
		       GetModuleHandle (nullptr),
		       &moduleInfo,
		       sizeof (moduleInfo));
  return reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);
}

PIMAGE_NT_HEADERS64
util::get_nt_headers ()
{
  auto base_addr = util::get_base_of_image ();
  auto pehdr_ofs = *reinterpret_cast<uint32_t *>(base_addr + 0x3c);
  return reinterpret_cast<PIMAGE_NT_HEADERS64>(base_addr + pehdr_ofs);
}

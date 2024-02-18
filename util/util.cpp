/*
 * NGS2 NM Plugin by Nozomi Miyamori is marked with CC0 1.0.
 * This file is a part of NGS2 NM Plugin.
 *
 * This module has some utility functions and constants.
 */

#include "util.hpp"
#include <windef.h>
#include <winnt.h>
#include <psapi.h>
#include <memoryapi.h>
#include <libloaderapi.h>
#include <processthreadsapi.h>
#include <cstdint>
#include <cassert>
#include <span>
#include <functional>

#if !defined(NDEBUG)
#include <iostream>
#endif

using namespace std;
using namespace ngs2::nm::util;

namespace {
  uintptr_t
  get_base_of_image ()
  {
    MODULEINFO moduleInfo;
    GetModuleInformation(GetCurrentProcess (),
			 GetModuleHandle (nullptr),
			 &moduleInfo,
			 sizeof (moduleInfo));
    return reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);
  }

  PIMAGE_NT_HEADERS64
  get_image_nt_headers ()
  {
    uintptr_t base_addr = get_base_of_image ();
    uint32_t pehdr_ofs = *reinterpret_cast<uint32_t *>(base_addr + 0x3c);
    return reinterpret_cast<PIMAGE_NT_HEADERS64>(base_addr + pehdr_ofs);
  }

  NGS2_BINARY_KIND
  get_binary_kind ()
  {
    switch  (get_image_nt_headers ()->OptionalHeader.SizeOfCode)
      {
      case 0x179c000:
	return NGS2_BINARY_KIND::STEAM_JP;
      case 0x179c400:
	return NGS2_BINARY_KIND::STEAM_AE;
      default:
	return NGS2_BINARY_KIND::UNKNOWN;
      }
  }

  uintptr_t
  calc_base_of_data ()
  {
    auto optHdr = get_image_nt_headers ()->OptionalHeader;
    uintptr_t endOfCode = get_base_of_image () + optHdr.BaseOfCode + optHdr.SizeOfCode;
    return endOfCode + ((optHdr.SectionAlignment - endOfCode % optHdr.SectionAlignment) % optHdr.SectionAlignment);
  }
}

namespace ngs2::nm::util {
  const NGS2_BINARY_KIND binary_kind = get_binary_kind ();
  const uintptr_t base_of_image = get_base_of_image ();
  const uintptr_t start_of_code =
    get_base_of_image () + get_image_nt_headers ()->OptionalHeader.BaseOfCode;
  const uintptr_t start_of_data = calc_base_of_data ();

  void
  WriteMemory (SIZE_T addr, LPCVOID lpBuffer, SIZE_T nSize)
  {
    LPVOID p = reinterpret_cast<LPVOID>(addr);
    DWORD flOld;

    VirtualProtect (p, nSize, PAGE_READWRITE, &flOld);
    BOOL r = WriteProcessMemory (GetCurrentProcess (), p, lpBuffer, nSize, NULL);
    assert(r);
    VirtualProtect (p, nSize, flOld, &flOld);
  }

  uintptr_t
  VAof (const uint8_t *arr, size_t n)
  {
    auto text = span {
      reinterpret_cast<uint8_t *>(start_of_code),
      get_image_nt_headers ()->OptionalHeader.SizeOfCode
    };
    auto pat = span{arr, n};
    boyer_moore_searcher searcher {
      pat.begin (), pat.end ()
    };
    auto addr = searcher (text.begin (), text.end ());
    return start_of_code + distance (text.begin (), addr.first);
  }

  uintptr_t
  VAof (const uint8_t *arr, size_t n, const char *name)
  {
    using namespace std;
    uintptr_t r = VAof (arr, n);
    assert((cout << hex
	    << name << " = base_of_image + 0x" << r - base_of_image << ";"
	    << endl, 1));
    return r;
  }
}

/*
 * NGS2 NM Plugin by Nozomi Miyamori is marked with CC0 1.0.
 * This file is a part of NGS2 NM Plugin.
 *
 * This module loads plugin and data for the game.
 */

#include "util.hpp"
#include "hook.hpp"
#include <winbase.h>
#include <strsafe.h>
#include <fileapi.h>
#include <cstdint>
#include <algorithm>
#include <span>

using namespace std;
using namespace ngs2::nm::util;

namespace {
  struct databin_directory_header {
    uint32_t item_count;
    // We do not use this because id == index on NGS2's databin
    uint32_t offset_to_id_and_index_pairs;
    uint32_t id_and_index_pairs_count;
    uint32_t data0xc;
  };

  struct ProductionPackage {
    uintptr_t vfp1;
    uint8_t path[0x10];
    uint64_t data0x18;
    uint64_t data0x20;
    struct databin_directory_header &databin_directory_header;
    // imcomplete
  };

  struct chunk_info {
    uint32_t offset_to_data;
    uint32_t data0x4;
    uint32_t rawsize;
    uint32_t compressed_size;
    uint32_t data0x10;
    uint16_t data0x14;
    uint8_t data0x16;
    uint8_t data_type;
  };

  bool
  load_plugins ();

  bool
  load_data (ProductionPackage *thisptr, void *param2, struct chunk_info &di, void *out_buf);

  uint32_t
  get_rawsize (uint32_t data_id);

  InlineHooker<decltype(&load_plugins)> *check_dlls_hooker;
  InlineHooker<decltype(&get_rawsize)> *get_rawsize_hooker;
  VFPHooker<decltype(&load_data)> *load_data_hooker;

  // The check_dlls returns true if the number of loaded modules (exe + dlls) in the executable's directory
  // is equal to 2. Otherwise, this returns false. Our function, of course, always return true.
  bool
  load_plugins ()
  {
    // Dll search paths starting from the current directory
    const TCHAR *search_paths[] = {
      TEXT(""),
      TEXT("plugin\\"),
      TEXT("databin\\plugin\\"),
    };
    for (auto &i : search_paths)
      {
	// Main thread sets the current directory where the executable sits.
	TCHAR path[MAX_PATH];
	StringCbCopy (path, sizeof(path), i);
	SetDllDirectory (path);
	StringCbCat (path, sizeof(path), TEXT("*.dll"));

	WIN32_FIND_DATA findFileData;
	HANDLE hFindFile = FindFirstFile (path, &findFileData);
	if (hFindFile == INVALID_HANDLE_VALUE)
	  continue;
	do {
	  if (GetModuleHandle (findFileData.cFileName))
	    {
	      continue;
	    }
	  LoadLibrary (findFileData.cFileName);
	} while (FindNextFile(hFindFile, &findFileData));
	FindClose (hFindFile);
      }
    return true;
  }

  HANDLE
  open_mod_data (uint32_t data_id);

  HANDLE
  open_mod_data (struct databin_directory_header &dbi, struct chunk_info &di);

  bool
  load_data (ProductionPackage *thisptr, void *param2, struct chunk_info &di, void *out_buf)
  {
    HANDLE hFile = open_mod_data (thisptr->databin_directory_header, di);
    if (hFile == INVALID_HANDLE_VALUE)
      return load_data_hooker->get_trampoline () (thisptr, param2, di, out_buf);

    LARGE_INTEGER fsize;
    GetFileSizeEx (hFile, &fsize);

    DWORD nBytesRead;
    BOOL r = ReadFile (hFile, out_buf, fsize.u.LowPart, &nBytesRead, nullptr);
    CloseHandle (hFile);
    return r;
  }

  uint32_t
  get_rawsize (uint32_t data_id)
  {
    HANDLE hFile = open_mod_data (data_id);
    if (hFile == INVALID_HANDLE_VALUE)
      return get_rawsize_hooker->get_trampoline () (data_id);

    LARGE_INTEGER size;
    GetFileSizeEx (hFile, &size);
    CloseHandle (hFile);
    return size.u.LowPart;
  }

  HANDLE
  open_mod_data (uint32_t data_id)
  {
    // 16 is sufficiently enough for id string since the
    // number of items in databin is below 10000.
    TCHAR name[16];
    StringCbPrintf (name, sizeof(name), TEXT("%05d.dat"), data_id);

    const TCHAR *mod_dirs[] = {
      TEXT(""),
      TEXT("mods\\"),
      TEXT("databin\\mods\\"),
    };

    HANDLE hFile = INVALID_HANDLE_VALUE;
    for (auto &i : mod_dirs)
      {
	TCHAR path[MAX_PATH];
	StringCbCopy (path, sizeof(path), i);
	StringCbCat (path, sizeof(path), name);

	hFile = CreateFile (path, GENERIC_READ, 0, nullptr, OPEN_EXISTING,
			    FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile != INVALID_HANDLE_VALUE)
	  break;
      }
    return hFile;
  }

  HANDLE
  open_mod_data (struct databin_directory_header &dbi, struct chunk_info &di)
  {
    span<uint32_t> di_ofs {
      reinterpret_cast<uint32_t *>(reinterpret_cast<uintptr_t>(&dbi) + sizeof(dbi)),
      dbi.item_count
    };

    uintptr_t o = reinterpret_cast<uintptr_t>(&di) - reinterpret_cast<uintptr_t>(&dbi);

    // Search the entry having the offset to the passed chunk_info
    auto i = lower_bound (di_ofs.begin (), di_ofs.end (), o);
    return open_mod_data (distance (di_ofs.begin (), i));
  }
}

namespace ngs2::nm::plugin::core::loader {
  void
  init ()
  {
    uintptr_t load_data_vfp = start_of_data + 0x1516F0;
    uintptr_t check_dlls_func;
    uintptr_t get_rawsize_func;
  
    switch (binary_kind)
      {
      case NGS2_BINARY_KIND::STEAM_JP:
	check_dlls_func = base_of_image + 0xb5c4b0;
	get_rawsize_func = base_of_image + 0x13ab3b0;
	break;
      case NGS2_BINARY_KIND::STEAM_AE:
	check_dlls_func = base_of_image + 0xb5c460;
	get_rawsize_func = base_of_image + 0x13ab5e0;
	break;
      }

    check_dlls_hooker = new InlineHooker<decltype(&load_plugins)> (check_dlls_func, load_plugins);
    get_rawsize_hooker = new InlineHooker<decltype(&get_rawsize)> (get_rawsize_func, get_rawsize);
    load_data_hooker = new VFPHooker<decltype(&load_data)> (load_data_vfp, load_data);
  }
}

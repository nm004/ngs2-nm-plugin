/*
 * NGS2 NM Plugin Loader by Nozomi Miyamori is marked with CC0 1.0
 * This module loads plugin and data for NINJA GAIDEN SIGMA2 Master Collection
 */
#include "util.hpp"
#include <winbase.h>
#include <strsafe.h>
#include <fileapi.h>
#include <algorithm>
#include <span>
#include <iostream>
#include <stdexcept>

using namespace std;
using namespace ngs2::nm::util;

namespace {
  const uint8_t check_dlls_func_pattern[] = {
    0x48, 0x8b, 0xc4, 0x48, 0x89, 0x48, 0x08, 0x56,
    0x57, 0x41, 0x56, 0x48, 0x81, 0xec, 0x80, 0x00,
    0x00, 0x00, 0x48, 0xc7, 0x40, 0x98, 0xfe, 0xff,
    0xff, 0xff, 0x48, 0x89, 0x58, 0x18
  };
  const uint8_t get_rawsize_func_pattern[] = {
    0x48, 0x83, 0xec, 0x28, 0xb8, 0xff, 0xff, 0x00,
    0x00, 0x66, 0x3b, 0xc8, 0x75, 0x07,
  };
  // const uintptr_t load_data_vfp;
  // const uintptr_t databin_manager;
}

namespace {
#if NINJA_GAIDEN_SIGMA_2_STEAM_JP
  const uintptr_t check_dlls_func = VA (0xb5c4b0);
  const uintptr_t get_rawsize_func = VA (0x13ab3b0);

  const uintptr_t load_data_vfp = VA (0x18ee6f0);
  const uintptr_t databin_manager = VA (0x1e62f38);
#else
  const uintptr_t check_dlls_func = VA (0xb5c460);
  const uintptr_t get_rawsize_func = VA (0x13ab5e0);

  const uintptr_t load_data_vfp = VA (0x18ef6f0);
  const uintptr_t databin_manager = VA (0x1e63f38);
#endif
}

namespace {
  HookMap *hook_map;
  uintptr_t load_data_tramp;
  uintptr_t get_rawsize_tramp;

  bool
  load_plugins () noexcept
  {
    // Dll search paths starting from the current directory
    const TCHAR *search_paths[] = {
      TEXT(""),
      TEXT("plugin"),
      TEXT("databin\\plugin"),
    };
    for (auto &i : search_paths)
      {
	// Main thread sets the current directory where the executable sits.
	TCHAR path[MAX_PATH];
	StringCbCopy (path, sizeof(path), i);
	SetDllDirectory (path);
	StringCbCat (path, sizeof(path), TEXT("\\*.dll"));

	WIN32_FIND_DATA findFileData;
	HANDLE hFindFile = FindFirstFile (path, &findFileData);
	if (hFindFile == INVALID_HANDLE_VALUE)
	  continue;
	do {
	  if (GetModuleHandle (findFileData.cFileName))
	    {
	      continue;
	    }
	  D(HMODULE l = )LoadLibrary (findFileData.cFileName);
	  D(cout << (l ? "LOAD SUCCESS: " : "LOAD FAILURE: ") << findFileData.cFileName << endl);
	} while (FindNextFile(hFindFile, &findFileData));
	FindClose (hFindFile);
      }
    return true;
  }

  struct databin_info {
    uint32_t item_count;
    uint32_t data0x4;
    uint32_t data0x8;
    uint32_t data0xc;
  };

  struct ProductionPackage {
    uintptr_t vfp1;
    uint8_t path[0x10];
    uint64_t data0x18;
    uint64_t data0x20;
    struct databin_info &databin_info;
    // imcomplete
  };

  struct data_info {
    uint32_t offset_to_data;
    uint32_t data0x4;
    uint32_t rawsize;
    uint32_t compressed_size;
    uint32_t data0x10;
    uint16_t data0x14;
    uint8_t data0x16;
    uint8_t data_type;
  };

  HANDLE
  open_mod_data (uint32_t data_id) noexcept;

  bool
  load_data (ProductionPackage *thisptr, void *param2, struct data_info &di, void *out_buf) noexcept
  {
    auto &dbi = thisptr->databin_info;
    uintptr_t addr = reinterpret_cast<uintptr_t>(&dbi) + sizeof(dbi);
    span di_ofs {
      reinterpret_cast<uint32_t *>(addr), dbi.item_count
    };

    uintptr_t o = reinterpret_cast<uintptr_t>(&di) - reinterpret_cast<uintptr_t>(&dbi);

    auto i = lower_bound (di_ofs.begin (), di_ofs.end (), o);
    auto data_id = distance (di_ofs.begin (), i) / sizeof(*di_ofs.data ());
    HANDLE hFile = open_mod_data (data_id);
    if (hFile == INVALID_HANDLE_VALUE)
      return reinterpret_cast<decltype(load_data) *>
	(load_data_tramp) (thisptr, param2, di, out_buf);

    DWORD nBytesRead;
    BOOL r = ReadFile (hFile, out_buf, di.rawsize, &nBytesRead, nullptr);
    CloseHandle (hFile);
    return r;
  }

  uint32_t
  get_rawsize (uint32_t data_id) noexcept
  {
    HANDLE file = open_mod_data (data_id);
    if (file == INVALID_HANDLE_VALUE)
      return reinterpret_cast<decltype(get_rawsize) *>
	(get_rawsize_tramp) (data_id);

    LARGE_INTEGER size;
    GetFileSizeEx (file, &size);
    CloseHandle (file);
    return size.u.LowPart;
  }

  HANDLE
  open_mod_data (uint32_t data_id) noexcept
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

  // The check_dlls returns true if the number of loaded modules (exe + dlls) in the executable's directory
  // is equal to 2. Otherwise, this returns false. Our function, of course, always return true.
  bool
  init_check_dlls ()
  {
    return hook_map->hook (check_dlls_func, reinterpret_cast<uint64_t>(load_plugins));
  }

  bool
  init_load_data ()
  {
    // To make inline-hook working, we fills the code after the prologue with
    // NOPs, which makes polyhook think the prologue is long enough.
    const uint8_t nop9[] = {
      0x66, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00,
    };
    WriteMemory (get_rawsize_func + 0xc, nop9, sizeof(nop9));
    get_rawsize_tramp = hook_map->hook (get_rawsize_func, reinterpret_cast<uintptr_t>(get_rawsize));

    load_data_tramp = *reinterpret_cast<uintptr_t *>(load_data_vfp);
    uintptr_t load_data_addr = reinterpret_cast<uintptr_t>(load_data);
    return get_rawsize_tramp
      && WriteMemory (load_data_vfp, &load_data_addr, sizeof(load_data_addr));
			  
  }
}

namespace ngs2::nm::plugin::core::loader {
  void
  init ()
  {
    hook_map = new HookMap;
    if (!init_check_dlls ())
      throw runtime_error ("FAILED: nm_core::loader::init_check_dlls()") ;
    if (!init_load_data ())
      throw runtime_error ("FAILED: nm_core::loader::init_load_data()");
  }

  void
  deinit ()
  {
    delete hook_map;
  }
}

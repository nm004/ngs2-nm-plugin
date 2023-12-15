/*
 * NGS2 NM Plugin Loader by Nozomi Miyamori is marked with CC0 1.0
 * A plugin and data loader for NINJA GAIDEN SIGMA2 Master Collection
 */
#include "util.hpp"
#include <winbase.h>
#include <strsafe.h>
#include <fileapi.h>
#include <algorithm>
#include <span>
#include <iostream>

using namespace std;

namespace {
  const uintptr_t check_dlls_func = VA (0x0b5c460);
  const uintptr_t load_data_vfp = VA (0x18ef6f0);
  const uintptr_t get_rawsize_func = VA (0x13ab5e0);
  const uintptr_t databin_manager = VA (0x1e63f38);
}

namespace {
  uintptr_t load_data_tramp;
  uintptr_t get_rawsize_tramp;

  bool
  load_plugins ()
  {
    // Dll search paths starting from the current directory
    const TCHAR *search_paths[] = {
      TEXT(""),
      TEXT("plugin"),
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
  open_mod_data (uint32_t data_id);

  bool
  load_data (ProductionPackage *thisptr, void *param2, struct data_info &di, void *out_buf)
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
  get_rawsize (uint32_t data_id)
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
  open_mod_data (uint32_t data_id)
  {
    // 16 is sufficiently enough for id string the number of items in databin is
    // below 10000.
    TCHAR name[16];
    StringCbPrintf (name, sizeof(name), TEXT("%05d.dat"), data_id);

    // Search top `mods' dir first, then search one in the `databin'.
    const TCHAR *mod_dirs[] = {
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
    return hook (check_dlls_func, reinterpret_cast<uint64_t>(load_plugins));
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
    get_rawsize_tramp = hook (get_rawsize_func, reinterpret_cast<uintptr_t>(get_rawsize));

    load_data_tramp = *reinterpret_cast<uintptr_t *>(load_data_vfp);
    uintptr_t load_data_addr = reinterpret_cast<uintptr_t>(load_data);
    return get_rawsize_tramp
      && WriteMemory (load_data_vfp, &load_data_addr, sizeof(load_data_addr));
  }
}

// This is for a debugging purpose, e.g. calling this function directly from GDB.
D(bool
  reload_plugin (PCSTR plugin_name)
  {
    CHAR path[MAX_PATH];
    if (HMODULE hMod = GetModuleHandleA (plugin_name))
      {
	GetModuleFileNameA (hMod, path, sizeof(path));
	FreeLibrary (hMod);
      }
    SetDllDirectory ("plugin");
    HMODULE l = LoadLibraryA (path);
    D(cout << (l ? "SUCCESS: " : "FAILED: ") << endl);
    return l;
  })

namespace nm_core::loader {
  bool
  init ()
  {
    return init_check_dlls ()
      && init_load_data ();
  }

  void
  deinit ()
  {
    detours.clear ();
  }
}

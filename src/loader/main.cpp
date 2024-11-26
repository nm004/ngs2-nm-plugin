/*
 * NINJA GAIDEN Master Collection NM Plugin by Nozomi Miyamori
 * is marked with CC0 1.0. This file is a part of NINJA GAIDEN
 * Master Collection NM Plugin.
 */

#if defined(__MINGW32__)
#  if defined(NDEBUG)
#    define DLLEXPORT __declspec (dllexport)
#  else
#    define DLLEXPORT
#  endif
#else
#  define DLLEXPORT __declspec (dllexport)
#endif

#define WIN32_LEAN_AND_MEAN

#include "util.hpp"
#include <windows.h>
#include <strsafe.h>
#include <fileapi.h>
#include <cstdint>
#include <map>

#if !defined(NDEBUG)
#include <iostream>
#endif

using namespace nm;
using namespace std;

namespace {

struct databin_directory_header {
  uint32_t item_count;
  // We do not use this because id == index on NGS2's databin
  uint32_t offset_to_index_to_pos_pairs;
  uint32_t id_and_index_pairs_count;
  uint32_t data0xc;
};

struct ProductionPackage {
  uintptr_t vfp;
  uint8_t path[0x10];
  uint64_t data0x18;
  uint64_t data0x20;
  struct databin_directory_header &databin_directory_header;
  // imcomplete
};

struct chunk_info {
  uint32_t offset_to_data;
  // we use padding to store its index
  uint32_t padding0x4;
  uint32_t decompressed_size;
  uint32_t compressed_size;
  uint32_t data0x10;
  uint16_t data0x14;
  uint8_t data0x16;
  uint8_t data0x17;
};

chunk_info *get_chunk_info (ProductionPackage *, uint32_t);
bool load_data (ProductionPackage *, uintptr_t, struct chunk_info *, void *);
HANDLE open_mod_data (uint32_t);
HANDLE open_mod_data (struct databin_directory_header &, struct chunk_info &);

VFPHook<decltype (get_chunk_info) *> *get_chunk_info_hook;
VFPHook<decltype (load_data) *> *load_data_hook;
map<uint32_t, chunk_info> *chunk_index_to_new_chunk_info;

bool
load_data (ProductionPackage *thisptr, uintptr_t param2, struct chunk_info *ci, void *out_buf)
{
  HANDLE hFile;
  if (!chunk_index_to_new_chunk_info->contains(ci->padding0x4))
    return load_data_hook->trampoline () (thisptr, param2, ci, out_buf);

  if ((hFile = open_mod_data (ci->padding0x4)) == INVALID_HANDLE_VALUE)
    return false;

  DWORD nBytesRead;
  BOOL r {ReadFile (hFile, out_buf, ci->decompressed_size, &nBytesRead, nullptr)};
  CloseHandle (hFile);
  return r;
}

chunk_info *
get_chunk_info (ProductionPackage *thisptr, uint32_t index)
{
  auto ci = get_chunk_info_hook->trampoline () (thisptr, index);
  if (!ci)
      return ci;

  HANDLE hFile;
  if ((hFile = open_mod_data (index)) == INVALID_HANDLE_VALUE)
    {
      auto x = chunk_index_to_new_chunk_info->find(index);
      if (x != chunk_index_to_new_chunk_info->end())
	{
	  chunk_index_to_new_chunk_info->erase (x);
	}
      return ci;
    }

  LARGE_INTEGER size;
  GetFileSizeEx (hFile, &size);
  CloseHandle (hFile);
  auto p = chunk_index_to_new_chunk_info->insert (pair(index, *ci));
  p.first->second.decompressed_size = size.u.LowPart;
  p.first->second.padding0x4 = index;
  return &p.first->second;
}

HANDLE
open_mod_data (uint32_t index)
{
  // 16 is sufficiently enough for id string since the
  // number of items in databin is less than 10000.
  TCHAR name[16];
  StringCbPrintf (name, sizeof (name), TEXT ("%05d.dat"), index);

  const TCHAR *mod_dirs[] = {
    TEXT("mods\\"),
  };

  HANDLE hFile = INVALID_HANDLE_VALUE;
  for (auto &i : mod_dirs)
    {
      TCHAR path[MAX_PATH];
      StringCbCopy (path, sizeof (path), i);
      StringCbCat (path, sizeof (path), name);

      hFile = CreateFile (path, GENERIC_READ, 0, nullptr, OPEN_EXISTING,
			  FILE_ATTRIBUTE_NORMAL, nullptr);
      if (hFile != INVALID_HANDLE_VALUE)
	break;
    }
  return hFile;
}

void
init ()
{
  assert((cout << "INIT: loader" << endl, 1));
  chunk_index_to_new_chunk_info = new map<uint32_t, chunk_info>();

  switch (image_id)
    {
    case ImageId::NGS2SteamAE:
      load_data_hook = new VFPHook {0x18ef6f0, load_data};
      get_chunk_info_hook = new VFPHook {0x18ef6f8, get_chunk_info};
      break;
    case ImageId::NGS2SteamJP:
      load_data_hook = new VFPHook {0x18ee6f0, load_data};
      get_chunk_info_hook = new VFPHook {0x18ee6f8, get_chunk_info};
      break;
    }
}

} // namespace

extern "C" DLLEXPORT BOOL
DllMain (HINSTANCE hinstDLL,
	 DWORD fdwReason,
	 LPVOID lpvReserved)
{
  assert ((cout << "INIT: loader" << endl, 1));

  switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
      init ();
      break;
    default:
      break;
    }
  return TRUE;
}

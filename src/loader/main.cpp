/*
 * NINJA GAIDEN Master Collection NM Plugin by Nozomi Miyamori
 * is marked with CC0 1.0. This file is a part of NINJA GAIDEN
 * Master Collection NM Plugin.
 */

#if defined(_MSVC_LANG)
# define DLLEXPORT __declspec (dllexport)
# define WIN32_LEAN_AND_MEAN
#else
# define DLLEXPORT
#endif

#include "util.hpp"
#include <windows.h>
#include <strsafe.h>
#include <fileapi.h>
#include <cstdint>
#include <cstddef>
#include <map>

using namespace nm;
using namespace std;

namespace {

struct ProductionPackage {
  uintptr_t *vfpt;
  // imcomplete
};

struct chunk_info {
  int64_t offset_to_data;
  uint32_t decompressed_size;
  uint32_t compressed_size;
  uint32_t data0x10;
  uint16_t data0x14;
  uint8_t data0x16;
  uint8_t data0x17;
  // only valid for NGS1 databin
  uint8_t data0x18;
  uint8_t data0x19;
  uint8_t data0x1a;
  uint8_t data0x1b;
  uint32_t data0x1c;
};

LPCRITICAL_SECTION lpCriticalSectionOfRenderingThread;

struct chunk_info *get_chunk_info (ProductionPackage *, uint32_t);
bool load_data (ProductionPackage *, uintptr_t, struct chunk_info *, void *);
HANDLE open_mod_data (uint32_t);

void *(*tmcl_malloc)(void *, uint32_t);
VFPHook<decltype (get_chunk_info)> *get_chunk_info_hook;
VFPHook<decltype (load_data)> *load_data_hook;
map<uint32_t, struct chunk_info> *chunk_index_to_new_chunk_info;
map<struct chunk_info *, uint32_t> *new_chunk_info_to_chunk_index;

bool
load_data_ngs1 (ProductionPackage *thisptr, uintptr_t param2, struct chunk_info *ci, void *out_buf)
{
  HANDLE hFile;

  auto itr = new_chunk_info_to_chunk_index->find(ci);
  if (itr == new_chunk_info_to_chunk_index->end())
    goto BAIL;

  hFile = open_mod_data (itr->second);
  if (hFile == INVALID_HANDLE_VALUE)
    goto BAIL;

  DWORD nBytesRead;
  BOOL r;
  if (ci->data0x1a == 0x11)
    {
      void *buf = tmcl_malloc (out_buf, ci->decompressed_size);
      if (r = ReadFile (hFile, buf, ci->decompressed_size, &nBytesRead, nullptr))
	{
	  auto init_ngs1_tmcl_buf = reinterpret_cast<void (*)(ProductionPackage *, void *, void*)>(*(thisptr->vfpt + 1));
	  init_ngs1_tmcl_buf (thisptr, out_buf, buf);
	}
    }
  else
    r = ReadFile (hFile, out_buf, ci->decompressed_size, &nBytesRead, nullptr);
  CloseHandle (hFile);
  return r;

BAIL:
  return load_data_hook->call (thisptr, param2, ci, out_buf);
}

// param2 is never used.
bool
load_data (ProductionPackage *thisptr, uintptr_t param2, struct chunk_info *ci, void *out_buf)
{
  HANDLE hFile;

  auto itr = new_chunk_info_to_chunk_index->find(ci);
  if (itr == new_chunk_info_to_chunk_index->end())
    goto BAIL;

  hFile = open_mod_data (itr->second);
  if (hFile == INVALID_HANDLE_VALUE)
    goto BAIL;

  DWORD nBytesRead;
  BOOL r;
  r = ReadFile (hFile, out_buf, ci->decompressed_size, &nBytesRead, nullptr);
  CloseHandle (hFile);
  return r;

BAIL:
  return load_data_hook->call (thisptr, param2, ci, out_buf);
}

struct chunk_info *
get_chunk_info (ProductionPackage *thisptr, uint32_t index)
{
  auto ci = get_chunk_info_hook->call (thisptr, index);
  if (!ci)
      return ci;

  HANDLE hFile;
  if ((hFile = open_mod_data (index)) == INVALID_HANDLE_VALUE)
    {
      auto x = chunk_index_to_new_chunk_info->find(index);
      if (x != chunk_index_to_new_chunk_info->end())
	{
	  new_chunk_info_to_chunk_index->erase (&x->second);
	  chunk_index_to_new_chunk_info->erase (x);
	}
      return ci;
    }

  LARGE_INTEGER size;
  GetFileSizeEx (hFile, &size);
  CloseHandle (hFile);
  ci = &chunk_index_to_new_chunk_info->insert (pair(index, *ci)).first->second;
  ci->decompressed_size = size.u.LowPart;
  new_chunk_info_to_chunk_index->insert (pair(ci, index));
  return ci;
}

HANDLE
open_mod_data (uint32_t index)
{
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
  chunk_index_to_new_chunk_info = new map<uint32_t, chunk_info> {};
  new_chunk_info_to_chunk_index = new map<chunk_info *, uint32_t> {};

  switch (image_id)
    {
    case ImageId::NGS1SteamAE:
      load_data_hook = new VFPHook {0x0994b50, load_data};
      get_chunk_info_hook = new VFPHook {0x0994b58, get_chunk_info};
      tmcl_malloc = reinterpret_cast<decltype(tmcl_malloc)>(base_of_image + 0x07e5630);
      break;
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
  switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
      init ();
      break;
    case DLL_PROCESS_DETACH:
      delete chunk_index_to_new_chunk_info;
      delete new_chunk_info_to_chunk_index;
    default:
      break;
    }
  return TRUE;
}

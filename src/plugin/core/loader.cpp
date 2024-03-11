/*
 * NGS2 NM Plugin by Nozomi Miyamori is marked with CC0 1.0.
 * This file is a part of NGS2 NM Plugin.
 *
 * This module loads plugin and data for the game.
 */

#define WIN32_LEAN_AND_MEAN

#include "loader.hpp"
#include "util.hpp"
#include <winbase.h>
#include <strsafe.h>
#include <fileapi.h>
#include <cstdint>
#include <algorithm>
#include <span>

namespace {
  namespace detail {
    struct databin_directory_header {
      uint32_t item_count;
      // We do not use this because id == index on NGS2's databin
      uint32_t offset_to_id_and_index_pairs;
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
      uint32_t data0x4;
      uint32_t size;
      uint32_t compressed_size;
      uint32_t data0x10;
      uint16_t data0x14;
      uint8_t data0x16;
      uint8_t data0x17;
    };

    uint32_t get_chunk_size (uint32_t);
    bool load_data (ProductionPackage *, void *, struct chunk_info &, void *);
    HANDLE open_mod_data (uint32_t);
    HANDLE open_mod_data (struct databin_directory_header &, struct chunk_info &);

    util::Hook<decltype(get_chunk_size) *> *get_chunk_size_hook;
    util::Hook<decltype(load_data) *> *load_data_hook;
    void attach_hooks ();
  }
}

namespace {
  namespace detail2 {
    template <uintptr_t rva>
    auto get_chunk_size_hook
      = util::InlineHook<decltype(detail::get_chunk_size) *>
	{rva, detail::get_chunk_size};
  }
  namespace steam_ae {
    auto get_chunk_size_hook = detail2::get_chunk_size_hook<0x13ab5e0>;
  }
  namespace steam_jp {
    auto get_chunk_size_hook = detail2::get_chunk_size_hook<0x13ab3b0>;
  }
}

namespace {
  namespace detail2 {
    template <uintptr_t rva>
    auto load_data_hook
      = util::VFPHook<decltype(detail::load_data) *>
	{rva, detail::load_data};
  }
  namespace steam_ae {
    auto load_data_hook = detail2::load_data_hook<0x18ef6f0>;
  }
  namespace steam_jp {
    auto load_data_hook = detail2::load_data_hook<0x18ee6f0>;
  }
}

void
detail::attach_hooks ()
{
  get_chunk_size_hook->attach ();
  load_data_hook->attach ();
}

void
plugin::steam_ae::apply_loader_patch ()
{
  using namespace ::steam_ae;
  detail::get_chunk_size_hook = &get_chunk_size_hook;
  detail::load_data_hook = &load_data_hook;
  detail::attach_hooks ();
}

void
plugin::steam_jp::apply_loader_patch ()
{
  using namespace ::steam_jp;
  detail::get_chunk_size_hook = &get_chunk_size_hook;
  detail::load_data_hook = &load_data_hook;
  detail::attach_hooks ();
}

bool
detail::load_data (ProductionPackage *thisptr, void *param2, struct chunk_info &di, void *out_buf)
{
  HANDLE hFile = open_mod_data (thisptr->databin_directory_header, di);
  if (hFile == INVALID_HANDLE_VALUE)
    return load_data_hook->trampoline () (thisptr, param2, di, out_buf);

  LARGE_INTEGER fsize;
  GetFileSizeEx (hFile, &fsize);

  DWORD nBytesRead;
  BOOL r = ReadFile (hFile, out_buf, fsize.u.LowPart, &nBytesRead, nullptr);
  CloseHandle (hFile);
  return r;
}

uint32_t
detail::get_chunk_size (uint32_t data_id)
{
  HANDLE hFile = open_mod_data (data_id);
  if (hFile == INVALID_HANDLE_VALUE)
    return get_chunk_size_hook->trampoline () (data_id);

  LARGE_INTEGER size;
  GetFileSizeEx (hFile, &size);
  CloseHandle (hFile);
  return size.u.LowPart;
}

HANDLE
detail::open_mod_data (uint32_t data_id)
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
detail::open_mod_data (struct databin_directory_header &dbi, struct chunk_info &di)
{ 
  using namespace std;

  span<uint32_t> di_ofs {
    reinterpret_cast<uint32_t *>(reinterpret_cast<uintptr_t>(&dbi) + sizeof(dbi)),
    dbi.item_count
  };

  uintptr_t o = reinterpret_cast<uintptr_t>(&di) - reinterpret_cast<uintptr_t>(&dbi);

  // Search the entry having the offset to the passed chunk_info
  auto i = lower_bound (di_ofs.begin (), di_ofs.end (), o);
  return open_mod_data (distance (di_ofs.begin (), i));
}

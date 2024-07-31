/*
 * NGS2 NM Plugin by Nozomi Miyamori is marked with CC0 1.0.
 * This file is a part of NGS2 NM Plugin.
 */

#define WIN32_LEAN_AND_MEAN

#if defined(__MINGW32__)
#  if defined(NDEBUG)
#    define DLLEXPORT __declspec (dllexport)
#  else
#    define DLLEXPORT
#  endif
#else
#  define DLLEXPORT __declspec (dllexport)
#endif

#include "util.hpp"
#include "qol.hpp"
#include <windef.h>
#include <winbase.h>
#include <fileapi.h>
#include <strsafe.h>
#include <cstdint>
#include <cassert>

#if !defined(NDEBUG)
#include <iostream>
#endif

using namespace util;

namespace plugin {
  namespace steam_ae {
    void apply_core_patch ();
  }
  namespace steam_jp {
    void apply_core_patch ();
  }
}

namespace {
  namespace detail {
    void load_plugins ();
    BOOL init (LPCWSTR);

    // This is needed to keep SteamDRM from wrongly decoding our
    // codes. We inject our codes after the decoding phase by hooking
    // SetCurrentDirectoryW.
    auto SetCurrentDirectoryW_hook = SimpleInlineHook{0, SetCurrentDirectoryW, init};

    // check_dlls returns true if the number of loaded modules (exe + dlls)
    // in the executable's directory is equal to 2. Otherwise, it returns false.
    // We make it always return true.
    bool check_dlls ();
    SimpleInlineHook *check_dlls_hook;
  }
}

namespace {
  namespace detail2 {
    template <uintptr_t rva>
    auto check_dlls_hook = SimpleInlineHook{rva, detail::check_dlls};
  }
  namespace steam_ae {
    auto check_dlls_hook = detail2::check_dlls_hook<0xb5c460>;
  }
  namespace steam_jp {
    auto check_dlls_hook = detail2::check_dlls_hook<0xb5c4b0>;
  }
}

void
detail::load_plugins ()
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
}

BOOL
detail::init (LPCWSTR lpPathName)
{
  using namespace std;
  assert((cout << "INIT: core" << endl, 1));

  SetCurrentDirectoryW_hook.detach ();
  BOOL r = SetCurrentDirectoryW (lpPathName);
  load_plugins ();

  switch (image_id)
    {
    case IMAGE_ID::NGS2_STEAM_AE:
      {
	using namespace plugin::steam_ae;
	apply_core_patch ();
	apply_qol_patch ();
      }
      break;
    case IMAGE_ID::NGS2_STEAM_JP:
      {
	using namespace plugin::steam_jp;
	apply_core_patch ();
	apply_qol_patch ();
      }
      break;
    }

  return r;
}

bool
detail::check_dlls ()
{
  check_dlls_hook->detach ();
  return true;
}

void
plugin::steam_ae::apply_core_patch ()
{
  using namespace ::steam_ae;
  detail::check_dlls_hook = &check_dlls_hook;
  check_dlls_hook.attach ();
}

void
plugin::steam_jp::apply_core_patch ()
{
  using namespace ::steam_jp;
  detail::check_dlls_hook = &check_dlls_hook;
  check_dlls_hook.attach ();
}

extern "C" WINAPI DLLEXPORT BOOL
DllMain (HINSTANCE hinstDLL,
	 DWORD fdwReason,
	 LPVOID lpvReserved)
{
  switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
      detail::SetCurrentDirectoryW_hook.attach ();
      break;
    default:
      break;
    }
  return TRUE;
}

// We define the function as a stub.
extern "C" DLLEXPORT void
MiniDumpWriteDump () {}

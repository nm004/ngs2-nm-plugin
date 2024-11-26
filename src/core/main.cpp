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
#include <fileapi.h>
#include <strsafe.h>
#include <cstdint>
#include <cassert>

#if !defined(NDEBUG)
#include <iostream>
#endif

using namespace nm;

namespace {

void load_plugins ();
BOOL init (LPCWSTR);

// This is needed to keep SteamDRM from wrongly decoding our
// codes. We inject our codes after the decoding phase by hooking
// SetCurrentDirectoryW.
SimpleInlineHook<decltype (SetCurrentDirectoryW) *> *SetCurrentDirectoryW_hook
	= new SimpleInlineHook {SetCurrentDirectoryW, init};

// check_dlls returns true if the number of loaded modules (exe + dlls)
// in the executable's directory is equal to 2. Otherwise, it returns false.
// We make it always return true.
bool check_dlls ();
SimpleInlineHook<decltype (check_dlls) *> *check_dlls_hook;

bool
check_dlls ()
{
  delete check_dlls_hook;
  return true;
}

void
load_plugins ()
{
  // Dll search paths starting from the current directory
  const TCHAR *search_paths[] = {
    TEXT("plugin\\"),
  };
  for (auto &i : search_paths)
    {
      // Main thread sets the current directory where the executable sits.
      TCHAR path[MAX_PATH];
      StringCbCopy (path, sizeof path, i);
      SetDllDirectory (path);
      StringCbCat (path, sizeof path, TEXT ("*.dll"));

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
init (LPCWSTR lpPathName)
{
  using namespace std;
  assert((cout << "INIT: core" << endl, 1));

  delete SetCurrentDirectoryW_hook;
  BOOL r = SetCurrentDirectoryW (lpPathName);
  load_plugins ();

  switch (image_id)
    {
    case ImageId::NGS2SteamAE:
      check_dlls_hook = new SimpleInlineHook {0xb5c460, check_dlls};
      break;
    case ImageId::NGS2SteamJP:
      check_dlls_hook = new SimpleInlineHook {0xb5c4b0, check_dlls};
      break;
    }

  return r;
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
      break;
    default:
      break;
    }
  return TRUE;
}

// We define the function as a stub.
extern "C" DLLEXPORT void
MiniDumpWriteDump () {}

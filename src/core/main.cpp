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
#include <fileapi.h>
#include <strsafe.h>
#include <cstdint>
#include <cassert>

using namespace nm;
using namespace std;

namespace {

BOOL pre_init (LPCWSTR);
SimpleInlineHook<decltype (SetCurrentDirectoryW)> *SetCurrentDirectoryW_hook
  = new SimpleInlineHook {SetCurrentDirectoryW, pre_init};

bool check_dlls ();
SimpleInlineHook<decltype (check_dlls)> *check_dlls_hook;

struct Sigma2;
void Sigma2_vfunction1 (struct Sigma2 *);
VFPHook <decltype (Sigma2_vfunction1)> Sigma2_vfunction1_hook;

Patch<Bytes<7>> *patch1;

void
Sigma2_vfunction1 (Sigma2 *thisptr)
{
  // Dll search paths starting from the current directory
  const TCHAR *search_paths[] = { TEXT("plugin\\"), };
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
      do
        {
	  if (GetModuleHandle (findFileData.cFileName))
	    continue;
	  LoadLibrary (findFileData.cFileName);
        }
      while (FindNextFile(hFindFile, &findFileData));
      FindClose (hFindFile);
    }

  Sigma2_vfunction1_hook.call (thisptr);
}

// "check_dlls()" is called from WinMain. WinMain expects that "check_dlls()" returns true to
// continue the process, otherwise it aborts the process.
//
// We load plugins here. This always returns true.
bool
check_dlls ()
{
  delete check_dlls_hook;
  return true;
}

// This is needed to keep SteamDRM from wrongly decoding our codes.
// We postpone the execution of our codes by hooking SetCurrentDirectoryW, which
// is called from the WinMain function.
BOOL
pre_init (LPCWSTR lpPathName)
{
  constexpr auto XOR_R8_NOP4 = make_bytes (0x4d, 0x31, 0xc0,  0x0f, 0x1f, 0x40, 0x00);
  check_dlls_hook = new SimpleInlineHook {0xb5c460, check_dlls};
  patch1 = new Patch {0x1340d3b, XOR_R8_NOP4};
  Sigma2_vfunction1_hook = VFPHook {0x1462a00, Sigma2_vfunction1};

  delete SetCurrentDirectoryW_hook;
  return SetCurrentDirectoryW (lpPathName);
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
    DisableThreadLibraryCalls (hinstDLL);
    break;
  default:
    break;
  }
  return TRUE;
}

// We define this function as a stub.
extern "C" DLLEXPORT void
MiniDumpWriteDump () {}

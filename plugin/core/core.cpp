#include "util.hpp"
#include <windef.h>
#include <winbase.h>
#include <libloaderapi.h>
#include <iostream>
#include <stdexcept>

namespace ngs2::nm::plugin::core {
  namespace loader {
    void init ();
    void deinit ();
  }
}

using namespace std;
using namespace ngs2::nm::util;
using namespace ngs2::nm::plugin::core;

namespace {
  HookMap *hook_map;

  BOOL
  SetCurrentDirectoryW_ (LPCWSTR lpPathName);
  uintptr_t SetCurrentDirectoryW_tramp;
  
  // This is the early stage init to keep SteamDRM from decoding our codes.
  // We instead inject our codes after the decoding phase by hooking
  // SetCurrentDirectoryW.
  void
  init ()
  {
    hook_map = new HookMap;
    HMODULE hMod = GetModuleHandle ("Kernel32.dll");
    uintptr_t proc = reinterpret_cast<uintptr_t>
      (GetProcAddress (hMod, "SetCurrentDirectoryW"));

    SetCurrentDirectoryW_tramp =
      hook_map->hook (proc, reinterpret_cast<uintptr_t>(SetCurrentDirectoryW_));
  }

  // This is the real init.
  BOOL
  SetCurrentDirectoryW_ (LPCWSTR lpPathName)
  {
    try
      {
	loader::init();
      }
    catch (const exception &e)
      {
	D(cout << e.what () << endl);
      }

    BOOL r = reinterpret_cast<decltype(SetCurrentDirectoryW) *>
      (SetCurrentDirectoryW_tramp) (lpPathName);

    // We do not need the hook anymore.
    delete hook_map;
    return r;
  }
}

extern "C" WINAPI BOOL
DllMain (HINSTANCE hinstDLL,
	 DWORD fdwReason,
	 LPVOID lpvReserved)
{
  switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
      try
	{
	  init ();
	}
      catch (const exception &e)
	{
	  D(cout << e.what () << endl);
	  return FALSE;
	}
      break;
    case DLL_PROCESS_DETACH:
      loader::deinit ();
      break;
    default:
      break;
    }
    return TRUE;
}

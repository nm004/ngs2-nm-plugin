/*
 * NGS2 NM Plugin by Nozomi Miyamori is marked with CC0 1.0.
 * This file is a part of NGS2 NM Plugin.
 */

#include "util.hpp"
#include "hook.hpp"
#include <windef.h>
#include <winbase.h>
#include <cassert>

#if !defined(NDEBUG)
#include <iostream>
#endif

namespace ngs2::nm::plugin::core {
  namespace loader {
    void init ();
  }
  namespace bugfix {
    void init ();
  }
}

using namespace std;
using namespace ngs2::nm::util;
using namespace ngs2::nm::plugin::core;

namespace {
  InlineHooker<decltype(&SetCurrentDirectoryW)> *SetCurrentDirectoryW_hooker;

  // This is the real init.
  BOOL
  SetCurrentDirectoryW_ (LPCWSTR lpPathName)
  {
    assert((cout << "INIT: core" << endl, 1));
    loader::init ();
    bugfix::init ();

    // We do not need the hook anymore.
    delete SetCurrentDirectoryW_hooker;
    return SetCurrentDirectoryW(lpPathName);
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
      // This is the early stage init to keep SteamDRM from decoding our codes.
      // We instead inject our codes after the decoding phase by hooking
      // SetCurrentDirectoryW.
      SetCurrentDirectoryW_hooker =
	new InlineHooker<decltype(&SetCurrentDirectoryW)> (SetCurrentDirectoryW,
							   SetCurrentDirectoryW_);
      break;
    case DLL_PROCESS_DETACH:
      break;
    default:
      break;
    }
    return TRUE;
}

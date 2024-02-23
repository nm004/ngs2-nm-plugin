/*
 * NGS2 NM Plugin by Nozomi Miyamori is marked with CC0 1.0.
 * This file is a part of NGS2 NM Plugin.
 */

#include <windef.h>
#include <cassert>

#if !defined(NDEBUG)
#include <iostream>
#endif

namespace ngs2::nm::plugin::effect {
  namespace gore{
    void init ();
  }

  namespace crush {
    void init ();
  }

  namespace mutil {
    void init ();
  }
}

using namespace std;
using namespace ngs2::nm::plugin::effect;

extern "C" WINAPI BOOL
DllMain (HINSTANCE hinstDLL,
	 DWORD fdwReason,
	 LPVOID lpvReserved)
{
  switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
      assert ((cout << "INIT: effect" << endl, 1));
      gore::init ();
      mutil::init ();
      crush::init ();
      break;
    case DLL_PROCESS_DETACH:
      break;
    default:
      break;
    }
  return TRUE;
}

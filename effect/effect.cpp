/*
 * NGS2 NM Gore Plugin by Nozomi Miyamori is marked with CC0 1.0
 * This plugin restores NG2 vanilla gore to NGS2.
 */

#include <windef.h>

namespace nm_effect {
  namespace gore {
    namespace mutil {
      bool init ();
      void deinit ();
    }

    namespace crush {
      bool init ();
      void deinit ();
    }

    namespace misc {
      bool init ();
      void deinit ();
    }
  }
}

extern "C" WINAPI BOOL
DllMain (HINSTANCE hinstDLL,
	 DWORD fdwReason,
	 LPVOID lpvReserved)
{
  using namespace nm_effect;
  switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
      return gore::mutil::init ()
	&& gore::crush::init ()
	&& gore::misc::init ();
    case DLL_PROCESS_DETACH:
      gore::mutil::deinit ();
      gore::crush::deinit ();
      gore::misc::deinit ();
      return TRUE;
    default:
      return TRUE;
    }
}

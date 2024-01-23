/*
 * NGS2 NM Gore Plugin by Nozomi Miyamori is marked with CC0 1.0
 */

#include "debug.hpp"
#include "gore/gore.hpp"
#include <windef.h>
#include <iostream>
#include <stdexcept>

using namespace std;
using namespace nm_effect;

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
	  gore::mutil::init ();
	  gore::crush::init ();
	  gore::misc::init ();
	}
      catch (const exception &e)
	{
	  D(cout << e.what () << endl);
	  return FALSE;
	}
      break;
    case DLL_PROCESS_DETACH:
      gore::mutil::deinit ();
      gore::crush::deinit ();
      gore::misc::deinit ();
      break;
    default:
      break;
    }
  return TRUE;
}

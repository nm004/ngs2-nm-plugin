/*
 * NGS2 NM Gore Plugin by Nozomi Miyamori is marked with CC0 1.0
 */

#include "util.hpp"
#include "gore/gore.hpp"
#include <windef.h>
#include <iostream>
#include <stdexcept>

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
      try
	{
	  gore::gore::init ();
	  gore::crush::init ();
	  gore::mutil::init ();
	  D(cout << "INIT SUCCESS: nm::plugin::effect" << endl);
	}
      catch (const exception &e)
	{
	  D(cout << e.what () << endl);
	  return FALSE;
	}
      break;
    case DLL_PROCESS_DETACH:
      gore::gore::deinit ();
      gore::mutil::deinit ();
      gore::crush::deinit ();
      break;
    default:
      break;
    }
  return TRUE;
}

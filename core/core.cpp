#include "debug.hpp"
#include <windef.h>
#include <iostream>
#include <stdexcept>

namespace nm_core {
  namespace loader {
    void init ();
    void deinit ();
  }
}

using namespace std;
using namespace nm_core;

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
	  loader::init();
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

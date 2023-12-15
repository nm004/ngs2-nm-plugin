#include <windef.h>

using namespace std;

namespace nm_core {
  namespace loader {
    bool init ();
    void deinit ();
  }
}

extern "C" WINAPI BOOL
DllMain (HINSTANCE hinstDLL,
	 DWORD fdwReason,
	 LPVOID lpvReserved)
{
  using namespace nm_core;
  switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
      return loader::init ();
    case DLL_PROCESS_DETACH:
      loader::deinit ();
      return TRUE;
    default:
      return TRUE;
    }
    return TRUE;
}

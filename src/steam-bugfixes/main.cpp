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
#include <cstdint>

using namespace nm;
using namespace std;

namespace {

Patch<uint8_t> *patch1;
Patch<Bytes<5>> *patch2;

void
init ()
{
  using namespace std;

  // This fixes the "Return to main menu" hang up bug.
  // It sees out of the index at the point.
  patch1 = new Patch {0x13ddfa6 + 3, uint8_t {7}};

  // This fixes the never terminating bug. It enters dead lock at the point.
  // jmp 0xde
  patch2 = new Patch {0xC43060, make_bytes (0xe9, 0xde, 0x00, 0x00, 0x00)};
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
    init ();
    break;
  default:
    break;
  }
  return TRUE;
}

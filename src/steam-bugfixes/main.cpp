/*
 * NINJA GAIDEN Master Collection NM Plugin by Nozomi Miyamori
 * is marked with CC0 1.0. This file is a part of NINJA GAIDEN
 * Master Collection NM Plugin.
 */

#if defined(__MINGW32__)
#  if defined(NDEBUG)
#    define DLLEXPORT __declspec (dllexport)
#  else
#    define DLLEXPORT
#  endif
#else
#  define DLLEXPORT __declspec (dllexport)
#endif

#include "util.hpp"
#include <cstdint>
#include <array>

#if !defined(NDEBUG)
#include <iostream>
#endif

using namespace nm;
using namespace std;

namespace {

Patch<uint8_t> *patch1;
Patch<Bytes<5>> *patch2;

void
init ()
{
  using namespace std;

  assert((cout << "INIT: steam-bugfixes" << endl, 1));

  switch (image_id)
    {
    case ImageId::NGS2SteamAE:
      // This is for the fix of "Return to main menu" hang up bug.
      // It sees out of the index at the point.
      patch1 = new Patch {0x13ddfa6 + 3, uint8_t {7}};

      // This is for the fix of the never terminating game bug.
      // It enters dead lock at the point.
      // jmp 0xde
      patch2 = new Patch {0xC43060, make_bytes( 0xe9, 0xde, 0x00, 0x00, 0x00 )};
      break;

    case ImageId::NGS2SteamJP:
      patch1 = new Patch {0x13ddd76 + 3, uint8_t {7}};
      patch2 = new Patch {0x0c42e60, make_bytes( 0xe9, 0xde, 0x00, 0x00, 0x00 )};
      break;
    }
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
      init ();
      break;
    default:
      break;
    }
  return TRUE;
}

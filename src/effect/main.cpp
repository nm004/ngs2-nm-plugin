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

#define WIN32_LEAN_AND_MEAN

#include "util.hpp"
#include "mutil.hpp"
#include "crush.hpp"
#include "hit.hpp"
#include "bloodstamp.hpp"
#include <windows.h>
#include <cassert>

#if !defined(NDEBUG)
#include <iostream>
#endif

using namespace util;

namespace {

void
init ()
{
  assert ((cout << "INIT: effect" << endl, 1));

  switch (get_image_id ())
    {
    case ImageId::NGS2SteamAE:
      break;
    case ImageId::NGS2SteamJP:
      break;
    }
}

} // namespace

extern "C" DLLEXPORT BOOL
DllMain (HINSTANCE hinstDLL,
	 DWORD fdwReason,
	 LPVOID lpvReserved)
{
  using namespace std;

  switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
      detail::init ();
      break;
    default:
      break;
    }
  return TRUE;
}

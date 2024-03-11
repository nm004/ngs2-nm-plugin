/*
 * NGS2 NM Plugin by Nozomi Miyamori is marked with CC0 1.0.
 * This file is a part of NGS2 NM Plugin.
 */

#define WIN32_LEAN_AND_MEAN

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
#include "mutil.hpp"
#include "crush.hpp"
#include "hit.hpp"
#include <windef.h>
#include <cassert>

#if !defined(NDEBUG)
#include <iostream>
#endif

namespace {
  namespace detail {
    void init ();
  }
}

void
detail::init ()
{
  using namespace util;
  switch (ngs2::image_id)
  {
  case ngs2::IMAGE_ID::STEAM_AE:
    {
      using namespace plugin::steam_ae;
      apply_mutil_patch ();
      apply_crush_patch ();
      apply_hit_effect_patch ();
    }
    break;
  case ngs2::IMAGE_ID::STEAM_JP:
    {
      using namespace plugin::steam_jp;
      apply_mutil_patch ();
      apply_crush_patch ();
      apply_hit_effect_patch ();
    }
    break;
  default:
    break;
  }
}

extern "C" WINAPI DLLEXPORT BOOL
DllMain (HINSTANCE hinstDLL,
	 DWORD fdwReason,
	 LPVOID lpvReserved)
{
  using namespace std;
  assert ((cout << "INIT: effect" << endl, 1));

  switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
      detail::init ();
      break;
    case DLL_PROCESS_DETACH:
      break;
    default:
      break;
    }
  return TRUE;
}

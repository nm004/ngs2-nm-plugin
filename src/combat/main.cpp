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

using namespace std;
using namespace nm;

namespace
{

// TODO?: Separate these to the other module.

// table_n[x] <=> Ninja Dog? (not used), Acolyte, Warrior, Mentor, Master Ninja.
// table_n[x][y] <=> Enemy HP Multiplier, Enemy Damage Multiplier, Enemy
// Critical Damage Multiplier (e.g., striking a player on a wall).
float enemy_hp_damage_multiplier_table[5][3] =
{

      {0.9, 0.8, 0.8},
      {0.9, 0.8, 0.8},
      {1.0, 1.0, 1.0},
      {1.1, 1.3, 1.3},
      {1.2, 1.8, 1.8}
      // for debugging.
      //{.001, 0, 0}
};

Patch<array<uintptr_t, 35>> *patch1;
Patch<uint8_t> *patch2;
Patch<uint8_t> *patch3;

void
init ()
{
  // Let's make pointers to a table point to our table.
  array<uintptr_t, 16> arr;
  fill (arr.begin (), arr.end (), reinterpret_cast <uintptr_t> (&enemy_hp_damage_multiplier_table));
  // For Story Mode
  patch1 = new Patch {0x1829080, arr};
  // For Chapter Challenge
  patch1 = new Patch {0x1829110, arr};

  // Default damage multiplier (both player and enemy).
  // This is used when an override multiplier doesn't exist.
  //*reinterpret_cast <float *> (base_of_image + 0x1e26440 + 4) = 1;

  // Counter hit damage multiplier (player only?).
  //*reinterpret_cast <float *> (base_of_image + 0x1e26440 + 8) = 1;

  // These makes Flying Swallow refer to the damage multiplier.
  // ja imm
  patch2 = new Patch {0x0f4bfcd, uint8_t {0x77}};
  patch3 = new Patch {0x0f73607 + 1, uint8_t {0x87}};
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
  case DLL_PROCESS_DETACH:
    break;
  default:
    break;
  }
  return TRUE;
}

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
      {0.887, 0.786, 0.887},
      {0.887, 0.786, 0.887},
      {0.887, 0.887, 0.942},
      {1.000, 0.942, 1.000},
      {1.000, 1.128, 1.272}
      // for debugging.
      //{.001, 0, 0}
};
Patch<array<uintptr_t, 35>> *patch1;
Patch<uint8_t> *patch2;
Patch<uint8_t> *patch3;

void
init ()
{
  switch (image_id)
  {
  case ImageId::NGS2SteamAE:
    {
      // Let's make pointers to a table point to our table.
      array<uintptr_t, 35> arr;
      fill (arr.begin (), arr.end (), reinterpret_cast <uintptr_t> (&enemy_hp_damage_multiplier_table));
      patch1 = new Patch {0x1829080, arr};

      // Default damage multiplier (both player and enemy).
      // This is used when an override multiplier doesn't exist.
      //*reinterpret_cast <float *> (base_of_image + 0x1e26440 + 4) = 1;

      // Counter hit damage multiplier (player only?).
      *reinterpret_cast <float *> (base_of_image + 0x1e26440 + 8) = 1.618;

      // These makes Flying Swallow refer to the damage multiplier.
      // ja imm
      patch2 = new Patch {0x0f4bfcd, uint8_t {0x77}};
      patch3 = new Patch {0x0f73607 + 1, uint8_t {0x87}};
    }
    break;
  case ImageId::NGS2SteamJP:
    {
      array<uintptr_t, 35> arr;
      fill (arr.begin (), arr.end (), reinterpret_cast <uintptr_t> (&enemy_hp_damage_multiplier_table));
      patch1 = new Patch {0x1828080, arr};
      patch2 = new Patch {0x0f4bdcd, uint8_t {0x77}};
      patch3 = new Patch {0x0f73407 + 1, uint8_t {0x87}};

      //*reinterpret_cast <float *> (base_of_image + 0x1e25440 + 4) = 1;
      *reinterpret_cast <float *> (base_of_image + 0x1e25440 + 8) = 1.618;
    }
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
  case DLL_PROCESS_DETACH:
    break;
  default:
    break;
  }
  return TRUE;
}

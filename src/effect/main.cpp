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

#include "dismember.hpp"
#include "util.hpp"
#include <cassert>

#if !defined(NDEBUG)
#include <iostream>
#endif

using namespace std;
using namespace nm;

namespace {

namespace hit {
  Patch<uint32_t> *patch1;
  Patch<uint32_t> *patch2;
  Patch<float> *patch3;
  Patch<Bytes<3>> *patch4;
  Patch<uint16_t> *patch5;
  Patch<uint16_t> *patch6;
  Patch<uint16_t> *patch7;
  Patch<Bytes<6>> *patch8;
  CallOffsetPatch *patch9;
  CallOffsetPatch *patch10;
}

namespace bloodstamp {
  CallOffsetPatch *patch1;
  CallOffsetPatch *patch2;
  CallOffsetPatch *patch3;
}

namespace dismember {
  SimpleInlineHook<decltype (update_SUP_NodeObj_visibility<0>) *> *update_SUP_NodeObj_visibility_hook;
  SimpleInlineHook<decltype (get_OPTscat_indices<0>) *> *get_OPTscat_indices_hook;

  CallOffsetPatch *patch1;
  CallOffsetPatch *patch3;
  CallOffsetPatch *patch4;
  Patch<Bytes<6>> *patch5;
  CallOffsetPatch *patch6;
  CallOffsetPatch *patch7;
  Patch<uint8_t> *patch8;
  Patch<uint8_t> *patch9;

  Patch<Bytes<14>> *patch_jp1;
  Patch<Bytes<14>> *patch_jp2;
  Patch<Bytes<24>> *patch_jp3;
  Patch<Bytes<23>> *patch_jp4;
  Patch<Bytes<16>> *patch_jp5;
  Patch<Bytes<25>> *patch_jp6;
  Patch<Bytes<19>> *patch_jp7;
  Patch<Bytes<28>> *patch_jp8;
}

void
init ()
{
  assert ((cout << "INIT: effect" << endl, 1));

  switch (image_id)
    {
    case ImageId::NGS2SteamAE:
      {
	using namespace hit;

	// Credits: Fiend Busa
	// This is for the time how long the dismembered limbs remain.
	// They set 1 by default, which means limbs disappear immediately.
	// Instead, we set it to very very long time.
	patch1 = new Patch {0x1458099 + 1, uint32_t{0x7fffffff}};

	// Credits: Fiend Busa
	// This is the hitstop (micro freeeze) intensity when Ryu dismembers an enemy's limb.
	// The higher the value, the longer it freezes. They set 3 by default.
	patch2 = new Patch {0x1457f6d + 6, uint32_t{0}};

	// Credits: Fiend Busa
	// Bodies will not disappear until the time (the specified number of frames) expires.
	patch3 = new Patch {0x1907cf4, float{60 * 60 * 10}};

	// This circumvents the block to trigger the EFF_ArrowHitBlood
	// which makes blood particles when hitting enemies with an arrow.
	// We have chosen `and' over `xor' becase it has the same size of codes.
	// and eax, 0
	patch4 = new Patch {0x1048888, make_bytes( 0x83, 0xe0, 0x00 )};

	// These make Ryu's bow, Momiji's bow and Rachel's gatling gun attack category
	// 0x1002 which is needed to make EFF_ArrowHitBlood work.
	patch5 = new Patch {0x17f25c8, uint16_t{0x1002}};
	patch6 = new Patch {0x17f0428, uint16_t{0x1002}};
	patch7 = new Patch {0x17f3ca8, uint16_t{0x1002}};

	// These adjust the param1 of trigger_hit_effect() which makes blood particles larger
	// when hitting enemies. The param1 of trigger_hit_effect() is a category
	// of attack. 5th byte of Param1 is related to the blood particle. It seems that
	// 0x10000 or 0x20000 are only effective, however when you specify 0x20000, some
	// attacks produce less blood particles (e.g. Shuriken).
	// or ecx, 0x10000
	patch8 = new Patch {0x1047790 - 6, make_bytes( 0x81, 0xc9, 0x00, 0x00, 0x01, 0x00 )};
	patch9 = new CallOffsetPatch{0x104772e, 0x1047790 - 6};
	patch10 = new CallOffsetPatch{0x104775d, 0x1047790 - 6};
      }

      {
	using namespace bloodstamp;
	// TODO: write a description about these patches.
	patch1 = new CallOffsetPatch{0x120cbe1, 0x077b010};
	patch2 = new CallOffsetPatch{0x120c7f1, 0x077b010};
	patch3 = new CallOffsetPatch{0x120da69, 0x077ce10};
      }
      {
	using namespace dismember;

	update_SUP_NodeObj_visibility_hook = new SimpleInlineHook {0x1455880, update_SUP_NodeObj_visibility<0x1e38d10>};
	get_OPTscat_indices_hook = new SimpleInlineHook {0x144cb00, get_OPTscat_indices<0x1e38f30>};

	// We directly detour trigger_VanishMutilationRootEffect_func function to
	// trigger_BloodMutilationRootEffect because their parameters are perfectly the same,
	// even the flow of the function looks totally same.
        patch1 = new CallOffsetPatch {0x145866a, 0x1220060};

	// We overwrites the call to alloc_BloodMutilationRootEffect with the
	// call to adjust_BloodMutilationRootEffect_params, which calls
	// alloc_BloodMutilationRootEffect internally.

	patch3 = new CallOffsetPatch{0x12201f6, 0x1206e30};

	// We directly detour trigger_VanishViscosityEffect function to
	// trigger_MutilationViscosityBloodEffect because their parameters are
	// perfectly the same.
	patch4 = new CallOffsetPatch{0x1458761, 0x121fe20};

	// We directly detour trigger_VanishCrushRootEffect function to
	// trigger_BloodCrushRootEffect, however we need to reorder the parameter,
	// so we inject the parameters re-ordering code in the beginning.

	// xchg rdx, r8
	// xchg rcx, r9
	patch5 = new Patch{0x0c0fc40 - 6, make_bytes( 0x4c, 0x87, 0xc2, 0x48, 0x87, 0xd1 )};
	patch6 = new CallOffsetPatch {0x14152e7, 0x0c0fc40 - 6};
	patch7 = new CallOffsetPatch {0x1415c65, 0x0c0fc40 - 6};

	// Credit: enhuhu
	// This disables EFF_CommonIzunaBloodExp which produces dim purple effect.
	// ret
	patch8 = new Patch {0x1020990, uint8_t{0xc3}};

	// This disables EFF_CommonSuicideBloodExpPatch which produces dim purple effect.
	patch9 = new Patch {0x1020360, uint8_t{0xc3}};
      }
      break;
    case ImageId::NGS2SteamJP:
      {
	using namespace hit;
	patch1 = new Patch {0x1457f15 + 1, uint32_t{0x7fffffff}};
	patch2 = new Patch {0x1457dd1 + 6, uint32_t{0}};
	patch3 = new Patch {0x1906cf4, float{60 * 60 * 10}};
	patch4 = new Patch {0x10485e8, make_bytes( 0x83, 0xe0, 0x00 )};
	patch5 = new Patch {0x17f15c8, uint16_t{0x1002}};
	patch6 = new Patch {0x17ef428, uint16_t{0x1002}};
	patch7 = new Patch {0x17f2ca8, uint16_t{0x1002}};
	patch8 = new Patch {0x10474f0 - 6, make_bytes( 0x81, 0xc9, 0x00, 0x00, 0x01, 0x00 )};
	patch9 = new CallOffsetPatch{0x104748e, 0x10474f0 - 6};
	patch10 = new CallOffsetPatch{0x10474bd, 0x10474f0 - 6};
      }

      {
	using namespace bloodstamp;
	patch1 = new CallOffsetPatch{0x120c941, 0x077b050};
	patch2 = new CallOffsetPatch{0x120c551, 0x077b050};
	patch3 = new CallOffsetPatch{0x120d7c9, 0x077ce50};
      }
      {
	using namespace dismember;

	update_SUP_NodeObj_visibility_hook = new SimpleInlineHook {0x14556a0, update_SUP_NodeObj_visibility<0x1e37d10>};
	get_OPTscat_indices_hook = new SimpleInlineHook {0x144c8e0, get_OPTscat_indices<0x1e37f30>};

	patch1 = new CallOffsetPatch {0x14584f5, 0x121fde0};
	patch3 = new CallOffsetPatch {0x1206d26, 0x1206b90};
	patch4 = new CallOffsetPatch{0x14585f8, 0x121fba0};
	patch5 = new Patch{0x0c0fcb0 - 6, make_bytes( 0x4c, 0x87, 0xc2, 0x48, 0x87, 0xd1 )};
	patch6 = new CallOffsetPatch {0x14150b7, 0x0c0fcb0 - 6};
	patch7 = new CallOffsetPatch {0x1415a35, 0x0c0fcb0 - 6};
	patch8 = new Patch {0x10206f0, uint8_t{0xc3}};
	patch9 = new Patch {0x10200c0, uint8_t{0xc3}};

	// These are for circumventing the head dismemberment disablers
	// which can be found in the Asia version of the game.
	constexpr auto NOP5 = make_bytes( 0x0F, 0x1F, 0x44, 0x00, 0x00);
	constexpr auto NOP7 = make_bytes( 0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00);
	constexpr auto NOP8 = make_bytes( 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00);
	constexpr auto NOP9 = make_bytes( 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00);

    	// for crush
	patch_jp1 = new Patch {0x1457ef0, concat(NOP7, NOP7)};
	// for mutil
	patch_jp2 = new Patch {0x14583e5, concat(NOP7, NOP7)};

	// In order to prevent the head from getting back to his body
	patch_jp3 = new Patch {0x1456dcc, concat(NOP8, NOP8, NOP8)};
	patch_jp4 = new Patch {0x1456e71, concat(NOP7, NOP8, NOP8)};
	patch_jp5 = new Patch {0x145e463, concat(NOP8, NOP8)};

	// The following is not required but for the completness.
	patch_jp6 = new Patch {0x145E824, concat(NOP8, NOP8, NOP9)};
	patch_jp7 = new Patch {0xc31190, concat(NOP5, NOP7, NOP7)};
	patch_jp8 = new Patch {0x14cff5e, concat(NOP7, NOP7, NOP7, NOP7)};
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
    default:
      break;
    }
  return TRUE;
}

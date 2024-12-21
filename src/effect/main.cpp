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

#include "dismember.hpp"
#include "util.hpp"

using namespace std;
using namespace nm;

namespace {

namespace hit {
  Patch<Bytes<3>> *patch1;
  Patch<uint16_t> *patch2;
  Patch<uint16_t> *patch3;
  Patch<uint16_t> *patch4;
}

namespace bloodparticle {
  Patch<Bytes<9>> *patch1;
  Patch<int32_t> *patch2;
  Patch<int32_t> *patch3;
  Patch<uint32_t> *patch4;
}

namespace bloodstamp {
  Patch<int32_t> *patch1;
  Patch<int32_t> *patch2;
  Patch<int32_t> *patch3;
  Patch<int32_t> *patch4;
  Patch<int32_t> *patch5;
  Patch<int32_t> *patch6;
  Patch<int32_t> *patch7;
}

namespace dismember {
  SimpleInlineHook<decltype (update_SUP_NodeObj_visibility<0>)> *update_SUP_NodeObj_visibility_hook;
  SimpleInlineHook<decltype (get_OPTscat_indices<0>)> *get_OPTscat_indices_hook;
  Patch<int32_t> *patch1;
  Patch<int32_t> *patch2;
  Patch<uint32_t [4]> *patch3;
  Patch<int32_t> *patch4;
  Patch<Bytes<4>> *patch5;
  Patch<Bytes<5>> *patch6;
  Patch<int32_t> *patch7;
  Patch<int32_t> *patch8;
  Patch<int32_t> *patch9;
  Patch<float> *patch10;
  //Patch<int32_t> *patch10;
  Patch<uint8_t> *patch11;
  Patch<uint8_t> *patch12;

  namespace jp {
    Patch<Bytes<14>> *patch1;
    Patch<Bytes<14>> *patch2;
    Patch<Bytes<24>> *patch3;
    Patch<Bytes<23>> *patch4;
    Patch<Bytes<16>> *patch5;
    Patch<Bytes<25>> *patch6;
    Patch<Bytes<19>> *patch7;
    Patch<Bytes<28>> *patch8;
  }
}

void
init ()
{
  switch (image_id)
    {
    case ImageId::NGS2SteamAE:
      {
	using namespace hit;
	// This circumvents the block for triggering the EFF_ArrowHitBlood
	// which makes blood particles when hitting enemies with an arrow.
	// We have chosen `and' over `xor' becase it has the same size of codes.
	// and eax, 0
	patch1 = new Patch {0x1048888, make_bytes (0x83, 0xe0, 0x00)};

	// These make Ryu's bow, Momiji's bow and Rachel's gatling gun attack category
	// 0x1002 which is needed to make EFF_ArrowHitBlood work.
	patch2 = new Patch {0x17f25c8, uint16_t {0x1002}};
	patch3 = new Patch {0x17f0428, uint16_t {0x1002}};
	patch4 = new Patch {0x17f3ca8, uint16_t {0x1002}};
      }

      {
	using namespace bloodparticle;

	// This makes large shuriken blood particles by circumventing the gurad.
	// nop8; stc
	patch1 = new Patch {0x1049b01, make_bytes (0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00,  0xf9)};

	// These invoke large blood particles effect (each for humans and fiends).
	patch2 = new Patch {0x120a13f + 1, 0x075f720 - (0x120a13f + 5)};
	patch3 = new Patch {0x120a0f0 + 1, 0x075fd20 - (0x120a0f0 + 5)};

	// Blood particles for obliteration. We chose a largest and longest particle.
	// from -> to (the value is passed to FUN_141209c50)
	// 2 -> 0 == small (default), 3 -> 1 == large but faint (it is used when in water),
	// 4 -> 2 == large, 5 -> 3 == large and last longer (meybe not used in NG2, but in NGS1).
	patch4 = new Patch {0x1205b28 + 1, 0x5u};
      }

      {
	using namespace bloodstamp;
	// These make NG2 red blood stains on the ground. We change the calling functions to the function of oil stains,
	// because it can produce NG2 like blood stains (need to modify game data "04413.dat").
	// These are for permanent blood stains.
	patch1 = new Patch {0x120cc1d + 1, 0x077b010 - (0x120cc1d + 5)};
	patch2 = new Patch {0x120cbe1 + 1, 0x077b010 - (0x120cbe1 + 5)};
	// These are for temporal blood stains.
	patch3 = new Patch {0x120c82d + 1, 0x077b010 - (0x120c82d + 5)};
	patch4 = new Patch {0x120c7f1 + 1, 0x077b010 - (0x120c7f1 + 5)};
	// These are for ?.
	patch5 = new Patch {0x120daa5 + 1, 0x077ce10 - (0x120daa5 + 5)};
	patch6 = new Patch {0x120da69 + 1, 0x077ce10 - (0x120da69 + 5)};

	// This makes old bloodstains stay longer. They set 60 by default (1 sec).
	// (60*60*5) frams/60 fps = 5 min).
	patch7 = new Patch {0x120cb25 + 3, 60*60*5};
      }

      {
	using namespace dismember;

	update_SUP_NodeObj_visibility_hook = new SimpleInlineHook {0x1455880, update_SUP_NodeObj_visibility<0x1e38d10>};
	get_OPTscat_indices_hook = new SimpleInlineHook {0x144cb00, get_OPTscat_indices<0x1e38f30>};

	// We directly detour invoke_VanishMutilationRootEffect function to invoke_BloodMutilationRootEffect function.
	// Both functions parameters look the same.
        patch1 = new Patch {0x145866a + 1, 0x1220060 - (0x145866a + 5)};

	// We detour alloc_BloodMutilationRootEffect to alloc_BloodMutilationRootEffect_wrapper, which calls
	// alloc_BloodMutilationRootEffect internally.
	patch2 = new Patch {0x12201f6 + 1, 0x1206ec0 - (0x12201f6 + 5)};
	// We also modify the internal jump table to make larger blood jet always.
	patch3 = new Patch {0x1207430, *reinterpret_cast <uint32_t (*)[4]> (base_of_image + 0x1207430 + 4 * 0x10)};

	// We directly detour invoke_VanishViscosityEffect function to invoke_MutilationViscosityBloodEffect function.
	// Both functions parameters look the same.
	patch4 = new Patch {0x1458761 + 1, 0x121fe20 - (0x1458761 + 5)};

	// We directly detour invoke_VanishCrushRootEffect function to
	// invoke_BloodCrushRootEffect, however we need to pass parameters shifted,
	// rcx -> rdx, rdx -> r8 (rcx is never used).
	// lea r8, [rbp-60]
	patch5 = new Patch {0x14152a4, make_bytes (0x4c, 0x8d, 0x45, 0xa0)};
	// lea rdx, [rsp+54]
	patch6 = new Patch {0x14152d9, make_bytes (0x48, 0x8d, 0x54, 0x24, 0x54)};
	patch7 = new Patch {0x14152e7 + 1, 0x0c0fc40 - (0x14152e7 + 5)};

	// Credits: Fiend Busa
	// This is the hitstop (micro freeeze) intensity when Ryu dismembers an enemy's limb.
	// The higher the value, the longer it freezes. They set 3 by default.
	patch8 = new Patch {0x1457f6d + 6, 0};

	// Credits: Fiend Busa
	// The limbs stay for the specified time. They set 1 by default,
	// which makes limbs disappear immediately.
	// 3600 frames/60 fps = 5 min.
	patch9 = new Patch {0x1458099 + 1, 3600};

	// Credits: Fiend Busa
	// Bodies stay for the specified time.
	// The new specified time is 5 min.
	// TODO: use the other float value address instead of setting new float value
	// because other functions use it too.
	patch10 = new Patch {0x1907cf4, 60.f * 5.f};
	//patch10 = new Patch {0x145f459 + 5, 0x1907d98 - (0x145f459 + 9)};

	// Credit: enhuhu
	// This disables EFF_CommonIzunaBloodExp which produces dim purple effect.
	// ret
	patch11 = new Patch {0x1020990, uint8_t {0xc3}};

	// This disables EFF_CommonSuicideBloodExpPatch which produces dim purple effect.
	patch12 = new Patch {0x1020360, uint8_t {0xc3}};
      }
      break;
    case ImageId::NGS2SteamJP:
      {
	using namespace hit;
	patch1 = new Patch {0x10485e8, make_bytes (0x83, 0xe0, 0x00)};
	patch2 = new Patch {0x17f15c8, uint16_t {0x1002}};
	patch3 = new Patch {0x17ef428, uint16_t {0x1002}};
	patch4 = new Patch {0x17f2ca8, uint16_t {0x1002}};
      }
      {
	using namespace bloodparticle;
	patch1 = new Patch {0x1049861, make_bytes (0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00,  0xf9)};
	patch2 = new Patch {0x1209e9f + 1, 0x075f760 - (0x1209e9f + 5)};
	patch3 = new Patch {0x1209e50 + 1, 0x075ff60 - (0x1209e50 + 5)};
	patch4 = new Patch {0x1205888 + 1, 0x5u};
      }
      {
	using namespace bloodstamp;
	patch1 = new Patch {0x120c97d + 1, 0x077b050 - (0x120c97d + 5)};
	patch2 = new Patch {0x120c941 + 1, 0x077b050 - (0x120c941 + 5)};
	patch3 = new Patch {0x120c58d + 1, 0x077b050 - (0x120c58d + 5)};
	patch4 = new Patch {0x120c551 + 1, 0x077b050 - (0x120c551 + 5)};
	patch5 = new Patch {0x120d805 + 1, 0x077ce50 - (0x120d805 + 5)};
	patch6 = new Patch {0x120d7c9 + 1, 0x077ce50 - (0x120d7c9 + 5)};
	patch7 = new Patch {0x120c885+ 3, 60*60*5};
      }
      {
	using namespace dismember;
	update_SUP_NodeObj_visibility_hook = new SimpleInlineHook {0x14556a0, update_SUP_NodeObj_visibility<0x1e37d10>};
	get_OPTscat_indices_hook = new SimpleInlineHook {0x144c8e0, get_OPTscat_indices<0x1e37f30>};

	patch1 = new Patch {0x14584f5 + 1, 0x121fde0 - (0x14584f5 + 5)};
	patch2 = new Patch {0x121ff76 + 1, 0x1206c20 - (0x121ff76 + 5)};
	patch3 = new Patch {0x1207190, *reinterpret_cast <uint32_t (*)[4]> (base_of_image + 0x1207190 + 4 * 0x10)};
	patch4 = new Patch {0x14585f8 + 1, 0x121fba0 - (0x14585f8 + 5)};
	patch5 = new Patch {0x1415074, make_bytes (0x4c, 0x8d, 0x45, 0xa0)};
	patch6 = new Patch {0x14150a9, make_bytes (0x48, 0x8d, 0x54, 0x24, 0x54)};
	patch7 = new Patch {0x14150b7 + 1, 0x0c0fcb0 - (0x14150b7 + 5)};
	patch8 = new Patch {0x1457f15 + 1, 3600};
	patch9 = new Patch {0x1457dd1 + 6, 0};
	patch10 = new Patch {0x1906cf4, 60.f * 5.f};
	//patch10 = new Patch {0x145f009 + 5, 0x1906d98 - (0x145f009 + 9)};
	patch11 = new Patch {0x10206f0, uint8_t {0xc3}};
	patch12 = new Patch {0x10200c0, uint8_t {0xc3}};
      }
      {
	using namespace dismember::jp;

	// These are for circumventing the head dismemberment disablers
	// which can be found in the Asia version of the game.
	constexpr auto NOP5 = make_bytes (0x0f, 0x1f, 0x44, 0x00, 0x00);
	constexpr auto NOP7 = make_bytes (0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00);
	constexpr auto NOP8 = make_bytes (0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00);
	constexpr auto NOP9 = make_bytes (0x66, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00);

	// for crush.
	patch1 = new Patch {0x1457ef0, concat (NOP7, NOP7)};
	// for mutil.
	patch2 = new Patch {0x14583e5, concat (NOP7, NOP7)};

	// In order to prevent the head from getting back to its body.
	patch3 = new Patch {0x1456dcc, concat (NOP8, NOP8, NOP8)};
	patch4 = new Patch {0x1456e71, concat (NOP7, NOP8, NOP8)};
	patch5 = new Patch {0x145e463, concat (NOP8, NOP8)};

	// The following is not required but for the completness.
	patch6 = new Patch {0x145E824, concat (NOP8, NOP8, NOP9)};
	patch7 = new Patch {0x0c31190, concat (NOP5, NOP7, NOP7)};
	patch8 = new Patch {0x14cff5e, concat (NOP7, NOP7, NOP7, NOP7)};
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
      {
	using namespace hit;
	delete patch1;
	delete patch2;
	delete patch3;
	delete patch4;
      }
      {
	using namespace bloodparticle;
	delete patch1;
	delete patch2;
	delete patch3;
	delete patch4;
      }
      {
	using namespace bloodstamp;
	delete patch1;
	delete patch2;
	delete patch3;
	delete patch4;
	delete patch5;
	delete patch6;
      }
      {
	using namespace dismember;
	delete update_SUP_NodeObj_visibility_hook;
	delete get_OPTscat_indices_hook;
	delete patch1;
	delete patch2;
	delete patch3;
	delete patch4;
	delete patch5;
	delete patch6;
	delete patch7;
	delete patch8;
	delete patch9;
	delete patch10;
	delete patch11;
	delete patch12;
      }
      if (image_id == ImageId::NGS2SteamJP)
	{
	  using namespace dismember::jp;
	  delete patch1;
	  delete patch2;
	  delete patch3;
	  delete patch4;
	  delete patch5;
	  delete patch6;
	  delete patch7;
	  delete patch8;
	}
      break;
    default:
      break;
    }
  return TRUE;
}

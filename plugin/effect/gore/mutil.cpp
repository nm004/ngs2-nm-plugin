/*
 * NGS2 NM Plugin by Nozomi Miyamori is marked with CC0 1.0.
 * This file is a part of NGS2 NM Plugin.
 *
 * This module restores NG2 mutilation gore effects (e.g. Sword, Fang, etc.).
 */
#include "util.hpp"
#include "hook.hpp"
#include "gore.hpp"
#include <cstdint>

using namespace std;
using namespace ngs2::nm::util;
using namespace ngs2::nm::plugin::effect::gore;

namespace ngs2::nm::plugin::effect::gore::mutil {
  CallHooker *alloc_BloodMutilationRootEffect_hooker;
  CallHooker *trigger_VanishMutilationRootEffect_hooker;
  CallHooker *trigger_VanishViscosityEffect_hooker;

  void
  init ()
  {
    uintptr_t trigger_delimb_effect_func;
    uintptr_t trigger_VanishMutilationRootEffect_func;
    uintptr_t trigger_VanishMutilationRootEffect_caller;
    uintptr_t trigger_BloodMutilationRootEffect_func;
    uintptr_t trigger_VanishViscosityEffect_func;
    uintptr_t trigger_VanishViscosityEffect_caller;
    uintptr_t trigger_MutilationViscosityBloodEffect_func;
    uintptr_t adjust_BloodMutilationRootEffect_params_func;

    switch (binary_kind)
      {
      case NGS2_BINARY_KIND::STEAM_JP:
	trigger_delimb_effect_func = base_of_image + 0x1457c50;
	trigger_VanishMutilationRootEffect_func = base_of_image + 0x1229190;
	trigger_VanishMutilationRootEffect_caller = trigger_delimb_effect_func + 0x8a5;
	trigger_BloodMutilationRootEffect_func = base_of_image + 0x121fde0;
	trigger_VanishViscosityEffect_func = base_of_image + 0x1228e10;
	trigger_VanishViscosityEffect_caller = trigger_delimb_effect_func + 0x9a8;
	trigger_MutilationViscosityBloodEffect_func = base_of_image + 0x121fba0;
	adjust_BloodMutilationRootEffect_params_func = base_of_image + 0x1206b90;
	break;
      case NGS2_BINARY_KIND::STEAM_AE:
	trigger_delimb_effect_func = base_of_image + 0x1457df0;
	trigger_VanishMutilationRootEffect_func = base_of_image + 0x12293d0;
	trigger_VanishMutilationRootEffect_caller = trigger_delimb_effect_func + 0x87a;
	trigger_BloodMutilationRootEffect_func = base_of_image + 0x1220060;
	trigger_VanishViscosityEffect_func = base_of_image + 0x1229050;
	trigger_VanishViscosityEffect_caller = trigger_delimb_effect_func + 0x971;
	trigger_MutilationViscosityBloodEffect_func = base_of_image + 0x121fe20;
	adjust_BloodMutilationRootEffect_params_func = base_of_image + 0x1206e30;
	break;
      }

    // We inject the small code that adds 2 to param3 of adjust_BloodMutilationRootEffect_params, which
    // makes the function can adjust params to create bigger blood jet and wider blood shower.
    // If the param3 of bit 8 is high, it looks like the effect is the minor version of the one bit 8 is low.
    const uintptr_t adjust_BloodMutilationRootEffect_params_above_4 = adjust_BloodMutilationRootEffect_params_func - 4;
    const uint8_t add_r8b_0x02[] = { 0x41, 0x80, 0xc0, 0x02 };
    WriteMemory(adjust_BloodMutilationRootEffect_params_above_4, add_r8b_0x02);

    // We overwrites the call address of alloc_BloodMutilationRootEffect in
    // trigger_BloodMutilationRootEffect with the adjust_BloodMutilationRootEffect_params, which calls
    // alloc_BloodMutilationRootEffect internally.
    const uintptr_t alloc_BloodMutilationRootEffect_caller = trigger_BloodMutilationRootEffect_func + 0x196;
    alloc_BloodMutilationRootEffect_hooker = new CallHooker(alloc_BloodMutilationRootEffect_caller,
							    adjust_BloodMutilationRootEffect_params_above_4);

    // We directly detour trigger_VanishMutilationRootEffect_func function to
    // trigger_BloodMutilationRootEffect because their parameters are perfectly the same,
    // even the flow of the function looks totally same.
    trigger_VanishMutilationRootEffect_hooker = new CallHooker(trigger_VanishMutilationRootEffect_caller,
							       trigger_BloodMutilationRootEffect_func);

    // We directly detour trigger_VanishViscosityEffect function to
    // trigger_MutilationViscosityBloodEffect because their parameters are perfectly the same.
    trigger_VanishViscosityEffect_hooker = new CallHooker(trigger_VanishViscosityEffect_caller,
							  trigger_MutilationViscosityBloodEffect_func);
  }
}

/*
 * NGS2 NM Gore Plugin by Nozomi Miyamori is marked with CC0 1.0
 * This module restores NG2 mutilation gore effects (e.g. Sword, Fang, etc.).
 */
#include "util.hpp"
#include "gore.hpp"

using namespace std;
using namespace ngs2::nm::util;
using namespace ngs2::nm::plugin::effect::gore;

namespace {
  constinit uint8_t trigger_VanishMutilationRootEffect_func_pattern[] = {
    0x40, 0x53, 0x41, 0x55, 0x41, 0x56, 0x48, 0x83,
    0xec, 0x60, 0x44, 0x8b, 0xea, 0x49, 0x8b, 0xc9,
  };
  constinit uint8_t trigger_BloodMutilationRootEffect_func_pattern[] = {
    0x48, 0x89, 0x5c, 0x24, 0x18, 0x48, 0x89, 0x74,
    0x24, 0x20, 0x41, 0x56, 0x48, 0x83, 0xec, 0x60,
  };
  constinit uint8_t trigger_VanishViscosityEffect_func_pattern[] = {
    0x44, 0x89, 0x44, 0x24, 0x18, 0x53, 0x41, 0x54,
    0x41, 0x56, 0x41, 0x57, 0x48, 0x81, 0xec, 0xb8,
    0x00, 0x00, 0x00,
  };
  constinit uint8_t trigger_MutilationViscosityBloodEffect_func_pattern[] = {
    0x40, 0x53, 0x55, 0x57, 0x48, 0x81, 0xec, 0xa0,
    0x00, 0x00, 0x00, 0x8b, 0xac, 0x24, 0xe0, 0x00,
    0x00, 0x00,
  };
  constinit uint8_t adjust_BloodMutilationRootEffect_params_func_pattern[] = {
    0x48, 0x83, 0xec, 0x48, 0x4c, 0x8b, 0xd1, 0x41,
    0x8b, 0xc8, 0x83, 0xe9, 0x04, 0x74, 0x5c, 0x83,
    0xe9, 0x01,
  };

  uintptr_t trigger_VanishMutilationRootEffect_func;
  uintptr_t trigger_BloodMutilationRootEffect_func;
  uintptr_t trigger_VanishViscosityEffect_func;
  uintptr_t trigger_MutilationViscosityBloodEffect_func;
  uintptr_t adjust_BloodMutilationRootEffect_params_func;

  HookMap *hook_map;

  bool
  init_trigger_BloodMutilationRootEffect ()
  {
    // We overwrites the call address of alloc_BloodMutilationRootEffect in
    // trigger_BloodMutilationRootEffect with the adjust_BloodMutilationRootEffect_params, which calls
    // alloc_BloodMutilationRootEffect internally.
    const uintptr_t addr = trigger_BloodMutilationRootEffect_func + 0x196;

    // We inject the small code that adds 2 to param3 of adjust_BloodMutilationRootEffect_params, which
    // makes the function can adjust params to create bigger blood jet and wider blood shower.
    // If the param3 of bit 8 is high, it looks like the effect is the minor version of the one bit 8 is low.
    const uintptr_t adjust_BloodMutilationRootEffect_params_func_above4 = adjust_BloodMutilationRootEffect_params_func - 4;
    const uint8_t add_r8b_0x02[] = { 0x41, 0x80, 0xc0, 0x02 };
    WriteMemory(adjust_BloodMutilationRootEffect_params_func_above4, add_r8b_0x02);

    const uint32_t adjust_BloodMutilationRootEffect_params_func_off
      = -(addr + 5 - adjust_BloodMutilationRootEffect_params_func_above4);
    WriteMemory(addr + 1, adjust_BloodMutilationRootEffect_params_func_off);

    // We directly detour trigger_VanishMutilationRootEffect_func function to
    // trigger_BloodMutilationRootEffect because their parameters are perfectly the same,
    // even the flow of the function looks totally same.
    return hook_map->hook (trigger_VanishMutilationRootEffect_func,
		 reinterpret_cast<uint64_t>(trigger_BloodMutilationRootEffect_func));
  }

  bool
  init_trigger_MutilationViscosityBloodEffect ()
  {
    // We directly detour trigger_VanishViscosityEffect function to
    // trigger_MutilationViscosityBloodEffect because their parameters are perfectly the same.
    return hook_map->hook (trigger_VanishViscosityEffect_func,
		 reinterpret_cast<uint64_t>(trigger_MutilationViscosityBloodEffect_func));
  }

}

namespace ngs2::nm::plugin::effect::gore::mutil {
  void
  init ()
  {
    switch (binary_kind)
      {
      case NGS2_BINARY_KIND::STEAM_JP:
	trigger_VanishMutilationRootEffect_func = base_of_image + 0x1229190;
	trigger_BloodMutilationRootEffect_func = base_of_image + 0x121fde0;
	trigger_VanishViscosityEffect_func = base_of_image + 0x1228e10;
	trigger_MutilationViscosityBloodEffect_func = base_of_image + 0x121fba0;
	adjust_BloodMutilationRootEffect_params_func = base_of_image + 0x1206b90;
	break;
      case NGS2_BINARY_KIND::STEAM_AE:
	trigger_VanishMutilationRootEffect_func = base_of_image + 0x12293d0;
	trigger_BloodMutilationRootEffect_func = base_of_image + 0x1220060;
	trigger_VanishViscosityEffect_func = base_of_image + 0x1229050;
	trigger_MutilationViscosityBloodEffect_func = base_of_image + 0x121fe20;
	adjust_BloodMutilationRootEffect_params_func = base_of_image + 0x1206e30;
	break;
      }
    hook_map = new HookMap;
    if (!init_trigger_BloodMutilationRootEffect ())
      throw std::runtime_error ("INIT FAILED: nm::plugin::effect::gore::mutil::init_trigger_BloodMutilationRootEffect()");
    if (!init_trigger_MutilationViscosityBloodEffect ())
      throw std::runtime_error ("INIT FAILED: nm::plugin::effect::gore::mutil::init_trigger_MutilationViscosityBloodEffect()");
  }

  void
  deinit ()
  {
    delete hook_map;
  }
}

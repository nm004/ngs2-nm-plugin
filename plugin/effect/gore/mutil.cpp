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
  const uint8_t trigger_VanishMutilationRootEffect_func_pattern[] = {
    0x40, 0x53, 0x41, 0x55, 0x41, 0x56, 0x48, 0x83,
    0xec, 0x60, 0x44, 0x8b, 0xea, 0x49, 0x8b, 0xc9,
  };
  const uint8_t trigger_BloodMutilationRootEffect_func_pattern[] = {
    0x48, 0x89, 0x5c, 0x24, 0x18, 0x48, 0x89, 0x74,
    0x24, 0x20, 0x41, 0x56, 0x48, 0x83, 0xec, 0x60,
  };
  const uint8_t trigger_VanishViscosityEffect_func_pattern[] = {
    0x44, 0x89, 0x44, 0x24, 0x18, 0x53, 0x41, 0x54,
    0x41, 0x56, 0x41, 0x57, 0x48, 0x81, 0xec, 0xb8,
    0x00, 0x00, 0x00,
  };
  const uint8_t trigger_MutilationViscosityBloodEffect_func_pattern[] = {
    0x40, 0x53, 0x55, 0x57, 0x48, 0x81, 0xec, 0xa0,
    0x00, 0x00, 0x00, 0x8b, 0xac, 0x24, 0xe0, 0x00,
    0x00, 0x00,
  };
  const uint8_t adjust_BloodMutilationRootEffect_params_func_pattern[] = {
    0x48, 0x83, 0xec, 0x48, 0x4c, 0x8b, 0xd1, 0x41,
    0x8b, 0xc8, 0x83, 0xe9, 0x04, 0x74, 0x5c, 0x83,
    0xe9, 0x01,
  };
  const uint8_t update_SUP_nodeobj_visibility_func_pattern[] = {
    0x40, 0x55, 0x41, 0x0f, 0xb6, 0xe8, 0x83, 0xfa, 
    0x3f, 0x0f, 0x87, 0x08, 0x01, 0x00, 0x00,
  };
  const uintptr_t model_node_layer_list_offset_list = VA (0x1edd10 + data_section_rva);
}

namespace {
#if NINJA_GAIDEN_SIGMA_2_TARGET_STEAM_JP
  const uintptr_t trigger_VanishMutilationRootEffect_func = VA_PRINT_DECL (trigger_VanishMutilationRootEffect_func);
  const uintptr_t trigger_BloodMutilationRootEffect_func = VA_PRINT_DECL (trigger_BloodMutilationRootEffect_func);
  const uintptr_t trigger_VanishViscosityEffect_func = VA_PRINT_DECL (trigger_VanishViscosityEffect_func);
  const uintptr_t trigger_MutilationViscosityBloodEffect_func = VA_PRINT_DECL (trigger_MutilationViscosityBloodEffect_func);
  const uintptr_t adjust_BloodMutilationRootEffect_params_func = VA_PRINT_DECL (adjust_BloodMutilationRootEffect_params_func);
  const uintptr_t update_SUP_nodeobj_visibility_func = VA_PRINT_DECL (update_SUP_nodeobj_visibility_func);
#elif  NINJA_GAIDEN_SIGMA_2_TARGET_STEAM_AE
  const uintptr_t trigger_VanishMutilationRootEffect_func = VA (0x12293d0);
  const uintptr_t trigger_BloodMutilationRootEffect_func = VA (0x1220060);
  const uintptr_t trigger_VanishViscosityEffect_func = VA (0x1229050);
  const uintptr_t trigger_MutilationViscosityBloodEffect_func = VA (0x121fe20);
  const uintptr_t adjust_BloodMutilationRootEffect_params_func = VA (0x1206e30);
  const uintptr_t update_SUP_nodeobj_visibility_func = VA (0x1455880);
#endif

  HookMap *hook_map;

  void
  update_nodeobj_visibility (struct model_node_layer *nl, uint8_t visibility, bool is_top) noexcept;

  // This function is called when delimb happens. This updates the visibility of
  // SUP_* NodeObj that has nodeobj_idx for its parameters. That makes delimbed body
  // parts are separated from the body.
  //
  // We have rewritten this function because an visual glitch happens with the default
  // implementation. Specifically, when you delimb certain enemie's head such as Brown
  // Claw Ninja, Purple Mage and Gaja, some vertices of their accessories (WGT_* NodeObj)
  // are still tied to their body, and that creates very stretched objects. We just remove
  // their accessories here too, to avoid that glitch. Therefore, Brown Ninjas and Purple
  // Mages reveal their faces when players delimb their heads (but the very short period
  // time: until their heads land :)
  void
  update_SUP_nodeobj_visibility (struct model &mdl, uint32_t nodeobj_idx, uint8_t visibility) noexcept
  {
    // It looks like passed visibility is always zero.
    const uintptr_t mnl_list_offset = reinterpret_cast<uintptr_t *>
      (model_node_layer_list_offset_list)[mdl.info_idx];
    const auto nl = reinterpret_cast<model_node_layer **>(*mdl.p_state + mnl_list_offset)[nodeobj_idx];
    update_nodeobj_visibility (nl->first_child, visibility, true);
  }

  void
  update_nodeobj_visibility (struct model_node_layer *nl, uint8_t visibility, bool is_top) noexcept
  {
    enum {
      MOT = 1,
      WGT = 3,
      SUP = 4,
    };
    while (nl)
      {
	switch (nl->nodeobj_type)
	  {
	  case MOT:
	    // We recursively make descendent WGTs invisible too.
	    update_nodeobj_visibility (nl->first_child, visibility, false);
	    break;
	  case WGT:
	    nl->is_visible1 = visibility;
	    break;
	  case SUP:
	    if (is_top)
	      nl->is_visible1 = visibility;
	    break;
	  }
	nl = nl->next_sibling;
      }
  }

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
    WriteMemory(adjust_BloodMutilationRootEffect_params_func_above4, &add_r8b_0x02, sizeof(add_r8b_0x02));

    // two's complement
    const uint32_t adjust_BloodMutilationRootEffect_params_func_off
      = ~static_cast<uint32_t>(addr + 5 - adjust_BloodMutilationRootEffect_params_func_above4) + 1;
    WriteMemory(addr + 1, &adjust_BloodMutilationRootEffect_params_func_off, sizeof(adjust_BloodMutilationRootEffect_params_func_off));

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

  bool
  init_update_SUP_nodeobj_visibility ()
  {
    // This makes inline-hook working.
    const uint8_t nop6[] = { 0x66, 0x0f, 0x1f, 0x44, 0x00, 0x00 };
    const uint8_t nop7[] = { 0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00 };
    WriteMemory (update_SUP_nodeobj_visibility_func + 0x9, nop6, sizeof(nop6));
    WriteMemory (update_SUP_nodeobj_visibility_func + 0x9, nop7, sizeof(nop7));

    return hook_map->hook (update_SUP_nodeobj_visibility_func,
		 reinterpret_cast<uintptr_t>(update_SUP_nodeobj_visibility));
  }
}

namespace ngs2::nm::plugin::effect::gore::mutil {
  void
  init ()
  {
    hook_map = new HookMap;
    if (!init_trigger_BloodMutilationRootEffect ())
      throw std::runtime_error ("FAILED: nm_effect::gore::mutil::init_trigger_BloodMutilationRootEffect()");
    if (!init_trigger_MutilationViscosityBloodEffect ())
      throw std::runtime_error ("FAILED: nm_effect::gore::mutil::init_trigger_MutilationViscosityBloodEffect()");
    if (!init_update_SUP_nodeobj_visibility ())
      throw std::runtime_error ("FAILED: nm_effect::gore::mutil::init_update_SUP_nodeobj_visibility()");
  }

  void
  deinit ()
  {
    delete hook_map;
  }
}

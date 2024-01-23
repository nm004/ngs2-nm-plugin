/*
 * NGS2 NM Gore Plugin by Nozomi Miyamori is marked with CC0 1.0
 * This module restores NG2 mutilation gore effects (e.g. Sword, Fang, etc.).
 */
#include "util.hpp"
#include "gore.hpp"

namespace nm_effect::gore::mutil {
  const uintptr_t trigger_VanishMutilationRootEffect_func = VA (0x12293d0);
  const uintptr_t trigger_BloodMutilationRootEffect_func = VA (0x1220060);
  const uintptr_t trigger_VanishViscosityEffect_func = VA (0x1229050);
  const uintptr_t trigger_MutilationViscosityBloodEffect_func = VA (0x121fe20);
  const uintptr_t adjust_BloodMutilationRootEffect_params_func = VA (0x1206e30);
  const uintptr_t update_SUP_nodeobj_visibility_func = VA (0x1455880);
  const uintptr_t model_node_layer_list_offset_list = VA (0x1e38d10);
}

using namespace std;
using namespace nm_effect::gore;
using namespace nm_effect::gore::mutil;

namespace {
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
    return hook (trigger_VanishMutilationRootEffect_func,
		 reinterpret_cast<uint64_t>(trigger_BloodMutilationRootEffect_func));
  }

  bool
  init_trigger_MutilationViscosityBloodEffect ()
  {
    // We directly detour trigger_VanishViscosityEffect function to
    // trigger_MutilationViscosityBloodEffect because their parameters are perfectly the same.
    return hook (trigger_VanishViscosityEffect_func,
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

    return hook (update_SUP_nodeobj_visibility_func,
		 reinterpret_cast<uintptr_t>(update_SUP_nodeobj_visibility));
  }
}

namespace nm_effect::gore::mutil {
  void
  init ()
  {
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
    detours.clear ();
  }
}

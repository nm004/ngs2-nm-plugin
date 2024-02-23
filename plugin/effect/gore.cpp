/*
 * NGS2 NM Plugin by Nozomi Miyamori is marked with CC0 1.0.
 * This file is a part of NGS2 NM Plugin.
 *
 * This module restores NG2 misc gore effect.
 */
#include "util.hpp"
#include "hook.hpp"
#include "gore.hpp"
#include <limits>
#include <cstdint>

using namespace std;
using namespace ngs2::nm::util;
using namespace ngs2::nm::plugin::effect::gore;

namespace {
  void
  update_SUP_nodeobj_visibility (struct model &mdl, uint32_t nodeobj_idx);

  void
  update_nodeobj_visibility (struct model_node_layer *nl, uint8_t visibility, bool is_top);

  InlineHooker<decltype(&update_SUP_nodeobj_visibility)> *update_SUP_nodeobj_visibility_hooker;
  CallHooker *trigger_hit_effect_hooker1;
  CallHooker *trigger_hit_effect_hooker2;

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
  update_SUP_nodeobj_visibility (struct model &mdl, uint32_t nodeobj_idx)
  {
    // Param3 that we ignore is a pointer to somewhere in the memory actually.
    // The original implementation converts param3 to byte (not byte*!),
    // then the function uses it as the new visibility value! That worked
    // somehow unbelevably.

    uintptr_t model_node_layer_list_offset_list = start_of_data + 0x69ad10;

    uintptr_t mnl_list_offset = reinterpret_cast<uintptr_t *>
      (model_node_layer_list_offset_list)[mdl.info_idx];
    auto nl = reinterpret_cast<model_node_layer **>
      (*mdl.p_state + mnl_list_offset)[nodeobj_idx];
    update_nodeobj_visibility (nl->first_child, 0, true);
  }

  void
  update_nodeobj_visibility (struct model_node_layer *nl,
			     uint8_t visibility,
			     bool is_top)
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
}

namespace ngs2::nm::plugin::effect::gore {
  
  void
  init ()
  {
    uintptr_t object_fade_out_start_time = start_of_data + 0x169cf4;
    uintptr_t momiji_bow_attack_category = start_of_data + 0x52428;
    uintptr_t ryu_bow_attack_category = start_of_data + 0x545c8;
    uintptr_t rachel_gun_attack_category = start_of_data + 0x55ca8;

    uintptr_t update_SUP_nodeobj_visibility_func;
    uintptr_t trigger_delimb_effect_func;
    uintptr_t trigger_hit_effect_func;
    uintptr_t calc_trigger_hit_effect_param_func;

    uintptr_t trigger_delimb_effect_limb_time_limit_imm_ofs;
    uintptr_t trigger_delimb_effect_hs_intens_imm_ofs;

    switch (binary_kind)
      {
      case NGS2_BINARY_KIND::STEAM_JP:
	update_SUP_nodeobj_visibility_func = base_of_image + 0x14556a0;
	trigger_delimb_effect_func = base_of_image + 0x1457c50;
	trigger_hit_effect_func = base_of_image + 0x10474f0;
	calc_trigger_hit_effect_param_func = base_of_image + 0x1047220;

	trigger_delimb_effect_limb_time_limit_imm_ofs =
	  trigger_delimb_effect_func + 0x2c5 + 1;
	trigger_delimb_effect_hs_intens_imm_ofs =
	  trigger_delimb_effect_func + 0x181 + 6;
	break;
      case NGS2_BINARY_KIND::STEAM_AE:
	update_SUP_nodeobj_visibility_func = base_of_image + 0x1455880;
	trigger_delimb_effect_func = base_of_image + 0x1457df0;
	trigger_hit_effect_func = base_of_image + 0x1047790;
	calc_trigger_hit_effect_param_func = base_of_image + 0x10474c0;

	trigger_delimb_effect_limb_time_limit_imm_ofs =
	  trigger_delimb_effect_func + 0x2a9 + 1;
	trigger_delimb_effect_hs_intens_imm_ofs =
	  trigger_delimb_effect_func + 0x17d + 6;
	break;
      }

    // This circumvents the head dismemberment disabler.
    if (binary_kind == NGS2_BINARY_KIND::STEAM_JP)
      {
	constexpr uint8_t nop14[] = {
	  0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00,
	  0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00,
	};

	constexpr uint8_t nop16[] = {
	  0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00,
	  0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00,
	};

	constexpr uint8_t nop19[] = {
	  0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00,
	  0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00,
	  0x0F, 0x1F, 0x44, 0x00, 0x00
	};

	constexpr uint8_t nop23[] = {
	  0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00,
	};

	constexpr uint8_t nop24[] = {
	  0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	constexpr uint8_t nop25[] = {
	  0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00,
	};

	constexpr uint8_t nop28[] = {
	  0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00,
	  0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00,
	  0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00,
	  0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00,
	};

	uintptr_t update_dismembered_limb_state_func = base_of_image + 0x1456d20;

	WriteMemory (trigger_delimb_effect_func + 0x2a0, nop14); // for crush
	WriteMemory (trigger_delimb_effect_func + 0x795, nop14); // for mutil
	WriteMemory (update_dismembered_limb_state_func + 0xac, nop24);
	WriteMemory (update_dismembered_limb_state_func + 0x151, nop23);
	WriteMemory (base_of_image + 0x145e2e0 + 0x183, nop16);

	// The following is not required but for completness.
	WriteMemory (base_of_image + 0x0c31150 + 0x40, nop19);
	WriteMemory (base_of_image + 0x145e2e0 + 0x544, nop25);
	WriteMemory (base_of_image + 0x14cfe30 + 0x12e, nop28);
      }

    // Credits: Fiend Busa
    // This is the time how long the dismembered limbs remain.
    // They set 1 by default, which means limbs disappear immediately.
    // Instead, we set very very long time limit.
    WriteMemory (trigger_delimb_effect_limb_time_limit_imm_ofs,
		 static_cast<uint32_t>(0x7fffffff));

    // Credits: Fiend Busa
    // This is the hitstop (micro freeeze) intensity when you trigger delimb.
    // The higher the value, the longer it freezes.
    // They set 3 by default.
    // const uintptr_t delimb_hit_stop_intensity = base_of_image + 0x212160c;
    WriteMemory (trigger_delimb_effect_hs_intens_imm_ofs, static_cast<uint32_t>(0));

    // Credits: Fiend Busa
    // Corpses will not fade out.
    WriteMemory (object_fade_out_start_time, numeric_limits<float>::infinity ());

    // This circumvents the blocking to trigger the EFF_ArrowHitBlood.
    // We have chosen `and' over `xor' becase it has the same size of codes.
    constexpr uint8_t and_eax_0[] = { 0x83, 0xe0, 0x00 };
    uintptr_t addr = trigger_hit_effect_func + 0x10f8;
    WriteMemory (addr, and_eax_0);

    // To make EFF_ArrowHitBlood work.
    constexpr uint16_t attack_category = 0x1002;
    WriteMemory (ryu_bow_attack_category, attack_category);
    WriteMemory (momiji_bow_attack_category, attack_category);
    WriteMemory (rachel_gun_attack_category, attack_category);

    // This restores the blood particle effect like vanilla NG2. Param1 of
    // trigger_hit_effect() is a kind of attack. 5th byte of Param1 is related
    // to the blood particle. It seems that 0x10000 or 0x20000 are only effective,
    // however when you specify 0x20000, some attacks does not produce blood
    // particle (e.g. Shuriken).
    constexpr uint8_t or_ecx_0x10000[] = { 0x81, 0xc9, 0x00, 0x00, 0x01, 0x00 };
    WriteMemory (trigger_hit_effect_func - sizeof(or_ecx_0x10000), or_ecx_0x10000);
    trigger_hit_effect_hooker1 =
      new CallHooker (calc_trigger_hit_effect_param_func + 0x26e,
		      trigger_hit_effect_func - sizeof(or_ecx_0x10000));

    trigger_hit_effect_hooker2 =
      new CallHooker (calc_trigger_hit_effect_param_func + 0x29d,
		      trigger_hit_effect_func - sizeof(or_ecx_0x10000));


    update_SUP_nodeobj_visibility_hooker =
      new InlineHooker<decltype(&update_SUP_nodeobj_visibility)>
      (update_SUP_nodeobj_visibility_func, &update_SUP_nodeobj_visibility);
  }
}

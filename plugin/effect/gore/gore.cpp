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
    // The original implementation converts param3 to byte (not byte*!), then the function uses
    // it as the new visibility value! That worked somehow unbelevably.

    uintptr_t model_node_layer_list_offset_list = start_of_data + 0x69ad10;

    uintptr_t mnl_list_offset = reinterpret_cast<uintptr_t *>
      (model_node_layer_list_offset_list)[mdl.info_idx];
    auto nl = reinterpret_cast<model_node_layer **>(*mdl.p_state + mnl_list_offset)[nodeobj_idx];
    update_nodeobj_visibility (nl->first_child, 0, true);
  }

  void
  update_nodeobj_visibility (struct model_node_layer *nl, uint8_t visibility, bool is_top)
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
    uintptr_t momiji_bow_attack_type_id = start_of_data + 0x52428;
    uintptr_t ryu_bow_attack_type_id = start_of_data + 0x545c8;
    uintptr_t rachel_gun_attack_type_id = start_of_data + 0x55ca8;

    uintptr_t update_SUP_nodeobj_visibility_func;
    uintptr_t trigger_delimb_effect_func;
    uintptr_t make_hit_effect_func;

    uintptr_t trigger_delimb_effect_limb_time_limit_imm_ofs;
    uintptr_t trigger_delimb_effect_hs_intens_imm_ofs;

    switch (binary_kind)
      {
      case NGS2_BINARY_KIND::STEAM_JP:
	update_SUP_nodeobj_visibility_func = base_of_image + 0x14556a0;
	trigger_delimb_effect_func = base_of_image + 0x1457c50;
	make_hit_effect_func = base_of_image + 0x10474f0;

	trigger_delimb_effect_limb_time_limit_imm_ofs = trigger_delimb_effect_func + 0x2c5 + 1;
	trigger_delimb_effect_hs_intens_imm_ofs = trigger_delimb_effect_func + 0x181 + 6;
	break;
      case NGS2_BINARY_KIND::STEAM_AE:
	update_SUP_nodeobj_visibility_func = base_of_image + 0x1455880;
	trigger_delimb_effect_func = base_of_image + 0x1457df0;
	make_hit_effect_func = base_of_image + 0x1047790;

	trigger_delimb_effect_limb_time_limit_imm_ofs = trigger_delimb_effect_func + 0x2a9 + 1;
	trigger_delimb_effect_hs_intens_imm_ofs = trigger_delimb_effect_func + 0x17d + 6;
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
    WriteMemory (trigger_delimb_effect_limb_time_limit_imm_ofs, static_cast<uint32_t>(0x7fffffff));


    // Credits: Fiend Busa
    // This is the hitstop (micro freeeze) intensity when you trigger delimb.
    // The higher the value, the longer it freezes.
    // They set 3 by default.
    // const uintptr_t delimb_hit_stop_intensity = base_of_image + 0x212160c;
    WriteMemory (trigger_delimb_effect_hs_intens_imm_ofs, static_cast<uint32_t>(0));

    // Credits: Fiend Busa
    // Corpses will not fade out.
    WriteMemory (object_fade_out_start_time, numeric_limits<float>::infinity ());

    {
      // This circumvents the blocking to trigger the EFF_ArrowHitBlood.
      // We have chosen `and' over `xor' becase it has the same size of codes.
      constexpr uint8_t and_eax_0[] = { 0x83, 0xe0, 0x00 };
      uintptr_t addr = make_hit_effect_func + 0x10f8;
      WriteMemory (addr, and_eax_0);

      // To make EFF_ArrowHitBlood work.
      constexpr uint16_t attack_type_id = 0x1002;
      WriteMemory (ryu_bow_attack_type_id, attack_type_id);
      WriteMemory (momiji_bow_attack_type_id, attack_type_id);
      WriteMemory (rachel_gun_attack_type_id, attack_type_id);
    }

    update_SUP_nodeobj_visibility_hooker =
      new InlineHooker<decltype(&update_SUP_nodeobj_visibility)> (update_SUP_nodeobj_visibility_func,
								  &update_SUP_nodeobj_visibility);
  }
}

namespace {
  // const uintptr_t number_of_delimbed_limbs_list = base_of_image + 0x21b3000;
  // const uintptr_t limb_appearance_timer_list = base_of_image + 0x6426880;
  // we don't use this struct for now, but keep this for reference.
  struct limb_appearance_timer {
    uint32_t is_timer_used;
    uint32_t _maybe_nodeobj_item_id;
    uint32_t data0x8;
    uint32_t data0xc;

    uintptr_t hielay_item;
    uint32_t data0x18;
    uint32_t data0x1c;

    uintptr_t _maybe_limb_pos_matrix;
    uint64_t data0x28;

    uint32_t data0x30;
    uint32_t data0x34;
    uint32_t data0x38;
    uint32_t data0x3c;

    uint32_t data0x40;
    uint32_t data0x44;
    uint32_t data0x48;
    uint32_t _maybe_part_id;

    uint32_t data0x50;
    uint32_t data0x54;
    uintptr_t _maybe_another_limb_pos_matrix;

    uint32_t elapsed_time;
    uint32_t time_limit;
    uint64_t data0x68;
  };
}

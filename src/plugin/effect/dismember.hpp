/*
 * NGS2 NM Plugin by Nozomi Miyamori is marked with CC0 1.0.
 * This file is a part of NGS2 NM Plugin.
 */

#ifndef NGS2_PLUGIN_EFFECT_DISMEMBER
#define NGS2_PLUGIN_EFFECT_DISMEMBER

#include <cstdint>

namespace plugin::dismember {
  struct model;
  struct model_tmc_relation;
  struct model_geo;
  struct model_node_layer;
  struct limb_appearance_timer;
}

struct plugin::dismember::model {
  uintptr_t p_state[3];
  uint8_t mdl_id;
  uint8_t data0x19;
  uint8_t info_idx;
  uint8_t data0x1b;
  uint8_t data0x1c;
  uint8_t list_pos;
  uint8_t data0x1e;
  uint8_t seq_num;
  uint8_t data0x1a[0x10];
};

struct plugin::dismember::model_tmc_relation {
  uint8_t data0x0[8];
  uintptr_t tmc1;
  struct model_node_layer &no_laeyer;
  struct model_geo &og_list;
  uintptr_t tmc2;
};

/*
  * model_node_layer example: e_you_a (Purple Mage)
  *
  * In case of p.nodeobj == MOTe_you_alhip:
  * p.parent == MOTe_you_ahips
  * p.first_child == SUP_lhip
  * p.next_sibling == MOTe_you_achest

  MOTe_you_ahips
  |-OPTscat16
  |-OPTscat15
  ...
  |-OPTscat0
  |
  |-MOTe_you_arhip
  |
  |-MOTe_you_alhip
  | |-SUP_lhip
  | |
  | |-MOTe_you_alshin
  |   |-SUP_lshin
  |   |-MOTe_you_alfoot
  |
  |-MOTe_you_achest
    |-SUP_chest
    |-MOTe_you_arsholder
    |
    |-MOTe_you_alshoulder
    | |-SUP_lshld
    | |-MOTe_you_alfore
    |   |-SUP_lfore
    |   |-MOTe_you_alhand
    |
    |-MOTe_you_ahead
      |-WGT_acs_maedare
      |-OPTcorps_maedare
      |-OPT_acs_maedare_r03
      |-OPT_acs_maedare_r02
      ...
      |-OPT_acs_maedare_l03
      ...
      |-OPT_acs_maedare_l00
*/
  
struct plugin::dismember::model_node_layer {
  uintptr_t nodeobj;
  uintptr_t hielay;
  uint32_t &nodeobj_type;
// for scanning entire layer (unordered, maybe)
  model_node_layer *next;
  model_node_layer *parent;
  model_node_layer *first_child;
  model_node_layer *next_sibling;
  model_geo &geo;
  model_tmc_relation &mt_relation;
  uintptr_t data0x48;
  uint8_t is_visible1;
  uint8_t is_visible2;
  uint16_t data0x52;
  uint32_t data0x54;
  uintptr_t data0x58;
  uint64_t data0x60;
};

struct plugin::dismember::model_geo {
  float data0x0[4];
  uintptr_t p_objgeo;
  uint32_t &nodeobj_idx;
  uint32_t &nodeobj_type;
  // Is there any difference between model_geo.is_visible
  // and model_node_layer.is_visible1?
  uint16_t is_visible;
  uint16_t data0x1a;
  uint32_t data0x1c;
  struct objgeo_list *next;
  // imcomplete
};

// const uintptr_t number_of_delimbed_limbs_list = base_of_image + 0x21b3000;
// const uintptr_t limb_appearance_timer_list = base_of_image + 0x6426880;
// we don't use this struct for now, but keep this for reference.
struct plugin::dismember::limb_appearance_timer {
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

#endif

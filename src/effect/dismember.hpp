/*
 * NINJA GAIDEN Master Collection NM Plugin by Nozomi Miyamori
 * is marked with CC0 1.0. This file is a part of NINJA GAIDEN
 * Master Collection NM Plugin.
 */

#ifndef NGS2_NM_PLUGIN_EFFECT_DISMEMBER_H
#define NGS2_NM_PLUGIN_EFFECT_DISMEMBER_H

#include "util.hpp"
#include <utility>
#include <cstdint>

namespace nm {

struct model {
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

struct model_tmc_relation {
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
  
struct model_node_layer {
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

struct model_geo {
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

void
update_NodeObj_visibility (struct model_node_layer *nl, uint8_t visibility, bool is_top);

// This function is called when dismemberment happens. This updates the visibility of
// SUP_* NodeObj that has nodeobj_id as its ID. This makes parts of the body
// separated from the body.
//
// We have rewritten this function because an visual glitch happens with the default
// implementation. Specifically, when you dismember certain enemie's head such as Brown
// Claw Ninja, Purple Mage and Gaja, some vertices of their accessories (WGT_* NodeObj)
// are still tied to their body, and that creates very stretched objects. We just remove
// their accessories here too, to avoid that glitch. Therefore, Brown Ninjas and Purple
// Mages reveal their faces when players dismember their heads (but the very short period
// time: until their heads land :)
template <uintptr_t model_node_layer_list_offset_list_rva>
void
update_SUP_NodeObj_visibility (struct model &mdl, uint32_t nodeobj_id)
{
  using namespace std;
  using namespace nm;

  uintptr_t model_node_layer_list_offset_list
    = base_of_image + model_node_layer_list_offset_list_rva;
  // Param3 that we ignore is a pointer to somewhere in the memory.
  // The original implementation converts param3 to byte (not byte *!),
  // then the function uses it as the new visibility value! That worked
  // somehow unbelevably.
  uintptr_t mnl_list_offset = reinterpret_cast<uintptr_t *>
    (model_node_layer_list_offset_list)[mdl.info_idx];
  auto nl = reinterpret_cast<model_node_layer **>
    (*mdl.p_state + mnl_list_offset)[nodeobj_id];
  update_NodeObj_visibility (nl->first_child, 0, true);
}

void
update_NodeObj_visibility (struct model_node_layer *nl, uint8_t visibility, bool is_top)
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
	  update_NodeObj_visibility (nl->first_child, visibility, false);
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

// It returns the OPTscatN NodeObj indices in the TMC data. The target enemy's TMC
// data is obtained from the first param. We have reimplemented this function because
// the original function has a bug, which return immedeately search first 0x1f nodes.
template <uintptr_t model_tmc_relation_offset_list_rva>
int
get_OPTscat_indices (struct model &mdl, uint32_t *indices_out)
{
  using namespace std;
  using namespace nm;

  uintptr_t model_tmc_relation_offset_list = base_of_image + model_tmc_relation_offset_list_rva;
  uintptr_t mt_rel_ofs = reinterpret_cast<uintptr_t *>
    (model_tmc_relation_offset_list)[mdl.info_idx];

  uintptr_t tmc = (*reinterpret_cast<model_tmc_relation **> (*mdl.p_state + mt_rel_ofs))->tmc1;
  uint32_t *offset_table = reinterpret_cast<uint32_t *> (tmc + *reinterpret_cast<uintptr_t *>(tmc + 0x20));
  uint32_t start_of_optional_data = *reinterpret_cast<uint32_t *>(tmc + 0x40);

  // Optscat's type is 5.
  // first = num of nodes, second = first index of the nodes.
  auto &header_of_opt = reinterpret_cast<pair<uint16_t, uint16_t> *> (tmc + offset_table[start_of_optional_data])[5];
  uint32_t *obj_type_info = reinterpret_cast<uint32_t *> (tmc + offset_table[start_of_optional_data + 1]);

  // They expect the size of indices_out is 16.
  int n = 0;
  for (int i = 0; n < 0x10 && i < header_of_opt.second; i++)
    {
      int j = header_of_opt.first + i;

      // The address calculation below looks odd, though, it is what it is.
      uint32_t &x = obj_type_info[j];
      uint32_t *t = reinterpret_cast<uint32_t *> (reinterpret_cast<uintptr_t> (&x) + (x & 0x0fffffff));

      // t[0] is major type, t[1] is sub type, and t[2] is sequential id (if exists).
      // sub type of OptscatN is 3.
      if (t[1] == 3)
	{
	  indices_out[n++] = j;
	}
    }

  return n;
}

} // namespace nm

#endif // NGS2_NM_PLUGIN_EFFECT_DISMEMBER_H

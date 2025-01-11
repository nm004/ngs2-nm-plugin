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

namespace nm
{

struct game_object
{
  uintptr_t data_ptr0x0;
  uintptr_t data_ptr0x8;
  uintptr_t data_ptr0x10;
  uint8_t obj_id;
  uint8_t data0x19;
  uint8_t data0x1a;
  uint8_t data0x1b;
  uint8_t data0x1c;
  uint8_t data0x1d;
  uint8_t data0x1e;
  uint8_t obj_group_index;
  // imcomplete
};

struct node_layer
{
  uintptr_t nodeobj;
  uintptr_t hielay;
  uint32_t *nodeobj_type;
// for scanning entire layer (unordered, maybe)
  node_layer *next;
  node_layer *parent;
  node_layer *first_child;
  node_layer *next_sibling;
  uintptr_t ptr0x38;
  uintptr_t ptr0x40;
  uintptr_t data0x48;
  uint8_t is_visible1;
  uint8_t is_visible2;
  uint16_t data0x52;
  uint32_t data0x54;
  uintptr_t data0x58;
  uint64_t data0x60;
};

void
update_NodeObj_visibility (struct node_layer *n, uint8_t visibility, bool is_top);

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
template <uintptr_t object_data_offset_table_rva>
void
update_SUP_NodeObj_visibility (struct game_object &obj, uint32_t nodeobj_idx)
{
  using namespace std;
  using namespace nm;

  auto offsets = reinterpret_cast <uintptr_t (*)[4]> (base_of_image + object_data_offset_table_rva);
  auto n = reinterpret_cast <struct node_layer **> (obj.data_ptr0x0 + offsets[48][obj.data0x1a])[nodeobj_idx];
  update_NodeObj_visibility (n->first_child, 0, true);
}

void
update_NodeObj_visibility (struct node_layer *n, uint8_t visibility, bool is_top)
{
  enum
  {
    MOT = 1,
    WGT = 3,
    SUP = 4,
  };
  while (n)
    {
      switch (*n->nodeobj_type)
      {
      case MOT:
	// We recursively make descendent WGTs invisible too.
	update_NodeObj_visibility (n->first_child, visibility, false);
	break;
      case WGT:
	n->is_visible1 = visibility;
	break;
      case SUP:
	if (is_top)
	  n->is_visible1 = visibility;
	break;
      }
      n = n->next_sibling;
    }
}

} // namespace nm

#endif // NGS2_NM_PLUGIN_EFFECT_DISMEMBER_H

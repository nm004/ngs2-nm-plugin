/*
 * NGS2 NM Plugin by Nozomi Miyamori is marked with CC0 1.0.
 * This file is a part of NGS2 NM Plugin.
 *
 * This module restores NG2 mutilation gore effects (e.g. Sword, Fang, etc.).
 */

#include "util.hpp"
#include "mutil.hpp"
#include "dismember.hpp"
#include <cstdint>
#include <algorithm>
#include <bit>
#include <tuple>

namespace {
  namespace detail {
    using namespace plugin::dismember;
    template <uintptr_t model_node_layer_list_offset_list_rva>
    void
    update_SUP_NodeObj_visibility (struct model &mdl, uint32_t nodeobj_id);

    void
    update_NodeObj_visibility (struct model_node_layer *nl, uint8_t visibility, bool is_top);
  }
}

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
detail::update_SUP_NodeObj_visibility (struct model &mdl, uint32_t nodeobj_id)
{
  uintptr_t model_node_layer_list_offset_list
    = util::base_of_image + model_node_layer_list_offset_list_rva;
  // Param3 that we ignore is a pointer to somewhere in the memory.
  // The original implementation converts param3 to byte (not byte *!),
  // then the function uses it as the new visibility value! That worked
  // somehow unbelevably.
  uintptr_t mnl_list_offset = std::bit_cast<uintptr_t *>
    (model_node_layer_list_offset_list)[mdl.info_idx];
  auto nl = std::bit_cast<model_node_layer **>
    (*mdl.p_state + mnl_list_offset)[nodeobj_id];
  update_NodeObj_visibility (nl->first_child, 0, true);
}

void
detail::update_NodeObj_visibility (struct model_node_layer *nl,
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

namespace {
  namespace steam_ae {
    auto update_SUP_NodeObj_visibility_hook
      = util::InlineHook{0x1455880, detail::update_SUP_NodeObj_visibility<0x1e38d10>};
  }
  namespace steam_jp {
    auto update_SUP_NodeObj_visibility_hook
      = util::InlineHook{0x14556a0, detail::update_SUP_NodeObj_visibility<0x1e37d10>};
  }
}


// We directly detour trigger_VanishMutilationRootEffect_func function to
// trigger_BloodMutilationRootEffect because their parameters are perfectly the same,
// even the flow of the function looks totally same.
namespace {
  namespace steam_ae {
    auto patch1 = util::CallOffsetPatch{0x145866a, 0x1220060};
  }
  namespace steam_jp {
    auto patch1 = util::CallOffsetPatch{0x14584f5, 0x121fde0};
  }
}

// We inject the small code that adds 2 to param3 of
// adjust_BloodMutilationRootEffect_params, which makes the function can adjust
// params to make blood jet larger and wider. If the param3 of bit 8 is high,
// it looks like the effect is the minor version of the one whose bit 8 is low.
//
// Also, We overwrites the call to alloc_BloodMutilationRootEffect with the
// call to adjust_BloodMutilationRootEffect_params, which calls
// alloc_BloodMutilationRootEffect internally.
namespace {
  namespace detail {
    // add r8b, 0x02
    template <uintptr_t rva> auto
    patch2 = util::Patch{rva - 4, util::make_bytes( 0x41, 0x80, 0xc0, 0x02 )};
  }
  namespace steam_ae {
    auto patch2 = detail::patch2<0x1206e30>;
    auto patch3 = util::CallOffsetPatch{0x12201f6, patch2.rva ()};
  }
  namespace steam_jp {
    auto patch2 = detail::patch2<0x1206b90>;
    auto patch3 = util::CallOffsetPatch{0x1206d26, patch2.rva ()};
  }
}


// We directly detour trigger_VanishViscosityEffect function to
// trigger_MutilationViscosityBloodEffect because their parameters are
// perfectly the same.
namespace {
  namespace steam_ae {
    auto patch4 = util::CallOffsetPatch{0x1458761, 0x121fe20};
  }
  namespace steam_jp {
    auto patch4 = util::CallOffsetPatch{0x14585f8, 0x121fba0};
  }
}

// These are for circumventing the head dismemberment disablers
// which can be found in the Asia version of the game.
namespace {
  namespace steam_jp {
    constinit const auto NOP5 = util::make_bytes(
      0x0F, 0x1F, 0x44, 0x00, 0x00
    );
    constinit const auto NOP7 = util::make_bytes(
      0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
    );
    constinit const auto NOP8 = util::make_bytes(
      0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00
    );
    constinit const auto NOP9 = util::make_bytes(
      0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00
    );

    // for crush
    auto patch_jp1 = util::Patch{0x1457ef0, util::concat(NOP7, NOP7)};
    // for mutil
    auto patch_jp2 = util::Patch{0x14583e5, util::concat(NOP7, NOP7)};

    // In order to prevent the head from getting back to his body
    auto patch_jp3 = util::Patch{0x1456dcc, util::concat(NOP8, NOP8, NOP8)};
    auto patch_jp4 = util::Patch{0x1456e71, util::concat(NOP7, NOP8, NOP8)};
    auto patch_jp5 = util::Patch{0x145e463, util::concat(NOP8, NOP8)};

    // The following is not required but for the completness.
    auto patch_jp6 = util::Patch{0x145E824, util::concat(NOP8, NOP8, NOP9)};
    auto patch_jp7 = util::Patch{0xc31190, util::concat(NOP5, NOP7, NOP7)};
    auto patch_jp8 = util::Patch{0x14cff5e, util::concat(NOP7, NOP7, NOP7, NOP7)};
  }
}

void
plugin::steam_ae::apply_mutil_patch ()
{
  using namespace ::steam_ae;
  update_SUP_NodeObj_visibility_hook.attach ();
  patch1.apply ();
  patch2.apply ();
  patch3.apply ();
  patch4.apply ();
}

void
plugin::steam_jp::apply_mutil_patch ()
{
  using namespace ::steam_jp;
  update_SUP_NodeObj_visibility_hook.attach ();
  patch1.apply ();
  patch2.apply ();
  patch3.apply ();
  patch4.apply ();

  patch_jp1.apply ();
  patch_jp2.apply ();
  patch_jp3.apply ();
  patch_jp4.apply ();
  patch_jp5.apply ();
  patch_jp6.apply ();
  patch_jp7.apply ();
  patch_jp8.apply ();
}

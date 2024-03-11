/*
 * NGS2 NM Plugin by Nozomi Miyamori is marked with CC0 1.0.
 * This file is a part of NGS2 NM Plugin.
 *
 * This module restores NG2 crushing gore effects (e.g. Stuff, Tonfa).
 */

#include "util.hpp"
#include "crush.hpp"
#include "dismember.hpp"
#include <cstdint>
#include <bit>

namespace {
  namespace detail {
    using namespace plugin::dismember;

    // This is called from trigger_BloodCrushRootEffect internally.
    // This returns the OPTscatN NodeObj indices in the TMC data.
    // The target (enemy's) TMC data is obtained from the first param.
    // We have reimplemented this function because the original function has a bug.

    template <uintptr_t model_tmc_relation_offset_list_rva>
    int
    get_OPTscat_indices (struct model &mdl, uint32_t *out_indices);
  }
}

template <uintptr_t model_tmc_relation_offset_list_rva>
int
detail::get_OPTscat_indices (struct model &mdl, uint32_t *out_indices)
{
  uintptr_t model_tmc_relation_offset_list
    = util::base_of_image + model_tmc_relation_offset_list_rva;
  uintptr_t mt_rel_ofs = std::bit_cast<uintptr_t *>
    (model_tmc_relation_offset_list)[mdl.info_idx];

  auto &r = **std::bit_cast<model_tmc_relation **>
    (*mdl.p_state + mt_rel_ofs);

  uint32_t *tmc_ofs_tbl = std::bit_cast<uint32_t *>
    (r.tmc1 + *std::bit_cast<uintptr_t *>(r.tmc1 + 0x20));

  struct nodeobj_type_table_info {
    uint16_t idx0;
    uint16_t size;
  };

  uint32_t tti_idx = *std::bit_cast<uint32_t *>(r.tmc1+0x40);
  // OptscatN is type 5 NodeObj.
  auto &ti = std::bit_cast<struct nodeobj_type_table_info *>
    (r.tmc1 + tmc_ofs_tbl[tti_idx])[5];

  // The type offset table is next to the info list
  uint32_t *tt_ofs = std::bit_cast<uint32_t *>
    (r.tmc1 + tmc_ofs_tbl[tti_idx + 1]);

  // They expect the size of out_indices is at most 16.
  int n = 0;
  for (int i = 0; n < 0x10 && i < ti.size; i++)
    {
      uint32_t j = ti.idx0 + i;

      // The address calculation below looks odd, though, it is what it is.
      uint32_t *type = std::bit_cast<uint32_t *>
	(std::bit_cast<uintptr_t>(&tt_ofs[j]) + (tt_ofs[j] & 0x0fffffff));

      // type[0] is major type, type[1] is sub type, and type[2] is some sequential id (if exists).
      // subtype of OptscatN is 3.
      if (type[1] == 3)
	{
	  out_indices[n++] = j;
	}
    }

  return n;
}

namespace {
  namespace steam_ae {
    auto get_OPTscat_indices_hook
      = util::InlineHook{0x144cb00, detail::get_OPTscat_indices<0x1e38f30>};
  }
  namespace steam_jp {
    auto get_OPTscat_indices_hook
      = util::InlineHook{0x144c8e0, detail::get_OPTscat_indices<0x1e37f30>};
  }
}

// We directly detour trigger_VanishCrushRootEffect function to
// trigger_BloodCrushRootEffect, however we need to reorder the parameter,
// so we inject the parameters re-ordering code in the beginning.
namespace {
  namespace detail {
    // xchg rdx, r8
    // xchg rcx, r9
    template <uintptr_t rva>
    auto patch1 = util::Patch{rva-6, util::make_bytes( 0x4c, 0x87, 0xc2, 0x48, 0x87, 0xd1 )};
  }
  namespace steam_ae {
    auto patch1 = detail::patch1<0x0c0fc40>;
    auto patch2 = util::CallOffsetPatch{0x14152e7, patch1.rva ()};
    auto patch3 = util::CallOffsetPatch{0x1415c65, patch1.rva ()};
  }
  namespace steam_jp {
    auto patch1 = detail::patch1<0xc0fcb0>;
    auto patch2 = util::CallOffsetPatch{0x14150b7, patch1.rva ()};
    auto patch3 = util::CallOffsetPatch{0x1415a35, patch1.rva ()};
  }
}

// Credit: enhuhu
// This disables EFF_CommonIzunaBloodExp which produces dim purple effect.
namespace {
  namespace detail {
    // ret
    template <uintptr_t rva>
    auto patch4 = util::Patch{rva, uint8_t{0xc3}};
  }
  namespace steam_ae {
    auto patch4 = detail::patch4<0x1020990>;
  }
  namespace steam_jp {
    auto patch4 = detail::patch4<0x10206f0>;
  }
}


// This disables EFF_CommonSuicideBloodExpPatch which produces dim purple effect.
namespace {
  namespace detail {
    template <uintptr_t rva>
    auto patch5 = util::Patch{rva, uint8_t{0xc3}};
  }
  namespace steam_ae {
    auto patch5 = detail::patch5<0x1020360>;
  }
  namespace steam_jp {
    auto patch5 = detail::patch5<0x10200c0>;
  }
}

void
plugin::steam_ae::apply_crush_patch ()
{
  using namespace ::steam_ae;
  get_OPTscat_indices_hook.attach ();
  patch1.apply ();
  patch2.apply ();
  patch3.apply ();
  patch4.apply ();
  patch5.apply ();
}

void
plugin::steam_jp::apply_crush_patch ()
{
  using namespace ::steam_jp;
  get_OPTscat_indices_hook.attach ();
  patch1.apply ();
  patch2.apply ();
  patch3.apply ();
  patch4.apply ();
  patch5.apply ();
}

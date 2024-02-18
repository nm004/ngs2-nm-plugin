/*
 * NGS2 NM Plugin by Nozomi Miyamori is marked with CC0 1.0.
 * This file is a part of NGS2 NM Plugin.
 *
 * This module restores NG2 crushing gore effects (e.g. Stuff, Tonfa).
 */

#include "util.hpp"
#include "hook.hpp"
#include "gore.hpp"
#include <cstdint>

using namespace std;
using namespace ngs2::nm::util;
using namespace ngs2::nm::plugin::effect::gore;

namespace {
  uint8_t *
  trigger_VanishCrushRootEffect (uintptr_t param1, uintptr_t param2);

  int
  get_OPTscat_indices (struct model &mdl, uint32_t *out_indices);

  uintptr_t trigger_BloodCrushRootEffect_func;
  InlineHooker<decltype(&trigger_VanishCrushRootEffect)> *trigger_VanishCrushRootEffect_hooker;
  InlineHooker<decltype(&get_OPTscat_indices)> *get_OPTscat_indices_hooker;

  uint8_t *
  trigger_VanishCrushRootEffect (uintptr_t param1, uintptr_t param2)
  {
    // trigger_BloodCrushRootEffect calls trigger_BloodCrushRootEffect_helper
    // internally. trigger_BloodCrushRootEffect_helper setups the most of the
    // things for BloodCrushRootEffect.

    // param1 of trigger_BloodCrushRootEffect is never used
    uint8_t *r = reinterpret_cast<uint8_t *(*)(uintptr_t, uintptr_t, uintptr_t)>
      (trigger_BloodCrushRootEffect_func) (0, param1, param2);

    // In sigma2, some enemies such as Gray Ninja, Black Spider Ninja, Vangelf, etc.
    // do not have OPTscat NodeObj in TMC, so trigger_BloodCrushRootEffect_helper
    // cannot continue correctly (data OPTscat is important since even if we circumvent
    // the guard flow in trigger_bloodCrushRootEffect_helper by NOPing or something,
    // we cannot get the good result or the game crushes). In that case, unfortunately,
    // we are going to fallback to the original trigger_VanishCrushRootEffect until
    // somehow we can implement the OPTscat injection.

    if (!r[1])
      return trigger_VanishCrushRootEffect_hooker->get_trampoline () (param1, param2);

    return r;
  }

  // This is called from trigger_BloodCrushRootEffect_helper.
  // This returns the OPTscatN NodeObj indices in the TMC data.
  // The target (enemy's) TMC data is obtained from the first param.
  // We have reimplemented this function because the original function has a bug.
  int
  get_OPTscat_indices (struct model &mdl, uint32_t *out_indices)
  {
    uintptr_t model_tmc_relation_offset_list = start_of_data + 0x69af30;

    uintptr_t mt_rel_ofs = reinterpret_cast<uintptr_t *>
      (model_tmc_relation_offset_list)[mdl.info_idx];

    auto &r = **reinterpret_cast<model_tmc_relation **>
      (*mdl.p_state + mt_rel_ofs);

    uint32_t *tmc_ofs_tbl = reinterpret_cast<uint32_t *>
      (r.tmc1 + *reinterpret_cast<uintptr_t *>(r.tmc1 + 0x20));

    struct nodeobj_type_table_info {
      uint16_t idx0;
      uint16_t size;
    };

    uint32_t tti_idx = *reinterpret_cast<uint32_t *>(r.tmc1+0x40);
    // OptscatN is type 5 NodeObj.
    auto &ti = reinterpret_cast<struct nodeobj_type_table_info *>
      (r.tmc1 + tmc_ofs_tbl[tti_idx])[5];

    // offset table of type table is next to the info list
    uint32_t *tt_ofs = reinterpret_cast<uint32_t *>
      (r.tmc1 + tmc_ofs_tbl[tti_idx + 1]);

    // They expect the size of out_indices is at most 16.
    int n = 0;
    for (int i = 0; n < 0x10 && i < ti.size; i++)
      {
	uint32_t j = ti.idx0 + i;

	// The address calculation below seems odd, though, it is what it is.
	// Idk why the MSBit of *base is high (maybe MSByte is for some flag things)?
	uint32_t *type = reinterpret_cast<uint32_t *>
	  (reinterpret_cast<uintptr_t>(&tt_ofs[j]) + (tt_ofs[j] & 0x0fffffff));

	// type[0] is major type, type[1] is sub type, and type[2] is some sequential id (if exists).
	// subtype of OptscatN is 3.
	if (type[1] == 3)
	  {
	    out_indices[n++] = j;
	  }
      }

    return n;
  }
}

namespace ngs2::nm::plugin::effect::gore::crush {
  void
  init ()
  {
    uintptr_t trigger_VanishCrushRootEffect_func;
    uintptr_t get_OPTscat_indices_func;
    uintptr_t alloc_EFF_CommonIzunaBloodExp_func;
    uintptr_t alloc_EFF_CommonSuicideBloodExp_func;

    switch (binary_kind)
      {
      case NGS2_BINARY_KIND::STEAM_JP:
	trigger_VanishCrushRootEffect_func = base_of_image + 0x14608f0;
	trigger_BloodCrushRootEffect_func = base_of_image + 0xc0fcb0;
	get_OPTscat_indices_func = base_of_image + 0x144c8e0;
	alloc_EFF_CommonIzunaBloodExp_func = base_of_image + 0x10206f0;
	alloc_EFF_CommonSuicideBloodExp_func = base_of_image + 0x10200c0;
	break;
      case NGS2_BINARY_KIND::STEAM_AE:
	trigger_VanishCrushRootEffect_func = base_of_image + 0x1460d20;
	trigger_BloodCrushRootEffect_func = base_of_image + 0x0c0fc40;
	get_OPTscat_indices_func = base_of_image + 0x144cb00;
	alloc_EFF_CommonIzunaBloodExp_func = base_of_image + 0x1020990;
	alloc_EFF_CommonSuicideBloodExp_func = base_of_image + 0x1020360;
	break;
      }

    trigger_VanishCrushRootEffect_hooker =
      new InlineHooker<decltype(&trigger_VanishCrushRootEffect)> (trigger_VanishCrushRootEffect_func,
								  trigger_VanishCrushRootEffect);

    get_OPTscat_indices_hooker =
      new InlineHooker<decltype(&get_OPTscat_indices)> (get_OPTscat_indices_func,
							get_OPTscat_indices);

    // Credit: enhuhu
    // These disable EFF_CommonIzunaBloodExp and EFF_CommnSuicideBloodExp,
    // which cause dim purple steam.
    uint8_t ret = 0xc3;
    WriteMemory (alloc_EFF_CommonIzunaBloodExp_func, ret);
    WriteMemory (alloc_EFF_CommonSuicideBloodExp_func, ret);
  }
}

/*
 * NGS2 NM Gore Plugin by Nozomi Miyamori is marked with CC0 1.0
 * This module restores NG2 misc gore effect.
 */
#include "util.hpp"

namespace nm_effect::gore::misc {
  const uintptr_t make_hit_effect_func = VA (0x1047790);
  const uintptr_t ryu_bow_attack_type_id = VA (0x17f25c8);
  const uintptr_t momiji_bow_attack_type_id = VA (0x17f0428);
  const uintptr_t rachel_gun_attack_type_id = VA (0x17f3ca8);
  const uintptr_t trigger_delimb_effect_func = VA (0x1457df0);
  const uintptr_t object_fade_out_start_time = VA (0x1907cf4);
}

namespace nm_effect::gore::misc {
  void
  init ()
  {
    {
      // Credits: Fiend Busa
      // This is the time how long the delimbed limbs remain.
      // They set 1 by default, which means limbs disappear immediately.
      // Instead, we set very very long time limit.
      uint32_t time_limit = 0x7fffffff;
      WriteMemory (trigger_delimb_effect_func + 0x2a9 + 1, &time_limit, sizeof(time_limit));
    }

    {
      // Credits: Fiend Busa
      // This is the hitstop (micro freeeze) intensity when you trigger delimb.
      // The higher the value, the longer it freezes.
      // They set 3 by default.
      // const uintptr_t delimb_hit_stop_intensity = VA (0x212160c);
      uint32_t hs_intens = 0;
      WriteMemory (trigger_delimb_effect_func + 0x17d + 6, &hs_intens, sizeof(hs_intens));
    }

    {
      // Credits: Fiend Busa
      // Corpses will not fade out.
      const float time = numeric_limits<float>::infinity ();
      WriteMemory (object_fade_out_start_time, &time, sizeof(time));
    }

    {
      // This circumvents the blocking to trigger the EFF_ArrowHitBlood.
      // We have chosen `and' over `xor' becase it has the same size of codes.
      const uint8_t and_eax_0[] = { 0x83, 0xe0, 0x00 };
      WriteMemory (make_hit_effect_func + 0x10f8, and_eax_0, sizeof(and_eax_0));

      // To make EFF_ArrowHitBlood work.
      const uint16_t attack_type_id = 0x1002;
      WriteMemory (ryu_bow_attack_type_id, &attack_type_id, sizeof(attack_type_id));
      WriteMemory (momiji_bow_attack_type_id, &attack_type_id, sizeof(attack_type_id));
      WriteMemory (rachel_gun_attack_type_id, &attack_type_id, sizeof(attack_type_id));
    }
  }

  void
  deinit ()
  {
    detours.clear ();
  }
}

namespace {
  // const uintptr_t number_of_delimbed_limbs_list = VA (0x21b3000);
  // const uintptr_t limb_appearance_timer_list = VA (0x6426880);
  // we don't use this struct for now, but keep this for reference.
  struct limb_appearance_timer {
    uint32_t data0x0;
    uint32_t data0x4;
    uint32_t data0x8;
    uint32_t data0xc;

    uintptr_t data0x10;
    uint32_t data0x18;
    uint32_t data0x1c;

    uintptr_t data0x20;
    uint64_t data0x28;

    uint32_t data0x30;
    uint32_t data0x34;
    uint32_t data0x38;
    uint32_t data0x3c;

    uint32_t data0x40;
    uint32_t data0x44;
    uint32_t data0x48;
    uint32_t data0x4c;

    uint32_t data0x50;
    uint32_t data0x54;
    uint32_t data0x58;
    uint32_t data0x5c;

    uint32_t elapsed_time;
    uint32_t time_limit;
    uint32_t data0x68;
    uint32_t data0x6c;
  };
}

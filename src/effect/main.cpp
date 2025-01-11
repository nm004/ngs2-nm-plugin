/*
 * NINJA GAIDEN Master Collection NM Plugin by Nozomi Miyamori
 * is marked with CC0 1.0. This file is a part of NINJA GAIDEN
 * Master Collection NM Plugin.
 */

#if defined(_MSVC_LANG)
# define DLLEXPORT __declspec (dllexport)
# define WIN32_LEAN_AND_MEAN
#else
# define DLLEXPORT
#endif

#include "dismember.hpp"
#include "util.hpp"

#include <utility>
#include <queue>
#include <set>
#include <span>

using namespace std;
using namespace nm;

namespace
{
uintptr_t (*object_data_offset_table)[4];

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

struct corpse_object_data
{
  uint8_t data0x0;
  alignas (8) uintptr_t ptr0x8;
  uintptr_t ptr0x10;
};

struct corpse_object_data_table
{
  uint8_t data0x0;
  alignas (8) struct corpse_object_data corpse_obj_data[0x100];
  uintptr_t tmc_object_ptr;
  uint32_t data0x1810;
  float timer0;
  float timer1;
  uint32_t data0x181c;
  uint32_t data0x1820;
  uint32_t padding0x1824;
};

struct corpse_object_data_table *corpse_object_data_table_ptr;

constexpr auto NOP4 = make_bytes (0x0f, 0x1f, 0x40, 0x00);
constexpr auto NOP5 = make_bytes (0x0f, 0x1f, 0x44, 0x00, 0x00);
constexpr auto NOP7 = make_bytes (0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00);
constexpr auto NOP8 = make_bytes (0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00);
constexpr auto NOP9 = make_bytes (0x66, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00);

namespace hit
{
  Patch<Bytes <3>> *patch1;
  Patch<uint16_t> *patch2;
  Patch<uint16_t> *patch3;
  Patch<uint16_t> *patch4;
}

namespace bloodparticle
{
  Patch<int32_t> *patch1;
  Patch<int32_t> *patch2;
  Patch<uint8_t> *patch3;
  Patch<uint8_t> *patch4;
}

namespace bloodstamp
{
  struct BloodStampEffectManager;

  struct BloodHitStampEffect
  {
    BloodHitStampEffect *next;
    uint64_t data0x8;
    uintptr_t *ptr0x10;
    uint64_t data0x18;
    uintptr_t *vft0x20;
    uint32_t fadeout_duration;
  };

  struct managed_effect_data
  {
    uint64_t data0x0;
    BloodHitStampEffect *effect_data_ptr;
  };

  struct SphericalResourceGenerator
  {
    uintptr_t *vft0x0;
    uint8_t data0x8[0x168];
    struct managed_effect_data *managed_effect_data;
  };

  void BloodStampEffectManager_vfunction2 (BloodStampEffectManager *thisptr);
  void SphericalResourceGenerator_vfunction2 (SphericalResourceGenerator *, float *param_1);
  set<BloodHitStampEffect *> bloodstamp_effects_set;
  queue<BloodHitStampEffect *> bloodstamp_effects_queue;

  Patch<int32_t> *patch1;
  Patch<int32_t> *patch2;
  Patch<int32_t> *patch3;
  Patch<int32_t> *patch4;
  Patch<int32_t> *patch5;
  Patch<int32_t> *patch6;
  Patch<Bytes <8>> *patch7;
  Patch<Bytes <8>> *patch8;

  VFPHook<decltype (BloodStampEffectManager_vfunction2)> *BloodStampEffectManager_vfunction2_hook;
  VFPHook<decltype (SphericalResourceGenerator_vfunction2)> *SphericalResourceGenerator_vfunction2_hook;
}

namespace dismember
{
  int get_OPTscat_indices (struct game_object &, uint32_t *);

  SimpleInlineHook<decltype (update_SUP_NodeObj_visibility<0>)> *update_SUP_NodeObj_visibility_hook;
  SimpleInlineHook<decltype (get_OPTscat_indices)> *get_OPTscat_indices_hook;
  Patch<int32_t> *patch1;
  Patch<int32_t> *patch2;
  Patch<uint32_t [4]> *patch3;
  Patch<int32_t> *patch4;
  Patch<Bytes <35>> *patch5;
  Patch<uint8_t> *patch6;
  Patch<Bytes <10>> *patch7;
  Patch<Bytes <6>> *patch8;
  Patch<Bytes <6>> *patch9;

  namespace jp
  {
    Patch<Bytes <23>> *patch1;
    Patch<Bytes <14>> *patch2;
    Patch<Bytes <14>> *patch3;
    Patch<Bytes <27>> *patch4;
    Patch<Bytes <25>> *patch5;
    Patch<Bytes <19>> *patch6;
    Patch<Bytes <24>> *patch7;
    Patch<Bytes <28>> *patch8;
  }
}

namespace afterimage
{
  struct afterimage_param
  {
    // Character Id?
    uint32_t unknown0x0;

    alignas(0x10) float distorted_image_colors_rgb[3];
    float distorted_image_glow_power;
    float deep_image_colors_rgb[3];
    float deep_image_glow_power;

    float unknown0x10;
    uint32_t update_interval;
    int32_t duration;
  };

  template <uintptr_t afterimage_params_rva>
  void update_afterimage_params ();

  Patch<uint8_t> *patch1;
}

namespace rigidbody
{
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

  struct lump_data
  {
    struct game_object *obj;
    uint32_t data0x8;
    uint32_t data0xc;
    uint32_t data0x10;
    int32_t data0x14;
    uint32_t data0x18;
    uint32_t data0x1c;
    float data0x20;
    float data0x24;
  };

  struct lump_manager
  {
    game_object *obj;
    uint32_t data0x8;
    uint32_t padding0xc;
    node_layer *node_lay;
    uintptr_t ptr0x18;
    struct lump_data data0[0x10];
    uint32_t data0x2a0;
    uint32_t padding0x2a4;
    alignas (8) struct lump_data data1[0x20];
    uint32_t data0x7a8;
    uint32_t padding0x7ac;
  };

  void FUN_13ffb50 (uintptr_t param_1, uintptr_t param_2);
  void FUN_1444ab0 ();

  float *get_pose_matrix (float *, struct node_layer &, game_object &);
  float *(*get_lump_pose_matrix) (lump_data &, float *, game_object &);

  SimpleInlineHook <decltype (FUN_13ffb50)> *FUN_13ffb50_hook;
  SimpleInlineHook <decltype (FUN_1444ab0)> *FUN_1444ab0_hook;

  queue <struct corpse_object_data_table *> delete_corpse_queue;

  lump_manager *lump_manager_ptr;

  Patch<Bytes <8>> *patch1;
  Patch<uint8_t> *patch2;
  Patch<Bytes <5>> *patch3;
  SimpleInlineHook <decltype (get_pose_matrix)> *get_pose_matrix_hook;

}

namespace combat
{
  // TODO?: Separate these to the other module.

  // table_n[x] <=> Ninja Dog? (not used), Acolyte, Warrior, Mentor, Master Ninja.
  // table_n[x][y] <=> Enemy HP Multiplier, Enemy Damage Multiplier, Enemy
  // Critical Damage Multiplier (e.g., striking a player on a wall).
  float enemy_hp_damage_multiplier_table[5][3] = {
	{.618, .786, .887},
	{.786, .887, .942},
	{.887, .942, 1.},
	{.942, 1., 1.272},
	{1., 1.272, 1.618}
  };

  Patch<int32_t> *patch1;
  Patch<Bytes <5>> *patch2;
  Patch<Bytes <2>> *patch3;
  Patch<array<uintptr_t, 35>> *patch4;

  template <uintptr_t>
  void update_enemy_hp_damage_multiplier_table ();
}

// It returns the OPTscatN NodeObj indices in the TMC data. The target enemy's TMC
// data is obtained from the first param. We have reimplemented this function because
// the original function has a bug, which return immedeately search first 0x1f nodes.
int
dismember::get_OPTscat_indices (struct game_object &obj, uint32_t *indices_out)
{
  using namespace std;
  using namespace nm;

  struct object_data
  {
    uint8_t data0x0[8];
    uintptr_t tmc;
  };

  uintptr_t tmc = reinterpret_cast <struct object_data **> (obj.data_ptr0x0 + object_data_offset_table[65][obj.data0x1a])[0]->tmc;

  uint32_t *offset_table = reinterpret_cast<uint32_t *> (tmc + *reinterpret_cast<uintptr_t *>(tmc + 0x20));
  uint32_t start_of_optional_data = *reinterpret_cast<uint32_t *>(tmc + 0x40);

  // Optscat's type is 5.
  // first = num of nodes, second = first index of the nodes.
  auto &header_of_opt = reinterpret_cast<pair<uint16_t, uint16_t> *> (tmc + offset_table[start_of_optional_data])[5];
  uint32_t *obj_type_info = reinterpret_cast<uint32_t *> (tmc + offset_table[start_of_optional_data + 1]);

  // We expect the maximum size of indices_out is 16.
  int n = 0;
  for (int i = 0; n < 16 && i < header_of_opt.second; i++)
    {
      int j = header_of_opt.first + i;

      // The address calculation below looks odd, though, it is what it is.
      uint32_t &x = obj_type_info[j];
      uint32_t *t = reinterpret_cast<uint32_t *> (reinterpret_cast<uintptr_t> (&x) + (x & 0x0fffffff));

      // t[0] is major type, t[1] is sub type, and t[2] is sequential id (if exists).
      // sub type of OptscatN is 3.
      // We skip OPTscat08 since it's small and we prefer OPTscat16 which is larger.
      if (t[1] == 3 && t[2] != 9)
	{
	  indices_out[n++] = j;
	}
    }

  return n;
}

// Reset all managed bloodstamp resources.
void
bloodstamp::BloodStampEffectManager_vfunction2 (BloodStampEffectManager *thisptr)
{
  bloodstamp_effects_set.clear ();
  bloodstamp_effects_queue = {};
  BloodStampEffectManager_vfunction2_hook->call (thisptr);
}

// This function set the fadeout_duration of bloodstamp to release it. We need this because some
// effects will disappear or will not appear at all when there are too many bloodstamps.
void
bloodstamp::SphericalResourceGenerator_vfunction2 (SphericalResourceGenerator *thisptr, float *param_1)
{
  auto e0 = thisptr->managed_effect_data->effect_data_ptr;

  SphericalResourceGenerator_vfunction2_hook->call (thisptr, param_1);

  auto e = thisptr->managed_effect_data->effect_data_ptr;
  if (e == e0)
    return;

  if (e->fadeout_duration != -1)
    return;

  // We need a set to check if a bloodstamp was already managed. This function may be called
  // with the same arguments.
  if (!bloodstamp_effects_set.emplace (e).second)
    return;

  bloodstamp_effects_queue.emplace (e);
  if (bloodstamp_effects_queue.size () < 1024)
    return;

  e = bloodstamp_effects_queue.front ();
  bloodstamp_effects_queue.pop ();
  bloodstamp_effects_set.erase (e);

  while (e)
    {
      // Effects are released in 'duration' time.
      // 60 frames == 1 sec.
      e->fadeout_duration = 60;
      e = e->next;
    }
}

template <uintptr_t afterimage_params_rva>
void afterimage::update_afterimage_params ()
{
  auto &params = *reinterpret_cast <afterimage_param (*)[18]> (base_of_image + afterimage_params_rva);

  // TODO?: Make these configurable for players.
  // p[0] == Ryu
  // p[1] == Ayane
  // p[2] == Momiji
  // p[3] == Reichel
  // p[i > 3] == Enemies
  for (auto &p : params)
    {
      p.distorted_image_colors_rgb[0] *= 3;
      p.distorted_image_colors_rgb[1] *= 3;
      p.distorted_image_colors_rgb[2] *= 3;
      p.deep_image_colors_rgb[0] *= 4;
      p.deep_image_colors_rgb[1] *= 4;
      p.deep_image_colors_rgb[2] *= 4;
      p.duration /= 2;
      p.update_interval = 1;
    }
}

float *rigidbody::get_pose_matrix (float *out, struct node_layer &n, game_object &obj)
{
  uint32_t node_index = *reinterpret_cast <uint32_t *> (n.nodeobj + 0x34);
  for (auto &i : lump_manager_ptr[obj.obj_group_index].data0)
    if (i.data0xc == node_index)
      return get_lump_pose_matrix (i, out, obj);
  return get_pose_matrix_hook->call (out, n, obj);
}

void
rigidbody::FUN_1444ab0 ()
{
  FUN_1444ab0_hook->call ();
  delete_corpse_queue = {};
}

void
rigidbody::FUN_13ffb50 (uintptr_t param_1, uintptr_t param_2)
{
  FUN_13ffb50_hook->call (param_1, param_2);

  span <struct corpse_object_data_table> T {corpse_object_data_table_ptr, 40};
  for (auto &t : T)
    {
      if (t.tmc_object_ptr == param_2)
	{
	  t.timer1 = (1 << 24) - 1;
	  delete_corpse_queue.push (&t);
	  break;
	}
    }

  // We release 2 elements from the queue since the game register an enemy and its item separately.
  // Objects get released when timer0 reaches timer1.
  if (delete_corpse_queue.size () == size(T))
    {
      delete_corpse_queue.front ()->timer0 = (1 << 24);
      delete_corpse_queue.pop ();
      delete_corpse_queue.front ()->timer0 = (1 << 24);
      delete_corpse_queue.pop ();
    }
}

void
init ()
{
  switch (image_id)
  {
  case ImageId::NGS2SteamAE:
    object_data_offset_table = reinterpret_cast <uintptr_t (*)[4]> (base_of_image + 0x1e38710);
    {
      using namespace hit;
      // This circumvents the block for triggering the EFF_ArrowHitBlood
      // which makes blood particles when hitting enemies with an arrow.
      // We have chosen `and' over `xor' becase it has the same size of codes.
      // and eax, 0
      patch1 = new Patch {0x1048888, make_bytes (0x83, 0xe0, 0x00)};

      // These make Ryu's bow, Momiji's bow and Rachel's gatling gun attack category
      // 0x1002 which is needed to make EFF_ArrowHitBlood work.
      patch2 = new Patch {0x17f25c8, uint16_t {0x1002}};
      patch3 = new Patch {0x17f0428, uint16_t {0x1002}};
      patch4 = new Patch {0x17f3ca8, uint16_t {0x1002}};
    }

    {
      using namespace bloodparticle;
      // These invoke large blood particles effect (each for humans and fiends).
      patch1 = new Patch {0x120a13f + 1, 0x075f720 - (0x120a13f + 5)};
      patch2 = new Patch {0x120a0f0 + 1, 0x075fd20 - (0x120a0f0 + 5)};

      // Blood particles for obliteration (the value is passed to FUN_141205a40).
      // 2 == small (default), 3 == large but faint (it is used when in water),
      // 4 == large, 5 == large and last longer (meybe not used in NG2, but in NGS1).
      patch3 = new Patch {0x12058c9 + 3, uint8_t {0x4u}};
      *reinterpret_cast <uintptr_t *> (base_of_image + 0x1c8e218) = base_of_image + 0x12058b0;
      patch4 = new Patch {0x1205899 + 3, uint8_t {0x5u}};
      *reinterpret_cast <uintptr_t *> (base_of_image + 0x1c8e1f8) = base_of_image + 0x1205880;
    }

    {
      using namespace bloodstamp;
      // These make NG2 red blood stains on the ground. We change the calling functions to the function of oil stains,
      // because it can produce NG2 like blood stains (need to modify game data "04413.dat").

      // For BloodStampEffect.
      patch1 = new Patch {0x120cc1d + 1, 0x077b010 - (0x120cc1d + 5)};
      patch2 = new Patch {0x120cbe1 + 1, 0x077b010 - (0x120cbe1 + 5)};
      // For BloodSpreadStampEffect.
      patch3 = new Patch {0x120c82d + 1, 0x077b010 - (0x120c82d + 5)};
      patch4 = new Patch {0x120c7f1 + 1, 0x077b010 - (0x120c7f1 + 5)};
      // For ?.
      patch5 = new Patch {0x120daa5 + 1, 0x077ce10 - (0x120daa5 + 5)};
      patch6 = new Patch {0x120da69 + 1, 0x077ce10 - (0x120da69 + 5)};

      // These make bloodstains last forever.
      patch7 = new Patch {0x120cafe, concat (NOP7, uint8_t {0xeb})};
      patch8 = new Patch {0x120c70e, concat (NOP7, uint8_t {0xeb})};

      BloodStampEffectManager_vfunction2_hook = new VFPHook {0x188a890, BloodStampEffectManager_vfunction2};
      SphericalResourceGenerator_vfunction2_hook = new VFPHook {0x18c5578, SphericalResourceGenerator_vfunction2};
    }

    {
      using namespace dismember;

      update_SUP_NodeObj_visibility_hook = new SimpleInlineHook {0x1455880, update_SUP_NodeObj_visibility <0x1e38710>};
      get_OPTscat_indices_hook = new SimpleInlineHook {0x144cb00, get_OPTscat_indices};

      // We directly detour invoke_VanishMutilationRootEffect function to invoke_BloodMutilationRootEffect function.
      // Both functions parameters look the same.
      patch1 = new Patch {0x145866a + 1, 0x1220060 - (0x145866a + 5)};

      // We detour alloc_BloodMutilationRootEffect to alloc_BloodMutilationRootEffect_wrapper, which calls
      // alloc_BloodMutilationRootEffect internally.
      patch2 = new Patch {0x12201f6 + 1, 0x1206ec0 - (0x12201f6 + 5)};
      // We also modify the internal jump table to make larger blood jet always.
      patch3 = new Patch {0x1207430, *reinterpret_cast <uint32_t (*)[4]> (base_of_image + 0x1207430 + 4 * 0x10)};

      // We directly detour invoke_VanishViscosityEffect function to invoke_MutilationViscosityBloodEffect function.
      // Both functions parameters look the same.
      patch4 = new Patch {0x1458761 + 1, 0x121fe20 - (0x1458761 + 5)};

      // We make invoke_VanishCrushRootEffect_or_BloodCrushRootEffect function always
      // invoke BloodCrushRootEffect.
      patch5 = new Patch {0x1460d34, concat (NOP9, NOP9, NOP9, NOP7, uint8_t {0xeb})};

      // This circumvents the block of transition to the dead state of an
      // enemy.
      patch6 = new Patch {0xf8758f, uint8_t {0xeb}};

      // Additional blood shed for crushing effect.
      patch7 = new Patch {0x144c0d0, concat (NOP5, NOP5)};

      // Credits: Fiend Busa
      // Limbs stay until the timer reach its limit. The limit is 1 by default,
      // We just disable the timer.
      // xor eax eax; nop4
      patch8 = new Patch {0x145a338, concat (make_bytes (0x31, 0xc0), NOP4)};

      // We disable the lump appearance timer.
      patch9 = new Patch {0x144ae80, concat (make_bytes (0x31, 0xc0), NOP4)};

      // Credit: enhuhu
      // This replaces EFF_CommonIzunaBloodExp which produces dim purple effect with
      // EFF_CommonWazaIzuna.
      *reinterpret_cast <uintptr_t *> (base_of_image + 0x1c74980) = base_of_image + 0x100a990;

      // This replaces EFF_CommonSuicideBloodExp which produces dim purple effect with
      // EFF_CommonWazaIzuna.
      *reinterpret_cast <uintptr_t *> (base_of_image + 0x1c74900) = base_of_image + 0x100a990;
    }

    {
      using namespace afterimage;

      afterimage::update_afterimage_params <0x1e39b40> ();
      // We rewrite update interval check (to make it work when the interval is 1).
      patch1 = new Patch {0x1053d20 + 2, uint8_t {0}};
    }

    {
      using namespace rigidbody;

      // Fix corpse data initialization.
      // xorps xmm0, xmm0; xorps xmm1, xmm1; nop2;
      patch1 = new Patch {0x145e67a, make_bytes(0x0f, 0x57, 0xc0, 0x0f, 0x57, 0xc9, 0x66, 0x90)};

      corpse_object_data_table_ptr = reinterpret_cast <decltype (corpse_object_data_table_ptr)> (base_of_image + 0x6439cc0);
      FUN_13ffb50_hook = new SimpleInlineHook {0x13ffb50, FUN_13ffb50};
      FUN_1444ab0_hook= new SimpleInlineHook {0x1444ab0, FUN_1444ab0};

      // Disable corpse deleting timer update.
      patch2 = new Patch {0x145f4dc, uint8_t {0xeb}};

      // Fix a wrong rigidbody initialization.
      patch3 = new Patch {0x1473a37, NOP5};

      lump_manager_ptr = reinterpret_cast <lump_manager *> (base_of_image + 0x642a6b0);
      get_lump_pose_matrix = reinterpret_cast <decltype (get_lump_pose_matrix)> (base_of_image + 0x144a330);
      get_pose_matrix_hook = new SimpleInlineHook {0xc0fdc0, get_pose_matrix};
    }

    {
      using namespace combat;

      // Credits: Fiend Busa
      // This is the hitstop (micro freeeze) intensity when Ryu dismembers an enemy's limb.
      // The higher the value, the longer it freezes. They set 3 by default.
      patch1 = new Patch {0x1457f6d + 6, 0};

      // These make the enemy dead state detection delayed.
      // jmp 0xa0
      patch2 = new Patch {0x0f29437, make_bytes (0xe9, 0xa0, 0x00, 0x00, 0x00)};
      // xor eax, eax
      patch3 = new Patch {0x0f294dc, make_bytes (0x31, 0xc0)};

      //update_enemy_hp_damage_multiplier_table <0x1d9f370> ();

      // Let's make pointers to a table point to our table.
      array<uintptr_t, 35> arr;
      fill (arr.begin (), arr.end (), reinterpret_cast <uintptr_t> (&enemy_hp_damage_multiplier_table));
      patch4 = new Patch {0x1829080, arr};
    }
    break;
  case ImageId::NGS2SteamJP:
    object_data_offset_table = reinterpret_cast <uintptr_t (*)[4]> (base_of_image + 0x1e37710);
    {
      using namespace hit;
      patch1 = new Patch {0x10485e8, make_bytes (0x83, 0xe0, 0x00)};
      patch2 = new Patch {0x17f15c8, uint16_t {0x1002}};
      patch3 = new Patch {0x17ef428, uint16_t {0x1002}};
      patch4 = new Patch {0x17f2ca8, uint16_t {0x1002}};
    }
    {
      using namespace bloodparticle;
      patch1 = new Patch {0x1209e9f + 1, 0x075f760 - (0x1209e9f + 5)};
      patch2 = new Patch {0x1209e50 + 1, 0x075ff60 - (0x1209e50 + 5)};
      patch3 = new Patch {0x1205629 + 3, uint8_t {0x4u}};
      *reinterpret_cast <uintptr_t *> (base_of_image + 0x1c8d218) = base_of_image + 0x1205610;
      patch4 = new Patch {0x12055f9 + 3, uint8_t {0x5u}};
      *reinterpret_cast <uintptr_t *> (base_of_image + 0x1c8d2d8) = base_of_image + 0x12055e0;
    }
    {
      using namespace bloodstamp;
      patch1 = new Patch {0x120c97d + 1, 0x077b050 - (0x120c97d + 5)};
      patch2 = new Patch {0x120c941 + 1, 0x077b050 - (0x120c941 + 5)};
      patch3 = new Patch {0x120c58d + 1, 0x077b050 - (0x120c58d + 5)};
      patch4 = new Patch {0x120c551 + 1, 0x077b050 - (0x120c551 + 5)};
      patch5 = new Patch {0x120d805 + 1, 0x077ce50 - (0x120d805 + 5)};
      patch6 = new Patch {0x120d7c9 + 1, 0x077ce50 - (0x120d7c9 + 5)};
      patch7 = new Patch {0x120c85e, concat (NOP7, uint8_t {0xeb})};
      patch8 = new Patch {0x120c46e, concat (NOP7, uint8_t {0xeb})};
      BloodStampEffectManager_vfunction2_hook = new VFPHook {0x1889890, BloodStampEffectManager_vfunction2};
      SphericalResourceGenerator_vfunction2_hook = new VFPHook {0x18c4578, SphericalResourceGenerator_vfunction2};
    }
    {
      using namespace dismember;
      update_SUP_NodeObj_visibility_hook = new SimpleInlineHook {0x14556a0, update_SUP_NodeObj_visibility <0x1e37710>};
      get_OPTscat_indices_hook = new SimpleInlineHook {0x144c8e0, get_OPTscat_indices};
      patch1 = new Patch {0x14584f5 + 1, 0x121fde0 - (0x14584f5 + 5)};
      patch2 = new Patch {0x121ff76 + 1, 0x1206c20 - (0x121ff76 + 5)};
      patch3 = new Patch {0x1207190, *reinterpret_cast <uint32_t (*)[4]> (base_of_image + 0x1207190 + 4 * 0x10)};
      patch4 = new Patch {0x14585f8 + 1, 0x121fba0 - (0x14585f8 + 5)};
      patch5 = new Patch {0x1460904, concat (NOP9, NOP9, NOP9, NOP7, uint8_t {0xeb})};
      patch6 = new Patch {0x0f8738f, uint8_t {0xeb}};
      patch7 = new Patch {0x144beb0, concat (NOP5, NOP5)};
      patch8 = new Patch {0x1459e0d , concat (make_bytes (0x31, 0xc0), NOP4)};
      patch9 = new Patch {0x144ac60, concat (make_bytes (0x31, 0xc0), NOP4)};
      *reinterpret_cast <uintptr_t *> (base_of_image + 0x1c73980) = base_of_image + 0x100a790;
      *reinterpret_cast <uintptr_t *> (base_of_image + 0x1c73900) = base_of_image + 0x100a790;
    }
    {
      using namespace dismember::jp;
      // These are for circumventing the head dismemberment disablers
      // which can be found in the Asia version of the game.
      
      // These make head dismemberment works.
      patch1 = new Patch {0x1456e71, concat (NOP9, NOP9, NOP5)};
      patch2 = new Patch {0x1457ef0, concat (NOP9, NOP5)};
      patch3 = new Patch {0x14583e5, concat (NOP9, NOP5)};

      // In order to prevent the head from getting back to its body.
      constexpr auto SETNZ_DIL = make_bytes (0x40, 0x0f, 0x95, 0xc7);
      patch4 = new Patch {0x145e45d, concat (NOP9, NOP9, NOP5, SETNZ_DIL)};
      patch5 = new Patch {0x145e824, concat (NOP9, NOP9, NOP7)};

      // I could not find any visible differences when disabling these, but
      // the codes look similar to the others.
      patch6 = new Patch {0x0c31190, concat (NOP9, NOP5, NOP5)};
      patch7 = new Patch {0x1456dcc, concat (NOP9, NOP8, NOP7)};
      patch8 = new Patch {0x14cff5e, concat (NOP9, NOP9, NOP5, NOP5)};
    }
    {
      using namespace afterimage;
      afterimage::update_afterimage_params <0x1e38b40> ();
      patch1 = new Patch {0x1053a80 + 2, uint8_t {0}};
    }
    {
      using namespace rigidbody;
      patch1 = new Patch {0x145e20a, make_bytes(0x0f, 0x57, 0xc0, 0x0f, 0x57, 0xc9, 0x66, 0x90)};
      corpse_object_data_table_ptr = reinterpret_cast <decltype (corpse_object_data_table_ptr)> (base_of_image + 0x6438cc0);
      FUN_13ffb50_hook = new SimpleInlineHook {0x13ff920, FUN_13ffb50};
      FUN_1444ab0_hook= new SimpleInlineHook {0x1444890, FUN_1444ab0};
      patch2 = new Patch {0x145f08c, uint8_t {0xeb}};
      patch3 = new Patch {0x1473627, NOP5};
      lump_manager_ptr = reinterpret_cast <lump_manager *> (base_of_image + 0x64296b0);
      get_lump_pose_matrix = reinterpret_cast <decltype (get_lump_pose_matrix)> (base_of_image + 0x144a110);
      get_pose_matrix_hook = new SimpleInlineHook {0x0c0fe30, get_pose_matrix};
    }
    {
      using namespace combat;
      patch1 = new Patch {0x1457dd1 + 6, 0};
      patch2 = new Patch {0x0f29237, make_bytes (0xe9, 0xa0, 0x00, 0x00, 0x00)};
      patch3 = new Patch {0x0f292dc, make_bytes (0x31, 0xc0)};
      array<uintptr_t, 35> arr;
      fill (arr.begin (), arr.end (), reinterpret_cast <uintptr_t> (&enemy_hp_damage_multiplier_table));
      patch4 = new Patch {0x1828080, arr};
    }
    break;
  }
}

} // namespace

extern "C" DLLEXPORT BOOL
DllMain (HINSTANCE hinstDLL,
	 DWORD fdwReason,
	 LPVOID lpvReserved)
{
  switch (fdwReason)
  {
  case DLL_PROCESS_ATTACH:
    init ();
    break;
  case DLL_PROCESS_DETACH:
    break;
  default:
    break;
  }
  return TRUE;
}

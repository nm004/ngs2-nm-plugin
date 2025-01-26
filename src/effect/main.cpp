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
  uint8_t vanish_timer_table_index;
  uint8_t data0x1e;
  uint8_t obj_group_index;
  uint8_t data0x20[0x10];
};

struct obj_type
{
  uint32_t type;
  uint32_t sub_type;
  uint32_t seq_id;
};

struct game_object *game_object_ptr;

struct corpse_object
{
  uint8_t in_use;
  alignas (8) struct
  {
    uint8_t data0x0;
    alignas (8) uintptr_t ptr0x8;
    uintptr_t ptr0x10;
  } corpse_obj_data[0x100];
  uintptr_t tmc_object_ptr;
  uint8_t to_render;
  alignas (4) float timer0;
  float timer1;
  uint32_t game_object_type;
  uint32_t object_id;
  uint32_t padding0x1824;
};

struct game_model
{
  uint16_t data0x0;
  uint16_t data0x2;
  float data0x8;
  float data0xc;
  float data0x10;
  uint32_t data0x14;
  uint8_t data0x18[0x188][0x30];
  float *data0x4998[5];
  //uint64_t padding0x49b8;
  float data0x49c0[3][0x20c];
  int8_t data0x6250[0x4b0];
  int8_t data0x6700;
  int8_t data0x6701;
  int8_t padding0x6702[0xe];
};

struct game_model game_model_data_manager[0xa0];

struct corpse_object *corpse_table_ptr;

uintptr_t (*object_data_offset_table)[4];
uint8_t *in_pause_ptr;
int32_t *vanish_timer_table_ptr;

constexpr auto NOP3 = make_bytes (0x0f, 0x1f, 0x00);
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
  Patch<int32_t> *patch5;
  Patch<Bytes <5>> *patch6;
  Patch<Bytes <2>> *patch7;
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
  int get_OPTscat_indices (struct game_object &, uint32_t (&)[16]);

  SimpleInlineHook<decltype (update_SUP_NodeObj_visibility<0>)> *update_SUP_NodeObj_visibility_hook;
  SimpleInlineHook<decltype (get_OPTscat_indices)> *get_OPTscat_indices_hook;
  Patch<int32_t> *patch1;
  Patch<int32_t> *patch2;
  Patch<uint32_t [4]> *patch3;
  Patch<int32_t> *patch4;
  Patch<Bytes <35>> *patch5;
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
    struct obj_type *obj_type;
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
    // crushed node (part of the body) index?
    uint32_t data0x8;
    uint32_t gib_node_index;
    uint32_t rigidbody_id;
    uint32_t timer;
    uint32_t padding0x18;
    uint32_t state;
    // maybe both of two are never used.
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
    struct lump_data lump_data0[0x10];
    uint32_t total_lump_count;
    uint32_t padding0x2a4;
    alignas (8) struct lump_data data1[0x20];
    uint32_t data0x7a8;
    uint32_t padding0x7ac;
  };

  struct limb_data
  {
    uint32_t data0x0;
    uint32_t data0x4;
    uint32_t data0x8;
    alignas (16) uintptr_t hielay_chunk;
    int32_t rigidbody_id;
    alignas (16) float *matrix_ptr;
    float location_vec[4];
    uint32_t data0x40;
    int32_t data0x44;
    uint64_t data0x48;
    uint64_t data0x50;
    uint64_t data0x58;
    uint32_t data0x60;
    uint32_t data0x64;
    uint64_t data0x68;
  };

  struct character
  {
    uint8_t data0x0[0x210];
    uint8_t dead;
    uint8_t data0x211[0x102f];
  };

  uint32_t (*rigidbody_id_table_ptr)[0x80];
  uint32_t *limb_count_ptr;
  struct limb_data (*limb_manager_ptr)[4];

  struct character *character_ptr;

  void FUN_13ffb50 (uintptr_t param_1, uintptr_t param_2);
  void (*release_corpse)(struct corpse_object &);

  bool is_stationary (int32_t obj_group_idx);
  SimpleInlineHook <decltype (is_stationary)> *is_stationary_hook;

  float *get_pose_matrix (float (&)[16], struct node_layer &, game_object &);
  float *(*get_lump_pose_matrix) (lump_data &, float (&)[16], game_object &);

  SimpleInlineHook <decltype (FUN_13ffb50)> *FUN_13ffb50_hook;

  int corpse_table_next_corpse_index;
  int corpse_table_next_corpse_item_index;

  lump_manager *lump_manager_ptr;

  Patch <Bytes <8>> *patch1;
  Patch <Bytes <18>> *patch2_1;
  Patch <Bytes <1>> *patch2_2;
  Patch <Bytes <10>> *patch3;
  Patch <Bytes <5>> *patch4;
  Patch <Bytes <20>> *patch5;

  bool is_to_be_vanished (game_object &obj);
  SimpleInlineHook <decltype (is_to_be_vanished)> *is_to_be_vanished_hook;

  SimpleInlineHook <decltype (get_pose_matrix)> *get_pose_matrix_hook;

  void init_game_model_data_manager ();
  SimpleInlineHook <decltype (init_game_model_data_manager)> *init_game_model_data_manager_hook;

  Patch <Bytes <5>> *patch6_1;
  Patch <Bytes <5>> *patch6_2;
  Patch <Bytes <5>> *patch6_3;
  Patch <Bytes <5>> *patch6_4;
  Patch <Bytes <5>> *patch6_5;
  Patch <Bytes <5>> *patch6_6;
  Patch <Bytes <5>> *patch6_7;
  Patch <Bytes <5>> *patch6_8;
  Patch <Bytes <5>> *patch6_9;
  Patch <Bytes <5>> *patch6_10;
  Patch <Bytes <5>> *patch6_11;
  Patch <Bytes <5>> *patch6_12;
  Patch <Bytes <5>> *patch6_13;
  Patch <uint32_t> *patch7;
  Patch <int32_t> *patch8;

}

// It returns the OPTscatN NodeObj indices in the TMC data. The target enemy's TMC
// data is obtained from the first param. We have reimplemented this function because
// the original function has a bug, which return immedeately search first 0x1f nodes.
int
dismember::get_OPTscat_indices (struct game_object &obj, uint32_t (&indices_out)[16])
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

  struct X
  {
    uint16_t begin_index;
    uint16_t count;
  };

  // Optscat's type is 5.
  struct X &header_of_opt = reinterpret_cast <struct X *> (tmc + offset_table[start_of_optional_data])[5];
  uint32_t *obj_type_info = reinterpret_cast <uint32_t *> (tmc + offset_table[start_of_optional_data + 1]);

  int n = 0;
  int q = header_of_opt.begin_index;
  int m = header_of_opt.count;
  for (int i = 0; n < size (indices_out) && i < m; i++)
    {
      int j = q + i;

      // The address calculation below looks odd, though, it is what it is.
      uint32_t &x = obj_type_info[j];
      uintptr_t y = reinterpret_cast <uintptr_t> (&x) + (x & 0x0fffffff);
      struct obj_type &o = *reinterpret_cast <struct obj_type *> (y);

      // We skip OPTscat08 since it's small and we prefer OPTscat16 which is larger.
      if (o.sub_type == 3 && o.seq_id != 9)
	indices_out[n++] = j;
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
  if (bloodstamp_effects_queue.size () < 768)
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

float *rigidbody::get_pose_matrix (float (&out)[16], struct node_layer &nl, game_object &obj)
{
  if (!obj.data_ptr0x8)
    {
      out[0] = out[4*1+1] = out[4*2+2] = out[4*3+3] = 1.f;
      return out;
    }

  if (obj.data0x1a == 0)
    {
      uint32_t node_index = *reinterpret_cast <uint32_t *> (nl.nodeobj + 0x34);
      auto &L = lump_manager_ptr[obj.obj_group_index];
      int n = L.total_lump_count - 0x20;
      for (int i = 0; i < n; i++)
      {
	auto &ld = L.lump_data0[i];
	if (ld.obj && ld.gib_node_index == node_index)
	  return get_lump_pose_matrix (ld, out, obj);
      }
    }
  return get_pose_matrix_hook->call (out, nl, obj);
}

bool
rigidbody::is_stationary (int32_t game_object_idx)
{
  game_object &obj = game_object_ptr[game_object_idx];

  int grp = obj.obj_group_index;
  if (character_ptr[grp].dead != 4)
    return 0;

  {
    int n = limb_count_ptr[grp];
    auto &L = limb_manager_ptr[grp];
    for (int i = 0; i < n; i++)
      {
	uintptr_t p = reinterpret_cast <uintptr_t> (L[i].matrix_ptr);
	if (L[i].rigidbody_id != -1 && *reinterpret_cast <uint8_t *> (p-0x310))
	  return 0;
      }
  }

  {
    auto &L = lump_manager_ptr[grp];
    int n = L.total_lump_count - 0x20;
    for (int i = 0; i < n; i++)
      {
	auto &d = L.lump_data0[i];
	if (d.state > 2)
	  continue;

	auto &T = *rigidbody_id_table_ptr;
	for (int j = 0; j < size (T); j++)
	  if (d.rigidbody_id == T[j])
	    return 0;
      }
  }

  {
    int i = obj.vanish_timer_table_index;
    if (vanish_timer_table_ptr[i] > 0)
      return 0;
  }

  return 1;
}

void
rigidbody::FUN_13ffb50 (uintptr_t param_1, uintptr_t param_2)
{
  FUN_13ffb50_hook->call (param_1, param_2);

  // We make corpse_table[0] always empty and the game always writes the corpse
  // object data to corpse_table[0]. Then, we copy the data to corpse_table[i]
  // and keep track of them.
  auto &c0 = corpse_table_ptr[0];
  int i;

  // We want to render more corpses than thier items. Glitched rendering happens
  // if we render too many corpses.
  // 0 < i < 28 for corpses, 28 <= i < 40 for items.
  if (c0.game_object_type != 0)
    {
      i = corpse_table_next_corpse_item_index % 4;
      corpse_table_next_corpse_item_index = i + 1;
      i += 28;
    }
  else
    {
      i = corpse_table_next_corpse_index % (40 - 13);
      corpse_table_next_corpse_index = i + 1;
      i++;
    }
  auto &c = corpse_table_ptr[i];
  release_corpse (c);
  c = c0;
  c0 = {};
  c.to_render = 1;
  return;
}

void
rigidbody::init_game_model_data_manager ()
{
  auto &x = *reinterpret_cast <float (*)[0x140]> (base_of_image + 0x219bff0);
  fill (x, end (x), 1.f);
  *reinterpret_cast <uintptr_t *> (base_of_image + 0x219c4f0) = reinterpret_cast <uintptr_t> (game_model_data_manager);
  struct game_model y {};
  fill (game_model_data_manager, end (game_model_data_manager), y);

  for (auto &i : game_model_data_manager)
    i.data0x6701 = 0xa0;
}

bool
rigidbody::is_to_be_vanished (struct game_object &obj)
{
  switch (obj.obj_id)
  {
  case 0x1f: // quadrupedal bone enemy.
  case 0x33: // turret on the airship.
  case 0x83: // human skull ghostfish.
  case 0xa2: // quadrupedal robot.
  case 0xb2: // fish skull ghostfish.
    return true;
  }
  return false;
}

void
init ()
{
  switch (image_id)
  {
  case ImageId::NGS2SteamAE:
    object_data_offset_table = reinterpret_cast <uintptr_t (*)[4]> (base_of_image + 0x1e38710);
    game_object_ptr = reinterpret_cast <struct game_object *> (base_of_image + 0x3195300);
    in_pause_ptr = reinterpret_cast <uint8_t *> (base_of_image + 0x211e753);
    vanish_timer_table_ptr = reinterpret_cast <int32_t *> (base_of_image + 0x211e990);

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

      // Credits: Fiend Busa
      // This is the hitstop (micro freeeze) intensity when Ryu dismembers an enemy's limb.
      // The higher the value, the longer it freezes. They set 3 by default.
      patch5 = new Patch {0x1457f6d + 6, 0};

      // These make the enemy dead state detection delayed.
      // jmp 0xa0
      patch6 = new Patch {0x0f29437, make_bytes (0xe9, 0xa0, 0x00, 0x00, 0x00)};
      // xor eax, eax
      patch7 = new Patch {0x0f294dc, make_bytes (0x31, 0xc0)};
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

      corpse_table_ptr = reinterpret_cast <struct corpse_object *> (base_of_image + 0x6439cc0);
      FUN_13ffb50_hook = new SimpleInlineHook {0x13ffb50, FUN_13ffb50};

      // This rewrites the reset corpses loop to co-op with our codes.
      // nop2; lea rax, [rbx+1808]; cmp qword [rax], 0; je imm;
      patch2_1 = new Patch {0x145d60a,
	  make_bytes (0x66, 0x90,
		      0x48, 0x8d, 0x83, 0x08, 0x18, 0x00, 0x00,
		      0x48, 0x83, 0x38, 0x00,
		      0x74, 0x11,
		      0x48, 0x8b, 0x00)
      };
      // jne 0x145d60a;
      patch2_2 = new Patch {0x145d635 + 1, concat (uint8_t {0xd5})};

      // This prevents the corpse_table[0] from accidtally being used since we
      // do not use corpse_table[0].
      patch3 = new Patch {0x145d7d8, concat (NOP5, NOP5)};

      // This prevents a corpse timer from updating even when there are many
      // corpses since we manually release corpses by ourselves.
      // xor eax, eax; nop3;
      patch4 = new Patch {0x145f584, concat (make_bytes (0x31, 0xc0), NOP3)};

      release_corpse = reinterpret_cast <decltype (release_corpse)> (base_of_image + 0x145dde0);

      rigidbody_id_table_ptr = reinterpret_cast <uint32_t (*)[0x80]> (base_of_image + 0x64b4850);
      character_ptr = reinterpret_cast <struct character *> (base_of_image + 0x2f78b10);
      limb_count_ptr = reinterpret_cast <uint32_t *> (base_of_image + 0x21b3000);
      limb_manager_ptr = reinterpret_cast <struct limb_data (*)[4]> (base_of_image + 0x6426880);
      lump_manager_ptr = reinterpret_cast <lump_manager *> (base_of_image + 0x642a6b0);
      get_lump_pose_matrix = reinterpret_cast <decltype (get_lump_pose_matrix)> (base_of_image + 0x144a330);
      get_pose_matrix_hook = new SimpleInlineHook {0xc0fdc0, get_pose_matrix};

      is_stationary_hook = new SimpleInlineHook {0x0f87410, is_stationary};

      // These keep fiends and enemies on stairs from vanishing.
      patch5 = new Patch {0xf291de, concat (NOP9, NOP8, NOP3)};
      is_to_be_vanished_hook = new SimpleInlineHook {0x1635e60, is_to_be_vanished};

      init_game_model_data_manager_hook = new SimpleInlineHook {0x141b6d0, init_game_model_data_manager};

      auto NOP_CMP_CL_IMM_JB_IMM = make_bytes (0x90, 0x80, 0xf9, 0xa0, 0x72);
      auto NOP_CMP_DL_IMM_JB_IMM = make_bytes (0x90, 0x80, 0xfa, 0xa0, 0x72);
      auto CMP_R8B_IMM_JB_IMM = make_bytes (0x41, 0x80, 0xf8, 0xa0, 0x72);
      patch6_1 = new Patch {0x0f896d0, NOP_CMP_CL_IMM_JB_IMM};
      patch6_2 = new Patch {0x0f89700, NOP_CMP_CL_IMM_JB_IMM};
      patch6_3 = new Patch {0x1377bb0, NOP_CMP_CL_IMM_JB_IMM};
      //patch6_4 = new Patch {0x1381ce5, NOP_CMP_CL_IMM_JB_IMM};
      patch6_5 = new Patch {0x14075d0, NOP_CMP_CL_IMM_JB_IMM};
      //patch6_6 = new Patch {0x140b805, NOP_CMP_CL_IMM_JB_IMM};
      patch6_7 = new Patch {0x140c8e0, NOP_CMP_CL_IMM_JB_IMM};
      patch6_8 = new Patch {0x141a257, NOP_CMP_DL_IMM_JB_IMM};
      //patch6_9 = new Patch {0x141b0d6, NOP_CMP_CL_IMM_JB_IMM};
      //patch6_10 = new Patch {0x141ba79, NOP_CMP_CL_IMM_JB_IMM};
      patch6_11 = new Patch {0x15f1e08, NOP_CMP_CL_IMM_JB_IMM};
      patch6_12 = new Patch {0x15f2ad2, NOP_CMP_CL_IMM_JB_IMM};
      patch6_13 = new Patch {0x163507a, CMP_R8B_IMM_JB_IMM};

      // Allocate more memory for model manager.
      patch7 = new Patch {0x146794d + 1, 0x800000u};

      // This calls the proper reset corpses function when continuing from death.
      patch8 = new Patch {0x1444f23 + 1, 0x145dde0 - (0x1444f23 + 5)};
    }

    break;
  case ImageId::NGS2SteamJP:
    object_data_offset_table = reinterpret_cast <uintptr_t (*)[4]> (base_of_image + 0x1e37710);
    game_object_ptr = reinterpret_cast <struct game_object *> (base_of_image + 0x3194300);
    in_pause_ptr = reinterpret_cast <uint8_t *> (base_of_image + 0x211e753);

    {
      using namespace hit;
      patch1 = new Patch {0x10485e8, make_bytes (0x83, 0xe0, 0x00)};
      patch2 = new Patch {0x17f15c8, uint16_t {0x1002}};
      patch3 = new Patch {0x17ef428, uint16_t {0x1002}};
      patch4 = new Patch {0x17f2ca8, uint16_t {0x1002}};
      patch5 = new Patch {0x1457dd1 + 6, 0};
      patch6 = new Patch {0x0f29237, make_bytes (0xe9, 0xa0, 0x00, 0x00, 0x00)};
      patch7 = new Patch {0x0f292dc, make_bytes (0x31, 0xc0)};
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
      patch2_1 = new Patch {0x145d19a,
	  make_bytes (0x66, 0x90,
		      0x48, 0x8d, 0x83, 0x08, 0x18, 0x00, 0x00,
		      0x48, 0x83, 0x38, 0x00,
		      0x74, 0x11,
		      0x48, 0x8b, 0x00)
      };
      patch2_2 = new Patch {0x145d1c5 + 1, concat (uint8_t {0xd5})};
      patch3 = new Patch {0x145d368, concat (NOP5, NOP5)};
      patch4 = new Patch {0x145f134, concat (make_bytes (0x31, 0xc0), NOP3)};

      corpse_table_ptr = reinterpret_cast <struct corpse_object *> (base_of_image + 0x6438cc0);
      FUN_13ffb50_hook = new SimpleInlineHook {0x13ff920, FUN_13ffb50};
      lump_manager_ptr = reinterpret_cast <lump_manager *> (base_of_image + 0x64296b0);
      get_lump_pose_matrix = reinterpret_cast <decltype (get_lump_pose_matrix)> (base_of_image + 0x144a110);
      get_pose_matrix_hook = new SimpleInlineHook {0x0c0fe30, get_pose_matrix};

      release_corpse = reinterpret_cast <decltype (release_corpse)> (base_of_image + 0x145d970);

      rigidbody_id_table_ptr = reinterpret_cast <uint32_t (*)[0x80]> (base_of_image + 0x64b3850);
      character_ptr = reinterpret_cast <struct character *> (base_of_image + 0x2f77b10);
      limb_count_ptr = reinterpret_cast <uint32_t *> (base_of_image + 0x21b2000);
      limb_manager_ptr = reinterpret_cast <struct limb_data (*)[4]> (base_of_image + 0x6425880);
      lump_manager_ptr = reinterpret_cast <lump_manager *> (base_of_image + 0x64296b0);

      is_stationary_hook = new SimpleInlineHook {0x0f87210, is_stationary};
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
    DisableThreadLibraryCalls (hinstDLL);
    init ();
    break;
  case DLL_PROCESS_DETACH:
    break;
  default:
    break;
  }
  return TRUE;
}

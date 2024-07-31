/*
 * NGS2 NM Plugin by Nozomi Miyamori is marked with CC0 1.0.
 * This file is a part of NGS2 NM Plugin.
 *
 * This module restores NG2 attack hit effect.
 */

#include "util.hpp"
#include "hit.hpp"

using namespace util;

// Credits: Fiend Busa
// This is for the time how long the dismembered limbs remain.
// They set 1 by default, which means limbs disappear immediately.
// Instead, we set it to very very long time.
namespace {
  namespace detail {
    template <uintptr_t rva>
    auto patch1 = Patch{rva + 1, uint32_t{0x7fffffff}};
  }
  namespace steam_ae {
    auto patch1 = detail::patch1<0x1458099>;
  }
  namespace steam_jp {
    auto patch1 = detail::patch1<0x1457f15>;
  }
}

// Credits: Fiend Busa
// This is the hitstop (micro freeeze) intensity when Ryu dismembers an enemy's limb.
// The higher the value, the longer it freezes. They set 3 by default.
namespace {
  namespace detail {
    template <uintptr_t rva>
    auto patch2 = Patch{rva + 6, uint32_t{0}};
  }
  namespace steam_ae {
    auto patch2 = detail::patch2<0x1457f6d>;
  }
  namespace steam_jp {
    auto patch2 = detail::patch2<0x1457dd1>;
  }
}

// Credits: Fiend Busa
// Bodies will not disappear until the time (the specified number of frames) expires.
namespace {
  namespace detail {
    template <uintptr_t rva>
    auto patch3 = Patch{rva, float{60 * 60 * 10}};
  }
  namespace steam_ae {
    auto patch3 = detail::patch3<0x1907cf4>;
  }
  namespace steam_jp {
    auto patch3 = detail::patch3<0x1906cf4>;
  }
}

// This enables EFF_ArrowHitBlood which makes blood particles when hitting
// enemies with an arrow.
namespace {
  namespace detail {
    // This circumvents the block to trigger the EFF_ArrowHitBlood.
    // We have chosen `and' over `xor' becase it has the same size of codes.
    // and eax, 0
    template <uintptr_t rva>
    auto patch4 = Patch{rva, make_bytes( 0x83, 0xe0, 0x00 )};

    // This makes Ryu's bow, Momiji's bow and Rachel's gatling gun attack category
    // 0x1002 which is for EFF_ArrowHitBlood.
    template <uintptr_t rva>
    auto patch5 = Patch{rva, uint16_t{0x1002}};
  }
  namespace steam_ae {
    auto patch4 = detail::patch4<0x1048888>;
    auto patch5 = detail::patch5<0x17f25c8>;
    auto patch6 = detail::patch5<0x17f0428>;
    auto patch7 = detail::patch5<0x17f3ca8>;
  }
  namespace steam_jp {
    auto patch4 = detail::patch4<0x10485e8>;
    auto patch5 = detail::patch5<0x17f15c8>;
    auto patch6 = detail::patch5<0x17ef428>;
    auto patch7 = detail::patch5<0x17f2ca8>;
  }
}

// This adjusts the param1 of trigger_hit_effect() which makes blood particles larger
// when hitting enemies. The param1 of trigger_hit_effect() is a category
// of attack. 5th byte of Param1 is related to the blood particle. It seems that
// 0x10000 or 0x20000 are only effective, however when you specify 0x20000, some
// attacks produce less blood particles (e.g. Shuriken).
namespace {
  namespace detail {
    // or ecx, 0x10000
    template<uintptr_t rva>
    auto patch8 = Patch{rva-6, make_bytes( 0x81, 0xc9, 0x00, 0x00, 0x01, 0x00 )};

  }
  namespace steam_ae {
    auto patch8 = detail::patch8<0x1047790>;
    auto patch9 = CallOffsetPatch{0x104772e, patch8.rva ()};
    auto patch10 = CallOffsetPatch{0x104775d, patch8.rva ()};
  }
  namespace steam_jp {
    auto patch8 = detail::patch8<0x10474f0>;
    auto patch9 = CallOffsetPatch{0x104748e, patch8.rva ()};
    auto patch10 = CallOffsetPatch{0x10474bd, patch8.rva ()};
  }
}

void
plugin::steam_ae::apply_hit_effect_patch ()
{
  using namespace ::steam_ae;
  patch1.apply ();
  patch2.apply ();
  patch3.apply ();
  patch4.apply ();
  patch5.apply ();
  patch6.apply ();
  patch7.apply ();
  patch8.apply ();
  patch9.apply ();
  patch10.apply ();
}

void
plugin::steam_jp::apply_hit_effect_patch ()
{
  using namespace ::steam_jp;
  patch1.apply ();
  patch2.apply ();
  patch3.apply ();
  patch4.apply ();
  patch5.apply ();
  patch6.apply ();
  patch7.apply ();
  patch8.apply ();
  patch9.apply ();
  patch10.apply ();
}

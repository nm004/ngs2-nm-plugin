/*
 * NGS2 NM Plugin by Nozomi Miyamori is marked with CC0 1.0.
 * This file is a part of NGS2 NM Plugin.
 */

#include "qol.hpp"
#include "util.hpp"
#include <cstdint>

using namespace util;

// This is for the fix of "Return to main menu" hang up bug.
// It sees out of the index at the point.
namespace {
  namespace detail {
    template <uintptr_t rva>
    auto patch1 = Patch{rva + 3, uint8_t{7}};
  }
  namespace steam_ae {
    auto patch1 = detail::patch1<0x13ddfa6>;
  }
  namespace steam_jp {
    auto patch1 = detail::patch1<0x13ddd76>;
  }
}

// This is for the fix of the never terminating game bug.
// It enters dead lock at the point.
namespace {
  namespace detail {
    // jmp 0xde
    template <uintptr_t rva>
    auto patch2 = Patch{rva, make_bytes( 0xe9, 0xde, 0x00, 0x00, 0x00 )};
  }
  namespace steam_ae {
    auto patch2 = detail::patch2<0xC43060>;
  }
  namespace steam_jp {
    auto patch2 = detail::patch2<0x0c42e60>;
  }
}

void
plugin::steam_ae::apply_qol_patch ()
{
  using namespace ::steam_ae;
  patch1.apply ();
  patch2.apply ();
}

void
plugin::steam_jp::apply_qol_patch ()
{
  using namespace ::steam_jp;
  patch1.apply ();
  patch2.apply ();
}

#include "util.hpp"
#include "bloodstamp.hpp"
#include <cstdint>

using namespace util;

namespace {
  constinit const auto NOP5 = make_bytes(
    0x0F, 0x1F, 0x44, 0x00, 0x00
  );
  constinit const auto NOP6 = make_bytes(
    0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00
  );
  namespace steam_ae {
    auto patch1 = CallOffsetPatch{0x120cbe1, 0x077b010};
    auto patch2 = CallOffsetPatch{0x120c7f1, 0x077b010};
    auto patch3 = CallOffsetPatch{0x120da69, 0x077ce10};
    auto patch4 = Patch{0x120cba1, concat(NOP5, NOP6)};
    auto patch5 = Patch{0x120c7b1, concat(NOP5, NOP6)};
    auto patch6 = Patch{0x120da29, concat(NOP5, NOP6)};
  }
  namespace steam_jp {
    auto patch1 = CallOffsetPatch{0x120c941, 0x077b050};
    auto patch2 = CallOffsetPatch{0x120c551, 0x077b050};
    auto patch3 = CallOffsetPatch{0x120d7c9, 0x077ce50};
    auto patch4 = Patch{0x120c901, concat(NOP5, NOP6)};
    auto patch5 = Patch{0x120c511, concat(NOP5, NOP6)};
    auto patch6 = Patch{0x120d789, concat(NOP5, NOP6)};
  }
}

namespace plugin::steam_ae {
  void apply_bloodstamp_patch ()
  {
    using namespace ::steam_ae;
    patch1.apply ();
    patch2.apply ();
    patch3.apply ();
    patch4.apply ();
    patch5.apply ();
    patch6.apply ();
  }
}
namespace plugin::steam_jp {
  void apply_bloodstamp_patch ()
  {
    using namespace ::steam_jp;
    patch1.apply ();
    patch2.apply ();
    patch3.apply ();
    patch4.apply ();
    patch5.apply ();
    patch6.apply ();
  }
}

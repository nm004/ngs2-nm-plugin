#include "util.hpp"
#include "bloodstamp.hpp"
#include <cstdint>

namespace {
  namespace steam_ae {
    auto patch1 = util::CallOffsetPatch{0x120cbe1, 0x077b010};
    auto patch2 = util::CallOffsetPatch{0x120C7F1, 0x077b010};
    auto patch3 = util::CallOffsetPatch{0x120da69, 0x077ce10};
  }
  namespace steam_jp {
  }
}

namespace plugin::steam_ae {
  void apply_bloodstamp_patch ()
  {
    using namespace ::steam_ae;
    patch1.apply ();
    patch2.apply ();
    patch3.apply ();
  }
}
namespace plugin::steam_jp {
  void apply_bloodstamp_patch ()
  {
    using namespace ::steam_jp;
  }
}

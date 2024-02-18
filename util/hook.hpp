/*
 * NGS2 NM Plugin by Nozomi Miyamori is marked with CC0 1.0.
 * This file is a part of NGS2 NM Plugin.
 */
#ifndef NGS2_NM_UTIL_HOOK_HPP
#define NGS2_NM_UTIL_HOOK_HPP

#include "util.hpp"
#include "distormx.h"

namespace ngs2::nm::util {
  template<typename T, typename S = T>
  class Hooker {
  protected:
    T target;
    S stub;
  public:
    constexpr Hooker (auto tgt, auto stb)
      : target (tgt), stub(stb) {}
    constexpr virtual T get_trampoline () = 0;
  };

  template <typename T, typename S = T>
  class InlineHooker final : protected Hooker<T, S> {
  public:
    constexpr InlineHooker (auto tgt_fn, auto stb)
      : Hooker<T, S> (reinterpret_cast<T>(tgt_fn), reinterpret_cast<S>(stb))
    {
      int r = distormx_hook(reinterpret_cast<void **>(&this->target), reinterpret_cast<void *>(this->stub));
      assert(r);
    }
    constexpr ~InlineHooker () { distormx_unhook (&this->target); }
    constexpr T get_trampoline () override { return this->target; }
  };

  template <typename T, typename S = T>
  class VFPHooker final : protected Hooker<T, S> {
    uintptr_t vfp;
  public:
    constexpr VFPHooker (uintptr_t tgt_vfp, auto stb)
      : Hooker<T, S> (*reinterpret_cast<T *>(tgt_vfp), reinterpret_cast<S>(stb)),
	vfp (tgt_vfp)
    {
      WriteMemory (vfp, reinterpret_cast<uintptr_t>(stb));
    }
    constexpr ~VFPHooker () { WriteMemory (vfp, reinterpret_cast<uintptr_t>(this->target)); }
    constexpr T get_trampoline () override { return this->target; }
  };

  class CallHooker : protected Hooker<uintptr_t> {
    uintptr_t o_callee_fn;
  public:
    constexpr CallHooker (uintptr_t caller_addr, uintptr_t callee_fn)
      : Hooker<uintptr_t> (caller_addr, callee_fn)
    {
      this->o_callee_fn = caller_addr + *reinterpret_cast<int32_t *>(caller_addr + 1);
      WriteMemory(caller_addr + 1,
		  -static_cast<int32_t>(caller_addr + 5	- callee_fn));
    }
    constexpr ~CallHooker ()
    {
      WriteMemory (this->target + 1, -static_cast<int32_t>(this->target + 5 - this->o_callee_fn));
    }
    constexpr uintptr_t get_trampoline () override { return this->target; }
  };
}

#endif

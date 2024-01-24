#ifndef NGS2_NM_UTIL_HPP
#define NGS2_NM_UTIL_HPP

#define WIN32_LEAN_AND_MEAN
#include <polyhook2/IHook.hpp>
#include <polyhook2/Detour/x64Detour.hpp>
#include <windef.h>
#include <psapi.h>
#include <memoryapi.h>
#include <libloaderapi.h>
#include <processthreadsapi.h>
#include <cstdint>
#include <map>
#include <memory>

#if defined(NDEBUG)
# define D(x)
#else
# define D(x) x
#endif

namespace ngs2::nm::util {
  using namespace std;

  // Make hook with hook() function. The detour instance can be obtained
  // with a trampoline as a key
  class HookMap final : map<uintptr_t, unique_ptr<PLH::IHook>> {
  public:
    // This makes inline hook that calls fn_callback from fn_hookee,
    // and returns trampoline for fn_hookee.
    inline uintptr_t
    hook (uintptr_t fn_hookee, uintptr_t fn_callback);
  };

  inline uintptr_t
  HookMap::hook (uintptr_t fn_hookee, uintptr_t fn_callback)
  {
    uintptr_t tramp;
    auto d = make_unique<PLH::x64Detour> (fn_hookee, fn_callback, &tramp);
    if (!d->hook ())
      return 0;
    map::insert_or_assign (tramp, std::move(d));
    return tramp;
  }

  // This calculates virtual address of rva.
  inline uintptr_t
  VA (uintptr_t rva)
  {
    MODULEINFO moduleInfo;
    GetModuleInformation(GetCurrentProcess (),
			 GetModuleHandle (nullptr),
			 &moduleInfo,
			 sizeof (moduleInfo));
    return reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll) + rva;
  }

  // This forcibly overwrites the content of addr with lpBuffer.
  inline bool
  WriteMemory (SIZE_T addr, LPCVOID lpBuffer, SIZE_T nSize)
  {
    LPVOID p = reinterpret_cast<LPVOID>(addr);
    DWORD flOld;

    VirtualProtect (p, nSize, PAGE_READWRITE, &flOld);
    BOOL r = WriteProcessMemory
      (GetCurrentProcess (), p, lpBuffer, nSize, NULL);
    VirtualProtect (p, nSize, flOld, &flOld);

    return r;
  }
}

#endif

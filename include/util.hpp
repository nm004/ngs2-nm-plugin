#ifndef NGS2_NM_UTIL_HPP
#define NGS2_NM_UTIL_HPP
#define WIN32_LEAN_AND_MEAN

#include "debug.hpp"
#include <polyhook2/IHook.hpp>
#include <polyhook2/Detour/x64Detour.hpp>
#include <windef.h>
#include <psapi.h>
#include <memoryapi.h>
#include <libloaderapi.h>
#include <processthreadsapi.h>
#include <cstdint>
#include <vector>
#include <memory>

namespace {
  using namespace std;

  vector<unique_ptr<PLH::IHook>> detours {};

  // This calculates virtual address of rva.
  uintptr_t
  VA (uintptr_t rva)
  {
    MODULEINFO moduleInfo;
    GetModuleInformation(GetCurrentProcess (),
			 GetModuleHandle (nullptr),
			 &moduleInfo,
			 sizeof (moduleInfo));
    return reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll) + rva;
  }

  // This makes inline hook that calls fn_callback from fn_hookee,
  // and returns trampoline for fn_hookee.
  uintptr_t
  hook (uintptr_t fn_hookee, uintptr_t fn_callback)
  {
    uintptr_t tramp;
    auto d = make_unique<PLH::x64Detour>(fn_hookee, fn_callback, &tramp);
    if (!d->hook ())
      return 0;
    detours.push_back(std::move(d));
    return tramp;
  }

  // This forcibly overwrites the content of addr with lpBuffer.
  bool
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

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
#include <span>
#include <functional>
#include <iostream>

#if defined(NDEBUG)
# define D(x)
#else
# define D(x) x
#endif

namespace ngs2::nm::util {
  // We use the already calculated value for now.
#if NINJA_GAIDEN_SIGMA_2_TARGET_STEAM_JP
  constexpr uintptr_t rdata_section_rva = 0x179d000;
  constexpr uintptr_t data_section_rva = 0x1c4a000;
#elif  NINJA_GAIDEN_SIGMA_2_TARGET_STEAM_AE
  constexpr uintptr_t rdata_section_rva = 0x179e000;
  constexpr uintptr_t data_section_rva = 0x1c4b000;
#endif

  // Make hook with hook() function. The detour instance can be obtained
  // with a trampoline as a key
  class HookMap final : std::map<uintptr_t, std::unique_ptr<PLH::IHook>> {
  public:
    // This makes inline hook that calls fn_callback from fn_hookee,
    // and returns trampoline for fn_hookee.
    inline uintptr_t
    hook (uintptr_t fn_hookee, uintptr_t fn_callback);
  };

  inline uintptr_t
  HookMap::hook (uintptr_t fn_hookee, uintptr_t fn_callback)
  {
    using namespace std;
    uintptr_t tramp;
    auto d = make_unique<PLH::x64Detour> (fn_hookee, fn_callback, &tramp);
    if (!d->hook ())
      return 0;
    map::insert_or_assign (tramp, move(d));
    return tramp;
  }

  namespace internal {
    inline uintptr_t
    get_process_base_addr ()
    {
      MODULEINFO moduleInfo;
      GetModuleInformation(GetCurrentProcess (),
			   GetModuleHandle (nullptr),
			   &moduleInfo,
			   sizeof (moduleInfo));
      return reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);
    }
  }

  // This calculates virtual address of rva.
  inline uintptr_t
  VA (uintptr_t rva)
  {
    return internal::get_process_base_addr () + rva;
  }

  // This finds the address that holds the same pattern.
  template <size_t N>
  inline uintptr_t
  VA (const uint8_t (&arr)[N])
  {
    using namespace std;
    // We use the already calculated value for now.
    const uintptr_t base_of_code = 0x1000;
#if NINJA_GAIDEN_SIGMA_2_TARGET_STEAM_JP
    const uintptr_t size_of_code = 0x179bf90;
#elif  NINJA_GAIDEN_SIGMA_2_TARGET_STEAM_AE
    const uintptr_t size_of_code = 0x179c2e0;
#endif
    const uintptr_t pe_base = internal::get_process_base_addr ();
    auto text = std::span<uint8_t>{
      reinterpret_cast<uint8_t *>(pe_base) + base_of_code,
      size_of_code
    };
    auto pat = span{arr};
    boyer_moore_searcher searcher {
      pat.begin (), pat.end ()
    };
    auto addr = searcher (text.begin (), text.end ());
    return pe_base + base_of_code + distance (text.begin (), addr.first);
  }

  // This is the wrapper for the search version of VA() above, and prints out the
  // declaration of the virtual address varibale. You might be want to use this via
  // the macro VA_PRINT_DECL. You should use this for development purpose.
  template <size_t N, size_t M>
  inline uintptr_t
  VA (const uint8_t (&arr)[N], const char (&name)[M])
  {
    using namespace std;
    uintptr_t r = VA(arr);
    cout << hex <<
      "const uintptr_t " << name << " = VA (0x" << r - internal::get_process_base_addr () << ");"
	 << endl;
    return r;
  }
#define VA_PRINT_DECL(x) VA (x##_pattern, #x)

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

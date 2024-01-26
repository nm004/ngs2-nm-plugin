#ifndef NGS2_NM_UTIL_HPP
#define NGS2_NM_UTIL_HPP

#define WIN32_LEAN_AND_MEAN
#include <polyhook2/IHook.hpp>
#include <polyhook2/Detour/x64Detour.hpp>
#include <windef.h>
#include <winnt.h>
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
  enum class NGS2_BINARY_KIND {
    STEAM_JP, // for Japan and East Asia
    STEAM_AE, // for America, Europe and other than East Asia
    UNKNOWN,
  };

  namespace internal {
    inline uintptr_t
    get_base_of_image () noexcept
    {
      MODULEINFO moduleInfo;
      GetModuleInformation(GetCurrentProcess (),
			   GetModuleHandle (nullptr),
			   &moduleInfo,
			   sizeof (moduleInfo));
      return reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);
    }

    inline PIMAGE_NT_HEADERS64
    get_image_nt_headers () noexcept
    {
      uintptr_t base_addr = get_base_of_image ();
      uint32_t pehdr_ofs = *reinterpret_cast<uint32_t *>(base_addr + 0x3c);
      return reinterpret_cast<PIMAGE_NT_HEADERS64>(base_addr + pehdr_ofs);
    }

    inline NGS2_BINARY_KIND
    get_binary_kind ()
    {
      switch  (get_image_nt_headers ()->OptionalHeader.SizeOfCode)
	{
	case 0x179c000:
	  return NGS2_BINARY_KIND::STEAM_JP;
	case 0x179c400:
	  return NGS2_BINARY_KIND::STEAM_AE;
	default:
	  return NGS2_BINARY_KIND::UNKNOWN;
	}
    }

    inline uintptr_t
    calc_base_of_data ()
    {
      auto optHdr = get_image_nt_headers ()->OptionalHeader;
      uintptr_t endOfCode = get_base_of_image () + optHdr.BaseOfCode + optHdr.SizeOfCode;
      return endOfCode + ((optHdr.SectionAlignment - endOfCode % optHdr.SectionAlignment) % optHdr.SectionAlignment);
    }
  }

  inline const NGS2_BINARY_KIND binary_kind = internal::get_binary_kind ();
  inline const uintptr_t base_of_image = internal::get_base_of_image ();
  inline const uintptr_t start_of_code =
    base_of_image + internal::get_image_nt_headers ()->OptionalHeader.BaseOfCode;
  inline const uintptr_t start_of_data = internal::calc_base_of_data ();

  // Make hook with hook() function. The detour instance can be obtained
  // with a trampoline as a key
  class HookMap final : std::map<uintptr_t, std::unique_ptr<PLH::IHook>> {
  public:
    // This makes inline hook that calls fn_callback from fn_hookee,
    // and returns trampoline for fn_hookee.
    inline uintptr_t
    hook (uintptr_t fn_hookee, uintptr_t fn_callback) noexcept;
  };

  inline uintptr_t
  HookMap::hook (uintptr_t fn_hookee, uintptr_t fn_callback) noexcept
  {
    using namespace std;
    uintptr_t tramp;
    auto d = make_unique<PLH::x64Detour> (fn_hookee, fn_callback, &tramp);
    if (!d->hook ())
      return 0;
    map::insert_or_assign (tramp, move(d));
    return tramp;
  }

  // This finds the address that holds the same pattern.
  template <size_t N>
  inline uintptr_t
  VAof (const uint8_t (&arr)[N]) noexcept
  {
    using namespace std;

    auto text = std::span {
      reinterpret_cast<uint8_t *>(start_of_code),
      internal::get_image_nt_headers ()->OptionalHeader.SizeOfCode
    };
    auto pat = span{arr};
    boyer_moore_searcher searcher {
      pat.begin (), pat.end ()
    };
    auto addr = searcher (text.begin (), text.end ());
    return start_of_code + distance (text.begin (), addr.first);
  }

  // This is the wrapper for the search version of VAof() above, and prints out the
  // declaration of the virtual address varibale. You might be want to use this via
  // the macro VAof. You should use this for development purpose.
  template <size_t N, size_t M>
  inline uintptr_t
  VAof (const uint8_t (&arr)[N], const char (&name)[M]) noexcept
  {
    using namespace std;
    uintptr_t r = VA(arr);
    cout << hex <<
      name << " = base_of_image + 0x" << r - base_of_image << ";"
	 << endl;
    return r;
  }
#define VAof_(x) VAof (x##_pattern, #x)

  // This forcibly overwrites the content of addr with lpBuffer.
  inline bool
  WriteMemory (SIZE_T addr, LPCVOID lpBuffer, SIZE_T nSize) noexcept
  {
    LPVOID p = reinterpret_cast<LPVOID>(addr);
    DWORD flOld;

    VirtualProtect (p, nSize, PAGE_READWRITE, &flOld);
    BOOL r = WriteProcessMemory
      (GetCurrentProcess (), p, lpBuffer, nSize, NULL);
    VirtualProtect (p, nSize, flOld, &flOld);

    return r;
  }

  template <typename T>
  inline bool
  WriteMemory (SIZE_T addr, const T &v) noexcept
  {
    return WriteMemory (addr, &v, sizeof(T));
  }
}

#endif

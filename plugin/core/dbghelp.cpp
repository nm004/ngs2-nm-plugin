#define WIN32_LEAN_AND_MEAN
#include <windef.h>
#include <unknwn.h>
#include <strsafe.h>

#if defined(__MINGW32__)
# define DLLEXPORT
#else
# define DLLEXPORT __declspec (dllexport)
#endif

typedef DWORD MINIDUMP_TYPE;
typedef LPVOID PMINIDUMP_EXCEPTION_INFORMATION;
typedef LPVOID PMINIDUMP_USER_STREAM_INFORMATION;
typedef LPVOID PMINIDUMP_CALLBACK_INFORMATION;

// This calls MiniDumpWriteDump in the original dbghelp.dll
extern "C" DLLEXPORT BOOL
MiniDumpWriteDump (HANDLE hProcess,
		   DWORD ProcessId,
		   HANDLE hFile,
		   MINIDUMP_TYPE DumpType,
		   PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
		   PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
		   PMINIDUMP_CALLBACK_INFORMATION CallbackParam)
{
  TCHAR path[MAX_PATH];
  GetSystemDirectory (path, MAX_PATH);
  StringCbCat (path, sizeof(path), TEXT("\\dbghelp.dll"));
  SetDllDirectory (TEXT(""));
  HMODULE hMod = LoadLibrary (path);

  return reinterpret_cast<decltype(MiniDumpWriteDump) *>
    (GetProcAddress(hMod, "MiniDumpWriteDump"))
    (hProcess, ProcessId, hFile, DumpType,
     ExceptionParam, UserStreamParam, CallbackParam);
}

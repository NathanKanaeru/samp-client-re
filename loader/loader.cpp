// samp_loader: clean-room launcher. Original samp.exe is UPX-packed
// (EntryRVA 0x1216A0, sections UPX0/UPX1) so its import table is hidden;
// observed neighbors: bass.dll, samp.saa (SAA2), SAMP/ game data.
// This launcher: finds gta_sa.exe next to itself, verifies samp.saa +
// client DLL exist, then CreateProcess + inject via remote LoadLibrary.
#include <windows.h>
#include <string>
#include <cstdio>

static std::string DirOf(const std::string& p) {
  auto i = p.find_last_of("\\/");
  return i == std::string::npos ? "." : p.substr(0, i);
}

static bool FileExists(const std::string& p) {
  DWORD a = GetFileAttributesA(p.c_str());
  return a != INVALID_FILE_ATTRIBUTES;
}

static bool Inject(HANDLE proc, const std::string& dll) {
  size_t n = dll.size() + 1;
  void* mem = VirtualAllocEx(proc, nullptr, n, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  if (!mem) return false;
  if (!WriteProcessMemory(proc, mem, dll.c_str(), n, nullptr)) return false;
  HMODULE k = GetModuleHandleA("kernel32.dll");
  auto* fn = (LPTHREAD_START_ROUTINE)(void*)GetProcAddress(k, "LoadLibraryA");
  HANDLE th = CreateRemoteThread(proc, nullptr, 0, fn, mem, 0, nullptr);
  if (!th) return false;
  WaitForSingleObject(th, 10000);
  CloseHandle(th);
  return true;
}

int main(int argc, char** argv) {
  char self[MAX_PATH] = {};
  GetModuleFileNameA(nullptr, self, MAX_PATH);
  std::string dir = DirOf(self);
  std::string gta = dir + "\\gta_sa.exe";
  std::string saa = dir + "\\samp.saa";
  std::string dll = dir + "\\samp_client.dll";
  if (argc > 1 && argv[1][0]) gta = argv[1];
  if (!FileExists(gta)) { printf("missing %s\n", gta.c_str()); return 1; }
  if (!FileExists(saa)) printf("warning: missing %s\n", saa.c_str());
  if (!FileExists(dll)) printf("warning: missing %s\n", dll.c_str());

  STARTUPINFOA si{}; PROCESS_INFORMATION pi{}; si.cb = sizeof si;
  std::string cmd = "\"" + gta + "\"";
  if (!CreateProcessA(gta.c_str(), cmd.data(), nullptr, nullptr, FALSE, CREATE_SUSPENDED,
                      nullptr, dir.c_str(), &si, &pi)) {
    printf("CreateProcess failed %lu\n", GetLastError());
    return 1;
  }
  bool ok = Inject(pi.hProcess, dll);
  ResumeThread(pi.hThread);
  CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
  printf(ok ? "injected %s\n" : "inject failed\n", dll.c_str());
  return ok ? 0 : 2;
}

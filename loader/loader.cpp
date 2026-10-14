// samp_loader: clean-room launcher implementing the observed samp.exe contract.
// Evidence: docs/LOADER.md (unpacked PE: CreateProcessA + WriteProcessMemory +
// CreateRemoteThread(LoadLibraryA) + ResumeThread; CLI `-c -h -p -n [-z]|-d`).
// The exact remote-allocation primitive in the original is unresolved
// (VirtualAllocEx absent from imports) — we use VirtualAllocEx, the standard
// equivalent, and say so. No game assets shipped; needs local GTA:SA install.
#include <windows.h>
#include <string>
#include <cstdio>
#include <cstring>

namespace {

std::string DirOf(const std::string& p) {
  auto i = p.find_last_of("\\/");
  return i == std::string::npos ? "." : p.substr(0, i);
}

bool FileExists(const std::string& p) {
  return GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES;
}

void Usage() {
  printf("samp_loader - clean-room SA-MP 0.3.DL launcher\n"
         "usage:\n"
         "  samp_loader -c -h HOST -p PORT -n NICK [-z PASS]\n"
         "  samp_loader -d\n"
         "  samp_loader [gta_path]   (defaults: .\\gta_sa.exe + .\\samp_client.dll)\n");
}

bool InjectRemote(HANDLE proc, const std::string& dll) {
  size_t n = dll.size() + 1;
  void* mem = VirtualAllocEx(proc, nullptr, n, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  if (!mem) { printf("VirtualAllocEx failed %lu\n", GetLastError()); return false; }
  if (!WriteProcessMemory(proc, mem, dll.c_str(), n, nullptr)) {
    printf("WriteProcessMemory failed %lu\n", GetLastError());
    return false;
  }
  auto* fn = (LPTHREAD_START_ROUTINE)(void*)GetProcAddress(
      GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
  HANDLE th = CreateRemoteThread(proc, nullptr, 0, fn, mem, 0, nullptr);
  if (!th) { printf("CreateRemoteThread failed %lu\n", GetLastError()); return false; }
  DWORD wr = WaitForSingleObject(th, 15000);
  CloseHandle(th);
  return wr == WAIT_OBJECT_0;
}

} // namespace

int main(int argc, char** argv) {
  char self[MAX_PATH] = {};
  GetModuleFileNameA(nullptr, self, MAX_PATH);
  std::string dir = DirOf(self);
  std::string gta = dir + "\\gta_sa.exe";
  std::string dll = dir + "\\samp_client.dll";

  // Parse observed CLI: -c -h HOST -p PORT -n NICK [-z PASS] | -d
  std::string host, port = "7777", nick = "Player", pass;
  bool connect = false, debug = false;
  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "-h") && i + 1 < argc) host = argv[++i];
    else if (!strcmp(argv[i], "-p") && i + 1 < argc) port = argv[++i];
    else if (!strcmp(argv[i], "-n") && i + 1 < argc) nick = argv[++i];
    else if (!strcmp(argv[i], "-z") && i + 1 < argc) pass = argv[++i];
    else if (!strcmp(argv[i], "-c")) connect = true;
    else if (!strcmp(argv[i], "-d")) debug = true;
    else if (argv[i][0] != '-') gta = argv[i];
    else { Usage(); return 1; }
  }
  if (!connect && !debug && host.empty()) {
    // No flags: legacy direct mode (gta path only).
  }
  if (connect && host.empty()) { printf("missing -h HOST\n"); return 1; }
  if (!FileExists(gta)) { printf("missing %s\n", gta.c_str()); return 1; }
  if (!FileExists(dll)) printf("warning: missing %s\n", dll.c_str());

  // Forward the exact contract to the game process.
  std::string cmd = "\"" + gta + "\"";
  if (debug) cmd += " -d";
  if (connect) {
    cmd += " -c -h " + host + " -p " + port + " -n " + nick;
    if (!pass.empty()) cmd += " -z " + pass;
  }

  STARTUPINFOA si{};
  PROCESS_INFORMATION pi{};
  si.cb = sizeof si;
  if (!CreateProcessA(gta.c_str(), cmd.data(), nullptr, nullptr, FALSE,
                      CREATE_SUSPENDED | CREATE_DEFAULT_ERROR_MODE,
                      nullptr, dir.c_str(), &si, &pi)) {
    printf("CreateProcess failed %lu\n", GetLastError());
    return 1;
  }
  bool ok = InjectRemote(pi.hProcess, dll);
  ResumeThread(pi.hThread);
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);
  printf(ok ? "launched + injected %s\n" : "launched but inject failed\n", dll.c_str());
  return ok ? 0 : 2;
}

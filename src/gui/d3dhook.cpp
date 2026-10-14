// D3D hook. Observed: FUN_10098c60 dynamic load, FUN_100a1190 borrowed device.
// No DX SDK at build time: everything via LoadLibrary/GetProcAddress.
#include "samp/gui/d3dhook.h"
#include <windows.h>
#include <string>

namespace samp { namespace gui {

static void* g_d3d9 = nullptr;
static void* g_device = nullptr;

void* LoadD3D9() { // FUN_10098c60
  if (g_d3d9) return g_d3d9;
  char sys[MAX_PATH] = {};
  GetSystemDirectoryA(sys, MAX_PATH);
  std::string p = std::string(sys) + "\\d3d9.dll";
  g_d3d9 = LoadLibraryA(p.c_str());
  return g_d3d9;
}

void* GetGtaDevice() { // FUN_100a1190: returns *(0x7F9D50|0x7F9D90) by version
  return g_device; // TODO(rea): read GTA pointer once version gate passes (x86 only)
}

bool InstallD3DHooks() {
  LoadD3D9();
  // TODO(rea): vtable Present(17)/Reset(16) patch on borrowed device.
  return g_d3d9 != nullptr;
}
void RemoveD3DHooks() {}
void OnLostDevice() {}
void OnResetDevice() {}

}} // namespace

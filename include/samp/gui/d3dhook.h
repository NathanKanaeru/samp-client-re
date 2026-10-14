#pragma once
// D3D9 overlay. Observed: FUN_10098c60 (LoadLibrary \d3d9.dll +
// GetProcAddress Direct3DCreate9), FUN_100a1190 (borrow GTA device via
// 0x7F9D50/0x7F9D90), DXUT window FUN_1007bf40/10081510/10080ab0,
// d3dx9_25.dll 50 imports. Reference (d3dhook.*): COM-object replacement —
// *(IDirect3DDevice9Hook**)0x00C97C28 = new IDirect3DDevice9Hook() wraps the
// borrowed device; only Present (slot 17) / Reset (slot 16) / SetTransform
// (view/proj cache) are overridden, rest forward. Fan-out OnLost/OnReset
// lists in main.cpp:643-711.
namespace samp { namespace gui {
void* LoadD3D9();        // FUN_10098c60
void* GetGtaDevice();    // FUN_100a1190 (+DAT_102ac9d0/102ac9b8)
bool  InstallD3DHooks(); // TODO(rea): Present/Reset vtable patch
void  RemoveD3DHooks();
void  OnLostDevice();
void  OnResetDevice();
}} // namespace

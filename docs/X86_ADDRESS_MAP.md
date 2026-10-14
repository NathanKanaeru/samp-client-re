#pragma once
// x86 GTA:SA 1.0 address map (human-readable). Source: samp-source
// client/game/address.h + game.cpp/hooks.cpp/patches.cpp; cross-checked
// against REA observations on samp.dll 0.3.DL-R1 (docs/REVERSE_NOTES.md).
// All addresses are ABSOLUTE and valid only inside the 32-bit GTA process.

| Address | Name | Meaning |
|---|---|---|
| 0x00C97C1C | HWND | main window |
| 0x00C97C28 | D3DDevice ptr | IDirect3DDevice9* — COM-replaced by hook (main.cpp:369) |
| 0x00C97C20 | D3D ptr | IDirect3D9* |
| 0x00B73458 | KeyStates | key states |
| 0x00BA67A4/A5/A6 | Menu 1/2/3 | menu flags |
| 0x00BA677B | StartGame | start flag (=0 in StartGame) |
| 0x00BA6831 | GameStarted | 1 = started |
| 0x00C8D4C0 | EntryState | 5 pre-game, 7 = wait target, 8 in-game |
| 0x00747483 | BypassVids US | 0x89 = USA10 (version detect) |
| 0x007474D3 | BypassVids EU | 0xC8 = EU10 |
| 0x007F9D50/90 | RwGetD3D US/EU | RenderWare::GetD3DDevice() |
| 0x00BA6769 | EnableHud | HUD on/off byte |
| 0x00442D30 | IdFromActor | PED* -> id |
| 0x00404910 | ActorFromId | id -> PED* |
| 0x00B74490 | PedTable | actor pool ptr |
| 0x0042C4B0 | IdFromVehicle | VEHICLE* -> id |
| 0x004048E0 | VehicleFromId | id -> VEHICLE* |
| 0x00B74494 | VehicleTable | vehicle pool ptr |
| 0x0094DBE0 | ObjectTable | object table |
| 0x00451C30 | ObjectFromId | id -> object |
| 0x00B6F99C | Camera | camera base |
| 0x00C17044/48 | Screen W/H | screen size ints |
| 0x00859520/24 | HudScale X/Y | HUD scale floats |
| 0x005B8E6A | StreamingMem | streaming memory dword (tiers 96–1024 MB) |
| 0x0058DB60 | WantedLevel | wanted byte |
| 0x00C81320/1C | Weather cur/lock | weather |
| 0x00B7CE50 | LocalMoney | money int |
| 0x00B7CB64 | GameSpeed | speed float |
| 0x00863984 | Gravity | gravity float |
| 0x009788C0 | PickupPool | pool base, stride 0x20 |
| 0x00BAB318 | LoadingActive | CLoadingScreen::g_bActive |
| 0x00B5FCC8 | RadioVolume | radio volume 0–1 |

Patch/hook call sites (patches.cpp/hooks.cpp): game loop 0x0053EB13,
radar 0x0058FC53, crosshair 0x0058FBBF, timer 0x0053E930, shutdown 0x0053C900,
camera process 0x0053C104, weapon shot 0x0073C252, train derail 0x006F8CF8,
pickup 0x004579C6, anim fix 0x004D41C0, horn 0x00501B1D/42/C2/67/AE.
Vtable slots (method hooks): ped render 0x0086D1B0/…/0x0086C3A0,
player control 0x0086D190, task gun 0x0086D744, vehicles 0x00871148…0x00872398.
D3D vtable: Reset slot 16, Present slot 17.

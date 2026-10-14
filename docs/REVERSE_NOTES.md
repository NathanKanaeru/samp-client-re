# REVERSE_NOTES — samp.dll 0.3.DL-R1 (SHA256 bccdb297…cd4b)

Sumber: REA + Ghidra 12.1.4, 5 agen paralel. Semua alamat = default-space `0x…`.
JANGAN `close_binary` sebelum export bila ingin melanjutkan sesi.

## Identitas

* PE x86 DLL, ImageBase `0x10000000`, size 1466368, entry CRT `0x100fdb60`,
  DllMain `0x100c6780` (thunk `0x100c6930`), 6293 prosedur / 4265 string.
* Versi: `{FFFFFF}SA-MP {B9C9BF}0.3.DL-R1`, `0.3.DL-R1`, `SA-MP 0.3.DL-R1`.
* Export directory kosong (di-load via injeksi samp.exe, bukan LoadLibrary ekspor).
* Import: d3dx9_25 50 (obs) / USER32 88 / KERNEL32 129 / WSOCK32 25 ord /
  WS2_32 7 / BASS 17 / ADVAPI32 Crypt+Reg / PSAPI 1 / WINMM 2 / GDI32 10.

## CORE (agen 1)

* `0x100c6780 DllMain`: ATTACH → Validate → SEH(`0x10060430`) → samp.saa →
  fonts → `0x10062720/0x100623f0` → mgr `0x142` (`0x1009fdc0`) →
  `__beginthread(0x100c5150)`; DETACH → `0x10062810`.
* `0x100c5c90 ValidateGame`: zero `0x144` dword @`0x102abe78`, parse
  `-c/-d/-h/-l/-n/-p/-z` (`0x100c5590/0x100c55e0`).
* `CArchive 0x10064dd0` (obj `0x935`, entryCap `0x78`), `Open 0x10064f20`
  (vtable+4, 2 reads, XOR-check `0x83433`/`0x200000`), `Close 0x10064890`,
  vtable `0x1011bae0`. `CFile 0x10064bf0 → 0x10063f00 → 0x100f9cc9(__fsopen)`.
* `0x10062720 InstallFileHooks`: detour CreateFileA→`LAB_100625d0`,
  ReadFile→`LAB_100620f0`, GetFileSize→`0x10062270`,
  SetFilePointer→`0x100622c0`, CloseHandle→`0x10062360`,
  GetFileType→`0x100623a0` (inline-JMP, `0x10110960/0x10110700/0x101111e0`).
* `0x100623f0 InstallCursorHook`: ShowCursor→`LAB_100623e0`.
* `LAB_100c5150 InitThreadProc` (label!): `DetectGtaVersion 0x100a06d0` →
  wait `[0x00C8D4C0]==7` → `FlagInitDone 0x1009db90` →
  `InitGameSystems 0x100a0720` → ExitThread.
* `LAB_10060430` filter → `DialogBoxParamA(...,0x10060330,...)`;
  `0x10060330` dialog proc → `ReportCrash 0x10060260`
  (`sprintf team.sa-mp.com/report_037.php?addr=0x%X&gta=sa`).
* `0x100c66b0 IdleTick` → `DAT_102ac9cc`.
* Globals: `0x102ac9c8 hInstance`, `0x102abe78/7c valid`,
  `0x102ac9d4 archive`, `0x102aca3c manager`, `0x102ac9cc idle`.
* `samp.saa`: literal hanya `0x1011fc38`; `SAA2` 0-hit di binary (isi file).
  File riil: 2689408 B, header `SAA2`, entryCap runtime `0x78`.

## NET (agen 2)

* Strings: `Connecting to %s:%d...@0x10117a0c`, `Lost…@0x10117bc8`,
  `banned/full/password/norespond/closed/connected@0x10117xxx`,
  `0.3.DL-R1@0x10117bfc`, `Failed to initialize encryption.`,
  `Get/SetIncomingPassword`, `Connect(...)`, `S|R,Typ,...Local IP:Port`.
* `RPC/BitStream/ID_CONNECTION_*` = 0-hit (absent dari defined-strings).
* Tabel nama `0x10033650` (not_in_procedure, decode manual):
  200 VEHICLE, 201 RCON_CMD, 202 RCON_RESP(typo asli), 203 AIM, 204 WEAPONS,
  205 STATS, 206 Unknown, 207 PLAYER, 208 Unknown, 209 PASSENGER,
  210 SPECTATOR, 211 TRAILER, 212 UNOCCUPIED. Hanya stringify (RET per case).
* `0x10008630 SendConnectRetry` (timer 3s, vtable[1] Connect),
  `0x1000b290 Pump`, `0x1000af40 Dispatch` (Receive=v[8], Deallocate=v[9],
  `pkt[0]==40h?pkt[5]`), `0x1000aa60 OnAccept` (tulis `0xFDE`+nick+hash+
  `0.3.DL-R1`, Send slot 25 pri=1 rel=8 ch=0), `0x1000a9d0 OnLost`,
  `0x10008ad0 OnClosed`, `0x1000a230 ResetWorld`, `0x10009df0 AuthReply`.
* BitStream `0x1001f300/1f1f0/1f390/1f470/1f960/1faa0/1fb50/1f630`.
* `0x10034cd0` constructor pendaftar nama RakPeer (Initialize…Connect…
  SetIncomingPassword…), `0x100203f0→0x10020260`, `0x10030db0`,
  `0x10036a00` console pw.
* Socket: `send/recv/connect` thunk callers → HTTP client
  (`0x100029d0/0x10002a90/0x100026b0`), BUKAN RakNet. `sendto/recvfrom`
  tanpa thunk → RakNet statik-link. Port 7777/RPC/RCON-handler = UNKNOWN.

## GAME (agen 3)

* Hook file di atas + `0x10062000` (range `FF000000–FF000101`) +
  `0x10062040` (search 0x32 entri).
* Patcher: `0x100a5d80/0x100a9d10/0x100aa0b0` (VirtualProtect, tanpa
  WriteProcessMemory), `0x100a5e30` (6 CALL `0053EB13…` + vtable
  `0086D1B0…→LAB_100a3b90 PUSHAD`), `0x100a6020` (E8 `007330A2…`,
  E9 `004579C6…`, vtable `…→LAB_100a26d0/29d0/2b50/3e50/2e70`),
  `0x100a6d50/0x100a6f00/0x100a19f0`, `0x100aaad0` NOP-er,
  `0x100aa300` version gate (`00747483` 0x89 US / 0xC8 EU),
  `0x100a0720` orkestrator.
* Manager `0x1009fdc0` (`0x142` B, `+0x55=0,+0x59=1,+0x5D=0x5A`).
* Pool: vehicle 2000 (`0x1001eb40/0x100a0090/0x100b7f00`, obj `0xB8`),
  player 1004 (`0x100109d0/0x10016c50/0x1009ff50`, obj `0x32D`,
  `Warning: Couldn't spawn player`), object 2101, actor 1000.
* Spawn/class: `0x100109d0`, `0x10068440` (`AddPlayerClass fprintf`),
  `0x10010530` connect, `0x100c6ce0` UI Spawn (`0x1EA`).
* Kamera hook asli UNKNOWN; `0x10004710` hanya debug overlay
  (`CAMERA_TARGET_TYPE_...`).
* Model DL: `SAMP.img` 47MB dkk; string model `%s/0x%X.dff/txd`,
  validasi `[warning] Could not validate…` via `0x1000BB10/0x100b5900`.

## GUI (agen 4)

* `0x10098c60`: `GetSystemDirectoryA+\d3d9.dll→LoadLibrary→
  GetProcAddress(Direct3DCreate9+D3DPERF_*)`, cache `0x10181aa4`.
* `0x100a1190`: pinjam device GTA (`*0x7F9D50` jika ver==1 else `*0x7F9D90`).
* DXUT `0x1007bf40/0x10081510(Direct3DWindowClass)/0x10080ab0/
  0x1008c170(CDXUT…CreateFont)`. Present/EndScene 0-hit → hook = vtable
  Present/Reset (inferred).
* Chat `0x10067c40` (surface 1024×512/2048×1024 → direct fallback),
  `0x100c5e70` root init (cfg/chatlog, 40 callees), `0x10068b50` cmd,
  `0x100c6950/69a0/69e0` fontsize, `0x10069b90` GTAWEAPON3,
  `0x100695f0/0x1006b320(+calloc 0x186a1)/0x10071600` dialog/scoreboard/
  textdraw, `0x1006b570` F1 help, `0x10070350` SignText,
  `0x10061cf0+0x10075040` screenshot (`\screens\sa-mp-%03i.png`).
* `gtaweap3/sampaux3` via AddFontResourceA di DllMain.

## AUDIO/INPUT/MISC (agen 5)

* BASS 17 import; `0x10065f00` init (`Init(-1,0xAC44)`, `"SA-MP/0.3"`),
  `0x100663e0` request + `__beginthread(0x10066180)`,
  thread `StreamCreateURL(flags 0x940000)` + Play + Tags + SetSync +
  `0x10066040` ICY parse + 3D errors.
* Input: USER32 88; `0x10005f40` (Shift/Left/Right),
  `0x100751b0` (RMB/F1/F8/F9), `0x10061970→0x100610d0` WndProc
  (body UNKNOWN), `0x10068b50` cmd table (fpslimit…audiomsg…rcon…).
* Screenshot/crash/loader/ADVAPI32-Crypt (`0x100e2a20` GenRandom,
  `0x100e4050` MD5-hash) + PSAPI stats `0x100607f0` — lihat laporan agen.
* `samp.exe` 413696 B UPX-packed (entry `0x1216A0`), strings minim;
  injeksi exact = UNKNOWN (butuh `upx -d` + analisis ulang, tanpa eksekusi).

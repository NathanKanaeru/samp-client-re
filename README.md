# samp-client-re — SA-MP 0.3.DL-R1 clean-room client

Reimplementasi clean-room dari `samp.dll` (PE x86, 1.466.368 bytes,
SHA256 `bccdb297464bd382625635be25585df07a8fa6668bc0015650708e3eb4ffcd4b`,
string versi `{FFFFFF}SA-MP {B9C9BF}0.3.DL-R1`, 6293 prosedur / 4265 string
hasil `binary_overview`). Tujuannya: **client sendiri yang buildable** dengan
arsitektur yang sama, bukan byte-identical copy.

## Cara build (Windows + MinGW x64, terverifikasi)

```powershell
cd samp-client-re
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

Hasil: `build/samp_client.dll`, `build/samp_loader.exe`, `build/samp_smoke.exe`.
Jalankan smoke test: `.\build\samp_smoke.exe` (tidak butuh GTA).

> Catatan 32-bit: GTA:SA itu x86. Source ini portable (tidak ada inline ASM
> x86, alamat GTA disimpan sebagai `uintptr_t`/konstanta). Untuk injeksi nyata,
> build 32-bit dengan toolchain i686 (`cmake -DSAMP_X86=ON`) atau MSVC `-A Win32`.

## Peta arsitektur (hasil REA/Ghidra 12.1.4)

| Modul | Asli (alamat) | File baru |
|---|---|---|
| DllMain + init thread + validasi cmdline | `0x100c6780`, `0x100c5c90`, `LAB_100c5150`, `0x100a06d0/0x100a0720/0x1009db90` | `src/core/dllmain.cpp`, `src/core/version.cpp` |
| Arsip `samp.saa` (obj `0x935`, XOR-check `0x83433`) | `0x10064dd0/0x10064f20/0x10064890`, vtable `0x1011bae0` | `src/core/archive.cpp` |
| File-hook detour (CreateFile/ReadFile/ShowCursor) | `0x10062720/0x100623f0/0x10062810`, stub `LAB_100625d0/100620f0/100623e0` | `src/core/hooks_detour.cpp` |
| Crash reporter (`report_037.php`) | `LAB_10060430`, `LAB_10060330`, `0x10060260` | `src/core/exception.cpp`, `src/util/crash.cpp` |
| RakNet wrapper + packet dispatch + BitStream | `0x10008630/0x1000b290/0x1000af40/0x1000aa60/0x1000a9d0`, tabel nama `0x10033650`, `0x1001fxxx` | `src/net/*.cpp` |
| Patch memori GTA + pool player/vehicle | `0x100a5d80/0x100a5e30/0x100a6020/0x100aaad0/0x100aa300`, `0x1009fdc0` (mgr `0x142`) | `src/game/*.cpp` |
| D3D9 overlay + chat/dialog/scoreboard/textdraw | `0x10098c60/0x100a1190`, `0x10067c40/0x100c5e70`, `0x10075040` | `src/gui/*.cpp` |
| Audio BASS (stream + 3D) | `0x10065f00/0x100663e0` + thread `0x10066180` | `src/audio/audio.cpp` |
| Input/WndProc + command table | `0x10061970/0x100610d0`, `0x10068b50`, `0x10005f40/0x100751b0` | `src/input/input.cpp` |
| Config/screenshot/string | `%s\sa-mp.cfg`, `\screens\sa-mp-%03i.png` | `src/util/*.cpp` |
| Launcher (samp.exe UPX-packed, entry `0x1216A0`) | — statis saja | `loader/loader.cpp` |

Detail alamat + evidence_id per klaim: `docs/REVERSE_NOTES.md`.

## Batasan jujur

* Binary stripped: semua nama `FUN_xxx` dari Ghidra adalah inferred. Nama baru
  (`CArchive`, `CGame`, `Net_Pump`, …) adalah interpretasi analis.
* Tabel packet `0x10033650` hanya stringify (tiap case langsung RET), bukan dispatcher.
* Handler RCON/RPC penuh, port default 7777 wire-format, dan trampolin `Present`
  exact = UNKNOWN dan ditandai `TODO(rea)` di kode — butuh dynamic tracing.
* `samp.saa` terkunci XOR (`mask 0x83433`); header plaintext `SAA2` tidak ada di
  binary (hanya di file), jadi parser arsip adalah行为-equivalent, bukan decryptor penuh.

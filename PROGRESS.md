# PROGRESS — samp-client-re (SA-MP 0.3.DL-R1 clean-room)

> Aturan: file ini WAJIB diupdate setiap ada perubahan besar (fitur baru,
> modul selesai, temuan REA baru, build/test status berubah). Lihat `AGENTS.md`.
> Format baris history: `| YYYY-MM-DD | area | +/-x% | alasan singkat |`.

## Overall: 81% ✅ buildable (x64) · smoke PASS · loader+D3D+sync

```
[███████████████████░░░░░] 81%
```

| # | Area (bobot) | % | Status / bukti |
|---|---|---|---|
| 01 | Identitas & inventaris REA (10) | 100% | PE x86, SHA `bccdb297…`, 6293 proc / 4265 str, import map, versi `0.3.DL-R1` — `docs/REVERSE_NOTES.md` |
| 02 | Core: DllMain/archive/hooks/exception/version (20) | 84% | +`LOADER.md` (UPX unpack 414K→1.1M, imports/strings injeksi) + loader CLI penuh `-c -h -p -n [-z]`; sisa: RSA-verify, `LAB_*` boundaries |
| 03 | Net: packets/BitStream/NetGame/RakPeer/RPC (20) | 86% | +`RouteSync` + counter per-ID (netstats) + dispatch 200→pool; sisa: vendor RakNet + wire live |
| 04 | Game: manager/pools/patches (15) | 76% | +`BlendVehiclePos` (deadband/snap/gain PACKET200) teruji; sisa: aplikasi patch runtime x86 |
| 05 | GUI/D3D: hook/chat/dialog/score/textdraw/fonts/shot (15) | 72% | +`D3DDeviceHook` vtable-copy (__thiscall, slot 16/17) + fake-vtable test PASS; sisa: uji D3D nyata |
| 06 | Audio/Input/Util/Loader (10) | 76% | +loader CLI penuh; sisa: unpack exact-alloc, 3D penuh |
| 07 | Build & tes (10) | 75% | +CI GitHub Actions (MinGW build + smoke); smoke +blend +router +D3D PASS; sisa: build x86, injeksi nyata |

Perhitungan: `(10×1.00 + 20×0.84 + 20×0.86 + 15×0.76 + 15×0.72 + 10×0.76 + 10×0.75)/100 = 81.3 → 81%`.

## Definisi % per area

* 0%: belum mulai. 25%: inventarisasi string/fungsi selesai.
* 50%: struktur + stub kompilasi, perilaku inti terdokumentasi.
* 75%: logika setara perilaku, teruji (unit/smoke), sisa edge-case.
* 100%: setara perilaku + terverifikasi runtime (GTA x86 / jaringan nyata).

## History

| Tanggal | Area | Perubahan | Overall |
|---|---|---|---|
| 2026-10-14 | semua | Baseline: REA 5 agen → 57 file source, CMake x64 build OK, smoke PASS | 62% |
| 2026-10-14 | docs | Repo GitHub dibuat + push awal (`NathanKanaeru/samp-client-re`, public): AGENTS.md, PROGRESS.md, .gitignore | 62% |
| 2026-10-14 | semua | Port referensi samp-source (4 agen miner): `gta_addrs.h` + `X86_ADDRESS_MAP.md`, `limits.h`, `rpc_ids.h`, format SAA exact, COM-replacement D3D, cmd list penuh; smoke static_assert PASS, build OK | 72% |
| 2026-10-14 | core+net | Port libsamp (4 agen miner): `saa.h/.cpp` + endpoint/socket/bootstrap/adapter; **SAA terverifikasi file asli** (main 103876/script 194560, magic cocok); UDP loopback + lifecycle PASS; `PACKET200` + `BOOT_SEQUENCE` docs | 77% |
| 2026-10-14 | loader+gui+net | UPX unpack samp.exe → `LOADER.md` + loader CLI; `D3DDeviceHook`; `RouteSync`/`BlendVehiclePos`; CI workflow; build+smoke PASS | 81% |

## Next (untuk naik %)

* [x] ~~unpack `samp.exe` (UPX)~~ selesai → `docs/LOADER.md` (sisa: exact-alloc primitive).
* [ ] `+5%`: vendor Knogle/RakNet + drain loop + wire-test live → Net 86→95%.
* [ ] `+4%`: build x86 terverifikasi (i686/MSVC) → Build 75→100% sebagian.
* [ ] `+3%`: uji D3D nyata + fan-out OnLost/OnReset → GUI 72→85%.

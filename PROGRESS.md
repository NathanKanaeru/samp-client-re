# PROGRESS — samp-client-re (SA-MP 0.3.DL-R1 clean-room)

> Aturan: file ini WAJIB diupdate setiap ada perubahan besar (fitur baru,
> modul selesai, temuan REA baru, build/test status berubah). Lihat `AGENTS.md`.
> Format baris history: `| YYYY-MM-DD | area | +/-x% | alasan singkat |`.

## Overall: 62% ✅ buildable (x64) · smoke PASS

```
[██████████████░░░░░░░░░░] 62%
```

| # | Area (bobot) | % | Status / bukti |
|---|---|---|---|
| 01 | Identitas & inventaris REA (10) | 100% | PE x86, SHA `bccdb297…`, 6293 proc / 4265 str, import map, versi `0.3.DL-R1` — `docs/REVERSE_NOTES.md` |
| 02 | Core: DllMain/archive/hooks/exception/version (20) | 65% | DllMain flow, `CArchive` XOR-check, detour engine, SEH reporter, version gate — build OK; sisa: key-schedule arsip penuh, tabel VFS 0x32 slot, `LAB_*` boundaries |
| 03 | Net: packets/BitStream/NetGame/RakPeer/RPC (20) | 55% | Tabel paket 200–212, BitStream round-trip OK, state-machine connect/join, API registry — `samp_smoke` PASS; sisa: transport UDP nyata, RPC/RCON handler, port default wire |
| 04 | Game: manager/pools/patches (15) | 50% | `CGame 0x142`, pool vehicle 2000/player 1004, alamat patch GTA absolut — kompilasi OK; sisa: aplikasi patch runtime x86, spawn FSM penuh, kamera hook |
| 05 | GUI/D3D: hook/chat/dialog/score/textdraw/fonts/shot (15) | 55% | LoadD3D9 dinamis, fallback surface→direct, font/screenshot logic — tanpa DX SDK; sisa: vtable Present/Reset nyata, render TextDraw/SignText exact |
| 06 | Audio/Input/Util/Loader (10) | 60% | BASS dynamic-load stub, polling kunci, cmd table, config/crash/shot, `samp_loader.exe` inject stub; sisa: 3D positional penuh, WndProc body `0x100610d0`, unpack `samp.exe` |
| 07 | Build & tes (10) | 60% | CMake MinGW-x64 OK → `samp_client.dll` + `samp_loader.exe` + `samp_smoke.exe` PASS; sisa: build x86 (`-DSAMP_X86`, butuh toolchain i686/MSVC), uji injeksi GTA nyata |

Perhitungan: `(10×1.00 + 20×0.65 + 20×0.55 + 15×0.50 + 15×0.55 + 10×0.60 + 10×0.60)/100 = 61.75 → 62%`.

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

## Next (untuk naik %)

* [ ] `+5%`: unpack `samp.exe` (UPX) → dokumentasikan injeksi exact → `loader/` nyata.
* [ ] `+6%`: transport UDP (SLikeNet/RakNet) + `SendConnectRetry` wire-test → Net 55→80%.
* [ ] `+5%`: build x86 terverifikasi (i686/MSVC) → Build 60→100% sebagian.
* [ ] `+4%`: Present/Reset vtable hook + uji D3D → GUI 55→80%.

# PROGRESS — samp-client-re (SA-MP 0.3.DL-R1 clean-room)

> Aturan: file ini WAJIB diupdate setiap ada perubahan besar (fitur baru,
> modul selesai, temuan REA baru, build/test status berubah). Lihat `AGENTS.md`.
> Format baris history: `| YYYY-MM-DD | area | +/-x% | alasan singkat |`.

## Overall: 72% ✅ buildable (x64) · smoke PASS · referensi samp-source di-port

```
[████████████████░░░░░░░░] 72%
```

| # | Area (bobot) | % | Status / bukti |
|---|---|---|---|
| 01 | Identitas & inventaris REA (10) | 100% | PE x86, SHA `bccdb297…`, 6293 proc / 4265 str, import map, versi `0.3.DL-R1` — `docs/REVERSE_NOTES.md` |
| 02 | Core: DllMain/archive/hooks/exception/version (20) | 75% | +format SAA exact (`SAA_FILE_ID 0x83433` konvergen REA↔referensi, TEA/RSA layout), daftar hook API + skema handle `0xFF000001…step 2`/50 slot; sisa: key-schedule penuh, `LAB_*` boundaries |
| 03 | Net: packets/BitStream/NetGame/RakPeer/RPC (20) | 70% | +enum RPC penuh 0–49/100–189 (`rpc_ids.h`), limit `limits.h`, join flow challenge^4242/mod/GPCI; smoke static_assert PASS; sisa: transport UDP nyata, verifikasi wire live |
| 04 | Game: manager/pools/patches (15) | 65% | +peta alamat x86 penuh (`gta_addrs.h`, `docs/X86_ADDRESS_MAP.md`), rekonsiliasi cap net-vs-overlay, situs patch/hook; sisa: aplikasi patch runtime x86, spawn FSM penuh |
| 05 | GUI/D3D: hook/chat/dialog/score/textdraw/fonts/shot (15) | 65% | +COM-replacement `0x00C97C28`, slot Present 17/Reset 16, cap tiap komponen, fan-out OnLost/OnReset; sisa: uji D3D nyata |
| 06 | Audio/Input/Util/Loader (10) | 72% | +BASS 44100/flags, daftar command penuh, alur WndProc; sisa: 3D penuh, unpack `samp.exe` |
| 07 | Build & tes (10) | 60% | CMake MinGW-x64 OK → `samp_client.dll` + `samp_loader.exe` + `samp_smoke.exe` PASS; sisa: build x86, uji injeksi GTA nyata |

Perhitungan: `(10×1.00 + 20×0.75 + 20×0.70 + 15×0.65 + 15×0.65 + 10×0.72 + 10×0.60)/100 = 71.7 → 72%`.

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

## Next (untuk naik %)

* [ ] `+5%`: unpack `samp.exe` (UPX) → dokumentasikan injeksi exact → `loader/` nyata.
* [ ] `+6%`: transport UDP (SLikeNet/RakNet) + `SendConnectRetry` wire-test → Net 55→80%.
* [ ] `+5%`: build x86 terverifikasi (i686/MSVC) → Build 60→100% sebagian.
* [ ] `+4%`: Present/Reset vtable hook + uji D3D → GUI 55→80%.

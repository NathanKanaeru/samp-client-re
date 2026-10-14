# AGENTS.md — aturan kerja agen di repo ini

> Berlaku untuk semua agen (manusia + AI) yang menyentuh `samp-client-re`.
> Proyek: clean-room reimplementasi SA-MP `0.3.DL-R1` client (`samp.dll` PE x86,
> SHA256 `bccdb297464bd382625635be25585df07a8fa6668bc0015650708e3eb4ffcd4b`).
> Bukan byte-copy — perilaku-setara, berdasarkan evidence REA.

## 1. Build / test (wajib verifikasi)

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
.\build\samp_smoke.exe
```

* Jangan klaim selesai tanpa `cmake --build build` + smoke PASS.
* Build default = MinGW-w64 **x64** (logika + DLL shell). Untuk injeksi GTA nyata
  butuh x86: `cmake -S . -B build-x86 -G "MinGW Makefiles" -DSAMP_X86=ON`
  (butuh toolchain i686) atau MSVC `-A Win32`.
* DITEMUKAN 2026-10-14: eksekusi proses 32-bit diblokir di sandbox lokal ini
  (bukti: `cc1.exe` i386 0x14c gagal diam-diam, `build-x86` configure gagal).
  Jangan coba build-x86 lokal berulang — gunakan CI (`mingw-x86` job di
  `.github/workflows/build.yml`, runner GitHub mengeksekusi 32-bit normal).
* Referensi kode pakai pola `path:line`, mis. `src/net/bitstream.cpp:40`.

## 2. REA (reverse engineering)

* Gunakan REA MCP + Ghidra hanya untuk **fakta teramati** (alamat, string, xref,
  pseudocode). Bedakan di kode/komentar: `observed` vs `inferred` vs `unknown`.
* Fungsi Ghidra `FUN_xxx` = stripped/inferred — beri nama usulan + alamat asli
  di komentar. Alamat `LAB_*` = boundary inferred (`not_in_procedure`).
* JANGAN `close_binary` di tengah sesi orang lain; export evidence bila perlu:
  `docs/rea_evidence.json`. Klaim tanpa `evidence_id` = inferensi.
* DILARANG mengeksekusi `gta_sa.exe` / `samp.exe` / DLL target. Analisis statis saja.

## 3. Clean-room & gaya kode

* C++17, tanpa DX/BASS SDK (dynamic-load via `LoadLibrary`/`GetProcAddress`).
* Tanpa inline ASM x86 — alamat GTA sebagai `constexpr uintptr_t` + guard
  `#ifdef _WIN64` (inert di x64, aktif di x86).
* Tandai yang belum terbukti dengan `TODO(rea): ...`.
* Alamat/konstanta magis dari binary (mis. XOR `0x83433`, magic join `0xFDE`,
  handle `0xFF000000`, packet 200–212) wajib pakai nama konstanta + komentar alamat.

## 4. PROGRESS.md (wajib update tiap perubahan besar)

* Perubahan besar = modul baru/selesai, temuan REA baru, build/test status berubah.
* Update tabel area + overall + tambah 1 baris History. Hitung ulang overall
  dengan bobot di `PROGRESS.md`. Jangan naikkan % tanpa bukti (build log / test / evidence).

## 5. Git + GitHub (wajib push tiap perubahan besar)

* Remote: `origin` → GitHub (dibuat via `gh`). Branch kerja: `main`.
* Alur baku per perubahan besar:
  ```powershell
  git add -A
  git commit -m "<area>: <ringkasan> (progress xx%->yy%)"
  git push origin main
  ```
* Pesan commit format: `<area>: <apa>`. Area: `core|net|game|gui|audio|input|util|loader|docs|build`.
* Jangan commit `build/` (sudah di `.gitignore`). Jangan push token/secret.
* Jika `gh` meminta auth, jangan buat repo duplikat — pakai `gh repo view` dulu.

## 6. Komunikasi

* Bahasa: Indonesia, ringkas, faktual. Tanpa superlatif/emoji.
* Setiap jawaban perubahan besar sertakan: apa diubah, % baru, hasil build/test,
  dan batasan/unknown yang tersisa.

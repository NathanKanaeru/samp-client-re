# BOOT SEQUENCE — DL-R1 (ours) vs R5 spec (libsamp SPEC-ABI-001)

Our binary (`samp.dll` 0.3.DL-R1, DllMain `0x100c6780`) follows the same
5-phase shape libsamp recovered for R5 (`SPEC-ABI-001_BOOT_SEQUENCE.md:18-45`,
R5 entry `0x100cbc90`, guard `fcn.100ce430`):

| Phase | R5 (spec) | DL-R1 (ours, observed) |
|---|---|---|
| 1 guard | entry0 / `fcn.100cbb0f` cmdline gate | `FUN_100c5c90` ValidateGame (`-c/-d/...`), early-out if `!validA && !validB` |
| 2 filter+paths | `SetUnhandledExceptionFilter` + `GetModuleFileNameA` | same: filter `LAB_10060430`, module path → `samp.saa` |
| 3 archive+fonts | archive load + `AddFontResourceA` | same: `CArchive::Open("samp.saa")` + `gtaweap3/sampaux3.ttf` |
| 4 hooks+monitor | file hooks + launch-monitor thread, wait entry-state | same: `FUN_10062720/100623f0` + `__beginthread(LAB_100c5150)` wait `[0x00C8D4C0]==7` |
| 5 game+net | pre-game patch/clamp, hook install, D3D/UI, net/session + banner | same: `DetectGtaVersion 0x100a06d0` → `InitGameSystems 0x100a0720` → `"SA-MP 0.3.DL-R1 Started"` |

Key deltas DL-R1 vs R5: version string `0.3.DL-R1`, custom-model pipeline
(`SAMP.img` 47 MB), manager object `0x142` B, archive object `0x935` B,
SAA v2 `SAA2`+120 fake words (shared), packet IDs 200–212 (older RakNet).

Detach (both): reverse-order teardown; on process-termination (`reserved!=NULL`)
only clear hook flags (loader-lock safety) — mirrored in `src/core/dllmain.cpp`.

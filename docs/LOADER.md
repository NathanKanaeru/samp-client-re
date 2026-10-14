# LOADER — samp.exe (browser) analysis + reimplementation notes

Target: `samp.exe` 413696 B (UPX-packed) → unpacked with UPX 5.2.1 to
1148416 B, PE32 entry `0x40EBF0`, sections CODE/DATA/BSS/.idata/.tls/.rsrc.
Unpacked SHA256 `2d90c2a3a9f70423a393f59675bfde67a3648d6a472ce05e82bf06898056946c`.
Work copy kept OUT of this repo at `%TEMP%\opencode\unpacked\` (proprietary).

## Observed (static only, never executed)

* UPX sections: `UPX0` (vsz `0xBE000`, raw 0) / `UPX1` (`0x63000`/`0x62A00`).
* Injection imports (observed): `CreateProcessA`, `WriteProcessMemory`,
  `CreateRemoteThread`, `ResumeThread`, `OpenProcess`, `LoadLibraryA`,
  `GetProcAddress` — classic remote-thread `LoadLibraryA("samp.dll")` shape.
* Strings (observed): `samp.dll`, `\gta_sa.exe`, `CreateRemoteThread`,
  `lists.sa-mp.com`, `/0.3.DL/hosted`, `/0.3.DL/internet`,
  `Mozilla/3.0 (compatible; SA:MP v0.3.DL)`, `SA-MP 0.3`,
  `San Andreas Multiplayer 0.3.DL`, `Enter your nickname/handle...`,
  `Enter new server HOST:PORT...`.
* Stack: Delphi + Indy HTTP (`IdHTTP`, `TIdSocksServer...`, `HTTP/1.0 200 OK`) —
  server browser with master lists + favorites + rcon/server password store.
* REA (Ghidra, 4953 procs / 2778 strings): `samp.dll@0x4E98D0`,
  `\gta_sa.exe@0x4E34F8` — both with **zero data xrefs** (Delphi shortstring
  construction), `procedure_callers(CreateRemoteThread)` empty (import stubs
  not linked). Call sequence NOT recovered statically at this pass.

## Unknown (honest)

* Allocation primitive: `VirtualAllocEx` absent from imports AND strings
  (only `VirtualAlloc`/`VirtualFreeEx` present); no `SuspendThread` /
  `Get/SetThreadContext` / `VirtualProtectEx` either. So the exact
  allocate-then-write step is unresolved: candidates are (a) dynamic
  `GetProcAddress("VirtualAllocEx")` via Delphi loader, (b) code-cave write
  into existing committed pages, (c) `SetWindowsHookExA` path (also imported,
  though likely browser-UI use). Needs deeper disasm of the launcher routine.
* Contrast: `samp-source/launch3/` (debug harness, `launch3Dlg.cpp:211-310`)
  uses thread-hijack (`SuspendThread` + `GetThreadContext` + ESP cave `0x480`
  + `WriteProcessMemory` + `SetThreadContext`) with CLI `-c -h 127.0.0.1
  -p 7777 -n Player` / `-d`. That is a TEST harness, not the production flow.

## CLI contract (observed across launch3 + ValidateGame + browser strings)

```
samp.exe [-c -h HOST -p PORT -n NICK [-z PASS]] | [-d] | [-l ...]
gta_sa.exe gets: -c -h <ip> -p <port> -n <nick> [-z <pass>] | -d
```

`samp.dll` parses `-c/-d/-h/-p/-n/-z/-l` (`FUN_100c5c90`). Our `samp_loader`
implements exactly this contract (see `loader/loader.cpp`).

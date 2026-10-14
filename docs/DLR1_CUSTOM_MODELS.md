# DL-R1 CUSTOM MODELS — the signature DL feature vs 0.3.7/R5

Observed in `samp.dll` 0.3.DL-R1 (REA strings + xrefs; see REVERSE_NOTES.md):

* Patterns: `%s/0x%X.dff`, `%s\0x%X.dff`, `%s/*.dff`, `%s/0x%X.txd`,
  `[warning] Could not validate…`, `Too many textures…`.
* Served via `FUN_1000BB10` through `FUN_100b5900` (model download/validate
  path); names built at runtime (no literal `SAMP.img` in binary).
* Data: `SAMP/SAMP.img` (47 MB) + `SAMP/custom.img` + `SAMP.ide/.ipl`.

Queue states in `game/models.h` (`ModelQueue`, cap 64): Requested →
Downloading → Ready | Failed ("Could not validate"). Full cap in the
reference is per-model-type; 64 here is our conservative ceiling until the
wire download (`ModelRequest` RPC 49) is traced — TODO(rea).

R5/libsamp notes this area as custom-asset pipeline
(`docs/re/samp_custom_asset_pipeline.md`, `custom_modelinfo_static_*`);
DL-R1 deltas (hex-keyed `%s/0x%X.*` names) are DL-only and live here.

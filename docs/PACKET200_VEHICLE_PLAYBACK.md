# PACKET 200 — remote vehicle playback (R5 behavior, DL-R1-compatible shape)

Source: libsamp `docs/re/packet200_remote_vehicle_playback_r5_20260727.md`
(vs `samp_installer.dll` SHA `b72b5…`, base `0x10000000`). Paraphrased here so
our sync code (`src/net/netgame.cpp`, `src/game/vehicle.cpp`) can converge.
All RVAs below are R5 file offsets; DL-R1 handler `FUN_1000a6f0` is the
counterpart to verify by xref.

## State kept per remote vehicle

* Full quaternion `w,x,y,z` + target position + target move-speed; exact speed
  applied (no `ApplyMoveSpeed`/opcode `0175`/`UpdateRwFrame` calls).

## Per-packet deadband + snap (position)

* Deadband `0.05` per axis; snap when `|X|>8.0` or `|Y|>8.0`, Z `0.5`
  (`2.0` for heli/boat/plane).
* Correction `error*0.06` when `>0.05`; write when result `>0.01`.

## Per-frame attitude

* Turn clamp `[-0.02,+0.02]`; Slerp `t=0.75` + normalize; write 3 matrix axes.

## Keys / horn / siren

* Keys from remote offsets `+0x1B(LR)/+0x1D(UD)/+0x1F(keys)` via pad setter
  (R5 `+0xAF340`); trains skip attitude but keep key path.
* Horn: `CVehicle+0x514 = (pad idx18 != 0)` (R5 `+0xAE270`).
* Siren: preserve bits 0..6 of `CVehicle+0x42D`, set bit7 from wire (R5 `+0xB7540`).

## TODO(rea)

Trace `FUN_1000a6f0` in our DL-R1 binary and confirm field offsets match
before marking `src/game/vehicle.cpp` playback complete.

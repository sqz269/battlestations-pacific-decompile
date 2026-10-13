# Segment 83: render command queue and D3D9 renderer helpers (cc13-seg83)

Segment 83 is `00B1BF40..00B25D90`: the render command queue (mode, worker start, world
labels), its label and batch locks, the render batch pool, the D3D9 renderer's small device
wrappers and the renderer's record-array push backs. Names are descriptive hypotheses.

Status: *reconstructed* in `src/render_queue_seg83_cc13.cpp` /
`include/bsp/render_queue_seg83_cc13.hpp` (namespace `bsp::cc13_seg83`) and *build-tested*
(`scripts/build.ps1`). Nothing is fixture-tested, ABI-compatible or game-validated. Objects
are image layouts; image routines and COM slots are host virtuals named by address or by the
IDirect3DDevice9 slot, nothing is stubbed and no global is invented. The shared primitives
(NativeString release, ref slots, the lifetime lock) come from `bsp::cc13_terrain`
(docs/CC13_TERRAIN_1.md).

## Triage

At `origin/main` 34f1082ca the segment held 61 functions with a ledger name, no record and
no tag. Four already had reconstructions in `scene_renderer_cc13` (`00B1E6D0`, `00B1EF60`,
`00B1F4D0`, `00B1F5E0`, packet cc13_seg83_queue) and got backfilled records. 55 are new here
(scope `table_model` / `shared_shape_model` for the identical bodies). Two are open.

## Notes

* `00B1C460` calls the renderer's +11Ch only on the transition from mode 2 to mode 0; any
  other change just stores +20h. `00B1C4B0` leaves mode 2 the same way, clears the mode, then
  runs the renderer's +0Ch and +14h (begin / end frame). `00B1C4A0` is a tail jump to the
  renderer's +118h (`00B21740`), whose three stack arguments the target cleans up (RET 0Ch).
* The label lock `[00F8D43C]` is created by a double-checked getter (`00B1CCD0`, lifetime lock
  with the depth counter, 00B1CD15 / 00B1CD6D). `00B1ECA0` enters the lock's own section and
  bumps its depth counter at cs+18h (00B1ECDE) around the label push.
* COM slots: `00B1FF20` is IDirect3DDevice9::ShowCursor (+30h) and `00B201D0` is ColorFill
  (+8Ch) with a null rectangle. The six `00B20E90` bodies return AddRef() - 1 after a matching
  Release (the reference count); the three `00B1F7F0` bodies release and clear.
* `00B215D0` (renderer slot +84h) creates a system-memory offscreen surface from its first
  three arguments and constructs the 34h wrapper with (surface, first argument, 0)
  (00B2163B reads [esp+24h] = the first argument). Its fourth argument is never read.
* `00B214E0` hands the back buffer to the target's +14h and does not release the reference
  GetBackBuffer added (the target owns it).
* The fourteen record push backs share one body: doubling growth (1 below 2) through a named
  reserve, element copied as dwords; element sizes 4, 0Ch, 10h, 14h per address.

## Open items

| Address | Ledger name | Why |
| --- | --- | --- |
| 00B20010 | D3D9Renderer_HasAdapterMode | the width/height comparison against the enumerated D3DDISPLAYMODE sits in stack slots the decompile drops; needs a listing trace |
| 00B1F3C0 | RenderCommand_CreateAndQueueInstanceGroup | unreferenced; command construction with a separate owner and an instance-group push |

## Handoff

59 of the 61 targets have records (55 new, 4 backfilled); two are open (above). The records
script is `local/s83_rec.py` in the cc13-seg83 tree (re-runnable, lease-checked).

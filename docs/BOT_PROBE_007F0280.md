# `007F0280`, the bot probe: the contract, its eighteen callers, and what each host stands in with

Packet `cc8_bot_probe_007f0280`, owner `agent/cc8-plane-squadron`.

**Status: NOT READ. This document is the contract and the caller census, not the routine.** It is
written first and on its own because eighteen bot state ticks depend on this routine and at least
three hosts currently substitute it; the next worker should start from the facts below rather than
re-deriving them, and should not treat anything here as a reading of the body.

## 1. ABI, established

| property | value | evidence |
| --- | --- | --- |
| body | `007F0280`-`007F0B21` | `bsp.py ghidra proto` |
| return | `RET 0x18` at `007F0B1F` | disk bytes: `007F0B19 ADD ESP,0x110` then `007F0B1F RET 0x18`, `INT3` padding from `007F0B22` |
| stack arguments | **six** | from the `RET 0x18` cleanup, not from counting pushes |
| receiver | `ECX` | `009D0CB0 MOV ECX,[EAX+4]` at the geometry-update site |
| frame | `0x110` bytes | the matching `ADD ESP,0x110` |
| exceptions | **SEH-registered** | the decompiler shows `local_4 = 0xFFFFFFFF`, `puStack_8 = &LAB_00C8F35B`, `local_c = ExceptionList` at entry |

`docs/BOT_TASK_STATES.md` row 356 records the shape as `unit / in, out, out, out, flag / void`,
which is five stack arguments. **The `RET 0x18` says six.** Checklist rule 7 - the argument count
comes from the cleanup, not from the pushes you happened to list - so the existing row is the one to
doubt. Resolving that is the first thing the reading packet should do.

## 2. What the entry does, from the decompiler only

Not a reading of the listing, and flagged as such:

* everything is guarded on `param_2 != 0`;
* `param_4[0..2]` and `param_5[0..2]` are **zeroed** before anything else - two out-triples;
* `param_3[0..2]` is copied into locals - one in-triple;
* if `*(char *)(param_2 + 0x10C) == 0`, it calls `BSP_EntityPose_RefreshWorld`, sets that byte to 1,
  and calls the orthogonal-scaled-affine-inverse builder.

Its callees are matrix copies and multiplies (`004134F0`, `00413920`), an affine point transform, a
pose refresh (`00414DB0`) and an orthogonal scale build, which is why it reads as **a probe point
placed in the unit's own frame rather than a terrain query**. That is a hypothesis from the callee
list, not a result.

## 3. Every caller, exhaustive over rel32

`python tools/callsite_census.py 0x007F0280`, **eighteen** sites (the brief said sixteen):

| call site | containing function |
| --- | --- |
| `007B4B22` | `007B48E0` |
| `007B505D` | `007B4F60` |
| `009A3805` | `BSP_BotStateDepthChargeAttackRun_Tick` |
| `009A3D95` | `009A3CF0` |
| `009A4122` | `BSP_BotStateDepthChargeAim_Tick` |
| `009A72DD` | `009A71E0` |
| `009B0632` | `009AFFF0` |
| `009B4DC1` | `009B4D00` |
| `009B580F` | `009B5760` |
| `009B5F17` | `009B5C80` |
| `009C42B8` | `BSP_BotStateDiveBombAttackRun_Tick` |
| `009C4878` | `009C47D0` |
| `009C6B3A` | `BSP_BotStateDiveBombFlyAbove_Tick` |
| `009CAA12` | `009CA870` |
| `009CAEAD` | `009CADB0` |
| `009D0857` | `BSP_BotStateTorpedoAttackRun_Tick` |
| `009D0CBC` | `BSP_BotStateTorpedoGoAway_UpdateGeometry` |
| `009D1A94` | `BSP_BotStateTorpedoAim_Tick` |

`009FD570` is **not** a caller, which matters because this stream reached `007F0280` through
`009D0C10` and could have assumed the two travel together. They do not.

## 4. The call sites whose constants are known, and they differ

| site | extents pushed | final argument |
| --- | --- | --- |
| `009C42B8`, dive-bomb attack run | 80, 60, 120 (`009C4258`, `009C4268`, `009C4287`) | **1** |
| `009D1A94`, torpedo aim tick | `{72t, min(0.7 * 72t, 150), 1.5 * 72t}` - a **computed** triple scaling with the time metric | - |
| `009D0CBC`, torpedo goaway geometry | **60** (`00CEB4B0`), **50** (`00CEB4D4`), **90** (`00D1A918`) | **0** |
| `009D0857`, torpedo attack run | `00CE5444` = 80 among them | **0** |

So the triple is a **per-caller extent**, sometimes authored and sometimes computed, and the final
argument is a per-caller mode that the dive bomb sets and the torpedo states clear. Any
reconstruction that hard-codes the triple is wrong for fifteen of the eighteen sites.

## 5. What each host stands in with today

| caller | host | stand-in |
| --- | --- | --- |
| `009C42B8` | `src/game_hosts_units.cpp:4533` | `in.sampler_result = 0.0f`, labelled SUBSTITUTION; `src/dive_bomb_task.cpp:412` then makes `lateral_offset_20 = -sampler_result * ...`, so **the dive-bomb run-in flies straight at the target instead of weaving**. Its owner has this labelled at the call site and is waiting on this contract |
| `009D1A94` | `src/game_hosts_units.cpp:5411` | `sector_probe_009d1a94` returns a default `TorpedoAimSectorProbe{}`, so the aim tick's obstacle arm is inert |
| `009D0CBC` | not bound | the goaway geometry update is not reconstructed at all |
| the other fifteen | - | not surveyed |

**One substitution is provably safe and should stay.** At `009D0CBC` the three out-slots are zeroed
by the caller itself (`009D0C96`-`009D0CAA`, `XORPS`/`MOVSS`) immediately before the call, and the
caller's use of them is `probe = -p[0] * p[1] * p[2]` feeding a strict sign test at `009D0D50`. A
no-hit answer leaves `probe` at zero and the test cannot fire, so an inert probe is exactly faithful
there. `docs/TORPEDO_AFTER_THE_DROP.md` section 3.5 has the argument.

## 6. What the reading packet has to produce

1. The `RET 0x18` versus five-argument contradiction in section 1, settled from the listing.
2. What the six arguments are, with the receiver's identity: `param_2` carries a byte at `+10Ch` that
   gates a pose refresh, which is **not** the `+C8h` a plane unit uses, so the receiver may not be
   the unit at every site.
3. The meaning of each element of the two out-triples, and of the in-triple, read at two sites with
   different constants so the meaning is not inferred from one.
4. The return convention: `void` per `docs/BOT_TASK_STATES.md`, but three callers consume a float
   (`009C42B8`'s `sampler_result`, `009D1A94`'s probe, `009D0CBC`'s triple product) - check whether
   any leaves something in `ST0`.
5. A name, a ledger record with evidence, and a pure function in a shared header of this lineage.
6. This table's fifteen unsurveyed callers filled in, so each owner can replace its own stand-in.

## Coverage

| item | coverage |
| --- | --- |
| ABI and frame | complete from the cleanup and the disk bytes |
| the caller census | complete, exhaustive over rel32 |
| four call sites' constants | complete |
| the host stand-ins | **partial**: three of eighteen |
| the body | **not started** |

## Uncertainty

Everything in section 2 is decompiler output, not a listing read, and the routine carries SEH, which
is the case where a decompiler's frame reasoning is least trustworthy. Nothing in section 2 should
be built on.

# `007F0280`, the bot probe: the contract, its eighteen callers, and what each host stands in with

Packet `cc8_bot_probe_007f0280`, owner `agent/cc8-plane-squadron`.

**Status: the body is now READ for its structure, its six arguments and its rejection rule; see
section 0. The per-axis accumulator arithmetic at `007F06AF`-`007F0916` was read for its gates and
its sign rule only. Sections 1-6 below are the original contract document and are kept as written,
with section 1's argument table corrected in section 0.**

**Original status line, retained:**
**This document is the contract and the caller census, not the routine.** It is
written first and on its own because eighteen bot state ticks depend on this routine and at least
three hosts currently substitute it; the next worker should start from the facts below rather than
re-deriving them, and should not treat anything here as a reading of the body.

## 0. The body, read. It is a near-field unit-vs-unit avoidance box

Read by packet `cc8_dive_geometry` (`agent/cc8-dive-bomb`) from the listing, ESP anchored on the
prologue and propagated forward only across pushes whose cleanup is visible in the same block.

### 0.1 The six arguments, corrected

The prologue pushes 12 (the SEH triple) + 260 (`SUB ESP,0x104`) + 8 (`PUSH EBP`, `PUSH ESI`) = 280
= `0x118` before `007F029D` reads arg0 at `[ESP+11Ch]`. **`007F02A4 PUSH EDI` then makes the depth
`0x11C` for the whole rest of the routine**, which is where
`docs/HANDOFF_DIVE_BOMB_PROBE.md` section 2's table goes wrong: it assigns indices at the entry
depth and so calls arg1/arg2/arg3 by the names arg1..arg3 one slot early and calls arg5 "arg4".
`RET 0x18` = six is right; the table is not.

| arg | slot at depth `0x11C` | first read | what it is |
| --- | --- | --- | --- |
| 0 | `[ESP+120h]` | `007F029D` (as `[ESP+11Ch]`, depth `0x118`) | the unit whose frame the box sits in. NULL-checked; byte `+10Ch` gates the `00414DB0` pose refresh and the `00B63D50` inverse build; matrix at `+110h` |
| 1 | `[ESP+124h]` | `007F02E9`, and as `EBX` at `007F055A` | **the half-extents triple, in** - the 80/60/120 the dive-bomb attack run passes by pointer, never pushed |
| 2 | `[ESP+128h]` | `007F02B8` | out-triple A, zeroed at `007F02BF`-`007F02C9` |
| 3 | `[ESP+12Ch]` | `007F02CD` | out-triple B, zeroed at `007F02D4`-`007F02DE` |
| 4 | `[ESP+130h]` | `007F06AF` (as `[ESP+134h]`, depth `0x120`) | a per-axis **weight** triple, in. Each of the three arms is skipped unless its component is `> 0`; the dive-bomb caller passes `(0,0,0)` and so takes 1.0 on all three |
| 5 | `[ESP+134h]` | `007F038D` (as `[ESP+138h]`, depth `0x120`) | the **byte mode**, `CMP ... ,0`. The first thing pushed at every call site, hence the last argument |

At `009C42B8` the four triples are one contiguous block of the caller's frame - `[ESP+10h]` the
weights (zeroed by the caller at `009C429A`-`009C42A6`), `[ESP+1Ch]` the extents (80/60/120 stored
by `MOVSS` at `009C4262`/`009C4281`/`009C4294`), `[ESP+28h]` out B, `[ESP+34h]` out A - which is why
counting pushes gave five: three of the six arguments are pointers into that block and the extents
are stored, not pushed.

### 0.2 What it iterates

* **mode != 0** (`007F039F`): the entity list at `arg0+C50h`, `begin` at `+30h` and `count` at
  `+34h`, keeping every element whose `vtable[5Ch](15)` returns true (`007F03D8`, the only indirect
  call in the routine). `unit+C50h` is the same object `007DF360(unit+C50h, target, point)` answers
  reachability on - `docs/TORPEDO_RUN_IN_PATH.md`.
* **mode == 0**, and **mode != 0 with a null `+C50h`** (`007F0481`): `this+3D0h` for `this+3CCh`
  entries, `this` being `ECX`, skipping the element equal to arg0 - itself.

Either way the survivors go into one heap vector (`operator new` at `007F0385`, grown `2n+2` at
`007F03EE`, freed at `007F0B00`).

### 0.3 The test, and why it cannot see a ship

Each candidate's pose is refreshed if `+C8h` is clear, its matrix composed (`00413920`/`004134F0`),
and its position at `+FCh` transformed into the probe frame by `004142E0`. Then three rejections,
`007F05F7`-`007F06A9`, each `FCOMIP` on the extent against the masked-sign magnitude with a `76`
JBE:

```
reject unless |p.x| < extents[0] && |p.y| < extents[1] && |p.z| < extents[2]
```

**It is an axis-aligned box of the given half-extents, centred on arg0.** For the dive-bomb attack
run that box is 80 x 60 x 120 m. A ship 1 to 10 km ahead is never inside it, and neither is anything
else over open sea; the only things that ever are, are other aircraft in close formation. The sign
of each axis's answer is tie-broken on the integer at `entity+9D0h` compared between candidate and
self (`007F0792`, `007F07E3`, `007F082F`), which is what makes two aircraft avoid to opposite sides.

### 0.4 The zero is a proof at `009C42B8` too, for almost every tick

`007F0936` tests the byte at `[ESP+1Bh]`, set to 1 at `007F06C0` only by a candidate that survived
all three rejections, and jumps past **the entire output block** to the epilogue when it is clear.
With nothing inside the box both out-triples keep the zeros written at `007F02B5`-`007F02DE`, so the
attack run's `-out_a[0] * out_b[1] * out_b[2]` (`009C42BD`-`009C42C7`) is exactly 0 and
`lateral_offset_20` is exactly 0.

So section 5's "**the dive-bomb run-in flies straight at the target instead of weaving**" is right
about the behaviour and wrong about the fault: the image flies straight at the target too, whenever
no other unit is within 80/60/120 m. `in.sampler_result = 0.0f` is a **proof** for those ticks and a
**hole** only in close formation. `docs/HANDOFF_DIVE_BOMB_PROBE.md`'s closing line, that the bomber
"ditches for want of a weave", is withdrawn: a lateral offset of at most 30 degrees
(`00CEC730` = 0.5236, `009C42D3`) applied on the run-in cannot explain a dive entered 470 m past the
target, and the run-in is not where the dive-bomb task fails. See `docs/DIVE_BOMB_TASK.md` on
`009C4F80`.

### 0.5 Return convention, settling section 6 item 4

**Void.** `007F0A21` takes arg2 into `EAX` and writes its three components at `007F0A64`/`007F0A6A`/
`007F0A71`; `007F0A96` takes arg3 into `ECX` and writes its three at `007F0AA1`/`007F0AC5`/
`007F0AE0`. Nothing is left in `ST0` by intent - the three callers that "consume a float" all read
it out of the out-triples. The x87 stack balance across the two exit paths was not walked
instruction by instruction; that is the one piece of this section taken on the shape rather than
proved.

### 0.6 What is still not read

The per-axis accumulator arithmetic between `007F06AF` and `007F0916`: the running per-axis maxima
in `[ESP+44h..4Ch]` and `[ESP+50h..58h]`, the weighted `1 + w*|p|/e` factors the `arg4 > 0` arms
build, and the sums `007F0A28`-`007F0A5C` that become out-triple A. The gates, the rejection rule,
the tie-break and the zero-output proof above do not depend on it, but a faithful pure function
does.

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

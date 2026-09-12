# The dynamics world's construction and its step settings

Addresses: 004DDB90, 00C420E0, 00C41AD0, 00C5C540; read as contracts 00875BB0, 00C55F50,
00C5BB30, 00C41550, 00C5B1B0.

Packet `cc_ship_motion_tail`, 2026-09-11. Reconstructed in
`include/bsp/dyn_world_settings.hpp` and `src/dyn_world_settings.cpp`; semantic C++
interfaces for MSVC Win32, not drop-in binary replacements. Descriptive names are
hypotheses, not recovered symbols. The saved project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. This worker made no Ghidra mutation; the ledger records the
new names.

`docs/RIGID_BODY_INTEGRATION.md` left `world+00h` unestablished and wrote: "If `world+00h`
is `0.05f` the loop runs zero times and the remainder branch takes one substep of the full
step; if it is smaller, several substeps run and only the first sees the commanded force."
It is `0.05f`. One substep of the full step.

## Where the world comes from

`004DDB90` is the game object's constructor. It publishes itself at `DAT_00E188A8`
(`004DE105`), builds a `40h`-byte world descriptor on its stack based at `ESP+1Ch`
(`004DE176`), takes the Dyn engine from `00C55F50` and stores it at `game+14h` (`004DE194`),
calls the world factory with that engine in `ECX` (`004DE1D3`), and stores the result at
**`game+18h`** (`004DE1DB`). That is the pointer `00875E06` loads once per fixed game step
before calling the substep schedule at `00875E0C`.

`00C420E0` is the factory, `__thiscall(engine, const desc*)`: `operator_new(0x48C)` at
`00C420F8`, `00C41AD0` at `00C4211B`, then the world is appended to the engine's vector. The
world object is therefore `48Ch` bytes.

`00C41AD0` is `__cdecl world*(world*, const desc*)` (`00C42119` pushes the descriptor,
`00C4211A` the world). Its fifteen copies at `00C41AF2..00C41B4A` are the whole of the
descriptor's effect on the world.

## The descriptor and the fields it feeds

The descriptor's stores are split around the `PUSH ECX` at `004DE191`, so a linear read of
the listing sees two displacements for one block. Resolved:

| desc | store | value | world | reader |
| --- | --- | --- | --- | --- |
| `+00h` | `004DE168` | `0.05f` (`00CE7638`) | `+00h` | `00C5C540` at `00C5C651` |
| `+04h` | `004DE1B5` | `0.0f` | `+04h` | gravity x, `00C41550` |
| `+08h` | `004DE1BB` | `-10.0f` (`00CE6848`) | `+08h` | gravity y |
| `+0Ch` | `004DE1C1` | `0.0f` | `+0Ch` | gravity z |
| `+10h` | `004DE1AB` | `1` (EDI, int) | `+34h` | `00C5C540` at `00C5C63C` |
| `+14h` | `004DE1C7` | `0.0f` | `+3Ch` | sleep linear speed, `00C5B1B0` |
| `+18h` | `004DE1CD` | `0.0f` | `+40h` | sleep angular speed, `00C5B1B0` |
| `+1Ch` | `004DE197` | `20` (imm `14h`) | `+44h` | sleep countdown reload, `00C5B1B0` |
| `+20h` | `004DE19F` | `0` (EBX) | `+10h` | unread |
| `+24h` | `004DE1A3` | `10` (imm `0Ah`) | `+38h` | unread |
| `+28h` | `004DE1AF` | `0.0f` | `+14h` | unread |
| `+2Ch` | `004DE17A` | `0.1f` (`00D7A2F0`) | `+18h` | unread |
| `+30h` | `004DE13E` | `0.85f` (`00CE7480`) | `+1Ch` | unread |
| `+34h` | `004DE14C` | `1.0f` (`00D7A24C`) | `+20h` | unread |
| `+38h` | `004DE15A` | `0.02f` (`00CE746C`) | `+24h` | unread |
| `+3Ch` | `004DE183` | `0.5f` (`00CE3800`) | `+28h` | unread |

The eight "unread" rows are read by the collision, group and LCP phases, which this packet
does not cover; they are carried in `DynWorldDescriptor` by offset with no contract so a
later packet can name them without re-reading the constructor. `10` iterations, `0.85`,
`0.1` and `0.02` are the shapes of an iteration count, a relaxation factor, a penetration
allowance and a slop, but that is a guess and the header says so.

## What the substep schedule then does

`00C5C540`'s loop, with `world+00h = 0.05f`, `world+34h = 1` and the `dt = 0.05f` that
`00875E09` pushes (the float at `00D0DE84`):

```
accumulator  = world+48h + dt          ; 00C5C639..00C5C64E, accumulator was left at 0
budget       = world+34h               ; 00C5C63C, an int in a stack local
if (world[0] < accumulator) { ... }    ; 00C5C65D FCOMIP / 00C5C65F JBE -> not taken
if (budget != 0 && accumulator > 5.0e-5)   ; 00C5C698, 00C5C6B2 against the double 00D7A398
    00C5BB30(world, accumulator);      ; 00C5C6BD, one substep of 0.05 s
accumulator = 0                        ; 00C5C6C9
```

`0.05f < 0.05f` is false, so the loop body never runs and the remainder branch takes the
whole step in one substep. Two consequences for `docs/RIGID_BODY_INTEGRATION.md`:

* **The commanded force is integrated by the whole game step, not by a fraction of it.**
  That doc's warning that "only the first substep sees the commanded force" is real for a
  smaller substep, but the shipped world does not have one.
* The damping, the clamps and the sleep countdown all run once per game step, not several
  times. The sleep countdown at `world+44h` is 20, so a body that is exactly at rest sleeps
  after one second.

## Coverage

| routine | state | coverage |
| --- | --- | --- |
| `00C41AD0` | the descriptor copy reconstructed | partial: `00C41B4D..00C420DB` builds the pools, the broad phase and the task lists and is not read |
| `00C420E0` | reconstructed as allocate-construct-append | complete for the factory's own body |
| `004DDB90` world block `004DE13E..004DE1DB` | reconstructed | partial: the rest of the game constructor is not this packet's |
| `00C5C540` | the schedule re-read to settle the budget's type | as in `docs/RIGID_BODY_INTEGRATION.md`, plus the correction below |

## Correction from docs/DYN_WORLD_RUNTIME.md (2026-09-12)

The original `00C41AD0` ends with `RET 8` at `00C420D9`, so its two-stack-argument
ABI is `__stdcall`, correcting the earlier `__cdecl` hypothesis above. The new
`dyn_world_storage_construct_00c41ad0` covers the complete normal fresh successful-
allocation storage sequence, including all three pools, scene, four task vectors
and the late shared motion. Its four original-byte fixtures matched 118 owned
buffers. The existing settings projection remains partial; the new full storage
interface still requires initialized engine/dispatch/task owners and does not
claim native exception ABI, task execution or game physics validation.

## Corrections

**To `docs/RIGID_BODY_INTEGRATION.md`, "`world+00h` itself is not established".** It is
`0.05f`, from `00CE7638` through `004DE168` and `00C41AF2`. `00424A10` was correctly ruled
out as its producer; the producer is the game constructor's stack descriptor.

**To `docs/RIGID_BODY_INTEGRATION.md`, the substep pseudocode, and to
`dyn_world_substep_plan_00c5c540` in `include/bsp/rigid_body_integration.hpp`.** Both treat
`world+34h` as a float decremented through an integer cast. It is a plain `int`: `00C5C63C`
`MOV EAX,[EDI+34h]`, `00C5C661` `CMP dword ptr [ESP+10h],0`, `00C5C674` `SUB dword ptr
[ESP+10h],1`. The float projection reproduces the same sequence for every integral value
passed **by value**, which is why the existing function is left alone and
`dyn_world_substep_plan` converts; it would diverge for a caller that passed the field's raw
bit pattern, since the shipped field holds the integer `1`, whose float reading is
`1.4e-45`. The routine also copies the budget into a stack local before the loop, so the
world's own field is never decremented.

**To `docs/RIGID_BODY_INTEGRATION.md`, "the world fields the phases read".** It lists
`world+3Ch` and `world+40h` as sleep speed thresholds without values. Both are `0.0f`, so
the sleep test `threshold^2 < speed^2` is passed by any non-zero velocity at all: only an
exactly stationary body ever counts down.

## Follow-up packets

| packet | addresses | what it answers |
| --- | --- | --- |
| `dyn_world_solver_settings` | `world+10h`, `+14h`, `+18h`, `+1Ch`, `+20h`, `+24h`, `+28h`, `+38h`; readers inside `00C5BB5F..00C5C455` | what the eight carried fields mean. Overlaps `dyn_contact_solver` |
| `dyn_engine_00c55f50` | `00C55F50`, `game+14h` | the Dyn engine object the world hangs off, and whether more than one world is ever created |

## no_ghidra_function

none. Every address named or reconstructed in this packet lies inside an existing Ghidra
function body, checked with `python tools/bsp.py ghidra proto <addr> --brief`.

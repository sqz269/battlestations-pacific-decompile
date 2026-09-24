# Render-resource initialization entry fragment (CC10)

Addresses: `00B107F0` (entry fragment only), `00B21F10` (complete leaf).

This packet reconstructs the first stage of `B107F0`, stopping **before
`B109BC`**. It also completes the original already-initialized early-return
branch. It does not initialize the resource graph, write service `+70`, or
authorize `B0F6E0`/`B14F60` destruction. In particular, the native `+1C4=1`
store at `B10904` precedes most member construction and cannot be used as a
source completion token. Application binding remains deferred.

| Routine | Native boundary / ABI | Coverage |
| --- | --- | --- |
| `B107F0` | Full observed body `[B107F0,B13029)`, 10,297 bytes; ECX service, three stacked DWORDs, `B13026 RET 0C` (3 bytes) | **Partial entry fragment** `[B107F0,B109BC)` and existing-init tail `[B13009,B13029)`; `[B109BC,B13009)` excluded |
| `B21F10` | `[B21F10,B21F28)`, 24 bytes; ECX renderer, stacked output, EAX output; `B21F25 RET 4` (3 bytes) | Complete normal leaf; new C++ interface |

Live Ghidra project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, was checked by BSP wrappers. Every byte of both
observed full bodies matched the installed PE. The report includes hashes,
all four direct caller receipts, the 11 covered CALL sites, and an explicitly
excluded inventory of the rest of the full function's 471 CALL instructions
(36 distinct direct targets). No Ghidra annotations were changed. `B21F10`
was not a defined function during recovery: inclusive range
`B21F10..B21F27`, final instruction `B21F25` size 3.

## Inputs and producer evidence

The three input cells deliberately have address-order names. Native callers
do not agree about the first and third inputs. Each uses current `F8D39C`
as ECX; the native callee pops 12 bytes.

| Call site / containing function | word 0 | word 1 | word 2 |
| --- | --- | --- | --- |
| `4DF3BB` / `4DE610` | DWORD `F889D8` | zero-extended byte `F88A0D` | zero-extended byte `F88A0C` |
| `5045A9` / `504130` | zero-extended byte `F88A0C` | zero-extended byte `F88A0D` | DWORD `F889D8` |
| `67F2AA` / `67F080` | zero-extended byte `F88A0C` | zero-extended byte `F88A0D` | DWORD `F889D8` |
| `8D60E6` / `8D5B50` | settings ESI `+58` DWORD | settings `+8D` byte | settings `+8C` byte |

The covered stage loads only the low byte of word 1 at `B1080B`, before its
first flag sample. The source borrows the three argument cells and retains
their identity for subsequent stages; it does not pre-read the other cells.
These model the native stacked argument cells, not the caller's original
global/settings locations.

`B14A10` already constructs the initial `+1D4` 40h frame owner with
`B1FBB0`, sets color zero through current renderer slot `+128` / `B24DC0`,
and depth through slot `+12C` / `B20090`. This stage reuses those same
published owner and surface domains. `NativeFrameTargetOwnerStorage` and
`NativeSurfaceOwnerStorage` are the existing types; no second owner,
reference count, string pool, surface pool, or renderer publication is added.

`B21F10` reads renderer `+1A28` and `+1A2C`, captures both, then stores
two DWORDs through the supplied output and returns that output. These are the
actual D3D presentation width/height cells produced by the existing
`B2AEB0` device-startup provider. The renderer constructor leaves that block
unwritten. Original profile `D5F0A8+80` (`D5F128`) contains `B21F10`;
the live adjacent getter slots contain `B24DC0` and `B20090`. The leaf's
data xref is its profile slot; generic indirect caller coverage is not claimed.

## Covered ordering

The source captures the initial `+1C4` test, writes the captured input byte
to `+219`, then takes the captured branch. With the initial flag nonzero it
calls `B50010` on current nullable `+20`, reloads `+1C4`, and calls
`B4ECC0` on current nullable `+30` when still enabled. That branch reaches
the original return through `B13009..B13028` without allocating anything.

Fresh entry captures old `+1D4` before reading the current `CE2220`
decrement import. A zero result dispatches the old frame's current slot zero
and refreshed deleting slot, using the existing `BD30E0` / `B1FCF0`
concrete profile. Only after that call returns does it clear current `+1D4`,
overwriting any returning callback's edits. It allocates 40h through the
shared CRT service, invokes `B1FBB0`, and publishes the result. Native null
allocation is not converted to success or an alternate owner.

It separately reloads current `F8D394` and its profile for color and depth,
reloads `+1D4` after each getter, and uses the existing `B1FAB0` / `B1FB00`
assignment implementations. Those publish and retain incoming surfaces before
releasing a captured outgoing reference. Dispatch accepts the concrete
`D5F0A8` renderer profile and `D5E600` old-frame profile only; current
profile mismatches are source diagnostics, never numeric host-vtable calls.

The four source scalar words at `+294,+290,+28C,+298` are all loaded
before writing the three bytes `+1C4,+218,+B0=1` and destinations
`+628..+634`. A second captured group from `+2A4,+2A0,+29C,+2A8`
fills `+638..+644`. These are raw MOVSS bit copies, with no float conversion.
Current renderer dimensions are then fetched via slot `+80`. The alignment
uses the exact signed remainder-by-eight idiom, rounding signed negatives
toward zero, not simply clearing three bits.

## Continuation and lifetime boundary

`NativeRenderResourceInitEntryState` is a caller-owned observation and
continuation record. At `awaiting_b109bc_continuation` it retains:

- actual service and argument-cell identities; native EH state `-1`;
- captured old-frame EDI identity, possibly already freed, and the allocation
  identity from native `ESP+20`;
- original dimensions at native `ESP+18/+1C`, aligned height at `ESP+24`,
  aligned width in EAX, signed remainders in ECX/EDX, and the last four
  scalar low words from XMM0..3.

At this frontier ESI is the service, EBP is zero, and EBX is `FFFFFFFF`.
The width spill at `B109BE` has **not** executed. This is not a resumable
native stack or FH3/SEH frame; EFLAGS, XMM upper lanes and stack aliases are
not represented. The source exposes a new C++ interface. It must not replay
a used state or treat the fresh-path frontier as a successful return. There
is no automatic rollback/free or destruction admission on scope exit.
Escaping source diagnostics retain the published attempt state and are not
a recovered native exception-cleanup contract.

The first excluded operations are `B109BC PUSH 18`, `B109BE`'s width
spill, `B109C2 BF681B`, and `B109EE B4E020`; the latter receives original
width/height, format `15`, multisample zero, mode zero, external surface
null, and returns with `RET 18`. It publishes `+4C` at `B10A00`.
The first observed `+70` write is much later at `B11563`, after `B11555`
calls `B4E470` on a temporary name constructed at `B11535`. None of those
operations belongs to this implementation.

## Next provider composition

The old BM document is a historical dependency inventory. Current source
already exposes `B4E020` (`native_render_texture_surface_owner`), surface
factory `B2A7C0` and resize `B0FC10` (`native_render_resources_surfaces`),
`B4E470` (`native_post_effect20_construction`), material parameter/texture
helpers, and the pass/downscale/bright/luminance/bloom/distortion/dust
initializers. The report maps observed direct targets to current headers.
These declarations are discovery evidence, not an audit that all contexts
can yet be composed for this function. No matching source entry for
`B4E2B0` or `B54CD0` was found in the bounded header/source search;
`B18AC0` is represented inline by an established parameter-provider call.

The next packet must retain actual acquired texture/surface/material owners
and their canonical registration contexts, then compose the next constructors
in native order. It must audit those leaves, every indirect profile, all
input-cell reads and the native EH map before claiming full initialization.
Existing source providers must not be replaced with new caches, callbacks
that always succeed, or raw numeric vtable dispatch.

## Verification limits

The packet uses the strict MSVC Win32 build and existing CTest checks. One
ignored local probe compares the mapped original entry fragment to the
source with explicit controlled bindings for reused callees. It does not
execute the excluded resource constructors or a D3D device. Native control
flow and source use the same existing frame allocation/construction,
assignment/deletion and dirty/root providers; therefore this is evidence
for the covered control/data ordering, not a new differential proof of
those reused leaves. The native fixture jumps at `B109BC` to record the
frontier, pop EDI/EBX and reach the original epilogue.

The fixture covers final/nonfinal old frames, a decrement callback that
changes the current renderer and parent `+1D4`, actual surface reference
counts, positive/negative dimension alignment, all four nullable existing
branch combinations, argument-byte preservation, and overlapping B21F10
input/output. Exact final build/probe outcomes are recorded in the report.
It provides no full graph, application startup, native exception ABI, game
or visual validation. No production application was launched for this packet.

## Primary integration and dimensions leaf definition

Published worker 658df51ce together with the raw Lua reader leaves in main 41d46cd1f after the strict Win32 integration build and all three existing CTests passed. The primary defined the exact 24-byte B21F10 dimensions leaf under the Ghidra write lock, saved the program, retained the mutation record in reports/cc10_lua_renderer_function_definitions.json, and refreshed the export. B107F0 remains the documented entry fragment and existing-initialization tail; +1C4 does not admit complete initialization or destruction.

Primary evidence archives and the separate finite application startup result are recorded in reports/cc10_lua_gui_renderer_integration.json. That application run does not exercise these still-unbound raw stages.

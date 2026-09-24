# Render-resource initialization: B109BC continuation

This packet reconstructs the normal construction phase **[00B109BC,00B11599)**
inside `00B107F0`. It consumes the actual retained entry state from
`native_render_resource_init.hpp`. It creates the initial targets and render
passes, configures the `+80` pipesight post effect, and publishes the result of
the `+70` HDRFinalPass constructor. It stops before the first instruction at
`B11599`, which starts configuring `+70`. It neither finishes initialization nor
admits destruction. The containing function's existing partial name is retained.

The new interface is host C++, not a replacement x86 ABI. `B107F0` takes ECX plus
three DWORD stack cells and eventually executes `RET 0C`; this fragment executes
no native return. Its entry must be the published `awaiting_b109bc_continuation`
state, with the original service, actual argument cells and retained dimensions.
It claims that entry once. Its return is `awaiting_b11599_continuation`, with
the entry marked `awaiting_later_continuation`.

## Evidence and exact boundary

Ghidra was read only, through BSP wrappers which verified
`C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`. The live 3,037-byte
fragment matched the installed PE. Its final included instruction is the
five-byte `CALL BD1510` at `B11594`, inclusive final byte `B11598`. The next
instruction is `MOV ECX,[ESI+64]` at `B11599`. The containing body continues
through the three-byte `RET 0C` at `B13026` (inclusive `B13028`). There are no
missing Ghidra function definitions in this packet.

The report contains every CALL in the fragment and both recovered leaves,
separate all-site leaf receipts, original-byte hashes, relevant producer
receipts, and the retained frontier. The earlier packet remains the evidence
source for the entry, complete early-return branch and all four B107F0 input
callers: `4DF3BB`, `5045A9`, `67F2AA`, and `8D60E6`.

The native FH3 descriptor is `DF3F34` (magic `19930522`, 106 states, unwind map
`DF3F58`, handler `CBC1E1`). States 0 through 30 were inspected for this boundary.
Source exception handling **retains** acquisitions and diagnostics; it does not
claim native FH3 cleanup, private stack aliases, instruction-exact fault order,
or resumability after an exception.

## Ordered production

All offsets below are on the original service. Allocations use the existing
shared CRT provider; fields retain the original profile words and actual `+04`
counts. The source does not zero whole allocations or normalize format tokens.

| Order | Actual production and publication |
|---|---|
| 1 | Three 18h `B4E020` holders at `+4C`, `+38`, `+3C`: original dimensions, format 15h, mode 0. Their allocation/constructor pairs are `B109C2/B109EE`, `B10A03/B10A2F`, `B10A44/B10A70`. |
| 2 | Share current `+4C` into `+40`, using the old pointer captured first. Capture the increment import once at `B10A79`; retain incoming before freshly reading the decrement import for old. |
| 3 | `B2A7C0` at `B10AD4` produces `+50`, original dimensions, format 71h and the original first argument cell as read at `B10ABA`. |
| 4 | Two more real `B4E020` holders at `+44/+48`, mode 1. Each reloads current `+50` after its own allocation. |
| 5 | Current renderer slot 88h (`B2A070`) produces `+54`: signed original dimensions divided by two, one level, format 71h, flags 10h. Current texture slot 30h (`B3FD80`) produces retained surface `+5C`. Share `+50` into `+58` with native retain/release ordering. |
| 6 | Inline 20h D5E164 depth pass `+60`, initialized by `B540B0` from `+3C`, half original dimensions and format 73h. Inline 20h D5E178 particle pass `+64`, initialized by `B542D0` from original dimensions, `+54`, `+60`'s texture and current `+50`. Argument cell zero is reread after particle allocation at `B10C50`. |
| 7 | Real `B4E470` post effects at `+68/+6C`, names `dummy_passtrough.mshd` / `blendparticles.mshd`, mode 3, null explicit material. Register their original float2 parameter pointers at service `+04`. |
| 8 | Inline 220h D5E18C down4 pass `+18` and 90h D5E1A0 down2 pass `+1C`, using `B544F0/B546F0`, `+64`'s holder, and quarter/half **aligned** dimensions. |
| 9 | `B50D40/B51090` produce 250h luminance `+20`, borrowing current CE6650 and `+18`'s holder. Inline 224h D5E1B4 plus `B54940` produce bright pass `+24`, quarter aligned dimensions and format 15h. |
| 10 | `B54E70/B54F90` produce 43Ch bloom `+28`, eighth aligned dimensions, format 15h, input `+24` and CE3854 via FLD/FSTP semantics. |
| 11 | `B4E470` produces `+80` named `pipesight.mshd`. Bind textures from `+44` and the current third bloom holder at `+28+20`, register ten original parameter pointers, then bind color zero from the primary surface of `+4C`. |
| 12 | `B4E470` at `B11555` constructs the mode-3 `HDRFinalPass.mshd` post. Publish its actual returned pointer to `+70` at `B11563`, and return the temporary name through the current string pool. Stop at `B11599`. |

The four post temporary names preserve their native mask bits 1/2/4/8. The
current data pointer is captured before disarming; current length+1 and the
current `419CC0` singleton are used for `BD1510`. No stale header is cleared.
The twelve parameter registrations borrow actual service or bloom field
addresses; they do not snapshot the parameter values. Literal bytes and
registration callsites are enumerated in the report, including the original
`dummy_passtrough`, `cLolka`, and `cBolka` spellings.

Null allocation branches are kept: they publish null where native does, and
later genuine providers retain their own preconditions. `field70_published`
records that store, including a possible null; it is not a success predicate.
The existing bloom initializer's native four-float range at `+430..+43F` exceeds
the nominal 43Ch allocation. This packet preserves that established provider
boundary and makes no extra capacity or production safety claim.

## Canonical owners and retained state

Contexts borrow the same current renderer cell, frame/surface owners, actual
post owners, string pool, material parameter allocator, texture providers and
current decrement import. The entry/frame, pass, downscale, luminance, bright,
bloom and lifetime contexts are checked for shared domain identity. Current
numeric profile entries are compared before dispatching to genuine source
providers; no numeric host-vtable call is made.

The caller owns an immovable continuation state. It retains all five holder
acquisition records, the target factory record, runtime texture/surface records,
seven actual pass allocations, four direct post blocks, nested persistent
construction blocks and sixteen actual string headers. On any exception those
records and all actual service publications remain available. There is no
automatic rollback, retry, free, metadata unbind, or cleanup admission. Prepared
child blocks also require their existing explicit quiescence contract before
being destroyed or reset.

At `B11599`, native ESI is the same service, EBX remains FFFFFFFF, EBP is service
`+A0`, and EDI/ESP14 hold the raw `+70` allocation. ESP20 has become half aligned
width at `B10F13`; it is no longer the earlier allocation spill. ESP58 retains
aligned width, ESP24 aligned height, ESP18/1C original dimensions, ESP1D8 half
aligned height, ESP10 mask zero and EH state -1. The host state records these
values or retains them in the entry; it is not a native machine stack image.

The seven service-visible passes `+60,+64,+18,+1C,+20,+24,+28` still require
explicit stable companions in the canonical actual-owner registry before the
service release dispatcher can retire them. Nested post/frame/holder lifetime
already uses its direct provider ownership; blanket nested registration would
leave stale metadata on direct deletion. No registry bridge or automatic
registration is introduced here. A separately published companion helper can
be composed by the integration owner later; it is not consumed by this packet.

## Two complete leaves

`assign_native_post_effect_frame_00b4e2b0` covers the complete 59-byte body
`[B4E2B0,B4E2EB)`, last instruction `RET 4` at `B4E2E8`, size 3. Native ECX is
the actual post effect and the sole stacked DWORD is the incoming frame.
Same identity does nothing. Otherwise it captures old `+08`, publishes incoming,
retains incoming through the current CE221C, then releases captured old through
the newly read CE2220. Zero dispatches the actual D5E600 frame's BD30E0 adapter
to B1FCF0 with flags 1. Reentrant writes to the parent are not cleared afterward.
ESI is saved/restored; EAX/ECX/EDX and flags follow native callees, not this new
C++ interface. All 14 callers in B107F0/B156C0 were inspected, including shared
push predecessors at B16CCF and B16E39.

`native_bloom_output_holder_00b54cd0` covers four bytes `[B54CD0,B54CD4)`:
`MOV EAX,[ECX+20]`, then one-byte `RET` at B54CD3. It returns a borrowed holder,
without validation, retain, or conversion to a texture. All seven callers were
inspected. Producer B54E70 zeros `+20`, then B54F90 publishes the third real
B4E020 holder there; B4E470 similarly establishes the setter's frame field `+08`.
The existing `TRIV_body_00b54cd0` Ghidra name was not changed by this worker.

## Validation and next work

Strict `scripts/build.ps1` passed with both configured CTests. The first build
identified use of C++20 `bit_cast`; it was replaced with the C++17-compatible
bit-preserving `memcpy` conversion before the passing build.

One ignored Win32 `/MD` probe links with `/MANIFEST:EMBED`. It compares copied,
live/PE-equal leaf bodies against source: seven setter cases and three borrowed
getter values passed. Only the native setter's two IAT operands and deleting
leaf binding were instrumented. It uses actual frame constructors, the same
actual counts and real frame deletion, with a controlled COM Release endpoint.
It verifies null/same/nonfinal/final paths, publication before retain, import
reread after a returning callback, and a parent write during terminal COM
release. It is a controlled leaf comparison, not original destructor or full
continuation execution. The probe's preferred image base was moved away from
D5E600 so its exact-profile fixture mapping could be reserved.

The composed continuation is **build/static only**. No application startup,
real device initialization, full function differential, SEH/FH3, teardown or
game validation is claimed. The adjacent FLD/FSTP helper preserves ordinary
float conversion bits; the original has integer/publication instructions
between FLD and FSTP, so unmasked floating exception ordering is not established.

The next phase starts exactly at `B11599`: `B4D170` on service `+64`, `B4CB10`,
`B4CBA0` on `+70`, then `B189F0` at `B115B5`. Those providers already exist.
Later native production includes additional post effects, `+1CC` frame and
`+2C` bloom, and many more members through `B13009`. The report carries the
existing full-function provider inventory as a reference, not coverage. Finish
those phases, explicit canonical service companion integration and the native
failure/lifetime schedule before admitting the service to full initialization
or destruction.

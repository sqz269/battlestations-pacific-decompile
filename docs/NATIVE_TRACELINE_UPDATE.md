# Native particle tracer motion and traceline point insertion

Addresses: `00B0A110`, `00AF2630`, `00AF22B0`; analyzed render body `00AF26A0`.
Descriptive names are reconstruction hypotheses, not recovered symbols.

| Routine | Inclusive native body | Original ABI | Coverage |
|---|---|---|---|
| Particle tracer motion, B0A110 | B0A110..B0A3EB | ECX definition; stack(state,age,word,matrix,delta); RET14 at B0A3E9, length3; EAX1 | Complete body through existing current-clock service; full original-byte differential |
| Point/time thunk, AF2630 | AF2630..AF2643 | ECX node; stack(point,interval); RET8 at AF2641, length3 | Complete; full original-byte differential |
| Ring insertion, AF22B0 | AF22B0..AF2620 | ECX node, EDX point; stack(interval); RET4 at AF2360/AF261E, length3 | Complete; full original-byte differential |
| Mesh fill/submission, AF26A0 | AF26A0..AF3168 | ECX node; stack(render context,float,float,flags); RET10 at AF3166, length3 | Analyzed only; entire body has no source implementation in this packet |

Source fastcall adds borrowed EDX access to B0A110 and AF2630. AF22B0 keeps
native ECX/EDX and adds access as its second stack word (source RET8). The
particle kernel saves access separately; native local offsets and every x87
spill retain their relative positions. These are new source interfaces, not
drop-in original calling conventions.

## Producers and actual storage

Use the same derived 1BCh node, actual payload80h and ring allocation as the
Traceline constructor and AF3440. AF3440 writes node+184 from its payload
argument at AF348A, allocates payload+20 times14h at AF34AB..AF34C5, publishes
node+188 at AF34CD and clears count190/head18C at AF34D3/AF34D9. Constructor
858260 clears flags194/195. No copied node tail, ring owner, reference count,
curve representation or allocation domain is introduced here.

AF22B0 rejects count equal to payload+20 or either nonzero flag194/195. Empty
insertion writes the incoming point, payload+14 scalar and current negative
zero minus interval. A one-row ring extends its head row. For count greater
than one, the existing endpoint is removed and the preceding row supplies the
origin. Intermediate points are spaced by payload+24. Their count comes from
the original register-input CRT truncation of distance/spacing. A full ring
evicts its head before final endpoint insertion. Signed IDIV/remainders,
unchecked capacity, zero/NaN distance behavior and field reloads are retained.

B0B6A0 constructs the6Ch state consumed by B0A110. B0A110 tests **DWORD**
state+30 against zero at B0A118. The export's `float != 0` is incorrect. The
definition's +2C/+30/+34 and optional+48 are established particle parameter
owners: scalar/linear/Hermite selection uses the WORD at parameter+A. Direction
and base vectors are read from the actual state and matrix; the native global
direction remains a borrowed contiguous triple at E13028/2C/30. Word and delta
are accepted but never read. Result EAX is1 even when state+30 is null.

## Required existing services

| Caller sites | Native callee | Source binding and evidence |
|---|---|---|
| AF2403 | 419440 | Existing `camera_vector_length_00419440`, same `CameraAxesCrtAccess`. Whole callee listing retains individual square spills, sum ordering, BF7030 and final float32 spill. |
| AF2437 | BF7420 | Existing `native_crt_truncate_st0_00bf7420`, current0109EEA4 pointer. ST0 input/EAX result, RET; hardware fallback BF7456 also retained. |
| B0A159/B0A1D3/B0A2B2/B0A33C | AFFD20 | Existing public naked-JMP wrapper for the parameter loader's integral-linear body; ECX parameter, stack age, RET4/ST0. |
| B0A160/B0A1DA/B0A2B9/B0A343 | AFFCB0 | Existing public naked-JMP wrapper for the integral-Hermite body; ECX parameter, stack age, RET4/ST0. |
| B0A3C1 | Current01090AB0 virtual1C | Existing `SystemTimeTimerVirtuals::interval_1c` on the freshly loaded canonical `FrameClock`. Concrete BEE070 is LEA EAX,[ECX+40]; RET. Source returns the same service's `ClockTimestamp`, then executes FILD64/FILD64/FDIVP/FSTP32. |
| B0A3DA | AF2630 | Complete reconstructed thunk, receiving state.xyz and the clock interval. |
| AF263C | AF22B0 | Complete reconstructed ring insertion. |

All direct AF2630 callers were inspected: B0A3DA and858627. The second copies
its linked node's current world position120/124/128 and supplies its frame
interval, then calls **AF1EB0 separately**. AF1EB0 is the existing particle
submodel update, not a missing replacement for this work. AF2650 is an
unrelated F8C284 section destructor. B0A110 is D5E048+28; the initializer's
existing current-slot dispatch supplies all five stack words.

## Render body evidence and remaining work

AF26A0 is current D0C928+20. Its full installed-byte CFG contains641 reachable
instructions, with balanced return depth, all direct branches inside the
body, no indirect jumps or jump tables, and no unreachable/free gaps. The last
instruction startsAF3166, length3, endingAF3168; AF3169..AF316F are CC padding
before AF3170. Ghidra had no function at this address during this worker's
read-only inspection; root owns any definition, annotation, export and save.

The body first tests render-context+8 camera+198, samples current clock virtual14
to node+198, rejects fewer than two ring rows, then culls current node virtual48
bounds through B71530. It obtains actual model geometry0 (B74640), stream0
(B73260), maps stream virtual10 with(count,0,0), fills48-byte vertex records,
sets actual section0 draw ranges (B732C0), unmaps current stream virtual14 and
forwards all four original arguments to B748E0. Optional payload+58 with clear
node+1A8 adds ten vertices and corresponding draw counts. AF1C20 performs the
forward 48-byte record copy (x87 floats, raw color DWORD); 419510 normalizes
directions and B6DB70 refreshes the current camera transform when needed.

These calls were inspected for analysis; no render host facade or successful
fallback is implemented. Completion needs canonical current bounds/culling,
stream map/unmap and B748E0 draw bindings over the actual model/mesh/section
domains. The report retains every analyzed call site with its containing
AF26A0 address; its live verifier will require root to define that function.

## Validation and limits

Project/program verification used `bsp.py ghidra` and `verify-seeds` against
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. All9 captured body spans
and7 constant spans matched installed bytes; hashes are in the report. The
combined BF7420..BF74CB fixture keeps its short branches at original relative
positions, including the BF7456 fallback. No Ghidra writes were performed.

Strict MSVC Win32 compilation passed `/MD /W4 /WX /O2 /fp:strict`. A temporary
probe outside the repository passed138 cases covering all three complete core
bodies, pointer/count/flag gates, constant/linear/Hermite and absent optional
channels, ring wrap/replacement, zero/NaN displacement, both CRT conversion
modes and three x87 control words. All raw mutations, EAX1, x87 status/control
and MXCSR matched. Native-side callers, integrals, vector-length and conversion
bodies execute original installed bytes; both sides share only the established
BF7030 sqrt service. The clock comparison uses the existing concrete timer
adapter and original BEE070 on equivalent timestamp storage. This does not
validate arbitrary timer overrides or exceptional CRT handlers.

`scripts/build.ps1` passed the existing worktree CMake targets and both CTests
separately; root owns registration of this new source and the public integral
wrappers. The probe also passed linked directly to the current root libraries,
without compiling another parameter-loading source object.
No permanent tests were added. The temporary replay is
`C:/Users/sqz269/bsp-as-traceline-update/build.ps1`; after integration it accepts
`-SourceRoot` equal to the current root and links only its current built
libraries. `AF26A0` is not source-reconstructed or fixture-tested. Neither the
core nor render path is game-validated, and no frame-timing claim is made.

# Ship model binder: complete body audit, open native dependencies

Addresses: 0082FE30

This is a read-only evidence audit of `0082FE30`, not a reconstruction or an
activation change. The complete live listing was read on 2026-09-15 against
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. This worker changed no
Ghidra definitions, annotations, flow flags, prototypes, or project saves. No names
are promoted and no source or ledger records are added. Referenced callees
remain with their existing owners.

## Coverage and ABI

| Routine | Coverage in this audit |
| --- | --- |
| `0082FE30` | Complete **listed executable body**, `0082FE30–00831801`: 1,732 instructions, 111 direct CALL sites, 6,610-byte enclosing span. Six skipped bytes `0083105A–0083105F` follow an unconditional jump and are not executable evidence. |
| `0082D700`, `0082CA40`, `0082C560`, `0082E1F0`, `0082F4E0`, `0082F8E0`, `004D1510`, `004215D0` | Complete live listings read; transitive callees remain open as listed in the report. |
| `00829180`, `0082E990` | Complete schedules read using live listing plus disk continuations, then the parent's repair independently rechecked: 52 and 87 listed instructions, no gaps. |
| `0082B220`, `0082F3C0`, `0082A1B0`, `0082E600`, `004F9B30`, `004072D0`, `00718870`, `0071B2C0` | Complete live listings read as dependency evidence. No additional ownership or implementation claimed. |
| `0082E280` | **Partial live body** `0082E280–0082E2C7`; candidate continuation `0082E2C8–0082E311` read from disk, currently unowned in Ghidra. |
| `00C91F60–00C91FEE`, `00C91FEF–00C91FF8` | Complete 13 unwind stubs and handler instruction bytes read; stubs have existing functions, handler does not. |
| `0095F500`, `0082D040`, other library/canonical bodies | No new whole-body semantic audit. Existing source availability is recorded separately; the vehicle binder belongs to the sibling packet. |

Entry consumes the actual class through ECX. `0082FE4F` captures it in EDI;
`00831724` advances EDI to class+6FCh, and `008317D1` later reuses EDI as the
stack copy destination. Thus the old statement that EDI remains the descriptor
for the **whole** body is false. EBX and EBP both have several roles: zeros,
indices, vector addresses, node pointers and scratch. Register provenance was
read across the entire function, not inferred from one initialization.
The entry has no stacked inputs and ends in `RET`; the steady stack is entry
ESP minus 128h. This documents native call shape, not the current incomplete
Ghidra prototype or a new binary-compatible C++ interface.

## Corrections to the earlier ship audit

The following supersede the corresponding statements in
`SHIP_CLASS_BIND_MODEL_DATA.md`. All arithmetic shorthand below assumes valid,
unchanged point storage; a native reconstruction must retain repeated loads,
returning validation, x87 spills and SSE operations.

1. The enclosing span is **6,610 bytes**, not 9,938. The current listing contains
   1,732 instructions, not the older snapshot's 1,719.
2. Both `orrhullam` frames use point **0** as their origin. For each, translation
   is p0, row+10h is p2-p0, and row+20h is `(p2-p0) x (p1-p0)` before `0085DC80`.
   The first point load at `008302F4` reads `[begin]`; the p2 load at `0083033A`
   reads `[begin+18h]`. The three cross calls at `00830524`, `008307FD`, and
   `008316AB` all pass p2-p0 through EDX and p1-p0 on the stack. `004F9B30` was
   read completely and confirms that operand order. For `idle`, row+20h is
   p2-p0 and row+10h receives that same cross product.
3. `0082E990` is a **resize** of a vector whose 10h-byte elements are themselves
   vector headers; it receives the requested count and a by-value 10h prototype,
   then returns with `RET 14h`. The current path index is the requested count,
   not a field written into each path record. The binder selects the last
   element and appends all node points into it.
4. For stable valid storage, the final reconciliation **shrinks to the smaller
   count**. `00831746 CMP ESI,EAX; 00831748 JNC 00831775` sends fewer matrices to
   `0082F8E0(records, matrix_count, prototype)`. Otherwise `008317A7/A9` sends
   fewer records to `0082C560(matrices, record_count, matrix_value)`. Both helper
   listings include explicit shrink branches. The by-value arguments and
   constructor still execute even when the shrink path does not need their
   values. Equal counts do nothing; either side can shrink to zero.
5. There are **five** indexed loops: `kemeny`, `farviz`, `explosion`, `path`,
   `idle`. The last two stop on the first missing group **or fewer than two
   points**. `idle` subsequently reads point 2, so the count==2 case still
   reaches the point accessor's validation boundary.

The fallback at `008309F1–00830DD6` resizes class+584h to 12 points. Let
`x=round32([class+A4h]*0.5)`, `y=round32([class+A8h]*0.5)`, and
`z=round32([class+A0h]*0.5)`. Here `nx`, `ny`, `nz` mean native `SUBSS -0.0,
component`, not a sign-bit flip. The actual ordering is:

| Indices | XYZ values |
| --- | --- |
| 0, 1, 2, 3 | `(nx,ny,nz)`, `(x,ny,nz)`, `(nx,y,nz)`, `(x,y,nz)` |
| 4, 5, 6, 7 | `(nx,ny,z)`, `(x,ny,z)`, `(nx,y,z)`, `(x,y,z)` |
| 8, 9, 10, 11 | `(nx,ny,+0)`, `(x,ny,+0)`, `(nx,y,+0)`, `(x,y,+0)` |

Each element has its own checked-vector access and fresh begin load. This closes
the earlier eleven unread sign patterns. The first dimension load is A4h at
`00830A07`; the historical snippet put A0h there incorrectly. No new producer
meaning is assigned to A8h by this packet.

## Complete body schedule and direct boundaries

The JSON report contains **all 111 call-site rows**, each with its actual
`address`, `native`, and containing `function`, plus the audited dependency
edges. There are 22 distinct immediate CALL targets. No indirect CALL occurs
inside this body.

Execution first calls `0095F500` then `0082D700`. It passes both named hull-line
point vectors and class+71Ch to `0082D040`; copies or synthesizes debarkation
points; seeds/builds two bow frames; copies the required `wave` pair; copies or
zeros `shipcenter` and `wave_stern`; appends indexed point families; constructs
path vectors and idle matrices; then truncates mismatched idle counts.
`wave` has no null-node check. The two hull-line lookups also proceed directly
to node+44h; no missing-resource fallback is established here.

The ABI evidence includes `RET 10h` for `004D1510` (count plus a 0Ch by-value
prototype, whose bytes the caller does not initialize), `RET 14h` for
`0082E990`, `RET 20h` for `0082F8E0`, and `RET 44h` for `0082C560`. The last
call receives 40h bytes copied from the local matrix by `REP MOVSD`. Omitting
these by-value inputs loses the native construction/cleanup behavior.

The report deliberately leaves unknown callees as `call_<address>` with
`contract: unread`. Reading a wrapper does not establish its unread copier,
allocator, eraser or destructor contract. No host stubs are introduced.

## Cleanup and listing repairs needed

The immediate handler `00C91FEF` loads FuncInfo `00DC377C` and jumps to
`00BF6B43`. FuncInfo has 13 states and unwind map `00DC37A0`. State 1 unwinds
to state 0; every other state unwinds to -1. States 1–3 destroy the SBO string
at EBP-BCh; state 0 and states 4–12 use EBP-F0h. The 13 existing `Unwind@...`
stubs tail-jump to canonical string destructor `004072D0`. Keep their names.
Normal execution has 13 string-free CALL sites and no new ownership protocol.

Two internal repairs were applied by the parent during this audit and then
independently verified here. The body extension remains outstanding:

| Function | Current evidence gap | Disk continuation / action |
| --- | --- | --- |
| `00829180` | Former gap `008291F6–008291FE`, after CALL `00BF6989` at `008291F1` | **Parent repaired; verified.** `ADD ESP,4; MOV [ESI],EBX; MOV [ESI+8],EBP; POP EBX`. Bytes `83 c4 04 89 1e 89 6e 08 5b`. |
| `0082E990` | Former gap `0082EA5C–0082EA5E`, after CALL `00BF65AC` at `0082EA57` | **Parent repaired; verified.** `ADD ESP,4`, bytes `83 c4 04`. |
| `0082E280` | Live function stops after CALL `00BF65AC` at `0082E2C3` | Candidate continuation `0082E2C8–0082E311` includes second-tree cleanup, another free at `0082E2F2`, header resets and RET. Establish/extend ownership before attributing those calls or repairing flow. |

`00C91FEF–00C91FF8` is an existing two-instruction handler without a function
definition; its definition is a separate leased metadata task. Other helpers'
FH3 tables are not fully audited. A read-only `flow-properties` query could not
return flags because the bridge reports script execution disabled. The raw
response is retained in `local/ship-binding-flow-properties.json`; this audit
did not change bridge settings. Parent repair evidence is
`reports/vehicle_binding_dependency_flow_repairs_orch4.json`; parent owns that
file. This worker performed no mutations. The helper body extension and handler
definition are still proposed work.

## Minimal implementation order

| Packet | Readiness and exact ownership proposal |
| --- | --- |
| Native borrowed pointer append | **Ready**: `0082D700–0082D7D1` only, new dedicated `native_ship_model_pointer_binding` source/header. Reuse `insert_native_game_type54_one_0071b2c0` and existing invalid-parameter callbacks. Its only live caller is `0082FE58`. Preserve destination entries, source iterator capture/reloads and no item retain/release. Parent must approve files before implementation. |
| Counted 10h point/index reserve | **Ready after parent repair**: `00829180–00829203`, dedicated `native_ship_indexed_point_array` source/header. All five caller argument setups read. The ship and `00829CE8`/`0082A5A8` double capacity with a signed minimum of 1; `0095709B`/`0095863B` pass 0 only when capacity is signed-negative. Reuse canonical allocation/free and ordered x87 transfer conventions. Own only the helper, not callers. |
| Native 0Ch vector operations | Evidence/implementation packet for `004D1510`, `004215D0`, then exact dependencies `0041FA40`, `004C82B0`, `0041C7B0`, `004206A0`. These bodies are not replaced by the raw counted-array helper `0074D190`. Transitive closure remains open. |
| Native 40h matrix vector | Evidence/implementation packet for `0082CA40`, `0082C560`, `0082A2E0`, `0082C610`, `0082BFF0`, `0082B220`, `0082A1B0`, `00828C30`. Transitive closure remains open. |
| Nested path vector | `0082E990` plus `0082E320`/`0082DD80`, required returning-flow repair and nested-vector cleanup. Do not store an invented index in the element. |
| Idle/traffic record lifecycle | `0082E1F0`, `0082E280`, `0082F8E0`, `0082F3C0`, `0082E600`, `0082DFC0`, `0082F560`; separate traffic fallback `0082F4E0` with `0082EAB0`/`0049F910`. Allocate/erase/copy/destruct dependencies and helper EH tables require further reading. |
| Native buoyancy producer | `0082D040` remains a native-reconstruction task. Existing `ship_buoyancy_elements.cpp` supplies pure rules/std::vector interfaces, not actual class storage, native allocation, validation, or x87-compatible body closure. |
| Full ship binder | Wait for the above and sibling `0095F500`, then implement the complete `0082FE30` schedule with its FH3 lifetime evidence. No partial source from this audit. |

Current reusable source is present for the named-group lookup/count/point
accessors, raw SBO strings, `0074D190`, native `0085DC80`, native `004F9B30`,
and type54 pointer insertion. BG's `00879AD0` is reached through the external
vehicle binder; it is **not** a substitute for `0095F500` or for the ship body.
The required vehicle boundary is ECX=the same actual class, no stacked args,
normal return before ship-specific writes, canonical damageable behavior plus
the complete vehicle-specific effects. Its internal fields remain with the
sibling audit. Model/resource ownership and activation are external contracts.

## Verification limits

The report also contains ready-to-append correction text for the older audit;
the historical file is not edited by this packet. The first-origin load is
backed by bytes `8b 46 48 85 c0 f3 0f 10 00` at `008302F4`, the p2 load by
`8b 46 48 d9 40 18 83 c0 18` at `0083033A`, and the two tail comparisons by
`3b f0 73 2b` at `00831746` and `3b c1 73 3e` at `008317A7`.

`python tools/verify_report_calls.py reports/native_ship_class_binding_audit_orch4.json`
passed: **192 call rows checked, 0 failed**.
No C++ build is required for these two documentation files. No differential
fixture, throwing cleanup, live class parsing, binary replacement, executable
admission, or gameplay result is claimed. This audit closes the main-body
reading gaps while preserving the open dependency and cleanup work.

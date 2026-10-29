# Undefined Dyn radix manager constructor: 00C37210

The original bytes establish a complete 39-byte store-and-return sequence at
`00C37210..00C37236` inclusive. The constructor interpretation follows its
fixed vtable store for `Dyn::SAPRadixBroadPhaseManager`; the new name
`construct_manager_00c37210` is a descriptive hypothesis. No Ghidra function
was created or changed. Body credit and runtime activation remain provisional
until the primary accepts the boundary and establishes genuine provider custody.

## Original bytes and boundary

The configured and verified live target is project `bsp`,
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`,
`x86:LE:32:default`, image base `00400000`. Bounded `bsp.py ghidra proto` and
`ghidra bytes` queries verify that target. `bsp.py disasm-raw` separately
decodes the installed executable's bytes. The live 80-byte window at
`00C37200` agrees with the disk sequence and padding described below.

Installed executable SHA-256:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The exact 39-byte body has SHA-256
`3ef730a6dfaf25c93827c89028a4d6e7ffb539eed403857651c2f45b4ec70a35`:

```text
33 c9 c7 00 18 a1 d7 00 89 48 04 89 48 08 89 48
0c 89 48 10 89 48 14 89 48 18 89 48 1c 89 48 20
89 48 24 89 48 28 c3
```

| Instruction address | Bytes or operation |
| --- | --- |
| `00C37210` | `33 C9`: XOR ECX,ECX. |
| `00C37212` | `C7 00 18 A1 D7 00`: MOV [EAX],00D7A118. The absolute immediate occupies `00C37214..00C37217`; `00C37214` is not an instruction start. |
| `00C37218..00C37233` | Ten three-byte MOV instructions store ECX at EAX+04h,+08h,+0Ch,+10h,+14h,+18h,+1Ch,+20h,+24h,+28h. |
| `00C37236` | `C3`: plain RET; this byte is the inclusive body end. |

The preceding defined function, `00C37180`, ends at `00C37202` after its
three-byte `RET 4` at `00C37200`. Bytes `00C37203..00C3720F` are thirteen
INT3 padding bytes. The constructor ends before nine INT3 padding bytes at
`00C37237..00C3723F`. The separate sequence at `00C37240` is MOV EAX,[ECX]
followed by RET at `00C37242`; it too is currently undefined and must not be
absorbed. The next snapshot-defined function starts at `00C37290` and ends
at `00C372FC`. `bsp.py lookup 00C37210` independently reports an undefined
39-byte chunk with exclusive end `00C37237`.

## ABI, stores and provider contract

EAX is the incoming manager and remains unchanged through the plain RET,
so the source returns that same pointer. No stack arguments are accessed or
popped. ECX is zeroed; all other general-purpose registers are unchanged by
the original instructions. There are no calls, allocations, frees, exception
handlers or branches in the body. Native register/flags transport is not
reproduced by the explicit C++ interface.

The source stores the required binding corresponding to image identity
`00D7A118` at manager+00h, then zeros all ten proven dwords at +04h..+28h
in order using raw Win32 addresses and volatile stores. It does not infer
roles for those fields or touch bytes beyond +2Bh. It adds no null guard,
whole-object initialization, fake table or default provider. The object
extent is proven only to be at least 2Ch for these accesses.

`VtableBinding` is a required explicit adaptation of the image's fixed
immediate. This packet supplies no callable host table. An arbitrary pointer
or zero does not establish genuine provider custody. The original table's
first word at `00D7A118` is `00C55D30`, matching the recovered owner's slot.
The raw 36-byte table window is recorded in the report; its ninth word at
`00D7A138` is `00D7C814`, and no nine-slot callable-table claim is made.
The primary must establish the actual table/provider contract before runtime
use or accepted body credit under that contract.

## Incoming-reference evidence

The existing `tools/image_ref_scan.py` scan found no raw reference candidates
to `00C37210`. A separate endpoint-inclusive PE scan confirmed zero matching
absolute dwords in every file-backed section and zero E8/E9 rel32 candidates
in `.text`. It scanned each final complete dword/instruction position too.
The audit is preserved in ignored
`local/cc16_dyn_radix_constructor_audit.json`, with the results in the report.

Four positive controls were recovered: the vtable immediate at operand
`00C37214`, the owner pointer at data slot `00D7A118`, and the two owner-to-leaf
rel32 sites `00C55D79` and `00C55D88`. These distinguish the raw scan's
coverage from the known incomplete live xref view. The preceding owner packet
found no live references to the constructor, and live xrefs miss its vtable
store even though the original bytes prove it.

This static evidence supports the prior unused-class census within the scan
model. It does not exclude runtime-computed/indirect references, short branches,
or other call encodings. The primary owns acceptance of an original dead chain
or proof of a real incoming runtime path. No activation is claimed here.

## Safe definition request and validation

After review, the primary may use locked tooling to define a function with
entry `00C37210` and exact body `00C37210..00C37236`, excluding both padding
ranges and the neighboring undefined getter. Recheck the exact body hash and
absence of overlapping functions before mutation; preserve comments and
record prior values. Suggested hypothesis name:
`BSP_DynRadixManager_Construct_00C37210`. Preserve the observed EAX receiver
ABI rather than assigning ECX as a thiscall receiver. The return is normal,
with no calls or flow overrides justified by these bytes.

The report separates the constructor's empty call set from two positive-control
owner call rows; `verify_report_calls.py` checked both with zero failures.
Its Markdown check found zero failed call claims and six expected warnings
for the undefined body and padding ranges. These are not proof of a live
function; no function was fabricated to silence them.
No new tests, compiler, game or
probe runs are part of this packet. Shared ledger/CMake edits, function
definition, annotation, build and provider acceptance remain primary-owned.
Previous leaf and owner files are unchanged.

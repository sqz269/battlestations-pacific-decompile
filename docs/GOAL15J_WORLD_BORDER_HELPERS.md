# Whole raw world-border helpers

This packet reconstructs the complete entries `004C2310` (51 bytes),
`004CF450` (147 bytes) and `004B6C40` (59 bytes) as separately callable MSVC
Win32 Source interfaces. It supplies actual raw storage operations rather than
promoting the older typed border geometry projection. The old typed APIs and
their historical ledger entries remain intact.

| Native entry | Original physical interface | New Source interface |
| --- | --- | --- |
| `004C2310..004C2342` | three stacked DWORD cells: next, previous, value-cell address; EAX node; RET 0Ch | `allocate_native_world_border_node_004c2310` (`__stdcall`) |
| `004CF450..004CF4E2` | ECX actual 0Ch list header, stacked addition, RET 4; no semantic result | `increment_native_world_border_count_004cf450` (`__fastcall` with ignored Source-only EDX slot) |
| `004B6C40..004B6C7A` | ECX actual game, stacked edge DWORD, ST0 float result, RET 4 | `span_native_world_border_004b6c40` (same Source-only EDX slot) |

Target verification used `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 image base `00400000`. All 257 Native body
bytes were read live through `bsp.py ghidra` and compared with the installed
PE selected by `config/target.json`. The report retains all Native and emitted
instructions, bytes, relocations, operand mappings and hashes.

## Node allocation and actual argument cells

The complete 19-instruction body is preserved in inline assembly. Its only
external call is rebound from Native `00BF681B` to a genuine current service
bridge calling `singleton_lifetime_allocate` with kind `object`, native size
0Ch and current host size 0Ch. That service uses actual current `malloc`,
`_callnewh` retry and `std::bad_alloc`; a successfully returned node is released
by `singleton_lifetime_free`. It is not a callback, synthesized allocation or
an assumed Native private-CRT identity.

The allocation completes before the three incoming stack words are read.
The next argument is read only before the first store, the previous argument
only before the second, and the value-cell address and its current pointed
DWORD only before the third. No arguments or payload are snapshotted across
the allocation. The independent destination guards test EAX, EAX+4 and EAX+8
with wrapping x86 LEA operations. No blanket null-return guard or pointer
repair was added. Original call/exception boundaries remain qualified.

Strict compilation emits exactly 51 bytes, identical to the entire Native
body after normalizing only the CALL's four-byte relative target operand.
Every other opcode, operand, conditional transfer and RET 0Ch agrees.

## Complete count admission and error behavior

Read the current DWORD at header+8 and capture the stacked addition. Compute
the DWORD subtraction `uint32(3FFFFFFFh-count)`. If that value is less than
the unsigned addition, construct and throw the genuine current-SDK
`std::length_error("list<T> too long")`; otherwise publish the wrapping sum
at header+8. Header+0/+4 remain untouched. In particular, a count already
above the nominal limit does not introduce a separate rejection rule. The
fixture's `40000000h + FFFFFFFFh` is admitted and publishes `3FFFFFFFh`.
The rejecting path performs no header store before construction/throw.

All 37 Native instructions were inspected, including their error and FH3
path. Original `004CF47C..004CF4C8` initializes a temporary legacy SBO string,
assigns the 16-byte message through `00408720`, arms its cleanup state, calls
`00411700`, installs length-error vtable `00D69260`, then calls `00BF6885`
with ThrowInfo `00D83F98`. Its FH3 handler at `00C65BC8` loads `00D8E5BC`
and jumps to `00BF6B43`; that FuncInfo has one state and unwind map
`00D8E5B4 = {-1,00C65BC0}`. The cleanup funclet takes `[EBP-50h]` and
tails to `004072D0`. These are evidence, not invented Source globals.

The new exact-address entry follows the established policy used by
`native_list_count_guards.cpp`: the complete normal/error operation is
implemented with current SDK exception handling. The 65-byte emitted entry
contains the arithmetic, both branch outcomes, direct genuine length_error
construction, current ThrowInfo and current `_CxxThrowException`. The report
maps every Native instruction to its retained operation or explicit current
SDK frame/exception boundary. It claims neither byte equivalence for this
147-byte body nor identical legacy string/object allocation schedules,
failure sites, FH3 frames, RTTI, ThrowInfo, throw/unwind ABI or private CRT.

The old descriptive name `STL_xlen_throw_004cf450` misleadingly excluded its
returning increment path. The reviewed name is
`STL_List_IncrementSizeChecked_004cf450`; the prior name and full preexisting
comments are retained in the receipt. The node's existing STL shape name and
the span's existing game name are preserved. The worker made no Ghidra
mutations: the primary applies names, physical prototypes and appended
comments under its coordinated write lock after the worker releases ownership,
then saves and refreshes the affected exports.

## Complete x87 span and floating-point environment

All 15 instructions and all 59 bytes are reproduced exactly, without
relocations or a C++ floating-point expression. Edge DWORD 0 or 2 reads
current game+7128 then subtracts current game+711C. Every other DWORD,
including `FFFFFFFFh`, reads current game+7124 then subtracts current
game+7130. Each path does `FLD`, `FSUB`, `FSTP [ESP+4]`, `FLD [ESP+4]`,
and RET 4. The actual incoming edge word is the float spill/reload cell.
There is no copied bounds object, ordinary double intermediate, validation
of unsupported edges, control-word override or status clearing in the entry.
Native x87 precision, rounding, masked/unmasked exceptions and fault order
therefore remain governed by the caller's actual environment and addresses.
The Source instruction addresses differ, including recorded x87 instruction
pointers; this is not a claim of identical saved FPU-environment bytes.

## Verification and limits

The new translation unit compiled with `/MD /O2 /Gy /W4 /WX /fp:strict
/EHsc /std:c++20` for x86. The local `world_border_probe.exe` embeds its
manifest and links the real reconstructed `bsp_core.lib` allocator service.
It checks real allocated node contents and real free, normal/wrapped count
publication, and a caught current length_error with the exact message and
unchanged rejecting header. No fake allocator or exception provider is used.

The same fixture executes the whole installed 59-byte position-independent
span body from protected executable storage and compares it directly to
Source. Its 36 comparisons use edges 0/2/1/FFFFFFFFh, nearest/down/up x87
rounding, normal/rounding-sensitive/NaN-infinity cells. They compare result
bits, actual argument-cell contents, x87 control/status and RET 4 stack
balance. This is focused differential execution of the standalone span,
not startup, whole Native ABI replacement or gameplay proof.

Reproduction helpers are ignored artifacts under
`local/goal15j_world_helpers/`: `build_helpers.ps1`, `audit_helpers.py` and
`world_border_probe.cpp`. The report records their hashes, complete byte
receipts and the compiler/build/fixture outcomes. The standard repository
build and its two registered tests (`reconstructed_math`, `tool_tests`) passed.
This fresh worktree does not inherit the primary's ignored seed-header test
registration. This worker does not edit shared
CMake registration, which belongs to the integrator.

The full 1666-byte `004D5BD0` parent, 756-byte `004C71C0` layout,
conditional `0049F260 -> 0049E560` traffic chain and its genuine current
provider composition remain incomplete. Current producers for the mutable
48-word authored globals and size cells `E0826C/E08270` were not established
here. The helpers make none of those dependencies a stub or an adopted world
path, and do not claim whole-world production adoption or game validation.

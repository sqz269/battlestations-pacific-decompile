# GOAL16J: three complete signed-span count entries

This packet reconstructs the complete standalone entries below as readable
C++17. Operation names and formal raw-pointer/signed-return types are new
interfaces, not recovered original class or field identities. Each strict and
registered MSVC32 function section equals all 31 Native bytes, from entry through
both returns. No Native-size prefix, helper omission or guessed valid-range
restriction is used.

| Native entry | Previous tagged name | Operation | Whole bytes |
|---|---|---|---:|
| 00BB40E0 | STL_Vector_Size_00bb40e0 | Signed wrapped byte span /36 | 31 |
| 00BB41E0 | STL_Vector_Size_00bb41e0 | Signed wrapped byte span /20 | 31 |
| 00BBA220 | STL_Vector_Size_00bba220 | Signed wrapped byte span /24 | 31 |

The machine-readable evidence is `reports/goal16j_tagged_sizes_bb.json`.
All three fresh typed ranges are complete, have no flow overrides, and their live
CLI bytes match the installed PE. The batch verified
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, base `00400000`.
No Ghidra name, prototype, comment, export or tag was modified by this author.
The name ledger retains the correct existing STL_Vector_Size names and appends
the measured signed-wrap/stride evidence. Descriptive Source-interface names
are kept separate. Initial owned name rows using Source hypotheses were
corrected before commit at Root direction; the correction is preserved locally.

## Original physical inputs and effects

Each entry consumes only ECX as raw backing. First read DWORD `[ECX+4]` into
EAX exactly once. TEST that captured word; if zero, return zero without reading
`[ECX+8]`. Otherwise load `[ECX+8]` into ECX, subtract captured first modulo
2^32, then interpret that bit pattern as signed int32 and divide by the listed
positive stride, truncating toward zero. There is no capacity read, backing-null
guard, element dereference, object-extent requirement, pointer subtraction,
store, callback, global, external provider or FP instruction.

Plain RET consumes the current return word and advances ESP by four; there are
no actual stack arguments, prologue, local spill or hidden stack access. Null
first leaves ECX as backing and EDX unchanged, with EAX zero and TEST-zero flags
(CF/OF zero, ZF/PF set, SF clear, AF undefined). Otherwise ECX is the wrapped
delta, EDX is the arithmetic-shifted signed-product high half, EAX is the
quotient, and flags are those of the final sign-correction ADD. EBX/ESI/EDI/EBP
are retained. x87, x87 FIP and SIMD state are unchanged.

Effective-address addition wraps at 32 bits. The measured instructions preserve
unaligned access, arbitrary address bits, negative/wrapped spans and INT32_MIN.
The +8 load is conditional and ordered after the +4 load; if it faults, EAX is
the captured first, ECX is still backing, and TEST-nonzero flags remain. The
whole bodies retain return-slot/backing aliases and fault prefixes. C++17
`memcpy` describes word/bit transfers without invented alignment or signed
conversion assumptions; source-language defined behavior requires accessible
bytes. Whole emitted identity establishes the measured MSVC32 physical effects,
including invalid-input fault behavior, without claiming identical mapped
return/fault EIP locations or a universal C++ language guarantee.

## Full-domain integer quotient

Unsigned `last-first` supplies the native SUB32 bits. A C++17 memcpy transfers
those bits to int32 before positive-constant division. This avoids signed
subtraction overflow, out-of-range unsigned-to-signed conversion and pointer
difference assumptions. No domain is narrowed to valid positive element counts.

Native uses `q0=floor(m*x/A)`, then `q=q0+(q0<0)`, where `A=2^(32+k)`.
The `(divisor,m,k,m*divisor-A)` tuples are
`(36,38E38E39h,3,4)`, `(20,66666667h,3,12)` and
`(24,2AAAAAABh,2,8)`. For nonnegative x, the positive error is below 1/divisor,
so the floor is ordinary integer division. For negative x=-y, the same bound
makes q0 one less than `-floor(y/divisor)`, including exact multiples; the final
one corrects it. The only error-bound equality is divisor24/y=2^31. Its remainder
is eight, so it still has that floor/ceil relationship; explicitly q0=-89478486,
q=-89478485, the truncation of INT32_MIN/24. The report records exact constants
and bounds. All three complete Native-identical emitted bodies provide an
independent code check; no broad arithmetic test suite or probe was added.

## Bounded discovery and existing Source

The indexed non-TRIV, tagged, unreconstructed, zero-indexed-callee query in
B9-BB excluded Root I's fifteen entries and seven global candidates. It returned
only fifteen names; all fifteen whole bodies were inspected, 593 Native bytes.
The selected three supply 93 bytes/39 instructions. The twelve other bodies
supply 500 bytes and each actually calls `[00CE2220]` and dynamic vtable slot0:
the call index omitted those import/indirect edges. The installed PE identifies
KERNEL32!InterlockedDecrement. That genuine SDK provider is not a private CRT
blocker; the selected entries' dynamic lifecycle targets remain unqualified as
independent leaves. The report lists each deferred address and its actual calls.
No claim of blanket Source absence or impossible future interface is made.

Whole src/include and indexed ledger checks found no standalone selected body
in worker, Root, Main or origin. `native_mpak_lookup_storage.cpp:13ff` inlines a
related signed /36 count inside BB4140. `native_mpak_container_storage.cpp:16ff`
uses an unsigned-stride count in larger append operations.
`native_vfs_lookup_leaves.cpp:41ff` contains a provider-local MSAR /24 count at
different offsets. Those existing operations are retained, explicitly mapped,
and not counted again as standalone entries. The BBDEC0 weather header hit is
only a partition-boundary comment. All four new paths were verified absent
twice before creation on worker, current Root/Main and origin; exact commits
are recorded. No other function was leased or changed.

## Build, complete providers and preservation

Strict `/std:c++17 /MD /O2 /W4 /WX /fp:strict /Gy` and actual registered
Release/Win32 `/std:c++17` objects contain exactly three executable sections,
93 bytes and 39 instructions, with all function symbols at section offset zero.
There are no executable relocations, hidden helpers or undefined providers.
Each object has three type7 `.debug$F` metadata relocations to its own function
symbols. Every section, data byte, symbol and relocation is included in the
report, as is the unique real `bsp_core.lib` member and full archive hash.

Required `scripts/build.ps1` session77763 terminated with exit zero in
17.7197712 seconds. Existing CTests reconstructed_math and tool_tests passed
2/2 in 7.01 seconds. The Source compiled successfully on its first attempt;
one COFF wrapper relative-path error was repaired using the same object without
recompiling. No new tests, probes, Native execution or game validation occurred.

Before compilation, 38 full prior-G/current-B5/E provider/evidence files totaling
93,927,607 bytes were frozen under local/goal16j_tagged_sizes_bb/frozen_16g.
Earlier B5/E freezes remain retained. The resulting archive still contains one
real unchanged B5, E and G provider each, and their Source/header/report pins are
preserved. Root's independent G freeze is also recorded.

## Inclusive author time and remaining review

First readiness clock: 2026-10-16 07:14:36 UTC. Readiness ended after lease
release at 07:20:39 UTC, 363 seconds. Implementation begins at that same endpoint.
The report checkpoint and final ignored handoff receipt include the continuing
implementation, preservation, strict/warm build, path-wrapper repair, arithmetic
and artifact review, docs, ledger/index, commit, lease release and handoff work.
The full clock is not reset at implementation or compilation. No throughput or
controlled speedup claim is made. Root performs independent review, annotations
and integration; these new interfaces do not establish parent adoption,
installed binary replacement or game validation.

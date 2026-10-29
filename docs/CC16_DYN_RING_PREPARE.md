# Dyn ring preparation: 00C4D370

This packet reconstructs the complete normal body at `00C4D370..00C4D594`
inclusive, 549 bytes. The current Ghidra name is `DYN_physics_00c4d370` and
the prior ledger row is `DYN_unreachable_00c4d370`, status
`metadata_only_no_function_body_verified_oct24`. The source name
`prepare_chain_00c4d370` and field roles are descriptive hypotheses.

The source uses the actual raw owner and existing
`cc15_dyn_live::DynLiveCrt`, with a required explicit `00C410E0` provider.
It does not import the sibling's active owner/query source as an accepted
runtime capability. Shared registration, metadata, runtime composition and
dependency acceptance remain primary-owned.

## Target, ABI and raw storage

Read-only `bsp.py ghidra proto` verified project `bsp` at
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, before the
pseudocode/listing batches. The complete evidence was paged through
`bsp.py show 00C4D370` and `--asm`, backed by
`exports/bsp/functions/00c4d370/{decompiled.c,assembly.txt}`.

Native ESI is the owner, with no stack arguments and a plain RET at
`00C4D594`. EBX, EBP and EDI are saved/restored; ESI is unchanged. The
parent ignores EAX. The source returns void and supplies explicit arguments,
without claiming native register/stack or exception transport.

The observed fields agree with the sibling's read-only 50h owner layout:

| Offset | Proven access |
| --- | --- |
| +04h | Current anchor, updated after each rejected candidate. |
| +08h | Points-owner pointer; its first dword is the point-data pointer. Each point has two float dwords. |
| +0Ch | Next-index data pointer, captured once in EBP at entry and reused throughout. |
| +18h | Previous-index data pointer, loaded per candidate and again on successful publication. |
| +24h / +28h / +2Ch | Scratch dword data / unsigned count / capacity. |
| +30h / +34h | Next / previous cursor, written by this preparation. |

The source deliberately accepts `void*` rather than duplicating or importing
the unaccepted sibling owner type. It does not initialize unrelated fields.

## Search and ambient floating-point behavior

At each candidate, the routine snapshots next-point, anchor-point and
previous-point raw words in the original memory/FLD order. Its x87 sequence
forms four float-spilled differences: anchor-minus-previous x/y and
next-minus-previous x/y. It then multiplies next-dy by anchor-dx, multiplies
anchor-dy by next-dx, and subtracts in x87 without spilling that determinant.
`FCOMIP` against zero followed by JBE rejects nonpositive or unordered results.

The implementation retains the original x87 instructions, all four FSTP-float
epochs and memory/FLD ordering in MSVC Win32 inline assembly. SETA records
the complement of the native JBE rejection condition. It leaves actual
ambient precision, rounding, exception masks and MXCSR in force; it adds no
control change, fixed-PC24/PC53 model, C++ double determinant or SSE substitute.
It neither clears FP status nor adds an x87-stack cleanup beyond the native
balanced sequence. Hardware fault/exception transport through C++ is unproven.

Only a strictly positive ordered orientation invokes the required query.
For captured anchor `a`, its exact inputs are:
EAX = next[next[a]], EBX = next[a], and stack(owner, previous[a],
previous[a], a). The previous index is the one captured before the orientation
gate; the two next indices are reloaded after it. The whole query result is
tested for zero. Rejection writes owner+04h = cached-next[a] and repeats,
without a new iteration limit or malformed-ring guard.

`QueryProvider` requires the real `00C410E0` behavior and its actual ambient
x87/MXCSR contract, including the unresolved barycentric dependency. The
existing PC53 value helper for `00C37860` is not silently substituted. There is
no fallback query body or hidden precision choice.

## Cursor publication and three appends

On success, `00C4D436..00C4D448` reloads the previous-data base and writes
owner+34h = previous[captured successful anchor], then writes owner+30h =
cached-next[captured successful anchor]. These use the captured anchor even
if a query provider changed owner+04h. The scratch appends then read values
from +34h, current +04h, and +30h, in that order. Each value is read after
its own growth/free sequence and only for a nonzero output address.

Each append grows only when count equals capacity. New capacity is wrapping
uint32 `(capacity * 2 + 2)`; allocation size is wrapping capacity times four.
Capacity is written before allocation. Copy loops reload count and old data,
test each destination address for zero, and advance with wrapping x86 address
arithmetic. Current old storage is freed if nonzero before new storage is
published. The final append store also tests its destination for zero, but
count increments regardless. Count greater than capacity does not trigger
growth. No overflow, null-allocation, count, point-index or loop guard is added.

The three identical native append expansions share one source helper while
retaining their order and callback-sensitive field reloads. All buffers use
the supplied existing CRT custody: only its `00BF55BE` and `00BF6989` entries
are consumed here. Allocation exceptions can propagate after capacity,
cursor and earlier append mutations; no catch, rollback or cleanup is added.
The free provider must preserve the actual native CRT contract.

Original function `00C4D370` has these seven direct call instructions:

```text
00C4D421 CALL 00C410E0
00C4D467 CALL 00BF55BE
00C4D49E CALL 00BF6989
00C4D4D2 CALL 00BF55BE
00C4D503 CALL 00BF6989
00C4D537 CALL 00BF55BE
00C4D56E CALL 00BF6989
```

## Validation and remaining obligations

One isolated Win32 object compiled with `/std:c++17 /EHsc /W4 /WX
/fp:strict /arch:SSE2 /O2`, with zero warnings. Object disassembly preserves
the reviewed FLD/FSUB/FSTP/FMUL/FCOMIP sequence, four float spills, no final
determinant spill, and SETA after FCOMIP/FSTP. Artifacts are ignored
`local/cc16_dyn_ring_prepare.obj` and `local/cc16_dyn_ring_prepare_object.txt`.
This is compilation/static instruction evidence, not differential FP,
native ABI or runtime validation. No new tests, game or probe runs occurred.

The report records all seven direct sites and the parent site at `00C4D622`
inside `00C4D5A0` for live call verification. The live checker passed all eight
report rows and all seven listing claims with zero failures. The recovered ring owner's
preparation hook must be explicitly connected after both packets are accepted.
The actual query and ambient barycentric providers remain required, as do the
owner's other predicates. The primary owns CMake, shared C40000 metadata,
annotation, common build/checks, and acceptance of a real incoming runtime
path or an evidenced original dead chain. No activation is claimed.

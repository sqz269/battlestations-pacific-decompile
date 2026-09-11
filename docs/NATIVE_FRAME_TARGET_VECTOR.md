# Native frame-target vector

The complete `00B1F970`, `00B1F9F0`, and `00B1FB90` operations now use actual
native storage. They cover 128, 88, and 23 installed bytes respectively. The
header is the three DWORDs at frame-target owner `+1C`: data pointer, signed
count, signed capacity. Each row contains four opaque DWORDs. Its semantic
field names remain unknown; this packet does not introduce ownership for them.

`include/bsp/native_frame_target_vector.hpp` declares `NativeFrameTargetVectorRow`
and `NativeFrameTargetVectorStorage`, without initialization or destruction.
The three functions accept a reference to that actual header:

| Original address | Source interface | Original ABI |
| --- | --- | --- |
| `00B1F970` | `reserve_native_frame_target_vector_00b1f970` | ECX header, signed capacity on stack, RET4 |
| `00B1F9F0` | `resize_native_frame_target_vector_00b1f9f0` | ECX header, signed count on stack, RET4 |
| `00B1FB90` | `destroy_native_frame_target_vector_00b1fb90` | ECX header, RET |

The source interfaces use C++ cdecl reference parameters. They are not binary
replacements for the original call convention. No stable EAX result is claimed.

## Established behavior

Reserve clamps its requested capacity to at least one, then uses a signed
growth comparison. The allocation size is the wrapped DWORD product of the
request and 16. It invokes the existing shared `singleton_lifetime_allocate`
provider with equal native and host byte counts. That provider implements the
`00BF55BE -> 00BF681B` malloc/new-handler/retry-or-throw contract.

After allocation, reserve reads current count and copies each current row as
four ordered DWORD loads and stores. It reloads the current data pointer for
each row and current count for each loop comparison. A null destination skips
that row's loads and stores. It frees the current header data pointer using
`singleton_lifetime_free`, corresponding to `00BF6989 -> 00BF65AC`. Only after
free returns does it publish replacement data and capacity, in that order.
It preserves the current count, including changes made by an allocation
callback. No new rollback behavior is supplied.

Resize reserves when its signed requested count exceeds current capacity.
It then reads current count, zeroes four DWORDs in each newly exposed row,
and decrements current count while shrinking. The final store writes the
requested count. Shrinking preserves removed row bytes. Row destinations use
wrapped DWORD address arithmetic and the original null-address check.

Destroy calls complete resize with zero and then frees the current data
pointer. The pointer remains dangling and capacity remains unchanged. The
routine does not destroy row contents or clear the header.

Each reached nonnull address requires valid backing memory. Arithmetic wrap
is preserved, including an allocation request that becomes zero bytes; no
overflow rejection or recovery for undersized allocations is introduced.

## Native evidence and analysis repairs

Every live byte query verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 language and image base through `bsp.py ghidra`.
The full spans match installed PE SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Exact bytes, full assembly, ABI, source hashes and evidence artifact hashes
are in `reports/native_frame_target_vector_audit.json`.

The saved Ghidra bodies still have two false CALL_RETURN sites. The primary
integrator must repair them under the Ghidra write lock, then annotate and
refresh exports:

| Function | CALL_RETURN site | Missing reachable bytes | Effect |
| --- | --- | --- | --- |
| `00B1F970` | `00B1F9DC` | `00B1F9E1..00B1F9EA` | Stack repair and publication of replacement data/capacity |
| `00B1FB90` | `00B1FB9D` | `00B1FBA2..00B1FBA6` | ADD ESP,4; POP ESI; RET |

The reconstruction used complete installed/live byte spans decoded as x86,
including both missing tails. This worker made no Ghidra changes.

## Verification and integration

`scripts/build.ps1` passed with MSVC Win32 `/W4 /WX /fp:strict` after registering
this source through an ignored local CMake hook. Both existing CTests passed;
all eight seed functions matched live Ghidra and installed bytes.

An ignored fixture executes all 239 original bytes, relocating only the five
outgoing/internal call displacements. Native internal calls stay within the
complete original bodies. The external allocator call uses a byte-count ABI
adapter to the same complete shared library allocator; free calls the shared
library free provider directly. It does not execute the original CRT's
exception-object construction or claim its exception RTTI ABI.

The source side links the strict built `bsp_core.lib`; no owned function is
copied or recompiled for the fixture. The audit proves archive membership and
complete COFF-to-linked byte equality after resolving every relocation for all
three owned functions and both allocator/free providers. The entire executable
text section matches before and after execution. Every byte of each original
postimage also matches before and after execution.

Six focused comparisons passed: grow/copy/zero/shrink/regrow/destroy; signed
clamp and no-growth; size wrap to actual `malloc(0)`; actual new-handler
return-zero followed by `bad_alloc`; an actual new-handler exception; and null
row skipping with signed shrink. The two failure cases retain callback header
and row mutations. Heap-event observers only record and forward actual UCRT
malloc/free calls, then restore their IAT cells. Normalized event sequences and
row/header postimages match exactly. No permanent test was added.

The handoff worktree retains `local/build_frame_vector.ps1`,
`local/build_frame_vector_probe.ps1`, `local/audit_frame_vector.py`, the ignored
fixture sources and `build/frame-vector-check/` artifacts. The primary must
register `src/native_frame_target_vector.cpp` in its shared CMake source list.
To relink against the primary library, use the recorded fixture command with
the primary `build/win32/Release/bsp_core.lib`; retain the pinned worker fixture
source and report, and regenerate linked/runtime proof for the new executable.

This packet establishes vector operations only. Append `00B1FA50`, the
frame-target owner lifecycle, renderer composition and game execution are
outside its validation. Descriptive source names are evidence-based hypotheses.

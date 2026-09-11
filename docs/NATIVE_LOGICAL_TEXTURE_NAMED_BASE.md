# Native logical texture named base

This packet reconstructs five complete entry bodies against actual Win32 owner
and name-header storage. The source is
`src/native_logical_texture_named_base.cpp`; declarations are in
`include/bsp/native_logical_texture_named_base.hpp`. Descriptive names remain
hypotheses. The explicit C++ interfaces are not binary replacements.

| Native entry | Complete bytes | Original ABI | Reconstructed behavior |
| --- | ---: | --- | --- |
| `00B34120` | 176 | ECX owner; stack name header, COM, flags; EAX owner; RET 0Ch | Named base construction |
| `00B34230` | 37 | Same | Complete base construction, then profile `00D5F228` |
| `00B33F50` | 103 | ECX owner; RET; EAX unspecified | Name destruction and reference-counted base action |
| `00B34010` | 5 | ECX owner; tail JMP to `00B33F50` | Complete destructor forwarding entry |
| `00B33E40` | 4 | ECX owner; EAX owner+8; RET | Name-header address, without any dereference |

The five entries total 325 original bytes. This packet does not claim or
reimplement `00B3F4C0`. Its existing full body is
`destroy_native_buffer_diagnostic_record_00b3f4c0` in
`native_physical_buffer_owner.cpp`. The focused fixture composes that existing
symbol when checking the shared diagnostic record contract.

## Actual storage and construction

The constructor accesses this owner prefix; bytes beyond it are not initialized
or owned by this packet:

| Offset | Behavior |
| --- | --- |
| `+00` | Profile DWORD: `00CEB130`, then `00D5F1F4`; wrapper replaces it with `00D5F228` after success |
| `+04` | Reference count initialized to one |
| `+08/+0C` | Native eight-byte name: DWORD length and data pointer |
| `+10` | Supplied borrowed COM pointer; no AddRef or Release |
| `+14/+18` | Both DWORDs remain untouched |
| `+1C` | Supplied flags copied verbatim |
| `+20` | Previous shared texture serial DWORD |

Construction first clears both actual name fields. An old name is abandoned;
this entry is not assignment. The equality comparison at `00B3416E` occurs
after clearing, so a source equal to `owner+8` becomes empty and skips copying.
No alternate C++ string or owner object is introduced.

For a different source header, the constructor reads its current length and
calls the existing `resize_native_string_header_0041dd40` with preserve=true.
After that call returns, it tests the **current source length**, then captures
the **current destination length**, **current source data**, and **current
destination data** at `00B3418A..00B3418F`. Thus storage callbacks can modify
either actual header, and a source changed to zero suppresses the final copy.
The implementation does not retain a source string snapshot across allocation
or release.

The direct call at `00B34195` reaches native `00BF7680`. That CRT body detects
backward overlap at `00BF7694..00BF769A` and enters the backward-copy path at
`00BF7844`. The direct reconstructed copy therefore uses `std::memmove`,
including when allocation has made the current buffers overlap. The complete
869-byte CRT body is pinned as evidence. This does not change the existing
shared resize implementation or its allocator contract.

COM and flags are stored only after copying. `00B341AF` reads the actual
shared DWORD `0108D6E8`, `00B341B4` stores it at owner+20, and `00B341B7`
increments the current shared DWORD modulo 2^32. The host interface borrows a
reference to that same caller-supplied DWORD; callers of named and unnamed
constructors must share it. No private counter is created. The increment
re-reads the DWORD after the owner store instead of using an earlier snapshot.
`00B34230` consumes no additional serial.

`NativeStringStorage` remains the explicit storage boundary. In the fixture it
is backed by the existing `PooledStringStorage` and an actual `SizedStoragePool`.
There is no independent allocator or persistent storage field inside the owner.

## Destruction and original unwind states

`00B33F50` installs `00D5F1F4`, captures name data at +0C, and skips the length
read if that pointer is null. Otherwise it captures current length+1 with
DWORD wrap and returns that captured buffer through storage. Both name fields
remain unchanged, including changes made during release. Count, COM, flags,
serial, untouched metadata and trailing owner bytes are preserved.

Normal destruction then performs the complete seven-byte `00BD30F0` action:
store `00CEB130` at owner+00. It does not decrement a count or release COM.
The captured five-byte `00A81880` unwind thunk jumps to the same base action.

| Entry | Original handler / FuncInfo | Original unwind map |
| --- | --- | --- |
| `00B34120` | `00CBE0C3` / `00DF6940` | `00DF6930`: state 1 -> 0 via `00CBE0B8` (current name destruction); state 0 -> -1 via `00CBE0B0` (base action) |
| `00B33F50` | `00CBE098` / `00DF690C` | `00DF6904`: state 0 -> -1 via `00CBE090` (base action) |

Both FuncInfos have zero catch-map entries. The reconstruction uses armed
cleanup-only scope guards, so no catch/rethrow is introduced. Constructor state
0 is armed before the name fields are cleared; state 1 follows their
initialization. A failed allocation destroys the current name before the base
action. No later COM, flags, serial or derived-profile store resumes.

The destructor guard is disarmed before its normal base action, matching the
state=-1 store at `00B33F9A`. The constructor guards are disarmed on successful
completion. The explicit storage release interface is noexcept, so the host
domain does not reproduce an original singleton-getter exception during a
destructor release; that native destructor-unwind path is statically captured
but not dynamically compared.

## Focused validation

`local/named_base_diff/proof.json` contains the independently checkable original
preimages, all loaded postimages, relocation/bridge destinations, source and
artifact hashes, linker providers and raw comparison traces. Its SHA-256 is
`d64ffa7e768d0a14795f05d281588ab052bd144e2fda258549b2275b8c63561f`.
The tracked audit copies the relevant evidence and pins that local proof.

All 19 freshly captured spans, totaling **1,592 bytes**, matched the installed
PE and the verified Ghidra program `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`. Each CLI read verifies project, program, language
and image base. The installed EXE SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The single ten-step caller fixture passed **1,410 equal behavior words**:

- Named profile, embedded NUL, untouched metadata, borrowed COM, and serial wrap.
- Name-header self-aliasing and a source header at owner+14/+18.
- Allocation-time changes to the current name and source headers, including a
  current zero source length that suppresses the final copy.
- A real allocation exception through the original constructor's unwind map,
  current-name cleanup and base cleanup, with no later constructor publication.
- Wrapped release size zero and release-time mutations of the actual header.
- Both destructor routes and the already integrated diagnostic cleanup helper.
- Name-address EAX for normal, null and wrapping inputs, without a read.
- Backward overlap across adjacent real byte-aligned pool allocations.

The native side executes the original five bodies and original string
resize/destruction algorithms. Exactly five service entries are bridged:
storage getter, storage allocate, storage release, the overlap-safe CRT copy,
and host FH3. Eleven absolute operands are accounted for. Two registration
operands target executable-resident FH3 adapters loading the original relocated
FuncInfo; original constructor cleanup funclets and maps execute unchanged.
The original registered-handler thunks themselves are captured but not executed.
The original CRT copy body is captured as evidence, not executed.

The actual host CxxThrowException import reports exactly one throw on each
side, with equal raw traces; no synthetic rethrow is omitted from comparison.
All 19 postimages match their preimages plus only the recorded changes.
Eight source pins, ten linker providers and 24 artifacts are checked. The five
new production functions come from the exact production source object;
existing string, pool and diagnostic dependencies come from the frozen
worktree `bsp_core.lib`.

The new source and fixture compile as MSVC Win32 C++17 with `/EHsc /fp:strict
/O2 /Oy- /MD /W4 /WX`. After seed verification, `scripts/build.ps1` passes both
existing tests: `reconstructed_math` and `native_math_differential`. No tracked
test, shared CMake, ledger or Ghidra mutation belongs to this worker packet;
the primary integrator handles those shared writes.

## Evidence boundaries

This establishes complete entry reconstruction, build validation and the
bounded original-byte caller comparison. It does not establish general ABI or
gameplay compatibility. Raw profile constants are stored; this packet neither
binds nor calls the complete texture vtables. Native singleton initialization
and sized-pool instruction execution remain existing host boundaries.

Storage must satisfy the existing nonnull allocation, valid accessible bytes
and sized-release contracts. A native zero-byte copy is omitted under the
shared host policy. The shared resize helper retains its established C++ copy
policy; overlapping final constructor buffers use the explicit memmove above.
Malformed inaccessible storage, concurrent mutation, and exceptions from the
original pool getter during destructor release are not fixture-covered domains.

# Actual resource cache pair construction

Address: 00b7f290.

`construct_native_resource_cache_pair_00b7f290` reconstructs the complete
161-byte body against the caller's actual 0Ch pair and the shared raw string
pool publications. It preserves captured normal-input cleanup, current-header
unwind cleanup, and the completed-pair flag. It does not add a resource
reference, release the resource, or implement a cache tree.

The original ABI is ECX destination, EDX raw resource, stack by-value native
name `{DWORD length, pointer data}`, EAX original destination, RET8. The new
C++ API exposes the actual input-header address so the original self-identity
comparison and later current-header cleanup remain expressible. It does not
copy that header into another C++ owner or claim the original binary ABI.

## Evidence and classification

Live project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`,
was verified by each `bsp.py ghidra` batch. Bytes `00b7f290..00b7f330` match
the installed binary SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The function body SHA-256 is
`36810d3cd7124a84c3d9b5e2d8f64dd65edfe0b2023e9405e6b21e125042924c`.
All four direct calls were checked against the live listing; the complete
listing has no flow gaps. The sole current containing caller is the loader
`00b80720`, at call site `00b80910`.

The destination consists of the game's 8-byte pooled native string at `+0/+4`
and a borrowed resource pointer at `+8`. This packet recovers that specific
custom-name/resource ownership boundary. It does not reconstruct generic STL
pair, tree, allocator, iterator or rebalancing algorithms. Compiler template
provenance is not established; the descriptive name is a hypothesis.

The [frontier audit](NATIVE_RESOURCE_MANAGER_FRONTIER_BC.md) retains the raw
resource manager's remaining parser/factory/tree/reader dependencies and the
distinction from the useful existing semantic `GameResourceManager` projection.

## Exact field and cleanup schedule

At `00b7f2b0/ba/be`, the function captures resource, input pointer, and input
length before zeroing destination `+0/+4`. It then compares destination with
the by-value input header's address. Self-identity skips resize/copy, but still
stores the resource and returns the captured incoming buffer. No implicit
destructor clears either header.

For distinct headers, the function calls complete `0041dd40` with the captured
length and preserve flag 1. If that captured length is nonzero, it copies from
the captured input pointer using the current destination length/data. BF7680
allows backward overlap; source uses `memmove` and omits a zero-byte call under
the existing raw-string source policy. Valid nonzero ranges remain the caller's
responsibility; no length sanitization or fallback allocation is introduced.

After storing resource `+8`, it sets the completed-pair local flag and changes
EH state to 0 before normal incoming-buffer return. That return uses captured
input pointer and captured length plus one, with unsigned DWORD wrap. It calls
the actual `00419cc0` getter on every nonnull return, then `00bd1510`, borrowing
the same pool, shutdown gate and raw lifetime-manager publication cells used by
the existing raw string implementation.

Handler `00cc2051` selects descriptor `00dfb1c8`; the two-state unwind map is
at `00dfb1b8`:

| State | Action | Result |
| --- | --- | --- |
| 1 | `00cc2030` -> `0041dd20` | Destroy the **current** input header at EBP+4, then descend to state 0 |
| 0 | `00cc2038` -> `00b7e8f0` | If the completed flag is set, clear it and destroy the **current** saved destination name |

`00b7e8f0` uses the same current pointer/length and sized-pool return schedule
as complete raw `0041dd20`, which the source composes directly. Initial
allocation/copy failure reaches state 1 with the completed flag clear, so it
cleans the current input without adding destination rollback. A failure during
normal input return reaches state 0 with that flag set, so it cleans the
completed destination. No resource ownership operation appears in either path.

Both resize and destruction use `NativeStringRawPoolContext` overloads. The
normal captured-buffer return calls the same concrete getter/return functions;
it does not narrow getter failures through a `noexcept` storage adapter.
Secondary exceptions thrown by cleanup, original FH3 dispatch, SEH faults and
invalid raw storage are outside the proven C++ exception domain.

## Validation

The standard strict Win32 build passed. After verifying native seeds, both
existing CTests passed: `reconstructed_math` and `native_math_differential`.
No permanent test or framework was added.

One ad hoc fixture addresses the concrete exceptional-ownership risk. It links
the exact `native_resource_cache_pair.obj` produced by the current CMake
build, checks that object against its unique current `bsp_core.lib` member,
and retains its link MAP. The original normal body and two original unwind
actions (33 additional code bytes) use four shared test-only support contracts.
The source body uses those same raw-signature support contracts in this fixture.
This isolates the new body; it is not a replay of the underlying pool/string
implementations.

The three comparisons are one normal scenario and two cleanup states:

1. During resize, mutate the input header and destination length. The body
   copies the original buffer and returns that original buffer with its original
   length plus one, while the changed input header remains changed.
2. Fail initial resize after mutating the input header. Original state-1 action
   and source catch both destroy the changed input, with no destination cleanup.
3. Fail normal input-buffer return after construction. Original state-0 action
   and source catch both destroy the completed destination name.

The normal case executes original `00b7f290`. Exceptional cases invoke the
original action code directly with the metadata-established state, then compare
against source exceptions. They do **not** throw through the native body or
claim original FH3-dispatch parity. Expected output is generated and frozen
by original mode before source mode; a separate replay passes against that
fixed baseline.

Two initial fixture attempts failed during compilation due to test-only inline
assembly issues (reserved `out` identifier and EBP manipulation in a regular
C++ helper). Their directories are retained. The corrected fixture uses a
naked action adapter. Production source was unchanged by those fixture fixes.

The companion [report](../reports/native_resource_cache_pair.json) records
body/call/unwind evidence, source dependency boundaries, build and fixture
hashes, fixed expected output, and artifact paths. Local fixture helpers and
all attempts remain under the worker `local/` directory for integrator replay
and retention. No Ghidra mutation, installed-game change, or gameplay validation
was performed. Primary integration will apply reviewed annotations separately.

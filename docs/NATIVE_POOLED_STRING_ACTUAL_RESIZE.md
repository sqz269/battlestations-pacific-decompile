# Actual pooled-string header resize (`0041DD40`)

`resize_native_string_header_0041dd40(void*, NativeStringStorage&, uint32_t, bool)`
executes the complete resize body on the supplied native eight-byte header.
The length is the DWORD at `+0`; the data pointer is the DWORD at `+4`.
It does not initialize the header, construct an owner, copy fields into a
temporary, or cast the header to `NativeString`. `NativeString::resize_0041dd40`
now delegates to this body using its own actual address. The existing storage
interfaces and their allocation policies are unchanged.

This permits a live embedded header, such as the name at the start of a render
resource record, to use the same resize implementation. This packet does not
implement the name-copy fragment at `00B3051D..00B30545`, the enclosing record
assignment, or any alias-list operation. Existing fresh-header consumers are
not changed here.

## Evidence and original ABI

The full native span is `0041DD40..0041DDE4` (exclusive end), 164 bytes.
All 164 bytes were read from the saved `bsp` project and compared with the
installed executable. They match. Each live read used `bsp.py ghidra`, whose
project guard verified `bsp`, `/battlestationspacific.exe`, the x86 language and
image base `00400000`. No Ghidra state was written.

The original interface is `__thiscall`: ECX points to the actual header,
stack DWORD 1 is the requested unsigned length, and the low byte of stack
DWORD 2 controls preserve-copy. Both returns use `RET 8`. ESI holds the header,
EDI the requested length, and EBX the replacement buffer. The procedure has
no local exception handler. The host C++ function returns `void`; incidental
native EAX values are not an advertised result. Its `bool` parameter expresses
the low-byte decision, rather than an arbitrary 32-bit native stack value.

The existing descriptive name `BSP_NativeString_Resize` is retained. The live
stored prototype remains `undefined BSP_NativeString_Resize(void)` and is not
the ABI evidence: the parameter and calling-convention conclusions above come
from the assembly. The old prototype and complete existing comment were
captured in the audit for an integrator to preserve during later annotation.

| Native instructions | Behavior retained in the shared implementation |
| --- | --- |
| `0041DD43..0041DD54` | Capture the initial length. Equal requested length returns before reading the pointer. In particular, equal zero does not release a stale pointer, and equal nonzero does not repair a null pointer. |
| `0041DD56..0041DD7F` | For a changed request of zero, capture data and pass `initial_length + 1` to release if data is nonnull. Then store null data and zero length, overwriting any header changes made by release. |
| `0041DD82..0041DD95` | Allocate unsigned `requested + 1` before any old-data release. The replacement pointer remains unpublished. |
| `0041DD9E..0041DDB6` | If preserve is set, reread the current length after allocation, choose the unsigned minimum with requested length, and copy from the current data pointer. |
| `0041DDB9..0041DDD0` | Reread data after the copy. If nonnull, reread length and release that captured data with current length plus one. |
| `0041DDD5..0041DDDA` | Publish replacement data, publish requested length, then write the zero terminator using Win32 address addition. |

An allocation callback may replace the actual length and data. The preserve
copy and release must use those replacement fields. A release callback sees
the old/current header and the copied replacement bytes before final
publication; any header edits it makes are overwritten by the later native
stores. Allocation failure propagates without restoring callback mutations,
releasing the original string, publishing the replacement, or inventing a
cleanup guard. The function does not add a maximum-length check: request plus
one and the terminator address use the native 32-bit arithmetic.

## Existing host boundaries

The following policies already belong to the repository's host interfaces
and remain explicit:

- `NativeStringStorage` supplies allocation and a `noexcept` release.
  `PooledStringStorage` still calls the existing `SizedStoragePool` allocate
  and release methods. Supplying this object replaces the original repeated
  `00419CC0` global singleton lookup; this helper does not reproduce singleton
  first-use side effects or create a new process-global pool.
- The native calls `memcpy` even when the chosen count is zero. The existing
  C++ implementation skipped that call to avoid a standard-library call with
  a null source. The new shared body retains this omission. The focused
  fixture accounts for the one native zero-byte copy separately.
- `crt_string_storage()` still allocates one byte for a zero-byte request,
  throws `std::bad_alloc` for a failed `malloc`, and ignores the supplied
  size when freeing. It is not replaced with a different allocator policy.
- The existing `SizedStoragePool` host projection still bounds its arena and
  can throw where the native carve would overrun it. This extraction does not
  change that behavior or claim equivalence outside that established boundary.
- Neither the actual-header helper nor the original host method repairs an
  invalid header. Storage must provide a suitable nonnull returned pointer
  for the requested operation; nonzero copies still require valid ranges.
  The helper adds no recovery after an invalid pointer access.

The original `NativeString` initialization, move behavior, assign constructor,
copy fragment, explicit `release_to`, and duplicate helpers are unchanged.
In particular, `release_to` remains its existing explicit cleanup operation;
it is not substituted for resize's equal-zero early return.

## Verification

The private focused fixture executes the full original 164-byte function from
the installed executable in a sparse Win32 image. It checks the installed
bytes again before executing, then patches four separately verified five-byte
preimages: `00419CC0` to the supplied storage object, `00BD1120` and `00BD1510`
to recording storage operations, and `00BF7680` to the host copy operation.
These are explicit fixture service boundaries; the original pool and CRT
bodies do not execute in this fixture. Every native allocation and release
flag is checked to be one. No instruction inside the resize body is changed.

One fixture run compares original execution and the C++ actual-header helper
over shrinking, growing, disabled preserve, empty-to-nonempty, equal-length
null/stale headers, both zero branches, allocation replacement of live fields,
a throwing allocation callback, release mutation followed by publication,
and the wrapping `FFFFFFFF` request. It compares release sizes, pointer roles,
header state, replacement-buffer bytes and call order. The result was:

```text
PASS: full original 0041DD40 versus actual-header helper: 483 normalized words; 12 getter boundaries; 1 native zero-copy omission.
```

The focused fixture and changed C++ source compile with MSVC Win32, `/O2`,
`/EHsc`, `/fp:strict`, `/W4`, and `/WX`. `scripts/build.ps1` passed, then all
eight `verify-seeds` spans matched and the final build passed both existing
CTest checks (`reconstructed_math` and `native_math_differential`). No new
tracked tests or CMake changes were needed. The existing `NativeString`
allocation/release checks run through the new shared implementation.

The private preparation, fixture and build scripts are under `local/` with
the prefix `native_pooled_string_actual_resize` (build/preparation script
prefixes differ by their verbs). The report pins their hashes, native spans,
original annotations, implementation files and build outputs. It also records
the base revision `a8c184b` and original `native_string` source blobs.

This is a reconstructed and build-tested full body against actual header
storage, with a focused native-byte differential test across explicit host
boundaries. It is not a drop-in binary replacement, original pool/CRT ABI
validation, or game/runtime validation. The integrator owns shared ledger
and Ghidra annotation updates.

# Native camera frustum storage

This packet reconstructs three complete raw storage routines used by the camera
frustum path. It does not promote the camera hierarchy, dirty-cache getters, or
renderer camera preparation. Existing semantic camera APIs remain unchanged.

| Original body | Bytes | Native ABI | New explicit entry |
|---|---:|---|---|
| `00B650B0..00B65106` | 87 | ECX=float4; RET; EAX=same storage | `normalize_native_camera_plane_00b650b0` |
| `00B653F0..00B65557` | 360 | ECX=six packed float4 records; stack=matrix pointer; RET4; EAX=destination | `extract_native_camera_frustum_00b653f0` |
| `00B658E0..00B6598A` | 171 | ECX=plane set; stack=packed planes, flags DWORD; RET8 | `assign_native_camera_frustum_planes_00b658e0` |

The source uses MSVC Win32 naked assembly for the actual x87/SSE instructions and
32-bit address arithmetic. The first two entries add an EDX context; assignment
has an explicit unused EDX parameter to retain its stacked arguments. Names are
descriptive hypotheses. These interfaces are not general drop-in game ABI claims.

`NativeCameraFrustumContext` borrows the actual `0109DD78` CRT dispatch word and
the original `00D7A208` negative-zero word. Its first member is the existing
`CameraAxesCrtAccess`, with its handler fixed by construction to the complete
`legacy_crt_87except_00c27489` provider. It accepts no caller-selected arithmetic
or error callback. The existing `LegacyCrtMathRuntime` must already be bound to
the actual `00E16BD0` word and the owning runtime's real per-thread errno accessor.
All borrowed storage and both immutable contexts must outlive calls. Context
construction does not publish or change the existing CRT runtime binding.

Normalization calls the full existing `camera_vector_length_00419440` schedule,
including its complete private `00BF7030` square-root path. It does not use the
FSQRT-only kernel in the older semantic camera-frame helper. The shared provider
retains the current CRT dispatch reads, classification, control/status handling,
and full C27489 composition. The public standalone square-root wrapper is not
called separately: the complete shared private square-root body is reached from
the length provider. The existing C27489 provider uses the installed default
matherr semantics, real errno, and Win32 RaiseException; the new packet does not
introduce replacements for them.

After spilling the length to binary32, `FCOMI`/`JBE` chooses a binary32 positive
reciprocal only for ordered positive length. Nonpositive and unordered lengths
choose positive zero. All four coefficient multiplies and stores still execute:
this distinction matters for NaNs, infinities, signs of zero, and x87 exceptions.
The original floating-point instruction sequence is unchanged. The compiled
normalizer is 87 bytes with one provider relocation.

Extraction writes six packed 16-byte planes in forward order. For matrix columns
`c0..c3`, the planes are `c3+c0`, `c3-c0`, `c3-c1`, `c3+c1`, `2*c2`, and `c3-c2`.
Each output store precedes the next native matrix read; overlapping input/output
is not snapshotted. The six D coefficients are negated by the original ordered
`SUBSS` operations from negative zero, including the original interleaving of
the last two stores. The six normalizations then run in order. The explicit
context uses one added stack word, integer reloads before each normalization,
and the borrowed constant load. Every original instruction has an audited
compiled-instruction mapping. The resulting body is 389 bytes with six calls.

Assignment copies six float4 records to six 20-byte `NativePlaneRecord` rows.
It captures the flags DWORD after the first FLD and before the first FSTP, then
performs four FLD/FSTP pairs followed by one full flags store for each row.
Caller B70710 passes 7; the full entry preserves every supplied flag bit. Rows
6..15 and count at +140 remain unchanged. Unaligned and overlapping views keep
the original forward behavior. The compiled 171-byte body is identical to the
original, with no relocations. No function catches a floating-point exception or
rolls back earlier writes.

Verification is recorded in `reports/native_camera_frustum_audit.json` and the
immutable ignored `local/camera_frustum` and `build/camera-frustum-probe` artifacts.
The fresh configured Ghidra target was checked by `bsp.py ghidra` before every
query. Twenty-five live/PE-image spans total 2,737 bytes, including all 618 owned
bytes, full CRT entry/helper evidence, the globals and constants. The dispatch
word is zero-filled PE virtual storage; it is distinguished from file-backed
bytes. The installed executable SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The strict Win32 build used an ignored CMake source hook, passed both existing
CTests, and verified all eight native seeds. No permanent test was added. One
focused probe executes 16 original/full-library pairs: finite and signed-zero
normalization, NaN/infinity/denormal/precision behavior, current CRT word changes,
forward and byte overlaps, flags and count preservation, and exception partial
writes. It compares complete 2,048-byte arenas, stack cleanup, nonvolatile
registers, x87 control/status/tag/opcode and all eight 80-bit register payloads,
data pointers/MXCSR, plus instruction-mapped FIP or exception EIP.

The original 618 bytes remain intact except two declared operand relocations:
the normalizer's external length call goes through an integer-only context
bridge to the actual archive length provider, and extraction's negative-zero
address refers to its read-only copied original word. Original extraction calls
original normalization directly at the same image delta. Consequently this is
complete original **owned-body / actual-library-provider composition**, not
execution of original CRT machine code. Both paths use the same actual library
CRT implementation and host errno. The mapped global words are concrete copies
of original storage with explicitly selected current values; no game process is
attached or claimed.

A hardware execution breakpoint observes the unchanged complete C27489 entry.
It changes no code or result, and resumes the trapped instruction using RF.
Quiet NaN reaches type 7 and sets actual errno to 33; extraction reaches type 8
six times; current dispatch bypass suppresses that call and retains errno 123.
The unmasked-precision cases instead trap in the shared length provider before
C27489. The unmasked assignment sNaN traps after two coefficient stores. Both
paths have matching partial storage and exception state. The probe does not
claim runtime coverage of C27489's Win32 RaiseException branch.

All 16 reached complete owned/provider symbols were checked from their exact
archive members through every COFF relocation to linked bytes and runtime
postimages: 3,340 bytes and 62 resolved relocations. Seventeen snapshots preserve
the whole fixture executable text and all three original bodies. Loaded runtime
module paths, Win32 image sizes and physical SysWOW64 file hashes are pinned.
Whole library/provider and source/header pins make the handoff reproducible.

Remaining limits are the existing shared CRT contract: new constant/name pointer
identities, rebuilt-process errno and exception runtime, and documented FPIEEE
fields rather than unspecified reserved stack bytes. Full-stack x87 faults,
arbitrary invalid pointers, every control/MXCSR combination, and all OS exception
modes were not separately sampled. No renderer behavior, camera hierarchy,
rendering parity, or game validation is claimed. Ghidra, shared metadata, and
permanent build configuration were not changed by this worker.

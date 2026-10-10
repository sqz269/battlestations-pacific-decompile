# CC12 instance-generator serial process owner readiness

The original `0108FD30` cell has a loader-zero DWORD preimage. Current Source
requires a borrowed reference to that cell but supplies no application/process
owner. The bounded next proposal is one appended DWORD and a stable accessor in
the existing `GameNativeRendererScalarProcess`, subject to primary replay and
acceptance of this evidence. This packet implements no owner or generator
activation. Its current Source baseline remains
`cf99be09f982c35c5fa15c8747a43650a50cddbc` throughout.

## Existing generator contract

The retained complete `00B451D0` body is 339 bytes: original `ECX` is the material
effect, stack arguments are section/opaque input, and it returns with `RET 8`.
`00B85610` is a 30-byte wrapper (`ECX` section, one stacked opaque argument,
`RET 4`) that checks section `+20` material and material `+7C` effect before
calling attachment. The current subset Source reaches it after section/layout
publication at native site `00B94468`.

The binding is `0x10` bytes: profile `00D619F8`, count at `+4`, serial at `+8`,
and retained generator at `+C`. The retained listing establishes these exact
operations, with `ESI = 1` on both selected building/generic paths:

| Native site | Operation |
| --- | --- |
| `00B452A7` | Null binding allocation branches around the serial accesses. |
| `00B452BB` | `MOV ECX, [0108FD30]` |
| `00B452C1` | `MOV [EAX+8], ECX` |
| `00B452C4` | `ADD [0108FD30], ESI` |

Thus the binding receives the old DWORD before the live process cell advances.
The recorded increment has no lock prefix or interlocked call. Empty or
unrecognized generator names, a missing generator, and the null-binding branch
do not consume the serial. This does not characterize the safety of subsequent
operations on a null binding. DWORD arithmetic wraps; no uniqueness guarantee
across wraparound or concurrent calls is introduced.

Current `NativeInstanceGeneratorContext` requires
`volatile std::uint32_t& binding_serial_0108fd30`. Source attachment copies it into
binding `+8`, then reads/adds/stores the cell before companion registration and
generator/section publication. Later registration failure retains the completed
effects and advanced cell; there is no serial rollback. Existing generator and
binding destructors release their owned fields without touching this cell.
These are retained/current Source observations, not a newly executed Native path.

## Data ownership and original preimage

The authorized exact-address `ghidra typed-flow 0108fd30` query verified
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, `x86:LE:32:default`, and
image base `00400000`. At modification 32 before and after, it reports a defined
four-byte `DataDB` beginning exactly at `0108FD30`, with no instruction or
function at/containing the address. It returns no data value or block mapping.

The separate Root-owned `memory-metadata` facade attempted only containing block
and source-FileBytes metadata. Its script endpoint rejected the request with
`Script execution disabled`; the full failed receipt and exact attempted facade
are frozen. No script opt-in or fallback occurred. Root subsequently reverted
that facade. Ghidra's containing block/source mapping remains unavailable and
does not support the preimage conclusion below.

Root separately authorized an original-PE header-only gate. Before and after
parsing, a streaming identity hash of the 12,223,752-byte original executable
matched
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The stream was used only for identity. Exactly 472 interpreted header bytes were
retained, with complete bytes, hashes and ranges:

| File range, end exclusive | Purpose |
| --- | --- |
| `[0, 64)` | DOS header, including `e_lfanew = 0x120` |
| `[288, 312)` | PE signature and COFF header |
| `[312, 536)` | 224-byte PE32 optional header |
| `[536, 696)` | Four 40-byte section headers |

PE32 image base is `00400000`, `SizeOfImage` is `00E2F000`, and section/file
alignment is `0x1000`. The `.data` header at file offset `0x268` records:

| Field | Value |
| --- | --- |
| RVA / mapped start | `00A08000` / `00E08000` |
| VirtualSize / virtual end exclusive | `00297EDC` / `0109FEDC` |
| SizeOfRawData / raw file offset | `00010000` / `00A08000` |
| File-backed mapped end exclusive | `00E18000` |
| Characteristics | `C0000040` |

The target interval `[0108FD30, 0108FD34)` has RVA `00C8FD30` and section offset
`00287D30`. Its complete four bytes fit inside `VirtualSize` and `SizeOfImage`;
its section offset is beyond `SizeOfRawData`. It therefore lies wholly in the
original loader-zero tail. There is no target file interval to read. No target
DWORD, image-body, import/resource body, or new callee bytes were interpreted or
retained. This establishes loader-initial bits, not the value after constructors
or at a running program's first use.

## Reference and Source scope

The separately authorized exact incoming-reference metadata query used limit 128
and facade line cap 80. Its complete returned output contains two rows:

| Source address | Function | Type |
| --- | --- | --- |
| `00B452BB` | `BSP_MaterialEffect_AttachInstanceGenerator` | `READ` |
| `00B452C4` | `BSP_MaterialEffect_AttachInstanceGenerator` | `READ_WRITE` |

The endpoint/facade returned no total-count, completeness or truncation fields,
and no spill marker. No additional initializer, reset or address-taking row
appeared in this listing. Two rows below the requested limit do not prove the
absence of dynamic writes, indirect references, address escapes or analysis
omissions. No new function body was fetched from those rows; both sites were
already present in the retained complete attachment listing.

A current text census of 4,207 tracked files under `include`, `src`, `tests` and
`cmake` finds exactly two serial matches: the context reference and its use in
attachment. All 44 generator-context and six attachment-name matches are retained
without terminal truncation. No current production construction of a generator
context or serial owner was found. The 17 selected implementation roots have a
525-file repository include closure with 1,014 resolved quoted include edges.
All 539 full Source/context files are frozen against the exact baseline Git
blobs, with raw hashes and explicitly identified CRLF/LF-only equality. This is a
Source include/ownership audit, not a compile, link or Native writer-completeness
claim. The four complete retained attachment/wrapper exports are frozen too.

The historical CK report has 15 captured data intervals; none covers `0108FD30`.
Its recorded fixture source is absent at both checked worktree/main paths and
remains unavailable. Historical fixture success and the neighboring scalar
domains' documented zero-fill do not substitute for this cell's header evidence.

## Bounded next proposal and remaining separation

After primary replay, an owner-only Source packet can append a private DWORD to
`GameNativeRendererScalarProcess` and expose a `volatile std::uint32_t&` accessor
named for `0108FD30`. Initialize that cell once with the established loader-zero
bits. Preserve existing member order, verify old physical offsets during the
Source packet, and return the same cell across renderer/application instances.
The accessor performs no increment, reset, callback, allocation or native work.
It must not alias the separate texture serial `0108D6E8`, introduce an atomic
counter, or derive/reset a private counter from renderer lifetime.

The proposed Source scope is the existing scalar-process header/implementation
plus its packet document/report. It needs no CMake registration and no new test
that merely checks value initialization. Normal Win32 compilation and existing
checks, with focused layout/emission review, belong to that later Source packet.
No C++ file changed and no build, test, probe or runtime execution ran here.

`GameNativeReadOnlyData` accepts only its read-only `CE2000..E07B23` domain and
cannot supply this mutable cell. The canonical mutable CRT owner maps only
`E15000`, `E16000` and `109E000` pages for its admitted CRT cells; it does not map
or own this serial. The existing scalar process already gives the renderer
stable process references. Extending that owner preserves the current Source
model without claiming a binary ABI or a fixed-address Native mapping.

A later generator application-binding packet must borrow this exact process
cell into every `NativeInstanceGeneratorContext`, alongside the actual current
graphics, layout, geometry, string/declaration pools and qualified profiles/name
data. It must retain the same process/application domains and failed acquisition
frames, and retire native bindings/generators before shared drain. Current
metadata-only mesh-field services do not expose or activate that generator path.
Material/effect/cache composition, compiler/sampler preimages, full mesh loading,
parser admission and payload retirement remain separate holds.

The machine report is
`reports/cc12_instance_generator_serial_owner_readiness.json`; full local receipts,
Source/header freezes, verifier and bundle are under
`local/cc12_instance_generator_serial_owner_readiness/`.

# Native mesh metadata and lighting dependencies (CH)

CH adds complete raw-storage source for compressed vertex metadata and material
lighting fields. These remove two dependencies of the unfinished mesh/subset
reader; they do not complete the subset or aggregate parser.

| Address | Bytes | Source routine | Original ABI |
| --- | ---: | --- | --- |
| `B17840` | 232 | `initialize_native_mesh_lighting_record_00b17840` | ECX record; EAX same; RET |
| `B179D0` | 28 | `set_native_material_lighting_record_00b179d0` | ECX material; stacked ignored slot/record; RET8 |
| `B47900` | 4 | `native_vertex_declaration_element_count_00b47900` | ECX declaration; EAX +10; RET |
| `B61D90` | 10 | `set_native_vertex_compressed_format_00b61d90` | ECX stream; stacked allocation; RET4 |
| `B73260` | 11 | `native_mesh_vertex_stream_unchecked_00b73260` | ECX mesh; stacked index; EAX pointer; RET4 |
| `B93390` | 176 | `read_native_mesh_lighting_record_00b93390` | ECX handle; EDX output; RET |
| `B937A0` | 89 | `read_native_material_lighting_field_00b937a0` | ECX unused; stacked material/handle; RET8 |
| `B93800` | 116 | `read_native_mesh_compressed_format_00b93800` | ECX unused; stacked mesh/handle; RET8 |

All 666 ordinary bytes agree between the installed executable and live saved
Ghidra. The audit checks all 210 instruction owners, 30 direct transfers and
one indirect call. There are no support funclets, excluded alignment bytes,
missing owners, or control-flow repairs in this packet. Data evidence includes
the two lighting constants, actual `D61D6C` profile, and the four original bytes
of the existing declaration getter `B48CE0`.

## Metadata behavior

`B93800` selects the current last stream, calls its captured current +24 getter
and obtains the declaration's flat element count through `B47900`. Its unsigned
multiply by `0x20` has two distinct consumers: allocation saturates to
`0xFFFFFFFF` when the high product is nonzero, while the later read length wraps
as a DWORD. The allocator call precedes the wrapping shift. The source uses the
existing host CRT/new-handler boundary, with the recovered allocation size.

After one raw `BE9A20` read, the original rereads mesh+7C and selects the last
stream again. A callback can therefore change the destination stream without
changing the captured source declaration/count. `B61D90` only stores the raw
allocation at destination+50; it does not free an old value, retain a stream,
decode metadata or roll back earlier effects. There is no EH cleanup. The
source acquired record retains an untransferred allocation on failure and
records the separately captured source and destination identities.

The current +24 profile selects the existing complete
`native_logical_vertex_stream_get_declaration_00b48ce0`. Native numeric table
entries are never executed by the reconstructed source. Changed unsupported
profiles fail explicitly at that source boundary. The caller keeps the real
mesh/stream/declaration and reader domains valid across callbacks.

The older `native_mesh_vertex_stream_00b73260` GUI interface adds a six-slot
range check. The new raw interface preserves the complete native unchecked
load with wrapping `mesh + 0x64 + 4*index` arithmetic. Existing GUI callers remain
on their checked interface; the address ledger now points to the complete raw
entry. Neither the original nor the new entry tests the mesh stream count.

## Lighting behavior

`B17840` captures the live `D7A24C` bits before storing the first four words,
then stores twelve positive zeros and the `CE38B8` bits as the final word. The
last constant is captured before stores+38/+3C, as in the listing. SSE moves
preserve the original constant bits; no arithmetic or semantic field names are
invented. The observed constants represent 1 and 10 respectively.

`B179D0` captures the source pointer, sets material byte+10C to1, and performs
17 forward DWORD loads/stores into material+38. The serialized slot argument is
never loaded. This is deliberately not `memcpy` or `memmove`: overlap can cause
later source words to observe earlier stores. Signaling-NaN bit patterns are
copied without floating-point conversion in this setter.

`B93390` calls the existing actual `BE99D0` float reader 17 times and stores each
ST0 result directly with `FSTP32` into the next output word. A narrow naked
helper keeps that x87 return/store sequence intact. `B937A0` reads an unsigned
record count, then repeats ignored-slot read, local default initialization,
17-float read and publication to the same material. Zero records leave the
material untouched. A later read exception preserves the last complete
material record; incomplete local values are not published.

## Validation and scope

`scripts/build.ps1` compiled the new source into the actual MSVC Win32 core and
passed both existing CTests. The ignored CH CMake hook adds pending CC/CE/CF/CG/CH
sources. `cc7` held `cmake/startup.cmake` when this build was configured. This
frozen build precedes tracked registration of those five sources and main
integration; the file lease became available afterward. Remove the cached
`CMAKE_PROJECT_INCLUDE_BEFORE` hook when tracked registration is performed.

One controlled-child fixture copies all eight original CH bodies, relocates
their 30 direct transfers, and executes original-to-original CH calls. Raw
reader/control/float and CRT allocation dependencies are shared source
boundaries. The original metadata indirect call executes the actual four
`B48CE0` bytes at their original address, leaving the immutable D61D6C table
unchanged. The parent reserves that private code band before resuming its own
new suspended child; the child verifies, commits, protects and releases it.
No installed game process, binary or Ghidra code bytes are modified.

The paired checks passed:

- Raw default bits, 17-word forward overlap with signaling-NaN bits, and two
  unchecked getter indices whose arithmetic wraps to readable mesh words.
- Two lighting records containing signed zeros, subnormals, infinities, quiet
  and signaling NaNs: identical material bytes, ordered read events and budget4.
  A separate zero-count pair preserves the complete material preimage.
- A 32-byte metadata field on actual D3D9 logical streams. A read callback
  changes mesh count2 to1, and both original and source publish to the newly
  selected last stream. Payload bytes, read events and remaining budget4 agree.

Two CH source failure observations also passed. A failure during the second
lighting record retains the first material record and budget64. A metadata
read failure retains its allocated block, preserves existing stream+50 values,
and leaves budget32. Fixture cleanup occurs only after these observations.
Native copied-code exception paths are not exercised.

The existing CG HAL fixture runs in the same child with the rebuilt core:
index16/index32 and ordinary/rope vertex readbacks, post-map read failure,
replacement and final owner cleanup all pass. The last metadata allocation is
left on the actual stream for its real native-lifetime source teardown. All
eight canonical companions retire; cache stride and renderer counts reach0,
raw slabs return to32 free slots, and final device/API COM releases reach0.

Fixture corrections were limited to moving inline assembly out of MSVC lambdas
and reserving the fixed helper address before child startup. The initial late
address-reservation failure log is retained. No reconstructed source changed
in response to these fixture issues.

This is original-body agreement within explicit shared-dependency and fixture
boundaries, not original CRT/FH3/SEH or binary ABI replacement. Saturating huge
allocation failures, short reads, every callback/alias permutation, native
hardware faults, unmasked FPU exceptions, cold declaration loads, device loss,
XLive/focus policy, threading and gameplay remain unverified. Raw pool headers
and declaration cache records use the same preconstructed fixture domains
documented in CG; full pool/renderer startup is not claimed.

## Remaining subset and aggregate work

Inspection of `B941D0` identified unfinished texture field `B93D30` and section
finalizer `B85610`, followed by aggregate `B944E0` and parser wrappers beginning
at `B94710`. The existing actual material factory `535320` directly invokes a
callable renderer+48 table entry in `src/native_material_factory.cpp`. The newer
native effect and texture cache interfaces expose `B318B0` and `B319B0` against
numeric native profiles. Subset admission must compose those actual services
and canonical owner registration without substituting a fake callable table.
The existing `B941D0` decompile also has a suspicious cleanup tail; inspect its
listing/EH map before implementation. This packet does not annotate or promote
the subset reader, texture reader or material factory as newly complete.

The report, flow audit, preserved annotation history and integration manifest
are under `reports/native_mesh_remaining_fields*_ch.json`. Descriptive symbols
remain hypotheses. Publication is on the orch4 agent branch only; the broader
game-reconstruction goal remains active.

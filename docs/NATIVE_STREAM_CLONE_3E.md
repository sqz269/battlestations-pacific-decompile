# Actual stream copies for Text flags3E

The new `clone_native_gui_text_model_00b752b0_flags3e` composes the existing
canonical Model/mesh clone with actual separately owned index and vertex
streams. The original flags26 entry retains its stream-sharing behavior.
Both paths copy sections/materials, optional fields and weight names through
the existing implementations. This is geometry ownership; it does not adopt
an ABB2C0 copied Text runtime or retain a copied raw Text pool allocation.

| Routine | Original ABI and range | Coverage |
|---|---|---|
| B729A0 index stream copy | ECX source, EAX new creator, RET; B729A0..B72A65 | Complete normal caller over actual observed logical/renderer profiles |
| B72A70 vertex stream copy | ECX source, EAX new creator, RET; B72A70..B72B19 | Complete normal caller over actual observed logical/renderer profiles |
| B73F50 mesh copy | ECX source, stack destination/flags, RET8; end B74270 | Complete flags26 and flags3E branches; other flags remain outside the interface |
| B742A0 mesh current10 | ECX source, stack flags/unused, RET8; end B742F9 | Allocating flags26/3E branch; share-the-mesh branch remains outside this interface |
| B752B0 Model current10 | ECX source, stack flags/parent, RET8; end B753F1 | Complete flags26/3E,parent0 domain over existing base/resource services |

All instruction listings were read through their last instruction. Stream
returns are one byte; the three outer final RET8 instructions are three bytes.
Live Ghidra access used the verified read-only BSP wrappers for the existing
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. No function, prototype,
comment, name, program or project was changed. Descriptive source names remain
hypotheses; the `_memcpy` library name is preserved in evidence.

## Actual providers and identity

`NativeStreamCloneServices` borrows the existing vertex-owner context, actual
index-creation context, raw mapping context and `GuiNativeGeometryOwners`.
These share the current renderer publication, synchronization state, physical
services and canonical `NativeRenderActualOwners`. The source must resolve to
the matching live `NativeLogicalVertexReference` or `NativeLogicalIndexReference`.
Numeric native table entries select the existing implementations; they are
never called as host addresses. Logical views span ten vertex/nine index words,
renderer views25, and physical mapping views nine words.

The actual factories are B287C0/B288B0, which construct through B4BC00/B4BF30
and publish native renderer registries. They preserve real COM creation and
the required B29670 device-recreation boundary. The private index constructor
uses its original format; it is not replaced by the shared vertex path.
There is no successful fallback for missing device/resource providers.

`GuiNativeGeometryOwners` holds new stream companions alongside its existing
mesh/section/material companions, using its SAME supplied registration callbacks.
Each companion borrows actual +04 without initialization or extra retention.
Native terminal dispatch destroys the actual stream, unregisters renderer/
physical references, returns the original pool slot, then retires that companion.
Source declarations remain the same borrowed/retained native declaration identities.
No `LogicalVertexStream`, `LogicalIndexStream`, byte-vector geometry or duplicate
widget hierarchy is substituted for raw storage.

## Stream call sequence

Both routines capture current F8D394 and its table BEFORE the first source
query. Index queries current20(format), current18(flags), current1C(count),
then captured renderer60(count,flags,format), RET0C. Vertex queries current24
(declaration), current1C(flags), current20(count), then captured renderer5C
(count,flags,declaration), RET0C. The observed query bodies are callback-free
MOV/RET leaves; later native calls reload the source's current table.

Index current0C maps source at B729E7 and destination at B72A00. Vertex current10
maps source at B72AB7 and destination at B72AD0. Each map receives a newly read
SOURCE count, offset0, readonly0, RET0C. Source mapping captures its table before
the first count query; destination mapping captures its table before the second.
These use the existing raw B49B60/B49980 mapping routines and their actual
physical locks, including native optional renderer guards and callback reloads.

After BOTH maps, index rereads source format at B72A0B: 65h means width2,
66h width4, and all other formats width0. Vertex rereads the source declaration
at B72ADB and then its live +CC stride. Both then reread the source count and
multiply with native low-DWORD wrap. `_memcpy` occurs at B72A44/B72AF8 with
destination, source and byte count; native ADD ESP,0C confirms three cdecl
operands. There is no positive-count shortcut, clamping or format substitution.
Valid nonoverlapping mapped extents and native arithmetic are prerequisites.

Source unlock comes first at B72A53/B72B07, then current destination unlock at
B72A5C/B72B10. These compose B49C70/B49A80, including the latter's live +08
clear after its physical callback. No scope guard silently adds an unlock.

## Mesh and Model integration

At AA96EC the source type indexes D5C0B8. The verified Text/type3 cell D5C0C4 is
`3E 00 00 00`; AA9640 supplies the saved parent0, and AA96FC calls Model current10.
The existing glyph route at AB9D87 uses26. Flags3E only adds bits08/10 relative
to26: bit20 still skips child recursion and flags&6 still clones materials.

B73F94 copies nonnull source index60, B73F9E publishes/retains the new stream,
then B73FD8 consumes its creator. A null source index clears/releases current
destination60. B7400F copies each source vertex slot, B7401C publishes it and
B74025 consumes its creator. The vertex loop rereads live source7C; null slots
are not silently skipped. The remaining shared section/material/tail code is
unchanged. B7532F passes the same flags into current geometry; only after it
returns do the existing x87 source17C/178 reads and model association occur.

## Interruptions and cleanup obligations

`NativeStreamCloneAcquired` is diagnostic bookkeeping, not a second owner or
a resumable native stack. It records the raw creator, canonical companion and
registration status, both mapping return values, each map/unmap call phase,
the source identity and the reached native site. The enclosing mesh and Model
creator records remain published too.

The optional factory output publishes a COMPLETED raw constructor result before
the first renderer-array append. For vertex this is the B28809/B2880B boundary;
the index factory supplies the corresponding producer point. Constructor failure
still executes native allocation unwind without publishing. A later factory
exception can leave a creator and partial renderer registration; that raw creator
is not represented as already canonical. The output adds no native retain,
registry mutation or rollback. Existing callers omit the optional argument.

After successful factory return, canonical registration can independently fail.
Its transactional bind leaves no lookup entry on failure; any already constructed
companion remains in the geometry domain and in the acquired record. Once the
interrupted operation is understood, an unregistered companion may be released
directly through its same-count interface. A creator without companion still
requires its actual logical terminal context and completed registry/mapping
obligations. Dropping either is invalid.

For map/unmap exceptions, `call_in_progress` or `unlock_in_progress` means nested
effects may already have happened; it does not mean a lock definitely failed or
that another unlock is safe. The acquired raw identities and actual contexts
must remain alive for diagnosis/cleanup. The caller must not restart the clone.
Completed source/destination unlocks are separately recorded. Mesh publication
clears the creator cell before its release, preventing double consumption if a
terminal callback interrupts. Host diagnostics do not claim original SEH parity.

## Validation

The report `reports/native_stream_clone_3e.json` records numeric CALL rows and
separate indirect-slot evidence, exact bounds and verification results. The
checker verified all 30 direct numeric CALL rows with zero failures; 42 indirect
rows have separate assembly evidence. `scripts/build.ps1` passed in strict MSVC
Win32 Release, including both existing CTest checks. The actual index provider
dependency is commit `c1d5325b5606997b50a3591b362c3cacc789ea63`.

The ignored `local/stream_clone_probe.cpp` fixture compiled with `/W4 /WX`
and `/fp:strict`, linked with `/MANIFEST:EMBED`, and passed using a real D3D9
HAL device. It exercised six vertices with a 12-byte declaration stride,
INDEX16 and INDEX32 streams, unchanged flags26 sharing, distinct flags3E raw
streams and copied bytes. Altering a copied vertex stream left its source bytes
unchanged. All nine canonical companions retired, native renderer stream
registrations became empty, and the original declaration retention returned to
its initial count. A deliberately failed transactional metadata bind preserved
the completed creator and unregistered companion without entering any map;
releasing that companion completed its actual native terminal cleanup.

The fixture supplies raw renderer/declaration storage and actual pools, with
optional synchronization disabled. It has no positive section/material or full
Model/Text adoption case and is not an original-byte differential test. It does
not exercise B29670/device loss, installed rendering or gameplay. No permanent
tests were added.

## Integration analysis

The primary defined the four consumed MOV/RET query leaves at B48CC0/B48D80/B48D90/B48DA0 under the write lock after verifying current Ghidra bytes. Each body is exactly4 bytes; following alignment is excluded. See `reports/orch5_stream_getter_definitions.json`. The original worker no-function list is historical. `GuiWidgetModelCopyRuntime` now connects the established AA9520 flags3E/26 call to this same Model/mesh/stream composition; its positive fixture result is reported separately in the batch report.

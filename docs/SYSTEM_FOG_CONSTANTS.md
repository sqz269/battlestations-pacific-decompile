# System fog constants

`write_system_fog_constants_00b46d97` reconstructs the interior range
`[00B46D97,00B46ED1)` of `00B46A70`. It patches caller-supplied initialized
prefix words and retains native write order, x87 scalar conversions, raw color
words, and camera-owner reload timing. It does not construct the native fog
owner or implement the surrounding system builder.

The original parent enters with ECX as the optional scene owner and EDX as the
camera and returns with plain `RET`. At this fragment, ESI holds the fog owner
captured from camera+184 at `00B46D79`, EDI holds the camera, the stack prefix
begins at ESP+14, and ESP+10 retains the parent's scene value. These are
interior register/stack contracts, not a separately callable native function.
The C++ API has a new calling convention and requires MSVC Win32.

## Retained owner and storage

`SystemFogState` is a compact retained field projection. `color_08`,
`underwater_color_18`, four `directional_colors_28` records and eleven scalars
represent actual native fields. Suffixes are native offsets; C++ member
offsets are different. No native header, allocation, initialization defaults,
factory, destruction or reference-count contract is supplied. Caller-owned
fields must have actual initialized values and remain alive through their last
read. Semantic names are hypotheses; the field offsets and instructions are
direct evidence.

The full owner is separate from `MaterialLighting`. The existing camera
`ambient_rgba` binding represents only the native owner's +08 float4 and cannot
stand in for this projection. The integrator supplies a camera binding to the
full retained owner and captures that pointer at the native boundary. It then
passes a reference to the same camera's live owner slot. Captured and current
owners may differ; the fragment must not resnapshot or recapture its scalar
and directional source from the current slot.

The minimum output capacity is 300 float words, through c74.w. The surrounding
builder normally has 77 registers. The writer never allocates, resizes, clears
or zero-fills storage. It leaves c35.w, c36.w, c72.zw, c73.w, every other
unselected word, and prior builder effects intact. A null captured owner
corresponds to the `00B46D7F`/`00B46D91` gate and skips every fog destination,
without consulting the live owner slot.

## Execution order

Each scalar getter is `FLD m32; RET`; the parent immediately performs one
`FSTP m32`. The implementation preserves those pairs under the caller's x87
control word. A scalar signaling NaN is quieted when invalid is masked, while
color signaling NaNs are copied as raw DWORDs.

| Order | Destination | Captured native field | Getter |
| --- | --- | --- | --- |
| 1 | c72.x | +80 | 00B84DA0 |
| 2 | c72.y | +84 | 00B84DB0 |
| 3 | c73.x | +88 | 00B84E20 |
| 4 | c73.y | +8C | 00B84E30 |
| 5 | c73.z | +90 | 00B84E40 |
| 6 | c35.x | +6C | 00B84CB0 |
| 7 | c35.y | +70 | 00B84CC0 |
| 8 | c35.z | +68 | 00B84CA0 |
| 9 | c36.x | +74 | 00B84CD0 |
| 10 | c36.y | +78 | 00B84CE0 |
| 11 | c36.z | +7C | 00B84CF0 |

Next, four iterations call `00B84FD0` with ECX as the captured owner and stack
index 0..3. The getter returns EAX = owner+28+16*index and uses `RET4` without
bounds checking. The caller copies each lane with an individual integer
read/store pair into c38..41. The C++ fragment uses only those four established
indices and preserves per-word overlap effects.

At `00B46E69`, the native caller reloads camera+184. `00B84C60` returns this
owner+08 in EAX with plain `RET`. The first three lanes go to c37.xyz as
individual raw read/store pairs. The last lane has a significant interleave:

1. `00B46E91`: read the captured color pointer's fourth word into EDX.
2. `00B46E94`: reload camera+184 into ECX for the underwater getter.
3. `00B46E9A`: store EDX into c37.w.
4. `00B46EA1`: call `00B84C90`, which returns the reloaded owner+18 in EAX.
5. Copy four raw read/store pairs into c74.xyzw.

The second reload occurs before c37.w is written. A slot that aliases c37.w
therefore still supplies its previous owner. The implementation keeps the
last source read, slot reload and destination write in one assembly block.
The native EBP restoration at `00B46EA8` is a parent-frame concern, not a fog
field operation; the bounded C++ API does not recreate that stack frame.

## Host errors and remaining boundary

Storage validation is a new host contract. With a nonnull captured owner,
null/short output storage fails before writes. Missing owners at either later
reload return an error at their actual consumption point, retaining preceding
stores. In particular, c37.w has already been stored before a missing
underwater owner is reported. Native code does not null-check these two
reloaded owners and would access invalid memory; a late missing owner must
never become the native no-fog skip.

There are no fabricated getters, default field values or callbacks in the fog
implementation. Native factory/lifecycle and the camera binding remain
required integration dependencies. The surrounding time/wave stage must
capture camera+184 at `00B46D79` after its earlier side effects. Native
exception traps, CPU instruction-pointer exception metadata, concurrent
external mutation, full owner lifetime, ABI replacement and game rendering
are not validated by this packet.

## Evidence and verification

`reports/system_fog_constants_audit.json` records address ranges, installed-PE
and Ghidra byte comparisons, original ABI, existing annotation preimages and
proposed annotations. The saved `bsp` project at `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe` were checked by `bsp.py ghidra` before native
batches. All 15 ranges, totaling 395 bytes, match the installed executable.
No Ghidra names or comments were changed by this worker.

An isolated MSVC Win32 `/W4 /WX /fp:strict /O2` compile passed. One local
differential fixture executes the copied 314-byte native fragment with its
fourteen real getter bodies, rebasing only relative calls and appending a
return at the continuation. It supplies the original ESI/EDI/stack contract;
it uses no getter stubs, game process or game globals. Two variants compare
all 308 prefix words, source-owner preimages and the six x87 exception-status
bits: distinct captured/current owners, then a current-owner slot aliasing
c37.w. Both pass with x87 status 3 (invalid and denormal), including raw color
NaNs and scalar NaN conversion. The alias case makes a reload after the final
store observably invalid and proves the required interleave.

The same focused fixture checks null-capture preservation and late c37/c74
host-error behavior. The native full builder, all FP control modes, original
object lifecycle and gameplay remain unvalidated. Integration into the main
build and its normal `scripts/build.ps1` checks belong to the primary agent.

# Native shader interpolator fields and mappings

Addresses: `00b36800`, `00b34aa0`, `00b346e0`.

These entries operate on the existing actual `NativeMaterialProgramBuilderStorage`
(`B0h`), `NativeShaderFieldStorage` (`1Ch`) and `NativeShaderDescriptorArray`
(`0Ch`). The earlier `shader_source.cpp` semantic routines remain separate.
The new implementation uses the supplied actual string service and shared CRT
allocation service; it introduces no field translation or second owner registry.
Names are descriptive hypotheses, not recovered symbols.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| `00b36800` select fields | ECX builder; stacked TEXCOORD usage, COLOR usage, output header; `RET 0Ch` at `00b36e29` | Complete normal readable-domain body; host failures retained |
| `00b34aa0` append mappings | ECX builder; stacked input field header; `RET 4` at `00b34be0` | Complete normal readable-domain body; host failures retained |
| `00b346e0` reserve mapping words | ECX two-byte array header; stacked signed request; `RET 4` at `00b3473b` | Complete normal readable-domain body; host failures retained |

Selection always appends an independently allocated `ScreenSpacePos` float4
POSITION0 field with mask0. It then visits the current `descriptor_70` and current
`mode_descriptor_74` arrays at `DCh`/`E0h`, in that order. It does not traverse
`effect_78`. With both usage pointers null, every source field is copied and its
mask is regenerated from its component count, including a zero-width field.
The source mask is ignored. A nonnull usage pointer enables only its semantic:
TEXCOORD2 or COLOR1. Each DWORD carries four flags in bits0..3. Width-sized
segments share separate running TEXCOORD and COLOR offsets across both
descriptors. Components are tested in descending order; selected width is the
number of set flags, while the mask retains their original component positions.
An enabled semantic advances its offset even when no component was selected.
Fields with other semantics are omitted in the filtered path.

The output is appended without clearing or deduplicating. Pointer capacity grows
only at count==capacity, to max(capacity+5,10). After each allocation the code
reloads the current descriptor and separately captures the index, semantic,
scalar and name-source pointers before the pooled name copy. Scalar, width,
mask, semantic and index are written only after that copy returns. The initial
temporary name remains address-stable through publication and pool return.
The implementation reuses `construct_native_shader_field_00b34e20` for the
equivalent inline constructor bodies and the existing native pointer reserve.

Mapping starts at field1. It chooses the destination from the field semantic,
then reloads the current field and mask for each of bits0..3. The low byte is
the field index modulo256; the high byte is the component index. These words
append to the builder's actual arrays at54h and60h, growing by doubling with
minimum1. Component count and semantic index do not affect this pass. Every
FOG8 replaces the WORD at6Ch with field-low-byte/zero, so the last FOG wins.
Neither mapping array is cleared by this entry.

`00b346e0` reserves two-byte elements, rather than appending a byte. It clamps a
signed request to1, returns if capacity suffices, allocates request*2, copies
rows while rereading current signed count/data, frees current old data and only
then publishes replacement data/capacity. The primary repaired the saved
Ghidra post-free gap `00b34730..00b34738` under the write lock, saved and
refreshed the export. These nine bytes contain stack cleanup, data publication,
capacity publication and POP EBX. No gap remains in the owned three bodies.
Worker Ghidra activity was read-only through verified BSP wrappers.

The caller `00b3b3c0` has three selection sites: `00b3b522` null/null into1Ch,
`00b3b910` stack usage arrays into28h, and `00b3bab2` null/null into1Ch.
Mapping is called at `00b3b52a` and `00b3ba26`. Caller-side clears are outside
these helpers. The early ordering is vertex fields (`00b3b515`), selection,
mapping, then pixel fields (`00b3b531`). This module does not replace the
compiler continuation or claim source-generation/pass completion.

| Native service | Evidence-backed contract |
| --- | --- |
| `00bf681b` / `00bf55be` | cdecl size, raw EAX; malloc/new-handler retry, throw if exhausted; caller `ADD ESP,4` |
| `0041e870` | ECX actual8h destination, stacked C string, constructor, `RET4` |
| `00b34e20` | ECX actual1Ch record, six stack operands, `RET18h`; name before scalar stores |
| `0041dd40` | ECX actual8h string, length/preserve, `RET8`; current-header resize |
| `00bf7680` | cdecl destination/source/size; caller `ADD ESP,0Ch` |
| `00b34680` | ECX current0Ch pointer header, capacity, `RET4`; minimum10 |
| `00419cc0` / `00bd1510` | pool getter has no stack arguments; following return consumes block/size/unused with `RET0Ch` |
| `00bf6989` | cdecl current data pointer, returning free; caller `ADD ESP,4` |

`NativeShaderInterpolatorOperation` retains the actual builder/output/string
identities, temporary8h header, raw1Ch acquisition, source pointer, captured
operands, current row/component/offsets and any pending two-byte allocation.
A failed frame cannot replay; its destructor rejects pending/failed state.
The caller must keep all borrowed owners and buffers alive and prohibit their
teardown until explicit diagnostic cleanup. Published fields belong to the
same output owner and are cleaned by existing builder/descriptor teardown.
This is a retained host failure boundary, not the original FH3 cleanup path.
Valid readable nonnegative array extents and adequate usage buffers are
preconditions. Invalid pointers, overflow allocations and asynchronous races
are not claimed as safe input domains.

Validation and immutable local artifact hashes are recorded in
`reports/native_shader_interpolators.json`. The focused original/source fixture
uses original executable bytes verified against saved Ghidra bytes and real
native string pool ownership. Its descriptors are explicit inputs produced
through the existing actual descriptor/field constructors; it does not claim
installed descriptor loading or game execution. The default build registry was
initially leased; after it became free, both the new source and the accepted
system-field dependency were appended under the existing lease transaction.
The stable default registered Win32 build passes with `/W4 /WX` and the existing
CTest passes 1/1. No additive include was needed for the final validation. The
focused fixture passes against that combined library.

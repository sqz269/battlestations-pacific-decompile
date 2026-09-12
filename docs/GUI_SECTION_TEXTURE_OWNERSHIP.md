# Canonical Section properties and texture ownership

Addresses: 00AC0280, 00ABF6F0, 00ABF4F0, 00AA2660, 00435C40, 0041DD40,
004C12B0, 00CB8460, 00CB8468, 00DEF324, 00DEF334.

`GuiSectionRuntimeImplementation` now reads Section properties and owns the
current native texture/string on the SAME canonical widget tail used by the
existing mapped geometry emitter. There is no second widget, atlas rectangle,
texture reference collection, logical texture wrapper, mesh or material graph.
This companion supersedes the property/destruction gates described in
`docs/GUI_SECTION_RUNTIME.md`; that document remains the geometry evidence.

| Routine | Original ABI / inclusive body | Coverage |
| --- | --- | --- |
| AC0280 properties | ECX Section, reader stack, RET4; AC0280..AC058B | complete nine derived descriptors and texture-setter composition over an evaluated GuiTable, after the loader's existing single AAA710 base/children pass; live Lua visitor/SEH ABI excluded |
| ABF6F0 texture setter | ECX Section, native8h string pointer stack, RET4; ABF6F0..ABF76C | complete successful actual-resource sequence; unwritten native size scratch represented by two required caller-supplied DWORDs |
| ABF4F0 destructor | ECX Section, no stack, RET; ABF4F0..ABF58D | partial projection: derived release/string portion ABF514..ABF56D; primary owns native profile publication and AA9730 base retirement; no pool deletion |
| AA2660 resolver dependency | stack string, float4, float2, scale, RET10h; AA2660..AA27E0 | existing resolver reused with canonical array and actual renderer/texture ABI bindings; inherited atlas/CRT semantic boundaries |

## Descriptor writes and canonical storage

The loader has already performed AC02A2's AAA710 base/children pass before
`properties_bound`. Repeating that pass would duplicate child creation.
AC0280 calls the reader's current0C, established BD68D0, nine times in this order:

| Site | Key | Type / canonical destination | Missing-or-nil default |
| --- | --- | --- | --- |
| AC02FD | Value | float F4 | +0 |
| AC0347 | StartAngle | float F8 | +0 |
| AC0386 | Texture | native8h temporary | empty string |
| AC03CC | ClockWise | byte 10C | 1 |
| AC0416 | TextureAngle | float FC | +0 |
| AC0460 | U0 | float 100 | +0 |
| AC04AF | U1 | float 104 | current D7A24C |
| AC04FE | RimWidthPercent | float 108 | current CE3800 |
| AC0543 | TextureMode | signed DWORD 110 | 0 |

The existing evaluated BD63B0 conversions supply Lua numeric/string/boolean
behavior. A Texture number uses the existing Lua number-to-string projection;
embedded NUL truncates, and other non-string values become empty. The resulting
bytes populate a real temporary NativeString through the existing storage with
preserve=false, before ClockWise and the remaining descriptors. Missing defaults
overwrite prior values. AC054C invokes ABF6F0 and the temporary is destroyed on
normal completion or exception. Consequently, equal Texture spelling can skip
emission even after the other properties changed; no extra rebuild was added.

The ABF420 producer already establishes the Section fields. Its four float UV
members now form `std::array<float,4> atlas_118` at the same native118..124 offsets,
verified by static assertions. The existing numeric emitter uses the unchanged
40h tail offsets. This lets AA2660 write the canonical rectangle before texture
getter callbacks, without copying a result back from a temporary rectangle.

## Setter ordering and actual references

ABF705 uses existing 435C40 equality: recorded lengths first, then host CRT
case-insensitive string comparison. Equal strings return without resizing,
singleton access, resolution or current7C. ABF70E..ABF733 use the same native
string self-copy guard, preserve=true resize and post-callback length/data reads
as the existing BE0A30 actual-header copy fragment. The memcpy call's ADD ESP,0Ch
at ABF733 confirms its three arguments.

ABF736 calls the existing `GuiStartupHost::gui_manager` service. Its 4C12B0 body
really performs lazy singleton construction/registration; it is not replaced by
a cached arbitrary token. AA2660 ignores incoming ECX and uses its atlas/global
renderer, so the getter's result is discarded while its effects are preserved.
The caller supplies the existing live atlas lookup callback and its records,
whose texture identities must be actual native resources in the same retained
owner domain as Section's Model/material. The callback is borrowed and invoked
in place; its mutable state is not copied into a new owner.

AA2660 uses these bindings:

| Native site | Binding | Verified contract |
| --- | --- | --- |
| AA2679 -> AEFB20 | supplied existing atlas lookup | borrowed existing item, name normalization/search remains the existing atlas implementation |
| AA2723 / AA2743 | CURRENT actual texture table3C/40 | width then height, each reloading item.texture; native-ABI functions must be callable |
| AA27BD | atomic increment of actual texture+04 | exactly one new atlas texture reference, after getters and UV/size writes |
| AA27DA | CURRENT renderer table64 | SAME canonical NativeString header and flags0; returns its one owned texture reference, including null; no extra retain |

The atlas match retains and returns the current item texture, while an atlas
miss leaves UV and size untouched and delegates ownership to renderer64. Null
allocation/result behavior is inherited from the native factory contract;
Section publishes a null result and still enters its actual current7C.

ABF6F0 reserves eight stack bytes but never initializes them. ABF741 passes
those two DWORDs as AA2660's size. AA26F8..AA2715 only enter the dimension branch
when BOTH words compare as zero (either signed zero); nonzero or NaN skips it.
The required `texture_size_scratch` service makes those bits explicit, copied
as bits without a fabricated zero/default. The computed size is discarded, but
dimension calls and their possible effects cannot be assumed pure or omitted.
This is a caller-state input to a new C++ interface, not recovered stack values.

ABF755 OVERWRITES texture114 without releasing the previous reference. There
is no decrement, release call or helper hidden between ABF750 and ABF762. Only
the newly returned pointer is later released by Section destruction. Therefore
repeated different names can abandon earlier retained references, as in the
native implementation. No release-old fix or substitute owner collection was
invented. Material texture replacement in current7C separately follows the
existing B189F0 publish/retain/release sequence on the actual material slots.

## Retirement and exceptional boundary

ABF4F0 installs D5C8E8, captures current texture114, decrements its actual+04,
invokes CURRENT virtual0 only on zero, then clears114 AFTER the callback. It
next reads the CURRENT string buffer and length and returns length+1 through
the existing string storage. It does not clear the native string header.
`before_scene_release` supplies these derived steps exactly once, using
`buffers.geometry.actual_owners()` for the captured texture. A retired flag
prevents reuse of the intentionally stale string header; successful host
destruction never frees it twice. The primary supplies base AA9730 once.

The SEH map at DEF334 has two states; DEF324 entries are {-1,CB8460} and
{0,CB8468}. CB8460 tail-jumps AA9730; CB8468 adds EC and tail-jumps 41DD20.
Thus a texture-release exception at state1 destroys the string then the base.
The companion preserves the derived string cleanup on a throwing owner lookup,
but retains its existing terminal failed-operation policy. It does NOT claim
native base unwind, native SEH continuation or rollback of published resources.
Property temporary string cleanup is independent of the derived string.

All public property/texture/value/emission entries reject active, failed or
retired operations before stores. Internal setter-to-emitter composition is
permitted; callbacks cannot reenter through the public emitter/setters. The
existing active lifetime guard prevents retirement during callbacks. Exceptions
after name, UV or texture publication preserve those preceding effects and
mark failure terminal; failed operations and retirement are not retried.

ABF5B0 copying and ABEC40/native Section pool allocation/deletion remain
separate contracts. No drop-in native Section object or vtable ABI is claimed.

## Verification

Read-only Ghidra batches used the repository wrapper, verifying
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Primary restored the
previously missing exact ABF4F0 function. This worker performed no Ghidra writes.
ABF4C0..ABF4E7 is a separate still-missing membership leaf, outside this packet.

The complete updated TU compiled MSVC Win32 `/W4 /WX /fp:strict /permissive-`.
One ignored local fixture compiled the canonical Section fields with the actual
existing resolver and passed callback-visible UV publication, item texture
reload after a width callback, actual+04 retention, zero/nonzero scratch branches
and atlas-miss preservation. It is a resolver/storage fixture, not a full
Section-owner, renderer factory, native-byte differential, gameplay or visual
test. `tools/verify_report_calls.py` checks the report's live direct call rows;
the primary runs the combined repository build and existing tests.

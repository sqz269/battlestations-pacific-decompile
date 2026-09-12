# Canonical Section geometry companion

Addresses: 00ABE6E0, 00ABE7B0, 00ABE960, 00ABF420, 00ABEDA0, 00ABF770,
00D5C8E8, 00AA19B0, 00AC0280, 00ABF6F0, 00D61D6C, 00D61DE0.

`GuiSectionRuntimeImplementation` owns the one Section tail on the existing
`GuiWidgetOwner`. Its `current7C` emits all three native texture modes into that
owner's actual Model/mesh, logical vertex/index mappings, draw section and
material. It creates no replacement widget, renderer geometry, or owner map.
The existing timed-entry numeric helper now has a concrete owner-side consumer:
`update_gui_section_timed_owner_00abe7b0`. The integrator binds it after the
existing current5C gate in `gui_timed_entry_types.cpp`.

## Producer and coverage

| Native routine | Original ABI / inclusive body | Coverage |
| --- | --- | --- |
| ABF420 constructor | ECX destination, EAX destination, RET; ABF420..ABF4B7 | partial projection: complete derived stores; existing widget owner supplies AA9390 base construction; Section pool allocation/native table ABI are external |
| ABE6E0 setter | ECX Section; four stack floats; RET10h at ABE76F, body through ABE771 | complete supported current7C composition; native ordered/unordered x87 comparisons and MOVSS stores |
| ABE7B0 timed update | ECX 14h entry; delta stack; AL changed; RET4; ABE7B0..ABE886 | owner branch ABE7C9..ABE877; existing helper supplies numeric fragment and parent supplies initial current5C gate |
| ABE960 angle fold | float stack; x87 ST0 result; RET4; ABE960..ABE9B8 | partial projection: host CRT fmod substitutes for original CRT BF857A dispatch; subsequent float stores/x87 comparisons preserved |
| ABEDA0 current74 | ECX Section; RET; ABEDA0..ABEE57 | complete successful native owner composition; allocate/associate actual mesh only when +74 clear, then set same widget position to zero |
| ABF770 current7C | ECX Section; RET; ABF770..AC0272 | all three mode writers, skip-write branches, stream creation, material/section/layout publication; successful valid owner/storage domain, original CRT/SEH failure behavior excluded |

AA19B0 selects ABF420 for default construction and ABF5B0 for copying. ABF420
passes 11h to AA9390 and installs D5C8E8. Live table bytes establish current40
AA87B0, current4C AA6980, current50 AA6870, current54 AA68F0, current58 ABE670,
current5C A9E110, current70 AAA3E0, current74 ABEDA0, current78 ABE630 (tail jump
to AA7170), and current7C ABF770. ABEDA0 and ABF770 initially lacked Ghidra
functions; the parent defined their verified exact bodies and saved under its
write lock. This worker made no Ghidra mutations or renames.

AC0280's descriptor writer establishes the field names below; ABF420 establishes
their defaults. Descriptive source names are reconstruction hypotheses.

| Offset | Sole canonical field | Constructor value |
| --- | --- | --- |
| EC/F0 | native Texture string header | 0/0 |
| F4 | Value | +0 |
| F8 | StartAngle | +0 |
| FC | TextureAngle | +0 |
| 100 / 104 | U0 / U1 | +0 / live D7A24C |
| 108 | RimWidthPercent | live CE3800 (observed 0.5) |
| 10C | ClockWise byte | 1; 10D..10F remain untouched |
| 110 / 114 | TextureMode / retained texture | 0 / null |
| 118..124 | atlas UV rectangle | 0,0,one,one |
| 128 | signed segment count | unwritten until current7C |

The 40h `GuiSectionFields` tail begins at native EC. The canonical base layout
retains its original size, color, transform and bounds fields. No physical 12Ch
Section pool object or original binary replacement ABI is claimed.

## Geometry and floating-point evidence

ABE6E0 compares StartAngle, Value, U0, U1 in that order using FUCOMIP, LAHF,
TEST AH,44h. Unequal **or unordered** takes the setter; equal signed zeros skip.
It publishes all four values before current7C. ABE7B0 captures F8/100, uses its
existing Section-specific approach kernel, copies setter arguments through
FLD/FSTP, and passes literal FLD1. It returns changed even if a zero step leaves
the value equal and the setter subsequently performs no work.

ABF770 clamps Value using COMISS (unordered remains NaN) and folds StartAngle.
It obtains the current actual Model geometry, and, when vertex stream count is
zero, requests `SimpleColor.mvfm`, 146 vertices and 432 INDEX16 indices. Creation
uses the current renderer's actual native-ABI factory slots and existing native
mesh publication/reference helpers. Original numeric addresses are evidence
tokens and are never called as host function pointers.

The segment count uses BF7420 on `Value * [CF0058]`, retains the x87 fractional
remainder, and increments when the remainder exceeds CE3C70 and the truncated
count is below 72. This is not a generic `ceil` substitution. The source keeps
every original FSIN/FCOS, x87 arithmetic instruction, single-precision spill,
COMI branch, packed/float-color path, and low16 index store in the numeric block.

| Mode | Vertex lock count | Index lock count | Active vertices / primitives |
| --- | --- | --- | --- |
| 0, radial fan with fixed edge UV | 74 | 216 | segments+2 / segments |
| 1, inner/outer rim strip | 146 | 432 | segments*2+2 / segments*2 |
| 2, radial fan with angle-transformed UV | 74 | 216 | segments+2 / segments |

ClockWise controls angle sign and index winding. Mode1 uses U0/U1, atlas U and
RimWidthPercent; mode2 additionally reads TextureAngle. +74 set or other mode
values skip vertex/index writes but still publish a zero-count draw section,
matching AC007F rather than silently inventing another geometry mode.

`section_numeric` is a six-stage, address-commented transcription of 491
instructions. EDI offsets are remapped by EC only; base size/+74 and mutable
constants are pointer aliases. Its 20-word scratch is the native ESP+10..5F
temporary area, copied between stages so pre-lock UV captures survive mapping
callbacks. It contains no widget snapshot. The stages are:

| Stage | Native range(s) | Boundary |
| --- | --- | --- |
| 0 | ABF8D6..ABF941 | segment count and angle extent; existing noexcept BF7420 conversion |
| 1 | ABF965..ABF99D | fan UV center before vertex lock; outgoing argument pushes omitted and corresponding scratch offsets adjusted |
| 2 | ABF9A0..ABFA25 plus ABFDCC..AC0007 | mapped fan vertices, then vertex unlock in ordinary C++ |
| 3 | ABFA38..ABFD24 | mapped rim vertices, then vertex unlock in ordinary C++ |
| 4 | AC0023..AC0064 | fan indices, then index unlock |
| 5 | ABFD40..ABFDAC | rim indices, then index unlock |

Mapping invokes the existing B49980/B49A80 and B49B60/B49C70 implementations
outside assembly frames in original order. Raw logical profiles D61D6C and
D61DE0 are required at every map/unmap. Live table DWORDs at D61D7C/80 and
D61DEC/F0 prove those entries. Unsupported profiles throw; no guessed dispatch
or lock callback substitutes for the actual mapping owner.

At ABF797 EDI comes from incoming ECX and is callee-saved; ESI captures stream0
at ABF8E7, EBX captures mesh+60 at ABF8DC (also saved in the native scratch at
ABF8EB), and EBX is restored after rim writer temporaries. The staged wrapper
initializes these registers from the same captured pointers. It preserves the
caller's EBX/EBP/ESI/EDI; stage exits have balanced x87 stacks. Added alias loads
use PUSH/POP EAX and preserve flags. Windows x86 ABI requires a clear direction
flag for the scratch REP MOVSD copies. Masked x87 exceptions and valid mapped
extents/layout offsets remain preconditions, as for the original writer.

After writes, current7C acquires existing section0/material or creates the
actual section and `GuiDefault.mshd` material. It borrows the SAME owner's
overbright94 as `cOverbrightFactor`, binds current texture114, sets section
material, drops its temporary material reference, stores primitive4/ranges,
rebuilds the actual layout and conditionally appends the new section. It drops
the section creator, recomposes the same widget, then invokes base current50 on
its current color. Section does not call AA9F10/B18A40 here; no clip registration
or separate widget retention was invented.

## Integration and remaining boundaries

The parent's type factory can return `GuiSectionRuntimeImplementation` for
type17 using its existing `GuiWidgetOwner`. `GuiSectionRuntimeServices` borrows
the existing native buffer/mesh/material/string/render domains, mapping context,
layout services and live constants. Generic base current50/54/4C/5C/70 profile
gates must include Section according to D5C8E8. The parent owns those shared
edits, CMake registration, timed-entry dispatch and the ABEDA0/ABF770 ledgers.

AC0280 property/texture loading, ABF6F0 texture acquisition, ABF5B0 copy and
Section pool/deleting lifetime are not reconstructed by this companion. Calling
`properties_bound` throws explicitly. `before_scene_release` rejects a nonempty
authored texture/string tail instead of silently leaking it. Default empty-tail
construction, emission and retirement use the existing base owner. Direct tail
access is the canonical storage API; a future property reader must write it and
provide actual retained texture ownership, not stage a second Section object.

Factories, parameter sources and current owner domains must stay live through
callbacks and retained resources. Null allocations, invalid mapped layouts,
unmasked floating exceptions and original SEH cleanup are not modeled. A failed
operation can retain preceding native publications/creator references; it is
not safe to retry indiscriminately. Host CRT fmod is a library substitution,
not verified binary equivalence to the shipped CRT's exceptional dispatch.
In particular, a callback/profile error after a successful map can leave that
mapping outstanding; no implicit unlock guard or rollback is supplied. Callbacks
must also keep this Section companion, its canonical widget/model/mesh and the
captured streams alive until emission returns. The canonical owner now rejects
retirement during Section operations. Emission reentry, including an equal-value
setter during emission, is rejected before stores. A failed Section operation is
terminal: subsequent operations and retirement are rejected. These host lifetime
guards supply neither rollback nor a continuation for outstanding mappings.

Verification: the full translation unit compiled MSVC x86 `/W4 /WX /fp:strict`.
A focused local executable compiled the exact staged kernel plus the existing
BF7420 implementation and passed all three modes in both directions at fill
0.25: segment count18, center/first-rim positions, index winding, active white
colors, and unchanged x87 stack TOP/tag. It used mapped-storage fixtures, not
fake rendering callbacks. No full Section owner/service runtime, native-byte
differential, game execution, visual output or drop-in ABI validation is claimed.
The parent passed the combined repository `scripts/build.ps1` after integrating
its owned dispatch/profile changes. `reports/gui_section_runtime.json` carries
the exact call rows and coverage boundaries.

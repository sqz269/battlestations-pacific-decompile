# GUI material clip, source-owner and color bindings

This packet supplies the two callbacks required by `GuiGeometryRuntimeServices`
over the same `GuiWidgetOwner`, layout, native node and `MaterialCloneState`.
`bind_gui_material_callbacks` performs clip registration followed by the
material source-owner assignment, and publishes current widget color through
the verified base color implementation. It does not complete production hookup:
the current unique GUI page ownership cannot furnish the required retained
widget lifetime, and a canonical native material/projection owner is still needed.

## Evidence and ABI

All live queries used `python tools/bsp.py ghidra` and verified
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 LE32, base00400000.
Ghidra remained read-only. Full exports and listings have no gaps; all five
complete saved spans matched the installed PE. Exact hashes, prior names and
comments are retained in [the report](../reports/gui_material_binding.json).
New descriptive names are hypotheses; prior annotation text is preserved.

| Entry | Original ABI | Last instruction / bytes | Inclusive end |
|---|---|---|---|
|00AA9F10|ECX widget; stack material; RET4 at00AAA0D7|00AAA0E4 JMP /5|00AAA0E8|
|00B18A40|ECX material; stack owner, low byte retain flag; RET8|00B18A98 /3|00B18A9A|
|00AA6870|ECX widget; stack float4 source; RET4|00AA68E7 /3|00AA68E9|
|00B74650|ECX node/model; EAX bool; RET|00B7465B /1|00B7465B|
|00B72B40|ECX mesh; EAX DWORD+58; RET|00B72B43 /1|00B72B43|

AA9F10 pseudocode omits its material stack argument; its listing establishes
the preserved material receiver and all registration source addresses. The
high-address tail is a backward jump to the common registration path, not a
missing free continuation. These C++ functions expose new interfaces and do
not claim the original memory layout or binary ABI.

Live table slots all contain00AA6870:

|Type|Table|Current +50 slot|
|---|---|---|
|Base|00D5C130|00D5C180|
|Group2|00D5CB80|00D5CBD0|
|Icon6|00D5C4C0|00D5C510|
|FrameBox18|00D5D130|00D5D180|
|Screen1|00D5BE38|00D5BE88|

The installer accepts these four concrete owner types and fails for unverified
derived color slots. The base algorithm can also be called directly.

## Live clip sources and exact registration order

AA9F10 starts at the supplied widget and follows its existing parent chain
until virtual+5C returns16. The existing transform type field implements that
base getter. The same owner's `extra_fields().clip_enabled_e8` is set to1 or0
and borrowed by `cClip`. AA9390 does not initialize+E8, so the added field has
no constructor initializer; registration writes it before every read.

If active, the existing parameter table then receives `cClipCenter` (two
words from the found ancestor+EC), `cClipBorder` (four words from ancestor+F4),
and `cAspectRatio` (one word from actual mutable00E12FC0). The ClipBox service
returns references to these real fields. It does not create clip rectangles.
If inactive, the three latter registrations are omitted, preserving any old
records. Every registration uses the existing reflected effect metadata and
`MaterialCloneState.parameters`; an absent shader constant is a legitimate
no-match result. No second parameter registry or copied constant source exists.

Read-only follow-up evidence identifies type16 constructor00ACE0A0 and copy
constructor00ACE0F0, both stamping table00D5D058. Its current+24 is00ACE120;
that routine writes center+EC/F0, half extents+F4/F8 and border values+FC/100
from fields+104/108. Its x87 arithmetic and source-owner construction belong to
the next ClipBox packet. The generic binder does not fabricate those fields.

## Parameter ownership and color publication

B18A40 releases the prior owner only if byte+10D and pointer+C are nonzero,
then clears+C, writes the incoming pointer and exact byte, and retains the
incoming nonnull pointer if that byte is nonzero. Equal pointers take the same
release/store/retain path. The incoming object must independently survive the
old release. The reconstruction uses the material's existing `pointer0c`,
`owner0c`, and `byte10d`, requiring a genuine owning token with matching pointer
identity. It does not synthesize an intrusive counter. The callback installer
requires that the widget token also keep the same layout and its borrowed
parameter fields alive. Missing/invalid host ownership fails explicitly;
acquisition failures occur after the preceding stores and are not atomic.

AA6870 first copies all four supplied words into the same layout color. It
also keeps the pre-existing transform alpha projection coherent. It then
reads actual node+180; absent geometry returns normally. A zero actual mesh+58
count also returns normally. Otherwise it reloads node+180, obtains element0
through the existing+54 pointer array accessor, reads that element's actual
material+20, resolves its canonical `MaterialCloneState`, and copies the source
quartet into the existing diffuse record returned by00B179F0. It does not set
lighting flag+10C, touch other elements or recurse into children. The source is
read again for the material writes, following native order.

## Validation and remaining integration

`scripts/build.ps1` passed MSVC Win32 Release with `/W4 /WX /fp:strict` and both
existing CTests (`reconstructed_math`, `native_math_differential`). Eight native
seed spans were verified first. No permanent tests or fixtures were added.
This is build and evidence validation; GUI callback runtime, game rendering,
native ABI compatibility and visual parity were not tested.

Remaining required owners are explicit:

- The true retained widget/page lifetime, including native base destruction
  00AA9730 and scalar deleting caller00A9E130. Existing saved AA9730 has a false
  free no-return boundary; its documented continuation reaches RET00AA99B8.
- ClipBox constructor/property/update ownership, beginning at00ACE0A0,
  00ACE0F0,00ACE120 and property reader00ACE650.
- The native110h material owner and its association to the same semantic
  material, beginning at ordinary constructor00B18900 and clone00B18B60.
- Native geometry section publication, supplied by the separate GUI native
  geometry packet. Merely owning a semantic geometry section does not create
  the raw mesh+54/+58 and section+20 values this color path reads.

Only after these actual services exist should callers install the callbacks
and treat the GUI geometry path as connected. Installation itself is not proof
of retained widget lifetime or an operational draw path.

## Correction from docs/GUI_GEOMETRY_MATERIAL_INTEGRATION.md

An inherited AA6870 slot does not make an arbitrary native node a model.
AA5840's plain-page branch constructs a cGroup via B8F450/B8DB80 and supplies
it to AC6600/AA6720. Its +180 field is the child-array capacity integer, while
AA6870/B74650 interpret model+180 as geometry. The proven Icon/FrameBox geometry
callers use model-backed child widgets. No arbitrary Screen color-call safety
is inferred from the shared vtable entry.

The integrated adapter therefore requires the same canonical live
NativeModelReference before any color write, and the raw accessor accepts a
NativeModelOwner. Roots without that companion are unsupported, including
actual group roots. This host precondition bounds the recovered function; it
does not add a native class check or claim that the original game calls this
slot on a group root. Actual model-backed resource roots need their canonical
association established before this adapter can support them.

## Constructor-address correction from docs/GUI_CLIP_BOX.md

The earlier B8DB80 shorthand in the model/group discussion was incorrect.
The verified plain Screen path calls B8F450 atAA58FF, B8F5E0 atAA5914, then
AC6600 atAA5944. B8F5E0 is the actual named cGroup constructor. The storage
class distinction and canonical model guard are unchanged by this correction.

## Retention correction from docs/GUI_WIDGET_RETENTION.md

The earlier reference to A9E130 as the base scalar deleting caller was wrong.
The base wrapper is AA9A90, which calls AA9730 before returning storage through
AA75F0/F8BC94. A9E130 is the D5BBF8 derived destructor; A9E210 is its wrapper.
Manager and parent direct deleting+04 paths do not wait on widget+04. The host's
independent page/widget count projections and unique layout ownership require
a coordinated native identity/deletion/lifetime change. An isolated owning token
cannot establish the required borrowed parameter lifetime.

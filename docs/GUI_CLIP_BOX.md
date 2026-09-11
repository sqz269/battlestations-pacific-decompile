# ClipBox fields, reader and current layout update

Type16 now uses the existing GUI type factory and child-model allocation path.
Its companion owns only the native+EC..108 fields absent from the base owner;
the layout, transform, parent chain, node, reference bookkeeping and clip flag
remain in the same `GuiWidgetOwner`. `gui_clip_box_parameter_sources` resolves
that owner's actual type implementation and returns stable references for the
existing material binder's `clip_box_sources` callback.

## Native evidence and boundaries

Read-only Ghidra batches verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 LE32, base00400000. Four complete owned spans
and the two native differential dependencies match the installed executable.
The owned listings have no gaps or missing free continuations. Prior names
were `FUN_<address>`, with no comments; proposed names and all original values
are recorded in [gui_clip_box.json](../reports/gui_clip_box.json). Ghidra was
not changed. Descriptive names are hypotheses and the C++ interfaces are new.

|Entry|Original ABI|Last instruction / length|Inclusive end|
|---|---|---|---|
|00ACE0A0|ECX widget; EAX widget; RET|00ACE0B3 /1|00ACE0B3|
|00ACE0F0|ECX destination; stack source; EAX destination; RET4|00ACE106 /3|00ACE108|
|00ACE120|ECX widget; RET|00ACE226 /1|00ACE226|
|00ACE650|ECX widget; stack visitor; RET4|00ACE6C4 /3|00ACE6C6|

Table00D5D058 proves current+18=00ACE650 and current+24=00ACE120. Its retained
base slots are +34=00AA8530, +38=00A9E0D0, +3C=00A9E100, +50=00AA6870,
+5C=00A9E110, +60=00AA6A30, +74=00A9AC00 and +78=00AA7170. Thus the new
type can reuse the existing Group implementation for those supported base
operations. No unrelated type slots or reflection methods are reconstructed.

## Construction, copying and property order

00ACE0A0 calls base00AA9390 with type16, then stamps00D5D058. It performs no
derived field stores. The existing owner constructs that same base before
creating the new type companion. Its derived field member deliberately has no
value initializer; a zero-filled clipping rectangle would invent native state.

00ACE0F0 calls00AA9520 and stamps the same table, also without any derived
field stores. The base copy creates a fresh child-list sentinel and clones
the source node through its current+10, among other base work. It leaves base
+E8 and all derived+EC..108 bytes unwritten. `copy_gui_clip_box_00ace0f0`
therefore requires the actual base-copy operation and adds no fieldwise copy.
That base copy and its native node-clone lifetime are unresolved in the current
GUI owner runtime; ordinary type16 loading does not imply that copying works.
The companion's implicit C++ copy is disabled.

00ACE650 runs00AAA710 first. Existing loader integration has already completed
that base reader and all child loading when `properties_bound` runs. The
derived continuation reads `BorderWidth` as tag6/Vec2 into+104/+108. Missing or
nil input uses the two identical float defaults at00D7A2F0, bits3DCCCCCD
(0.1f). Actual table values use the existing00BD63B0 converter, including its
numeric-string and missing-lane behavior. Wrong aggregate shapes fail before
the update instead of consuming uninitialized destination values; this is an
explicit host restriction, not a native default conversion.

The reader immediately calls current+24 at00ACE6BD. Descendant material
registration can occur during the earlier base reader while parent ClipBox
fields are still unwritten. Returning their addresses is valid; packing or
drawing must wait until the parent reader/update has established the values.

## Exact x87 update

00ACE120 calls the existing00AA6750 resolved-position kernel, computes and
spills the width*pivot products, subtracts those from resolved X/Y, and loads
size again through00AA6740. It produces center+EC/F0 and half extents+F4/F8.
Two interleaved SSE moves copy the authored BorderWidth bits from+104/+108
into+FC/+100, the latter two lanes of the borrowed `cClipBorder` quartet.
Scale, rotation and aspect globals are not read by this routine.

The implementation preserves the native x87 stack, operand order and every
float32 spill. Ghidra prints both `DC C9` and `D8 C9` as abbreviated `FMUL
ST1`. The bytes establish different destinations:00ACE1BA and00ACE1F0 use
`FMUL ST1,ST0`, retaining the half on ST0;00ACE1C6 uses `FMUL ST0,ST1`.
Treating these alike doubled extents in the initial implementation, which the
native fixture caught. The committed implementation uses the verified operand
forms. Border bits move without floating-point conversion.

The existing `GuiWidgetTransformHost::on_layout_finished` is the separate
00AA8710 current+24 integration point after layout notification, recompose and
bounds refresh. A host resolving the same owner can call the companion's
`update24_00ace120`. This packet supplies that operation and the reader's
immediate update; it does not invent a global per-frame update dispatcher.

## GUI node class and color-call evidence

GUI type names do not establish the native node class. In00AAA710,
00AAAD89 calls the GUI type factory,00AAAD8E requests184h,00AAAD95 calls the
model allocator00B74EB0,00AAADB1 calls model constructor00B75030, and00AAADC4
stores that result at widget+4C. This unconditional child path covers GUI
Group2 and ClipBox16 as well as Icon/FrameBox. The standalone00AA6640 path
likewise requests184h at00AA6662, calls00B75030 at00AA6681 and binds through
00AA6720 at00AA6695. Existing pool ownership supplies the actual188h model slot.

Screen creation differs:00AA5840's missing-.mmod branch calls the group
allocator00B8F450 at00AA58FF and constructor00B8F5E0 at00AA5914, then calls
00AC6600 at00AA5944 with that node. Its native+180 is the group's
signed array capacity, whereas the model's+180 is geometry. Inherited color
slot+50 alone therefore does not prove a Screen root can safely use the model
color accessor. The directly indexed caller of00AA6870 is Text00AB6B50; the
verified Icon rebuild loads+50 at00AB448C and calls it at00AB4495 with its own
Color. Base property reading merely supplies the destination+50 pointer at
00AAA950. No claim is made that arbitrary later Screen color calls are absent.
The parent integrator owns the canonical model guard for material publication;
this packet does not alter those material files.

## Validation and remaining work

MSVC Win32 Release passed `/W4 /WX /fp:strict`, and both existing CTests passed
after eight native seed byte checks. One ignored native differential executable
was linked with `/MANIFEST:EMBED`. It runs copied, disk-verified00ACE120,
00AA6750 and00AA6740 with only their two absolute constants and two external
calls relocated. Eight focused cases cover ordinary and parented positions,
cancellation, signed zeros, quiet/signaling NaN payloads, infinities and
subnormals. All eight ClipBox field words matched byte for byte, and the
missing-property BorderWidth default was checked. No permanent tests were added.

This establishes bounded arithmetic and source/default behavior, plus build
integration. It does not establish all floating-point environments, native
constructor/copy ABI, actual GUI page loading, material packing, rendering or
game validation. Actual retained widget/material ownership remains unresolved:
ClipBox source fields do not close that lifetime boundary. Full base copy
00AA9520 and the host's current+24 layout callback remain required integrations.

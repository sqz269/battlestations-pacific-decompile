# Main-menu Listbox current08 selection listener

Addresses: 005966F0, 004260B0, 004263B0, 005C35D0.

`main_menu_listener_current08_005966f0` reconstructs the complete normal caller
sequence over the existing canonical menu, widget, Listbox, Text, Icon,
mission-tree and command-bar owners. **The complete provider composition is
partial**: the explicit required operations below still need their actual
resource/profile/objective providers. No replacement menu list, widget tree,
profile, selection index or callback-success default is created.

Evidence was read through the verified wrappers from `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`. This worker did not mutate Ghidra. Proposed
new descriptive names are hypotheses; correct existing NativeString/STL/CRT
names are retained. Parent annotation and full build are separate work.

| Routine | Original ABI and body | Coverage |
| --- | --- | --- |
| 5966F0 | ECX screen+8, selected row and callback Listbox stack, RET8; 5966F0..59786C | Complete normal caller sequence; partial concrete provider composition |
| 4260B0 | ECX native8h output, signed integer stack, EAX output, RET4; 4260B0..426194 | Complete normal NativeString helper, new C++ interface |
| 4263B0 | ECX native8h prefix, output/integer stack, EAX output, RET8; 4263B0..426490 | Complete normal NativeString helper, new C++ interface |
| 5C35D0 | ECX mission-tree owner, native id-string pointer stack, RET4; 5C35D0..5C3600 | Complete normal composition of existing last-match mission lookup |

## Dispatch, live storage and pages

CEFC48+08 contains `F0 66 59 00`, making this the actual main-menu Listbox
selection target. It is distinct from CEFC04+04/5993A0 on screen+40 and from
CEFC48+04/598B60 on screen+8. Both incoming5966F0 stack operands are **unread**.
596713 reads `[ESI+1B0]` with ESI=screen+8, then59671A calls A9C990. Therefore
the screen's current1B8 supplies selected D8 even if the callback Listbox or
selected row differs. The public interface keeps both RET8 operands.

The callback captures D8 as a signed DWORD, not a row ordinal. Its page switch
reads E08874 once. Pages1/2/3 load the pointer-table entry, publish E194C4/C8/CC,
construct a native string, then update the current E1930C command owner. Page11
uses E0884C without publishing one of those globals. Page9 and unrecognized
pages return. These help-table arms have no native index bounds check; their
valid populated selection domain is required, including the absence of an
implicit -1 clamp. Only mission pages4..8 clamp D8=-1 to0.

All added field carriers are references to the same owner storage. Writable
arrow1C4/1C5 references are checked against the existing widget listener's
references. Actual screen280/298/2A4 lists use `MainMenuCommandWidgetListView`;
native offsets shown relative to screen+8 must be adjusted before interpreting
the count fields. Screen+64/+68/+6C retain address-only field names until the
objective producer is reconstructed. No initialized alternative counters exist.

## Objectives, page12

The three actual lists receive screen260 color first, in order, reloading
current counts after every Text/material callback. Signed, DWORD-wrapping
thresholds choose a list and iterator. The selected row reaches580820, then a
fresh iterator payload receives screen270 color. The same contiguous
`GuiTextColor` quartet is borrowed through the MSVC Win32 float-array carrier;
AB6B50's post-base shadow-alpha read observes changes to that screen color.

The ordinary objective branch uses actual widget vectors2CC/2DC through519DC0
and each current widget+4C node through B6DA70. It resets the previous pair using
current CE3E18, sets the selected pair to FLD1, and publishes the same screen60.
Each constant is read after its vector lookup. The back row uses `globals.back`.
Primary/secondary help comes from the current E08868/E0886C pointers. Hidden
help reads a fresh selected mission, then the low byte of block0.enabled via
5C27E0, and that same side's hiddenHints+44 vector. Missing hints use the exact
installed fallback literal. This matches the existing MissionTree Lua schema.

Hidden help can dim the earlier pair with CE3800. Every page12 terminal path
calls AA6BC0 on the screen's freshly resolved1B8 with null,0. This clears base
widgetDC and raw79; it does **not** clear the derived Listbox114 listener.

## Mission pages4..8

580650..580662 captures E198AC+5C and its group vector before E194D8. 57D4A0 indexes that same
group's434h mission vector with selected D8. Mission record +8 is the name,
+18 the helpline, and +60/+64/+68 the date triple, as established by5C6A70.
The picture reader at5C75CC..5C75F1 consumes the authored picture name into a
retained texture+A0 and UV+A4. The required texture association must return
that actual retained resource; `MissionRecordExtra::picture` is not a texture.
The caller gives AB2690 state0, that texture, and `(0,0,currentOne,currentOne)`.
AB27A0 supplies state0's native size before actual Icon current58/AB1EF0.

Name/date/content submissions use the same actual Text lifetimes. AC0820
receives source name304 in ECX, destination background334 in EDX and +0 on the
stack. Both register operands are preserved. The background becomes visible;
43BC30 constructs the native date result using EDX=date60 and stack date64/68.
The date temporary is destroyed before content submission. Screen110 is first
cleared, then set from a fresh page read:4/6 use314,5/7 use310,8 uses320.

The top/down guards use D8, actual current Listbox104 count and next row77.
Disabled arrows run actual Icon current88(0,0,1) before their fresh slot alpha
write. These are the same enable fields consumed by the command listener.
Map names retain the native construction order: `_Icon`, prefix,
prefix-plus-decimal, combined name. The lookup uses the same group110's direct
children and actual native node names. Its second DWORD is unused;0/1 does not
mean optional/required lookup or recursive traversal.

The selected map-point330 publication occurs before temporary destruction.
During the full point/flag pass the caller captures the original group iterator
but reloads E198AC+5C before E194D8 at every end comparison, preserving the
same load order as580650. The current group must still own the iterator.
Each point's current88 uses sideMission+360 (selected1/3, other0/2); flags receive
current D7A24C or CE7804 captured before name allocations. Map-name cleanup
precedes point state selection but follows flag alpha, as in the listing.

E194DC is published before594B60's required medal update. A fresh selected
mission then reaches5C35D0 on the current mission-tree owner. Its existing
case-insensitive lookup keeps the last id match; an unknown id preserves both
selection fields. Checkpoint calls deliberately use the **earlier mission
record id+0**, not the fresh selected mission or its title. Each of the three
profile reads is fresh. Background/Text current34 targets are captured before
the profile call and receive the freshly resolved same-screen receiver.

ABBE50's cached C-string equality gate precedes native temporary allocation,
and its Text receiver remains captured across that allocation. The subsequent
FrameBox width preserves the original x87 spills: float(text114/CEC380),
float(frame10C), float(frame110), then one x87 sum narrowed to float. D5D130+58
contains `20 F1 AC 00`, identifying the required current58 as **ACF120**. That
callee updates all existing state sizes before current88, not merely base size.

Changed selection briefly runs actual Icon84/AB1110(2,0,currentD7A2F0), then
publishes E194E0. AA6BC0 clears current1B8 base listener fields;683790 resets
the same mission scroller with null while retaining its attached content.
Actual Text AB6BD0 returns its float32 spill through the parent's existing
runtime method. The caller stores the first height as double, adds the second
and current CEED60 in x87, stores double, then subtracts live clipbox230 height
and narrows/reloads float before6834A0. Scroller+5A supplies the scroll glyph.

The five command triples are `(A2,select,0)`, `(9/0,zoom/empty,1)`,
`(B3,navigate,1)`, `(B2/0,scroll/empty,1)`, `(A3,back,2)`. The current F88A30
gate is read after constructing the scroll string. 54B530's RET3C at54C045
confirms all fifteen DWORD operands. Caller cleanup is select,zoom,navigate,
back,scroll. This uses the actual current command owner and existing bar logic.

## Required providers and source limits

| Boundary | Evidence and status |
| --- | --- |
| AB2690 | Full body read: retain replacement/release old actual texture, UV copies/flips, active-state current80; required resource provider |
| AB27A0 | Full body read: actual state index and AB17B0 texture/UV native size; required resource provider |
| AC0820 | Full listing read: ECX source,EDX destination,float stack,RET4; AC06C0 size and resolved-position chain remain required |
| 43BC30 | Full body/listing read: language-sensitive French/Italian/Spanish/German/default date builder; required actual provider |
| 580820 | Full body/listing read: show actual294, AC0820 on selected row, size height+CEE4E8; required same-screen provider |
| 594B60 | Full body/listing read: selected mission/profile6B4 score map,9052D0,medal2F8 visibility/state; required same-screen provider |
| 519DC0/B6DA70 | Full bodies read; same screen vectors/widgets/nodes required; objective producer association still open |
| ACF120 | Full body read and D5D188 table bytes verified; actual FrameBox resize/current88 provider required |
| 7F8D60/7FC370/5806A0 | Existing `MainMenuCommandServices` required against the same actual profile/mission owners; bodies read |
| AB6BD0 | Parent exposes the already implemented exact helper on the actual Text runtime; no substitute height callback |
| command owner/host/environment | Resolve current E1930C and its existing bar/widgets; retain their documented semantic interface limits |

Existing record and command-bar structs retain their documented C++ string,
container and source-interface limits. The caller requires live owner/profile
identity and synchronous completion of Text work. A pending Text operation
cannot be treated as success; the larger menu continuation would need to retain
and resume its caller frame. Native SEH, invalid iterator/debug CRT paths,
string/vector/object ABI, allocation failures and runtime/game/render parity
are not claimed.

## Validation

Strict MSVC Win32 compilation passed with `/W4 /WX /O2 /MD /fp:strict`, using
the parent's current actual Text runtime declaration. The focused ignored
`menu_selection_strings_probe.exe` passed negative concatenation and INT32_MIN
contents plus the exact nested allocation/release sequence. It linked this
whole source, a fresh compilation of the parent's actual Text factory, and
the existing parent libraries; the report records input SHA256 hashes. This
is string-fixture evidence, not selection-callback runtime evidence.

`verify_report_calls.py` checked all184 numeric rows with zero failures:
166 direct and18 resolved indirect sites. The tool confirms containing bodies
and instructions; indirect targets additionally depend on the table/profile
evidence above. No permanent test or Ghidra write was added. Parent combined
build, annotation/export refresh, concrete provider wiring and any gameplay
validation remain separate.

Peer review caught an ordering error inherited from580650 pseudocode:
580650/580656/580659 capture the owner/vector before580662 reads E194D8.
Both caller sites now preserve that assembly order. The correction is also
recorded in the report; strict compilation and the focused fixture were rerun.

# Listbox first selection, highlight positioning and paging

The canonical `GuiListboxRuntime` now contains the concrete helpers consumed by
Listbox frame/menu paths. It retains the same owner, FC rows, selected iterator,
listener and sound callback. The native second list at14C borrows the same row
identities, including duplicates. No row/widget/tree is cloned.

Evidence was read from `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; every live wrapper verifies these before its batch.
No Ghidra definitions, names, comments or saves were changed by this worker.
Names below are descriptive hypotheses. The C++ interfaces are not native
checked-STL/raw180h/SEH ABI replacements and have not run in the game.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| A9C310..A9C373 first selectable | ECX owner, RET | Complete normal valid FC list domain |
| AA0F50..AA0F6E highlight selector | ECX manager, signed selector stack, EAX borrowed widget, RET4 | Complete normal body over existing resource slots74/78 |
| A9ABD0..A9ABFD hierarchy factor | ECX native node, x87 float return, RET | Complete live acyclic parent30/scalarAC domain |
| A9C540..A9C73E highlight placement | ECX owner, row/paging low byte/depth stack, RETC | Complete normal sequence for established actual source/current34/current58 profiles |
| A9E230..A9E3F3 enable paging | ECX owner, signed page size stack, RET4 | Partial projection: complete successful allocation sequence, native allocator/SEH and allocator callback reentry excluded |
| A9D870..A9D9CA rebuild window | ECX owner, RET | Complete normal valid FC/14C sequence; current node and captured selected widget must survive callbacks; replacing14C during traversal excluded |
| A9D9D0..A9DA2D assign list | ECX destination, two checked iterators plus unused word stack, RET14 | Partial projection: E230 full FC range to distinct14C, normal clear/append only |
| A9D560..A9D673 insert range | ECX destination, destination/begin/end iterators plus unused word, RET1C | Partial: empty-end insertion normal A9D560..A9D604 and A9D661..A9D673; generic insertion/allocator/checked-STL/SEH excluded |

A9C310 walks FC until hidden77 is zero, stores that actual iterator, then calls
current80(false). Empty and all-hidden lists preserve selection without
notification. This differs from select-index, which clears selection first.
Callers are A9D101 in frame40, 5E8FA5/5E8FD1 in5E8B20, and raw5EC7AE.
The latter currently has no containing Ghidra function; no ownership attribution
was invented from its nearby instructions.

AA0F50 returns manager74 for selector1, manager78 for2, and null otherwise.
The existing AA5E20 producer fills those same `GuiResourceState` slots from the
actual `hl_FrameBox` and `hlCircle_FrameBox` children. It is not an array or
selector0 fallback. All seven direct callers were inspected: two5162B0 sites
pass1; CF30/C540/D030/CCE0 pass current118; D030's A9D327 passes0 and subsequently
dereferences the null result. That final native path remains an unresolved
caller boundary, not a newly supplied default highlight.

A9D301..A9D310 proves C540's arguments: the current148 paging byte, captured row
EDI and current highlight resolved depth. C540 first obtains the manager,
then reads current118, obtains its highlight and makes TWO resolved-position
reads (Y before X). It sets XYZ with the supplied depth before checking row.
A null row then hides the required highlight. A null highlight fails at the
initial native dereference, even when row is null.

For a nonnull row, C540 reads the row's current scene node after initial
position callbacks. It multiplies the live native parent30 hierarchy's scalarAC
factors with a float32 spill at each level, then dispatches highlight current34
with the resulting nonzero predicate. Unordered/NaN gives true, matching the
FUCOMIP/LAHF/TEST44 branch. A9ABD0 and the inlined final row product share the
actual native node registry, never a copied GUI alpha or parent hierarchy.

C540 calls the parent's concrete AC0820 fit using live width124, then stores
resolved position into the same F0/F4/F8 cache. It computes offset additions in
native x87 order with their float spills, captures the paging-dependent extra
height before the second position callback, then reads width and height
separately and dispatches actual current58 with live extra128. Required live
float aliases are D7A23C (bits3A83126F), D5BBF0 (BB03126F), D5BBEC (BBC49BA6).
Nonzero paging uses D7A23C for Y and D5BBF0 for height; zero uses D5BBF0 for Y
and D5BBEC for height. It does not reuse compile-time approximations.

A9DFD0..A9DFF0 initializes124,128,12C..134 to positive zero. A9E008..A9E03E
constructs the second14C list empty; it does not initialize158/15C/160/164/168.
Those fields, EC and F0 cache therefore use optional storage until their
observed producers write them. E230 writes EC=0 and148=1, clears14C and copies
FC pointers in order, writes signed158, resolves arrows, then writes164=0 and
168=positive zero before select-index0 and D870. Both menu callers at601EE4 and
6021F6 pass16. Nonpositive page sizes produce an empty FC window; no clamp or
invented default changes the signed comparisons.

Horizontal11F chooses ScrollLeft_Icon/ScrollRight_Icon; vertical chooses
ScrollUp_Icon/ScrollDown_Icon. The actual AA7E00 name adapter examines only the
borrowed direct-child list and current scene-node NativeString names; argument1
is unused, not a recursive-search flag. E230 publishes each borrowed arrow
slot before releasing its actual temporary pooled string. A missing arrow is
stored as null and fails only at D870's eventual dereference.

D870 saves listener114, clears it and captures the selected row pointer. It
repeatedly calls current34(false) on FC's current front identity, removes all
matching FC nodes, resets selection/cache, then80(false) and7C. The same
existing remove-row implementation reproduces this inlined native sequence.
It advances through14C by the live signed164, then repeatedly appends the live
window row through the established null-position A9D750 ownership transfer.
The page-size limit is re-read after callbacks. This moves the actual GUI child
allocation while retaining a separate borrowed14C entry; duplicate FC/14C
identities do not create additional GUI allocations.

The saved selected row is reselected by identity when nonnull, listener114 is
restored,80(false) runs, then current15C is shown followed by a fresh read of
160 after the first arrow's callback. Intermediate sound requests are retained:
clearing the listener does not suppress the existing paging-aware80/sound path.
Native restoration is a normal-sequence action, not a C++ exception guard.
Allocation failure, invalid iterators and callbacks that replace the current
14C list are outside the supported domain; replacement is detected before the
C++ iterator is advanced. Existing FC/current widget lifetime rules still apply.

D9D0's export has a misleading no-return gap after `_free`. Verified live bytes
and the installed PE decode A9D9F8..A9DA02 as ADD ESP4; CMP EDI,[ESI+4]; MOV
EAX,EDI; JNE A9D9F0; POP EDI. Every prior node is freed before D560. The exported
linear listing also includes a separate Catch_All@A9D606 function; its six
excluded EH call sites are attributed to D606 in the report. Existing correct
CRT/STL names remain unchanged.

The report records all65 CALL sites:59 numeric rows pass the live verifier,
with6 explicit symbolic virtual rows. Strict MSVC Win32 `/W4 /WX /fp:strict`
translation-unit compilation passed. The parent's new resolved-position,
relative-fitting and current58 providers are required link dependencies.
The final parent `scripts/build.ps1` Win32 Release build passed both existing
CTests. The focused `local/navigation_probe.cpp` passed against those production
libraries with no replacement TUs. It covers actual FC/14C duplicate identities,
first-selectable/all-hidden preservation, pooled direct-child names, repeated
nonempty14C replacement, listener suppression/restoration, a page-size change
during a real attachment callback, native node hierarchy factors, and C540
Group current34/58 plus null-row/invalid-selector behavior. The report pins the
build log, library, source, runner and executable hashes. Its Group highlight
profile does not claim installed FrameBox texture/geometry or Text rendering.
The existing CTests are math tests, not a navigation native differential.

The inherited resolved-position kernels are reused. Native differential and
parentless signaling-NaN load/store equivalence remain unverified; the normal
control/dispatch coverage is not a claim of all-float bit parity.

Full frame40 A9D030, directional84 A9DA60, CF30 width discovery, full properties,
non-null row insertion and native scalar/copy/teardown remain separate work.
`reports/gui_listbox_navigation.json` contains numeric sites, exact ABI/ranges,
corrections, caller audit and validation boundaries.

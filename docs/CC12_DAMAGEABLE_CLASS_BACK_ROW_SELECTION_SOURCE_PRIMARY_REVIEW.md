# Back-row selection Source primary review

The registered ordinary Source fragment covers only CE88..CEBA: 51 Native
bytes and 15 instructions inside 87CA80. It borrows the actual retained header,
raw S+DCh slot, successful state13 iterator/scratch and the same genuine
context service used by the preceding vector append. It owns no storage and
does not supply the application binding, full reader or original binary ABI.

Root reviewed the whole header and implementation. Raw inline x86 accesses
avoid inventing typed objects in the original header/cursor extents. The code
captures current end once, performs the first begin check and possible real
service call before any cursor store, then compares the wrapping candidate
against current end before publishing captured end to S+DCh. The saved
comparison preserves the short-circuit begin read. The second call precedes
the separate row subtraction; the final current end read calls the service
only for unsigned row>=end. There is no retry, row dereference, state/owner
change, callback cast, fallback service or UCRT-policy substitution.

A first callback failure precedes the fragment's slot write. Later failure
preserves publication and provider mutation. A provider's replacement of that
slot is neither overwritten nor used to recompute the eventual row. Normal
return carries the captured-end-derived raw address without certifying that
it remains inside the current vector. Existing ordinary enclosing owners
handle Source failure; the already closed state14 temporary is not retried.

Root replayed all 51 selected bytes/15 instructions and all 858 retained
parent starts against the unchanged original PE. No Native child, data,
handler or adjacent window was newly opened.

The normal MSVC Win32 Release build passes all three existing CTests. Its
actual retained CL command explicitly has /Oy-; the complete normal candidate
is five sections, 167 bytes and 64 instructions. This is distinct from the
worker's separately generated Release-default 190-byte object. Both worker
objects remain frozen; only the normal Root object receives current build
credit. Root verifies all three real indirect context-service calls, the
publication branches and preservation of the raw row across the final call.
The object has no relocation, project external, helper or new EH scope.
Complete physical symbols including auxiliary records and all sections/code
are retained. Absence of a direct external is expected for this explicitly
borrowed service; it is not proof of a concrete production callback binding.

Source627 retains 627 selected project inputs, 84 whole Core objects, three
whole App objects and 225 unique positive selected Core definitions. All 83
prior Core objects and all three App objects are byte-identical. The only
changed prior input is deferred registration. Root freezes 178 report pin
occurrences, verifies 38 baseline/current blobs, all 22 complete Source
snapshots and 106 actual compiler include pins, and replays all 629 prior
Source625 pins. This is a selected Source/artifact scope, not the complete
compiler/SDK or build dependency closure.

Repository tools record the fragment and append parent evidence. The Ghidra
write lock records previous annotations, preserves the full old parent
comment and name, saves the project and refreshes exports. No function body,
prototype or flow override changed. Earlier metadata captures at modification
5 remain historical captures; this annotation is a later project modification.

CEBB row fields, next iteration, complete parent, actual header/cursor/service
application binding, Native register ABI/FH3/SEH/longjmp/fault identity,
fixture execution, startup, Present and gameplay remain held. No new game
execution is claimed. The previous startup control stopped at FMOD output
initialization with no active audio endpoint.

Report: [normal build, exact code and preserved annotation receipts](../reports/cc12_damageable_class_back_row_selection_source_primary_review.json).

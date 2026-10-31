# CC16 raw node pointer-field representation

Raw scene/root/tree/base helpers previously evaluated typed DWORD or pointer
lvalues over fields whose existing C++ object type can differ. The exact seven-
CPP patch aa13b1a2 uses four-byte representation copies and compiler barriers.
Existing Node.scene_170 is already a real SceneResource* member; no pointer/Word
object is placed over it and no semantic SceneResource dereference is added.
Algorithms, signatures, Native call schedules and diagnostic exclusions remain.

The independent Source review18b3b481 and whole seven-file inverse pass. The
producer's standard MSVC Win32 Release baseline/after builds pass all targets
and both existing suites in11.97s/12.88s, MP2/PCH ON/coveragePDB OFF. Fourteen
actual014C objects retain250 matched literal section/function identities,228
complete code/relocation-exact sections,22 changed,2 added and1 unused pointer
reader removed. No new undefined/copy/barrier call or FP/DF/fence instruction
was added to the changed field graph. Fixed representation accesses are DWORDs.

Independent actual-Core review605ac28b and proposal-author expert review23668e57
accept the complete changed graph. B6F310's moved child-loop block preserves
reached order, CURRENT reloads and genuine decrement-completed before zero
terminal. B6F440 preserves normal/selected cleanup and its140B Source EH table.
B72110 and all19 surviving instance sections are code/relocation exact. The
Source-unchanged legacy NULL helper changes through reviewed B72220 inlining.

Root's same-worktree build passes existing2/2 in12.49s. Its three previously
reviewed Model/Matrix/Plain provider objects stay whole-byte exact. All252 after
field-code sections and relocations match producer objects under seven explicit
private TU-name mappings plus two constructor-role-paired renderer lambda-name
mappings. This is code/provider binding, not literal RTTI/whole-object/PE or
runtime equivalence across worktrees. Generic name stripping supplies no proof.

Published Source60 Main4fafe37 is joined at9e9e93176. The final official combined
build at94c76910e passes every target and existing2/2 in12.47s. All ten frozen
Root objects remain whole-byte exact; canonical flags/overlay are empty, all nine
raw switches stay false and all twelve Matrix/Plain requested entries remain
absent from the game map. No Game/provider entry was invoked. This closed packet
is ready for normal Main integration. Root artifacts are under
local/matrix_raw_outer_primary_cc16_20261031/Q0_actual_Source. Complete original
producer artifacts remain in the constructor worktree local/q0_raw_node_pointer_fields_gui_20261031.

This admits qualified MSVC32 ordinary accessible representation operations,
with real object birth/backing/credit/registry/provider and synchronized caller
lifetimes still required. It proves no Original private ABI/fault/EFLAGS/SEH/FH3
or volatile-I/O ordering, actual family/canonical permission, E0/Mesh failure
transport, S1/Mission/VM activation, visual/gameplay or whole-runtime parity.
No new tests, Native/vendor body credit, startup flag or game invocation.
The original strict13 USN12 discrepancy remains open and unchanged.

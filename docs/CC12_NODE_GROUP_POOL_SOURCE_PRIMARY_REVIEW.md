# Actual plain-node and group pool startup: primary Source review

Accepted worker commit `23239fa31` retains plain-node `0108FF58` and group
`010902F4` storage in the actual production resource-pool process, using the
same existing `E188B4` list domain. Plain-node storage is distinct from model-base
`0109008C`; the retained group companion is the one later consumers must borrow.
The real provider initialization and exit callbacks remain in use. Sticky once
states cache the actual registration status and reject cold access; a nonzero
atexit result does not invent cleanup or rollback.

The production Source sequence is plain-node, camera, mesh, model, model-base,
section, hierarchy, group. Eight actual REL32 calls in the compiled WinMain
physical section resolve to unique genuine Core definitions in this order.
These are explicit Source composition positions; original Native CRT-table order
has not been proved. At normal CRT exit their callbacks run in reverse while the
real process storage and shared list remain alive. Payloads must return all raw
slots before that cleanup. The worker's one focused Source probe observed all
eight real cleanup checkpoints and distinct storage; it constructed no payloads.

Primary normal MSVC Win32 Release build passed all three existing CTests.
Source585 is a selected project-input scope: 585 inputs, 66 selected whole Core
objects, three selected whole App objects, and 126 unique positive Core
definitions. The 64 unaffected prior Core objects are byte-identical; the actual
resource-pool object changed. Four fresh selected whole objects retain indexed
physical symbols, auxiliary records, sections, relocations, all executable
function spans and complete linear decode. Initial WinMain symbol-span boundaries
are not substituted for the whole physical section or whole-object coverage.
This selection is not the full compiler, SDK, preprocessing or application graph.
All 587 frozen Source583 pins and 74 worker pin occurrences were independently
replayed; worker evidence was copied before worktree reuse.

The actual Source585 production executable ran at 2026-10-10T00:03:27Z (PID
47788). Plain-node and group initialization each returned atexit status zero.
It then failed before device creation/Present with the same FMOD chain as the
earlier unchanged control: EventSystem_Init 61, CreateSound 78, GetLength 37;
bank_returned=0. No new Windows endpoint enumeration was performed for this
attempt, and the earlier zero-active-output diagnosis remains separate evidence.
Successful startup, Native ABI compatibility and gameplay remain unproved.

No new Native body, data or handler reads, GPR mutations, reconstructed Native
functions or descriptor consumers are admitted here. Forced allocation/binding
exceptions and nonzero registration paths have Source inspection, not injected
runtime coverage. The original installed executable is unchanged.

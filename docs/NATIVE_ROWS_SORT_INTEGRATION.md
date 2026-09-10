# Native row storage and pointer-slot sort integration

Primary integration closes 14 original bodies: three queue-row functions and
eleven pointer-sort/comparator functions. Thirteen are newly complete entries;
the unsigned-key predicate upgrades an earlier semantic interface. The former
interface remains available and is preserved in the new ledger record without
counting the original function twice. Earlier typed sort fragments remain
separate. This does not close queue acquisition, command execution or gameplay.

Worker commits `4f0bc07` and `37432f1` supplied four files each. Primary review
retained all four implementation files unchanged, registered both source files
in CMake, and rebuilt the actual library. It reverified 24 native spans totaling
2,236 bytes against both the installed PE and the live Ghidra program, 24
archived fixture artifacts, and 19 source blobs at their recorded Git revisions.

The primary reran both existing private fixtures against the newly built
`bsp_core.lib`, without separately compiling either reconstructed module into
the fixture. Row operations again matched 11,890 normalized observations,
including allocation failure, the actual native no-op unwind action, x87 NaN
quieting/status, callback field changes and retained dangling header fields.
The supplied string-storage and allocator boundaries remain explicit; the
fixture preserves its documented native EH registration plumbing.

Sorting again matched 594,239 trace words across 6,423 comparator calls and
933 final pointer positions. Four callbacks mutate actual equal-key slots.
The low-byte-only predicate contract, unsigned high/low key reads and no-access
signed-distance cases also pass. The original sorter executes its complete
closed helper family without call hooks. This establishes the checked native
mutation/order behavior, not arbitrary concurrent or invalid-memory behavior.

Two row functions had false no-return free continuations. Primary repair
restored B1DC49..B1DC52 within B1DB30 and B1F162..B1F166 within B1F150. The full
exclusive ends are B1DC66 and B1F167. Prior comments and unknown prototypes were
preserved; the project was saved and the function count stayed unchanged.

All 14 names received appended evidence, with old names/comments journaled and
the Ghidra project saved. All affected exports were refreshed. Strict MSVC
Win32 build and both existing CTests pass. No tracked tests were added.

The resulting public sorter and row interfaces are C++ interfaces over actual
native storage. The comparator edge uses its native register-call shape.
Neither packet is installed into the game, and neither establishes full queue,
frame, rendering or gameplay validation. See the detailed worker documents
`NATIVE_RENDER_QUEUE_ROWS.md` and `NATIVE_RENDER_POINTER_SLOT_SORT.md`, and the
primary `reports/native_rows_sort_integration_audit.json` for the complete
evidence, limitations, saved annotations and hashes.

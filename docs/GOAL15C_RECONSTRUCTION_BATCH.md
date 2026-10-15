# Checked counts, deferred release and avoidance cleanup

This batch adds four Source routines and repairs the existing hull destructor.
The list guards at `00420140` and `0041FDD0` preserve unsigned subtraction and
sum wrap, including accepted states above the nominal limit. Their complete
normal/error operation uses the genuine current SDK `length_error`; legacy
exception objects, FH3 metadata and physical operand-read ordering remain
qualified. The misleading unconditional-throw name at `00420140` is corrected.

The `00C34F70` routine appends a body DWORD to a deferred-release queue. Its
naked kernel preserves the native late writable argument-slot read and all
capacity/copy/free/publication/count ordering. Only its two allocator/free CALL
operands are redirected to the existing concrete current services. The 146
native bytes captured from Ghidra independently agree with the installed EXE.

The `0041E960` cleanup composes that queue with returning invalid-parameter
reporting, secure copy, genuine node frees and the existing typed hull destructor.
It borrows the actual game publication-cell address explicitly and reloads its
value for every queue call. Vector capacity and list sentinel remain allocated.
The Source context adjustment and current provider identities are documented.

The hull destructor at `00C37450` now captures its data pointer once. It reads
vertices from that captured object after releasing adjacency, then releases the
captured object. A genuine-release fixture safely reproduced the old library's
wrong release sequence `0,4,5`, where Native scheduling requires `0,1,2` after a
callback republishes the handle. The historical 719-case hull evidence remains
intact and is not represented as freshly rerun.

Packet evidence is in `GOAL15C_LIST_COUNT_GUARDS.md`,
`GOAL15C_DYN_RELEASE_QUEUE.md` and `GOAL15C_AVOID_CACHE_CLEANUP.md`.
Machine-readable integration results are recorded in
`reports/goal15c_reconstruction_batch.json`. Independent peer reviews found no
correctness blockers. No new tracked tests or test target were added.

Registered Win32 build and all three existing CTests passed. Linked fixtures
passed for both guards, queue growth/alias/wrap, the corrected hull publication,
and one complete cleanup composition. The latter confirmed four hull/node blocks
freed, retained capacity/sentinel/body/queue ownership, and all eleven fixture
blocks reclaimed after cleanup. Registered COFF comparisons and 24 report call
rows also passed. Ghidra annotations are saved and targeted exports refreshed.

These Source checks do not establish original CRT/exception/heap/physics-engine identity,
production ABI compatibility or gameplay validation.

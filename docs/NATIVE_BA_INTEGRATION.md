# BA debug-feature owner and MPAK record copy

Addresses: 0051F460, 00543E50, 00BB65A0, 00BB6630, 00BE8210,
00BE8350, 00BE8DF0, 00BE8F30, 00BE94F0, 00BE9560, 00BE9600.

This batch reconstructs eleven complete bodies / 1,296 original bytes in
isolated worktrees: eight debug-feature owner/record-array bodies (964 bytes)
and three MPAK record/shared-string-vector copy bodies (332 bytes). Existing
support routines and two directly executed native unwind actions are not
counted as new reconstructed bodies. Descriptive names remain hypotheses.

The singleton at 0109DB70 is a debug-feature configuration owner. The prior
FileStoreService name was based on segment adjacency; debug-menu and render
consumers supply the stronger current interpretation. Original names/comments
and failed call audits remain in the evidence. Primary repaired returning-free
flow in BE9560, BE8DF0 and BE9600, recreated the full156-byte destructor,
applied all eleven reviewed names/comments, saved Ghidra, and refreshed exports.
The six alignment bytes after the unconditional jump in BE8DF0 remain untouched.

The implementation, evidence and boundaries are in
[debug owner](NATIVE_DEBUG_FEATURE_OWNER.md),
[MPAK record copy](NATIVE_MPAK_RECORD_COPY.md), and
[the integration report](../reports/native_ba_integration.json).
All four implementation/discovery reports pass their direct-call audits;
the two Win32 imports in each debug report remain explicit unchecked boundaries.

The strict combined Win32 build and both existing CTests passed. The MPAK
fixture compares six original/source cases, including callback mutation and
name cleanup after nested-copy failure. Its fixed original rows were printed
before each source comparison in the retained development run; this was not
a separate original-only capture. BB6180 remains an explicit shared STL test
contract, and failure fixtures invoke original unwind actions directly. They
do not execute the original FH3 dispatcher.

Final acceptance requires a fresh strict build and both fixed fixture families
at one clean candidate commit. The external immutable receipt identifies that
exact commit; this tracked document is not a promotion receipt. The gate checks
compiler-recorded inputs across build/replay, linked object equality against
unique current library members, unchanged expectations, and sealed evidence.

No original ABI, native FH3/SEH, arbitrary virtual dispatch or game execution
is claimed. D68B94 dispatch through the application raw lifetime manager is
still pending: BD0400 and the shared deletion binding files are under another
orchestrator's active lease. The debug getter's known application edge is the
shutdown path, and the separate BE9100 loader has no established live caller.
These bodies do not establish an archive bootstrap or a running gameplay loop.

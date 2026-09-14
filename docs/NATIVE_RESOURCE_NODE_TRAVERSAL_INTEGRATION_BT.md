# Resource traversal integration BT

Addresses: BEA250, BEA680, BF02A0, BF03E0, BE9A40, BE9C40, 715BF0;
EH support CC7150, CC7158, CC7163, CC716B, CC71F0, CC71FB.

This batch adds seven complete ordinary source bodies (582 bytes) over actual
node/reader storage. It closes the child/control/skip prerequisites of the raw
resource dispatcher. It preserves current versus captured reader accesses,
wrapping actual-transfer budgets and parent declared-payload debits, and the
observed constructor unwind state order. One allocation-unwind membership gap
was repaired and two FH3 handlers defined under the Ghidra write lock.

Initial strict compilation and the actual-service traversal fixture passed
against frozen BS libraries. The fixture covers nested child construction,
control reads, seek/detach and releases, two call-boundary mutation checks and
one source constructor exception after an actual stream read. It does not
execute the native original bodies or prove FH3/SEH identity, binary ABI or
gameplay compatibility. Details and remaining paths are in
`NATIVE_RESOURCE_NODE_TRAVERSAL_BT.md`.

The integration report records the exact final code revision, source-input
manifest, final-library fixture and full build results when complete. Incoming
main work is reviewed separately from BT's evidence and remains subject to its
own stated limits. Production raw resource loading and queue shutdown remain
incomplete.

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

Final code revision `7a75e190ea90b929838738d8be7646ba9e6e5ba2` includes reviewed main `ec3b1f8bf2c833d72244a68a20b6bb3adc6b0c31`. The combined Win32 build and both existing tests passed. The traversal fixture passed linked to copied, hash-verified libraries from this revision; its manifest records 2635 exact build inputs. The audit verifies all 203 ordinary and 16 EH-support instruction starts, 18 direct transfers, two indirect sites and 13 preserved Ghidra annotations. The fixture includes a source exception injection; native original-body, FH3/SEH and game validation remain unperformed. Incoming unit-part compatibility and pilot floating-point claims were not independently certified by BT.

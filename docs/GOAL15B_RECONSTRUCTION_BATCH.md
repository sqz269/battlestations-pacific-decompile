# Reconstruction goal: registry and grid-storage batch

This continuation advances three explicitly leased packets from the current
repository state: registry-list teardown (`0041D710`, tail thunk `0041E290`),
the gameplay-settings vector cleanup (`0041F950`), and terrain-grid byte-vector
resize/insertion/erase (`004219A0`, `00420DD0`, `0041D7D0`). Source implementation,
review and registered validation are in progress. Existing correct library
names are retained; semantic corrections must preserve their prior evidence.

The allocator annotation at `00418540` is corrected from a zero-request null
claim to the actual `operator_new(0)` call. The original value was recorded in
[the batch receipt](../reports/goal15b_reconstruction_batch.json) before edits.
For every positive unsigned 32-bit request, `UINT_MAX / n >= 1`, so its local
exception-construction branch is unreachable for those inputs. The called
allocation service retains its separate failure/throw policy.

The cleanup closures have passed the registered MSVC Win32 build and all three
existing CTest suites. Primary and independent peer reviews found no blockers;
11 direct report rows and seven markdown call/jump claims pass. Their Ghidra
names/comments were saved and exports refreshed, including the allocator
correction. The thunk's Clear-to-Tidy correction retains its previous name and
evidence in its packet report. The grid byte-vector family remains in progress.

Source coverage, compiler/build checks, original ABI evidence and original-game
execution remain separate claims. These existing tests do not execute the new
cleanup entries or prove their original ABI/gameplay equivalence.

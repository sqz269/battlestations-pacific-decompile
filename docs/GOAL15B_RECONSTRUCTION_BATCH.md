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

Source coverage, actual compiler/build checks, original ABI evidence and
original-game execution remain separate claims throughout this batch.

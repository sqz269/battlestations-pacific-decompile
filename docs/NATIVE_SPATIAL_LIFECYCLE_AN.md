# Raw spatial detachment, refresh and index destruction

Addresses: `0042D4B0`, `00710B80`, `0098A2C0`, `0098A3D0`, `0098A4C0`, `0098A500`, `0098BC70`, `0098BDB0`.

The raw index created in [AL](NATIVE_SPATIAL_INDEX_PUBLICATION_AL.md) and populated by [AM](NATIVE_SPATIAL_ATTACHMENT_AM.md) now has complete source detachment, refresh and index-deletion bodies. These operate on the same actual raw storage, borrowed pose views and publication cells. They do not create an alternate index or ownership domain. The older `spatial_index.cpp` interfaces remain semantic projections; the new functions are in `native_spatial_lifecycle.cpp`.

## Evidence and ABI

The saved `bsp.gpr` program `/battlestationspacific.exe` and installed PE were verified before the analysis batches. Eight whole bodies, **877 bytes**, match live Ghidra and the original image. Every direct call has an `address`/`native` report row: **20 sites**. The formerly undefined vtable target at `CE3CEC[0] = 0042D4B0` was defined over `0042D4B0..0042D4D8` under the shared write lock, with the mutation recorded in `reports/native_spatial_lifecycle_definitions_an.json`. No neighboring function was changed.

| Native body | Native inputs and return | Recovered behavior |
| --- | --- | --- |
| `0042D4B0`, 41 bytes | ECX index, stacked flags; EAX original address, RET4 | Capture flag bit 0, unconditionally clear `F8A0D8`, write base profile `CE3818`, optionally call actual CRT free. |
| `00710B80`, 34 bytes | ECX part; RET | Any nonzero part `+184` gets the current index, detaches the part, then clears `+184`. |
| `0098A2C0`, 77 bytes | ECX parent, stacked child; RET4 | Decrement parent count first, swap the last child when the signed comparison permits it, update the moved child's slot, clear removed child's parent. |
| `0098A3D0`, 239 bytes | ECX index, EDX node; RET | Repair each inline link, then matching grid heads across the captured packed rectangle; clear node count. |
| `0098A4C0`, 52 bytes | ECX index, EDX node; RET | Search captured positive signed loose count, swap first match with captured last, decrement current count. |
| `0098A500`, 134 bytes | ECX index, stacked node; RET4 | Attached guard, child/loose/grid removal, root-list repair, final attached-byte clear. |
| `0098BC70`, 260 bytes | ECX node; RET | Pose/inverse copy, world bounds, changed-key grid replacement, recursive child refresh. |
| `0098BDB0`, 40 bytes | ECX index, unused float stack word; RET4 | Refresh dynamic grid roots, reading each current next pointer after the call. |

Source signatures add borrowed access in unused EDX where necessary; the destructor receives the actual publication-cell address there. Original register/stack contracts are recorded in the public header. These are explicit source interfaces, not drop-in binary replacements. Descriptive names are hypotheses, not recovered symbols. `_free` keeps its library name and existing canonical source CRT boundary.

## Ownership and ordering

Detachment retains stale child slots, the removed child's slot number, allocated child capacity, local/world bounds, cached keys and inline link fields. It does not shrink a parent's bounds or free its array. The unregister loop rereads the live signed count; the rectangle walk uses the key captured before link changes. The loose search uses its captured count for the last slot but decrements the current count. The source assembly keeps these reads and writes, including aliases, without adding recovery or classification checks.

Refresh copies the first captured owner's world matrix, reloads the owner before inverse validation, and sets inverse-valid before invoking the canonical inverse body. World bounds are refreshed even for a child. A changed grid key makes **two distinct** singleton lookups, one before unregistering and another before registering. No loose/grid reclassification or new span check occurs here. Four inline links and valid grid coordinates remain caller invariants.

Child-array begin and end are captured after re-registration; each element is read from that captured range immediately before recursion. Child static bytes do not suppress recursion. The outer root pass skips any root with nonzero `+8`, including its subtree, and does not visit the loose array. The existing name `RefreshMovedNodes` does not establish a separate moved-node queue.

Index destruction clears the publication even when it currently names another object. Only flag bit 0 controls free. It neither traverses nodes nor unregisters the index from its singleton manager. The caller must coordinate manager drain and node lifetime; no extra cleanup is invented here.

## Validation and limits

The strict MSVC Win32 build and both existing CTests pass. An ignored native probe compares full normalized raw index, node, owner and child-buffer images plus x87/MXCSR status for **26 paired cases**: 11 detachment cases, six refresh scenarios under two control words, and three destructor flag cases. The reference executes all eight original bodies with relocated direct calls. A repeat run produces the same results hash. The probe adds no repository test cases.

Detachment checks cover inactive guards, middle/last child removal, loose swap/no-match/nonpositive count, grid head/middle removal, and part flags. Refresh checks cover direct loose refresh, unchanged and moved keys, existing cell occupants, nonzero root skipping, static-child recursion, inherited pose refresh and same-frame cached bounds. Deletion checks flags `0`, `2` and `0x81`, including clearing a different publication and preserving the nodes. Freed index storage is never read; actual free linkage and flag ordering are also verified in compiled disassembly.

The reference and reconstruction share canonical source pose, matrix, inverse, world-bounds, cell-conversion, registration, publication and CRT services. This batch does not newly prove the original internals of those services. In particular, the floor callback uses the actual SDK service for this fixture; actual game CRT dispatch remains the existing attachment boundary. Pure pose-resolver lookups have no mutation side effects.

Native fault delivery, actual unit dispatch, nonempty raw index admission into the executable, and original-game graphics/gameplay parity remain unproved. The next integration must bind actual unit/part storage and its destruction path to this lifecycle and establish singleton-manager teardown order; an empty compatibility index is not evidence of admission.

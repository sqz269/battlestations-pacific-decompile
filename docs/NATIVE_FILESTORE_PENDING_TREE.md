# Native FileStore pending-tree erase closure

Addresses: `00BE4A70`, `00BE4A90`, `00BE4AD0`, `00BE5140`, `00BE6A20`,
`00BE7580`, `00BE7650`, `00BE7A70`. Names are descriptive hypotheses. Evidence is in
[native_filestore_pending_tree.json](../reports/native_filestore_pending_tree.json).

All eight actual-storage routines are complete within the existing source CRT,
string-pool and exception-service boundaries. No Ghidra mutation was made by
this worker; proposed names and previous identities accompany the ledger.

| Inclusive native range | Bytes | Coverage | Original ABI | Source operation |
| --- | ---: | --- | --- | --- |
| BE4A70..BE4A8B | 28 | complete | ECX node; EAX result; RET | maximum |
| BE4A90..BE4AAA | 27 | complete | ECX node; EAX result; RET | minimum |
| BE4AD0..BE4B21 | 82 | complete | ECX tree, stack node; RET4 | right rotation |
| BE5140..BE518D | 78 | complete | ECX tree, stack node; RET4 | left rotation |
| BE6A20..BE6CEB | 716 | complete | ECX tree; stack output/owner/node; RET0Ch | erase iterator |
| BE7580..BE7648 | 201 | complete | ECX tree; output/first owner/node/last owner/node; RET14h | erase range |
| BE7650..BE7683 | 52 | complete | ECX tree; RET; no semantic result | destroy tree |
| BE7A70..BE7AA3 | 52 | complete | ECX tree; RET; no semantic result | provider unwind destruction entry |

The 1,236 owned bytes and nine dependency/data/EH spans match current live Ghidra
memory and the installed PE. `BE5690` allocates `1Ch`, initializes links, and
writes color `+18=1`, nil `+19=0`. `BE63E0` writes all three links, string
length/data at `+0C/+10`, payload at `+14`, then color/nil. The existing provider
constructor establishes the actual pending tree at provider `+20`, with head
at tree `+4` and count at `+8`. The sentinel holds minimum/root/maximum at
`+0/+4/+8`. These declarations reconcile with `NATIVE_FILESTORE_FOUNDATIONS.md`
and `NATIVE_FILESTORE_SUBTREE.md`; they do not introduce another container.

`BE6A20` advances a by-value checked iterator using the existing `BE4E40` body.
It replaces a zero/one-child node directly; with two children, it transplants
the actual successor without copying the string or the pending callback word.
Both successor placements and both red/black repair directions follow the
FileStore assembly. The original node's key is released, that node is freed,
the current unsigned count is decremented only when nonzero, and the advanced
owner then node are published to output. Input owner is not compared with the
destination tree; only the iterator helper's null-owner validation is added
by the original call. Payload `+14` is never released or invoked.

The saved pseudocode incorrectly removes `BE6B2C..BE6B83`. At `BE6ABE` ECX
loads the iterator's advanced node, while EBP remains the original node. The
conditional jump at `BE6AC7` reaches that transplant block. EBX initially
captures the tree, which is also saved on the stack; after transplant it is
set to one at `BE6B84` and remains callee-saved across rotations and pool calls.
ESI tracks the repair parent and EDI the replacement child. Complete listing
inspection and the independently decoded byte span establish this provenance.

The stored Ghidra body ends at `BE6CB1` after `_free`. The actual continuation
`BE6CB2..BE6CEB` reloads tree/count, performs unsigned nonzero decrement, copies
the successor to output, restores FS and callee-saved registers, and returns
with `RET0Ch`. Similarly, `BE7674..BE7683` follows the stored destructor body's
`_free` and zeroes head/count before returning. These spans are continuations,
not newly inferred functions. The report records their absent Ghidra ownership
and exact inclusive endpoints for later locked flow repair.

`BE7580` checks the first owner, compares against the captured minimum, and
independently checks the last owner before recognizing the full-range fast
path. That path calls existing `BE66C0`, resets the current sentinel's links
and count, and returns `{tree,current minimum}`. Its partial-range loop checks
first/last owner identity on every iteration, advances its local first before
erasing the captured original node, and returns the final checked iterator.
Returning invalid-parameter callbacks continue at the original next step.
`BE7650` composes this full-range path, frees the current head and zeroes both
tree words. It does not release the tree object itself.

Provider state2 cleanup `CC6E03..CC6E0D` adds `+20` to the captured provider
and tail-jumps at `CC6E09` to the second destructor entry `BE7A70`. Its complete
52-byte instructions match `BE7650` except two relative CALL displacements,
including the raw `BE7A94..BE7AA3` head/count reset. The source retains a
separate `destroy_native_file_store_pending_tree_unwind_00be7a70` interface
and shares the already verified destruction schedule. The live xref labels
`CC6E09` a CALL, but the actual E9 opcode proves a tail JMP.

## Calls and exception evidence

Every direct owned CALL and each external incoming transfer to these eight routines
is recorded with call address, native callee and containing function. All live
incoming contexts were inspected: rotations also serve `BE6EE0` insertion;
`BE78B0` captures payload `+14` before iterator erase; `BE79C0`, `BE7A70` and
`BE7BF0` supply actual pending owner/node ranges. `BE7650` has no live incoming
references, so its execution is established only by the focused fixture.

| Native service | Contract and cleanup evidence |
| --- | --- |
| 408720 | Existing counted SBO assignment; ECX temp, text/count; RET8 |
| 411700 | Existing owning logic-error construction; ECX exception, stack temp; RET4 |
| BF6885 | Correct library `__CxxThrowException@8`; object/ThrowInfo stack; source owning transport |
| BE4E40 | Existing actual iterator successor; ECX iterator; RET or invalid-parameter tail |
| BE4A70 / BE4A90 | Inspected maximum/minimum leaves; ECX node, EAX result; RET |
| BE4AD0 / BE5140 | Inspected rotations; ECX tree, stack node; RET4 |
| BE66C0 | Existing complete subtree destruction; ECX tree, stack root; RET4 |
| 419CC0 / BD1510 | Actual pool getter, then current pool return; data/length+1/unused1; RET0Ch at BD1510 |
| BF65AC | Correct library `_free`; shared `singleton_lifetime_free`; ADD ESP,4 |
| BF6713 | Existing returning invalid-parameter boundary; no consumed arguments |

Sentinel erase assigns the 27-byte message, arms state zero only at `BE6A76`,
constructs the owning error, writes profile `D6926C` and throws with `D863A8`.
`CC6D28..CC6D32` loads FuncInfo `E01608` and jumps to the existing FH3 handler.
Map `E01600` contains state0 -> -1 through `CC6D20..CC6D27`, which computes
EBP-50 and tail-jumps `4072D0`. Existing `Unwind@00cc6d20` is preserved. Those
bytes, the profile and ThrowInfo agree with the established
`NativeHardwareLayoutInvalidIterator` owning source transport. The source guard
destroys only the completed temporary, and does not arm before assignment.

## Verification and limits

The strict MSVC Win32 build passed, then both existing CTests passed after the
eight native seed bodies were verified. No tracked tests were added. The ignored
fixture was adapted from the already established mount-tree differential harness
to actual FileStore bytes/layout and actual native string-pool services.

Its 20 native/source comparisons cover zero/one/two-child erase, both successor
placements, six balancing shapes, partial/full/empty ranges, populated/empty
destruction, returning invalid owners, post-release count mutation, zero count,
right-first subtree cleanup and owning invalid-iterator construction/throw
observation and the populated provider-unwind destruction entry. Snapshots compare normalized links, colors, nil, payload, padding,
counts, returned iterators, release order and actual pool bump/ring state.
The existing actual allocation leaf creates every non-sentinel node; the actual
pool lazily registers in the canonical lifetime domain and shuts down cleanly.

The fixture maps a private bounded copy of installed instructions with a uniform
address delta. It redirects only explicit dependency service boundaries to the
existing source CRT, real pool getter/return, counted-string and owning exception
helpers. The invalid throw is intercepted after checking the original profile,
message and ThrowInfo, with explicit fixture cleanup and saved-FS restoration.
That observation does not execute original FH3 dispatch or native unwind.

`local/pending_av/attempt02/manifest_before.json` physically seals 98 artifacts
before first execution: all 18 linked BSP objects equal archive members and
current objects; their source and BSP header closure, archive, executable,
original PE/native spans and driver are captured. All 98 hashes remained
unchanged. `run_fixture.py --repo <built repo> --attempt <new directory>` repeats
native verification and relinks the fixture for parent integration; attempts
cannot be overwritten and failures remain read-only. The earlier 19-case,
97-artifact `attempt01` remains preserved and read-only; adding the provider
unwind entry required a new build and a fresh attempt.

Source C++ RTTI/EH, the explicit noexcept string-release boundary, and host CRT
allocation remain the established interface limits. Arbitrary stack aliasing,
concurrent mutation, original FH3/SEH exception identity, binary replacement,
FileStore provider composition and gameplay have not been validated here.

## Correction from docs/NATIVE_FILESTORE_PROVIDER_LIFETIME.md

AV integration completed the proposed locked Ghidra annotations and required stored-body repairs, preserved prior comments, saved the project and refreshed affected exports. The combined Win32 build and both existing CTests passed. The fresh `local/native-av-pending-integrated01` fixture passed 20 native/source comparisons. All physical inputs remained unchanged; actual linked objects matched archive members.

The current source/header/object gate and preserved worker attempts are recorded in `reports/native_av_integration.json`. Earlier worker-only annotation and provider-composition limitations above are historical; actual provider composition is now separately tested with one actual pool/raw manager domain. Original FH3/SEH, zero-reference provider stream terminal coverage and gameplay remain qualified.

# Native render-group construction and binding storage

`native_render_group_storage` implements the complete `00B1D6F0`
constructor and `00B1CA50` retained-binding assignment over actual Win32
storage. The source-entry array specialization adds complete `00B1C4F0`,
`00B1C500`, `00B1C770` and `00B1D1D0` operations to the shared raw array API.

`NativeRenderGroupStorage` occupies the original 4Ch allocation. Its binding,
name, two counts, two output entries, two model pointers and two source-array
headers have compile-time offset checks. Construction writes exactly the
first 3Ch bytes in the original order. The final 10h allocation preimage is
preserved. No implicit owner cleanup or additional reference count is stored.

The source arrays use the already reviewed shared raw pointer reserve/resize
implementation. Reserve clamps to one and grows only; resize clears exposed
cells, while shrink preserves stale cells. Destruction sets count to zero and
frees the actual data allocation, leaving its data and capacity words intact.
Pointed entries remain borrowed.

Binding assignment captures source and old identities before publication.
On an identity change it stores the incoming raw pointer, increments that
owner's actual +04 atomic, and only then releases the captured old +04.
The canonical owner resolver runs only at zero and must return a companion
borrowing the same atomic. That companion executes its current terminal
operation. Same identity, including a source-cell alias of the destination,
does no reference traffic. There is no rollback on a failed terminal lookup.

The original constructor receives ECX = actual group and returns that same
address in EAX with `RET`. Assignment receives ECX = destination pointer cell
and EDX = source pointer cell, returning the destination address with `RET`.
The new C++ interfaces are not drop-in replacements for those register ABIs.

One extension of the existing native context-lifetime fixture checks the
entire group preimage, source arrays, and binding replacement with actual
context owners. The old owner's original destructor/scalar deleter really
frees its allocation; its terminal observation sees the new binding already
published and retained. The original and reconstructed runs match **549
normalized words**. This establishes generic intrusive binding behavior,
not the still-separate full instance-generator or model/geometry construction.

The reusable node constructor also now accepts any aligned allocation
covering its actual 174h prefix. The former 1F0h minimum was the node pool's
slot size, incorrectly imposed on the original 188h model caller. Reusing
the existing node fixture at 188h gives an exact **392-byte** comparison,
including untouched derived bytes and the trailing pool ID. The original
node pool itself is unchanged.

The strict Win32 build and both existing CTest checks pass. Native EH
execution and gameplay are unvalidated. `00B1D760` group destruction,
populated generated-model composition, and the full native command/queue
remain subsequent work. Exact spans, source hashes and integration records
are in `reports/native_render_group_storage_audit.json` and
`reports/native_render_storage_integration_audit.json`.

# Native keyboard iterators and scale-range erasure

Addresses: `00546840`, `00552770`, `00552D40`, `005540C0`, `0048D3B0`,
`0055B230`; supporting `00552E20`, `0055AC80`.

The actual-header integer-set lookup uses signed keys, retains the lower-bound
candidate, and publishes the output owner before the node. The three checked
increments use specialization nil bytes (sensitivity 29h, codes 25h, devices 99h),
walk the right subtree or parents, and publish intermediate ascents.

Packed-bit increment reads the actual bit count and begin/end fields. Zero
distance returns immediately, including an unbound iterator. Nonzero distances
retain native unsigned range arithmetic and backward DWORD crossing. The native
comparison covers all 9,409 pairs of positions in a 96-bit range plus the unbound
zero-distance bypass.

The existing typed range-erasure engine now exposes outer signed-key nodes of
20h (nil1Dh), whose mapped values own inner scalar nodes of 18h (nil15h). Full
reset and successor/transplant/rebalance paths release nested payloads before
the node and decrement the current count after release. Output publication
follows capture of the resulting node.

| Entry | Original ABI |
| --- | --- |
| 00546840 | ECX tree; stack output, key; EAX output; RET8 |
| 00552770 / 00552D40 / 005540C0 / 00552E20 | ECX iterator; no stack arguments; RET |
| 0048D3B0 | ECX iterator; stack signed distance; EAX iterator; RET4 |
| 0055B230 | ECX tree; stack output, first owner/node, last owner/node; EAX output; RET14h |
| 0055AC80 | ECX tree; stack output, position owner/node; EAX output; RET0Ch |

The saved 0055AC80 body had ended after its first free. Repair restored all 728
bytes and verified instruction ownership and live/disk bytes. Giving 00552E20
its actual iterator argument also restores the decompiler's two-child successor
branch, hidden by its old no-argument signature. No shared callee no-return flags
were changed. Receipts and independent static review are indexed by
`reports/native_input_keyboard_storage_ax_validation.json`.

These new source interfaces require valid consistent owned containers. Original
CRT invalid-parameter handler identity, FH3 exception ABI, malformed storage,
topology-mutating callbacks, private-stack aliases and hardware faults remain
outside the validation claim. Application routing and gameplay are separate.

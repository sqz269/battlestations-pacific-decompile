# Pending entity library provider reuse

Addresses: 00924B10, 008665F0, 009267F0, 008675E0, 00CA6D58, 00C94EA8.

The pending entity producer helpers can reuse two existing concrete providers
from `effect_deletion_queue.cpp`. No new STL body is implemented.

| Pending helper | Existing provider | Complete native evidence |
| --- | --- | --- |
| `00924B10` | `create_effect_deletion_node_008665f0` | Both 51-byte bodies match after their sole relative CALL is resolved to the same BF681B allocator |
| `009267F0` | `grow_effect_deletion_list_count_008675e0` | Both 147-byte bodies match after resolving identical CALL targets and normalizing the separately verified FH3 handler identity |

The node ABI is three stack DWORDs, EAX allocated node and RET0C. It publishes
next, previous and the dereferenced source cell in order, including the native
separate wrapped-address guards. Both use the same raw 0Ch node layout already
aliased by `NativePendingEntityNode`.

Count growth takes ECX=actual list and one stack increment, RET4. Both preserve
the unsigned `3FFFFFFF-count` check, captured-count addition, exact string and
throw-type pointers, and all three shared call targets. The two full 10-byte
handlers select equivalent 36-byte FuncInfo records; each has one unwind-map
entry transitioning state0 to -1. Their complete eight-byte cleanup thunks both
pass EBP-50 to `004072D0`. This establishes the static cleanup graph, not native
exception transport compatibility of the modern source provider.

All 12 spans match live Ghidra reads and the installed PE. Exact bytes, hashes,
normalizations and provider boundaries are recorded in
`reports/pending_entity_library_reuse.json`. The pending FH3 handler at CA6D58
was formally defined and saved as a 10-byte function after disk/live verification;
`reports/pending_entity_library_function_definitions.json` retains the result.
Affected annotations were applied under the write lock and exports refreshed.
The two list helper names retain their existing library identities.

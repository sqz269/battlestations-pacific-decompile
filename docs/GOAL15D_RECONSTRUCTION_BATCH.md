# Concrete avoidance group and manager lifetime

This batch completes the native-storage manager clear and supplies its concrete
group-deletion chain, together with the grid's three-word node producer. It
covers five native entries. Existing typed group routines and the older partial
manager projection retain their original records and evidence.

| Entry | Result |
| --- | --- |
| `0041C0B0` | Complete 51-byte stdcall node producer, preserving late stack/value reads, individual wrapped destination guards, EAX result and RET 0Ch |
| `0041F600` | Full twenty-slot virtual deletion loop followed by the actual cache-cleanup tail at `0041F635`; explicit borrowed publication-cell context |
| `0041D0C0` | Complete 132-byte group clear with current pointer/count reloads and real record/array/zone frees |
| `0041DE70` | Concrete destructor and recovered array cleanup during current Source C++ unwinding; actual Source vtable installed |
| `0041E940` | Complete 30-byte scalar deleting entry; bit 0 selects receiver free and the original pointer is returned |

The group destructor's native state-0 action remains active through its normal
array-free attempt. Handler `00C5E1DB`, FuncInfo `00D84330`, map `00D84328` and
action `00C5E1D0` establish the cleanup: free the current nonnull group array,
then null that field after free returns. The Source scope guard performs that
operation. MSVC initially removed its exception registration, so a volatile local
initialized to the concrete clear entry retains the emitted guard without
introducing an injectable provider. Registered COFF inspection confirms the
current EH registration, state, unwind funclet, guard and free/null helper.
The 95-byte Source destructor does not claim equivalence to the 87 native bytes.

The actual Source table points at the reconstructed scalar deleting entry.
Manager dispatch reads each group's current slot zero, calls it with flags 1,
and nulls the same manager slot only after return. Its tail uses the complete
cache cleanup from GOAL15C, including deferred body queueing and hull/node frees.
No abstract deletion or success callback supplies this chain.

The registered Win32 build and all three existing CTests passed. Two ignored,
manifest-embedded fixtures were compiled against the primary headers and linked
with the actual Release archives. Node allocation/fields/free passed. The
manager/group/cache fixture passed with real virtual dispatch, populated and
null zone slots, distant manager slots, deferred body queueing, retained storage
and flag-0 receiver retention. Eleven tracked allocations were used before the
manager clear and absent immediately afterward; the preallocated queue prevented
reuse during that observation. This is bounded Source heap evidence.

The primary reproduced comparisons for 271 native kernel bytes against 273
Source bytes. Exclusions are the manager's two context bytes and seven redirected
CALL/JMP operands; all other kernel bytes agree. The native captures also agree
with the installed EXE. Thirteen direct report rows and seven markdown claims
passed; the indirect dispatch claim is covered by the listing, kernel comparison,
table relocation and actual composition fixture.

Ghidra annotations are saved and exports refreshed. The node's incorrect saved
`undefined(void)` prototype is corrected to a three-argument stdcall pointer
result, with the old declaration retained in a local rollback receipt. The
prototype uses opaque DWORD data and introduces no payload class.

Machine-readable evidence, exact hashes, logs, exclusions and rollback paths are
in `reports/goal15d_reconstruction_batch.json`. Packet details are in
`GOAL15D_GRID_LIST_NODE.md`, `GOAL15D_AVOID_MANAGER_CLEAR.md` and
`GOAL15D_AVOID_GROUP_LIFETIME.md`. No new tracked tests or test target were added.

Objects passed into the new Source dispatch must already carry its matching
actual Source table. Original production group construction/adoption is not
supplied by this batch. Dynamic unwinding, original FH3/SEH/heap and
double-exception identity, additional native virtual slots, production ABI binding
and gameplay remain unproven.

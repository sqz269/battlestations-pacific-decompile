# Application constructor exception metadata

The constructor's own `CC7240` metadata selects two unwind actions: state 1
targets the local guard cleanup and transitions to state 0; state 0 targets
the saved receiver's base cleanup and transitions to `-1`. These facts were
recovered independently from the constructor handler, its absolute-x86
FuncInfo/UnwindMap, and both direct action bodies.

The intended frame operands match the captured `BEA810` parent and current
`ehdata.h` layout. The shared handler's actual helper-frame setup, dispatch and
nested-fault behavior remain outside this audit. This is neither a new Source
exception wrapper nor a claim of automatic rollback, unlock or owner deletion.

The [report](../reports/cc12_application_constructor_EH_readiness.json) retains
the raw captures, selected instructions, complete metadata, parent replay,
frame arithmetic, current Source pins and qualifications. The starting main
was `287e0bb7af049a33ad6e75f0e9e67113066a6770`.

## Scope, target, and independent handler

The existing `bsp` project and `/battlestationspacific.exe` were verified,
configured at `C:/Users/sqz269/bsp.gpr`. Live count remained **64729** before
capture. Leases were extended from `CC7240` only as its actual metadata and
direct action addresses were discovered.

`CC7240` has no function start in lookup. Its first ten bytes decode completely:

| Site | Instruction |
| --- | --- |
| `CC7240` | `MOV EAX,00E01CC4` |
| `CC7245` | `JMP 00BF6B43` |

The shared `BF6B43` target was not followed. A bounded 32-byte capture includes
the ten-byte handler, six alignment `CC` bytes and sixteen incidental following
bytes. Those final 22 bytes are excluded from selected handler/action evidence.
No neighboring handler or action was decoded from that tail.

Lookup's enclosing-candidate association with `CC7238` does not extend that
action: its live body ends at `CC723F` after a tail jump. The handler was decoded
separately without changing saved analysis. Root's retirement `CC7260`
metadata/actions were not used to supply constructor facts.

## Absolute FuncInfo and unwind map

The complete 36-byte FuncInfo at `E01CC4` has raw magic `19930522`, zero BBT
flags, `maxState=2`, and absolute `pUnwindMap=E01CB4`. Try-block count/pointer,
IP-map count/pointer and exception-specification pointer are all zero. Raw
`EHFlags=1` is retained without inferring original compiler options or
asynchronous-fault coverage.

The installed `ehdata.h` defines `maxState` as the highest state plus one and
uses signed state values. The complete map therefore has two eight-byte
entries, below the four-entry cap:

| State | To state | Action | Complete direct action |
| --- | --- | --- | --- |
| `0` | `-1` | `CC7230`, 8 bytes | `MOV ECX,[EBP-18h]`; `JMP 00412430`. |
| `1` | `0` | `CC7238`, 8 bytes | `LEA ECX,[EBP-14h]`; `JMP 00411EE0`. |

Each action is two instructions and passed its individual 300-byte gate. The
first loads a pointer from a frame slot; the second passes the address of a
frame object. They are tail transfers, not ordinary calls with a newly pushed
return address. Neither direct cleanup descendant was queried.

The map describes `1 -> 0 -> -1` when the runtime selects and successfully
completes those actions. It does not establish the shared runtime's dispatch
conditions, state-write timing, target-state selection or response to an
exception inside a cleanup.

## Parent frame and action operands

The published complete 145-byte/40-instruction `BEA810` parent was replayed
against the installed PE without a fresh parent query. Let `S` be its incoming
ESP, pointing at its return address. The prologue publishes the registration
node at `R=S-0Ch` and establishes quiescent body ESP `Q=S-20h`.

| Parent slot | Relative to S | Relative to Q |
| --- | --- | --- |
| Saved receiver word | `-18h` | `+08h` |
| Guard profile word | `-14h` | `+0Ch` |
| Guard section word | `-10h` | `+10h` |
| Saved exception-chain link / registration node | `-0Ch` | `+14h` |
| Handler word | `-08h` | `+18h` |
| State word | `-04h` | `+1Ch` |

Current x86 `ehdata.h` defines the registration node as three DWORD fields and
`PRN_FRAME` as node address plus its size. That gives `R+12=S`. **If the actual
shared helper establishes EBP at that frame address**, `[EBP-18h]` resolves to
the parent's saved receiver and `LEA[EBP-14h]` resolves to its actual eight-byte
guard. Both operands agree independently with the physical parent stores.

This is a corroborated intended mapping, not proof of the original helper's
EBP setup. `BEA810` itself does not establish EBP, and `BF6B43` is outside scope.
The current header is a layout reference, not the original compiler/runtime
implementation. A native helper-frame adapter is not supplied.

The actions use actual frame memory: action 0 reads the current saved receiver
word, while action 1 passes the guard address so its cleanup can read the
guard's current section word. Replacing those operands with current `E1AE90`,
another owner, or assumed surviving EDI/ESI values would require different
evidence.

## State arming and normal-release limits

The initial pushed state is `-1`. `BEA82C` saves the receiver before `BEA830`
stores whole state DWORD zero, **before** the `D68BC4` owner-table store.
State 0 covers the first manager lookup, guard materialization, optional
EnterCriticalSection and the following tracked-counter increment. It selects
only action 0; the guard action is not yet armed by the parent state.

`BEA861` stores low state byte one after optional entry/increment and before
publication. State 1 covers publication, the second manager lookup,
registration and normal section release. The complete parent has no later
state decrement or disarm before restoring its exception chain.

In particular, normal tracked decrement at `BEA883` and Leave at `BEA888`
execute while state 1 remains recorded. This audit does not infer a guard
action for every fault during entry, exactly-once release during a failure
in normal release, or successful remaining actions after a cleanup fault.
Shared-handler selection, cleanup descendants and nested exceptions remain
explicit limits.

## Concrete current Source cleanup services

`destroy_native_generic_singleton_base_00412430` currently performs one
volatile DWORD store of `CE3818` through the supplied receiver. Its header
identifies the ordinary base cleanup. It is not the `00412440` scalar deleter;
substituting that scalar would add a different ABI and ownership policy.

`destroy_native_singleton_guard_00411ee0` currently reads the actual guard's
`+4` section pointer, stamps `CE37FC`, then, for a nonnull section, decrements
its actual tracked DWORD `+18h` and calls LeaveCriticalSection. It preserves
the guard's section word. These concrete Source bodies and headers are pinned;
their native descendants and compiled bodies were not requalified here.

Both are ordinary explicit-pointer C++ interfaces. Their behavioral reuse does
not make them drop-in native ECX funclet targets or supply the shared helper's
frame ABI. The base Source declaration's `noexcept` and another Source caller's
RAII, catch or termination wrapper are not constructor exception policies.

Neither selected action sequence directly writes `E1AE90`, unregisters an
object or selects an owner free; the selected Source leaves add none. No total
publication/registration rollback or outer-owner lifetime result is established.
A future Source composition must retain those boundaries explicitly rather
than invent cleanup from this two-state map.

## Validation and remaining readiness

All **100 freshly captured bytes** match the installed PE. The selected
constructor evidence is **78 bytes**: 26 code bytes/six instructions plus
52 metadata bytes. The unused capture tail is counted separately. The reused
145-byte parent also matches and decodes completely.

The installed `ehdata.h` is hashed with bounded layout excerpts. Current cleanup
Source, target and parent pins were checked, including all 78 canonical pin
references inherited from the parent audit. Historical service audits were
pinned for identity; their builds, fixtures and Native results were not rerun
or imported as constructor proof.

Constructor map/action edges are now known. Actual shared-helper frame/dispatch
and fault compatibility, or a separately qualified Source failure policy,
still precede complete genuine Application owner, callable-profile and lifetime
integration. Only this document and its report changed; no Source, CMake,
ledger or GPR mutation, build, test, probe, native execution or new Original
credit occurred.

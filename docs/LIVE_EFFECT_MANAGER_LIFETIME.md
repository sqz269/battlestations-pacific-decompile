# Complete live-effect manager lifetime

Packet `orch3_live_effect_manager_lifetime_v` composes the established actual
28h F8765C manager constructor, owning-array operations and pending-deletion queue
into the complete getter, destructor and scalar deletion path. Source is
`src/live_effect_manager_lifetime.cpp`; declarations are in its matching header.
Names describe inferred behavior; existing reviewed getter/constructor and correct
compiler scalar names are retained with appended evidence.

| Span (exclusive end) | Original ABI | Behavior |
| --- | --- | --- |
| `004BA0F0..004BA10A` | ECX 0Ch header; RET | Free/clear auxiliary backing only |
| `004CDEA0..004CDEB7` | ECX 0Ch header; RET | Resize owning array to zero, free current backing |
| `004C8C20..004C8C25` | ECX list; tail JMP | Thunk to complete 4C5940 list-storage destruction |
| `004D1100..004D11BD` | No inputs; EAX published manager; RET | Locked lazy actual-owner getter |
| `00867C00..00867C98` | ECX actual 28h manager; RET | Complete manager destruction |
| `004CF770..004CF78E` | ECX manager, stack flags; EAX original; RET 4 | Scalar destruction/free |

## Ownership and normal destruction

The actual owner layout remains CE789C identity at +00, untouched allocator word
at +04, pending-list head/count at +08/+0C, and array headers at +10 and +1C.
The complete destructor establishes different teardown policies for the arrays.
It releases the actual owners in +10; +1C only frees its backing and clears its
data field, leaving count/capacity unchanged. The earlier constructor-only note
calling both arrays owning was too broad. That note/header/report and constructor
ledger evidence are updated here; earlier annotation text is preserved as history.

867C00 writes CE789C and performs these operations in native order:

1. Resize actual effects+10 to zero through 4C9550, including actual +04 releases.
2. Flush the current pending list through 8671A0 and required current scalar +04.
3. Free captured nonnull auxiliary+1C backing, then clear its current data field.
4. Resize effects+10 to zero again, then free its current backing.
5. Destroy pending-list storage+04 through 4C5940, clearing its head and count.
6. Clear F8765C unconditionally and restore base CE3818.

Callbacks in the first two phases may repopulate +10, so the second resize is
observable. Owning-array destruction leaves the freed data pointer/capacity stale.
Auxiliary destruction preserves count/capacity and issues no cell releases. The
list helper frees node storage without another payload dispatch. No internal lock
or extra pending flush is added to the destructor.

4CF770 always destroys first. Only flags bit zero requests ordinary owner free;
other bits are ignored for allocation disposal. A destructor exception propagates
without freeing the owner. It does not unregister: the canonical lifetime domain
already pops the actual owner before invoking its registered-owner callback.

## Unwind and singleton registration

Handler `00C94F99..00C94FA3` selects FuncInfo DC6E44, map DC6E24:

| State | Next | Action |
| --- | --- | --- |
| 3 | 2 | C94F8E -> 4BA0F0 on owner+1C |
| 2 | 1 | C94F83 -> 4CDEA0 on owner+10 |
| 1 | 0 | C94F78 -> 4C8C20 on owner+04 |
| 0 | -1 | C94F70 -> 4B7F50 on actual owner |

The main body sets state 3 before its first resize and keeps it through pending
flush and auxiliary disposal. Before the second resize it sets state 1, skipping
the owning-array cleanup state. A failure in that second resize therefore still
destroys list/base but does not free the owning-array backing. The C++ guard
preserves this difference and does not retry pending payload destruction. A
failure during C++ unwind cleanup follows its nonthrowing cleanup boundary;
native nested-exception dispatch is not claimed.

4D1100 uses the application's same actual `SingletonLifetimeDomain` for 01090AA0.
Its fast path returns the initially captured nonnull F8765C. Its slow path captures
the lifetime manager's +10 section, enters it, rechecks F8765C, allocates 28h and
constructs nonnull storage through 4CF700. It publishes the result, obtains the
lifetime manager again and registers the current global pointer. It unlocks the
captured section and reloads F8765C for the return. Publication is not rolled back
if registration fails. The native null-allocation branch is retained.

Getter handler `00C65E73..00C65E7D` selects D8EA20, map D8EA10. State 1 -> 0
uses C65E68 to free captured raw owner storage after constructor unwind; state
0 -> -1 uses C65E60 / 411EE0 to unlock. Constructor failure first runs the already
reconstructed global clear/base restore in 4B7F50, then raw free, then unlock.
The actual domain's required callback must invoke 4CF770 with established owner
and pending-scalar bindings. No replacement lifetime domain or success stub exists.

## Evidence and validation

All six complete bodies, two handlers and five supporting body spans matched
the installed executable and live `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Hashes and annotation history are in
`reports/live_effect_manager_lifetime.json`. Ghidra writes waited for another
orchestrator's active lock, then proceeded through the normal serialized tools.

`reports/live_effect_manager_flow_v.json` records restored interior continuations:
10 bytes at 867C4C, three at 4CF785, and nine at 4BA0FF. Two stored-body limits
remain: 867C00 ends at 867C6B and omits the 44-byte tail through 867C97; 4CDEA0
ends at 4CDEB1 and omits the five-byte tail through 4CDEB6. Those full tails were
verified in live/disk bytes and are included in source and copied-original probes.
Zero interior gaps does not imply complete stored Ghidra bodies. The destructor
tail contains list cleanup, global/base publication, exception-frame restore and
RET; the owning-array tail restores ESP/ESI and returns.

Strict Win32 build and both existing CTests passed. No permanent tests were added.
Ignored `local/live_manager_probe_v.cpp` runs all six complete original bodies,
with the native constructor/sentinel, resize, pending flush and list destructor.
It compares the complete dead 28h header with only stale allocation addresses
normalized; verifies auxiliary count/capacity and unretained cells, the second
release after pending callbacks repopulate +10, direct helper behavior, scalar
flags 2 and 1, getter fast path and actual shared-domain shutdown. The same domain
also owns the deletion lock created during a zero-owner callback.

Native allocator/free/Interlocked/lock operands are rebound to canonical allocation,
free and Win32 operations. Internal calls use the copied native bodies. The unused
resize growth branch points to canonical reserve. Lifetime getter/registration
bridges expose the actual native section address of the same concrete domain and
register the actual published owner. F8765C operands use the fixture publication
cell. Instrumented raw-owner terminal/scalar callbacks preserve the actual +04
count contract, enqueue through the reconstructed real queue, release their owned
payload and keep dead fixture wrapper storage for observations. These callbacks
exercise composition; they are not proof of game effect scalar implementation.

The ignored probe recompiles only the lifetime and established storage modules
with allocation/free symbols rebound to canonical delegates for inspection and
one 12-byte allocation failure. Production gains no seam. Focused C++ cases prove
state-3 cleanup after a throwing pending scalar, state-1 cleanup after a missing
zero-owner binding on the second resize, no scalar owner free on either failure,
and constructor global/base cleanup plus raw free while depth is one before final
unlock. Native handler immediates remain unchanged; original exception dispatch
was not executed. These are new C++ interfaces, not original ABI replacements.

Concrete effect current-virtual dispatch, point-effect virtual zero's frame-job
policy, whole point-effect construction, and gameplay validation remain work.

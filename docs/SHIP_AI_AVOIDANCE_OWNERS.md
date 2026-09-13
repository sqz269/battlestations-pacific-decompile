# Ship AI avoidance ownership prerequisites

This is a read-only producer and runtime-owner assessment for `009DA6E0` and
the sector callback to `00415970`. It reconstructs no new routine and changes
no runtime source or Ghidra state. Existing request, director, constructor and
query implementations remain the contracts to reuse. Findings are static;
this packet makes no frame-execution or gameplay claim.

The current process has a command-director owner and a separate persistent AI
request, but its director projection lacks the avoidance bytes, and consumers
do not consistently read the live request. The checked native allocation chain
does **not** initialize the searchers' box words. Those words cannot acquire a
native zero default merely by constructing the current semantic searcher type.

## Searcher storage before the first query

Let `composite` be the allocation returned by `009F3F20`, `brain=composite+58h`,
and `nav=brain+8h`. These are different receivers; historic names sometimes
call more than one of them the brain.

| Native site | Receiver and action | Coverage of this assessment |
| --- | --- | --- |
| `009F3F3E` in `009F3F20..009F3F7F` | `operator new(2268h)`; caller cleans four bytes at `009F3F43` | Complete allocation wrapper |
| `009F3F59` | Allocated pointer in ECX, original unit preserved in ESI and pushed; call `009F3BA0` | Complete allocation wrapper |
| `009F3BC1` in `009F3BA0..009F3C70` | `0072BBD0(composite,0)`, RET 4 | Pre-brain construction and receiver setup |
| `009F3BE5` | EDI=`composite+58h`, ECX=EDI, unit pushed; call `009F39C0` | Same |
| `009F39E5` in `009F39C0..009F3B99` | ESI=incoming brain, ECX unchanged; call `009F1160(unit)` | Pre-navigation construction; later state initialization not reconstructed |
| `009F118D` in `009F1160..009F1413` | ECX=`brain+8h`; call `009E4330(unit)`; the only prior object store is the brain vtable at `009F1187` | Pre-navigation construction; later brain state not reconstructed |

Live xrefs confirm the single callers between these constructor levels.
`0072BBD0..0072BC6D` has no callees and writes only within composite offsets
`00h..57h`; its last store is the float at `+54h`. It cannot initialize any
embedded navigation searcher. `009F39C0` and `009F1160` perform no earlier bulk
initialization. The allocator is `00BF681B`, whose success path calls `_malloc`
at `00BF6833`. No zero-fill contract is established: the inspected ordinary
heap path in `00BF9F1A` passes flags zero at `00BF9F8B`. A fresh-page accident
must not be treated as a producer for reused heap allocations.

The three searchers begin at nav `A24h`, `A44h`, and `A64h`. `009E4401..009E4449`
writes only their enabled byte `+00h=1`, integer cache key `+14h=-1`, and list
words `+18h=0`, `+1Ch=0`. It leaves `+04h,+08h,+0Ch,+10h` untouched. The
constructor's existing `ShipAiNavBlockFields::Memo` accurately omits these
box words. Its other callees remain existing bounded contracts; this packet
does not infer arbitrary writes through opaque external calls.

The initial key does not remove this storage dependency. `009D7068` compares
integer keys, and a mismatch jumps to `009D70F9`, which reads old max-x/min-x
and max-z/min-z at `009D70F9..009D710B` before computing the replacement box.
Disabling at `009DA724/009DA76E/009DA7BB` clears a list and resets its key; it
also does not write the box. Therefore neither the constructor sentinel nor
an enable transition proves initialized bounds.

The first binding needs actual initialized/captured box storage, or an explicit
supported-domain policy acknowledging that native allocation bytes were not
reproduced. This assessment does not supply such a policy or fabricated bounds.
`ShipAiAvoidZoneSearcher`'s four C++ zero initializers are interface defaults,
not recovered stores. Existing successful query fixtures supplied explicit
box words and do not prove this allocation prerequisite.

## Director identity and writers

`0080E160..0080E166` is exactly `MOV EAX,[ECX+738h]; RET`. Its receiver is the
**unit**, and the result is its command/weapon director. `00810F60` allocates
`250h` bytes at `00810F85`, calls `008366D0(unit)` at `00810FA0`, and stores the
result into `unit+738h` at `00810FA9`. `009F1160` also caches this pointer at
`brain+AB8h` (`009F11CF..009F11D5`); the four `009DA6E0` calls nevertheless
re-read through `nav+3FCh` (unit) rather than using that cached pointer.

The director flag has a real unconditional constructor value: `008366F4`
sets EBX=1, retained across calls, and `00836730` stores BL to `director+242h`.
The neighboring `+240h/+241h` stores also write one. `008362A0` associates
`+242h` with the literal `landCollisionAvoidance` at `00D09E64`; it passes a
copied boolean value to the sink and does not itself change the director.

| Writer or producer | Exact ownership and behavior |
| --- | --- |
| `00835A40..00835ABB` | Sender only. Builds kind `5Ah`, sub-kind 9, value from its byte argument; `00835AA5` routes through `director+34h` with message, channel 7, flags 0. RET 4. |
| `008A3B10..008A3CC4` | Lua entry registered as `NavigatorSetAvoidLandCollision` at table `00E0BC60`. Unit from argument 0 is retained in ESI; argument 1 uses `00B66250`. `008A3C60` loads `unit+738h`; `008A3C67` calls the sender. Broader Lua/error/parts side effects are not reconstructed here. |
| `00835640..0083568E` | Derived message receiver. For sub-kind 9, `00835676..0083567D` stores `message+24h != 0` to `director+242h`. RET 4; other sub-kinds follow their existing contract. |
| `007214C0..00721544` | State-message producer: director `+242h` is copied at `00721516/0072151F` to message `+46h`. |
| `007219C0..00721A38` | State-message apply: ECX is the message, stack argument is destination director. `007219F7/00721A09` copies message `+46h` into director `+242h`, preserving the byte. RET 4. |
| `008367F0..0083691C` | Mutable property access. `00836867` forms `director+242h`; `00836889` passes its address with bool type 3 and the same name to sink vtable `+10h`. The sink is an unresolved external writer contract, not the value-only dump. Caller `00820046` obtains the director from `unit+738h` at `00820027`. |
| `00720E50..00720E70` | Unreferenced direct three-byte setter, no current Ghidra function. Third stack byte is stored at `00720E68` to `director+242h`; RET 0Ch at `00720E6E` is three bytes. No name or definition added. |

The nearby getter `00720E40..00720E46` also has no Ghidra function; its last
instruction is the one-byte RET at `00720E46`. A byte-pattern scan helped find
these accessors, but offsets appearing in unrelated objects are not attributed
to directors. The scan is not an exhaustive proof against alias or bulk writes.

The sender's local effect is not unconditional. `0077C2A0` checks live session
existence/stage at `0077C2A3..0077C2BE`, mode and routing flags, then on its local
arm queues through `0076E520` at `0077C44D`. Its RET 0Ch confirms three route
arguments. This packet does not reconstruct delivery or make a sender into a
direct local store.

## Runtime synchronization and the next binding

The source snapshots in the report identify exactly what was inspected.
`GameUnitsHost::Impl::commands` owns a `GameCommandsHost`; that host's persistent
`directors` vector is initialized in `register_units`. `GameDirector` holds
slots, mode, stage, cruise fields and target hold, but no avoidance bytes and no
`WeaponDirectorState`. There is no runtime call to its constructor or derived
enable-message apply. AI auto-target paths borrow a temporary
`WeaponDirectorState{}` for allow-move; `ClearanceBinding` returns false for
director `+242h`. These are not a canonical mutable director flag.

Reuse the existing `WeaponDirectorState` in that director owner and expose its
live fields through a narrow Commands/Units interface. The compiled
`construct_director_008366d0` requires the actual unit endpoint, command-array
construction, endpoint class tests and `+38h` subobject service; their absence
must remain explicit if only the proven avoidance-store projection is bound.
Reconcile its cruise fields with the existing `GameDirector::cruise`, rather
than creating two authoritative copies. Session/property/state-message writers
must update the same record. The general session path remains recorded in the
runtime; its existing local command delivery does not implement kind `5Ah`.

The request uses **brain-relative historical C++ names**:

| Existing member | Native nav field | Meaning |
| --- | --- | --- |
| `avoidance.enable_3f4` | `+3ECh` | torpedo request |
| `avoidance.side_filter_3f8` | `+3F0h` | ship party filter |
| `avoidance.flag_3fc` | `+3F4h` | land request used by `009DA6E0` |

`Controller::avoidance` is already persistent. The compiled constructor helper
`ship_ai_avoidance_request_constructed_009e468b` supplies `{true,3,true}` and the
separate drive-bypass false; runtime construction currently copies only bypass
from `nav_block`, leaving `avoidance` at its C++ defaults `{false,-1,false}`.
The pre-pass tail `009F1B7B..009F1B9A` must run before state-specific writes on a
replan: land and torpedo true, bypass false, party filter from settings `+4h`.
Its compiled helper already exists, but the runtime pre-pass stops before this
tail. Settings `+4h` has a known producer (`0083BCD5=1`) and Lua override
`008D0852`; the current host has no represented mutable value. This is a missing
binding, not an unknown tuning key.

Verified state writers of the land request are stop (`009E15BB/009E15E4`),
cruise (`009E11EA/009E12FF/009E13D2`), land (`009E1C28/009E1EA2`), kamikaze
(`009E2235/009E22D8`), and attackmove engage (`009E25CC/009E2669`). The current
stop path writes `ctl_.avoidance`. The runtime cruise interface passes only
the control block/setter host and cannot update that request. Other listed
state paths remain partial or recorded. On non-replan frames the request
retains its value; resetting it every frame would change native scheduling.

Clearance currently copies `nav_block.plan_state_3f0` and `nav_block.flag_3f4`,
which are constructor snapshots. It should consume the live `avoidance` party
filter and `flag_3fc`; the row named `avoidance_enabled` currently reports the
torpedo member. New searcher gating should also use `flag_3fc`, and preserve
the four separate director queries in the recovered `009DA6E0` sequence.

The sector call `009EBDAA` in `009EB660` passes ECX=`nav+A3Ch`, the list at
searcher 0 `+18h`. The current callback returns false. Its eventual binding
must borrow that same selected-list owner, with native-backed zone/corner
lifetime and allocator/clear service, after the searcher refresh; it must not
invent a second empty list. The arc kernel itself belongs to the other worker.

Future file ownership should be narrow: Commands `.hpp/.cpp` for the existing
director owner and writer APIs; Units `.hpp/.cpp` for forwarding and the cruise
request handoff; Ship AI `.cpp` for constructor/pre-pass/request synchronization
and shared searcher/list consumers; Lua `.hpp/.cpp` only for actual mutable
settings and authored setter integration. Recheck those leases before edits.
No new core or duplicate state type is needed. Initial box storage, session and
property delivery, and unsupported state branches remain explicit dependencies.

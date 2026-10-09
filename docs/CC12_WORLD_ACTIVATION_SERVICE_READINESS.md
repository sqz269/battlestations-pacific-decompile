# Actual World expiry service: 00903610

`00903610` is the complete 93-byte / 42-instruction World expiry pass reached by
the zero-mode tail of `00874D00`. Its actual-storage Source refinement is held on
the direct child-destruction service `009035E0`, which is currently host-mediated
Source. This packet does not invent a callback or owner to replace that service.

The canonical address already has a counted
`release_expired_world_objects_00903610(WorldExpiryHost&)` reconstruction. This
read-only audit adds zero Original functions/bytes, Source, ABI or game-validation
credit. It changes only the requested document and JSON report.

## Evidence and bounded scope

- Published baseline: `bab59dd4aa832fbf9a5c26b4397ac198cdec5f90`.
- Worker baseline: `247d24a8b4bd5b7f80e3aeddd561181232a6cd24`, preserving pending
  activation-flush Source commit `e4997d8700cb759c59c9a19d5936d68679e25c04`.
- Verified live Ghidra target: `C:/Users/sqz269/bsp.gpr`, program
  `/battlestationspacific.exe`, `x86:LE:32:default`, image base `00400000`;
  live and snapshot counts were both 64,729.
- Metadata first established `00903610..0090366C`, 93 bytes, below the requested
  300-byte gate. The complete pseudocode, live listing and bytes were then read.
- All 93 live bytes equal the installed PE span. Body SHA-256:
  `469147a3ef98d0e543c4194ce2392ab735ab64e64a49058bf406a52e5befec59`.
  Installed PE SHA-256:
  `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
- The JSON embeds all bytes/instructions, nine branch destinations, both CALL
  sites, and canonical input pins. Only this native body was queried. Existing
  caller evidence and current Source dependencies were read without a descendant,
  caller-body, table or exception-funclet sweep.

The pinned shutdown-step audit records `00874DD1` as a tail JMP to this entry with
`ECX` holding that caller's freshly selected World, no explicit stack argument,
and the caller's return address. This audit independently establishes the target's
ECX/RET behavior; it does not re-query the caller's native body.

## Complete receiver and counter schedule

The entry saves `EBX`, retains the actual World in `EBX`, loads `[World+4]`, saves
`ESI`, reads the header's first entity, saves `EDI`, and clears `EDI` as the resume
anchor. Empty first returns after restoring `EDI`, `ESI`, `EBX`; a null header is
still dereferenced. No World vtable, header count, global or floating-point value
is read. There are no incoming EDX or explicit stack arguments; exit is plain RET.

At each entity, `00903620` reads the full DWORD `+6C`. The signed TEST/JLE at
`00903625` skips zero and every negative bit pattern without a store or anchor
change. Otherwise the native ADD increments modulo 2^32, CMP compares that result
signed against `3`, and `0090362D` stores the full result. The store preserves the
CMP flags used by `00903630`'s JL.

| Initial DWORD interpreted signed | Native result |
| --- | --- |
| Zero or negative | Skip unchanged; keep the previous anchor. |
| `1` | Store `2`; age and set anchor to this entity. |
| `2` through `0x7FFFFFFE` | Store incremented value; release. |
| `0x7FFFFFFF` | Store `0x80000000`; signed result is below `3`, so age and set anchor. |

The native wrap case is not a C++ signed `counter + 1` guarantee. Current projected
`entity_expiry_step_00903620` uses that expression; no overflow precondition or
test is invented here. The familiar `1 -> 2 -> 3` sequence describes normal
visits, not a general traversal or timing guarantee under arbitrary callbacks.

Only the age branch at `00903660` changes the anchor. Both skip and age then read
the current entity's `+38`. A skipped entity never becomes the resume point.

## Release calls and current storage

Here "child count" names the DWORD `+50` gate; its producers are not audited.

1. `00903632` compares the current entity's full DWORD `+50` with zero. Every
   nonzero pattern enters the child loop. `00903638` freshly loads child `+48`
   into `ECX` and calls `009035E0` without stack arguments. After return,
   `00903640` rereads the same entity's current `+50`; another iteration reloads
   `+48`. There is no cached count, saved child successor or null-child guard.
2. Once the count is zero, `00903646` loads the entity's current vptr and
   `00903648` loads its current slot zero. The code pushes DWORD `1`, puts the
   entity in `ECX`, and calls the actual target at `0090364E`. There is no caller
   stack adjustment, so a compatible target must consume that four-byte argument.
   The existing scalar-deleting-destructor label is an interpretation; no class
   implementation, typed return value or allocation domain is recovered here.
3. After that virtual call, this body never dereferences the released entity.
   If the retained anchor is nonnull, it reads the anchor's current `+38`.
   Otherwise it reloads the current `+4` header of the **same captured World**
   and that header's current first entity. It does not reuse the initial header
   and does not consult a newly published global World.

The late vptr/slot-zero load observes child-service effects. The post-release
anchor/head reload observes unlinking and any further callback mutation. The
anchor can precede skipped nodes, so those nodes can be revisited and their
current counters reevaluated. There is no saved-next iteration or index advance.

## Lifetime and failure contract

The captured World must survive any reached head reload. Each current entity
must remain valid through counter accesses, all child-service returns/count
reloads and its late virtual dispatch. A nonnull anchor must survive its
post-release `+38` read. The current virtual target must preserve compatible
stack and callee-saved register state, including World/current/anchor holders.

Child count/head mutations must make the intended child loop progress, and the
real retirement/unlink behavior must leave valid anchor/head continuations. No
production lifetime, unlink or allocation policy is supplied by this audit.
Although this function performs no released-entity read after its own virtual
call, other owners may impose additional requirements.

Invalid storage can fault. A nonzero child count with null head is forwarded to
the unresolved child service, not silently skipped. Cycles or callbacks that do
not make progress can loop. Earlier counter stores, child effects and retirement
effects remain on later failure. There is no local EH frame, rollback, catch,
new `noexcept`, guard or recovery path; native fault/unwind behavior is untested.

## Current Source and holds

`WorldExpiryHost` abstracts the head, successor, counter, child count/head and
virtual deletion. Its projected recursion and expiry algorithm are not an
actual-pointer service. `WorldExpiryBinding` uses `index + 1` pointer tokens,
vector counters and unit-slot order, always reports zero children, and logs
deletion. It then writes vector counter `-1` to avoid revisiting the retained
slot. That store/unlink substitute does not occur in the native body.

The existing actual chain-header type agrees with the head-at-offset-zero access
but provides neither this service nor its entity retirement graph. The qualified
66-byte `00922FD0` implementation marks state and calls slot `+84`; it does not
replace child destruction or this slot-zero retirement. At the assigned baseline,
the new `00903670` Source was awaiting Root emitted admission. Its later admission
would not remove this direct `009035E0` hold. These baseline statuses and hashes
remain historical pins if integration progresses during this audit.

The next bounded prerequisite is a separately leased actual-storage readiness
audit of `009035E0`, starting with metadata and a body-size gate. No Source packet
for `00903610` is recommended until that concrete direct service is established.
Actual current slot-zero targets, hierarchy mutation, captured-World/anchor
lifetimes, application wiring and complete teardown remain qualified external
obligations. No Source, CMake, ledger, GPR mutation, build, test or probe was run.

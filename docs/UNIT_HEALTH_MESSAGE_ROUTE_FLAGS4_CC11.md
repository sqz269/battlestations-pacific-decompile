# D2 health message, exact-flags4 router branch

`NativeUnitHealthMessageFlags4Calls` connects the reconstructed health setter
and D2 constructor to the ordinary exact-flags4 branch of `0077C2A0`. It is an
opt-in abstract adapter. `bind()` supplies the actual setter, whose constructor
call produces the borrowed D2 frame and whose route call always passes flags 4.
The final route override executes this branch against that same message.
Health notification/provider, entity-kind dispatch, actual field bindings and
complete peer transport remain required. No concrete runtime host is installed.

This is a named **fragment of parent `0077C2A0`**, not a complete generic router.
Its parent body is `0077C2A0..0077C46A` end-exclusive, ECX=receiver, stack
message/flags/out, RET0Ch. Other flags are outside this adapter's API. The
existing snapshot policy helpers in `session_message_dispatch.cpp` are not
called or promoted into actual bindings by this packet.

## Actual fields and services

| Native storage | Source obligation |
| --- | --- |
| `00E188A8` | One mutable game-pointer cell shared by setter, constructor and router |
| `00E0AF1C` | Actual volatile default-flags DWORD, read even though override 4 wins |
| `00E18DB7` | Actual volatile audit latch BYTE |
| game `+5D4` | Signed DWORD state, initial gate is at least 10 |
| game `+1FE4` | Actual mode cell returned by the inherited pure field accessor |
| game `+1EF0` | Actual embedded session address passed to the peer provider |
| receiver table `+5C` | Fresh table/entry dispatch for each reached entity-kind call |
| receiver `+528` | Borrowed signed DWORD player-slot cell |
| receiver `+174` | Borrowed sender WORD cell, reloaded for each peer |
| receiver `+2A8` | Borrowed actual sentinel-pointer cell, repeatedly reloaded |
| native list node `+0/+8` | Actual next link and peer payload; no reconstructed array |
| peer `+50` | Actual target-pointer cell returned by a pure field accessor |
| message `+0/+0C` | Current active D2 profile and actual predicate entry |
| message `+18` | Actual sender WORD destination |

The router constructor takes the same `NativeUnitHealthSetterGlobals` value
that it passes to the constructor adapter. Both copies retain the same
reference to the mutable pointer cell; neither copies a game-pointer value.
Game backing must remain readable at the reached offsets. Receiver bindings
are aliases to actual cells, not a native-layout receiver overlay or copied
host record. Binding/accessor methods do not observe field values early,
invoke callbacks, allocate, acquire ownership, or change FP state.

`send_message_to_nonlocal_peer_00770b50(session,target,message)` is a required
complete operation. Its provider owns the actual session+188h/+18Ch transport
selection, local-peer exclusion, secondary message query 29h and `00783DC0`
effects. The router does not substitute transport success, serialization
policy, a clone, allocator or factory registration. A provider must consume
the frame synchronously or create its own valid owned representation; it must
not retain or release the caller's borrowed frame.

## Native branch and observation order

1. Load the actual game pointer; null returns. Read signed game+5D4; less than
   10 returns before receiver/profile access.
2. Load the receiver's current table+5C entry and call kind 5 at `0077C2CC`.
   If true, read signed receiver+528. A nonnegative value permits a fresh
   table+5C kind-1Ch call at `0077C2E8`. If true, load the message's current
   profile+0Ch entry and call D3h at `0077C2FB`; its result is discarded.
3. Read the actual default-flags DWORD at `0077C302` even though the native
   CMOV selects nonzero override 4. Reload the game pointer at `0077C310`.
   The mode read at `0077C31D` must equal 1 or the branch returns. Preserve the
   second read at `0077C329` from that same captured game.
4. With no callback or concurrent mutation between those consecutive reads,
   the second mode is also 1. Flags 4 therefore reaches neither privilege
   query (`0077C344/36F`) nor relay rewrite. No new rejection guard is added.
5. Read the actual audit latch. When nonzero, reload the current D2 profile and
   invoke query 49h at `0077C392`. The admitted active D2 profile's executable
   predicate returns false; no payload-byte write is introduced. This is an
   actual profile call, not a hardcoded no-op predicate.
6. Preserve the fresh game/mode observation at `0077C39C/C3A2`, although flags
   4 lacks the host-send bit. Reload game/mode again at `0077C3B8/C3BE`; only
   mode 1 enters the peer walk. Mode is not checked again inside that walk.

The peer walk initially reads receiver+2A8 and captures that sentinel's next
node. Each iteration reloads the sentinel at `0077C3E2` for the end comparison,
then reloads it for the second iterator check at `0077C3F6`. The self-comparison
at `0077C3E0` proves only the `0077C3EB` violation call dead.

For each peer, load sender WORD+174, load node payload+8, store sender WORD
into message+18, read the actual peer+50 target, reload the game-pointer cell,
and call the complete peer provider with game+1EF0 at `0077C423`. After that
call, reload the sentinel for the `0077C428` check and only then read the
current node's next pointer at `0077C432`. Thus a returning send can replace
the game, sender, sentinel and valid links, and the remaining walk observes
those replacements. All reached storage must remain alive.

The conditional `00BF6713` operations at `0077C3FB` and `0077C42D` remain
required services. No no-return annotation, exception or recovery behavior is
invented. Their original invalid-iterator/fault semantics are not proved by
the ordinary retained-storage domain or by the focused fixture.

Flags 4 has no local bit on every admitted path. `0077C44D` local enqueue,
factory cloning and out-pointer clearing are therefore unreachable. Even a
nonnull out pointer is left untouched. The adapter's public override has the
explicit flags-4 precondition supplied by `bind()->00877B90`; calling it with
another flag word does not request a fallback generic router.

## Verification and boundaries

The ignored `local/cc11_health_message_route_flags4_check.cmd` compiled the
actual D2, base, cursor and health-setter translation units with MSVC Win32,
`/O2 /Gy /W4 /WX /fp:strict`, and an embedded `asInvoker` manifest. Its focused
probe passed **66 checks with zero failures** across six direct guard/empty-list
cases and one connected setter/constructor/router case.

The direct cases cover null game, negative signed state, a kind callback
switching to a client game before the exact-4 gate, a callback changing the
signed player slot, kind-1Ch false, and a valid active D2 profile replacement.
They preserve message bytes and the caller's out slot as appropriate.

The connected case uses actual borrowed list nodes. The first send changes
the game to another valid session in mode 2, changes the sender WORD, replaces
the sentinel and replaces the current node's next link. The second send uses
the new session/sender/next, skips the old next node, and terminates at the new
sentinel without an extra mode gate. The fixture invokes the actual D2 writer
synchronously and verifies the sender/payload bits. It does not implement a
real network transport or retain/delete the message. The constructor-selected
borrowed owner remains intact through both sends.

Generated assembly confirms the dead volatile default-flags load, both mode
reads on one captured pointer, later fresh game/mode reads, current-profile
indirect calls, signed guards, WORD sender store, fresh per-peer session load,
and post-send sentinel/next observations. Probe accessor instrumentation
records call arguments; it does not measure runtime read counts of plain
cells. Previous accepted constructor/profile bodies remain unchanged.

The claim is source reconstruction of this ordinary branch, locally
build-tested and fixture-tested. Active translated D2 profiles and all reached
backing must stay live. Concurrent mutation between ordinary observations,
other profile classes, freed/current-node corruption and malformed cycles are
outside the domain. The full router, original register ABI, invalid-iterator
faults/private EH, actual transport/allocator identity and gameplay remain
unproved. Primary integration and full-repository checks are separate.

Primary integration: ac4197a647b28e631d95f3d295a99200814a02d8; actual main sources independently recompiled for manifested focused probes, PASS. Full MSVC Win32 Release and all three existing CTests passed. Executable SHA256 97b412a3aaf2d3550b95b9c23b7e29558d40b07599a1d17df5f7b82f0ff6a585. Original ABI, actual runtime binding and game validation remain unclaimed. Build receipt: J:\PROG\battlestations-pacific-decompile\local\cc11_flags4_map_integrated_build.log.

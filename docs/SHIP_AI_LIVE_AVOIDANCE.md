# Live ship avoidance ownership

This process adapter keeps the existing director, stored settings and persistent
request as their respective owners. It does not recreate the native composite
object or message transport.

`GameMissionLuaHost` establishes settings+4 from the literal loader store at
0083BCD5. The actual registered `NavigatorSetAvoidAllShipCollision` callback
then applies 008D0852 using bare Lua Boolean conversion. Consumers read that
stored value; they do not reread a Lua key. The process uses its existing shared
interpreter, whereas the native settings loader creates a private interpreter.
Native loader error/partial-store ordering is not claimed.

Each ship controller initializes its persistent request through the existing
009E468B helper and runs the 009F1B7B reset only on replan ticks, before the
state step. The live party and land values feed clearance; constructor snapshots
are no longer read there. Existing stop-state writes retain this same request.

Each controller also owns `ShipAiSearchStorage`, whose 2268h backing preserves
its allocator's untouched bytes. The three 20h records expose live cache/list
views. The 009DA6E0 overload borrows these prefixes through pointers rather than
assuming a packed 18h stride. Existing packed-array callers use the same body.
Storage moves and cleanup are described in [SHIP_AI_SEARCH_STORAGE.md](SHIP_AI_SEARCH_STORAGE.md).

Sector crossing, arc clipping and static clearance borrow searcher zero's same
selected list. The 009D57E0 wrapper tests its enabled byte before forwarding to
00415970. Selected lists are retired before geometry replacement and before
their allocator disappears. The director getters read the canonical unit
director introduced in [GAME_DIRECTOR_AVOIDANCE.md](GAME_DIRECTOR_AVOIDANCE.md).

## Current verification and remaining connections

The working Win32 build and both existing tests passed. An ignored probe links
the actual game-host objects and passes 47 checks covering registered Lua
truthiness, error replay, stored settings reaching the prepass, actual director
message receipt, four live director reads and three cache-disable transitions.
The storage and geometry workers retain separate original-byte evidence.

Mission query refresh at 009DA6E0 and the cruise request overload still require
the live role-owner connection. The native bot at unit+740 has an attached
entity owner at bot+50; that is not a formation-group object. Role slots and
participant AI-held bytes cannot be inferred from Party or selected-unit state.
Until that connection is supplied, the chain query is explicitly recorded and
the constructed selected list remains empty. No active mission avoidance or
gameplay validation is claimed at this checkpoint. Five stored tuning inputs
and the remaining state/request writers are also separate work.

See `reports/ship_ai_live_avoidance.json` for the checkpoint evidence paths.

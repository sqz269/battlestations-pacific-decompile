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

MissionFrame registers controllers before loading world avoidance geometry.
That load retains existing storage owners, clears their borrowed selected lists
and invalidates cache keys before replacing geometry. This is process lifetime
management, not a recovered native loader store. The first combined run at
3db73f6e exposed and preserved an access violation caused by incorrectly
destroying those owners during geometry load. The corrected 87f9a9e0 run passes.

## Current verification and remaining connections

The pre-cruise combined Win32 build and both existing tests passed. An ignored probe links
the actual game-host objects and passes 56 checks covering registered Lua
truthiness, error replay, stored settings reaching the prepass, actual director
message receipt, four live director reads and three cache-disable transitions.
It also checks persistent tuning capture, later Lua mutation and retention after
a protected failed reload. This is the process loader's atomic projection; it
does not claim the native private loader's partial-store error ordering.
The storage and geometry workers retain separate original-byte evidence.

Mission query refresh at 009DA6E0 now borrows the controller's actual cache/list
views and world geometry. Its inputs come from the existing hull, look-ahead and
class projections. The five stored tuning fields are captured during the
represented settings load and feed sector and clearance consumers.

The cruise request overload now consumes the canonical unit role getter and
the borrowed session participant owner. The native bot at unit+740 has an attached
entity owner at bot+50; that is not a formation-group object. Role slots and
participant AI-held bytes cannot be inferred from Party or selected-unit state.
Role1=8 skips its participant lookup; the helm arm skips the player byte and
role0, and the player arm skips role0. Required unavailable reads preserve the
input output and record a partial state step. The constructor-owned role table
remains unassigned until the actual role-assignment receiver is connected.
The borrowed participant object must outlive the ship AI host. Its mission
publication is tracked in the separate session-owner packet.

The corrected combined Win32 build and both existing tests pass at 87f9a9e0.
The 120-frame mission exits normally with 18,557 finite trajectory rows and
241 unchanged Airfield2 samples. The live search executes 2,400 queries,
10 refills and 12 disable clears. All 1,080 cruise-owner reads are available;
the constructor-owned role8 path does not need a participant lookup. The
session pool is not yet published by MissionFrame at this checkpoint.

This verifies execution of the process avoidance path. It does not establish
original-game avoidance, visual or gameplay parity. The remaining state/request
writers and obstacle refresh producer are separate work; recovered point
predicates are not yet bound to unproduced geometry. Exact executable, source
identity and the failed predecessor are recorded in ORCH6_RECONSTRUCTION_IJK.md.

See `reports/ship_ai_live_avoidance.json` for the checkpoint evidence paths.

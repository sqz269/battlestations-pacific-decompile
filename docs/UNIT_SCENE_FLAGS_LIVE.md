# Canonical unit scene flags in the process host

Packet `orch6_unit_scene_flags_q` binds existing flag consumers to the stable
`GameUnitSlot` which the runtime world registry already carries as its direct
payload. `UnitInstanceState.active` and its historically named `simulate` member
remain the owners of native `+5C` and `+5D`. The same slot now retains `+5E`,
`+5F`, and `+60`. `SceneNodeFlags` is a copied view of the first four flag cells;
it is never a second persistent state object. This is a process binding, not a
binary-layout reconstruction of the complete native unit.

`GameUnitsHost::unit_identity(index)` returns that stable payload identity.
`unit_scene_node_flags` reads by index; `read_scene_node_flags` and
`store_scene_node_flags` resolve a borrowed identity before reading or writing.
The latter commits `active`, `torn_down`, `destroyed`, and `removed` to their
canonical cells and refreshes the public row's active snapshot. Separate
`unit_pending_destroy_0060` and `store_pending_destroy_0060` methods cover `+60`
without altering the shared `SceneNodeFlags` type. Invalid, foreign, or
unresolved-creator identities return false and preserve read outputs. A slot's
availability bit records process provenance; it is not an extra native flag.
Slots are individually owned by `unique_ptr`, so growing the slot vector does
not invalidate identities. Their lifetime ends with the Units host.

`PendingQueueBinding` in `game_hosts_ready.cpp` now uses the attached Units
host for its read/store hooks and rejects an unavailable identity. Its pending
list population, render/controller ownership, observer callbacks, and virtual
destruction services remain unresolved. No queue entries or new deletion path
are enabled here. The existing zero counts mean this incomplete binding stays
inactive; they do not prove that native queues are empty. In particular, the
store API does not claim to execute a complete Kill, Destroy, or Remove operation.

`LocalPlayerUnitListsBinding::unit_filter_flags` now reads all four predicate
cells instead of supplying false for three of them. `unit_alive_and_visible`
likewise consumes active, torn-down, pending-destroy, and destroyed. Both use
the native `+5C != 0 && +5D == 0 && +60 == 0 && +5E == 0` predicate. `+5F` is
retained but is not an additional filter condition. The world binding rejects
unavailable flags explicitly. Its older substitute source for local-player
walk 0 is unchanged and remains documented in `LOCAL_PLAYER_UNIT_LISTS.md` /
the world host header; this packet does not establish participant list ownership.

## Constructor and actual initialization producer

`00925CE0..00925EF3` clears the five cells in the order `+60`, `+5F`, `+5C`,
`+5E`, `+5D` at `00925E08`, `00925E0B`, `00925E0E`, `00925E11`, `00925E14`.
The complete EBX-use filter shows `XOR EBX,EBX` at `00925CFD`, no intervening
write to EBX, and BL as every store's source. ESI is the original ECX receiver.
This is a native `__thiscall` constructor, returning the receiver in EAX with a
plain RET; there are no stack arguments to these flag stores. The established
unit chain reaches it through `0095CC90 -> 0087B670 -> 0077EED0 -> 00928630 ->
00925CE0`. A ship adds the existing `0081ED40` layer. These functions have other
fields and services which this packet does not reconstruct.

The post-initialization active value has a separate producer. The complete
31-byte body `00923840..0092385E` first inherits `+58` (Race) only when it is
negative and a parent exists. Every normal path then executes `MOV AL,1` at
`00923853`, stores AL to `+5C` at `00923855` and `+BD` at `00923858`, and returns.
It has an ECX receiver, no stack arguments, plain RET, and no calls. The runtime
projects only its unconditional `+5C` store; it does not claim the parent/Race
or `+BD` operations. The five-byte `009277E0..009277E4` thunk jumps to this body.

`0077F0E0` retains its ECX unit in EDI, performs its existing global-list work,
reloads ECX from EDI at `0077F11B`, then calls the thunk at `0077F11F`. Property
kind tests start afterward at `0077F124`. The chain into this call is
`00955424 -> 0087BCC0`, `0087BCDC -> 0077F0E0`. At each call the entry receiver
is either unchanged or explicitly restored; the saved listings preserve those
register writes. `00925F20` invokes the unit's primary `+A0` slot at `00926110`,
with ECX loaded from the pending initialization node's direct payload `+8` at
`00926105`. Thus active=true in the represented initialized scene units is not
the native constructor's initial value and does not depend on a guessed
StartEnabled default.

The 21 creator identities and their primary table installations are reused
from `reports/unit_world_registration.json`. This packet reads the actual PE
words at each table `+A0`; `local/scene_flags_activation_map.json` preserves all
21 rows. Every supported leaf reaches the common initialization producer:

| Creator family | Primary +A0 target | Direct base call |
| --- | --- | --- |
| Destroyer, Cruiser, Cargo, BattleShip | `00822C20` | `00822CDB -> 00955420` |
| LandingShip | `0074BEC0` | `0074BEE7 -> 00822C20` |
| Submarine | `00853630` | `0085364C -> 00822C20` |
| TorpedoBoat | `00857C80` | `00857C83 -> 00822C20` |
| MotherShip | `007593D0` | `007593EC -> 00822C20` |
| Eight supported plane creators | `007D5D20` | `007D5DAC -> 00955420` |
| AirField | `006D3C10` | `006D3C2D -> 00955420` |
| Shipyard | `00849A30` | `00849A51 -> 00955420` |
| LandFort | `007482B0` | `007482D5 -> 00955420` |
| CommandBuilding | `006F2780` | `006F2783 -> 007482B0` |
| LandVehicle | `0074D800` | `0074D81E -> 00955420` |

The main ship and plane initializers make earlier virtual calls but restore
ECX to their saved unit before the common call. None of the inspected normal
paths skips that base call. This is activation-chain coverage, not a claim that
every leaf initializer or its external services is reconstructed. LandVehicle's
previously undefined body was formally defined by the integrator at exactly
`0074D800..0074DA0C`; its final instruction is one-byte RET and the next three
bytes are padding. The worker only read the definition and saved listing.

## Other writers and the supported input domain

The represented `GameSceneEntityRecord` comes from the kind-1 scene property
bag installed by `00922E20`; see `game_hosts_scene_contents.cpp`'s queue binding.
It does not represent the type-2 descriptor or type-3 mission-Lua `_entity`
payloads. Their flag writers must not be silently applied to kind 1:

* After the `+A0` pass, `00925F20` handles `container+4 == 2`, reads the payload's
  `+3C` StartEnabled byte, and stores `+5C=1` at `009261D0` or `+5C=0` at
  `00926207`, subject to the existing destroyed/active guards. It also makes
  virtual and child calls. This is a separate type-2 override.
* In `00927050..0092739D`, type-3 `_entity.active=false` calls `00922F80` at
  `0092729F`; `_entity.deadMeat` writes `+5D=1` at `009272EF` and `+70=1` at
  `009272F3`. Its kind-1 arm has no deadMeat write. Therefore `simulate` is a
  misleading historical C++ identifier, not permission to invent a simulation
  enable/disable meaning for native `+5D`.
* Existing `00922F30` and `00922F80` store active at `00922F4B` and `00922F9B`
  under destroyed/active gates. Both have an ECX receiver and RET 4 for the
  recursive/notification argument. Their shared C++ helpers cover only stores;
  native virtual `+68/+6C` callbacks and recursive child walks are broader.
* Kill `00922FD0` writes `+5E=1`, `+5D=1`, `+5C=0` at `00922FDE..00922FE4` and
  has further child/virtual `+84` work. Remove `009263C0` has the guarded store
  order `+5D`, `+5E`, `+5F`, `+5C` at `009263F4..009263FD`, in addition to
  render/controller checks and observer/virtual services. The flush body
  `009273A0..009275D1` includes the same removal stores at `009274CE..009274DA`.
* Destroy `00926C80..00926D8A` (ECX receiver, RET 4) checks/sets `+60` at
  `00926CBF/00926CD5` under its existing lock, propagates cause/children, and
  appends to the pending destroy list. Its `+74` delivery hook
  `00926390..009263B1` sets `+5D/+60` at `0092639B/0092639E` before its helper
  and virtual `+7C` tail. Cancel `00925A00..00925A89` clears `+5E/+60/+5D` at
  `00925A58/00925A70/00925A73` with its hierarchy and queue conditions. A bare
  store to retained `+60` does not claim any of these compound operations.

The ignored displacement scan was only a candidate generator; overlapping
decodes, bulk stores, aliases and other object layouts preclude an exhaustive
writer claim. For example `0092F31B` follows eight floating-point stores into a
different object's `+40..+5C` block and tail-jumps to `0092E5B0`; it does not
establish a unit flag writer merely because its destination is also `+60`.
Unit `+6B8` / DummyObjectID remains separate; no global live -1 assumption or
property-delivery implementation is introduced here.

## Verification and limits

The retained fixture `local/scene_flags_probe.cpp` compiles the production
pending binding unchanged into its translation unit and links the actual
Win32 game host objects plus `bsp_core`, `bsp_lua511`, and `bsp_zlib121`. It
provides explicit Lua descriptor and scene records through the real public host
APIs. It runs the original 15-byte constructor flag-store fragment with the
proved ESI/BL inputs and the complete 31-byte common activation body with a
positive Race input. Surrounding constructors, base-list services, and the
optional activation parent arm are outside that numerical/byte fixture.

The focused integration case checks retained reads and writes through the
production queue hooks, all four world-filter cells, +5F retention without an
extra filter, existing kill/remove store helpers, independent unit ownership,
public row synchronization, unchanged failed-read outputs, foreign identity
rejection, and stable identities after adding another unit. Queue delivery and
destruction callbacks are not exercised. The fixture does not launch the game,
load an installed mission, or change gameplay/camera state.

The corrected fixture passed 249 checks. It retains one Units owner throughout
and creates a fresh world consumer for each state observation: the existing
local-list gate runs once for each world owner. The first fixture incorrectly
expected a repeated call to rebuild those lists and failed that assertion. Its
source, executable and logs are retained separately in
`local/scene_flags_fixture_first_gate_attempt/`. No production gate change was
made to satisfy the test. Win32 and both existing CTests passed; the call gate
checked 24 direct rows with zero failures, with one virtual dispatch recorded
separately from the checked rows.

Final build/test, call-gate results and exact hashes are recorded in
`reports/unit_scene_flags_live.json`; `local/scene_flags_manifest.json` retains
the original bytes, scripts, source inputs, object/library identities and logs.
No native function, Ghidra name, or prototype is reconstructed/changed by this
host-only packet. No no-Ghidra-function entries remain after the integrator's
LandVehicle definition. Existing unsupported service and runtime input domains
above remain open.

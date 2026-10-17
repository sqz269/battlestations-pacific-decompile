# CC13 fix3 / fix4: hidden objectives and the objective-set membership

Two switches, both landed OFF (2026-10-17, granted by the cc9 gameplay lead). With both OFF the host
behaves exactly as before; the only addition is one summary log line.

| Switch | Where | Packet |
|---|---|---|
| `kObjectiveSetHiddenMembershipBound` | `include/bsp/game_hosts_ai.hpp`, next to `kObjectiveKindBound` | cc13-fix3 |
| `kReconSurfaceSetBranchBound` | `src/game_hosts_gunnery.cpp`, next to the recon switches | cc13-fix4 |

## fix3: the set's +18h tree (`kObjectiveSetHiddenMembershipBound`)

**Image.** An objective set (`[00E188A8]+21A4h+slot*4`) keeps two things: the objectives (list at
set+24h, head +28h), each with its own unit list at objective+20h, and a set-wide unit tree at set+18h
(size at +20h). Every reader of "is this unit in the objective set" reads the tree:
- 008DDF90 (`BSP_SzurkeNyil_ContainsUnit`): `set+20h == 0` answers 0 (008DDF93); an `IsKindOf(5)`
  entity is looked up as itself, an `IsKindOf(18h)` one through `+3D0h`; 008DDF00 is the tree find.
  Its callers include 00922C80 (set arm), 00A0F810, 00A2C450, 00A2DEF0 and the HUD markers.
- 00A26210 (escort think) walks `[[00E188A8]+own*4+21A4h]+18h` directly.

Writers of the tree:
- **Insert**, 008DF2B0 only (008DE6A0's other caller is the network sync 008DF000). After the name walk
  finds the objective (008DF2FE `JE 008DF3DC` ends with no add when it does not), the observer pair and
  the list push (008DF358 008DEE50, no duplicate test), 008DF36A `CMP [obj+18h],2` / 008DF36E `JNZ` /
  008DF370 `CMP [obj+1Ch],1` / 008DF374 `JNZ 008DF3DC` skip the insert for a **hidden** objective
  (kind 2 and state not 1). Otherwise 008DF3D7 `CALL 008DE6A0` with `ECX = set+18h` and the unit as key
  (the marker handle from 006DE3D0 is an empty stub).
- **Erase**, 008DF400 (reached from 008DFC00, so from `Objectives_RemoveUnit` and from 008DFE50 on a
  status change). It walks every objective: a same-named one (length then `_stricmp`, 008DF452-008DF48A)
  has the unit erased from its list (008DF49E 008DC3E0); any other with `+1Ch == 0` (008DF4B2), not
  hidden (008DF4C5 / 008DF4CB) and listing the unit (008DF4E3 008DC4A0) sets a hold flag. With no hold
  (008DF4FD), 008DF521 unregisters the observer, **008DF532 008DEB40 erases the unit from set+18h**, and
  008DF581 drops a marker.
- Nothing re-inserts when a hidden objective's state changes later. 008DFE50 returns at once for a
  hidden objective (008DFE6E), so its listed units stay listed and stay out of the tree.

**Host.** `GameObjectiveSets` now keeps `members[slot]`, maintained always:
- `add_unit` inserts when the objective already existed and is not hidden (`kind == 2 && state != 1`
  skips; counted in `hidden_member_skips`). The insert runs even when the objective already listed the
  unit. An objective that `add_objective` creates inside `add_unit` gets no insert (the image's walk
  would have found nothing; VERIFY13 O1).
- `remove_unit` and the drop path of `set_status` call `erase_member_unless_held`, which applies
  008DF4A5-008DF532's hold rule (holds counted in `member_holds`). `remove_unit` runs it whether or not a
  list held the unit, as 008DF4FD does.
- `units_in_slot` returns `members[slot]` while the switch is ON, otherwise the old union of the
  objectives' lists. Every caller of `units_in_slot` models a tree reader, so all of them change together:
  `game_hosts_ai.cpp` (00A0F810 weight, 00A26210 escort walk, 00A2DEF0 defend, 00A2C450 world set),
  `game_hosts_hud.cpp` (008DDF90 grey arrow), `game_hosts_ship_ai.cpp` and
  `game_hosts_script_orders.cpp` (00922C80 set arm), and fix4's gunnery reader.
- Summary line: `summary mission objective set membership bound=%d members=%zu hidden_skips=%llu
  holds=%llu`.

**Uncertainty and simplifications.**
- The tree is a pointer-keyed tree in the image; the host keeps insertion order. Only 00A26210's walk is
  order sensitive, and it already carried that label.
- The host still lists a unit in the objective's own list when the objective was created by `add_unit`
  (VERIFY13 O1, unchanged); only the tree follows the image there.
- `remove_unit` with an empty name is a host wildcard; for the hold rule an empty name matches only
  empty-named objectives, as 00435C40 would.
- 008DFC00 expands kinds 18h / 1Ah into members and 008DF9B0 does so on the add side; the host's units
  arrive already expanded, as before.
- The image's per-player slot (`[game+18ECh]`) stays the host's labelled slot 0 at the 00922C80 readers.

## fix4: 00922C80's set arm in the recon sensor pass (`kReconSurfaceSetBranchBound`)

**Image.** 00922DC0 is `MOV DL,1; JMP 00922C80`. 00922C80 (fastcall, ECX entity, DL allow_far) returns 0
for a null entity or `+5Dh != 0` (00922C8F), for `IsKindOf(0Fh)` / `IsKindOf(18h)`; accepts the ship
family (06h, with the submarine depth test), 45h, 46h, 1Ch; then 00922D3F-00922D5B asks 008DDF90 over
`[00E188A8]+21A4h+[+18ECh]*4` and returns 1 when the set holds the entity (`JNZ 00922D0E`), and only
otherwise runs the tail (`TEST BL,BL`, the dummy-target height and the LandFort `FakedType` test).

**Host.** `GunneryReconSensorPassHost::unit_is_surface_target` (00922DC0 for 008048A0 / 00806840) used to
take the tail at once with the comment "the set branch 008DDF90 is not built in this process", which was
stale: the sets exist and the ship-AI and chooser hosts already ask them. With the switch ON it asks
`units_in_slot(0)` (labelled slot 0, as in the ship-AI host) for an `IsKindOf(5)` entity and answers
`in_set || tail`. The gates before the arm are `entity_is_surface_target_00922c80`'s, unchanged. The
comment is corrected unconditionally; behaviour moves only under the switch.

## What a pair run should show

- fix3 ON vs OFF: on a mission with a hidden objective holding units (for example the RadioTowers of
  `us_osumi`, USNOS), `hidden_skips` is non-zero and those units leave every set reader: the surface-set
  hits in the ship-AI and chooser summaries drop, the chooser's `surface set branch: in_set=` lines for
  them read 0, so planes stop choosing RadioTowers as dive / surface targets through that arm; the AI
  objective weight hits and HUD grey-arrow tests drop for them. On missions without hidden objectives
  (JM08's LST and cargo lists are primary objectives) the run should be identical apart from the new
  summary line.
- fix4 ON vs OFF (fix3 ON): recon surface classification of land units listed in visible objectives of
  slot 0 becomes true; it moves the recon contact lists only for such units. With fix3 OFF, fix4 also
  counts units of hidden objectives, which is the pre-fix3 membership.

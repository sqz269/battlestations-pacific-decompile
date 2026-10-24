# October 24 host-tail identifier audit

This packet audits the **54 baseline unknown rows** assigned from
`local/output/oct24_wire_before.json` at baseline `515837c10`. Source inspection was in the
worker checkout after `aa2bf32a5`. All host C++ files remain read-only, including manually
leased files and cc9 gameplay hosts. No parser, host behavior, switch, Ghidra state, or game
process was changed.

## Result

| Source | Baseline rows | Real body mappings | Metadata-only | Reserved skip |
|---|---:|---:|---:|---:|
| `src/game_hosts_ai.cpp` | 4 | 4 | 0 | 0 |
| `src/game_hosts_commands.cpp` | 1 | 1 | 0 | 0 |
| `src/game_hosts_hud.cpp` | 2 | 2 | 0 | 0 |
| `src/game_hosts_init_tail.cpp` | 1 | 0 | 0 | 1 |
| `src/game_hosts_lua.cpp` | 20 | 20 | 0 | 0 |
| `src/game_hosts_scene_contents.cpp` | 7 | 7 | 0 | 0 |
| `src/game_hosts_script_orders.cpp` | 19 | 14 | 1 | 4 |
| **Total** | **54** | **48** | **1** | **5** |

`config/wire_identifiers_host_tail.json` contains 48 schema-v1 records. Every identifier
names an executable source body. Twelve records deliberately map to a verified folded body
whose suffix belongs to another native address, and only those records have
`shared_implementation:true`. Per-record evidence states the exact executable branch or
operation and its restrictions. Registration/name/address tables alone never establish a
mapping. The Lua binding trampoline and `GameScriptOrdersHost::dispatch` are used only where
an explicit executable branch applies the named semantic operation.

These are source-level identities for existing host projections, not 48 newly reconstructed
native functions. Their original register ABIs, source ownership, gate states and substitution
limits remain. Shared identifiers do not prove that each native address is installed at runtime.

## Correction: EndScene has no game implementation

`008b01b0` previously had status
`existing_host_substitution_labelled_verified_against_listing`. Its executable `EndScene`
branch in `GameScriptOrdersHost::dispatch` explicitly says RECORD: it increments
`end_scene_calls_`, stores the first diagnostic time, and logs
`Game::end_scene_prompt_or_unload` as unimplemented. It neither opens native restart prompt
`00531b00` nor calls scene unload `004d7970`.

The record is now `metadata_only_no_function_body_verified_oct24`, with `previous_status`
and appended evidence. All original fields and evidence are retained. The dispatcher remains
a real body for its implemented branches; it is not used to give this record completion credit.

## Two mapped-record qualifications

- `00923b80`: the original record said coverage `complete`, but the actual
  `set_script_entity_party_00928f50` / `set_unit_party_00928f50` methods accept only a party
  argument, write that party, and update Lua identity mirrors. They do not implement a
  general supplied race-argument write, recursive child processing, or `00696350` notification.
  Status is now `host_implementation_partial` and coverage
  `partial_host_party_store_and_lua_mirror_only`; `previous_status` and `previous_coverage`
  preserve the old values. The original RET0Ch ABI, notes and false ABI/game flags remain.
- `0089b840`: the historical `existing_host_implementation_defect_found` evidence describes
  a missing FABS. The current `GetCapturePercentage` branch executes
  `if (kLuaCapturePercentageFabsBound) fraction = std::fabs(fraction)` at the documented
  `0089b966` operation, and `include/bsp/game_hosts_script_orders.hpp` sets that switch
  **true**. Status is refreshed to
  `existing_host_substitution_labelled_verified_against_listing`; `previous_status` and
  the entire original defect evidence remain, with an appended statement that the old
  signed-fraction finding is superseded by the current source. Non-building fallback0 and
  existing host substitutions remain. This is static source/previous-listing evidence,
  not a new runtime or native differential result.

## Reserved ownership: five segment-40 rows

`docs/CC13_SEG40B.md` explicitly identifies segment40 as `00728a90..00735ec0` and segment58
as `008e3de0..008ee5f0`. The cc13 reservation applies even when its lease is absent. These
five assigned rows are **excluded from this manifest and from final ledger edits**:

| Address | Read-only finding for cc13 |
|---|---|
| `00733bb0` | Narrative start fields are folded into `run_narrative_update_00735100`: active/fade/display/state/alpha/text/callback/ref ownership. HUD text display remains unimplemented. |
| `00733d50` | Countdown callback path releases retained Lua refs in `run_countdown_update_00735100`; native vector/holder free/null-slot ABI is not reproduced. |
| `00733ff0` | `dispatch` Countdown branch executes countdown state/ref initialization while HUD begin remains unimplemented. |
| `00734ce0` | `narrative_clear_00734fa0` releases queued refs and clears the queue, with native list allocator/layout and HUD hide qualified. |
| `00735450` | `GameHardwareProbe::localized_message` discards formatting arguments, logs unimplemented and returns an empty string. `CC13_SEG40B.md` independently identifies this stub and missing locale table. It needs owner-approved metadata correction or implementation; its original ledger record is unchanged here. |

No segment67, segment40b or segment58b address has a final metadata/source mutation in this
packet. The packet's address lease was narrowed to the other49 rows after checking the
reserved segment range.

## Exhaustive mapped address evidence

The manifest carries the full branch-level qualification. The table below lists every mapped
address and actual body; source abbreviation is the file basename. A different native suffix
is an intentional folded implementation, marked by the manifest flag described above.

| Address | Source | Executable body identifier(s) |
|---|---|---|
| `00644a20` | `game_hosts_hud.cpp` | `run_root_observer_notice` |
| `006bf310` | `game_hosts_script_orders.cpp` | `player_air_ops_launch` |
| `006c6630` | `game_hosts_lua.cpp` | `run_get_property_0088bf80` |
| `006c7e20` | `game_hosts_lua.cpp` | `run_set_air_base_slot_count_008963e0` |
| `006d0e60` | `game_hosts_lua.cpp` | `run_get_property_0088bf80` |
| `006d3c10` | `game_hosts_scene_contents.cpp` | `retain_airfield_runway` |
| `006f7f40` | `game_hosts_commands.cpp` | `command_requires_target` |
| `006fd8b0` | `game_hosts_script_orders.cpp` | `publish_unit_deaths_00929800` |
| `006ff170` | `game_hosts_lua.cpp` | `read_loadout_text` |
| `00779820` | `game_hosts_ai.cpp` | `tick_request_join_formation` |
| `007f3820` | `game_hosts_scene_contents.cpp` | `queue_plane_squadron_wing_007f4580` |
| `00894100` | `game_hosts_script_orders.cpp` | `dispatch` |
| `00895570` | `game_hosts_lua.cpp` | `run_land_convoy_native` |
| `008956e0` | `game_hosts_lua.cpp` | `run_land_convoy_native` |
| `00895850` | `game_hosts_lua.cpp` | `run_land_convoy_native` |
| `0089a190` | `game_hosts_script_orders.cpp` | `dispatch` |
| `0089b840` | `game_hosts_script_orders.cpp` | `dispatch` |
| `0089cc50` | `game_hosts_script_orders.cpp` | `dispatch` |
| `008a3e70` | `game_hosts_script_orders.cpp` | `dispatch` |
| `008a7e60` | `game_hosts_script_orders.cpp` | `dispatch` |
| `008a9f90` | `game_hosts_script_orders.cpp` | `dispatch` |
| `008aadf0` | `game_hosts_script_orders.cpp` | `dispatch` |
| `008aaf30` | `game_hosts_lua.cpp` | `binding_trampoline` |
| `008ab070` | `game_hosts_lua.cpp` | `push_resolved_entity` |
| `008ab260` | `game_hosts_lua.cpp` | `binding_trampoline` |
| `008aca30` | `game_hosts_lua.cpp` | `mission_checkpoint_binding` |
| `008acb70` | `game_hosts_lua.cpp` | `mission_checkpoint_binding` |
| `008b16e0` | `game_hosts_script_orders.cpp` | `dispatch` |
| `008b19a0` | `game_hosts_script_orders.cpp` | `dispatch` |
| `008b2320` | `game_hosts_lua.cpp` | `run_simplified_multiplier` |
| `008b24a0` | `game_hosts_lua.cpp` | `run_simplified_multiplier` |
| `008b25f0` | `game_hosts_lua.cpp` | `run_simplified_multiplier` |
| `008b2770` | `game_hosts_lua.cpp` | `run_simplified_multiplier` |
| `008bd900` | `game_hosts_lua.cpp` | `binding_trampoline` |
| `008d0740` | `game_hosts_lua.cpp` | `binding_trampoline` |
| `00923b80` | `game_hosts_script_orders.cpp` | `set_script_entity_party_00928f50`, `set_unit_party_00928f50` |
| `00927c50` | `game_hosts_hud.cpp` | `selectable_inputs_00645060` |
| `0092bd00` | `game_hosts_script_orders.cpp` | `unit_parts_land_avoidance_disabled` |
| `0093bd80` | `game_hosts_lua.cpp` | `run_set_failure_0088eff0` |
| `00968710` | `game_hosts_lua.cpp` | `dispatch_exit_zone_listeners_00982120` |
| `009ffd60` | `game_hosts_ai.cpp` | `slot_team` |
| `00a0fc50` | `game_hosts_ai.cpp` | `command_pass_interval` |
| `00a10040` | `game_hosts_ai.cpp` | `destroy_group` |
| `00ada420` | `game_hosts_scene_contents.cpp` | `create_scene_world_object` |
| `00ada890` | `game_hosts_scene_contents.cpp` | `cell_height_00adb3a0` |
| `00ade500` | `game_hosts_scene_contents.cpp` | `create_scene_world_object` |
| `00adfd70` | `game_hosts_scene_contents.cpp` | `load_terrain_height_field_00adda60` |
| `00adfeb0` | `game_hosts_scene_contents.cpp` | `load_terrain_height_field_00adda60` |

## Material scope limits retained in the mappings

- Lua depth/class handling, entity identity, deck lookup and fused squadron handling are
  explicit host substitutions. GetProperty implements the four deck keys and predecessor
  class-reader order, while missing decks/class selection keep the existing departures.
- Countdown/Cancel have executable state and Lua-ref operations; their HUD calls remain
  unimplemented. They are qualified partial handlers rather than table-only rows.
- Ship-death mapping covers only the generic death/callback tail in
  `publish_unit_deaths_00929800`, retaining unverified ship teardown, scoring, `LastBanto`
  and frame-order scope. Carried-kamikaze mapping covers the executable Lua loadout field
  reader, retaining the unverified full BombClass/raw-class reader scope.
- Scene model-box accumulation uses the model BoundingBox substitute, rather than a native
  recursive terrain-node box algorithm. Heightfield read/load has executable U16 parsing;
  F32/U8 are rejected. Split-index behavior is folded into cell sampling. Squadron naming
  is the initial spawn projection, preserving the authored fused wing0 name, rather than
  general runtime rename/flag/mode behavior.
- HUD role query is the value0 call-site projection. Observer delivery is at interface update
  from host death records; native observer scheduling/raw root storage remains qualified.
- AI team lookup and command/group observers retain their existing host-record and target-arm
  limits. No native observer allocation-failure ABI is introduced.

## Validation

All54 assigned rows have exactly one outcome:48 mapped,1 metadata-only,5 explicit reserved
skips. All48 identifiers matched actual C++ definitions using the integrator's current
`DEF_RE`; twelve cross-suffix records are explicitly marked and verified. No duplicate
addresses appeared in the integrator's existing manifests during validation. Strict ledger
comparison verifies exactly three changed records, no added/deleted record keys, all original
fields retained except the documented status/evidence and single coverage correction. The
reserved `00730000` shard is unchanged. Index refresh and diff checks pass.

No C++ changes require a build here. No tests were added, and no build, native ABI fixture,
OFF/ON pair, or game run was performed. The integrator reruns combined board/readiness
reports after integration; no global current KPI is inferred from this isolated worker tree.

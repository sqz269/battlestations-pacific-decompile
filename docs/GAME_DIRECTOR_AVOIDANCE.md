# Live director avoidance and cruise request handoff

Packet `orch6_live_director_request_i` gives the existing per-unit command
director owner its three avoidance bytes and carries a caller-owned AI request
through the cruise step. It changes only the Commands/Units hosts and these
evidence files. It does not reconstruct a second director or the session router.

`GameDirector::avoidance` is the sole stored projection of director `+240h`
torpedo, `+241h` ship and `+242h` land. Construction copies just the three
documented `WeaponDirectorState` defaults. Native `008366F4` sets EBX=1 and
`00836724/0083672A/00836730` write BL unconditionally. The full
`construct_director_008366d0` endpoint, command-array and subobject callbacks
are not invoked or substituted. Existing cruise, target hold, command slots
and stage state remain in their current owner; no duplicate copies are added.

`GameCommandsHost::register_units` creates one director for each registered
unit. Re-registering creates new owners with constructor values; reading or
changing commands does not reset the flags. `GameUnitsHost` forwards access
to that same `GameCommandsHost::Impl::directors` vector.

| API on Commands and Units | Behavior |
| --- | --- |
| `director_avoidance(index, out)` | Reads all three live values. Returns false for an absent owner and leaves `out` unchanged. |
| `apply_director_avoidance_message_00835640(index, message)` | Delivery-side projection for kind `5Ah`, sub-kinds 7, 8 and 9. Stores `message.value != 0` to the selected flag and returns true. Missing owners, other kinds and unsupported sub-kinds return false without mutation. |

The apply API uses the existing `DirectorCommandMessage` and
`DirectorCommandSubKind` contracts. It directly projects the three host-free
stores in the already reconstructed `00835640`; it does not build a dummy
`WeaponDirectorHost` to call unrelated permission/target behavior.
The kind check is an explicit supported-delivery-domain check, not an extra
condition attributed to the native derived switch.

`00835A40` remains a sender: its `00835AA5` call routes a message through the
actual endpoint/session with channel 7 and flags 0. No call to that sender or
local-delivery shortcut is added here. Current Lua/script-order hosts do not
implement `NavigatorSetAvoidLandCollision` or kind `5Ah` delivery. Property
restore (`008367F0`) and state-snapshot apply (`007219C0`) likewise have no live
host delivery; this packet does not add disconnected invented restore inputs.
Those future writers must update these same three owned fields when their
actual delivery is implemented. Other derived-message sub-kinds remain for
their proper owner rather than being silently accepted.

## Cruise request publication

The new Units overload is:

```cpp
bool run_cruise_state_step_009e1170(
    std::size_t index, bsp::ShipAiControlBlock& blk,
    bsp::ShipAiSetterHost& setters, bsp::ShipAiAvoidanceRequest& request,
    const bsp::ShipAiCruiseAvoidanceInputs& avoidance_inputs);
```

It passes the **same request reference** through Commands and uses the compiled
`ship_ai_cruise_step_request_009e11d6` helper. Units supplies its real
`unit+184h` player-controlled value. The other inputs must come from their own
producers: group slot at `[[unit+740h]+50h]+1B0h`, unit slot at `unit+1ACh`, and
the corresponding `00927F10` predicates. A mission Party value is not a slot
assignment. Those slot owners are not yet represented by the runtime; the
integrator must establish availability before calling this overload.

The request keeps the existing historical member identities:

| Member | Nav offset | Role |
| --- | --- | --- |
| `enable_3f4` | `+3ECh` | torpedo request |
| `side_filter_3f8` | `+3F0h` | ship party filter |
| `flag_3fc` | `+3F4h` | land request |
| `blk.early_out_3f5` | `+3F5h` | separate drive bypass |

On `CruiseRule`, publication occurs after the existing speed/commanded-speed
host reads and immediately before the first desired-heading or steering
setter. It runs once, so callbacks from the setter observe the published
request. The native request stores are `009E12EB..009E12FF`; their own-slot
predicate call is `009E12E4`. The current compiled drive projection collects
cruise fields earlier than all the original getter sites, so this is not a
claim of instruction-level equivalence for every native read or FP exception.

The player and helm arms publish the compiled helper's exact request changes
before returning false: the remaining drive behavior of those arms stays
explicitly partial. Only the helm arm sets bypass true; the other two arms
preserve it. A unit without the cruise command leaves the request untouched.
The legacy overload remains drive-only for existing callers; the root owns
the AI call-site switch, constructor/pre-pass synchronization and settings
binding. This packet does not guess missing slot inputs to enable that switch.

## Verification scope

The standalone Win32 build and both existing tests passed. One
ignored focused probe drives actual `GameCommandsHost` owners, its real command
issue path, the new delivered-message API and the compiled cruise request
helper. It checks flag independence, absent-owner behavior, nonzero payload
normalization, refused messages, owner replacement, all three request arms,
bypass retention and publication before steering callbacks: 27 checks passed.
It links an archive of the actual successful executable build objects (excluding
the entry point) and the existing core/Lua/zlib libraries. Per-object hashes and
the reproducible archive command are retained with the ignored probe.

This is a host-state probe, not an original-byte differential test or mission
delivery run. Units forwarding is compiled in the full executable. No tracked
tests, new Ghidra functions, name changes or ledger records are added. Exact
source, probe, build-log and native-listing hashes are in the ignored artifact
manifest referenced by `reports/game_director_avoidance.json`.

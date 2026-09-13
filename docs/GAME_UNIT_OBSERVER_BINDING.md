# Actual unit observer binding

Addresses consumed: `00925CFF`, `00925D13`, `00925D44`, `00928662`,
`0092589D -> 00695870`, `009258AC -> 00695760`.

Packet T connects the prefix projections from
[NATIVE_UNIT_OBSERVER_ENDPOINT.md](NATIVE_UNIT_OBSERVER_ENDPOINT.md) to the existing
`GameUnitsHost` owners. This is source integration and partial constructor/teardown
coverage. It does not reconstruct a complete native scene/game/unit constructor,
native unit allocation, notification handler, or complete entity destructor.

`GameUnitSlot` embeds one `NativeUnitObserverPrefixStorage`. The slot remains the
canonical identity returned by `unit_identity()` and stored in world-list nodes.
There is no second unit, endpoint sidecar, or semantic-byte cast. The embedded
storage itself has the exact native observed `+0h` and callback `+10h` layout;
those offsets do not describe the encompassing semantic `GameUnitSlot` layout.

| Source stage | Recovered producer consumed | Coverage |
| --- | --- | --- |
| Stable slot construction | `00925CFF`, `00925D13` initialize the separate base tables and array triples, then `00925D44` publishes scene tables | Partial stores only; remaining `00925CE0` constructor work is outside this binding |
| Actual descriptor creator resolution | `00928662` game tables, then the existing 21-creator leaf table map | Partial table stores only; full `00928630` and leaf constructors are outside this binding |
| Slot teardown | `0092589D` invokes callback destruction, then `009258AC` invokes observed destruction | Partial `00925780`: preceding `00925780..00925896` non-observer teardown is outside this binding |

The consumed leaf map is selected from the same `VehicleClass.Type` descriptor
creator as motion/world registration. A bound host rejects unresolved creators
before publishing a slot. An unbound source fixture can retain its existing
unresolved unit behavior, but cannot acquire an observer alias. These table words
are native profile identities, not executable process vtables.

The public source API is:

```cpp
mission.bind_observer_runtime(singletons.observers());
// Forwarded to its current/future GameMissionFrameHost, then GameUnitsHost.
auto alias = units.observer_alias(units.unit_identity(index));
const void* canonical = alias
    ? units.unit_identity_from_observer(&alias->prefixes.observed_00) : nullptr;
```

Integration requires the actual `GameMenuHost` constructor to call the mission
setter immediately after allocating its mission, using its existing
`GameSingletonHost&`. That one-line application wiring is owned by the primary
integrator and is not present in this worker commit. The runtime must already
have its actual dispatch owner. Rebinding the same runtime is idempotent;
switching runtimes, binding after drain, and binding existing unresolved units
throw `std::logic_error`. No new observer runtime is allocated by these setters.

Aliases borrow the same live slot and expire when its teardown starts. Both
endpoint addresses reverse-map to that canonical unit; foreign/null addresses
return null. The host withdraws each alias before its teardown can reenter source
lookups. Runtime liveness is tested through the real owner publication, because
native shutdown deliberately retains the dangling dispatch alias. Previously
returned references must never be retained past unit/frame destruction.

Native `00925780` captures `ESI=ECX` at `0092579A` and `EDI=ESI+10h` at
`0092579D`. `00925896` passes EDI in ECX to `00695870` at `0092589D`, then
`009258A2` passes ESI in ECX to `00695760` at `009258AC`. Both native operations
take their endpoint in ECX and return with bare `RET`; the new host methods use
ordinary source C++ interfaces and are not binary replacements. Ghidra body
ownership for both sites is `00925780..009258C2`. Pseudocode and the assembly
register writes were rechecked read-only against configured `bsp.gpr`, program
`/battlestationspacific.exe`.

The frame explicitly releases borrowers before canonical slots: HUD detachment,
Lua `script_orders=nullptr`, step-subsystem unit detachment, unit-to-ship-AI
detachment, then destruction of ship AI, script orders, world host, and units.
The same helper runs when `load_scene_contents` replaces a frame's units and when
the frame is destroyed. The existing mission load destroys its previous frame
before replacing Lua. The menu declares HUD before mission, so HUD remains alive
during mission/frame cleanup. Application startup already deletes the menu before
raw singleton drain. None of these changes resets mission state or rearranges the
load inventory. Repeated-load borrower cleanup is source-reviewed, not a repeated
installed-mission runtime claim.

Units destroy their gunnery borrower, then each actual callback prefix through
`destroy_callback_owner_00695870`, followed by the observed prefix through
`destroy_observed_owner_00695760`, while all slots remain allocated. The borrowed
runtime must outlive this work and its dispatch owner must still be live. A
violated destruction contract terminates instead of dereferencing the retained
post-drain dispatch alias. Unsupported native edge-deleting profiles remain
unavailable in the existing runtime; this packet adds no callback or event shim.

Validation used `scripts/build.ps1` with MSVC Win32 Release and passed both
existing CTests (`reconstructed_math`, `native_math_differential`). The focused
local fixture creates actual `GameUnitsHost` units from one explicit Lua
`VehicleClass[7]` Destroyer row, checks canonical world-list identity, exact leaf
tables, adjustment and reverse lookup, and exercises live/unbound/foreign/
unresolved/drained binding guards. It registers real `CF7E64` native edges in both
directions across two actual source unit hosts, destroys one, and checks that
both surviving endpoint counts are zero before the second host and raw singleton
manager are destroyed. No events or fake callbacks are supplied. This fixture
does not validate the rest of gunnery, gameplay, or installed scene construction.

Retained artifacts are under this worktree's `local/`:
`unit_observer_binding_probe.cpp`, `run_unit_observer_binding_probe.ps1`, the exact
`.rsp`/`.cmd`, probe object/executable, extracted embedded `asInvoker` manifest,
compile/run/host logs, build log, native teardown listing, and
`unit_observer_binding_probe_manifest.json`. The manifest records SHA-256 and
paths for all 38 linked application objects, three project libraries, all named
system libraries, compiler/linker, and fixture artifacts. The compile log records
MSVC 19.51.36244.0; the executable explicitly uses `/link /MANIFEST:EMBED`.
`reports/game_unit_observer_binding.json` records the exact inputs/results.

Worker status: reconstructed source binding, build-tested, focused source
fixture-tested. Application wiring and combined 120-frame validation remain with
the primary integrator. No original-game execution, complete native ABI, or
gameplay validation is claimed.

# The stationary registration creator, 004E5B00, and why USN22 does not load

Addresses: 004E5B00, 004F0BE0, 004F1460, 004F0930, 00964790, 00425850.

## 1. Retraction

`docs/TORPEDO_MISSION_SURVEY.md` section 7 said that Ormoc Bay fails to load because its 189
stationary units register through `SceneContents::class_registration_creator` (`004E5B00`), which
the host leaves unimplemented, and it named that native as one of two that would open a mission.
**Both halves of that are wrong**, and this document retracts them.

`004E5B00` is not unimplemented in any meaningful sense. It is a single `RET` between two runs of
`INT3` padding:

```
004e5afc: ret            <- the previous function ends
004e5afd: int3
004e5afe: int3
004e5aff: int3
004e5b00: ret            <- the whole of 004E5B00
004e5b01: int3
004e5b02: int3
...
```

It is a no-op stub, shared as the registration creator by eight scene classes in
`src/scene_entity_factory.cpp`: `NavPoint`, `MovieCamPos`, `MovieCamLookat`, `LandingPoint`,
`Stationary`, `Wreck`, `WaterMine` and `Landscape`. None of them has anything to register. The
host's 192 `unimplemented` records against it are records of a routine that does nothing, and
implementing it would change nothing at all.

The second error follows from the first. The count of 192 was read as the number of stationary
classes that failed to register, and it is simply the number of times a no-op was recorded.

## 2. What actually stops the load

The refusal comes from the units host, not the scene pass:

```
startup failed: unit observer creator projection is unavailable
```

`src/game_hosts_units.cpp` raises it when
`publish_unit_leaf_observer_tables_for_creator(prefix, creator)` finds no row for the unit's
creator. The creator is not a scene-class property. It is
`VehicleClassDescriptorRow::allocate_instance`, the descriptor's vtable slot +28h, and the
descriptor is chosen by the class's `VehicleClass.Type` literal through the case-insensitive chain
at `00964790` (`009648EC`..`00964EEC`, comparing with `00425850`).

**A stationary class has no `Type`.** This installation's
`scripts/datatables/autoload/stationaryclasses.lua` (13 Jul 2024, the bulk install date, not
locally modified) defines 220 rows under the table name `StationaryClass`, not `VehicleClass`, and
the string `Type` does not occur in the file once. `Stat_kate` is representative:

```lua
StationaryClass["Stat_kate"] =
{
    ["Mesh"] = Platform("models/vehicles/planes/stat_kate.mmod", ...),
    ["HPBlack"] = 50,
    ["Armor"] = 1,
    ["ExplosionEfx"] = 158,
}
```

So the chain takes no branch, the factory returns a null descriptor, `unit_motion_dispatch` returns
its default with creator `0`, and the observer lookup for creator `0` fails. That is the throw.

USN22 reaches it because Ormoc Bay is a land battle whose scene rows are typed from
`StationaryTypes` while their scene class is `LandFort`:

```
scene type LandFort LandFortClasses:Hangar_new1=586      party=Japanese(1) x1
scene type LandFort LandFortClasses:cargo_wreck_a=215    party=Neutral(2)  x1
scene type LandFort LandFortClasses:coastal_gun_us=460   party=Japanese(1) x7
scene type LandFort LandFortClasses:light_aa_jap_sandbag=630 party=Japanese(1) x2
scene type LandFort StationaryTypes:Stat_kate=595        party=Japanese(1) x3
scene type LandFort StationaryTypes:Stat_zero=602        party=Japanese(1) x7
```

The run registered four units and stopped: `AirField`, `Light AA 1`, `Light AA 2` and `Hangar1`,
all of them creators that are in the table (`006D3110` and `00747000`). The next unit is the first
whose type comes from `StationaryTypes`.

USN04 never meets this because it has no stationary rows at all: its scene types are
`DestroyerGen`, `MotherShipGen`, `PlaneSquadronGen`, `TBoatGen` and `Path`.

## 3. The one gap in the factory that is real

`src/vehicle_class.cpp` holds 22 `Type` literals and
`src/native_unit_observer_endpoint.cpp` holds 21 observer rows. The literal with no observer row
and no motion-dispatch row is `DummyTargetVehicle` (`MDummyTarget`). That is a separate, smaller
gap from the stationary one and no mission in this survey exercised it.

## 4. What this packet changed

Only the diagnosis. The throw now names the unit and the creator and separates the two failures
that used to read identically:

* **creator `0`** means no vehicle-class descriptor was found at all, which is the stationary case
  above.
* **a non-zero creator** means the class resolved but is absent from the observer rows.

The message is also written to the log before it is thrown, so the reading survives in the run log
rather than only in the launcher's last line. That distinction is what a run costs otherwise: the
old message sent this thread's survey to the wrong native.

## Uncertainty

* **How the original creates a stationary unit is not established.** It plainly does, so there is
  a path that does not go through the `VehicleClass` descriptor chain; `004F0BE0`, the `Stationary`
  class's own instantiate address, is where to start, and it was not read here.
* Whether `Wreck`, `WaterMine`, `Landscape` and `NavPoint` take that same path, or are never units
  at all, is unread. `Landscape` (`004F1460`) is recorded 215 times in the USN22 run and skipped,
  so it at least never reaches unit creation.
* Whether any mission needs `DummyTargetVehicle`.

## Host methods

None added. One existing throw in `src/game_hosts_units.cpp` now carries the unit name, the creator
and which of the two failures it is.

## Corrections

This document is itself a correction; see section 1. It retracts two claims this thread published
in `docs/TORPEDO_MISSION_SURVEY.md` section 7 and section 8, and the matching rows of
`reports/torpedo_usn04.json`. Both of those files are corrected in the same commit.

The cause of the error is worth recording: the 192 calls to a routine recorded as `UNIMPLEMENTED`
were taken as 192 failures, without checking what the routine is. A one-byte `RET` reads in a
summary exactly like a native body that has not been written.

## no_ghidra_function

`004E5B00` has no Ghidra function; it is one byte inside the range Ghidra assigns to `FUN_004E5980`,
separated from it by three `INT3`. It was read from the image with `disasm-raw`.

## Validation

**Blocked.** No run is possible: session 1, where the agents run, is disconnected, so no window,
D3D device or FMOD output can be created. See `docs/MISSION_LUA_GETPROPERTY.md`. The build is clean
and both ctest suites pass. The run that would confirm section 2 is one call:

```
./tools/run_game.ps1 -Log local\usn22_stationary.log -WaitSeconds 2400 -- --frames 3200 `
    --press-start-frame 30 --menu-select USN22 --mission-frames 3000 --mission-frame-seconds 0.05
```

It should still fail, and the new message should name the unit after `Hangar1` with `creator=00000000`
and the stationary reason. That is the confirmation, not a fix.

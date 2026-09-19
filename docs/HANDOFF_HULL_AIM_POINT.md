# Handoff: the hull aim point, and the wiring bug the measurement caught

Packet `cc8_hull_aim_point`, worker `cc8-hull-aim`, branch `agent/cc8-hull-aim`,
base `ec14870c3`. Analysis commit `de315b718`.

Read `docs/HULL_AIM_POINT.md` first; this file is only what is left to do and
why.

## (a) Settled, do not re-derive

All of section 1-5 of `docs/HULL_AIM_POINT.md` is proved from the listing and
does not need revisiting:

* Slot `+100h` has exactly two implementations. `0042D810` returns the origin
  (dozens of vtables, including the plane unit instance `00D05F20`); `00816650`
  samples a tapered hull box and is carried by exactly nine vtables, each
  confirmed by a literal store in its own constructor.
* `00816650` draws a point in the authored hull box of `[unit+538h]`: along-hull
  `z` by half `Length` (`+A0h`), across-hull `x` by half `Width` (`+A4h`)
  tapered by `00419010(0.6,1.0,1.0,0.1,|rz|)`, `y` by a quarter of `Height`
  (`+A8h`), one sided. Spread seeds `(0.9, 0.5, 0.9)`.
* Slot `+104h` is `0042BB20` = `MOV AL,1; RET 0Ch` on **all nine** of those
  vtables, takes the point by value, always answers true, and `009FAEA5` skips
  the re-pick on true. **The offset is drawn once**, by the dirty byte
  `009FB272` sets. The 1.5-2.5 s timer fires forever and changes nothing.
* `009FA260` adds a bias at `sub+34h/+38h/+3Ch` after the pick.
* The packet's premise is **retracted**: the offset is `±0.45 x Length`, an
  order of magnitude larger than the census miss, and its mean is the hull
  centre. The sampler scatters the aim, it does not bias it.
* `00816650` was already bound as `bsp::ship_lead_point_00816650`; this packet
  reuses it and fixed a swapped `width`/`length` naming defect in
  `LeadAimHullExtents` (names only, arithmetic identical).

## (b) The bug the measurement caught, and the fix

`hull_aim_target_samples_hull` in `src/game_hosts_units.cpp` tested

```cpp
target.class_id == bsp::kVehicleClassIsShipKind   // 0x06
```

`kVehicleClassIsShipKind` is one of `00964790`'s *kind tests*
(`include/bsp/vehicle_class.hpp:176`), **not** the leaf `class_id` this host
stores on the slot. The USN04 log settles it:

```
unit hull input unit=York-class02        type_id=263 kind=10 length=180 width=16
unit hull input unit=Lexington-class01   type_id=1   kind=9  length=250 width=30
unit hull input unit=Northampton-class01 type_id=294 kind=10 length=180 width=16
unit hull input unit=Fletcher-class01    type_id=364 kind=7  length=110 width=10
```

`kind` there is `host.units.unit_class_id(index)`
(`src/game_hosts_ship_ai.cpp:5176`), the same field my predicate reads. Ships
are 7, 9 and 10 — never 6. **So the predicate was false for every ship and the
binding was inert: the first after-run measured nothing.**

The correct test is the ship family, which this repo already spells out once, in
`has_ship_navigation_class` (`src/game_hosts_ship_ai.cpp:73-89`):
`Destroyer, Submarine, MotherShip, Cruiser, Cargo, LandingShip, BattleShip,
TorpedoBoat`. That is also the right family on the evidence side: the nine
hull-sampling vtables are the unit-instance / vehicle-base / submarine classes,
and planes, land vehicles and land forts are exactly the families that carry
`0042D810`.

**Next step, concretely:** replace the predicate with that switch (it cannot
call `has_ship_navigation_class`, which is in another translation unit's
anonymous namespace — either promote it to a shared header or restate the
switch over `bsp::VehicleClassKind`), rebuild, and re-run the pair. Nothing
else in the binding needs to change.

## (c) What the two runs actually showed

Both on this tree, USN04, identical parameters, base `ec14870c3`:

* `local\hullaim_after_usn04.log` — build with the feed, but the feed was inert
  for the reason above.
* `local\hullaim_before_usn04.log` — `src/game_hosts_units.cpp` reverted to
  `de315b718~1`.

Because the predicate never fired, these two are expected to be **identical**,
and that is the control, not the result. The after-run's 20 census rows fall in
two groups:

| group | rows | miss vs target at impact | along course | across |
|---|---|---|---|---|
| `tgt=York-class02`, moving 16.7 m/s | 8 | 67.4 - 122.6 m | -45.9 to -117.5 | -28.0 to -72.6 |
| `tgt=-`, stationary | 12 | 5.5 - 29.2 m | +0.4 to +16.1 | -5.5 to -26.0 |

The stationary rows reproduce the handoff's quoted 11.3 / 25.5 m exactly, which
is the tell that nothing moved. The moving-carrier rows are large for the
already-known reason — **there is no lead term anywhere in this object** — and
not because of anything this packet changed.

One thing to carry into the re-measurement: the moving-target `across` values
(-28 to -72 m) are already far larger than `0.45 x Width` = `0.45 x 16` = 7.2 m
for a 180 m / 16 m hull. So the across component is dominated by something other
than the hull offset, and a corrected run must not be read as if the whole
`across` column were the hull draw. Decompose against the before run row by row.

## (d) Predictions for the corrected run

From `docs/HULL_AIM_POINT.md` section 7, now quantified with the real hulls
(`length=180 width=16` for the cruisers, `250 x 30` for Lexington):

1. Along-hull offset up to `±0.45 x 180` = **±81 m** on the cruisers, `±112 m`
   on Lexington. This should show up mostly in the `along course` column.
2. Across-hull offset at most `±0.45 x 16` = **±7.2 m** on the cruisers, less
   near bow and stern because of the taper. Small.
3. Vertical offset `0 .. 0.125 x Height`, upward only.
4. Within one squadron the `along course` values should **spread**, because each
   aircraft draws its own point once and keeps it. That spread is the packet's
   real claim and is what to report.
5. Every draw is inside the hull by construction, so a round that was a hull hit
   before should stay one.

## (e) State

* Commit `de315b718` (analysis, binding, doc, ledger records) is sound and worth
  keeping as is; only the one predicate in `src/game_hosts_units.cpp` is wrong.
* `src/game_hosts_units.cpp` must be restored to `de315b718` before editing —
  the before-run left it reverted to `de315b718~1`.
* Ledger records added: `00816650`, `0042D810`, `0042BB20` (new names) and an
  appended correction on `009FADA0` that preserves the `cc8_dive_aim` text.
* Lease `cc8_hull_aim_point`: 9 addresses, 4 files.

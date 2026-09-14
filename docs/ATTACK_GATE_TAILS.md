# The three unread attack gates, and the fields they read

Addresses: `0047B850`, `00604A50`, `00827F70`, `00828EC0`, `007CD930`, `007D673E`, `00749210`,
`0074C630`, `00831840`, `00746B90`, `009611CB`, `005471B0`. Constants `00D099AC`, `00CFF824`,
`00CFFD08`, `00D09D34`.

This closes Part 5 of `docs/ATTACK_CAPABILITY_INPUTS.md`. **All three gates are answerable from
authored data**; so are `plane+C24h` and `vehicleClass+178h`, and the stamp-count discrepancy is
resolved. Nothing here needs runtime state beyond what that document already named.

Everything is **exported / read**: the Ghidra listing, `.rdata` bytes, and byte scans over the
shipped PE's `.text`. No C++ was written, compiled or run.

## Gate 1 — `0047B850`, the `dogfight` and `strafe` gate

`__thiscall(plane)`, `RET 0`, no stack arguments. In full:

```
0047b858  PUSH 10h / CALL [vtable+5Ch]   ; IsKindOf(10h)  MPlaneBomber
0047b85e  JNZ  0047b873                  ;   -> return 1
0047b865  PUSH 16h / CALL [vtable+5Ch]   ; IsKindOf(16h)  MLargeReconPlane
0047b86d  JNZ  0047b873
0047b86f  XOR EAX,EAX / RET              ; else 0
```

```
bool Gate_0047B850(plane) { return IsKindOf(10h) || IsKindOf(16h); }
```

Both `dogfight` and `strafe` require it to answer **0**: neither command applies to a level bomber
or a large recon plane. **Answerable from the class id alone** — no field reads at all.

`dogfight` also tests `IsKindOf(16h)` directly at `007EEB1A`, so there the gate adds only
`!IsKindOf(10h)`; for `strafe` it supplies both halves.

## Gate 2 — `00604A50`, the `kamikaze` gate

`__thiscall(plane)`, `RET 0`:

```
bool Gate_00604A50(plane) { return IsKindOf(17h) && plane->+C24h == 0; }
```

`IsKindOf(17h)` is `MPlaneKamikaze`, which the `kamikaze` arm has already established at `007EEB53`
before it reaches this call, so the effective added condition is **`+C24h` clear**.

### `plane+C24h` is authored, and it is `PilotFires`

A scan of `.text` for every `[reg+0C24h]` memory operand finds **two write sites, both in
`FUN_007CD930`** (`007CD939` writes 0, `007CD981` writes 1); everything else is a read or compare.
`ghidra xrefs` gives `FUN_007CD930` exactly one caller: `007D673E` in `BSP_Plane_ReadPropertyBag`.
So the byte is computed **once at load**, not maintained at runtime.

```
plane->+C24h = 0;
for (child = plane->+48h; child; child = child->+44h) {     // 007CD934, 007CD988
    if (!child->IsKindOf(20h)) continue;                    // 007CD94A, the gun base class
    vec = plane->+538h + 94h;                               // 007CD954/007CD960
    i   = child->+38Ch;                                     // 007CD95A
    if (i >= vec.count /* +98h */) std::vector::at(005471B0) grows or throws;
    if (vec[i]->+0Ch != 0) plane->+C24h = 1;                // 007CD97B, 007CD981
}
```

Every element of that chain is already recovered elsewhere: `unit -> [+538h]` is the unit's class
descriptor and `descriptor+94h`/`+98h` its platform pointer array and count, indexed by the gun's
`+38Ch` (`docs/GUN_AIMING.md:91,125`); and the platform record's `+0Ch` byte is the Lua key
**`Platforms[].PilotFires`**, read at `009611CB` (`docs/VEHICLE_CLASS_FIELDS.md`, Platforms table).

So `plane+C24h` is **"any of this plane's gun platforms has `PilotFires` set"** — a pilot-operated
gun. `docs/GAME_AWARD_TRACKERS.md:174` and `docs/HUD_ROOT_UNIT_ROWS.md:104` both read it as the
machine-gun flag; that is consistent with the authored meaning but is not the authored name, and
the reading was not previously traced to a producer. It is now.

**Answerable from authored data**: the plane's platform list and each platform's `PilotFires`.

## Gate 3 — `00828EC0`, the `torpedo` gate

Both this and `00827F70` take **`target+538h`**, the vehicle class descriptor, and dispatch through
**`vtable[18h]`** — a third class-test slot, distinct from the entity's `+5Ch` and the projectile
descriptor's `+8h`.

```
bool Gate_00827F70(vehClass)            // BSP_VehicleClass_IsTorpedoBoatOrSmallLandingShip
{
    if (vt18(0Eh)) return true;                            // MTorpedoBoat
    if (!vt18(0Ch)) return false;                          // MLandingShip
    return vehClass->+808h == 0;                           // !BigLandingShip
}

bool Gate_00828EC0(vehClass)
{
    if (!Gate_00827F70(vehClass)) return false;            // 00828EC0..00828EEB, identical code
    return vehClass->+500h >= 8.938947f;                   // 00D099AC
}
```

The float compare is SSE, read from the listing: `00828EED MOVSS XMM0,[ESI+500h]` /
`00828EF5 COMISS XMM0,[00D099AC]` / `00828EFC JC reject`, so the gate accepts on
`+500h >= K` and rejects on `<` or unordered. `00D099AC` = `ED 05 0F 41` = **8.938947f**.

The `torpedo` arm requires `00828EC0` to answer **0** (`007EEAA9` then the shared tail at
`007EEA0D`, `JNZ` to reject). So a torpedo cannot be ordered against a torpedo boat or a small
landing ship that is fast enough.

### Both fields are authored

| field | key | producer | how |
| --- | --- | --- | --- |
| `+808h` | `BigLandingShip` (`00CFFD08`) | `BSP_LandingShipClass_ReadLuaFields` `0074C630`, store at `0074C687` | `PUSH 0CFFD08h` at `0074C65E`, the row lookup, then `00B662F0(ref, 0)` -> `MOV byte ptr [ESI+808h],AL` |
| `+500h` | `MaxSpeed` (`00D09D34`) | `BSP_ShipClass_ReadLuaFields` `00831840`, store at `0083191A` | `PUSH 0D09D34h` at `008318F4`, row lookup `00B67800`, value `00B66270`, then `D9 9F 00 05 00 00` = `FSTP dword ptr [EDI+500h]` |

`+808h` sits just past the ship reader's `+138h..+804h` range, which is why it is a **leaf** field:
the same offset is written by `BSP_CargoClass_ReadLuaFields` (`006EB515`, `006EB560`),
`BSP_CruiserClass_ReadLuaFields` (`006FB59C`) and `BSP_BattleshipClass_ReadLuaFields` (`006E013C`)
for their own purposes. Only the `MLandingShip` reader's meaning is in play here, because
`00827F70` reads it only after `vt18(0Ch)` has succeeded.

This also proves the ledger name `BSP_VehicleClass_IsTorpedoBoatOrSmallLandingShip` from the
authored data: "small" is literally `not BigLandingShip`.

## The `vtable[18h]` class test is the same id space

Proved the same way as `docs/ORDNANCE_KIND_IDENTITY.md` proved `vtable[8]`, and **enumerated over
all 22 vehicle class descriptors** in `docs/VEHICLE_CLASS_DESCRIPTORS.md`, not sampled. Each
vtable's slot `+18h` was read from `.rdata` and its body decoded from `.text`.

```
Type                 kind  vt[18h]   chain
Destroyer            07h  00963b70  07 06 05 04      AirField        45h  0095ff30  45 05 04
Cruiser              0Ah  00963bf0  0A 06 05 04      Shipyard        46h  0095ffa0  46 05 04
LandingShip          0Ch  00963c80  0C 06 05 04      LandVehicle     19h  00960030  19 05 04
Cargo                0Bh  00963d00  0B 06 05 04      LandFort        1Bh  00749030  1B 05 04
BattleShip           0Dh  00963d80  0D 06 05 04      CommandBuilding 1Ch  00953680  1C 1B 05 04
Submarine            08h  00963e10  08 06 05 04      DummyTargetVeh  35h  009600a0  35 05 04
TorpedoBoat          0Eh  00963e90  0E 06 05 04      ReconPlane      14h  009536b0  14 0F 05 04
MotherShip           09h  00963f10  09 06 05 04      SmallReconPlane 15h  00953740  15 14 0F 05 04
Fighter              13h  00953650  13 0F 05 04      LargeReconPlane 16h  009537d0  16 14 0F 05 04
DiveBomber           12h  00953590  12 0F 05 04      Kamikaze        17h  00953500  17 0F 05 04
TorpedoBomber        11h  00953620  11 0F 05 04      LevelBomber     10h  00953470  10 0F 05 04
```

* 22 distinct bodies, **no sharing** — the trap that made four projectile descriptor classes answer
  only `29h` does not occur here.
* Every chain's head is exactly the `kind` that `docs/VEHICLE_CLASS_DESCRIPTORS.md` records.
* **All 22 chains equal the entity chain of the same id, truncated above `04`** (checked against the
  decoded entity chains of `docs/ATTACK_CAPABILITY_INPUTS.md` Part 1: 22 classes, 0 mismatches).
  `04`'s own entity chain is `{00, 01, 02, 04}`, so `04` is the vehicle-class root.

So a host answers `vt[18h](n)` with the **same** id-to-parent table it uses for `vt[5Ch]`, stopping
the walk at `04` instead of `00`. Unlike the projectile descriptors, these chains do not diverge
from the entity graph anywhere.

## `vehicleClass+178h` is `FakedType`, and it is authored

`00922C80`'s `MLandFort` tail compares `target->+538h->+178h` with `1Bh`. `docs/VEHICLE_CLASS_FIELDS.md:200`
lists `+178h..+180h` as rotation accelerations, but that is the **plane** reader `007D1F70`'s region;
each leaf reader lays out its own range above the shared base, so the offset means something else on
a structure descriptor.

A scan of every `[reg+178h]` operand in `.text` finds **no `MOV [reg+178h], imm32` of `1Bh`
anywhere** — the only immediates written at that offset are `0` and `1`. The write that matters is a
register store at `00749684`, inside `BSP_StructureClass_ReadLuaFields` (`00749210`):

```
0074965E  PUSH 0CFF824h                  ; "FakedType"
00749668  MOV  ECX,EDI / CALL 00B67800   ; the row key lookup
0074966F  MOV  EDX,1Bh                   ; the default
00749676  MOV  ECX,EAX / CALL 00746B90   ; resolve, __fastcall(ref /*ECX*/, default /*EDX*/)
00749684  MOV  dword ptr [ESI+178h],EAX
```

`00CFF824` is the C string **`FakedType`**, and `1Bh` — `MLandFort`'s own kind — is its **default**.
`00746B90` has exactly one caller, this site.

So the tail reads: an `MLandFort` is an attackable surface target **iff its `FakedType` is `1Bh`**,
i.e. iff the fort is not authored to impersonate another class. **Answerable from authored data**,
given a reader for that key.

Not established: whether `00746B90` resolves the `E <Enum> : <symbol>` form or only a bare number.
It is 331 instructions and calls `00B65FB0` then `00B662B0`; only its entry and default path were
read. A host needs that answer before it can parse `FakedType` values other than the default,
though the **default is what the comparison tests**, so a host that treats an absent key as `1Bh`
reproduces the shipped behaviour for every row that does not author it.

## The stamp-count discrepancy, resolved

`docs/ENTITY_CLASS_IDS.md` records "`MOV dword ptr [reg+0C4h], imm32` occurs 92 times in `.text`,
with 74 distinct immediates". Re-running the scan with the `66`-prefix filter that
`docs/ATTACK_CAPABILITY_INPUTS.md` identified:

```
sites=92   distinct=80   rejected (66-prefixed 16-bit writes)=2
```

The **92 agrees**; the distinct count does not. The exact set of 80 is

```
00 01 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e
20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f 30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d
3e 3f 40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d   58 5b 5c 60
```

— that is `00h`..`4Dh` **less `02h` and `1Fh`**, plus `58h`, `5Bh`, `5Ch`, `60h`. `02h` and `1Fh`
have class tests but are never stamped (abstract); `58h`, `5Bh` and `5Ch` are stamped with no test
of their own, as that document records. **80 is the count**; the doc undercounts by 6. Which 6 its
author's list omitted cannot be determined from the doc, so that part is recorded, not resolved.

## Wiring contract

| gate | rule | host needs |
| --- | --- | --- |
| `0047B850` | `IsKindOf(10h) \|\| IsKindOf(16h)`, required **false** | the plane's class id |
| `00604A50` | `IsKindOf(17h) && !+C24h`, required **true** | class id + `Platforms[].PilotFires` over the plane's platforms |
| `00827F70` | `vt18(0Eh) \|\| (vt18(0Ch) && !BigLandingShip)` | target's class id + `BigLandingShip` |
| `00828EC0` | `00827F70 && MaxSpeed >= 8.938947f`, required **false** | the above + `MaxSpeed` |
| `vehClass+178h` | `FakedType == 1Bh`, default `1Bh` | `FakedType` on structure rows |

All five are load-time data. **No gate in this packet needs runtime state**, so with
`docs/ATTACK_CAPABILITY_INPUTS.md`'s eight class queries a host can now evaluate every arm of
`BSP_Unit_AttackCommandApplies` except the three inputs that document already marked runtime
(`unit_has_weapon_controller`, `target_is_air`, `target_is_surface`, the last two through the
`+5Dh` engageability byte, world `y` and the `008DDF90` unit set).

## Still open

* `00746B90`'s value grammar (above) — the default path only.
* `008DDF90 BSP_SzurkeNyil_ContainsUnit` and its singleton selector, left open by agreement.
* The identities of class ids `58h` and `5Ch` (`5Bh` was resolved to `{5Bh, 01, 00}` in
  `docs/ATTACK_CAPABILITY_INPUTS.md`), left open by agreement.
* Which six immediates `docs/ENTITY_CLASS_IDS.md`'s "74 distinct" omitted.
* Nothing was compiled or run. No C++ was written for this packet.

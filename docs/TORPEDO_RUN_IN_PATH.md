# The torpedo run-in path: the sector scan's sampler (packet `cc8_torpedo_run_in_path`)

Addresses: `0041BC20` (`0041BC20`-`0041BC9F`), `0041BAE0`, `00417FA0`, `00903BC0`, `00903860`,
`007DF360`, `007F1D90` (`007F1D90`-`007F1DD6`), `0087FA20`, `009D31B0` (`009D31B0`-`009D31C2`),
`009D15F0` (`009D2303`-`009D236E`), `0099ACD0`.

The packet's premise was that the sector scan of `009D3420` had no terrain and that this was what
kept an ordered torpedo aircraft outside the engage distance. **Both halves turn out to be wrong,
and the run below shows why.** The sampler is bound here and the scan runs, but it cannot change
the run-in on open water, and a pilot-planner fix on `main` had already solved the closing problem
before this packet started.

## (1) `0041BC20` is not the terrain heightfield

`float __thiscall(layer, float x, float z)`, called at `009D36A9`, `009D386D` and `009D3A42` with
`ECX` = `ctl->+34Ch`.

```
00417FA0(&cell, x, z);                     /* world to cell, a float pair */
ix = (int)cell.u; iz = (int)cell.v;
h00 = 0041BAE0(ix, iz);   h10 = 0041BAE0(ix+1, iz);
h01 = 0041BAE0(ix, iz+1); h11 = 0041BAE0(ix+1, iz+1);
return bilinear(h00, h10, h01, h11, cell.u - ix, cell.v - iz);
```

`0041BAE0(layer, ix, iz)` clamps both indices to `[0, layer->+8h - 1]` and indexes the buffer
between `layer->+1Ch` and `layer->+20h`, so the layer is a square grid of side `layer+8h`.

**What `ctl+34Ch` points at.** The disp32 store scans `C7 ?? 4C 03 00 00` and `89 ?? 4C 03 00 00`
return eleven sites; the one on this object is `007F1DCA` in
`BSP_PlaneSquadron_SelectAvoidZoneLayers`, which `docs/PLANE_SQUADRON.md` already records:

```
ctl->+350h = BSP_AvoidZoneRegistry_SelectLayerBySlope(slope, flag);
ctl->+34Ch = BSP_AvoidZoneRegistry_SelectLayerBySlope([00CE380C] = 1.5f, 1);
```

So `ctl+34Ch` is an **avoid-zone layer chosen by climb slope**, not the terrain heightfield. The
positive control for the scan form is `007F2CDB` in `BSP_PlaneSquadronTickableEntity_Construct`,
which writes the same offset on the same object.

**`ctl` is the plane squadron.** That constructor also writes `+370h` at `007F2DC1`, the attack mode
`docs/TORPEDO_RELEASE_ORDERS.md` traced, and `009D4A70` writes `+394h`/`+398h` on the same block.
One object carries the squadron's avoid-zone layers, its attack mode, its armed fraction and its
unit array, which is why `ctl+3D0h` is an array and `ctl+370h` is flight-wide.

**`0041BC20` and `0087FA20` do not reach the same heightfield.** `0087FA20` is a one-line thunk,
`(*(parent->+3D0h))->vtable[+24h](x, z)`, and a full-image dword scan for `0041BC20` finds it in no
vtable at all, so it is not that virtual's body. The two are different samplers: one indexes an
avoid-zone grid directly, the other dispatches a geometry interface.

## (2) `00903BC0`, the segment test, does share the geometry interface

`char __thiscall(world, const float3* a, const float3* b)` with `ECX` = `[00E188A8]->+19CCh`
(`009D39CC`), the world's ground manager. `009D39D3` is the call.

```
h = 00903860(a);  if (a->y < h) return 1;          /* 009D390E */
h = 00903860(b);  if (b->y < h) {                   /* 009D3946 */
    for (node = world->+34Ch; node; node = node->+4h)
        if ((*(node->+8h)->+3D0h)->vtable[+3Ch](a, b, &out)) return 1;
    return 0;
}
return 1;
```

`00903860(world, point, &out)` walks the same list and takes the **maximum** height over the
objects, each answering `(*(node->+8h)->+3D0h)->vtable[+28h](x, z)`, seeding the result with
`[00D7A240] = -1000.0f` and reporting whether it changed from `[00CE6658] = -1000.0`.

So `00903860` and `0087FA20` do reach one interface, the object at `+3D0h`, through slots `+28h`
and `+24h`. That is the binding the host uses. The occluder sweep through slot `+3Ch` is
`contract: unread`.

## (3) `007DF360`, the over-land decision

`char __thiscall(this, target, const float3* point)`, `ECX` = `unit+C50h` at `009D36E6`, the call at
`009D36EE`, and its result overwrites `approach+A9h` outright at `009D36F3`. It reads
`this->+4h->+28h` as an entity, refreshes its pose, takes its x and z, measures against the point
with `BSP_Vector3_LengthFloatThreshold`, and then walks the range `[this->+30h, this->+30h +
this->+34h)`. `coverage: partial` - the ABI, the receiver and the first stage only. The host keeps
the conservative answer and logs it.

## (4) Why the sampler cannot change the run-in on open water

`009D3C5A`-`009D3C88` is the branch that matters:

```
if (ctl->+58h == 0x24 || ctl->+58h == 0) {     /* every sector clear, or none */
    approach->+5Ch = 0; approach->+AAh = 1;
    approach->+68h = 0; approach->+6Ch = 0;
}
```

**All thirty-six clear takes the same branch as none clear**, and both leave the turn offset at
zero. The gap search that produces a non-zero `+5Ch` runs only when the scan finds a mixture. USN01
is an open-water strike: every probe is over sea, every sector is clear, and the run below confirms
`clear_sectors=36 of 36` with `turn_5c=0.0000`. Giving the scan a sampler therefore changes nothing
on this mission, by the routine's own rule rather than by an accident of the binding.

The scan also runs **once** rather than per replan: `009D3752`-`009D37A9` keeps the plan while the
target has moved under 120 units in both x and z since `approach+ACh`/`+B0h`, and a ship under way
does not clear that in a tenth of a second.

## (5) The gate that is actually left: the aim-complete byte `state+2Ch`

`009D31B0` (`009D31B0`-`009D31C2`), `char __fastcall(state)`:

```
if (state->+4h->+132h == 0) return 0;     /* no ordnance left */
return state->+2Ch;                        /* 009D31BF */
```

Step 11 of `009D4030` keeps the task in `aim` while `task+52Ah` holds and this returns false, so the
byte is the only way out of `aim` toward `goaway`, `done` and the release.

`009D15F0` writes it at `009D236E`, under two clauses at `009D235A` and `009D2368`:

```
ramp = BSP_Math_InterpolateClamped(0, 0, 1.0f, speed * 0.5f, t);   /* 009D2345 */
if ((bearing_error - ramp) < threshold || t < |turn_offset|)
    state->+2Ch = 1;
```

`speed` is the `009D3C99` switch on `approach+134h` and `[00D7A280]` is `0.5`. The two operands the
comparison reads, the `threshold` at `[ESP+0x34]` and `t` at `[ESP+0x20]`, are stack slots written
earlier in a 3464-byte body with three `SUB ESP` adjustments, and pinning them needs a frame walk
this packet did not complete. **`coverage: partial`, and this packet does not guess at the rule**:
the reconstruction in `torpedo_approach_update.hpp` leaves `state+2Ch` unproduced and the census
names the gate by address instead.

## ABI summary

| address | ABI | evidence |
| --- | --- | --- |
| `0041BC20` | `float __thiscall(layer, float x, float z)` | `009D3699 SUB ESP,8` for two floats, `MOV ECX,EBX` |
| `0041BAE0` | `float __thiscall(layer, int ix, int iz)` | index clamp against `layer+8h` |
| `00903BC0` | `char __thiscall(world, const float3*, const float3*)` | `009D39C1` and `009D39D2`, two pushes; `ECX` from `009D39CC` |
| `00903860` | `char __thiscall(world, const float3*, float* out)` | three arguments, out-parameter |
| `007DF360` | `char __thiscall(unit+C50h, target, const float3*)` | `009D36EC` and `009D36ED` |
| `007F1D90` | `void __thiscall(squadron, float slope, int flag)` | `007F1DD6`, and `docs/PLANE_SQUADRON.md` |
| `009D31B0` | `char __fastcall(state)` | `009D31C2 RET` |

## Host methods

| site | callee | host method | this / args / ret | gate |
| --- | --- | --- | --- | --- |
| `009D36A9`, `009D386D`, `009D3A42` | `0041BC20` | `terrain_height_0041bc20` | `ctl+34Ch / x, z / float` | the scan and the over-land test |
| `009D39D3` | `00903BC0` | `segment_blocked_00903bc0` | `world / two points / bool` | scan probes inside 450 units |
| `009D36EE` | `007DF360` | `target_reachable_007df360` | `unit+C50h / target, point / bool` | `unit+C50h != 0` |

The avoid-zone layer, the occluder sweep through `vtable[+3Ch]` and `007DF360`'s list walk stay
**contracts**, logged through the unimplemented-host mechanism.

## Corrections

### Correction to `docs/TORPEDO_APPROACH_UPDATE.md` (packet `cc8_torpedo_approach_update`)

That doc calls `0041BC20` "the terrain height sampler" and `ctl+34Ch` "the terrain sampler", and its
follow-up 4 says that without them "the sector scan cannot run at all, so `+5Ch` stays zero and the
run-in has no planned heading". Three corrections. `0041BC20` samples the **avoid-zone layer**
`BSP_AvoidZoneRegistry_SelectLayerBySlope` returns, not terrain. `ctl` is the **plane squadron**,
which is also where the attack mode and the unit array live. And `+5Ch` stays zero on an open-water
mission **even with the scan running**, because `009D3C5A` treats thirty-six clear sectors exactly
like zero clear sectors.

### Correction to `docs/TORPEDO_RELEASE_ORDERS.md` (packet `cc8_torpedo_release_orders`)

Its Validation section names the in-range latch as the gate, on the ground that the closest approach
was 2906 against an engage distance of 2200. A pilot-planner fix merged to `main` after that run
changes the baseline: the same five aircraft now close to between 5.8 and 206 metres, the latch
closes, and the rule reaches `aim`. That section's numbers should be read as superseded by the ones
below.

## `no_ghidra_function`

None. `0041BC20`, `0041BAE0`, `00417FA0`, `00903BC0`, `00903860`, `007DF360`, `007F1D90`, `0087FA20`
and `009D31B0` all have Ghidra functions.

## Validation

`./scripts/build.ps1` (MSVC Win32, `/W4 /WX`) succeeds; `ctest` passes both existing suites. No test
was added.

| run | result |
| --- | --- |
| USN01 baseline, `main` after the planner fix | five aircraft, `states[moveto=139..268 attackrun=586..624 aim=445..535]`, two touching `prepare` for one tick, `releases=0`, closest range 5.8 to 206.1 against `8Ch=2200` |
| USN01 with the scan live | **identical** state and range figures; the scan reports `runs=1 clear_sectors=36 of 36 home_sector=0 turn_5c=0.0000 rad` on every aircraft |
| USN01 orders | unchanged: `peak_C58h=999 arm_offers=1298 attack_mode_370=1 drop_timer_98=-1.0` |
| USN02 | `shots=734 first_shot=1.40 s`, `hull=180 part=0 deaths=2 total_damage=18525.6` - the milestone 2t baseline |

The scan now runs and costs nothing in behaviour, which is the point: **the sampler was never the
blocker.** The blocker before this packet was the flight path, and `main`'s planner fix solved it;
the blocker now is the aim-complete byte of section (5), which the census names by address.

## Follow-up packets

1. **The two operands of `009D235A` and `009D2368`**, the aim-complete test. A stack-frame walk of
   `009D15F0` with `tools/stack_frame_walk.py` across its three `SUB ESP` adjustments, then the
   rule, then a release. This is the last gate.
2. **`009D2720` end to end** once `aim` can exit, so the `done`/`prepare` tick's release arms are
   exercised rather than reasoned about.
3. **The avoid-zone registry**, `BSP_AvoidZoneRegistry_SelectLayerBySlope` and the layer grid, for a
   mission with land where the sector scan would produce a non-zero turn.
4. **`007DF360`'s list walk** and the occluder sweep behind `vtable[+3Ch]`.

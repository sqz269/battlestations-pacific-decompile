# Projectile flight-and-impact helpers (packet `cc2_projectile_helpers`)

Addresses: `0084B380`, `0084B4B0`, `0078D1B0`, `0084AFD0`, `007BC4E0`, `0084B6F0`, `0084B000`,
`0084B650`, `0084B8C0`, `007C0910`, `0070C370` (`0070C4A4`-`0070C795`).

This packet closes the seven rows `docs/PROJECTILE_IMPACT.md` left as `contract: unread` in its
coverage table, plus that doc's `0070C370` "proximity search" and its "other caller unread". The
chain above these leaves is already reconstructed (`0084C430` sweep, `0084BF00` trace-and-impact,
`0084BC60` impact, `009239A0` dispatch in `include/bsp/projectile_impact.hpp`; the blast `0084BAD0`
in `include/bsp/blast_damage.hpp`) and is reused unchanged.

Everything below is read-only static analysis of `bsp.gpr` / `/battlestationspacific.exe`, plus the
raw listing for `0070C370`, which has no Ghidra function. No Ghidra object was modified.

## 1. The two static traces are a water-surface crossing search

`0084B380` and `0084B4B0` are the same routine with two predicates flipped. Both are

```
__fastcall float* Trace(float out[4] /*ECX, hidden return buffer*/,
                        TraceSegmentList* list /*EDX*/)
```

with no stack argument at all (plain `RET` at `0084B4A6` and `0084B5D6`; the call sites
`0084C139`/`0084C13B` and `0084C146`/`0084C148` load `EDX = ESI` and `ECX = ESP+2Ch` and push
nothing). The result buffer is

| offset | meaning |
| --- | --- |
| `+0h`..`+8h` | the crossing point, written only on a hit |
| `+0Ch` | the hit flag, `1` at `0084B496` / `0084B5C6`, `0` at `0084B41D` / `0084B54D` |

`0084C15F`..`0084C16B` reads the byte back and copies the three floats over the segment end.

### The second argument is a segment list, not a `float[6]`

`0084C430` reserves `0C4h` bytes (`SUB ESP,0xc4`), writes one segment at `+0h`..`+14h` from its two
by-value vectors and stores `1` into `+0C0h` at `0084C4BD`; it then pushes the base as `0084BF00`'s
first stack argument. Both traces read the count from `list+0C0h` and walk `count` records of `18h`
bytes from `list+0h`, so the buffer is

```
struct TraceSegmentList { struct { float3 from; float3 to; } segments[8]; int count; };  // 0C4h
```

`0C0h / 18h` is exactly `8`, which fixes the capacity. The projectile path always sets `count = 1`;
`007C0910` is the caller that fills more than one (section 4).

### The rule table

Per segment, in list order, both routines sample the water surface under the segment's **end**
point and stop at the first segment that satisfies their predicate.

| routine | predicate on the segment end | bisection call | meaning |
| --- | --- | --- | --- |
| `0084B380` | `end.y <= surface` (`0084B401` `FCOMIP` / `JBE`) | `0078D1B0(from, to)` at `0084B485` | the first segment that **enters** the water |
| `0084B4B0` | `surface < end.y` (`0084B531` / `JAE`) | `0078D1B0(to, from)` at `0084B5B5` | the first segment that **leaves** it |

The argument swap is real, not a decompiler artefact: at `0084B42A`..`0084B474` the block written
first (the higher stack address, so argument 2) is `[rec+0Ch..14h]`, while at `0084B55A`..`0084B5A4`
the same position holds `[rec+0h..8h]`.

`0084BF00` picks between them at `0084C11C`..`0084C14C`:

| `shot->vtable[2Ch]()` | `classDesc[+74h]` | trace |
| --- | --- | --- |
| `0` | anything | `0084B380`, surface entry |
| non-zero | non-zero | `0084B380`, surface entry |
| non-zero | `0` | `0084B4B0`, surface exit |

Slot `2Ch` is the shot's **medium** query (`0` in air), which is why a round that is already under
water looks for where it comes back out.

### `0078D1B0` is a bisection, not an interpolation

`__thiscall void Bisect(World* this /*ECX = [[00E188A8]+19F0h]*/, float3* out /*hidden*/,
float3 from, float3 to)`, body `0078D1B0`-`0078D3C8`, both vectors by value.

| order | site | step |
| --- | --- | --- |
| 1 | `0078D1B9`..`0078D212` | the midpoint: each component sum is stored to a float first, then scaled by the double `0.5` at `00D7A280` |
| 2 | `0078D21C`..`0078D257` | `\|to.y - from.y\|`, folded through the `-0.0f` at `00D7A208`, must exceed the double `0.01` at `00CE6638`, else the plain midpoint is returned |
| 3 | `0078D277` | `0042B2F0` on `to - from`; the loop runs only while the length exceeds `1.0` (`FLD1` at `0078D280`) |
| 4 | `0078D2A8`, `0078D2C2` | `0078C890` and `00B9CF50` on `[this+A8h]`, the wave height times the coverage mask: `0078CF20` inlined |
| 5 | — | the half that still straddles the surface is kept, the midpoint is recomputed and the working length is halved |
| 6 | `0078D3A3`..`0078D3AF` | the fresh midpoint when the last probe was submerged, else the far endpoint |

So the crossing point is accurate to about one world unit, and the sea it converges on is the
animated wave surface, not a plane at `y = 0`.

### An axis defect in the surface lookup

`0078CF20` is `float __thiscall(world, float x, float z)` (`docs/OCEAN_HEIGHT.md`,
`src/ocean_height.cpp`); `0078C890` scales **both** arguments by `field+B4h`, takes the fraction of
each and feeds them to one 2-D sample, and the sibling `0078D3D0` central-differences both of them
to build a surface normal. The two arguments are the two horizontal axes.

All three routines in this section pass the point's **first two** components:

- `0084B3D8` / `0084B3EB` push `[rec+10h]` and `[rec+0Ch]`, and `0084B3FD`..`0084B405` compares
  `[rec+10h]` against the result.
- `0078D29D` / `0078D2A5` push the midpoint slots `[ESP+20h]` and `[ESP+1Ch]`, and `0078D2E5`
  compares `[ESP+20h]`.

The compared value has to be the height, and `0084C430` copies plain world positions into the list,
so the layout is `(x, y, z)` and the lookup is being given `(x, y)` where `(x, z)` belongs. It is
latent on a flat sea, because `0078C890` returns `FLDZ` when `field+F9h` is set and the mask is
`1.0f` where no region covers the point, so the product is exactly `0.0f` and only `y <= 0` matters.
It is live wherever a mission has wave regions. The reconstruction reproduces it and says so.

## 2. The four impact leaves

### `0084AFD0`, the entity-hit notification walk

`__thiscall void Notify(Entity* e /*ECX*/, Notice* payload)`, `RET 4` at `0084AFFA`, body
`0084AFD0`-`0084AFFC`.

| order | site | step |
| --- | --- | --- |
| 1 | `0084AFD3` | a null receiver returns before any call |
| 2 | `0084AFEB` | `e->vtable[F8h](payload)`; a non-zero answer ends the walk |
| 3 | `0084AFF1` | otherwise `e = [e+3Ch]`, the parent link, and repeat |

The one call site is `0084C2F2`, guarded at `0084C2C8` by the replay flag `CL` and at `0084C2D0` by
`staging+1Ch` being non-null, so it runs only for a real entity hit. `0084C2E5` passes a pointer to
an eight-byte pair built on `0084BF00`'s own stack: `1` at `0084C2E6` and `[shot+0CCh]` (the
projectile) at `0084C2EE`. Nothing else is read from the shot or the staging buffer.

### `007BC4E0`, the plane's crash effect

`__fastcall void Bind(Plane* p /*ECX*/)`, plain `RET` at `007BC548`, body `007BC4E0`-`007BC548`.

| order | site | step |
| --- | --- | --- |
| 1 | `007BC4E6`, `007BC4EF` | return when `[p+0AA4h]` or the byte `[p+0AA8h]` is already set |
| 2 | `007BC4F8` | `00414DB0` when the pose flag `[p+0C8h]` is clear |
| 3 | `007BC512` | `p->vtable[34h](&local)`, whose `EAX` becomes the effect's direction |
| 4 | `007BC524` | `0084B6F0([[p+730h]+24h], p+0FCh, direction)` |
| 5 | `007BC52B`..`007BC53D` | store the handle, `InterlockedIncrement` on `handle+4h`, set `[p+0AA8h]` |

The receiver at the only `0084BF00` call site (`0084C349`) is `EBP`, which `0084BF3F`/`0084BF66`
set to `[shot+0CCh]` and only when that object answers `IsKindOf(0Fh)`. `docs/ENTITY_CLASS_IDS.md`
maps class id `0Fh` to the plane base, so this is the plane branch, and the second guard at
`0084C33F`/`0084C344` requires `[EBP+60h]` or `[EBP+5Dh]` to be set. `p+730h` is the **unit** class
descriptor, so its `+24h` is a different field from the weapon descriptor's `+24h` in section 3.

The routine is idempotent by construction: a plane that scrapes the ground over many ticks keeps
one effect instead of accumulating them.

### `0084B000`, the network hit message

`__thiscall Message* Build(Message* this /*ECX*/, int mode, void* source, const float3* hitPos,
const float3* direction)`, `RET 10h` at `0084B062`, body `0084B000`-`0084B064`. Four stack
arguments from the cleanup.

| offset | site | written |
| --- | --- | --- |
| `+0h` | `0084B025` | the vtable `00D02D80` |
| `+4h` | `0084B033`, `0084B05E` | `1`, twice |
| `+18h`, `+1Ah` | `0084B013`, `0084B017` | zeroed |
| `+1Ch` | `0084B01E` | `mode` |
| `+20h` | `0084B02B` | `source` |
| `+24h`..`+2Ch` | `0084B036`..`0084B048` | `*hitPos` |
| `+30h`..`+38h` | `0084B04B`..`0084B05B` | `*direction` |

`0084B008` calls `0075B430` with `0BAh`, the session message kind. The single call site is
`0084BD89` inside `0084BC60` step 3, reached only when `[00E188A8]+1FE4h == 1` and `[shot+0CCh]`
answers `IsKindOf(2Ah)`; `00BD89`'s result then goes to `BSP_Session_RouteMessage(msg, 4, 0)` at
`0084BDA0`. The arguments are `mode = EBP` (the refined impact mode), `source = ` the return of
`[EDI+310h]->vtable[2Ch]()`, `hitPos = staging+24h..2Ch` and `direction = ` `0084BC60`'s fourth
stack argument.

### `0084B650`, the guarded down-cast

`__fastcall void* Cast(void* o /*ECX*/)`, plain `RET` at `0084B667`/`0084B66B`, body
`0084B650`-`0084B66B`: `o` when it is non-null and answers `vtable[5Ch](2Ah)`, else null. The
`PUSH EDI` at `0084BD43` before the only call site is a register save paired with the `POP EDI` at
`0084BDAD`, not an argument. The result `EDI` is then dereferenced at `+310h` without a null check,
which is safe only because `0084BD26`..`0084BD39` already ran the same `IsKindOf(2Ah)` test on the
same object.

## 3. `0084B8C0` is the impact-effect dispatch, not a decal

`__fastcall void Dispatch(int mode /*ECX*/, WeaponClassDesc* d /*EDX*/, int medium, const float3*
position, const float3* direction)`, `RET 0Ch` at every exit, body `0084B8C0`-`0084B96B`. It
contains nothing but a two-level switch; every arm calls `0084B6F0(d + offset, position,
direction)`.

| `mode` | `medium` | slot | site |
| --- | --- | --- | --- |
| `1` entity | `0` | `d+28h` | `0084B908` |
| `1` entity | `1` | `d+30h` | `0084B8F5` |
| `1` entity | `2` | `d+34h` | `0084B8E0` |
| `1` entity | other | none | `0084B969` |
| `2` static | any | `d+24h` | `0084B922` |
| `3` landscape | `0` | `d+38h` | `0084B943` |
| `3` landscape | `1` | `d+30h` | `0084B8F5` |
| `3` landscape | `2` | `d+34h` | `0084B8E0` |
| `4` plane | any | `d+3Ch` | `0084B95D` |
| other | any | none | `0084B969` |

Mode `3` reaches the shared arms through the `JNZ 0x0084b8ce` at `0084B93D`, which jumps into mode
`1`'s chain; that is why the submerged slots are shared and the dry ones are not.

At the call site `0084BDEA`, `ECX = EBP` is the refined mode, `EDX = [staging+8h]` is the weapon
class descriptor, and the three stack dwords are, low to high, the return of `shot->vtable[2Ch]()`
pushed at `0084BDE7`, `&hitPos` pushed at `0084BDD8` and `0084BC60`'s fourth argument pushed at
`0084BDC8`. The whole step is gated by the byte `[shot+45h]` at `0084BDB0`.

### The renderer contract

`0084B6F0` (`0084B6F0`-`0084B880`, `__fastcall(void* slot /*ECX*/, const float3* position /*EDX*/,
const float3* direction)`) is where this packet stops. It

1. builds an identity 4x4 at `0084B70x`;
2. when any component of `direction` is non-zero, calls `BSP_Matrix_BuildLookAt` then
   `BSP_Matrix_Copy4x4X87` to orient it;
3. writes `*position` into the translation row;
4. takes a reference on the effect definition `*slot` with `InterlockedIncrement`;
5. calls `BSP_PointEffect_CreateFromMatrix(&holder, [00E188A8]+19ECh, definition, matrix, 0, 0)`
   and returns `*result`;
6. releases its reference with `InterlockedDecrement`.

`BSP_PointEffect_CreateFromMatrix`, the effect manager at `[00E188A8]+19ECh` and the matrix helpers
are the renderer's and were not read. The contract this packet hands over is: *given a definition
pointer, a world position and a direction, create one oriented point effect and return its handle;
a zero direction means an unoriented effect.*

That last clause matters. `0084BF00`'s fifth stack argument, the one that reaches both `0084B000`
and `0084B8C0` as the `direction`, is the global `00F87574` on the projectile path (`0070C85E`
pushes it as an immediate, and `0084C430` forwards it). The eight bytes there are zero on disk and
all 41 cross-references are reads, so it is the shared read-only zero vector. Every shell impact
therefore spawns an **unoriented** effect and sends a hit message whose direction field is zero.

## 4. `007C0910` is the plane's own swept collision test

`__thiscall int Sweep(Plane* p /*ECX*/, void* a, void* b, void* extra, float step)`, body
`007C0910`-`007C0C5F`. It is the second caller of `0084BF00` and the reason that routine has a
`[shot+0CCh]->vtable[5Ch](0Fh)` branch: on this path `[shot+0CCh]` **is** the plane, class `0Fh`.

| order | site | step |
| --- | --- | --- |
| 1 | `007C0930` | return early while the cooldown `[p+9DCh]` is positive or `[p+61h]` is set, decrementing the cooldown by `step` |
| 2 | `007C095x` | the collision-point vector on the unit class descriptor, `[[p+538h]+5BCh]`..`[+5C0h]`, stride `0Ch` |
| 3 | per point | transform by the previous pose `p+924h` and the current pose `p+0CCh`, giving one segment per point |
| 4 | `007C0A4x` | a below-`00CE7D7C` arm shortens the segment to the surface crossing with `[00D7A308]` |
| 5 | `007C0C3x` | `0084BF00(list, p+72Ch, p, [p+730h], extra, &local)` |

The `0C0h`-byte local (`local_c4` in the pseudocode) is the segment list of section 1, filled with
one segment per collision point, which is exactly why the traces loop at all. So the same routine
answers "where did this shell hit" for a projectile with one segment and "did this aircraft touch
anything" for a plane with several, and `007BC4E0` attaches the crash effect on the way through.

`0084BF00`'s remaining arguments differ only in their sources: the plane passes its own `extra` and
a stack local where the projectile passes the zero vector `00F87574` and `projectile+1C8h`.

Coverage: this section is the caller's shape and its use of the trace. The rest of `007C0910`, in
particular the per-point arm at `007C0A4x`-`007C0C30`, is `contract: unread`.

## 5. The flak proximity search, `0070C4A4`-`0070C795`

`0070C370` has no Ghidra function. Read from the raw listing: `__thiscall(tickElement /*ECX*/,
float step)`, `RET 4`, body **`0070C370`-`0070CAD3`** (see the corrections). `ESI` is the tick
element at `projectile+244h`, so `EBP = ESI-244h` is the projectile and `[ESI-0D0h]` is the weapon
class descriptor.

### Setting up

| order | site | step |
| --- | --- | --- |
| 1 | `0070C41C`..`0070C491` | `segment = [proj+0FCh] - [proj+1DCh]`, length from `0042B2F0`, then normalised in place |
| 2 | `0070C4A8`..`0070C4FF` | the midpoint of the same two points, componentwise sum stored to a float then halved by the double at `00D7A280` |
| 3 | `0070C4A4`, `0070C503` | when `[proj+288h]` is already set the whole search is skipped: a latched round does not re-acquire |
| 4 | `0070C50F`..`0070C526` | the squared search radius, `(length/2 + 2 * classDesc[+70h])^2` |

`classDesc[+70h]` is the blast radius `docs/EXPLOSION_RADIAL_DAMAGE.md` names as the radius source
for `0070C210`, so the fuse reaches twice the lethal radius plus the step, and nothing else feeds
it: there is no separate fuse-radius field.

### The walk

`0070C52A` calls `BSP_Recon_EnsureSlot` with `ECX = [proj+54h]`, the owner's party index, and takes
the intrusive list at `slot+0DE8h` (`next` at `+4h`, payload at `+8h`, entity at `payload+4h`), the
same list `009F5D30` walks in `docs/BOT_FIRE_TARGET.md`. So a flak round searches **its own party's
recon set**, not the whole world.

| order | site | test |
| --- | --- | --- |
| 1 | `0070C546` | skip a null entity |
| 2 | `0070C553` | require `IsKindOf(5)` |
| 3 | `0070C566`, `0070C575`, `0070C586` | require one of `IsKindOf(0Fh)`, `(0Eh)` or `(0Ch)` |
| 4 | `0070C592` | refresh the candidate's pose when `[e+0C8h]` is clear |
| 5 | `0070C5A8`..`0070C61F` | the squared radius is clamped to `90000.0f` at `00CFD508`, so the search never reaches past 300 units |
| 6 | `0070C62D` | require `clampedRadius^2 > \|e.pos - midpoint\|^2` |
| 7 | `0070C639` | require the squared distance to beat the running nearest, seeded `FLT_MAX` at `00D7A248` |
| 8 | `0070C661`, `0070C665` | latch: `[proj+288h] = 1`, `[proj+294h] = e`, and `[proj+284h]`'s tracker takes the new distance at `0070C671` |
| 9 | `0070C677`, `0070C680` | `00427EB0(projectile)` then `00901C20(&worldPos, e, classDesc[+50h], 00F87574, &out, &out2)` |
| 10 | `0070C6A6`..`0070C6E8` | `[proj+28Ch] = max(0, [proj+290h] + dot(direction, out - [proj+1DCh]))` |

Step 9 is where `docs/PROJECTILE_IMPACT.md`'s argument attribution needs care: `00427EB0` is
`BSP_EntityPose_GetWorldPositionRefreshed`, `__fastcall(ECX)` with a plain `RET`, so the four
dwords pushed at `0070C656`..`0070C66E` are not its arguments. They belong to `00901C20`, which is
`RET 10h` (`0090227E`). `00901C20` is the ballistic lead solution `009030C0` also uses; it is
`contract: unread` here and in `docs/BOT_FIRE_TARGET.md`. Its first argument is the class muzzle
speed `classDesc[+50h]` and its second is the zero vector.

So `[proj+28Ch]` is **how much further the round must fly before it is level with the target's
predicted position**, measured along its own flight direction, and `[proj+290h]` is a standing bias
added to it.

### Firing, and the consolation burst

| order | site | step |
| --- | --- | --- |
| 1 | `0070C709` | nothing latched: fall through to the airburst roll |
| 2 | `0070C71C` | `step_length >= [proj+28Ch]`: the round covers the remaining distance this tick |
| 3 | `0070C726`..`0070C791` | move the projectile to `[proj+1DCh] + direction * [proj+28Ch]`, set `[proj+0C8h] = 1` and `[proj+10Ch] = 0` |
| 4 | `0070C79B` | `0070C210(projectile, 1)`, the detonation, then `RET 4` |
| 5 | `0070C7AD` | otherwise `[proj+28Ch] -= step_length` and carry on |
| 6 | `0070C7BE` | the airburst roll needs the nearest squared distance under `90000.0f` |
| 7 | `0070C7CA` | while the miss distance is still shrinking, record it in `[proj+284h]` and wait |
| 8 | `0070C7D7` | once it grows, a miss inside `2500.0f` (50 units) is left to the impact path |
| 9 | `0070C7E0`..`0070C806` | otherwise `00BD2F10(0.0f, [00CE3D08] = 100.0f) < [00CE38B8] = 10.0f`, a 10% per-tick chance, jumps to the same detonation |

The expiry branch at `0070C3B5` calls `0070C210(projectile, 0)`; every proximity burst passes `1`.
`0070C210` itself is reconstructed in `docs/EXPLOSION_RADIAL_DAMAGE.md` and is not re-read here.

## Corrections to earlier documents

| was | is | evidence |
| --- | --- | --- |
| `0084BF00` takes five stack arguments (`docs/PROJECTILE_IMPACT.md` prototype, from Ghidra) | six | `RET 0x18` at `0084C427`; `0084BF71`/`0084BFCA`/`0084BFD1`/`0084BFD8` read six slots; `007C0910` and `0084C430` both pass six |
| its third argument is `float segment[6]` | a `0C4h`-byte segment list, `8` segments plus a count at `+0C0h` | `SUB ESP,0xc4` and `MOV [ESP+0xd8],1` at `0084C430`/`0084C4BD`; `MOV EAX,[EDI+0xc0]` at `0084B388` |
| `[..+310h]->vtable[2Ch](&hitPos, extra)` and `shot->vtable[2Ch](&hitPos, extra)` | slot `2Ch` takes no arguments; the two pushes belong to `0084B000` / `0084B8C0` | `0084B000` is `RET 10h` with only two dwords pushed after the virtual call, `0084B8C0` is `RET 0Ch` with one; the other three sites `0084C12D`, `0084C364`, `0084C3E1` push nothing |
| `0084B8C0` `place_decal` | the impact-effect dispatch; every arm reaches `BSP_PointEffect_CreateFromMatrix` through `0084B6F0` | `0084B8E8`..`0084B964`, then `0084B7xx` |
| `0070C370` body `0070C370`-`0070C7AA` | `0070C370`-`0070CAD3` | `0070C3E6` and `0070C400` jump to `0070C808`; `0070C89B` calls `0084C430`; the last `ret 4` is at `0070CAD1`, `int3` padding follows to `0070CADF` |
| step 5's `00427EB0` and `00901C20` listed without argument attribution | the four pushed dwords are `00901C20`'s | `00427EB0` is a plain `RET` (`00427EC8`); `00901C20` is `RET 10h` (`0090227E`) and its `SUB ESP,0xcc` / `COMISS XMM0,[ESP+0xd0]` reads the first of them |
| `ProjectileImpactMode::kScoringTarget = 3`, `kUnit = 4` | mode `3` is a `Landscape` hit and mode `4` a plane hit | `PUSH 0x44` at `0084BC99` and `PUSH 0xf` at `0084BCB0`; `docs/ENTITY_CLASS_IDS.md` maps `44h` to `Landscape` and `0Fh` to the plane base |
| `kProjectileOffOwnerId = 0x54` | the owner's **party** index | `0070C512` loads it into `ECX` for `BSP_Recon_EnsureSlot`, whose table `00F874BC` has three slots; `docs/BOT_FIRE_TARGET.md` names `[unit+54h]` the same way |
| `0084BF00`'s `IsKindOf(0Fh)` branch, "other caller unread" | the caller is `007C0910`, the plane's swept collision test, and the branch is the plane crash effect | `007C0C3x` call site; `EBP = [shot+0CCh]` at `0084BF3F`/`0084BF66` |

`docs/PROJECTILE_IMPACT.md` remains correct on everything else this packet touched, including
`0084BC60`'s step order, the staging layout and the mode refinement itself.

## Coverage

| routine | coverage |
| --- | --- |
| `0084B380`, `0084B4B0` | complete |
| `0078D1B0` | complete; the wave field under `0078C890` / `00B9CF50` is `docs/OCEAN_HEIGHT.md`'s |
| `0084AFD0`, `0084B000`, `0084B650` | complete |
| `007BC4E0` | complete; `p->vtable[34h]` is a contract |
| `0084B8C0` | complete |
| `0084B6F0` | `contract: renderer`; the six-step shape above is read, the effect manager and matrix helpers are not |
| `007C0910` | partial: the caller's shape and the `0084BF00` call. `007C0A4x`-`007C0C30` is `contract: unread` |
| `0070C370` | partial: `0070C370`-`0070C795` and the tail at `0070C79B`-`0070C806`. `0070C808`-`0070CAD3` is `contract: unread` |
| `00901C20`, `00427EB0`, `008053C0`, `0070C210` | reused as contracts from `docs/BOT_FIRE_TARGET.md` and `docs/EXPLOSION_RADIAL_DAMAGE.md` |

`no_ghidra_function`: `0070C370`, inclusive end `0070CAD3`.

## Reconstruction

`include/bsp/projectile_helpers.hpp` and `src/projectile_helpers.cpp`, registered in
`cmake/startup.cmake`. Pure rules for the trace selection, the bisection, the two traces, the
effect-slot table, the flak search radius, the candidate filter, the burst test and the airburst
conditions; injected hosts for the five native call-site groups. The segment length reuses
`force_event_vector_length_0042b2f0` (`include/bsp/gamepad_force_events.hpp`), the same native
routine `0078D277` and `0070C45F` call, rather than a second square root.

Status: build-tested on the Win32 MSVC build with warnings as errors; the existing
`reconstructed_math` suite passes. Not fixture-tested against the binary and not ABI-compatible.

## Open questions

- `00901C20`'s own contract. It is `contract: unread` in two packets now and it decides where every
  flak round bursts.
- What writes `[shot+45h]`, the gate on the whole impact-effect step, and `[proj+290h]`, the bias
  added to the flak burst distance. Neither producer was found in this packet.
- `0084BF00`'s sixth argument. The projectile passes `projectile+1C8h` and the plane a stack local;
  nothing in the range read here consumes it.
- Whether the `(x, y)` surface lookup is reachable with a non-flat sea in any shipped mission. That
  needs the wave-region data, not the listing.
- `0070C808`-`0070CAD3`, the flak tick's own sweep-and-impact tail, and `007C0910`'s per-point arm.

## Correction from docs/KILL_CREDIT.md (packet cc2_kill_credit)

- **Was:** neither producer of [shot+45h] nor of [proj+290h] was found
  **Is:** 006e2353 inside the shot base constructor 006e22d0 sets [shot+45h] to 1; 0070ce6b inside BSP_FlakBulletClass_CreateProjectile writes [proj+290h] = Blast.BlastRange * arg6[+4h]
  **Evidence:** 006e2353 MOV byte ptr [ESI+0x45],AL with AL set at 006e2313; 0070ce65 FLD [ECX+0x70]; 0070ce68 FMUL [EDX+0x4]; 0070ce6b FSTP [ESI+0x290]

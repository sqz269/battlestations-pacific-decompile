# HUD central updates (packet `hud_central_updates`)

Addresses: 00649860, 006435D0, 005C0F20, 006463E0, 008E62A0, 008E9AF0, 00AB64C0, 00ABE6E0,
00AAB4C0, 00AA6740, 00AA8240, 00AA7DC0, 00AA1FE0, 00432650, 00427EB0, 00427E30, 004B4B00,
00927880, 00414DB0, 006430C0, 006434E0, 00640620, 00640D70, 0063B5E0, 0063BCD0, 00642C20,
00643360, 00645060, 00645600, 00648060, 006485A0, 00644CC0, 00644C20, 00644DB0, 00648C20,
00647080, 005BCF70, 005BE110, 005BD590, 005C0700, 005BD420, 00E197C4, 00F88C30, 00F876A4,
00F88A00, 0109CF04

This packet is the follow-up of `docs/HUD_SCREEN_PAGES.md`, which characterised the three central
HUD screens' update virtuals but left them as "analyzed only". All three were read end to end from
the listing here and the parts whose inputs are fully resolved were written as C++ in
`include/bsp/hud_updates.hpp` and `src/hud_updates.cpp`.

Every constant, every operation order and every stack slot below comes from `bsp.py ghidra disasm`,
not from the decompiler: the HUD root pseudocode aliases both loop counters to floats
(`unaff_EBX`, `fVar14`), the markers pseudocode hides the squared-distance test on the x87 stack,
and Ghidra's own disassembly desynchronises inside the markers body (see **Ghidra state**).

## Calling conventions and RET sizes

| Address | Screen | Convention | RET | Body | Ghidra function |
| --- | --- | --- | --- | --- | --- |
| 00649860 | slot 44h, the HUD root | `__thiscall(this, float)` | `ret 4` at 0064A24A | 00649860..0064A24C | yes |
| 006435D0 | slot 4Dh, the world markers | `__thiscall(this, float)` | `ret 4` at 00643D94 | 006435D0..00643D94 | yes, defined by the orchestrator |
| 005C0F20 | slot 35h, the minimap | `__thiscall(this, float)` | `ret 4` at 005C27DB | 005C0F20..005C27DB | yes, defined by the orchestrator |

Only 00649860 and 005C0F20 read the float. 006435D0 never touches its argument: it advances no
timer of its own and rebuilds the whole marker set every frame.

005C0F20 is the only one of the three with an SEH frame: it pushes the handler 00C729CC at
005C0F28 and unwinds at 005C27CE.

## Slot 44h, the HUD root update 00649860

### Cadence and gate

Three tests at 00649863..00649898, all before anything else:

1. `[00E188A8 + 61Fh]` must be zero.
2. `[00E188A8 + 620h]` must be zero.
3. `this->[F4h]` is decremented, and the body runs only when the result is not greater than zero
   (`CMP dword [EBX+F4h],0` / `JG`). On a running frame `+F4h` is reloaded with 2 at 006498A0.

So the clone pass runs on every second call. The enter virtual 006488D0 zeroes `+F4h`, so the first
frame after entering the screen runs the body.

The tail from 00649FB3 runs unconditionally, gate or no gate: the ticker timer, the pending unit
handle, the selector widget, the weapon-info field, the row and medal updates, the closed-HUD
toggle and the group-change check are all outside the cadence.

### The clone-and-place structure

`006463E0`, the layout virtual, binds two page widgets as clone templates and hides both
immediately with virtual `+34h(0)`:

| Field | Widget | Evidence |
| --- | --- | --- |
| `this->[FCh]` | `puptemplate_Icon` | name string at 00CF5A60, stored at 006464C9 |
| `this->[100h]` | `circletemplate_Section` | name string at 00CF5A48, stored at 0064654D |

The update keeps two clone vectors, both `std::vector<Widget*>` with the allocator at the vector
base: `+104h` (first `+108h`, last `+10Ch`, end `+110h`) for icon clones and `+114h`
(`+118h`/`+11Ch`/`+120h`) for circle clones. On a running frame 006498AC..006499CC destroys every
element through widget virtual `+4h(1)`, nulls the slot, and then erases the vector, the first
through the vector-erase instantiation 005A0B20 and the second through an inline `memmove_s`.

The entries themselves come from the power-up manager. 00649A36 calls `008E9AF0` with
`ECX = [00F88C30]` and the screen's five vectors plus two literals:

```
008E9AF0(this+164h, this+124h, this+134h, this+144h, this+154h, 0, 1)
```

`+164h` receives the untimed entries, `+124h` the timed entries and `+154h` their deadlines as
floats. `+134h` and `+144h` are filled but never read by this routine. The whole pass is skipped
when `[00E188D8]` is null (006499FC).

**Loop one, 00649A3D..00649C21**, over `+164h`:

1. `008E62A0(entry, [00E188D8])` decides whether the power-up applies to the controlled unit. That
   routine reads the entry's descriptor type at `[[entry+4] + 8]`: type 3 compares the scope byte
   at `[[entry+4] + 84h]`, where 4 means "same `unit+180h`" and 5 means "same `unit+54h`"; type 2
   scans sixteen 0Ch-byte list heads at `[00F88C30 + 84h + i*0Ch]` for an entry whose `+8h` is this
   power-up and that passes 008E4680.
2. `BSP_GuiWidget_CloneSubtree(this->[FCh], 0)`, then clone virtual `+34h(1)`, then a `push_back`
   into `+104h` (inline fast path, else 0043DB60).
3. `BSP_GuiWidget_GetSize(this->[FCh])`; only the height at `[result+4]` is used.
4. The position, with the row accumulator held in a stack slot (not in a register, despite the
   pseudocode): `accumulator += height + 00CEBDD0`, then
   `BSP_GuiWidget_SetResolvedPosition(clone, {00CE8190 + 00CF5C00, accumulator, 00CE65D8})`.
5. `00AB64C0(clone, texture, uvRect)` where
   `texture = [[entry+4] + 30h + id*38h]` and `uvRect = [entry+4] + 34h + id*38h`, with
   `id = [[00E188A8 + 18CCh + [00E188A8 + 18ECh]*4] + 28h]`, the local team's record id.
6. Clone virtual `+88h(1, 0, 1.0f)`, the state select that follows every add.

**Loop two, 00649C28..00649FAC**, over `+124h` with the deadlines at `+154h`:

1. The entry is skipped unless `deadline - 00F876A4 > 0` (00649C71, `FCOMIP` against `FLDZ`).
2. The same 008E62A0 test, the same icon clone, the same row advance and the same
   `00AB64C0` state add.
3. A second clone from `this->[100h]` pushed into `+114h` (grow path 00647B60), positioned at
   `{00CE8190 + 00CF5C00, 00CE8190 - 00CF0DE8, 00CF5BFC}`. The circle's y does **not** follow the
   row accumulator: it is pinned one 1/120 above the shared column anchor.
4. `00ABE6E0(circle, (deadline - 00F876A4) / [[entry+4] + 7Ch], 0, 0, 1.0f)`.

### The two widget setters

`00AB64C0`, `__thiscall(icon, texture, const float uv[4])`, body 00AB64C0..00AB66B8, appends an
Icon **state**: it takes a reference on the texture (`InterlockedIncrement` on `texture+4`), copies
its name string, reads four floats from the UV pointer, swaps the u pair and the v pair when the
icon's own flip fields equal the sentinels 00D7A24C and 00D7A218, appends the 40h-byte record
through 00AB63A0 and returns `((icon+F8h - icon+F4h) >> 6) - 1`, the new state index. The
`+F4h/+F8h/+FCh` state vector and the `>> 6` count match `docs/GUI_ICON_WIDGET.md`.

`00ABE6E0`, `__thiscall(widget, float, float, float, float)`, body 00ABE6E0..00ABE771, compares
then writes `+F4h`, `+F8h`, `+100h` and `+104h` and re-emits through virtual `+7Ch`. It is a
different widget class from Icon (those offsets are the Icon state vector), and on this call site
the target is a `Section`. The four values are `(fill, 0, 0, 1)`, so the fill is the first
component; which visual property that is was **not** established.

### Layout constants

| Constant | Value | Role |
| --- | --- | --- |
| 00CE8190 | 0.26 | the column anchor, shared by the icon x and the circle y |
| 00CF5C00 (double) | 0.935 | added to the anchor for the column x, 1.195 |
| 00CF5C08 | 17/120 = 0.14166668 | the first row's y when `[0109CF04 + 0Dh]` is set, else 0 |
| 00CEBDD0 (double) | 1/72 = 0.013888889 | the gap added to each template height |
| 00CF0DE8 (double) | 1/120 = 0.008333333 | subtracted from the anchor for the circle y |
| 00CE65D8 | -100 | icon clone depth |
| 00CF5BFC | -99 | circle clone depth, in front of the icon |
| 00CF5BF8 | 7 | reload of the ticker timer at `+BCh` |

`0109CF04` is the platform singleton (`docs/WINMAIN_STARTUP.md`); the byte at `+0Dh` selects the
inset first row. The layout space is height-normalised, not 0..1: an x of 1.195 puts the column at
the right edge of a wide viewport.

### The unconditional tail

- **00649FB3.** `+BCh -= delta` while positive; once it reaches zero and the queue at `+B0h..+B4h`
  (stride 10h) is non-empty, `00648060(this)` runs and `+BCh` is reloaded with 7. A message ticker.
- **0064A00E.** `+78h` holds a unit handle as a word. It is resolved through the standard handle
  table (`00F89A10`/`00F89A0C`/`00F89A54` below the split, `00F89A60`/`00F89AA8` above) and must
  pass `+5Ch` set with `+5Dh`, `+60h` and `+5Eh` clear. Then `00645060` runs with the resolved unit
  in ECX, the local team index in EDX and a literal 1 on the stack; **the EDX load at 0064A085 is
  not visible in the pseudocode, so 00645060 is `__fastcall`-shaped, not the `__thiscall(this, 1)`
  Ghidra shows**. On success `00645600(this, unit)` commits, then one of two interface requests
  goes to `BSP_FrontEndManager_PushInterfaceRequest`: with no controlled unit, id 34h
  (`INTF_LIMBO`) and payload 0, but only while the interface manager is idle
  (`[00E198C4+4h] == [+20h]` and `[+1Ch] == [+38h]`); with a controlled unit, 0059DA80 and
  005251C0 run first and the request is id 20h (`INTF_SCENE3D`) with the unit's virtual `+140h`
  as payload. `+78h` is cleared either way.
- **0064A108.** `this->[40h]` virtual `+34h(1)`, the selector widget shown every frame.
- **0064A114.** `00644CC0(&out, [00E188D8])`; when the word at `out+2h` is FFFFh the value is taken
  from `00644C20(&tmp, [00E188D8])` instead. The result is stored at `this + C2h`, an unaligned
  dword store.
- **0064A149.** `00644DB0(this, delta)` then `00648C20(this)`.
- **0064A165.** The closed-HUD toggle: it runs only with a controlled unit, `[00E188A8 + 19C4h]`
  clear, and the unit failing all three kind probes 9, 45h and 46h (virtual `+5Ch`). Input action
  DFh is polled through `BSP_InputAction_WasPressedThisFrame`; a unit of kind 18h whose `+379h`
  byte is set takes the `this->[1Ch]` branch instead of the raw press. `00647080(this)` performs
  the toggle.
- **0064A20B.** With `this->[20h]` set, a game at `00E188A8` with a group manager at `+1FE4h`, a
  controlled unit whose team at `+188h` is below 8 and differs from `[00E188A8 + 18ECh]`,
  `006485A0(this)` runs and `+20h` is cleared.

No localisation key is resolved anywhere in 00649860: every string it moves is a texture name
carried inside the power-up record.

## Slot 4Dh, the world markers update 006435D0

### Cadence and gate

No cadence counter: the routine runs its whole body every frame. Three gates at 006435D8:
`[00E188A8 + 18ECh]` must be in `0..7`, and `[00E188A8 + 61Fh]` and `+620h` must both be zero. A
fourth gate at 006436D1, `this->[28h]`, skips only the marker passes, after the clip rectangle and
the pool reset have already run.

### The clip rectangle

`00AA1FE0(&extent)` returns the GUI extent as two floats. Both are multiplied by 00CF5990 (0.4925)
and the results are laid out around 00D7A280 (0.5) into six consecutive floats:

| Global | Value |
| --- | --- |
| 00E197D0, 00E197D8 | `0.5 - width * 0.4925` |
| 00E197CC, 00E197D4 | `0.5 + width * 0.4925` |
| 00E197C8 | `0.5 - height * 0.4925` |
| 00E197C4 | `0.5 + height * 0.4925` |

Each horizontal bound is stored twice, so 00E197C4..00E197D8 is most likely two rectangles sharing
their x bounds. The x87 sequence at 0064361B..0064367F was read instruction by instruction; the
pseudocode does not show it.

### Frame order

1. `this->[A0h] = 0`.
2. `this->[24h] = 004B4B00()`, the camera unit. The same accessor feeds the minimap.
3. `00640620(this)`, the marker pool reset.
4. `0063BCD0(this + 34h)`, clearing the marker set. `+34h` is a vector with first at `+38h` and
   last at `+3Ch`; 004323D0 is its `push_back` and the routine's last act at 00643D46 erases it.
5. `this->[1Ch] = [00E188D8] ? 00927880([00E188D8]) : 0`, the unit the self marker points at.
6. `006394B0(this)` then `0063B5E0(this + A4h)`.
7. `006430C0(this->[1Ch], 0, 0, 2)`, the self marker.
8. The interface manager's target, `[[00E198C4 + CCh] + 4Ch]`, gets `006430C0(target, 1, 0, 0)`
   when the camera unit exists, the target is neither null nor the self unit, `[target + 54h]`
   equals the local team record id `[[00E188A8 + 18CCh + team*4] + 28h]`, `00804350(target, 5)`
   passes and `005220C0([[00E198C4] + CCh], target)` passes. A local byte records that this
   happened; it suppresses the group pass later.
9. The squad list at `[[00E188A8 + 19CCh] + 16Ch]`, a singly linked list with next at `+4h` and
   payload at `+8h`. Each member and the controlled unit are pose-refreshed with `00414DB0` when
   their `+C8h` byte is clear, then the test at 006437DD:

   ```
   dx = self[FCh]  - mate[FCh]
   dy = self[100h] - mate[100h]
   dz = self[104h] - mate[104h]
   r  = (float)(int)mate[7C4h]
   add a marker when r*r > (dx*dx + dy*dy) + dz*dz
   ```

   The sum is built in exactly that order on the x87 stack, and the comparison is strict: `FCOMIP`
   followed by `JBE` drops the member on equality. Survivors get `006430C0(mate, 0, 0, 2)`.
10. The screen-centre world pick at 00643886. `[00E188A8 + 19FCh]` is passed to `00B6FDE0`, whose
    result carries the viewport width at `+10h` and height at `+14h`. Then
    `0043A290(this->[44h] * width, this->[48h] * height, 0.0f, &out)`. The enter virtual 00639480
    writes 0.5 into both `+44h` and `+48h`, so this is the crosshair. On success the three floats
    are written to `[00E198C4 + F0h]`, `+F4h` and `+F8h` and `[00E198C4 + FCh]` becomes 1;
    otherwise `+FCh` becomes 0.
11. The objective sweep over `[00E188A8 + team*4 + 21A4h]` through the container helpers 008DA180,
    008DA140, 008DA130 and 008DACD0, with a nested list at `[objective + 20h]/[+24h]`. Each
    surviving objective is filtered by `00645180([[00E198C4] + 40h], objective->virtual[140h])`,
    its position read with `008DAD30` and placed by `00643360(this, position, objective, kind)`.
12. The command-unit sweep over `[[00E188A8 + 19CCh] + 58h]`, guarded by `[00E188A8 + 1FE4h]` and
    `this->[24h]`. Each unit passes the same four byte filters as the HUD root's pending handle,
    then kind probe 8 (with an extra `00CF1430 = -4.0` height test against `unit[100h]`), then
    `[unit + 1ACh] != 8` and `!00927F10(unit, [unit+1ACh])`, then `[unit+1ACh]` differing from the
    local team index, then kind probe 0Fh with a `[[unit+9D4h] + 3D0h] == unit` leader test, then
    `00804350(unit, 5)` and `005220C0`. Survivors go to `00642C20(this, unit)`.
13. The target group at 00643BB9. `this->[20h] = 0`; with a controlled unit and no target already
    marked, `00523020([[00E198C4] + CCh], &out)` supplies the target into `+20h`. When it passes
    kind probe 0Fh, the group object at `[target + 9D4h]` is walked: the member count is at
    `+3CCh` and the members at `+3D0h`, capped at index 4 (a sixth or later member reads as null).
    Every member other than the target gets `006434E0(member, 1)`; the target itself gets
    `006434E0(target, 0)`.
14. `00640D70(this)`, then `00640620(this)` again, then the marker set is erased.

Between 13 and 14 sits the reinforcement sweep at 00643C93, gated on `this->[24h]` and on
`[00F88A00] > 00CEB690` (0.03): it walks `[008053C0([[00E188D8] + 54h]) + DE8h]` and, for each
entry that passes kind probe 18h, adds a marker per group member at `+3D0h` (again capped at
index 4), or one marker for the entry itself.

The marker kinds are the trailing arguments of `006430C0(unit, a, b, kind)`: `(0, 0, 2)` for the
controlled unit and squad mates, `(1, 0, 0)` for the interface target.

## Slot 35h, the minimap update 005C0F20

### Cadence and gate

No cadence counter either. `005BD420(this)` runs before any test. The routine then splits on
`this->[F4h]`, the selected capture point:

- **`+F4h` non-null (005C0F59).** `+169h` is cleared, `this->[60h]` is shown through virtual
  `+34h(1)`, `+16Ch += delta`, and `this->[54h]` gets virtual `+88h([[+F4h] + 7ACh], 0, 1.0f)`.
- **`+F4h` null (005C1289).** `this->[60h]` virtual `+38h()` decides whether anything animates. On
  the first such frame (`+169h` clear) `+16Ch` is reset to 0 and `+169h` is set, so the fade-out
  restarts its own clock.

Both branches then run the same animation block, and both fall through to the unit-icon pass at
005C154E, which repeats the markers screen's team gate (`[00E188A8 + 18ECh]` in `0..7`) and adds a
camera-unit test.

### The pulse

```
pulse = sinf(00F876A4 * 4.0)                              ; 00D7A328
alpha = 0.5 * pulse + 0.5                                 ; 00D7A280 twice
this->[54h]->virtual[50h]({1, 1, 1, alpha})               ; 00D7A24C is the 1
ramp  = min(this->[16Ch] * 4.0, 1.0)                      ; 00D7A328, then FCOMIP against FLD1
scale = ramp * (pulse * 0.1 + 1.0)                        ; 00D7A3A0 and 00D7A210
this->[54h]->virtual[48h]({scale, scale})
ring  = 006F1F90(this->[F4h])
this->[58h]->virtual[48h]({ring, ring})
this->[5Ch]->virtual[48h]({ring, ring})
this->[1ACh]->virtual[34h](1)
```

`FSIN` is used directly, so the pulse is an x87 sine of the raw mission clock in seconds; there is
no range reduction and no lookup table.

### The unit icons

`004B4B00()` supplies the camera unit and `00432650()` a world descriptor whose `+6Ch` and `+70h`
are the map half-extents; both are squared once at 005C15A0. The camera unit is pose-refreshed
through `00414DB0` when `+C8h` is clear and its position read from `+FCh`, `+100h` and `+104h`.
`00927880([00E188D8])` supplies the unit the self icon represents.

The team unit list is `[[[00E188A8 + team*4 + 18CCh] + 30h] + E0Ch]`, a singly linked list with
next at `+4h` and payload at `[node+8h] + 4h`. Each unit must have `+5Ch` set and `+5Dh`, `+60h`
and `+5Eh` clear, must not be the camera unit, and must pass virtual `+5Ch(5)` and virtual `+B8h`.
Units other than the self unit are then culled: `00427EB0(unit)` returns an icon-space float3, the
per-axis differences against the camera position go through `00427E30`, and the icon is dropped
when the squared half-extent is smaller than that result.

Survivors are looked up in the per-unit icon map at `this + F8h` (`005BE110`); a miss allocates
0Ch bytes, constructs the entry with `005BD590(entry, unit)`, inserts it with `005C0700` and stores
the pointer. `00694A60(unit, this + 8h)` then attaches it.

### Icon placement

All three `BSP_GuiWidget_SetLocalPositionAndBounds` sites (005C1C99, 005C2269, 005C25ED) build the
same shape and differ only in the depth literal:

```
pos.x = world_dx * 00CEDAE8          ; 1/1024
pos.y = -world_dz / 00CE42B0         ; 768
pos.z = <depth>
```

The two axes do not share a divisor, so the minimap is anisotropic: 1024 units across and 768 down,
a 4:3 authoring space. The depth literals are -1 (00D7A260, the default icon), -4 (00CF1430, the
controlled unit's icon), -2 (00CE7D7C, a highlighted icon), -8 (00CE3CC8, the capture icons) and
-10 (00CE6848, the marker icons).

Icon rotation is `00CE3830 - heading`, that is `pi/2` minus the heading (005C1CBC, `FSUBR`), where
the heading comes from the renderer at `[00E188A8 + 19FCh]`: bit 1 of `+5Ch` gates a refresh through
`00B6DB70`, then `+110h` and `+118h` go through the CRT helper 00BF701A (an `atan2`-shaped x87 call
taking two loaded floats) and the result is divided into 00CF1440 (80.0) for the compass at
`this->[4Ch]`.

## Callers and callees

The three routines are reached only through their screens' vtables (`+20h`), which the in-mission
interface manager drives; see `docs/IN_MISSION_INTERFACE_MANAGER.md`. Their callees are listed
above; the ones this packet identified but did not read in full are 006430C0, 006434E0, 00640620,
00640D70, 00643360, 00642C20, 00645600, 00648060, 006485A0, 00644CC0, 00644C20, 00644DB0,
00648C20, 00647080, 005BE110, 005BD590, 005C0700, 005BCF70, 005C0450, 005C0930 and 005C0BD0.

## Uncertainties

- `00ABE6E0`'s four float fields are established as `+F4h`, `+F8h`, `+100h`, `+104h` with a
  re-emit through virtual `+7Ch`, but the widget class that owns them and the meaning of the four
  components were not established. The call site passes `(fill, 0, 0, 1)`.
- `00E197C4..00E197D8` is written as two overlapping rectangles. Whether it is one rectangle plus
  a duplicate or two distinct rectangles was not settled; no reader was traced.
- `00F88A00`, the float gating the reinforcement sweep against 0.03, was not identified.
- The five arguments of `008E9AF0` are established by position; only three of the five output
  vectors are read by 00649860, so the roles of `+134h` and `+144h` are unknown.
- The minimap's three tail sections (005C1F90 onward, 005C239E onward and 005C2523 onward) were
  read only far enough to identify their placement calls and their depth literals. Roughly 40% of
  005C0F20's 6332 bytes were read instruction by instruction.
- No localisation key is resolved by any of the three routines. Every string that moves is a
  texture name inside a power-up record.

## What remains

- The marker pool itself: 006430C0, 006434E0, 00640620, 00640D70 and 00643360 decide which icon a
  marker gets and where it is drawn. That is the bulk of what a player sees on slot 4Dh.
- The minimap's capture-point and objective icon passes, and the per-unit icon entry type the
  0Ch-byte allocation at 005C171A creates.
- `00644DB0` (1195 bytes) and `00648C20` (3103 bytes), the HUD root's per-unit row and medal
  updates, which are larger than 00649860 itself.

### Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `hud_marker_pool_icons` | 006430C0, 006434E0, 00640620, 00640D70, 00643360, 00642C20 | docs/HUD_MARKER_POOL_ICONS.md, reports/hud_marker_pool_icons.json, include/bsp/hud_marker_pool.hpp, src/hud_marker_pool.cpp | Which icon each marker kind gets, how the pool recycles entries and how a marker is projected into the clip rectangle at 00E197C4 |
| `hud_root_unit_rows` | 00644DB0, 00644CC0, 00644C20, 00648C20, 00647080 | docs/HUD_ROOT_UNIT_ROWS.md, reports/hud_root_unit_rows.json, include/bsp/hud_root_rows.hpp, src/hud_root_rows.cpp | The `unit_name_Text`, `unit_HP_*_Icon`, `Medal_Icon`, `Medal_Text` and `WeaponInfo_Text` writers, and the closed-HUD toggle |
| `hud_minimap_icon_entries` | 005BE110, 005BD590, 005C0700, 005BCF70, 005C0450, 005C0930, 005C0BD0 | docs/HUD_MINIMAP_ICON_ENTRIES.md, reports/hud_minimap_icon_entries.json, include/bsp/hud_minimap_entries.hpp, src/hud_minimap_entries.cpp | The 0Ch-byte per-unit icon entry, the map at screen+F8h and the six team groups the layout virtual binds |
| `hud_powerup_manager` | 008E9AF0, 008E62A0, 008E4680, 008E3680, 00F88C30 | docs/HUD_POWERUP_MANAGER.md, reports/hud_powerup_manager.json, include/bsp/powerup_manager.hpp, src/powerup_manager.cpp | The power-up manager at 00F88C30: the 38h-byte per-team records, the sixteen scope lists at +84h and the five vectors 008E9AF0 fills |

## Ghidra state

- **Ghidra's disassembly of 006435D0 desynchronises at 00643C0C.** The listing prints
  `TEST AL,0xD4` / `OR dword [EAX],EAX` / `ADD byte [ECX],BH` / `MOV EBP,0x3CC`. The bytes are
  `8B A8 D4 09 00 00` then `39 BD CC 03 00 00`, that is `MOV EBP,[EAX+9D4h]` then
  `CMP [EBP+3CCh],EDI`. Everything after 00643C18 resynchronises. The doc above uses the byte
  decode; a re-disassembly of that range would fix the listing.
- No `bsp.py ghidra flow` gap was found in any function this packet read.
- Every routine this packet names has a Ghidra function; `no_ghidra_function` in the report is
  empty.

## Reconstruction state

| Routine | State |
| --- | --- |
| 00649860 | reconstructed and build-tested (`hud_root_screen_update`) |
| 006435D0 | reconstructed and build-tested (`hud_markers_screen_update`) |
| 005C0F20 | partly reconstructed and build-tested: the animation block and the unit-icon pass; the rest analyzed |
| 00AB64C0, 00ABE6E0 | analyzed in full |
| 008E62A0 | analyzed in full |
| 006463E0 | re-read only for the two clone template fields |
| The clip rectangle, the pulse, the row layout, the icon placement | reconstructed as pure functions and build-tested |
| 008E9AF0 and the marker pool routines | referenced only |

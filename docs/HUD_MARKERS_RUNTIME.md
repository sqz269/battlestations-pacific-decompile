# HUD unit markers runtime (packet `cc_hud_minimap`)

Addresses: 006435D0, 006430C0, 006434E0, 00640620, 00640D70, 00642040, 00642B80, 00642C20,
00642960, 00641910, 00641E30, 00643360, 0063A6C0, 0063D1E0, 00638E50, 0043A660, 0043A290,
00639990, 0043F080, 0063ABD0, 004323D0, 006374B0, 00427EB0, 00427E30, 00AA1FE0, 00AA7DC0,
00AA7D00, 00E197C4, 00E197C8, 00E197CC, 00E197D0, 00E197D4, 00E197D8, 0109CF04.

The markers screen is HUD screen slot 4Dh; its page is `GUI_markers`
(`interface/gui_markers.lua`, registered by 0063E280, see docs/HUD_SCREEN_PAGES.md). Its update
virtual 006435D0 and its frame order are documented in docs/HUD_CENTRAL_UPDATES.md; this packet
reads the pool underneath it. The minimap half of the packet is docs/HUD_MINIMAP.md.

Names here are hypotheses, not recovered symbols.

## The projection chain

Three routines separate cleanly, and only the first touches the camera.

**`0043A660` `BSP_Camera_ProjectWorldToScreen`, `__fastcall(ECX=const float3* world,
EDX=float3* out, int mode, char clampDepth)`, `RET 8`, body 0043A660..0043A869.** It copies the
three floats plus a 1.0 (00D7A24C) into a vec4, asks 00B70490 for the view-projection matrix with
`ECX = [00E188A8 + 19FCh]` (the renderer), transforms with 00B62D10, and divides clip.xyz by
clip.w. `out[0..2]` receives the NDC triple before the mode step. The return value is a clip mask
built only when `1/clip.w > 0` (00D7A218):

| Bit | Condition |
| --- | --- |
| 1 | `-1.0 <= ndc.x < 1.0` (00D7A250 is a double -1.0, 00D7A24C a float 1.0) |
| 2 | `-1.0 <= ndc.y < 1.0` |
| 4 | `0.0 <= ndc.z` and, when `clampDepth` is set, `ndc.z < 1.0` |

The mode then rewrites `out.x` and `out.y`:

| mode | out.x | out.y |
| --- | --- | --- |
| 0 | `(ndc.x + 1) * 0.5` | `(1 - ndc.y) * 0.5` |
| 1 | `(ndc.x + 1) * 0.5` | `(1 - ndc.y * guiExtent.y) * 0.5`, extent from 00AA1FE0+4h |
| 2 | `(ndc.x + 1) * 0.5 * 1024` (00CE42B8) | `(1 - ndc.y) * 0.5 * 768` (00CE42B0) |
| 3 | `(ndc.x * [renderer+1C8h] + 1) * 0.5` | as mode 1 |

`out[2]` keeps `ndc.z` in every mode. The x87 sequence at 0043A6D5..0043A7A5 was read instruction
by instruction; the pseudocode loses the divide order.

**`00638E50` `BSP_HudMarker_ProjectAndClip`, `__fastcall(ECX=const float3* world,
EDX=float3* outScreen) -> bool`, `RET`, body 00638E50..00638F41.** This is the only projection the
marker pool uses. It calls the above with `mode = 1` and `clampDepth = 0`, keeps
`fully = ((flags & 7) == 7)`, and then splits on the byte `[0109CF04 + 0Dh]`:

- byte set: `out.x = 0.5 + (out.x - 0.5) * 1.3333334` (00CF5750, a double 4/3). When `fully` it
  goes straight to the y test; otherwise it requires `(flags & 6) == 6` and
  `00E197D0 < out.x < 00E197CC`.
- byte clear: `fully` is required, and `00E197D8 < out.x < 00E197D4`.

Both paths then require `00E197C8 < out.y < 00E197C4` and return that conjunction.

**`0043A290` `BSP_Camera_PickWorldPointOnHorizontalPlane`** is the inverse used once per frame at
00643886: it unprojects the crosshair through 00439F50 and intersects the ray with the plane
`y == planeY` (0.0f from 006435D0), publishing the hit at `[00E198C4 + F0h..+F8h]` with the valid
flag at `+FCh`.

### The clip rectangle is one rectangle, not two

docs/HUD_CENTRAL_UPDATES.md left open whether `00E197C4..00E197D8` is one rectangle plus a
duplicate or two distinct rectangles, because no reader had been traced. 00638E50 is the reader.
It uses `00E197D0`/`00E197CC` on the aspect-corrected path and `00E197D8`/`00E197D4` on the
uncorrected path, and 006435D0 writes the same `0.5 -+ width * 0.4925` into both pairs. It is one
rectangle whose x bounds are stored twice so each path can read its own pair; y has a single pair.

## The marker pools

`00640620` `BSP_InGameHudMarkers_ResetMarkerPools`, `__fastcall(this)`, body
00640620..0064079B, releases whatever the previous frame did not reuse. Four pools, each a
pointer vector plus a per-frame cursor:

| Pool | Vector header | first / last | Cursor | Release of entries at or past the cursor |
| --- | --- | --- | --- | --- |
| A | this+4Ch | +50h / +54h | +7Ch | `006374B0(entry)` hides every widget the entry owns |
| B | this+5Ch | +60h / +64h | +80h | `SetVisible(0)` on `entry+4h`, `+8h`, `+Ch`, `+14h`, `+10h` |
| C | this+6Ch | +70h / +74h | +84h | `SetVisible(0)` on `entry+4h` |
| D | this+90h | +94h / +98h | +A0h | virtual 0 with argument 1, then the slot is nulled |

Pool D is then truncated with `0063FFC0(this+90h, [this+A0h], 0)`. The routine zeroes `+7Ch`,
`+80h` and `+84h` on the way out but not `+A0h`; 006435D0 zeroes `+A0h` itself as its first act,
so the opening call of the frame tears pool D down completely and the closing call at step 14
keeps exactly the entries the frame used.

`006374B0` `BSP_HudMarkerEntry_HideAllWidgets` clears `entry+4h` and `entry+5h` and then calls
`SetVisible(0)` on the 24 widget pointers from `entry+8h` in six rounds of four, and on
`[entry+68h]`. Six rounds of four matches the six colour variants of the page's icon sets.

## Which unit gets a marker: `006430C0`

`BSP_InGameHudMarkers_AddUnitMarker`, `__thiscall(this, UnitInstance* unit,
char forceObjectiveColour, int kindA, int kindB)`, `RET 10h`, body 006430C0..0064335D. Every
caller in 006435D0 goes through it. `unit->vtable[5Ch]` is `IsKindOf(classId)` (006FE530, see
docs/UNIT_INSTANCE_UPDATE.md).

Rejection, in order:

1. `unit == null`.
2. `IsKindOf(6)` and `BSP_UnitInstance_GetPartsObject(unit)[14h] == 1`.
3. `IsKindOf(46h)`.
4. `IsKindOf(45h)`.
5. `0063ABD0(this, unit)` already has the unit in this frame's vector at `this+34h`.

Otherwise the unit is pushed into that vector through `004323D0` and three flags are derived:

```
isSelf     = (unit == this[1Ch])                                          ; 00643136
isFriendly = ([unit+54h] == [[00E188A8+18CCh + team*4] + 28h])            ; 0064315B
isTarget   = (unit == this[20h])                                          ; 0064316A
inObjective= 008DDF90([00E188A8 + 21A4h + team*4], unit)                  ; 00643183
if (isFriendly && inObjective && forceObjectiveColour == 0) isFriendly = 0 ; 006431A1
```

`team` is `[00E188A8 + 18ECh]`. A last gate, `0043F080(unit)`
(`BSP_UnitInstance_IsAliveAndVisible`: `[unit+5Ch] != 0 && [unit+5Dh] == 0 && [unit+60h] == 0 &&
[unit+5Eh] == 0`), drops the unit before any widget work.

The dispatch is a chain of `IsKindOf` probes:

| Probe | Call | Site |
| --- | --- | --- |
| 18h | `00642960(unit, isSelf, isFriendly, isTarget, 2)` | 006431D9 |
| 1Ah | `00642B80(unit, isSelf, isFriendly, isTarget, 2)` | 00643206 |
| 1Bh, and the `vtable[140h]` subobject answers 45h or 46h | `00642040(unit, isSelf, isFriendly, isTarget, 1, 2)` | 0064326B |
| 1Ch | `00641910(unit, isSelf, isFriendly, isTarget \|\| forceObjectiveColour)` | 006432A9 |
| 5 | `00642040(unit, isSelf, isFriendly, isTarget, kindA, kindB)` | 006432DB |
| 1 | `0063F1E0(this+A4h, &scratch, &subobject)`, a `std::set` insert | 00643311 |

After the dispatch, `IsKindOf(1Eh)` or `IsKindOf(1Fh)` with `[unit+54h]` different from the local
team record id calls `00641E30(this, unit)`.

A ship answers `IsKindOf(5)` and `IsKindOf(6)` (docs/GAME_EXECUTABLE.md), so the mission's
destroyers take probe 2 first, survive it whenever the parts object byte is not 1, and land on
`00642040` through probe 5 with `kindA`/`kindB` straight from the caller, that is `(0, 2)` for the
controlled unit and squad mates and `(0, 0)` for the interface target.

Argument counts are from the cleanups: `006430C0` ends `RET 10h`, `00642040` ends `RET 18h`,
`0063A6C0` ends `RET 24h`.

## Where a marker lands on screen: `0063A6C0`

`BSP_HudMarker_ProjectUnitScreenBounds`, `__thiscall(this, float3 anchorWorld by value,
UnitInstance* unit, char* outCollapsed, float* outMinX, float* outMinY, float* outMaxX,
float* outMaxY)`, `RET 24h`, body 0063A6C0..0063ABCD. This is the pure part of marker placement.

1. `*outMinX = *outMinY = 1e10` (00CE4970), `*outMaxX = *outMaxY = -1e10` (00CE4ADC),
   `*outCollapsed = 0`.
2. Pose refresh through 00414DB0 when `[unit+C8h]` is clear; the world matrix rows are
   `unit+CCh` (right), `unit+DCh` (up), `unit+ECh` (forward), translation `unit+FCh`
   (see docs/ENTITY_LOCAL_MATRIX.md).
3. For `IsKindOf(6)` without `IsKindOf(8)`, the anchor is raised by `unit[538h][A8h]` along the up
   row and the up row is then doubled (00D7A308 is 2.0). `unit[538h]` is the unit's class record
   and `+A0h`, `+A4h`, `+A8h` are its three extents.
4. A triple loop over `i, j, k` in `{-1, +1}` (`ADD reg,2` / `CMP reg,1` / `JLE`) builds the eight
   corners `anchor + 0.5*i*[538h+A4h]*right + 0.5*j*[538h+A8h]*up + 0.5*k*[538h+A0h]*forward`,
   projects each with `00638E50`, and accumulates min/max of the projected x and y.
5. A box narrower than `this[88h] / 1.5` (00CE3D78) is widened about its centre by 00CEC9E0
   (-0.5); the y axis uses the same margin multiplied by 1.33 (00CF5780). The enter virtual
   00639480 writes `this[88h] = 0.023f`.
6. When both axes had to be widened, the anchor alone is reprojected, all four bounds collapse to
   that point and `*outCollapsed` becomes 1.

## What a marker writes: `0063D1E0`

`BSP_HudMarker_WriteUnitMarkerWidgets`, body 0063D1E0..0063DEBD. It hides the screen widget slots
at `this+68h`, `+6Ch`, `+70h`, `+74h`, `+78h`, `+7Ch` and `+80h`, then binds the per-marker
children by name through `BSP_GuiWidget_FindChildByName`:

| String | Address | Length pushed to `BSP_NativeString_Resize` | Page node |
| --- | --- | --- | --- |
| `Unit_name_Text` | 00CF58CC | 0Eh | `sidemarker_Group.Unit_name_Text`, font Arial15 |
| `Distance_Text` | 00CF58BC | 0Dh | `sidemarker_Group.Distance_Text` |
| `HP_Icon` | 00CF58B4 | 7 | `sidemarker_Group.HP_Icon`, `gui/HUD/hp_health.tga` |
| `HP_BG_Icon` | 00CF58A8 | 0Ah | `sidemarker_Group.HP_BG_Icon`, `gui/HUD/hp_bg.tga` |
| `type_Icon` | 00CF57B4 | 9 | `sidemarker_Group.type_Icon`, 34 texture states |
| `commandbuilding_Icon` | 00CF5818 | 14h | `commandbuilding_Group.commandbuilding_Icon`, 3 states |

Position goes through `BSP_GuiWidget_SetLocalPositionAndBounds` (00AA7DC0) and
`BSP_GuiWidget_SetResolvedPosition` (00AA8240), the label through
`BSP_GuiText_SetLocalisedSource` and `BSP_GuiText_SetLocalisedCString`, and sizes are read back
with `BSP_GuiWidget_GetSize`. The colour variant comes from `00639990`
(`BSP_InGameHudMarkers_ResolveMarkerColourIndex`): 1 when `[unit+54h] == 0`, 0 when it is 1, 5
otherwise, and 2, 3 or 4 for objective units by `008DD240(unit)[18h]`. Six values, matching the
page's six colour sets (`CLOSE_Group` holds `Blue_Icon`, `Coffe_Icon`, `Red_Icon` and
`Unknown_Icon`, each with close, lock and far states).

## The target section callout: `00640D70`

`BSP_InGameHudMarkers_UpdateTargetSectionCallout`, `__fastcall(this)`, body 00640D70..00641903.
It runs only with a target at `this[20h]` whose parts object byte `+14h` is not 1. It reads the
eye point from the renderer at `+120h..+128h`, walks the target's part records through 00476B90
and the 2Ch-byte entries at `[part+3Ch]`, and for every entry whose kind `[entry+4h]` is 5, 6 or 8
projects the eight corners of the AABB at `[entry+20h]`/`[entry+24h]` with
`BSP_Vector3f_TransformAffinePoint` and `00638E50`, keeping the entry whose projected centre is
nearest the screen centre (00CE3800 is 0.5). The winner's kind maps to a localisation key:

| Kind | Key |
| --- | --- |
| 8 | `ingame.sections_magazine` |
| 5 | `ingame.sections_engine` |
| 6 | `ingame.sections_fuel` |

The chosen box is reprojected and emitted as four corner points through
`0063C620(corners, 3, 0, 1, key, 0)`.

## Host contract for the rebuilt executable

The executable's mission frame already drives the HUD pages; to put the destroyers on screen as
markers it must supply the calls below, in this order, once per frame from the slot 4Dh update.
`address` is the native call site and `native` the callee.

| # | Host method | address | native | Containing function | Owner area |
| --- | --- | --- | --- | --- | --- |
| 1 | `gui_extent()` | 00643616 | 00AA1FE0 | 006435D0 | GUI |
| 2 | `camera_unit()` | 0064368B | 004B4B00 | 006435D0 | controlled unit |
| 3 | `pool_reset()` | 00643695 | 00640620 | 006435D0 | this packet |
| 4 | `clear_marked_set()` | 006436A3 | 0063BCD0 | 006435D0 | this packet |
| 5 | `displayed_self_unit()` | 006436B7 | 00927880 | 006435D0 | controlled unit |
| 6 | `add_unit_marker(self, 0, 0, 2)` | 006436E5 | 006430C0 | 006435D0 | this packet |
| 7 | `refresh_pose(unit)` | 006437C2 | 00414DB0 | 006435D0 | entity matrix |
| 8 | `viewport_descriptor()` | 00643892 | 00B6FDE0 | 006435D0 | renderer |
| 9 | `pick_crosshair_world_point()` | 006438E8 | 0043A290 | 006435D0 | camera |
| 10 | `add_objective_marker()` | 00643A58 | 00643360 | 006435D0 | mission objectives |
| 11 | `add_command_unit_marker(unit)` | 00643BA9 | 00642C20 | 006435D0 | this packet |
| 12 | `add_group_member_marker(unit, flag)` | 00643C72 | 006434E0 | 006435D0 | this packet |
| 13 | `target_section_callout()` | 00643C8E | 00640D70 | 006435D0 | this packet |
| 14 | `pool_reset()` again | 00643D41 | 00640620 | 006435D0 | this packet |
| 15 | `parts_object(unit)` | 006430E4 | 0080E490 | 006430C0 | unit instance |
| 16 | `already_marked(unit)` | 0064311C | 0063ABD0 | 006430C0 | this packet |
| 17 | `mark_unit(unit)` | 00643131 | 004323D0 | 006430C0 | container |
| 18 | `objective_container_contains(unit)` | 00643183 | 008DDF90 | 006430C0 | mission objectives |
| 19 | `is_alive_and_visible(unit)` | 006431A8 | 0043F080 | 006430C0 | unit instance |
| 20 | `build_unit_marker(...)` | 006432DB | 00642040 | 006430C0 | this packet |
| 21 | `marker_colour_index(unit)` | 006420D8 | 00639990 | 00642040 | this packet |
| 22 | `screen_bounds(...)` | 00642211 | 0063A6C0 | 00642040 | this packet |
| 23 | `unit_class_name(unit)` | 006422F0 | 00803DC0 | 00642040 | unit instance |
| 24 | `unit_health(unit)` | 0064240B | 0077A2F0 | 00642040 | unit damage |
| 25 | `write_marker_widgets(...)` | 006426E6 | 0063D1E0 | 00642040 | this packet |
| 26 | `project_and_clip(world, out)` | 0063A9D6 | 00638E50 | 0063A6C0 | this packet |
| 27 | `project_world_to_screen(...)` | 00638E58 | 0043A660 | 00638E50 | camera |
| 28 | `view_projection_matrix()` | 0043A6A0 | 00B70490 | 0043A660 | renderer |
| 29 | `transform_vec4(v, m)` | 0043A6AF | 00B62D10 | 0043A660 | math |
| 30 | `gui_extent()` for mode 1 | 0043A7BB | 00AA1FE0 | 0043A660 | GUI |
| 31 | `set_local_position(widget, xyz)` | 0063D3AF | 00AA7DC0 | 0063D1E0 | GUI |
| 32 | `find_child(widget, name)` | 0063D42A | 00AA7E00 | 0063D1E0 | GUI |
| 33 | `set_localised_text(widget, key)` | 0063D694 | 00ABAED0 | 0063D1E0 | GUI |
| 34 | `widget_size(widget)` | 0063D7FD | 00AA6740 | 0063D1E0 | GUI |
| 35 | `set_resolved_position(widget, xyz)` | 0063D8B9 | 00AA8240 | 0063D1E0 | GUI |
| 36 | `set_local_xy(widget, x, y)` | 0064304C | 00AA7D00 | 00642C20 | GUI |

Steps 1 through 6, 14 through 22 and 26 through 32 are the minimum for a marker to appear at all;
23, 24, 33 and 35 only fill the label, the health bar and the resolved layout.
`unit->IsKindOf(id)` is reached through vtable slot `+5Ch` at 006430DC, 006430FC, 0064310F,
006431BF, 006431EC, 00643219, 0064327E, 006432B9 and 006432EB; those sites are indirect and are
recorded as `006FE530+vtable5C` in the report.

## Corrections

| Was | Is | Evidence |
| --- | --- | --- |
| docs/HUD_CENTRAL_UPDATES.md: "`00E197C4..00E197D8` is written as two overlapping rectangles. Whether it is one rectangle plus a duplicate or two distinct rectangles was not settled; no reader was traced." | One rectangle with duplicated x bounds. 00638E50 reads 00E197D0/00E197CC on the aspect-corrected path and 00E197D8/00E197D4 otherwise, and 006435D0 writes the same value into each pair. | 00638E9F, 00638EAD, 00638ED0, 00638EE0, 00638F08, 00638F18 |
| docs/HUD_CENTRAL_UPDATES.md: "`00427EB0(unit)` returns an icon-space float3." | It returns `unit+FCh`, the world-matrix translation row, after a lazy pose refresh. | 00427EB0..00427EC8: `CMP byte [ESI+C8h],0` / `CALL 00414DB0` / `LEA EAX,[ESI+FCh]` |
| docs/HUD_CENTRAL_UPDATES.md: "`004323D0` is its `push_back`" for the vector at `+34h`, listed among 006435D0's own steps. | Correct, but the call in 006430C0 at 00643131 passes the address of the incoming `unit` argument slot, which MSVC then reuses as the `isFriendly` byte. Reading that slot later as a dword is the `bool` passing convention, not a stale pointer. | 00643129..00643131, 0064315E, 006431C9 |

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `hud_marker_close_icons` | 00641910, 00639990, 0063D1E0, 006377E0 | docs/HUD_MARKER_CLOSE_ICONS.md, include/bsp/hud_marker_close.hpp | The `IsKindOf(1Ch)` path and the `CLOSE_Group` close/lock/far states per colour |
| `hud_marker_type_icon_states` | 0063D1E0 from 0063D700, `type_Icon` 34 states | docs/HUD_MARKER_TYPE_ICONS.md | Which class id selects which of the 34 `common/icons/class/*_s.tga` states |
| `hud_marker_box_emit` | 0063C620, 006407A0, 00640A20, 00640860, 00640940 | docs/HUD_MARKER_BOX_EMIT.md | The four-corner box marker emitter shared by the section callout and the group marker |
| `hud_marker_objective_pass` | 00643360, 008DA180, 008DA140, 008DACD0, 008DAD30 | docs/HUD_MARKER_OBJECTIVES.md | The objective sweep and `00643360(this, position, objective, kind)` |

## no_ghidra_function

| Start | End (inclusive) | Evidence |
| --- | --- | --- |
| 00643C1C | 00643C68 | A hole inside 006435D0's own body. `bsp.py ghidra proto 006435d0` reports `body 006435D0 - 00643D96` and the stored listing decodes every instruction from 00643C1C to 00643C67, but the bridge's `get_function_by_address` answers "No function found" for all 24 instruction starts in that range and answers 006435D0 for every start at 00643C18 and below and at 00643C69 and above. The hole opens one instruction after 00643C18, the end of the desynchronised range docs/HUD_CENTRAL_UPDATES.md records at 00643C0C, so it is a leftover of that repair rather than a new defect. |

Every other address this packet names has a Ghidra function; `bsp.py ghidra proto --brief`
reported a body range for each, and `bsp.py ghidra flow 006435d0` reports 565 listed instructions
and zero gaps, which is why the hole shows up only in the address set and not as a flow gap.

The block inside the hole is the target-group member loop of docs/HUD_CENTRAL_UPDATES.md step 13:
the count at `[target+3CCh]` and the members at `[target+3D0h]`, the index capped at 4 by
`CMP EDI,4` / `JA 00643C35` (a sixth or later member reads as null), `006434E0(member, 1)` at
00643C42 for members other than the target and `004323D0` at 00643C54 for the rest. The
reconstruction and the host table use 00643C72, the same step at a site Ghidra covers; an
integrator can close the hole with `python tools/ghidra_define_function.py` or a targeted
re-disassembly of 00643C1C..00643C68 before re-running the annotate step.

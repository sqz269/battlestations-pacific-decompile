# Main-menu map point geometry

Addresses: 004215D0 0041C7B0 005803E0 00588C70

`00588C70` positions a mission group's point/flag pairs around the selected
mission, moves and scales the backdrop, then moves the widget at screen+338h
to the selected point's resolved position with a further -1 z offset. It has
one caller function, `00599DB0`, with calls at `0059A3CB` and `0059A678`.
This packet reconstructs that sequence and its float3 interpolation helper.
It describes the native 12-byte vector append as a library contract.

All descriptive names are reviewed hypotheses, not recovered symbols.
Ghidra reads used the verified project/program wrapper for
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. No Ghidra mutations
were made. The three primary routines already have stored functions.

## ABI and list contract

| Routine | Listing evidence | Original ABI |
| --- | --- | --- |
| 004215D0 | 004215D7..00421612 checks size/capacity in 12-byte units; 00421633 constructs one element; 00421638..0042163B advances last by 12 | ECX=vector, stack=source element pointer, RET4; no consumed return |
| 005803E0 | 005803E5 copies ECX to EAX; 005803EA/0058041F load the two stack pointers; final instruction 0058048E | ECX=output XYZ, EDX=old XYZ, stack=(target XYZ pointer, fraction pointer), RET8; EAX=output pointer |
| 00588C70 | argument loads at 00588C96, 00588CCB, 00588D60, 00588DD5, 00588F8A; final instruction 00589428 | ECX=screen; six stack dwords; RET18h; return value unused |

The six stack arguments of `00588C70`, in order, are frame seconds (never
read), widget lookup root, zoom delta, point-list index, additional XYZ offset
pointer, and a flag in the last argument's low byte. Its caller supplies
screen+110h as root and screen+1A4h as offset. The body uses its actual supplied
root, rather than reloading screen+110h. Original signatures in Ghidra still
read `undefined(void)`; pseudocode's inferred argument list is not its saved ABI.

`004215D0` retains its existing `stl_probable` classification. Native vectors
have first/last/capacity at +4/+8/+Ch; +0 is library bookkeeping. The fast path
calls `0041C7B0` with destination=last, count=1 and the source pointer, then
advances last by 0Ch. The read-only decompile of that library callee copies
three dwords into each non-null destination. The full-capacity path checks
last >= first then delegates to `004206A0`; its allocator/growth/exception
policy is not reconstructed here. No replacement STL implementation is added.

The game owns five such lists at screen+134h + listIndex*10h. Existing mission
detail evidence at `0058C662..0058C6B3` establishes that entries are point-icon
resolved positions minus screen+124h..12Ch, appended once when the group's
list was initially empty. The geometry driver consumes x and y only; stored z
is not used for target or marker depth.

## Zoom and anchor

`00588C8B..00588D52` starts with interpolation fraction 0.2 (float at
00CE54A0). It enters the zoom block only when screen+19Ch or the supplied delta
is nonzero. Inside that block, fraction becomes 1 when the last-byte flag is
set or +19Ch is nonzero. The effective delta is +19Ch when nonzero, otherwise
the supplied delta. It stores `zoom + delta * double(float(0.1))` to float and
clamps ordered values to [0.5,1.5]. The double at 00D7A3A0 is exactly
0.10000000149011612. Unordered comparisons retain NaN. With both deltas zero,
zoom is not clamped and the last-byte flag does not force a snap.

Thus +19Ch is a carried zoom delta, not merely a Boolean smoothing gate. This
routine neither clears nor writes it. Its producer and reset remain open.

Zoom is processed before testing screen+330h. A null selected-point widget
skips all following geometry and widget calls but does not skip zoom.

`00588D60..00588E64` selects listIndex and bounds-checks global 00E194DC as an
unsigned selected-mission index. Let P be that list entry, O be screen+190h
plus the additional offset, and Z the updated zoom. Target is:

```
target.x = round32(round32(O.x) + round32(P.x * Z))
target.y = round32(round32(O.y) + round32(P.y * Z))
target.z = round32(round32(O.z) + 0.0)
```

`005803E0` interpolates each old screen+184h..18Ch component toward target:
round32(target-old), then round32(difference*fraction), then
round32(old+product). The final anchor is written before widget lookups.
Listing spills at 005803F9/00580411/0058042D and
00580445/0058044F/00580457 make this different from one fused lerp expression.
The new C++ uses Win32 x87 arithmetic with these store boundaries; no native
differential or exceptional FP-status equivalence has yet been established.

## Widget sequence and positions

Every iteration reloads the manager at 00E198AC, its mission tree at +5Ch and
published group index 00E194D8. The group array uses 34h-byte records; the
group's mission array uses 434h-byte records. That count controls the loop,
independently of the supplied point-list index. An invalid group or selected
point invokes 00BF6713. The projected host must enforce those preconditions.

Each iteration constructs one-based names and calls `00AA7E00` twice:
`mission_mappoint_<i+1>_Icon` at 00588F9C, then
`mission_mapflag_<i+1>_Icon` at 005890FE. Both lookups occur even if the first
misses. The last literal argument is 1, but the established `00AA7E00` contract
ignores it and searches direct children only. Nothing is created.

Only when both handles exist does the body range-check point i and move them.
With B=screen+124h..12Ch, A=the local interpolated anchor, and P=list[i],
the common position is:

```
R.x = round32(round32(P.x * Z + B.x) - A.x)
R.y = round32(round32(Z * P.y + B.y) - A.y)
R.z = round32(5.0 - A.z)
point = R + (0, 0, i == current_selected_index ? -1 : 0)
flag  = (R.x - float(0.003125), R.y - float(0.0069444445), R.z + 0)
```

The point move precedes the flag move (005892F0, 00589326). The selected index
is reloaded at 0058920A. Flag constants at 00CEFB88 and 00CEFB80 are doubles
holding exact promoted-float values 0.0031250000465661287 and
0.0069444444961845875. Flag depth does not receive the selected point's -1.
There are no visibility, alpha or point-state changes in this driver.

After the loop, including an empty group:

1. Move backdrop +328h to B-A (`00589378`, `00AA8240`).
2. Call its virtual+58h with zoom times the extent pair at +11Ch/+120h
   (`005893B5`). The extent pair is scaled, not the widget's current extent.
3. Query selected point +330h through `00AA6750` (`005893C5`).
4. Move +338h to that resolved position plus (0,0,-1) (`0058940B`). The exact
   layout name/role of +338h is not established by this packet.

## Integration and limits

The new module uses the existing `std::array<float,3>` convention from
`MissionDetailHost`, and reuses that header's name constants. The host exposes
distinct methods for each game call site; standard-library string temporaries
and native allocation/checking details are not transplanted.

The older `apply_map_zoom_00588c70` in `src/main_menu_screen.cpp` remains a
delta-only projection valid when carried delta is zero. A full adapter should
use this module's zoom-step rule and synchronize the projected screen zoom.
Do not apply another delta after `drive_map` if that call uses the new driver
against the same state: the existing update projection currently calls the
old helper after its abstract host call. Ownership of that integration is
with the primary agent; this packet does not edit its files.

Exported and listing-reviewed: all three primary routines. Reconstructed:
005803E0 value rule and 00588C70 typed host sequence. Library-contract-only:
004215D0. Source registration, standard Win32 build and existing checks are
pending primary integration. No new tests, native differential execution,
ABI-compatible replacement, live widget adapter or game validation is claimed.
Native SEH, string-pool failure paths, duplicate STL checks, malformed storage,
exceptional x87 status/trap timing and reentrant invalidation of borrowed
screen/list ownership are outside this interface.

## no_ghidra_function

None. Existing final instructions: 0042166C RET4 (3 bytes), 0058048E RET8
(3 bytes), 00589428 RET18h (3 bytes). No raw routine requires definition.

## Follow-up packets

- `main_menu_map_carried_delta`: identify writers and reset policy for
  screen+19Ch; this driver already takes its value explicitly.
- `main_menu_map_companion_binding`: identify +338h in the layout binder and
  connect the selected-point companion host operation to the real widget.
- `main_menu_map_runtime_adapter`: bind existing GUI position/extent APIs and
  mission-table projections, then validate map behavior on the installed page.

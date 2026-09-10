# Front-end screen scroll block

Addresses: 00683300 00683320 00683330 00683380 006833F0 00683440 006834A0 00683610 00683680 00683700 00683790 00683820 00683A40 00AA6740 00AA6750 00AA8240

## What the block actually is

The packet was named `front_end_screen_animation` from the follow-up entry in
docs/MULTI_MENU_SCREENS.md, which reads 00683610 as "a fade/slide animation
record". The listing does not support that. **00683610 is the constructor of a
vertical scroll controller**: a content widget that slides along Y, a scroll bar
thumb that tracks it, two optional arrow widgets that follow the thumb's alpha,
a pointer-drag path and a gamepad/axis auto-scroll path.

There is no phase, no duration, no easing curve and no frame delta anywhere in
the thirteen methods. The per-frame pump 00683820 takes a single `bool` and
applies the step in +24h once per call, so the scroll speed is frame-rate
dependent unless the axis the input manager reports is already time-scaled. The
only reason the block looked like an animation record from the pseudocode is
that its constructor zeroes a run of floats.

The whole class is contiguous at 00683300..00683A8F, immediately before
`BSP_FrontEndManager_InitDefault` (00683A90).

## Calling conventions and RET sizes

| Address | Convention | RET | Role |
| --- | --- | --- | --- |
| 00683300 | `__thiscall(this, int direction, float step, bool override)` | RET 0Ch | Push an auto-scroll direction and step, optionally suppressing the input read |
| 00683320 | `__fastcall(this)` | RET | End thumb drag: `MOV byte ptr [ECX+48h],0` |
| 00683330 | `__thiscall(this, float, float, float)` | RET 0Ch | Set the three widget alphas directly |
| 00683380 | `__thiscall(this, void* thumb, float track, void* arrow_start, void* arrow_end)` | RET 10h | Attach the scroll bar |
| 006833F0 | `__fastcall(this)` | RET | Re-capture both base positions |
| 00683440 | `__fastcall(this)` | RET | Push the thumb to the position the scroll offset implies |
| 006834A0 | `__thiscall(this, float range)` | RET 4 | Set the scroll range; resize, reposition and retint the bar |
| 00683610 | `__thiscall(this)` | RET | Constructor, no callees |
| 00683680 | `__thiscall(this, float delta)` | RET 4 | Scroll toward the start, clamped at 0 |
| 00683700 | `__thiscall(this, float delta)` | RET 4 | Scroll toward the end, clamped at -range |
| 00683790 | `__thiscall(this, void* content)` | RET 4 | Attach the content widget and reset the scroll state |
| 00683820 | `__thiscall(this, bool allow_pad_fallback)` | RET 4 | Per-frame pump |
| 00683A40 | `__fastcall(this)` | RET | Begin thumb drag |
| 00AA6740 | `__thiscall(widget) -> Vec2*` | RET | `LEA EAX,[ECX+20h]`, the widget's own size pair |
| 00AA6750 | `__thiscall(widget, Vec3* out) -> out` | RET 4 | Widget position resolved through the parent chain at +70h |
| 00AA8240 | `__thiscall(widget, const Vec3*)` | RET 4 | The inverse of 00AA6750 |

**Ghidra reads 00683330 wrong.** Its decompilation shows two arguments and
reuses the first for both the thumb and the first arrow. The listing reloads
from `[ESP+4]`, `[ESP+0Ch]` and `[ESP+10h]`, three distinct slots, because each
of the three virtual calls is a `__thiscall` that pops its own argument. RET 0Ch
confirms three arguments.

## Struct layout, 5Ch bytes

Evidence column: the address that writes or reads the field.

| Offset | Type | Field | Evidence |
| --- | --- | --- | --- |
| +00h | `void*` | content widget | `MOV [ESI],ECX` 006837A3; read 006839D9 |
| +04h | `void*` | first arrow widget | `MOV [ESI+4],ECX` 006833DA; alpha 00683592 |
| +08h | `void*` | second arrow widget | `MOV [ESI+8],EDX` 006833DD; alpha 006835C6 |
| +0Ch | `float` | scroll range | written 006834B1; gate `0 < x` 0068368E, 0068370B, 0068396D; gate `!= 0` 00683448 |
| +10h..+18h | `float[3]` | content base position | 006837AC..006837B8 from 00AA6750 |
| +1Ch | `float` | scroll offset, in [-range, 0] | 006836A2, 00683729, 00683994; read 00683461 |
| +20h | `int` | auto-scroll direction 0/1/2 | 006838D2, 006838F2; switch 006839CC |
| +24h | `float` | auto-scroll step | 006838ED; consumed 00683A16, 00683A2B |
| +28h | `void*` | scroll bar thumb widget | `MOV [ESI+28h],EDI` 00683398 |
| +2Ch | `float` | thumb travel | 006833A0 then narrowed 0068351B; read 00683467, 00683949 |
| +30h | `float` | track length minus minimum thumb height | 0068339B; read 006834D5 |
| +34h..+3Ch | `float[3]` | thumb base position | 006833AC..006833BA from 00AA6750 |
| +40h | `float` | minimum thumb height | 006833D5 from 00AA6740 +4h |
| +44h | `float` | current thumb height | 006833D0, 00683523 |
| +48h | `bool` | dragging | set 00683A40, cleared 00683324 and 006837D3; read 00683902 |
| +4Ch | `float` | cursor X latched at drag start | 00683A40 from GUI manager +5Ch; **no reader in the class** |
| +50h | `float` | cursor Y latched at drag start | 00683A40 from GUI manager +60h; read 0068391B |
| +54h | `float` | thumb offset at drag start | 00683A40; read 0068391E |
| +58h | `bool` | auto-scroll override, suppresses the input read | 00683316, cleared 006838FE and 006837D7; read 00683830 |
| +59h | `bool` | input gate, constructed to 1 | 0068366D; read 00683826. **No writer inside the class** |
| +5Ah | `bool` | scrollable, recomputed by every 006834A0 | 006834B6/00683517; read 00683562 |
| +5Bh | `bool` | tint widgets, constructed to 1 | 00683679; read 00683558 |

Size: 5Ch is a lower bound (nothing above +5Bh is touched). The tightest
observed spacing between two copies inside one screen is 74h, at 00626630 which
embeds blocks at +128h and +19Ch with nothing initialised between them, so the
true size is between 5Ch and 74h. The constructor leaves +4Ch and +50h
uninitialised, which is consistent: only the drag path writes them.

## Where screens embed it

Read from the `LEA ECX,[ESI+off]` immediately preceding each `CALL 00683610`.

| Screen constructor | Embed offsets |
| --- | --- |
| 005902E0 main menu | +1D4h, +354h |
| 005E6D90 multiplayer main menu | +6Ch |
| 005EA8C0 multiplayer mode selector | +3Ch |
| 00626630 rewards | +68h, +128h, +19Ch |

The mode selector's offset **contradicts** the existing comment on
`bsp::MultiMenuScreenAnimation` in include/bsp/multi_menu_screens.hpp, which
records "main menu and mode selector embed one at +6Ch". The listing at
005EA8F8 is `LEA ECX,[ESI+3Ch]` with only three vtable stores between it and the
call at 005EA914, none of which touch ECX.

Ten constructors call 00683610: 005098B0, 00555770, 00563370, 00574240,
005902E0, 005E3290, 005E6D90, 005EA8C0, 005FFCB0, 00626630.

## How each routine works

### Attach and reset

**00683380** stores the thumb, seeds both +2Ch and +30h with the track length,
captures the thumb's resolved position into +34h..+3Ch and copies the thumb's
current height (`00AA6740` returns `this+20h`, so `+4h` of that is the height)
into both +40h and +44h. The two arrow widgets come from the third and fourth
stack arguments.

**00683790** is the content attach. A null argument keeps the previous content
widget and its captured base but still performs the reset. It zeroes the scroll
offset, the auto-scroll direction and step, the drag flag, the override flag and
the drag anchor, then repositions the content and calls 00683440.

**006833F0** re-reads both base positions from whichever widgets are currently
attached, guarded separately on +00h and +28h. This is the routine a caller
would use after a layout change.

### Scroll bar geometry, 006834A0

```
+0Ch = range;  +5Ah = false
if (thumb == null) return
if (!(0 < range)) { travel = 0;  height = min_height + track;  scrollable = false }
else if (range < track) { height = max(min_height, (track + min_height) - range)
                          travel = range;  scrollable = true }
else                    { height = min_height;  travel = track;  scrollable = true }
+44h = height
thumb->SetPosition(thumb_base)            // 00AA8240 at 00683527
thumb->vtable[58h]({thumb->size.width, height})
if (+5Bh) {
    thumb->vtable[4Ch](scrollable ? 1.0f : 0.2f)      // 00D7A24C / 00CE54A0
    arrow_start->vtable[4Ch](scrollable ? 1.0f : 0.25f)  // 00CE3868
    arrow_end  ->vtable[4Ch](scrollable ? 1.0f : 0.25f)
}
```

`height + travel` is `min_height + track` in every branch, which is what fixes
the meaning of +30h: it is the track length minus the minimum thumb height, not
the track length itself. The x87 order matters in two places: `(track +
min_height) - range` is one expression with a single store to a float slot at
00683501, and `min_height + track` likewise at 006834C8.

The `max` at 0068350B is written as a pointer select (`LEA EDX,[ESI+40h]` then
`LEA EDX,[ESP+10h]` on `JBE` failing), so a NaN candidate keeps `min_height`.

The zero-argument compare at 006834C3 is `COMISS XMM1,XMM0` with XMM1 zero and
`JC`, so a NaN range takes the scrollable path. The compare at 00683448 in
00683440 is the `UCOMISS/LAHF/TEST AH,44h` inequality pattern, so a NaN range is
"not equal to zero" there too.

### Thumb sync, 00683440

```
if (range != 0 && thumb != null) {
    progress = (offset / range) * travel
    thumb->SetPosition(thumb_base - (0, progress, 0))
}
```

The quotient is never narrowed to float between the `FDIV` at 00683464 and the
`FMUL` at 00683467, so the divide and the multiply share one rounding. The
literal zeros in the X and Z components are `FLDZ` at 00683471: the compiler
folded a zero vector, which is why the subtraction reads as `base - offset` in
the source rather than a direct store.

### Scroll steps, 00683680 and 00683700

Both are gated on `0 < range`.

| Address | Update | Clamp |
| --- | --- | --- |
| 00683680 | `offset = delta + offset` | `if (offset > 0) offset = 0` |
| 00683700 | `offset = offset - delta` | `if (offset < -range) offset = -range` |

Both then set the content position to `content_base + (0, offset, 0)` (the same
`FLDZ` folding) and call 00683440. The offset therefore lives in `[-range, 0]`
and the thumb, at `thumb_base.y - progress`, moves along +Y as the offset goes
negative. Which screen direction that is depends on the GUI coordinate
convention, which this packet did not establish, so the names here are
"toward the start" and "toward the end" of the list rather than up and down.

### Per-frame pump, 00683820

```
if (+59h && !+58h) {
    axis = input_manager->[4h]->[36B4h]
    if (axis == 0) axis = input_manager->[4h]->[3714h]
    if (allow_pad_fallback && axis == 0) {
        pad = (*00F8BBF4)->004BA6D0(1, 0)
        if (pad) axis = pad->vtable[24h](10) * -0.5      // double 00CEC9E0
    }
    if (axis == 0) { +20h = 0; +24h = 0 }
    else { +20h = (0 > axis) ? 2 : 1;  +24h = fabs(axis * 0.025) }  // double 00D7A380
    +58h = 0
}
if (+48h) {                                     // dragging
    y = clamp((thumb_base.y + drag_anchor) + (cursor_y - anchor_cursor_y),
              thumb_base.y, thumb_base.y + travel)
    if (range > 0) offset = -range * ((y - thumb_base.y) / travel)
    thumb->SetPosition({thumb_base.x, y, thumb_base.z})
}
switch (+20h) {
  case 1: 00683680(this, +24h); break            // resyncs the thumb
  case 2: 00683700(this, +24h); break
  default: if (content) content->SetPosition(content_base + (0, offset, 0))
}
```

Three x87 details from the listing. The unclamped drag Y is one expression
(00683918..0068392D) with a single store to a float slot. The drag fraction at
00683984 **is** stored to a float slot and reloaded before the multiply, so it
rounds twice. Both scale factors are doubles, not floats: `FMUL double ptr
[00D7A380]` is 0.025 and `FMUL double ptr [00CEC9E0]` is -0.5.

The override flag +58h is only cleared inside the branch it gates, so once a
caller sets it through 00683300 the input read stays off until another
00683300 or 00683790 call clears it. That is consistent with a held arrow
button: press sets `(dir, step, 1)`, release sets `(0, 0, 0)`.

The drag branch runs whether or not the input branch ran, and the default arm of
the switch is the only one that does not resync the thumb, because the drag
branch has just positioned it directly.

### Drag anchors, 00683A40 and 00683320

00683A40 clears the auto-scroll direction (but deliberately not the step),
latches the GUI manager cursor at +5Ch/+60h into +4Ch/+50h, sets +48h and
records `thumb->GetPosition().y - thumb_base.y` into +54h. 00683320 is a
one-instruction release. Three routines begin the drag (00565190, 005993A0,
0062B3B0) and eight end it (004FAF40, 00561320, 005635F0, 00581970, 005E6E20,
005EA990, 005FCAE0, 0061D770). Six of the eight also call 00683300 (all but
005635F0 and 005FCAE0), and so do all three drag beginners, which is the pattern
of paired press and release handlers that set the auto-scroll on the way in and
clear it on the way out.

## The three GUI widget helpers

00AA6740 is `LEA EAX,[ECX+20h]; RET`, so a widget stores its size as two floats
at +20h/+24h. 00AA6750 and 00AA8240 are the position accessors: both look at the
parent pointer at +70h and, when it is non-null, recurse to the parent's
resolved position and apply the parent's scale pair at +18h/+1Ch times its size
pair at +20h/+24h. The widget's own local translation lives at +0Ch..+14h. The Z
term goes through `FSUB double ptr [00D7A258]`, which is 0.0, so Z passes
through untouched in both directions.

Virtual +58h takes a pointer to a two-float pair and is fed the widget's own
width from 00AA6740, so it is the size setter. Virtual +4Ch takes one float by
value (`PUSH ECX; FSTP float ptr [ESP]`) and its three call sites pass 0.2, 0.25
and 1.0, so it is the alpha setter. Both names are hypotheses from use, not
recovered symbols.

## Callers and callees

Callees of the block: 00AA6740, 00AA6750, 00AA8240, `BSP_InputManager_GetSingleton`
(004BEC00), `BSP_GuiManager_GetOrCreate` (004C12B0), 004BA6D0, and the two widget
virtuals. Nothing else.

Callers of the per-frame pump 00683820 (ten): 005162B0, 00560430, 00565540,
0057B600, `BSP_MainMenuScreen_Update` (00599DB0), 005D3F20, 005E5B30, 005ECD80,
00603CA0, 0062B640. These are screen update virtuals, which is what makes
00683820 the per-frame entry.

Callers of 006834A0 (21) are the routines that rebuild a list's contents;
callers of 00683680/00683700 outside the pump are 0055CEC0, 00561860 and
0056B610.

## Uncertainties

- **Screen direction.** The GUI Y convention is not established here, so
  "toward the start" and "toward the end" are the honest names for 00683680 and
  00683700. Anyone who fixes the convention should rename them.
- **Frame-rate dependence.** 00683820 applies +24h once per call with no delta.
  Either the input manager's axis at +36B4h is already time-scaled or the scroll
  really is frame-rate dependent. Not resolved.
- **Input manager fields.** +36B4h and +3714h on `input_manager->[4h]` are two
  axes with the second as a fallback; which physical control each is was not
  traced.
- **004BA6D0** is tagged `stl_instantiation` in the ledger but is called here as
  a `__thiscall(1, 0)` on the singleton at 00F8BBF4 returning an object whose
  virtual +24h reads an analog axis. The tag looks wrong for this call site.
- **+59h** has no writer in the class and **+4Ch** has no reader. Both point at
  code outside 00683300..00683A8F that this packet did not find.
- **Virtual +4Ch and +58h** are named from their arguments only. The widget
  class itself was not identified.

## What remains

- The widget class behind 00AA6740/00AA6750/00AA8240: its vtable, +4Ch and +58h.
- The screens' own use: which list each embedded copy scrolls, and where +59h is
  set and cleared.
- Reconciling `bsp::MultiMenuScreenAnimation` in
  include/bsp/multi_menu_screens.hpp, which is a placeholder with a wrong embed
  offset for the mode selector. That file is not owned by this packet.

## Follow-up packets proposed

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `gui_widget_transform` | 00AA6740, 00AA6750, 00AA8240, 00AA70E0, 00AA7220 | docs/GUI_WIDGET_TRANSFORM.md, include/bsp/gui_widget_transform.hpp | The widget parent chain at +70h, the scale pair at +18h/+1Ch and the size pair at +20h/+24h, and the vtable slots +4Ch and +58h the scroll block drives. |
| `gui_scroll_list_bindings` | 0055CEC0, 00561860, 0056B610, 00565190 | docs/GUI_SCROLL_LIST_BINDINGS.md | The three call sites that drive 00683680/00683700 outside the pump and the drag begin at 00565190: which button and hit test each is on. |
| `input_manager_axes` | 004BEC00 and instance +36B4h/+3714h, 004BA6D0 | docs/INPUT_MANAGER_AXES.md | Identify the two axis fields the scroll block reads and the pad object 004BA6D0 returns, including whether the axis is time-scaled. |

## State reached

| Address | State |
| --- | --- |
| 00683300, 00683320, 00683330, 00683380, 006833F0, 00683440, 006834A0, 00683610, 00683680, 00683700, 00683790, 00683820, 00683A40 | reconstructed, build-tested |
| 00AA6740 | analyzed (one instruction, fully recovered) |
| 00AA6750, 00AA8240 | analyzed to the parent-chain transform; not reconstructed |
| 005902E0, 005E6D90, 005EA8C0, 00626630 | read only, for embed offsets |
| 00599DB0 and the other nine pump callers | read only; owned elsewhere |

Every routine in the block has a Ghidra function; there are no listing-only
routines in this packet.

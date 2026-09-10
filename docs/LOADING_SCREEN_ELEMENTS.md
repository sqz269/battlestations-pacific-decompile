# Loading screen elements, hint rotation and worker callback

Addresses: 0057C560, 0057C990, 0057C4C0, 0057C360, 0057CA00, 00ABAED0

Packet `loading_screen_elements`. Continues `docs/GAME_FRONTEND_ENTRY.md`, which
recovered `BSP_LoadingScreen_Begin` (0057CB60) and left the four element fields,
the vtable `+10h` initialiser and the worker callback unopened. Every name here
is a hypothesis, not a recovered symbol. Ghidra was read-only for this packet:
no renames, comments or saves were applied.

## Corrections to the earlier packet

- **The vtable `+10h` initialiser is 0057C560, not 0057C4C0.** The screen's
  vtable is 00CEF264; its ninth dword (`+10h`) is `0057C560`, and 0057CCBB reads
  exactly that slot. 0057C4C0 is the hint-rotation tick, reached only from
  0057C990 and 0057CA00.
- **There are eight named GUI objects, not four.** `screen+0Ch` is the GUI page,
  `+10h`..`+28h` are six widgets, and a seventh lookup is discarded.
- **The `mp.loading_NN` strings are localisation keys, not textures.** 0057C360
  passes each one to the localisation resolver 00A9FAD0 and then to the text
  setter 00ABAED0, so they are the rotating hint line. This answers the open
  question of `docs/GAME_FRONTEND_ENTRY.md`: mode 0 does consume the list that
  004E4000 publishes, through the rotation rather than through the mode block.
- **The worker call's ECX is the GUI manager.** 0057CEC0 calls 004C12B0 and
  0057CED4 moves its result into ECX, so 00AA4040 is a GUI-manager method taking
  `(page, callback, context, rate)`; the page is `*(screen+0Ch)` and the context
  is `*(screen+10h)`, the values of those fields rather than their addresses.

## The screen object (00E194B4, 0x58 bytes)

| Offset | Contents | Evidence |
| --- | --- | --- |
| +00h | vtable 00CEF264; slot `+0h` (0057C230) is `MOV EAX,59h; RET` | 0057C303 |
| +04h/+05h | the wanted/active bytes of `FrontEndScreen` | 0057CE96/99 |
| +08h | the lifetime-hook sub-object; vtable slot `+28h` (0057C240) is `SUB ECX,8; JMP 0057C540`, its adjustor thunk | 0057C309 |
| +0Ch | GUI page handle for `FE_loading` | 0057C5D5 |
| +10h | widget `radar_Group`; the worker callback's context | 0057C6xx, 0057CEC5 |
| +14h | widget `wave_Icon`; **no reader found** | 0057C560 only |
| +18h | widget `hint_Text`; the rotating localised hint | 0057C648, 0057C46A |
| +1Ch | widget `frameFlag_Icon`; the background image | 0057CD2B, 0057CD82 |
| +20h | widget `titleLogo_Icon`; visible in mode 0 only | 0057CD46, 0057CDAC |
| +24h | widget `title_Text`; the mode 1 caption | 0057CDBE, 0057CDD2 |
| +28h | widget `loadingLogo_FrameBox`; the caption's plate | 0057CE79, 0057CE2D |
| +2Ch | int, last tick; `-1` on reset | 0057C99F |
| +30h | float, monotonic maximum progress; `0.0f` on reset | 0057C99A |
| +34h | not observed | |
| +38h..+47h | `ClockTimestamp` of the current hint's appearance | 0057C393..0057C3A9 |
| +48h | float, the current hint's minimum time on screen | 0057C43F |
| +4Ch | float, the progress each hint is worth | 0057C9DB |
| +50h | float, the progress at the last advance | 0057C9A2, 0057C51B |
| +54h | not observed | |

`+38h..+47h` settles the earlier packet's "`+38h`, `+3Ch`, `+44h` zeroed and
`+40h` set to 1" as the default `ClockTimestamp{ticks = 0, frequency = 1}` of
`include/bsp/frame_clock.hpp`, not four unrelated fields.

## 0057C560 - bind the page and the widgets

`__fastcall(this)`, RET 0. Vtable `+10h`, called once at 0057CCC2 right after the
screen is allocated.

1. 0057C566: `BSP_FrontEndScreen_Register` (004F71D0) publishes the screen at
   `00E18B60 + 59h*4`.
2. 0057C5C2..0057C5D5: the GUI manager (004C12B0), then `00AA5840(&"FE_loading",
   0, 0)`; the handle is stored at `+0Ch`.
3. Eight `00AA7E00(&name, 1)` lookups with `ECX = *(this+0Ch)` reloaded each
   time (0057C631). The trailing `1` is 00AA7E00's recursive flag; it compares
   with `__stricmp`.

| Order | Name | Field |
| --- | --- | --- |
| 1 | `hint_Text` | +18h |
| 2 | `radar_Group` | +10h |
| 3 | `wave_Icon` | +14h |
| 4 | `bg_Group` | discarded at 0057C609 |
| 5 | `titleLogo_Icon` | +20h |
| 6 | `frameFlag_Icon` | +1Ch |
| 7 | `title_Text` | +24h |
| 8 | `loadingLogo_FrameBox` | +28h |

Each name is built into a pooled native string (00041DD40 resize plus `memcpy`)
and returned to the sized storage pool immediately. Nothing is null-checked: a
missing widget leaves null in the field and faults at the first use.

The layout constants the packet asked for are not in this function. `FE_loading`
is a data-driven GUI page, so position and size live in the page asset; the only
layout arithmetic in native code is the caption plate below. No font or
localisation key is named here either; the keys arrive from the published list.

## 0057C990 - reset the rotation

`__fastcall(this)`, RET 0. Called at 0057CE83 for **every** mode, including the
modes that configure no element, and it is `LoadingScreenHost::prepare_extra_elements`
of `include/bsp/frontend_entry.hpp`.

```
+30h = 0.0f ; +2Ch = -1 ; +50h = 0.0f
N    = (00E08788 - 00E08784) >> 3          ; unsigned, 8-byte native strings
+4Ch = (00E08784 == 0 || N == 0) ? 200.0 : 1.0f / (float)(unsigned)N
00E0877C = -1                               ; the hint cursor
0057C360(this)                              ; show hint 0
JMP 0057C4C0                                ; tail jump, so the reset ends in the tick
```

An empty key list installs a step of `200.0` (00CE4D70), which progress can never
travel, so the rotation is pinned. 0057C9C9 adds `4294967296.0f` (00CE3978) when
the count's sign bit is set, which makes the conversion unsigned.

## 0057C360 - advance to the next hint

`__fastcall(this)`, RET 0. Called from 0057C990 and 0057C4C0.

```
N = (00E08784 == 0) ? 0 : (00E08788 - 00E08784) >> 3
if ((int)00E0877C >= N - 1) return          ; 0057C37E, signed
++00E0877C
clock(01090AB0)->vtable[20h](&t)            ; 16 bytes copied to +38h..+44h
len  = ResolveKeyList(00F8BC4C, &keys[cursor])   ; UTF-16 length only
+48h = (float)((float)(unsigned)len * 0.0625)    ; 00CEF290
00ABAED0(ECX = *(this+18h), &keys[cursor], 1)    ; the hint line
```

`0.0625` s per character is the hint's minimum time on screen, so a 40-character
line holds for 2.5 s. The resolved buffer is released without being read; only
its length is used. The native range-checks the cursor twice against the vector
and traps through the CRT invalid-argument helper 00BF6713 when it is out.

An empty list makes the guard `-1 >= -1`, so nothing is sampled or shown.

## 0057C4C0 - the rotation tick

`__fastcall(this)`, RET 0. Called from 0057C990 and, at 25 Hz, from 0057CA00.

```
if (!(+30h - +50h > +4Ch)) return           ; 0057C4D5, x87 difference vs the step
now = clock(01090AB0)->vtable[20h]()
BSP_Timestamp_Subtract(ECX = now, &out, +38h)
if (!((float)(out.ticks / out.frequency) > +48h)) return   ; 0057C50F
0057C360(this)
+50h = +30h
```

Both gates are strict `>`: a hint changes only when the reported progress has
travelled a whole step **and** the current hint has outlived its display time.
The progress that arms the first gate is written by
`BSP_LoadingScreen_ReportProgress` (0057BEC0), so the rotation is driven by the
loader's progress reports and paced by the worker.

The three pushes before the vtable `+20h` call are shared between two callees:
the clock method takes only the destination pointer (RET 4, EAX = destination,
matching `sample_frame_clock_00BEE080`), and the remaining two are
`BSP_Timestamp_Subtract`'s stack arguments (RET 8). 0057C360's one-push call of
the same slot is what fixes that split; the decompiler renders it as a
three-argument call and a separate two-argument call, double-counting.

## 0057CA00 - the render worker callback

`__fastcall(context)`, RET 0. `MOV ESI,ECX` at 0057CA1B is the only parameter;
there are no stack arguments.

**Which thread.** 0057CED6 calls `BSP_RenderMode_PrepareAndStartWorker`
(00AA4040) with `ECX` = the GUI manager, arg1 = `*(screen+0Ch)` (the page,
the render descriptor), arg2 = `0057CA00`, arg3 = `*(screen+10h)`
(`radar_Group`) and arg4 = `19h`. Per `docs/RENDER_WORKER.md` the start helper
stores callback/context/rate at worker `+1Ch`/`+20h`/`+18h` and the worker
thread 00B33C20 (priority -2) loads the context into ECX at 00B33CFC before
calling. So the callback runs on the render worker thread with
`ECX = radar_Group`, while the calling thread is still inside
`BSP_Game_EnterFrontEndShell`.

**The rate.** `19h` = 25. The worker runs the callback only when its own elapsed
milliseconds exceed `1000 / rate`, so 40 ms; otherwise it calls
`SwitchToThread`. The worker computes those milliseconds with the same timer
ratio, the same `1000.0` double and the same `-0.0 minus delta` negation this
callback repeats for its own clock.

**What it locks.** Nothing itself. It takes no critical section and touches no
event. The synchronisation is the worker's: 00B20220 is polled with `Sleep 10`
around the callback and the begin/end frame pair is the worker's, not the
callback's.

**The render calls it issues.** None. The callback issues no render command and
touches no queue; the queue was put in mode 2 by 00AA4040 before the worker
started, and the page at `+0Ch` is what the worker draws. The body is:

```
name = "wave_Icon"                          ; 00CEF2DC, pooled string
icon = 00AA7E00(ECX = context, &name, 1)    ; re-resolved every tick
clock(01090AB0)->vtable[20h](&sample)
t    = (float)((float)(sample.ticks / sample.frequency) * 1000.0)   ; 00CE47A0
d    = t - 00E194BC
mag  = (d > 0.0f) ? d : (-0.0f - d)         ; 00D7A208
icon->vtable[40h](mag / 1000.0)             ; RET 4, elapsed seconds
00E194BC = t                                ; recomputed from the same sample
if (00E194B4) 0057C4C0(ECX = 00E194B4)
00E188A8->BSP_OnlineStats_UpdateWrite()      ; 004CAA90
```

`00E194BC` is a standalone float eight bytes past the singleton pointer, not a
field of the screen; the callback keeps its own clock because the screen can be
null. The `-0.0f` subtraction is the native negation idiom, so two ticks on one
clock sample hand the widget a negative zero rather than `+0.0f`.

**How progress is read.** Indirectly: the callback passes the screen to
0057C4C0, which reads `+30h`, the monotonic maximum that 0057BEC0 raises.

**Observed cost.** The callback allocates a pooled string and walks the widget
tree case-insensitively 25 times a second to find a widget 0057C560 already
cached at `+14h`. `+14h` has no reader in the class's own methods, so the cache
appears to be dead. This is an observation about the shipped code, not a defect
to fix here.

## 00ABAED0 - the localised text setter

`__thiscall(NativeString* key, char localise)`, RET 8. Already carried a ledger
record from packet `font_context_ownership_audit` under the placeholder name
`FUN_00ABAED0`; this packet establishes what it does at its two call sites.

```
if (EqualsInsensitive(this+F4h, key)) return          ; unchanged text is a no-op
copy key into this+F4h/F8h
if (localise == 0) utf16 = 004C5E60(key.data ? key.data : 00F8BE4C)
else               utf16 = ResolveKeyList(00F8BC4C, key)
00ABA8D0(utf16)                                        ; rebuild the geometry
release the utf16 buffer
this->vtable[50h](this + 50h)
```

Both native call sites in this packet pass `localise = 1`: 0057CDC7 with
`ECX = *(screen+24h)` and the published picked name at 00E08790, and 0057C473
with `ECX = *(screen+18h)` and the current key. So both the caption and the hint
line are localisation keys.

The early-out on an unchanged string is why the rotation can call the setter
freely, and why `docs/GAME_FRONTEND_ENTRY.md` saw no image binding here: the
function has no image path at all.

## The mode block (0057CCCC..0057CE83) and the caption plate

The mode arms are already reconstructed in `src/frontend_entry.cpp`; what was
missing is which widget each index means and what the rect arm computes.

| Mode | Widget effects |
| --- | --- |
| 0 | load `interface/textures/menu.ats` (00CEF30C); `004C1AC0(2,1)` then `00518250(2,1)`; `frameFlag_Icon` vtable `+88h`(2, 0, 1.0f); `titleLogo_Icon` visible; `title_Text` and `loadingLogo_FrameBox` hidden |
| 1 | `004C1AC0(1,1)` then `00518250(1,1)`; `frameFlag_Icon` vtable `+88h`(00E08798 != 0, 0, 1.0f); `titleLogo_Icon` hidden; `title_Text` set from the picked key; `loadingLogo_FrameBox` sized; both visible |
| other | 0057CCC6 jumps to 0057CE83; only the rotation reset runs |

The image index is the published visibility byte itself (`SETNE` at 0057CD9C),
so mode 1 picks image 0 or 1 and mode 0 picks 2 out of the same element. The
third argument of `+88h` is `FLD1`, so alpha is always 1.0f; the middle argument
is 0 at both sites and its meaning is not recovered.

The plate arm, 0057CDD2..0057CE60:

```
r = (float)(title_Text->+114h / 960.0)                  ; 00CEC380
size.y = *(loadingLogo_FrameBox + 24h)                  ; 00AA6740 is LEA EAX,[ECX+20h]
size.x = (r > 0.26666666666666666) ? (float)(r + 0.26666666666666666)  ; 00CEC3E8
                                   : 0.5333333611488342f              ; 00CEC3E4
loadingLogo_FrameBox->vtable[58h](&size)
```

That is `max(r, 4/15) + 4/15`: the plate is the caption's width plus one padding
unit, with the caption floored at one padding unit, and the plate keeps its own
height. The short arm's float is the one nearest 8/15, which is what the long
arm produces at the threshold, so the two arms meet.

`+114h` on the caption class is read as its laid-out width in a 960-unit
reference space. The divide by 960.0 is the evidence: `docs/GUI_GEOMETRY_DISPATCH.md`
records `+114h` as `PartialDisplayRatio`, clamped to [0,1], on the **stateful
texture** class (constructor 00AB5C60, vtable 00D5C4C0). That class keeps a
vector at `+F4h`, while the class 00ABAED0 drives keeps a native string there,
so they are different classes and the field is reused. A ratio in [0,1] divided
by 960 could never exceed 4/15, which would make the long arm dead and the
constant pointless. This is the one field reading in this packet that rests on
an argument rather than on a store, and it is marked provisional.

## Constants

| Address | Value | Use |
| --- | --- | --- |
| 00CE47A0 | double 1000.0 | milliseconds per second, worker clock |
| 00D7A208 | float -0.0 | the negation the delta's non-positive arm uses |
| 00CE4D70 | double 200.0 | the hint step when the key list is empty |
| 00CE3978 | float 4294967296.0 | the signed-FILD-to-unsigned fixup |
| 00CEF290 | double 0.0625 | seconds per character of hint hold time |
| 00CEC380 | double 960.0 | the caption plate's reference width |
| 00CEC3E8 | double 0.26666666666666666 | the plate padding, the double nearest 4/15 |
| 00CEC3E4 | float 0.5333333611488342 | the minimum plate width, the float nearest 8/15 |
| 00CEF2DC | "wave_Icon" | the per-tick lookup name |
| 00CEF30C | "interface/textures/menu.ats" | the mode 0 atlas |

## Globals

| Address | Contents |
| --- | --- |
| 00E194B4 | the screen singleton pointer |
| 00E194BC | float, the worker callback's last clock sample in milliseconds |
| 00E0877C | int, the hint cursor; -1 before the first advance |
| 00E08784/88/8C | the key vector of `LoadingScreenConfig::images` |
| 00E08790 | the picked key, mode 1's caption |
| 00E08798 | the published visibility byte, mode 1's image index |
| 00F8BC4C | the localisation manager (ECX of 00A9FAD0) |
| 01090AB0 | the clock singleton (vtable `+20h` samples it) |

## Calling conventions and RET sizes

| Address | Convention | RET | Notes |
| --- | --- | --- | --- |
| 0057C560 | `__fastcall(this)` | 0 | vtable `+10h` |
| 0057C990 | `__fastcall(this)` | 0 | tail-jumps into 0057C4C0 |
| 0057C4C0 | `__fastcall(this)` | 0 | |
| 0057C360 | `__fastcall(this)` | 0 | |
| 0057CA00 | `__fastcall(context)` | 0 | context is `radar_Group`, not the screen |
| 00ABAED0 | `__thiscall(key, localise)` | 8 | |
| 0057C230 | `__fastcall(this)` | 0 | vtable `+0h`, returns 59h |
| 0057C240 | thunk | - | `SUB ECX,8; JMP 0057C540` |
| clock vtable `+20h` | `__thiscall(destination)` | 4 | EAX = destination |
| widget vtable `+40h` | `__thiscall(float)` | 4 | the per-tick update |
| widget vtable `+58h` | `__thiscall(const size*)` | 4 | |
| widget vtable `+88h` | `__thiscall(int, int, float)` | 12 | |

## Callers and callees

- 0057C560: no callers in Ghidra's graph; reached only through vtable 00CEF264
  `+10h` at 0057CCBB. Callees 004F71D0, 004C12B0, 00AA5840, 00AA7E00 and the
  string pool.
- 0057C990: called by 0057CB60 at 0057CE89. Calls 0057C360 and 0057C4C0.
- 0057C4C0: called by 0057C990 and 0057CA00. Calls the clock singleton,
  00530890 and 0057C360.
- 0057C360: called by 0057C990 and 0057C4C0. Calls the clock singleton,
  00A9FAD0, 00ABAED0 and the string pool.
- 0057CA00: no callers; it is the callback pointer pushed at 0057CECE. Calls
  00AA7E00, the clock singleton, widget `+40h`, 0057C4C0 and 004CAA90.
- 00ABAED0: called by 0057CB60 (0057CDC7) and 0057C360 (0057C473).

## Ghidra function definitions

All six routines have Ghidra functions with correct starts. 0057CB60's stored
body is truncated past 0057CCA0, so its tail was read from disk bytes with
`bsp.py disasm-raw`; the orchestrator may want to redefine that body, but no
address this packet names needs a function created.

## State reached

| Address | State |
| --- | --- |
| 0057C560 | reconstructed, build-tested |
| 0057C990 | reconstructed, build-tested |
| 0057C4C0 | reconstructed, build-tested |
| 0057C360 | reconstructed, build-tested |
| 0057CA00 | reconstructed, build-tested |
| 00ABAED0 | analysed; kept behind the host, not reimplemented |

`include/bsp/loading_screen_elements.hpp` and `src/loading_screen_elements.cpp`
hold the reconstruction. It reuses `ClockTimestamp`,
`subtract_timestamp_00530890` and `timestamp_seconds_x87` from
`include/bsp/frame_clock.hpp` and `LoadingScreenMode` from
`include/bsp/frontend_entry.hpp`, and it deliberately does not repeat
`begin_loading_screen`'s mode sequence: it supplies the element identities, the
image index and the plate arithmetic that sequence hides behind its host.

None of it is binary compatible: the widget classes, the pooled strings and the
GUI page are not reproduced, and the x87 evaluation is modelled with doubles
where the note in the source says so.

## Uncertainties

- `+114h` on the caption class is argued, not stored-to in anything read here.
- The middle argument of widget `+88h` is 0 at both call sites; unknown.
- The `(2,1)` / `(1,1)` pair of 004C1AC0 and 00518250 is still unrecovered, as
  in the earlier packet.
- Widget vtable `+40h` is read as a per-tick update from its float argument and
  its position in the callback. The slot was not cross-checked against another
  class's use of `+40h`.
- Screen `+34h` and `+54h` were never seen read or written.
- `+14h` (`wave_Icon`) has no reader among the class's own methods, checked
  across 0057C1C0, 0057C230, 0057C240, 0057C250, 0057C360, 0057C4C0, 0057C540,
  0057C560, 0057C990, 0057CA00, 0057CB60 and 0057CF10. A reader elsewhere would
  have to reach it through the singleton.
- The rotation runs in every mode, but only 004E4000 (mode 0) was found
  publishing keys. A mode 1 caller is still unidentified.

## Follow-up packets proposed

- `gui_widget_named_lookup`: 00AA7E00, 00AA5840, 00AA6740, files
  `docs/GUI_WIDGET_LOOKUP.md`, `include/bsp/gui_widget_lookup.hpp`. Contract:
  the page factory's two zero arguments, the recursive name search and the
  widget size pair at `+20h`, which four packets now reach through a host method.
- `gui_text_element`: 00ABAED0, 00ABA8D0, 00A9FAD0 and widget vtable `+50h`,
  files `docs/GUI_TEXT_ELEMENT.md`, `include/bsp/gui_text_element.hpp`.
  Contract: the cached string at `+F4h`, what `+114h` holds on this class, and
  what the geometry rebuild produces. This is what would settle the one
  provisional field reading above.
- `render_worker_callback_set`: 0057CA00's siblings, the other callbacks
  00AA4040's callers install (0060D6E0 is the second caller), files
  `docs/RENDER_WORKER_CALLBACKS.md`. Contract: whether every callback repeats
  the worker's own delta computation with a private accumulator.

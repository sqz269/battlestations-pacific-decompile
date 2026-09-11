# The interface runtime tail (packet `cc_interface_runtime`)

Addresses: 005B6960, 005BA7B0, 005BB130, 005B5BD0, 005B5C00, 00CF0ED8, 005CD1A0, 005CD240,
005CC170, 00E19894, 00683E90, 00684600, 004CC460, 004BC410, 004D2BB0, 004DA780, 00E19698,
0060D290, 0060D2B0, 0060D2C0, 0060D2D0, 0060D2F0, 0060D330, 0060D390, 0060D430, 00CF4AB8,
004E3DBB, 004CCCC0, 004DCF90, 00A933F0, 00927F30, 0077C470, 004E7BB0, 0054D510.

This packet closes the three named unknowns `docs/MAIN_MENU_SCREENS_RUNTIME.md` left at the end of
004C40F0. Reconstruction: `include/bsp/interface_runtime_tail.hpp`, `src/interface_runtime_tail.cpp`.
Names are hypotheses, not recovered symbols.

Every routine here was read from the listing. Both 005B6960 and 005CD1A0 have register inputs the
decompiler misplaces, and the `_PleaseWait` class is four functions Ghidra never claimed, so the
pseudocode alone is wrong or absent for all three items.

## 1. 005B6960, the mission-end blackout

`__thiscall(this)`, `RET`, body 005B6960..005B69AE. `SUB ESP,10h` then `PUSH ESI / MOV ESI,ECX`, so
`this` is the only input and there are no stack parameters.

```
005B6966  child = *(this + BCh)
005B6971  child->vtable[34h](child, 1)
005B6975  v = { 0.0f, 0.0f, 0.0f, *(float*)00D7A24C }      // 00D7A24C is 1.0f
005B69A3  child->vtable[50h](child, &v)
```

**The receiver.** 004C40F0 reaches 005B6960 only as the tail jump at 004C42EF, and 004C42DD reloads
`ECX` from `*(00E198C4 + A4h)` first. That is registry slot 33h, the same screen the three preceding
instructions forced visible (`+4h = +5h = 1`, 004F83B0, virtual `+18h`). Constructor 005BA7B0,
vtable 00CF0ED8, E8h bytes, per the slot table in `docs/IN_MISSION_INTERFACE_MANAGER.md`.

**The child.** The screen's register virtual `+10h` is 005BB130. At 005BB1AC it loads the
`GUI_blackout` page (name at 00CF113C, 0Ch chars) into `this+20h`, and at 005BB97E it looks
`Blackout_Icon` up in that page (name at 00CF106C, 0Dh chars) through 00AA7E00
`BSP_GuiWidget_FindChildByName` and stores the result into `this+BCh` at 005BB983. This is the only
write to `+BCh` anywhere in segment 20.

**The two virtuals.** `+34h` is the widget set-visibility slot (`docs/FRONTEND_PROMPT_SCREEN.md`
line 150 reads it that way on a background widget); `+50h` is the widget set-colour slot
(`docs/GUI_TEXT_WIDGET.md` has 00AB6B50 there, and `docs/LOADING_SCREEN_ELEMENTS.md` names it the
same). The screen's own enter virtual 005B5BD0 corroborates `+34h`: it calls `+34h(0)` on `this+28h`
and `this+ACh` and `+34h(1)` on `this+24h`, which is a show/hide of three sibling widgets and
nothing else.

So 005B6960 shows the blackout icon and paints it opaque black. It is the screen fade that ends a
mission, driven from the `game+624h != 0` arm of 004C40F0.

## 2. 005CD1A0, the only routine that engages the interface lock

`__thiscall(this, void* unit)`, `RET 4`, body 005CD1A0..005CD22E.

```
005CD1A4  BSP_RandomThreads_Seed(stream = 1, seed = 3039h)       // 12345
005CD1BA  BSP_RandomThreads_Seed(stream = 0, seed = D431h)       // 54321
005CD1CE  PushInterfaceRequest([00E198C4], 2Ch, unit)            // 004CC460
005CD1D3  if ([00E188A8] != 0 && game+1FE4h != 0) {
005CD1E9      00E19894 = 1
005CD1F6      InputManager::00A933F0(1Eh, 5)                     // getter 004BEC00 first
          }
005CD1FD  005CC170(this)
005CD202  if ([00E188D8] != 0 && IsLocalPlayerRole(unit, 0))
005CD224      0077C470(unit, 1FFh, 0)
005CD229  this+20h = 1
```

**The owner.** 0068A1F0, reached from 0068C1F0 `BSP_InGameInterface_Update`, calls both 005CD1A0 and
005CD240 with `ECX = manager->+9Ch`. That is registry slot 37h: constructor 005CC120, vtable
00CF17A8, 38h bytes, page `GUI_movie`, and `docs/HUD_SCREEN_PAGES.md` gives its raising interface id
as **2Ch** - the same id pushed at 005CD1CE and the one id 00683E90 exempts. The screen is the
movie-camera interface, and this routine is how the game enters it.

**The two fields.** `this+1Ch` is the 570h-byte movie camera 005CC170 builds on first use
(`operator new(570h)` then `_memset(...,0,570h)`, guarded by `this+1Ch == 0`). `this+20h` is the
engaged flag: 005CD240 runs the engage only while it is clear, then re-runs 005CC170 and returns
`this+1Ch`.

**The lock rule.** 00E19894 has six writers in the image and exactly one raises it:

| Site | Enclosing routine | Value | Guard |
| --- | --- | --- | --- |
| 005CD1E9 | 005CD1A0 | 1 | `[00E188A8] != 0 && game+1FE4h != 0` |
| 00683EA7 | 00683E90 | 0 | lock set, id != 2Ch, id < 20h |
| 00684636 | 00684600 | 0 | the same test inlined |
| 004BC493 | 004BC410 | 0 | unconditional in that arm |
| 004D2C5F | 004D2BB0 `BSP_Game_DestroyWorld` | 0 | `[00E198C4] != 0` |
| 004DAB5D | 004DA780 `BSP_Game_TeardownSessionState` | 0 | `[00E198C4] != 0` |

Readers: 004CC478, 00683E90 and 0068461C only. The decision itself is already reconstructed as
`classify_interface_request_00683e90` in `include/bsp/main_menu_screens_runtime.hpp`; this packet
adds the engage side and closes the writer set.

Stated as a rule: **the lock engages when the movie-camera interface is entered during a live
session, and it stays engaged until something asks for a front-end interface.** While it is up,
every in-session interface except 2Ch itself is dropped, and the first id below 20h both passes and
releases it. The two session-teardown writers are belt-and-braces: they clear a lock that a
front-end request would have cleared anyway.

**Register inputs the pseudocode loses.** Ghidra renders the two seeds as bare
`BSP_RandomThreads_Seed()` calls because both arguments arrive in ECX and EDX, and it hangs the
`PUSH 5 / PUSH 1Eh` pair on 004BEC00 `BSP_InputManager_GetSingleton` instead of on 00A933F0, whose
`this` is that getter's return value. 00A933F0 is `__thiscall(inputManager, int id, int level)`,
RET 8, per `docs/FRONTEND_SCREEN_SETS.md`.

## 3. 00E19698, the `_PleaseWait` screen

Not a gamepad prompt. `operator new(14h)` at 004E3DB9 inside `BSP_Game_OnInit`, constructor 0060D290
(which calls only 004F7180 `BSP_FrontEndScreen_Construct` and writes vtable 00CF4AB8), stored at
004E3DDB, and its register virtual `+10h` is called immediately at 004E3DEB.

The vtable has the ten front-end screen slots `docs/HUD_SCREEN_PAGES.md` tabulates:

| Slot | Target | Body |
| --- | --- | --- |
| +00h | 0060D2B0 | `mov eax, 53h; ret` - the registry slot id |
| +04h | 0060D2C0 | `mov al, 1; ret` |
| +08h | 004F7580 | base no-op |
| +0Ch | 0060D410 | scalar deleting destructor over 0060D330 |
| +10h | 0060D430 | register |
| +14h | 004F7590 | base; the class has no layout override |
| +18h | 0060D390 | enter |
| +1Ch | 0060D2D0 | exit |
| +20h | 0060D2F0 | update(float) |
| +24h | 0060D560 | |

**Registry slot 53h**, page `_PleaseWait` (name at 00CF4AF0, 0Bh chars; the installed file is
`interface/_pleasewait.lua`), one bound widget `Background_Icon` (00CF4AE0, 0Fh chars). The whole
14h-byte object is accounted for:

| Offset | Field | Written at |
| --- | --- | --- |
| +00h | vtable | 0060D298 |
| +04h | requested byte | the raise and clear sites below |
| +05h | applied byte | 00563624, 005B9A10, 00637445 |
| +08h | the `_PleaseWait` page | 0060D49E |
| +0Ch | the `Background_Icon` widget | 0060D518 |
| +10h | progress, `float` | 0060D53C, 0060D3B4, 0060D321 |

`+10h` starts at the constant at 00D7A260, which is **-1.0f**, and both the register virtual and the
enter virtual re-arm that sentinel. The update virtual leaves it alone while it holds the sentinel;
otherwise it reads two consecutive 64-bit counters from `[01090AB0]->vtable[20h]` and stores their
quotient with `FILD / FILD / FDIVP`. So `+10h` is a load progress fraction and -1.0f means "unknown".

**What `+4h` means.** It is the ordinary front-end screen requested byte and `+5h` the applied byte,
exactly as `docs/FRONTEND_STATE_MACHINE.md` and `include/bsp/frontend_state_machine.hpp` define
them. Three sites raise the screen by writing **both** bytes to 1 in the same breath - 00563618,
005B9A04 and 00637439 - which bypasses the commit pass; 005E58FF raises `+4h` only. Fifteen sites
clear `+4h`: 004D7C64, 004D844D, 004D85C1, 005629A6, 00565068, 00565B5B, 00565CC2, 0056DD46,
0057A17A, 005D9A3F, 005E5F48, 005F7432, 005F77AF, 005F77C6 and 0076FDFA. Clearing `+4h` is a request
to take the wait overlay down at the next registry pass.

**Why 004C40F0 may dereference it without a null test.** The global is written three times in the
whole image: the constructed object at 004E3DDB, and null at 004CCD75 and 004DD077. Both null
writes sit inside shutdown routines (004CCCC0 and 004DCF90 `CG_vector_deleting_dtor_004dcf90`) that
null-check the pointer, destroy the object through vtable `+0Ch(1)`, and then clear it. Between
`Game::OnInit` and shutdown the pointer is never null, and 004C41BD is far from alone in relying on
that: 004D85BC, 004D8650, 004D86B5, 005629A0, 00563618, 00565063, 0056509A, 00565B55, 00565C15,
00565CBD, 0056DD40, 0057A174, 005B9A04, 005D9A38, 005E58FF, 005E5F43, 005F7424, 005F77A9, 005F77B8,
005F786A and 00637439 all dereference it unguarded too. Only 004D7C5B, 004D8440, 0057CBF3 and
0076FDF1 test it first.

## Corrections

- **`005B6960` writes four floats, not three.** `include/bsp/main_menu_screens_runtime.hpp` line 156
  and `docs/MAIN_MENU_SCREENS_RUNTIME.md` line 231 both read the block as "a zeroed three-float
  vector and the constant at 00D7A24C". The listing stores four `MOVSS` into `[ESP+4]`, `[ESP+8]`,
  `[ESP+0Ch]` and `[ESP+10h]` and passes `LEA EDX,[ESP+4]`: one four-float argument whose fourth
  component is 1.0f, which is an RGBA colour, not a vector plus a scalar.
- **`005B6960`'s effect is now established**, so the host method
  `InterfaceOnlyHost::mission_overlay_finalize_005b6960` can stop being opaque. That header is not
  edited here; see the follow-ups.
- **`00E19698` is the `_PleaseWait` screen, registry slot 53h**, not a gamepad prompt.
  `docs/OPTIONS_MENU_SCREENS.md` line 350 calls `00E19698+4` "the gamepad-prompt byte"; it is the
  standard requested byte of a front-end screen whose page is `_PleaseWait`. The brief's
  `front_end_gamepad_prompt` name is retired.
- **"the five routines that clear it" is fifteen.** The brief and
  `docs/MAIN_MENU_SCREENS_RUNTIME.md` line 249 both say five; the image-wide sweep finds fifteen
  clear sites and four raise sites, listed above.
- **Registry slot 33h is not INTF_LAUNCHLANDING.** The two numbers collide but live in different
  spaces: slot 33h is the HUD narrative/blackout screen (`docs/HUD_SCREEN_PAGES.md` line 99), while
  *interface id* 33h is INTF_LAUNCHLANDING (`docs/IN_MISSION_INTERFACE_MANAGER.md` line 195). No
  document states this wrongly today, but the two tables sit in the same file and the join is easy
  to get wrong.
- **00E188D8 is a unit instance, and 0077C470 is a session send on it.** `BSP_UnitInstance_IsLocalPlayerRole`
  and 0077C470 share the same `ECX` here; the second reloads the global rather than keeping it, so
  the two reads are independent and a teardown between them would change the outcome.

## Uncertainties

- The **name `Blackout_Icon` -> "screen fade"** is an interpretation. What is established is the
  widget, its page, and the two virtual slots; that an opaque black full-screen icon is a fade is
  read from the name and from the `game+624h != 0` guard, not from a draw call.
- **Widget vtable `+34h` and `+50h`** are named from other packets' readings, not re-derived here.
  Both are corroborated by 005B5BD0's three-way show/hide, but neither routine was opened.
- `0060D2F0` **divides without checking the denominator**. The reconstruction refuses instead, which
  is a deliberate divergence; nothing establishes that the counter pair can be `0/0`.
- The **input context id 1Eh** set at 005CD1F6 is outside the `1..19h` range
  `docs/FRONTEND_SCREEN_SETS.md` walks at 004C4318, so it is recorded as a literal and not placed in
  that document's model.
- `00A94C50`, `00A92C40`, `00AA0F70`, `00AA0E00` and the object at `01090AB0` are host calls: their
  bodies were not opened, only their call shapes.
- The `_PleaseWait` **lifetime claim is an image-wide xref argument**, not an execution trace. It
  holds for every path the sweep can see; a write through an aliased pointer would not appear.

## Follow-up packets

- `interface_only_host_blackout` - fold this result into
  `include/bsp/main_menu_screens_runtime.hpp` line 154, replacing the "effect is not established"
  comment and the argument-free `mission_overlay_finalize_005b6960()` with the slot 33h receiver and
  the two widget virtuals. That header is outside this packet's lease.
- `please_wait_screen_drivers` - the four raise sites (00563618, 005B9A04, 005E58FF, 00637439) and
  the fifteen clear sites, to establish which operations the wait overlay covers. Only the byte
  writes are read here, never the enclosing routines.
- `please_wait_progress_source` - `[01090AB0]` and its virtual `+20h`, the 64-bit counter pair the
  progress fraction comes from. Probably the resource streamer.
- `movie_camera_object_005cc170` - the 570h-byte camera, its `00694A60` observer pair registration
  and its `00B6DB70` transform refresh. Only the allocation and the `this+1Ch` guard are read here.
- `gui_widget_colour_virtual` - 00CF0ED8's widget `+50h` target, to confirm the four-float argument
  is RGBA rather than a position plus a scale.

## `no_ghidra_function`

Five routines read for this packet have no Ghidra function. Four are the `_PleaseWait` class's own
small virtuals; the fifth is the slot 33h exit virtual, which is a single `RET`.

| Start | End (inclusive) | Note |
| --- | --- | --- |
| 005B5C00 | 005B5C00 | vtable 00CF0ED8 `+1Ch`, a bare `RET`; the slot 33h screen has no exit body |
| 005B5C10 | 005B5C30 | the routine immediately after it, `RET 8`; adjacent, not read for this packet |
| 0060D2B0 | 0060D2B5 | vtable 00CF4AB8 `+00h`, `mov eax, 53h; ret` |
| 0060D2C0 | 0060D2C2 | vtable 00CF4AB8 `+04h`, `mov al, 1; ret` |
| 0060D2D0 | 0060D2EF | vtable 00CF4AB8 `+1Ch`, the exit virtual |
| 0060D2F0 | 0060D32A | vtable 00CF4AB8 `+20h`, the update virtual, `RET 4` |

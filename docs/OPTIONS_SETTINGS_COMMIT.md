# Options settings commit (packet `options_settings_commit`)

Addresses: 008D5B50, 008D5430, 008D41C0, 008D41F0, 008D4520, 008D4820, 008D4210, 008D44C0,
008D48C0, 008D4260, 008D64A0. Read-only context: 008D8190, 008D5150, 008D56C0, 008D6170,
005F65C0, 007FAE70, 00699B80, 004C1710, 00B29E60, 00B0D580, 00B107F0, 00B0D080, 00B1FFB0,
00A7A440, 00A7ABF0, 00A7ACF0, 00A864F0, 00BD5680.

`docs/OPTIONS_MENU_SCREENS.md` left two questions open: which fields of the settings block the
screen rows edit, and what the commit pushes into each subsystem. Both are answered here. The
third question that doc asked, "which registry value each field lands in", turns out to rest on a
false premise: **no field of the settings block is written to the registry, and none of the ten
addresses writes the options file.** The persistence path is a visitor-driven key/value block at
008D64A0 that the player-profile save carries. That is the main correction this packet makes.

## The settings object

The object is the static `cGameSettings` at **00F88980**, the one `docs/APP_INIT_BOOTSTRAP.md`
loads at 008D8190. That doc recovered the fields the options file names; the table below adds the
rest, from the four reset writers, the serializer and the two commit paths.

| Offset | Type | Recovered name | Persisted as | Screen row | Evidence |
| --- | --- | --- | --- | --- | --- |
| +04h | int | language index | options file `Language` (by name) | page 1 row 0, screen +DCh | 008D48C0, 008D56C0 |
| +08h | bool | imperial | `imperial`, default 1 | page 1 row 1, +E0h | 008D41C5, 008D64CD |
| +09h | bool | subtitle | `subtitle`, default 0 | page 1 row 2, +E1h | 008D41C8, 008D6514 |
| +0Ch | int | hints (0..2) | `hints`, default 2 | page 1 row 3, +E4h | 008D41CC, 008D6555 |
| +10h | bool | cameraShake | `cameraShake`, default 1 | page 1 row 4, +E8h | 008D41D3, 008D659F |
| +14h/+18h | int | width, height | options file `Resolution` | page 3 row 0, +ECh/+F0h | 008D452B, 008D6086 |
| +1Ch | bool | NoLOD | options file `NoLOD` | none | 008D455D |
| +1Dh | bool | HiResShadow | options file `HiResShadow` | none | 008D455A |
| +1Eh | bool | fullscreen | options file `Fullscreen` | page 3 row 1, +F6h | 008D4593, 008D607D |
| +20h | float | masterVolume | `masterVolume` | page 2 row 0, +F8h | 008D41F8, 008D6623 |
| +24h | bool | sound enabled | not persisted | none | 008D5452 |
| +28h | float | musicVol | `musicVol` | page 2 row 1, +100h | 008D41FD, 008D6651 |
| +2Ch | float | sfxVol | `sfxVol` | page 2 row 3, +104h | 008D4202, 008D667D |
| +30h | float | speechVol | `speechVol` | page 2 row 2, +108h | 008D4207, 008D66A9 |
| +34h | float | unknown, 0x3F32B8C3 | not persisted | none | 008D4596 |
| +40h | bool | rumbleOff | `rumbleOff`, always written | page 4 row 1, +118h | 008D4830, 008D66D5 |
| +41h | bool | swapSticks | `swapSticks`, default 0 | page 4 row 4, +119h | 008D482D, 008D66F9 |
| +42h | bool | invertCameraY | `invertCameraY`, default 1 | page 4 row 2, +11Ah | 008D4827, 008D6738 |
| +43h | bool | invertPlaneY | `invertPlaneY`, default 1 | page 4 row 3, +11Bh | 008D482A, 008D677A |
| +44h | bool | swapMapSticks | `swapMapSticks`, default 0 | page 4 row 5, +11Ch | 008D4833, 008D67BC |
| +4Ah | bool | TargetIndicator | `TargetIndicator`, default 1 | page 1 row 7, +122h | 008D41D6, 008D65E1 |
| +54h | int | ObjectDetail | options file `ObjectDetail` | page 3 row 6, +12Ch | 008D4560, 008D5C2C |
| +58h | int | antialias sample count | options file `Antialias` | page 3 row 2, +130h | 008D4548, 008D6079 |
| +5Ch | int | antialias index | derived from `Antialias` | page 3 row 2, +134h | 008D4545 |
| +60h | bool | VSync | options file `VSync` | page 3 row 3, +138h | 008D454B, 008D6071 |
| +64h | float | gamma | `gamma`, always written | page 3 row 4, +13Ch | 008D454E, 008D693C |
| +68h | int | TextureDetail | options file `TextureDetail` | page 3 row 5, +140h | 008D4553, 008D5B7C |
| +6Ch | bool | Reflection | options file `Reflection` | page 3 row 8, +144h | 008D4573, 008D5C7C |
| +70h | int | unknown ocean value | not persisted | none | 008D4576, 008D5FB4 |
| +74h | bool | Clouds | options file `Clouds` | page 3 row 9, +14Ch | 008D4579, 008D5F7A |
| +78h | int | resolution index | derived from `Resolution` | page 3 row 0, +150h | 008D4528 |
| +7Ch | bool | waterDrops | `waterDrops`, always written | page 1 row 5, +154h | 008D41DF, 008D686C |
| +80h | float | markerAlpha | `markerAlpha`, always written | page 1 row 6, +158h | 008D41E2, 008D696C |
| +84h/+85h | bool | Shadow pair | options file `Shadow` | page 3 row 7, +15Ch | 008D4567, 008D5FE6 |
| +86h | bool | Foliage | options file `Foliage` | page 3 row 10, +15Eh | 008D45A3, 008D5F40 |
| +88h | int | ShaderModel | options file `ShaderModel` | none | 008D45A9, 008D421A |
| +8Ch | bool | post-effect flag | not persisted | none | 008D457F, 008D60D6 |
| +8Dh | bool | MotionBlur | `MotionBlur`, always written | page 3 row 11, +165h | 008D4585, 008D68F7 |
| +90h | int | oldFilmEffect | `oldFilmEffect`, always written | page 3 row 12, +168h | 008D458B, 008D68AE |
| +94h | bool | Firewall | options file `Firewall` | none | 008D8190 |
| +95h | bool | HardwareReported | `HardwareReported`, default 0 | none | 008D6827 |
| +98h/+9Ch | vector | downloaded content, stride 8 | `DownloadedContent` section | page 7 | 008D6A80 |
| +B0h | bool | XboxCompatibilityMode | `XboxCompatibilityMode` | page 4 row 0, +188h | 008D44C6, 008D699F |
| +B1h | bool | ShowSafeArea | `ShowSafeArea` | none | 008D69CA |
| +B2h | bool | CockpitMode | `CockpitMode` | page 1 row 8, +18Ah | 008D41D9, 008D69F5 |
| +B8h | int | ClanText | `ClanText` | none | 008D6A2B |

The screen-row column resolves the screen offsets `docs/OPTIONS_MENU_SCREENS.md` recorded. Every
page-4 row lines up with the byte run +40h..+44h in screen order +118h..+11Ch, which is what makes
the mapping more than a guess: the screen copies five consecutive bytes and the commit pushes four
of them into the input layer as a group.

## Persistence

### The options file, read only

`docs/APP_INIT_BOOTSTRAP.md` covers 008D8190 and its token table. Two additions:

- Correction from the 2026-09-10 assembly audit:008D6170 writes the options file. It is
  called by both008D8190 and008D64A0; the latter invokes it before any archive virtual.
  Its fopen("wt"), fwrite and fclose are at008D6424/008D6448/008D644E. The earlier
  hardware-derivation description and input-only conclusion were incorrect. This side
  effect is now required by the writer and concretely implemented; see
  `docs/SETTINGS_TEXT_PERSISTENCE.md`.
- The token list is one entry short. The installed file carries `HardwareReported`, and
  008D64A0 emits that key from +95h. The loader's else-chain was read at 008D846B-008D85FF; the
  `HardwareReported` arm was not located there, so whether the loader also parses it is open.

Checked against the installed game (read-only): `C:\Users\<user>\Documents\Battlestations-Pacific\options.txt`
holds `Language englishauthentic`, `Fullscreen 0`, `Resolution 2560 1440`, `Vsync 1`,
`ShaderModel 2`, `Antialias 0`, `Clouds 1`, `Foliage 1`, `Shadow 1`, `Reflection 1`,
`TextureDetail 2`, `ObjectDetail 2`, `SoundEnabled 1`, `Firewall 0`, `HardwareReported 1`. Every
token matches the loader's table, `Vsync` confirms the comparison is case-insensitive, and the
absence of any volume, control or gameplay key confirms that the options file carries only the
video and startup subset.

### The registry, one value and only as a fallback

Path B of 008D8190 reads `HKLM\SOFTWARE\Eidos\Battlestations Pacific` value `language`, REG_DWORD,
and only when the options file could not be opened. On the installed machine that key exists under
the 32-bit view with values `ApplicationDir`, `Patch` and `languages` (plural). **There is no
`language` value**, so on this install the fallback would fail the type/name test and the LCID
switch would never run; the loader would keep english. The key is otherwise untouched by the game:
nothing writes it.

### The real save path, 008D64A0

`void __thiscall FUN_008d64a0(cGameSettings* this, SettingsWriter* writer)`, RET 4, body
008D64A0-008D6AE0. It is the settings serializer, and it has **no direct caller in the binary**:
it is reached through a vtable. The writer interface it drives has three slots:

| Slot | Signature | Evidence |
| --- | --- | --- |
| +04h | `begin_section(Variant name)` | 008D64C4, 008D680F, 008D6A6C |
| +08h | `end_section()` | 008D6AC7, 008D6AD0 |
| +0Ch | `write_field(Variant key, Variant value)` | 008D6504 and every row |

Both variants are 8 bytes pushed by value: a type dword then the payload. Type codes are 0 string
(008D6A8E), 1 int (008D6AA8), 2 float (008D662D), 3 bool (008D64EB).

The body opens the `Options` section (00D15D50), emits the 24 scalar rows of the table above in
listing order, opens `keyboardSetup` (00D15C94) and delegates its body to
`006A51C0(BSP_InputSettings_GetSingleton(), writer)`, then opens `DownloadedContent` (00D15BFC) and
emits one row per vector entry whose key is the entry's `char*` at +4h (null falls back to the
empty buffer at 00F88A3C) and whose value is the loop index, then closes both sections.

**Rows equal to a default are omitted.** Each guarded row builds two variants on the stack and
calls `FUN_00BD5680`; a true result skips the emit. Floats and eight of the bool/int rows carry no
guard and are always written. The defaults column of the table above records which is which. The
consequence is that a reader has to reconstruct an absent key from the default, and that the
default in the serializer is *not* the same as the value the reset writers store: `invertCameraY`
and `invertPlaneY` are omitted at 1 but reset to 0, so a fresh control reset persists both.

The serializer runs the hardware derivation `FUN_008D6170` first (008D64A9), so a save can change
the hardware-derived fields before writing them.

The only caller chain found for the whole block is the player-profile save: `FUN_007FAE70` loads
ECX with 00F88980 at 007FAE97 before building the save blob. The blob writer itself was not opened.

## Reset writers

| Address | Signature | RET | Behaviour |
| --- | --- | --- | --- |
| 008D41C0 | `void __thiscall(void)` | 0 | +08h=1, +09h=0, +0Ch=2, +10h=1, +4Ah=1, +B2h=1, +7Ch=1, +80h=0.0f |
| 008D41F0 | `void __thiscall(void)` | 0 | +20h, +28h, +2Ch, +30h all = 0.5f (00CE3800) |
| 008D4520 | `void __thiscall(bool, bool)` | 8 | see below |
| 008D4820 | `void __thiscall(void)` | 0 | +40h=0, +41h=1, +42h=0, +43h=0, +44h=1, then a conditional tail |

008D4520 writes +5Ch=0, +58h=0, +60h=1, +64h=0.0f, +68h=2, +1Dh=0, +1Ch=0, +54h=2, +84h=1, +85h=0,
+6Ch=1, +70h=0, +74h=1, +7Ch=1, +8Ch=1, +8Dh=1, +90h=1, +34h=0x3F32B8C3, +86h=1, +88h=1
unconditionally. The first argument gates +78h=0 and the 640x480 pair into +14h/+18h. The second
argument is **inverted**: the `JNZ` at 008D4591 skips `+1Eh = 1`, so fullscreen is forced on only
when the argument is zero. The flags survive the intervening MOV/MOVSS/XORPS, which set no EFLAGS;
this was read from the listing, not the pseudocode. Both call sites pass `(0, 1)`.

008D4820's tail (008D4836-008D485C) jumps to 008D45D0 with ECX = this when the Xenon system manager
at 00F8ABE8 is non-null, `TRIV_body_00A3E500` (a getter for manager+28h) returns 2 and
`BSP_XenonSystemManager_HasSelectedUser` is true. 008D45D0 was not opened.

## The three small queries

- **008D4210** `char __thiscall(const cGameSettings* other)`, RET 4. Returns 1 when +88h differs,
  otherwise `+8Ch != other+8Ch`. Its one caller is the options commit 005F65C0, which runs it
  before deciding what an apply has to warn about. Ghidra drops the argument in the pseudocode; the
  signature is from the listing.
- **008D4260** `const char* __thiscall(void)`, RET 0. `FE.opt_game_metric` (00D15AAC) when +08h is
  non-zero, else `FE.opt_game_imperial` (00D15AC0). One caller, 005F4C90.
- **008D48C0** `const char* __thiscall(void)`, RET 0. `table[+04h * 0x20 + 0x0Ch]`, the lockit id of
  `bsp::LanguageEntry`, with the empty buffer at 00F88A3C as the null fallback. One caller,
  005F4C90.

**008D44C0** `void __thiscall(bool enabled)`, RET 4. Stores +B0h and mirrors it into the global at
0108FF20, then, when `((00E188A8)+1A08h)+4h` is non-null, pushes `X360COMP=true` (00CF7F38) or
`X360COMP=false` (00CF7F28) through `006B8AD0(text, 0, 0, 2)`. Five callers, including
`BSP_Game_OnInitOnce` and both press-start paths.

## 008D5430 BSP_Settings_ApplyAudio

`void __thiscall(void)`, RET 0, body 008D5430-008D56BF. Runs on every volume keystroke on the audio
page and as the first statement of the full commit. In order:

| Target | Source | Evidence |
| --- | --- | --- |
| `(00F8BBD8)+70h` | +24h | 008D5455 |
| `00A7A440(sound, float)`: stores +4Ch then applies group mask FFFFh | +20h | 008D546E |
| `(00F8BBCC)+218h` | +28h | 008D5483 |
| `(00F8BBCC)+21Ch` | +30h | 008D5490 |
| `00A864F0(((00E198AC)+50h), float)`: obj+14h | +28h | 008D54BA, both pointers guarded |
| `00A7ABF0(sound, FFFFh, float)`: walks the channel-group array at +98h/+9Ch | +2Ch | 008D54D9 |
| `00A7ACF0(sound, "Warnings", float)` then `00A7F8E0` | +30h | 008D5531 |
| `00A7ACF0(sound, "GUITestSpeech", float)` then `00A7F8E0` | +30h | 008D55BC |
| `00A7ACF0(sound, "GUIMusic", float)` then `00A7F8E0` | +28h | 008D5647 |
| `(004C1710())+10h`, clamped to [0, 1] | +20h | 008D568B-008D56B0 |

The three bus names are built as native strings with explicit lengths 8, 0Dh and 8 and copied from
00CE7848, 00CEF890 and 00D15B08. There is no FMOD call in this body; the sound system at 00F8BBD8
is the game's own wrapper.

The tail clamp is a plain `COMISS` pair: below 0 clamps to 0, above 1.0 (00D7A24C) clamps to 1.0.

## 008D5B50 BSP_Settings_ApplyAll

`void __thiscall(void)`, RET 0, body 008D5B50-008D6166, SEH frame with handler 00CA3103. Ten call
sites. The body carries a stack byte at [ESP+0Ch], cleared at 008D5EA4, set by three change
detections and passed as the last argument of the presentation-mode change. In order:

1. `008D5430` on the same object (008D5B6C).
2. `00B1FFB0(renderer, 2 - +68h)`: `renderer+1D84h`, so texture detail is stored inverted
   (008D5B80).
3. Renderer virtual `+F0h` with the float at +64h, the gamma push (008D5B9A).
4. `00699B80((00E188A8)+3Ch, +41h, +42h, +43h, +44h)`, RET 10h. Stores input+4C9h..+4CCh in that
   order and, when any of the four changed and input+520h is set, runs `00698730` then `00698A10`
   to rebuild the bindings (008D5BB6). The adjustor thunk 004B46E0 is what adds the +3Ch.
5. `004DCDF0(game)` (008D5BC1).
6. A 0x844-byte allocation and `009955F0`, guarded by a null content object at 00F8A304, a live
   Xenon manager and a selected user (008D5BE2). Not opened.
7. `(00E188A8)+19FCh` non-null: `that+4Ch = scales[+54h]` where the three-float table built on the
   stack is {0.5f, 0.7f, 1.0f} from 00CE3800, 00CE3E18, 00D7A24C (008D5C24).
8. `(00E188A8)+19F0h` non-null: `(that+A8h)+F8h = +6Ch`, the ocean reflection flag (008D5C7C).
9. `(004C1710())+14h = +09h`, subtitles into the UI singleton at 00F8AEF8 (008D5C85).
10. Language font: `table[+04h*20h+18h]` selects the font path at `+1Ch` (fallback 00F88A3C) or the
    empty string at 00CE3A0C, combined with the six-byte literal `Fonts\` (00CFEFA8) and pushed
    through `BSP_FontSystem_GetRegistry` and 00AC3610 (008D5D1C).
11. `BSP_Localization_LoadTable((00F8BC4C), table[+04h*20h+4])`, the lanfile (008D5DA1).
12. `00A94C50(CL)` where CL is 1 only when `game+634h` and +40h are both zero: rumble on
    (008D5DC8).
13. When 00F8A304 is non-null: with its byte +1Ch set, virtuals +24h, +14h(this+98h) and +20h;
    either way `"globals"` (00CF7774) through `BSP_Localization_RegisterTableName` and
    `BSP_Localization_ReloadTables(1)` (008D5DE7).
14. The three latches. 00F8A48 is a bit set; bit 0 seeds 00F88A44 from +8Dh, bit 1 seeds 00F88A40
    from +70h, bit 2 seeds 00F88A3D from +84h. A latch is seeded once and then used as the previous
    value, so a change is only ever seen from the second commit onward (008D5E86).
15. When `(00E188A8)+19ECh` is non-null: `00B72270(scene, "FoliageGroup", 0)` and, on a hit,
    `BSP_SceneNode_SetVisibilityFactor(node, +86h ? 1.0f : 0.0f, 1)`; when +19E8h is non-null,
    `00BBCFE0(cloudsys->+3Ch, +74h)`; when +19F0h is non-null the reflection store again plus, on a
    +70h change, `((that+A8h))->virtual +10h(+70h)` and the change flag; on a +84h change,
    `(00F8BBF0)->virtual +8h(+84h)` and the change flag; on a +8Dh change, the change flag only
    (008D5ED4-008D602A).
16. `00B0D580((00F8D39C), +8Dh, +58h)`: stores `+219h` then rebuilds the size-dependent render
    targets (008D603C). That +219h store is the same one `docs/OPTIONS_MENU_SCREENS.md` saw the
    motion-blur row mirror into.
17. Renderer virtual `+80h` returns the current back-buffer size; when it differs from +14h/+18h,
    `00B0FC00` (a thunk to 00B0F6E0) releases the size-dependent targets (008D6054).
18. `BSP_D3D9Renderer_ChangePresentationMode(+14h, +18h, +1Eh, +58h, +60h, changed)` (008D6092).
    The sixth argument is the accumulated stack byte, which is why the decompiler shows it as an
    unrelated local.
19. `(0109CF04)->virtual +0Ch(+1Eh, +14h, +18h)`, the platform window (008D60AF).
20. `XLiveOnResetDevice(00F8D394 + 1A28h)` when 00E198C4 is null (008D60B1). This is the device
    reset a video commit issues; the same call is duplicated in the screen's own A2h/A7h arms.
21. `00B107F0((00F8D39C), +8Ch, +8Dh, +58h)`, a 0x2838-byte body that was not opened (008D60E6).
22. `BSP_GuiManager_GetOrCreate` then `00AA4EF0` when 00E198C4 is null (008D60EB).
23. `00B0D080((00F8D39C), +90h)`: `+220h`, the old-film mode (008D610D).
24. `00439100()` returns the module directory; its length is measured inline and copied into the
    global native string at 0108FF24/0108FF28 (008D6112).

Calling conventions used above were taken from the listing. Ghidra's pseudocode for this body drops
the four control-byte arguments, mistakes the presentation-mode change flag for a local, and shows
`00BBDDF0` as consuming the pushed cloud flag when it is a three-byte getter (`MOV EAX,[ECX+3Ch];
RET`) whose caller's push belongs to `00BBCFE0`.

## What each subsystem receives

| Subsystem | Fields | Route |
| --- | --- | --- |
| Renderer 00F8D394 | +68h, +64h, +14h, +18h, +1Eh, +58h, +60h | 00B1FFB0, virtual +F0h, virtual +80h, 00B29E60 |
| Render targets 00F8D39C | +8Dh, +58h, +8Ch, +90h | 00B0D580, 00B0FC00, 00B107F0, 00B0D080 |
| Shadow system 00F8BBF0 | +84h | virtual +8h, only on change |
| Scene, game+19E8h/+19ECh/+19F0h/+19FCh | +74h, +86h, +6Ch, +70h, +54h | 00BBCFE0, 00B6DA70, direct stores |
| Sound 00F8BBD8 / 00F8BBCC / 00E198AC | +20h, +24h, +28h, +2Ch, +30h | 008D5430 |
| Input (00E188A8)+3Ch | +41h..+44h | 00699B80 into +4C9h..+4CCh |
| Rumble | +40h | 00A94C50 |
| UI singleton 00F8AEF8 | +09h, +20h | 004C1710 +14h and +10h |
| Localization 00F8BC4C | +04h | 00AA09D0, 00AA0D30, 00AA06D0 |
| Font registry | +04h | 007371D0 with `Fonts\` |
| Platform window 0109CF04 | +14h, +18h, +1Eh | virtual +0Ch |
| XLive | resolution change | XLiveOnResetDevice(00F8D394 + 1A28h) |

## Uncertainties

- The four input bytes at +4C9h..+4CCh gate the modifier vectors at +4D0h, +4E0h and +4F0h inside
  00698A10 and 006974F0, but which Lua modifier each vector holds was not pinned down. The names in
  this doc come from the serializer keys and from the page-4 row order, not from the input body.
- +34h, +70h and +8Ch have no recovered meaning. +34h is only ever written (008D4596); +70h reaches
  an ocean virtual; +8Ch reaches 00B107F0.
- 008D45D0 (the 008D4820 tail), 009955F0, 00B107F0 and 004DCDF0 were not opened.
- Whether the loader parses `HardwareReported` is open; the installed file contains it and the
  serializer writes it, but the arm was not found in the loader's else-chain.
- 008D64A0's caller is a vtable slot that was not resolved, so which object owns the writer, and
  therefore what the serialized block is stored in, is inferred from 007FAE70 alone.
- The `Language` value `englishauthentic` is a single lanfile token compared by 008D56C0 against
  language-table entry +0h/+4h; the language table's own contents were not enumerated here.

## Reconstruction

`include/bsp/game_settings.hpp` and `src/game_settings.cpp`. `bsp::GameSettingsBlock` composes the
existing `bsp::GameSettings` from `bsp/app_bootstrap.hpp` (the options-file projection) with four
new sub-structs for the offsets only the screens and the serializer touch; no field is duplicated.
The key table is data (`settings_persistence_keys_008d64a0`), the four resets, the three queries and
the compatibility setter are plain functions, and both commits run over injected hosts with one
method per native call site, in the style of `bsp::run_application_frame`. Nothing invents a global:
the state the commit reads from outside the object is an explicit `SettingsApplyEnvironment`, and the
latched globals are an explicit `SettingsCommitLatches`.

Not ABI compatible, not game validated. `008D6170`, `009955F0`, `00B107F0` and the
`keyboardSetup` body are host responsibilities rather than reconstructions.

One test case was added to `tests/math_tests.cpp`: the omit-when-default rule is the only place
where a wrong constant silently drops a persisted field, and the control reset is the case where
the reset value and the serializer default disagree.

## State reached

| Address | Name | State |
| --- | --- | --- |
| 008D5B50 | BSP_Settings_ApplyAll | exported, analyzed, reconstructed, build-tested |
| 008D5430 | BSP_Settings_ApplyAudio | exported, analyzed, reconstructed, build-tested |
| 008D41C0 | BSP_Settings_ResetGameDefaults | exported, analyzed, reconstructed, build-tested |
| 008D41F0 | BSP_Settings_ResetAudioDefaults | exported, analyzed, reconstructed, build-tested |
| 008D4520 | BSP_Settings_ResetVideoDefaults | exported, analyzed, reconstructed, build-tested |
| 008D4820 | BSP_Settings_ResetControlDefaults | exported, analyzed, reconstructed except the 008D45D0 tail, build-tested |
| 008D4210 | BSP_Settings_VideoModeDiffers | exported, analyzed, reconstructed, build-tested |
| 008D44C0 | BSP_Settings_SetXboxCompatibilityMode | exported, analyzed, reconstructed, build-tested |
| 008D48C0 | BSP_Settings_GetLanguageLockitId | exported, analyzed, reconstructed, build-tested |
| 008D4260 | BSP_Settings_GetUnitLabelKey | exported, analyzed, reconstructed, build-tested |
| 008D64A0 | BSP_Settings_WriteToStore | exported, analyzed, reconstructed, build-tested, installed-file-checked |

The installed-file check covers the options file and the Eidos registry key only; it establishes
that the loader's token table matches the shipped file and that the registry `language` fallback
would not fire on this install.

## Follow-up packets

- `settings_hardware_derivation`: 008D6170, 008D6DC0, 008D6AF0, 008D79A0. Files
  `docs/SETTINGS_HARDWARE_DERIVATION.md`, `include/bsp/settings_hardware.hpp`. Contract: what the
  no-options-file path derives from detected hardware, which fields it overwrites, and how
  `HardwareReported` gates it.
- `profile_settings_blob`: 007FAE70, 007FA710, 007FF100, 007FEFC0, 008D6C00. Files
  `docs/PROFILE_SETTINGS_BLOB.md`, `include/bsp/profile_blob.hpp`. Contract: the writer object whose
  vtable 008D64A0 drives, the save-blob format, and the read path that restores the settings block.
- `render_post_effect_rebuild`: 00B107F0, 00B0D580, 00B0FC00, 00B0F6E0, 00B0D080. Files
  `docs/RENDER_POST_EFFECT_REBUILD.md`, `include/bsp/post_effect_rebuild.hpp`. Contract: what the
  motion-blur, antialias and old-film fields actually change in the render-target set.
- `input_stick_modifiers`: 00699B80, 00698730, 006974F0, 006A51C0. Files
  `docs/INPUT_STICK_MODIFIERS.md`. Contract: which Lua modifier vector each of input +4C9h..+4CCh
  gates, and the keyboardSetup serialization body.

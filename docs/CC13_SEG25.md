# Segment 25: plane-spawn, please-wait, scoring and powerup-manager screens (cc13-seg25)

Segment 25 is 0060CB60-0061D330. Code: `include/bsp/seg25_cc13.hpp`, `src/seg25_cc13.cpp`. Status:
**reconstructed and Win32 build-tested only**. No fixture, nothing run in game, not ABI-compatible.
Names are hypotheses (the ledger's Ghidra names). No agent/argon* branch adds addresses in the range
(checked against origin/main on 2026-10-14), and the lease table had no other holder.

Conventions follow docs/CC13_SEG20.md and docs/CC13_SEG19_21.md. Objects are raw image storage
(`Raw::at`). Every engine routine the image calls is a named `Seg25Host` method invoked at the same
point, and vtable pointers are written as their image addresses. Native-string temporaries are built
exactly as the image inlines them: 0041DD40 resize, then memcpy of length + 1. They are released
through 00419CC0 / 00BD1510. SEH unwind paths are not modelled.

Many of the segment's HUD routines were already reconstructed in `src/hud_widgets.cpp`
(cc13-hud1 and cc13-sweep60). The functions below had no code. Those still without code are listed
under Skipped.

## Reconstructed

### Plane-spawn and please-wait screens

| address | C++ | notes |
|---|---|---|
| `0060D130` | `hud_plane_spawn_screen_destroy_0060d130` | vtables CF4A7C/CF4A64, page +28h removed, base destructor 0060C650 |
| `0060D1C0` | `hud_plane_spawn_screen_register_0060d1c0` | GUI_plane_spawn into +28h |
| `0060D280` | `hud_plane_spawn_screen_collect_children_0060d280` | pushes &+28h |
| `0060D290` | `please_wait_screen_construct_0060d290` | 004F7180, vtable CF4AB8 |
| `0060D2B0` | `please_wait_screen_get_screen_id_0060d2b0` | 53h |
| `0060D2C0` | `please_wait_screen_is_self_managed_0060d2c0` | MOV AL,1 |
| `0060D330` | `please_wait_screen_destroy_0060d330` | page +8h removed, 004F71A0 |
| `0060D430` | `please_wait_screen_register_0060d430` | _PleaseWait into +8h, Background_Icon into +0Ch, +10h = -1.0f |
| `0060D560` | `please_wait_screen_collect_children_0060d560` | pushes &+8h |

### Scoring screen (5Eh) and score rows

A score row is 24h bytes: native strings at +0h and +8h, a dword at +10h, a byte at +14h, and dwords
+18h..+20h.

| address | C++ | notes |
|---|---|---|
| `0060DE00` | `hud_scoring_screen_on_listbox_activate_0060de00` | ECX = screen+8h; Close 0060DD10(screen), RET 8 |
| `0060DE10` | `hud_score_row_destroy_0060de10` | strings +8h then +0h to the pool |
| `0060DFB0` | `hud_score_row_assign_0060dfb0` | strings skipped when the same object, tail fields copied |
| `0060F020` | `hud_score_row_copy_construct_0060f020` | strings zeroed then copied |
| `0060FD90` | `hud_scoring_screen_construct_0060fd90` | vtables CF4B98/CF4B84/CF4B64; +20h..+28h, +30h..+38h zeroed |
| `0060FE00` | `hud_scoring_screen_get_screen_id_0060fe00` | 5Eh |
| `0060FE20` | `hud_scoring_screen_is_self_managed_0060fe20` | [game+1FE4h] == 0 |
| `0060FE40` | `hud_scoring_screen_collect_children_0060fe40` | pushes &+44h |
| `0060FE50` | `hud_scoring_screen_destroy_0060fe50` | page +44h removed and zeroed, row vectors +2Ch then +1Ch (0060F9E0), 004F71A0 |

### Powerup-manager screen (4Fh, vtable 00CF4DEC) and neighbours

| address | C++ | notes |
|---|---|---|
| `00611750` | `powerup_manager_is_scrolling_00611750` | +1F9h or +13Fh |
| `00611AE0` | `powerup_manager_tick_reset_fade_00611ae0` | +208h -= 2dt; four alpha/vslot4C pairs (see the arithmetic) |
| `00611BF0` | `powerup_manager_hide_info_page_00611bf0` | hides +B0h |
| `00612B70` | `powerup_manager_tick_label_fade_00612b70` | label slide-in (see the arithmetic) |
| `006140D0` | `native_string_destroy_storage_006140d0` | pool release |
| `00614EE0` | `native_string_int_construct_00614ee0` | {string, dword} from a string and a dword pointer |
| `00614F30` | `native_string_int_copy_construct_00614f30` | copy of the same pair |
| `006161F0` | `powerup_carousel_init_006161f0` | zeroes the carousel state |
| `00618EA0` | `powerup_manager_destroy_00618ea0` | pages +B0h/+24h; lists +264h (00487110) / +258h (00613DF0); four maps; nine pointer vectors; 004F71A0 |
| `00619160` | `powerup_manager_get_screen_id_00619160` | 4Fh |
| `00619170` | `powerup_manager_collect_children_00619170` | pushes &+B0h then &+24h |
| `00619AE0` | `powerup_manager_refresh_second_rows_00619ae0` | row frames from the map +1CCh, rotated by +1ECh |
| `0061A480` | `powerup_manager_construct_0061a480` | zeroes, four map heads, two list heads |
| `0061AFF0` | `powerup_manager_tick_second_fade_0061aff0` | +1E8h -= dt; four alphas; refresh when it runs out |
| `0061B840` | `powerup_manager_on_listener_refresh_0061b840` | ECX = screen+8h, 0061B0F0(screen), RET 4 |
| `0061C800` | `rewards_screen_on_widget_hover_0061c800` | hover feedback by widget kind (3, 6, 12h), three locked tabs |

The four maps in the constructor have their head nodes at +1B8h, +1C4h, +1D0h and +1DCh. They are
built by 00613AE0 (Isnil at +19h) and 00613B30 (Isnil at +15h), alternating. The destructor erases
them in reverse order with 00616060 and 00616920.

## Notes on the arithmetic

- 00611AE0 computes v = (float)(+208h - 2dt). The bytes `DE E9` at 00611AF1 are FSUBP ST1,ST0. Above
  0.5f (00CE3800) the alpha is 2v - 1.0 (double 00D7A210); otherwise it is 1.0 - 2v (`DE E1`,
  FSUBRP ST1,ST0). Each alpha goes to 00AA8610 on widgets +28h..+34h. Half of it (double 0.5 at
  00D7A280) goes to vslot4C on widgets +38h..+44h. The decompile hides the 00AA8610 receiver; the
  listing shows ECX = [EDI-10h].
- 00612B70 computes t = (float)(dt * 8.0 + +230h), with 8.0 the double at 00CE3DB0. At t >= 1 (JC
  skips when below or unordered), label +D4h is placed at x(+E4h) - +254h * 0.5 (`DE E9` at
  00612C36), keeping its own y and z. The width test divides [+D4h]+114h by 960.0 (00CEC380). +234h
  is set to width + 0.013000000268220901 (00CED090). The result of the 00AA6750 call on +ECh just
  before vslot58 is never read; the call is still made.
- 0061AFF0 uses the same 2v - 1 / 1 - 2v pair (`DE E1` at 0061B0B2) on widgets +58h..+64h. Its
  final test (UCOMISS, LAHF, TEST AH,44h, JP) clears +1E4h only when +1E8h == 0.
- These sums are done in double where the image uses 64-bit x87. Every operand here is a float, so
  each intermediate is exact in double and the single rounding to float matches.

## Skipped

- **Long or string-building bodies**: 006113A0 AnimateScroll, 00611770 TickSlide (x87 with register
  state carried through the loop), 00612DA0 RefreshPromptWidgets, 00614320 BindSupportWidgets
  (string concatenations through 004260B0 / 0043C130 / 004261A0), 00616C70 FillSecondRows, 00617470
  UpdatePowerupBars, 006179D0 RebuildSlots, 006191A0 RebuildSecondSlots and 0061B0F0 RebuildLists.
  These are left for an x87 or long-body pass.
- 0061A400 AssignLists takes two vectors by value on the stack. The decompile's parameter recovery
  is unreliable, so a listing pass is needed first.
- 00618980 is tagged cg_vector_deleting_dtor but named RefreshSelectedLabel. The tag and the name
  disagree, so it needs a check before it is reconstructed.
- The compiler-generated deleting destructors and adjustor thunks (0060D1A0, 0060D410, 0060E080,
  0060FE10, 0060FFA0, 00614F80, 0061A6A0), and the trivial body 0061C7F0.

## Verification of existing segment 19 code (follow-up from cc13-seg19-21)

Ledger records were added for eight functions in `src/menu_screens_2.cpp` and three in
`src/frontend_entry.cpp`. Each record's notes say what was compared.

- **Verified**: 00574100, 0057A9C0, 0057AC10, 00576BE0, 00578320, 00579980 and 00576D20.
- **Verified for structure and call sequence only**: 00577650.
- **Verified semantic models**: 0057C250 and 0057D0D0.
- **Labelled substitution**: 0057CB60. A created flag stands in for the [00E194B4] null test, and the
  mode-1 vtable[58h] call is made without the aspect-corrected rect argument.

No code was changed: frontend_entry.cpp is not a cc13 file, and no real defect was found in
menu_screens_2.cpp. 00576D20 differs from the image in one case. When no record matches, the image
reads the players and masks through end(), while the C++ reads them from the result.

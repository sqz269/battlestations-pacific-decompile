# Front-end prompt records and completion
Addresses: `00531b00`, `00532a20`, `00530650`, `004b6e50`, `00533120`, `00530f40`, `00530f90`, `00532360`, `00532c50`, `00532cb0`, `00532dc0`; contracted layout tail `00530a60`.

All descriptive names are hypotheses, not recovered symbols. The implementation is
`include/bsp/frontend_prompts.hpp` / `src/frontend_prompts.cpp`. It uses the existing
`NativeString`, `ClockTimestamp`, timestamp subtraction, `MenuCommandScreen` and
`close_menu_command_screen` interfaces. It is a host reconstruction, not a binary hook.

## What the startup callers need

`00531b00` raises a prompt in a numeric priority slot, after attempting to dismiss slots
0 through 6. It does **not** guarantee the other records are empty: nondismissible
records can immediately restore themselves while those attempts run. `00532360` selects
the highest occupied slot. The seven records start at menu+60h, each 48h bytes.

The two title/pause gates are ordinary record fields:

| Menu field | Exact interpretation |
| --- | --- |
| +188h | `records[4].kind`, since 60h + 4*48h + 8h = 188h |
| +218h | `records[6].kind`, since 60h + 6*48h + 8h = 218h |
| +258h | current highest occupied slot, or -1 |
| +25Ch | OR of kinds 1, 2 or 4 over occupied slots through current |

The old eight-argument signature in `GAME_SESSION_POLLS.md` is incomplete. The
ten-logical-argument call in `PRESS_START_SCREEN.md` is confirmed by `00531b00`'s
own listing and RET 2Ch. Its fifth argument is **unread**, not a recovered policy flag.

```
ECX = menu
00531b00(slot, &message, kind, callback, unused_dword, &title,
         timeout_float, timeout_result, countdown_key_by_value, dismissible_byte)
```

The by-value native string occupies two stack dwords; together these are eleven
dwords / 44 bytes. The callee releases that owned value after refreshing. Callback
addresses use the result in ECX; they do not use a C++ callable object's ABI.

| Caller | Slot | Kind | Message / callback |
| --- | --- | --- | --- |
| Press start 0067d0f5 | 2 | 2 | `FE_xbox.xsm_profilechanged` |
| Press start 0067cded | 0 | 3 | `globals.saving_xbox` |
| Window close 004ca2f0 | 6 | 1 | `FE_pc.main_quit_confirm`; 004bbc50 exits only for result 1 |
| Session polls 004db290 | usually 2, storage 0 | 2 or 1 | See callsites in `GAME_SESSION_POLLS.md`; its former `kind` column is the slot |

## Record layout and ownership

| Offset | Field | Established behavior |
| --- | --- | --- |
| +00h | NativeString message | copied on raise, retained on dismissal |
| +08h | int kind | zero means unused |
| +0Ch | callback address | suppressed on clear for kind 3 or dismissible=0 |
| +10h | NativeString title | localized source for menu+1Ch widget |
| +18h | byte timed | timeout != 0, including NaN |
| +19h..1Fh | padding | not initialized by 00530f40 |
| +20h | 16-byte timestamp | ticks=0, frequency=1 in constructor; sampled on every refresh activation |
| +30h | float timeout | zeroed on clear |
| +34h | int timeout result | not initialized by record constructor; assigned on every raise |
| +38h | NativeString countdown key | key for game+1A0Ch localization substitution |
| +40h | byte dismissible | true initially; false means preserve on attempted dismissal |
| +41h..43h | padding | not initialized by 00530f40 |
| +44h | int original slot | 7 initially and after clear |

`00530f90` destroys the three strings in reverse field order: +38h, +10h, +0h. It
does not clear their headers. `NativeString` has no implicit cleanup; the reconstructed
record destructor body and the separate host cleanup make ownership explicit. Queued
nodes use `std::list` as the STL allocation/iterator boundary, with owned native strings.
The Win32 record size and timestamp/countdown offsets are compile-time checked.

`00533120` chains screen construction, sets interface vtables at +0/+8/+C/+10h, sets
+3Ch=true, constructs the two string vectors at +40h/+50h and seven records, clears
the scratch vector at +26Ch/+270h/+274h, initializes the list at +278h, sets +284h=-1
and +288h=false. It **does not initialize +258h..+264h**. The C++ constructor projection
initializes owned string/record/list state; harmless host defaults for later entry-owned
fields are marked separately. Native vector allocation traces and interface-vtable
construction are not reconstructed. The register/enter routines remain required.

The two seven-string tables, in order, are:

```
globals.yes, globals.no, globals.accept, globals.restartmission,
globals.exittomenu, globals.exitgame, globals.back

globals.dialog_yes, globals.dialog_no, globals.dialog_accept,
globals.dialog_restartmission, globals.dialog_exittomenu,
globals.dialog_exitgame, globals.dialog_back
```

## Dismissal, queue restoration and results

`00532a20(menu, slot)` does nothing for kind 0. Otherwise it captures the callback.
Kind 3 suppresses this callback; dismissible=0 also suppresses it and copies the record
to the pending list's back. The record is then marked unused, its callback/timer are
cleared, dismissible becomes true and original slot becomes 7. Message/title/countdown,
timestamp and timeout result remain untouched. A surviving callback runs with ECX=2
**before** refresh. This is also the result used by the Accept button; result 2 is not
a universal cancellation-only code.

If refresh finds no occupied record but the list is nonempty, it raises the front record
again, copying its by-value countdown key, and then erases the current first list node.
The node is deliberately re-read after raise. This order, the calls within each of the
seven dismiss attempts, and the fresh clock sample on restoration are preserved.

`00532c50(menu, result)` is actual completion: capture the callback, clear it, force
dismissible=true, then call `00532a20` for current. Only after the resulting refresh
does it invoke the captured callback with the supplied result. Thus even a kind-3
callback can run on explicit completion. Finally it updates input with 0.0 seconds,
probes dynamic-device buttons, and writes the result to input table+DDh.

## Refresh policy and button meanings

Refresh first detaches/disables existing navigation if the screen is active. When the
previous modal byte is false it captures game+634h. It recomputes the selected slot and
modal byte. In a mission with zero local players, it temporarily clears modal while
calling `004cd0f0(true,false,true)`, then restores modal and the saved cinematic byte.
When no modal prompt remains it restores game+634h if needed. These field transitions
are observable to callbacks and are kept in order.

With nothing active/pending it changes input mapping 1Ch to 0 and runs the existing
close-screen policy: optional virtual exit, wanted/active=false, visibility commit.
An active prompt sets mapping 1Ch to 5, marks wanted/active=true, commits, calls virtual
entry, samples its start timestamp, and sets the title and message localized sources.

| Kind | Buttons +25Dh / +25Eh / +25Fh / +260h | Modal |
| --- | --- | --- |
| 1 | Yes / No | yes |
| 2 | Accept | yes |
| 3 | none | no |
| 4 | Yes / No / Restart | yes |
| Other nonzero | keeps previous button bytes | no, unless another occupied record is modal |

Slot 6 replaces Yes/No labels with Exit Game/Back. Else kind 4 replaces No with Exit
to Menu. Input mode +264h selects one of the two tables. Mode 0 rebuilds navigation
in Yes, Accept, Restart, No order, chooses requested focus or last item, installs
callback 006964b0 and attaches the screen+8h owner. Widget operations and the tail
layout at 00530a60 remain concrete engine contracts, not no-op implementations.

## Update and widget callback

The missing vtable+20h body `00532dc0` was recovered from disk bytes. It synchronizes
the background widget's +34h visibility with menu+3Ch. Timed prompts compute elapsed
from clock virtual+20h and `00530890`, using the existing x87 timestamp conversion.
Expiry is **elapsed > timeout**, not >=; unordered comparisons do not expire. Expiry
completes with record+34h and returns immediately.

Before expiry it creates the localization scope, subtracts elapsed from timeout,
rounds through CRT 00bf85b0, converts using CVTTSS2SI, installs the countdown key's
integer, destroys the scope and refreshes the message. The exact CRT rounding helper
is deliberately a host contract; its current library name has not been changed or
silently replaced by `ceil`/`floor`.

Input actions are checked in this short-circuit order:

1. No enabled: F7h or 4Bh -> result 0.
2. Accept enabled: F8h, 4Ah or 4Bh -> result 2.
3. Restart enabled: F5h -> result 3.
4. Only in input mode 1, Yes enabled: F6h or 4Ah -> result 1.

Non-timeout completion continues the update with freshly read button bytes; callbacks
may have changed the active prompt. Visible widgets receive virtual+40h with the frame
delta. In title state 2, primary device (1,0), button 0, can dispatch each enabled
widget whose +D8h is nonzero through screen+10h virtual+4h. The update ends with
`004c1e90(2)` / `00427190`.

`00532cb0` receives ECX=screen+10h, not screen. Listing LEA ECX,[EDI-10h] proves the
adjustment. Event virtual+5Ch must return 3; virtual+4Ch receives 1.0f. It reads the
first byte at event+F0h (null falls back to the zero byte at 00E18E74): A7h means
Restart when enabled; A2h chooses Accept first, then Yes; A3h completes with 0 without
checking the No byte. Each subsequent event-code read is preserved.

## Evidence, status and boundaries

Live read/export tools verify project `bsp`, program `/battlestationspacific.exe`,
backed by `C:/Users/sqz269/bsp.gpr`, through the configured client guard. No worker
Ghidra mutation, annotation, function definition or save occurred.

| Address | Status in this packet | Native ABI |
| --- | --- | --- |
| 00531b00 | reconstructed, build-tested, restoration fixture-tested | ECX menu, RET 2Ch |
| 00532a20 | reconstructed, build-tested, restoration fixture-tested | ECX menu, RET 4 |
| 00530650 | reconstructed, build-tested, restoration fixture-tested | ECX menu, RET |
| 00532360 | controller reconstructed, build/fixture-tested; GUI layout tail contracted | ECX menu, tail JMP 00530a60 |
| 00532c50 | reconstructed, build/fixture-tested | ECX menu, stack result, RET 4 |
| 00532cb0 | reconstructed, build-tested; event path not fixture-tested | ECX menu+10h, stack event, RET 4 |
| 00532dc0 | reconstructed from disk bytes, build/fixture-tested | ECX menu, stack float, RET 4 |
| 00530f40 | constructor represented by record member initializers, Win32 layout build-tested | ECX record, EAX record, RET |
| 00530f90 | reconstructed, build/fixture-tested through queued record destruction | ECX record, RET |
| 00533120 | constructor projection fragment, build/fixture-tested | ECX allocation, EAX allocation, RET |
| 004b6e50 | previously reconstructed; existing close helper reused | ECX screen, tail JMP 004f83b0 |

Undefined starts at analysis time (final instruction address and length):

- 00530f40..00530f81: RET at 00530f81, length 1; following bytes CC padding.
- 00532dc0..00533117: RET 4 at 00533115, length 3; following bytes CC padding.

00532360 already includes the shared tail 00530a60..00530c17 in its Ghidra body; this
is not an undefined gap. Snapshot absence alone does not mean a live function is absent.
00532cb0 exported successfully despite its older snapshot lacking the entry.

The one local fixture (`local/frontend_prompts_probe.cpp`) exercises preservation,
notification acceptance, refresh-before-callback ordering, restored start time, strict
timeout equality and final busy completion. It links the Release Win32 `bsp_core.lib`;
no repository test target was added. Source/command/binary hashes and results are in
`reports/frontend_prompt_screen.json`. Both existing CTest checks pass, and all eight
native differential seed byte checks match disk. No game or visual validation occurred.

Limits: record allocation/STL machinery, exact exception unwinding, malformed containers,
GUI creation/register/entry/layout, rendering, navigation internals, localization and
input-device internals remain host contracts. Callers must supply valid slot/table/current
indices and native callback behavior. The record is Win32-layout checked; the host screen
is a projection whose kind mirrors are updated by its public operations. This module is
not ABI-compatible with the game and does not make the startup reconstruction playable.

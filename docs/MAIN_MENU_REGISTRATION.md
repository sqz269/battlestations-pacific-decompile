# Main-menu screen registration

`register_main_menu_screen_00582f30` reconstructs the complete ordinary caller
`00582F30..00583091`. Native ECX is the containing main-menu screen; it takes
no stack arguments and ends with a one-byte RET. The 354-byte body contains
86 listed instructions and no flow gaps. This is a new C++ reference interface,
not the original object/vtable ABI, SEH behavior, or a complete renderer.

The implementation surrounds the existing spacing producer with actual base
registration, fresh current +14 dispatch, current page visibility, native reset
stores, and the required existing presentation-mode service. No second screen,
GUI tree, settings object, or renderer is created.

## Screen publication and callback order

`004F71D0` was read completely at `004F71D0..004F71E1`: it retains the incoming
screen in ESI, dispatches its current +00 at `004F71D7`, and stores that same
screen at `00E18B60 + returned_id*4` at `004F71D9`. The reconstruction calls the
current-id service and then reuses `register_front_end_screen_004f71d0` with the
actual supplied `FrontEndScreen` and registry. Valid ids 0..94 are the supported
domain; invalid ids throw before publication instead of reproducing native
out-of-bounds writes. Existing occupants are replaced, as in the base routine.

After publication, native `00582F3C..00582F43` reloads the screen's current
table and dispatches +14. This dispatch is required even if current +00 changed
the table. `MainMenuRegistrationServices::screen_current14` must resolve that
current implementation on the same screen. The normal authored profile reaches
the existing `bind_main_menu_layout_005861b0`; arbitrary unresolved profiles
must stop there, rather than silently using this binder for every table.

Live table bytes at `00CEFC5C` are
`70 05 59 00 70 75 4F 00 80 75 4F 00 60 0D 59 00 30 2F 58 00 B0 61 58 00`.
They establish +00=`00590570`, +10=`00582F30`, +14=`005861B0` for this profile.
The entire `00590570..00590575` leaf is MOV EAX,1 / RET, but the caller does
not replace current dispatch with an unconditional id 1.

`compute_main_menu_spacing_00582f45_fragment` then uses the same authored Text
owners and existing layout +2B0/+2B4 cells. Its position/height accessors and
x87 binary64-spill schedule are documented in [MAIN_MENU_SPACING.md](MAIN_MENU_SPACING.md).
It returns the +470 receiver captured at `00582FDD`. The caller dispatches that
owner's current +34(false) at `00582FF1`. It neither reloads a different +470
receiver nor writes a visibility flag directly. The actual base current +34
body `00AA8530..00AA85AE` walks ancestor current +38 predicates, propagates
visibility, and applies node factor through `00B6DA70`; derived dispatch and
its resource requirements remain those of the existing GUI owner runtime.

## Canonical stores

Only after current +34 returns does the caller read `00CE54A0`. Native MOVSS
copies are carried as raw bits, preserving NaN payloads and signed zero without
an x87 conversion. The live values were `3E4CCCCD` at `00CE54A0` and `3EC7AE14`
at `00CEF7BC` when inspected; the implementation reads their supplied live
references rather than substituting these snapshot constants.

| Site | Native destination | Existing storage written |
| --- | --- | --- |
| `00582FFB` | +64 = 0 | `selection.field_64` |
| `00582FFE` | +68 = 0 | `selection.field_68` |
| `00583001` | +2C8 = null | `layout.objective_page_2c8` |
| `00583007` | +2EC = null | `layout.objective_background_2ec` |
| `0058300D` | +190 = captured `00CE54A0` | `map_geometry.base_offset[0]` |
| `0058301D` | +194 = fresh `00CEF7BC` | `map_geometry.base_offset[1]` |
| `00583028` | +198 = positive zero | `map_geometry.base_offset[2]` |
| `00583030` | +1A4 = positive zero | required same-screen `map_offset_x_1a4` reference |
| `00583038` | +1A8 = positive zero | `widget.screen.map_offset_y` |
| `00583040` | +1AC = positive zero | required same-screen `map_offset_z_1ac` reference |
| `00583048` | +110 = null | `selection.active_mission_group_110` |
| `0058304E` | byte +565 = 0 | `widget.screen.dlc_campaign` |
| `00583054` | byte +564 = 0 | `widget.screen.us_campaign` |

The rows execute in that order. The first float source aliases the already
existing `command.zoom_in_00ce54a0` reference. `MainMenuMapGeometryState` supplies
the existing base-offset array; the two residual offset references add no
storage. The registry base, geometry, selection, its widget screen/layout, and
residual references must belong to one actual main-menu instance. +6C, map zoom,
and other fields are not registration stores in this body.

## Required renderer boundary

The entire `00B29E60..00B2A063` callee was read in assembly after its pseudocode
showed a misleading saved-register return. It contains 141 instructions and
no gaps. ECX is the actual renderer; its six stack operands are width, height,
fullscreen byte, multisample type, sync byte, force byte. Both returns use
RET18 (`00B29F3E` and `00B2A061`, each length 3), and ordinary AL is true.
The menu caller ignores AL.

The caller captures the settings in the native order:
`vsync_60` (`00F889E0`, `0058305A`), `antialias_58` (`00F889D8`, `00583061`),
`fullscreen_1e` (`00F8899E`, `00583067`), `height_18` (`00F88998`, `00583071`),
then `width_14` (`00F88994`, `00583078`). Only then does it resolve the actual
renderer publication `00F8D394` at `00583080` and invoke the existing
`SettingsApplyHost::renderer_change_presentation_mode(width,height,fullscreen,
samples,vsync,true)` at `00583087`. The settings reference is the same live
`GameSettings` object; its Boolean fields use the existing normalized domain.
The renderer resolver is an identity lookup, not an extra game operation.

Force=1 bypasses the callee's no-change return. The callee holds the actual
tracked critical section at renderer+199C, conditionally replaces nonzero
dimensions, updates presentation fields, and either calls device recreation
`00B29670` or publishes pending reset `0108D4B8`. It then rereads the current
presentation dimensions into distinct +1A20/+1A24 fields, invokes current +F0
gamma and current +2C, conditionally calls device +A4 EndScene, and releases
the lock/optional guard. A few parameter stores cannot stand in for this body.

This packet does not implement that renderer infrastructure. The existing
settings service remains mandatory and must execute the actual operation on
the resolved receiver or throw when this boundary is reached. See
[NATIVE_RENDERER_PARAMETERS_NEXT.md](NATIVE_RENDERER_PARAMETERS_NEXT.md), especially
the mode-update boundary. Named base/binder/GUI helpers are reused with their
documented supported domains; names alone do not prove all providers complete.

## Scope and verification

Earlier publication and stores are retained if a later current callback,
Text/resource operation, or renderer boundary cannot finish. No whole-caller
preflight, rollback, successful default, or pending continuation is invented.
All borrowed identities must remain alive across callbacks. Original invalid
pointer faults, SEH/unwinding ABI, and arbitrary malformed Boolean storage are
outside this typed normal domain.

The worker used only verified read-only BSP/Ghidra wrappers. Call-site rows,
body boundaries, live bytes, and validation outcomes are in
`reports/main_menu_registration.json`. The new source is appended to the shared
CMake registry without a whole-file lease under the current coordination rule.
No new permanent tests are added.

`scripts/build.ps1` passed MSVC Win32 Release with `/W4 /WX /fp:strict`,
including compilation of the new source. The first CTest run passed 1/1;
after `verify-seeds` matched all eight live/disk seeds, the final run passed
`reconstructed_math` and `native_math_differential` (2/2). The ignored logs are
`local/menu-registration-build.log` and `local/menu-registration-build-final.log`.
The call report passed all eight direct rows; the three indirect dispatches
were checked manually against assembly and the current profile table. These
existing tests do not execute the complete registration chain.

Executable integration remains separate: `src/game_hosts_menu.cpp`'s
`register_screen(void*)` currently registers a base and starts manual page
attachment for the main-menu case. This packet exposes the actual full caller;
it does not claim that shortcut runs it, that every required service is bound,
or that complete registration, rendering, or gameplay has been validated.

# Main-menu registration spacing fragment

`compute_main_menu_spacing_00582f45_fragment` reconstructs only the spacing
producer inside `00582F30`, from instruction `00582F45` through the six-byte
store at `00582FE6` (inclusive byte end `00582FEB`). The containing registration
function uses ECX=main-menu, no stack arguments, and a bare RET at `00583091`.
This is a new C++ interface, not that native ABI or the full registration body.

The primary vtable at `00CEFC5C` has +10=`00582F30` and +14=`005861B0`:
live bytes at `00CEFC6C` are `30 2F 58 00 B0 61 58 00`. Registration calls
base `004F71D0` at `00582F37`, then its **current** +14 at `00582F43`.
Only after that callback returns does it compute spacing. The generic layout
binder `005861B0` alone does not publish these fields, and constructor
`005902E0` leaves them unwritten. The existing optionals remain disengaged until
their corresponding calculation completes; no numerical defaults are supplied.

## Exact producers

The existing binder obtains `Primary_Text` in +24C at `0058658D`, `dest_Text`
in +258 at `0058670D`, and `dest_2_Text` in +25C at `005867C2`, all from the
authored objectives group. This fragment uses those same widget owners and the
same `GuiTextRuntimeImplementation`, without clones or substitute metrics.

| Cell | Native store | Calculation |
| --- | --- | --- |
| +2B0 | `00582F99`, `D9 9E B0 02 00 00` | float32(float64(resolvedY(dest258) - resolvedY(primary24C)) - height(primary24C)) |
| +2B4 | `00582FE6`, `D9 9E B4 02 00 00` | float32(float64(resolvedY(dest2_25C) - resolvedY(dest258)) - height(dest258)) |

The formulas describe the data flow; explicit x87 operations preserve its
rounding schedule. Each Y accessor `00AA6750` returns its vector through EAX,
with one output pointer argument and RET4; its Y float is spilled to the frame.
`00582F79/00582FC8` load the first Y, subtract the second binary32 operand,
then `00582F81/00582FD0` spill **binary64 before calling Text height**.
`00582F8A/00582FD9` use FSUBR binary64 on the returned ST0 height, followed by
the binary32 field store. The implementation uses the current x87 environment.

`00AB6BD0` takes ECX=actual Text, no stack operands, bare RET, ST0 result. All
three native return paths first FSTP binary32 and then FLD the same cell
(`00AB6BE6/BE9`, `00AB6C08/C0B`, `00AB6C20/C23`), so C++ float transport
does not add a rounding step. Existing Text code reads live multiline +178 or
the current font's signed word +14 and divides by live double `00CEF1B8`.

The +24C/+258 height slots are reloaded after their position calls. +25C is
captured at `00582F8E` before publishing +2B0. +470 is captured at `00582FDD`
before publishing +2B4 and returned to the registration continuation. That
continuation must use this captured receiver for current +34(false), rather
than resolving a different GUI object. Missing owners, unsupported Text
companions, or pending Text operations stop at the reached call. Already
published spacing is retained; the fragment does not preflight both fields or
roll back earlier native writes.

## Remaining registration phases

The full stored body `00582F30..00583091` was read: final RET length 1,
no uncovered epilogue or alignment gaps. The following phases are outside this
fragment and remain required for a complete registration caller:

1. `00582F37`: call base registration `004F71D0`, ECX=screen; then dispatch
   current +14 at `00582F43`, ECX=screen. The current-table identity is preserved.
2. After this fragment, `00582FEC..00582FF1`: dispatch captured page+470's
   current +34 with stack argument zero. No fallback visibility publication.
3. `00582FF3..00583054`: zero screen dwords +64,+68,+2C8,+2EC,+110;
   write live `00CE54A0` to +190 and live `00CEF7BC` to +194; write positive
   zero to float +198,+1A4,+1A8,+1AC; zero bytes +565 and +564, in this order.
4. `0058305A..00583087`: call `00B29E60` with ECX=`[00F8D394]`, stack
   operands (first to last) `[00F88994]`, `[00F88998]`, zero-extended byte
   `[00F8899E]`, `[00F889D8]`, zero-extended byte `[00F889E0]`, and 1.
   This renderer presentation boundary is named but not assumed implemented
   by this fragment. The call's enclosing epilogue ends at `00583091`.

`0058F100` was also inspected: it erases a score-map iterator range and is not
a spacing producer. Bounded inspection of the binder and entry `005987F0`
found no competing +2B0/+2B4 writes. This is evidence for the registration
producer, not a claim that every executable write to those offsets was scanned.

## Validation and scope

Live verified read-only BSP wrappers supplied the register listing, producer
bytes, and table bytes. `reports/main_menu_spacing.json` records the six direct
call sites and the containing registration boundaries. The report verifier
passed eight direct call rows with zero failures; the two indirect rows were
checked against the assembly and table. `scripts/build.ps1` compiled the new
source for MSVC Win32 Release and passed both existing tests (`reconstructed_math`
and `native_math_differential`). The ignored build log is `local/spacing-build.log`.
No new permanent tests were added. These checks do not execute complete GUI
registration, the game, or rendering.

## Correction from docs/MAIN_MENU_REGISTRATION.md

The complete ordinary582F30 caller now composes this existing spacing fragment
after base registration and fresh current14, then hides captured470, performs
the screen reset stores in native order, and reaches the required renderer
presentation service with six recovered operands. It borrows the same screen,
layout and geometry fields. The executable's older manual attachment shortcut
still requires integration; the new caller is not gameplay validation.

# Real Win32 text-message binding (CC10)

`handle_platform_text_message_00bed3b0_fragment` is the Win32 boundary for two
interior arms of the native `00BED3B0` window procedure. It accepts the actual
`HWND`, `UINT`, `WPARAM`, and `LPARAM`, sends `WM_KEYDOWN` and `WM_CHAR` through
the existing `PlatformTextInput::enqueue_message_00bed3b0_fragment`, then
returns the real `DefWindowProcA` result. Other messages return `false` without
effects so the caller's other window-procedure arms can run.
The original `00BED3B0` takes five stack arguments (explicit platform receiver,
HWND, message, WPARAM, LPARAM) and returns with `RET 14h`; this C++ signature is
a typed adapter, not that ABI.

## Native scope and ownership

| Native range | Coverage | Observed behavior |
| --- | --- | --- |
| `00BED607..00BED67D` | Complete key-message arm; partial `00BED3B0` | Gate on explicit platform+170h. Compare the full key DWORD with `26,28,25,27,24,09,23,2E,2D,14,21,22h`. Append `{low8(WPARAM),1}` at `00BED664` via `00BED370`; otherwise do not append. Both routes call `DefWindowProcA` at `00BED671`. |
| `00BED6DB..00BED6FE` | Complete character-message arm; partial `00BED3B0` | Gate on the same byte. Exact WPARAM `16h` sets platform+44h. Append `{low8(WPARAM),0}` through the shared key-arm tail, then call `DefWindowProcA`. Disabled input directly reaches the default call. |

The only queue in this route is `PlatformTextInput::queue`, already implemented
in `src/text_input.cpp`. The application must own one `PlatformTextInput` for
the window lifetime and pass the same object to this adapter. Any consumer must
dispatch **that exact queue** via `dispatch_text_input_00a96f40_fragment`, with
a real `TextInputCallbacks` owner. Constructing a queue in the window callback
would strand events. The adapter never enables input: the native path defaults
off, and owner activation `00A966E0` enables and clears platform input through
`00A965A0`.

Receiver identity is explicit in the listing. After the four register pushes,
`[ESP+14h]` is the first stack argument, the active platform. Both `00BED607`
and `00BED6DB` load that pointer into EAX; all +170h, +44h and +174h accesses
use EAX. `GetWindowLongA(window,0)` at `00BED3C5` stores a distinct pointer in
ESI for other message arms. The text adapter therefore receives the active
platform's persistent text owner, even if window-extra points elsewhere.

The actual menu activation call at `00583E31..36` pushes context, then enabled
byte `1` into `00A966E0` with ECX at screen+0Ch. Menu entry `00590EAA..AF`
passes zero and disables it. Main-menu update `00599DD2..E6` checks screen+10h
and dispatches with that same owner at screen+0Ch. The current host
`MenuUpdateBinding::text_owner_has_queue()` still returns false and its
`dispatch_text_events()` is unimplemented. Text editing, fallback callbacks,
clipboard consumption, and a concrete frontend owner binding remain separate
dependencies; queued events alone do not prove editable UI text.

The original `Win32PlatformFields` projection has older raw-offset placeholders
at +170h/+178h/+17Ch. This adapter does not treat those placeholders as another
queue or copy events into them. `PlatformTextInput` is a typed owner, not native
object layout or an ABI-compatible replacement.

## Verification

The saved project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, was live for the read-only inspection. The
`00BED664` call site was verified within the `00BED3B0..00BED761` body and is
the sole xref to `00BED370`. The full-key comparisons, exact `16h` check, and
fallthrough were checked against the saved listing. `scripts/build.ps1` built
Win32 Release `bsp_core` and `bsp_game`, then passed both existing CTests. The
primary integration owns the real application window-message check; that
result is not claimed here. No Ghidra writes were made. This does not claim the
complete window procedure, native text-owner editing, or gameplay parity.

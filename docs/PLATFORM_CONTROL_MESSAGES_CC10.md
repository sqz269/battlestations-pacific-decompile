# Platform control messages and power policy

Read-only native recovery in `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Source uses the existing projected platform state
and actual SDK `POWER_POLICY`/`MINMAXINFO` types. No native object-layout or full
window-handler replacement claim is made. Descriptive names are hypotheses.

| Routine / source | Coverage |
| --- | --- |
| `disable_platform_frames_and_post_quit_00bebf70` | Complete `00bebf70..00bebf94` |
| `initialize_platform_power_00bed223_fragment` | Partial BECEE0: `00bed223..00bed276`; all earlier constructor/window/renderer/cache work and `00bed277..00bed289` epilogue excluded |
| `enable_platform_screensaver_00bece41_fragment` | Partial deleting destructor BECE30: API argument instructions `00bece31..00bece36`, `00bece39..00bece3a` and call `00bece41..00bece46`; vtable, text/base destruction, optional free and return excluded |
| `handle_platform_control_message_00bed3b0_fragment` | Partial BED3B0: six complete control arms below; other arms remain outside this module |

## Shutdown and the policy producer

BEBF70 saves ECX in ESI, loads scheme `+4c`, and compares it with `ffffffff`.
Unless equal, BEBF82 calls import thunk C2DDD0 with `(scheme, null, this+50)`.
It ignores the return, clears frames byte `+42` at BEBF89, posts quit code zero
at BEBF8D, and returns with plain RET. It does not call DestroyWindow, clear the
HWND, write loop-finished `+43`, or request close/exit at `+180/+181`. The only
xref to its entry is vtable D68CCC, slot `+8`; BECEE0 calls that slot when its
old HWND is nonzero. A C++ host can call this complete recovered operation.

The policy storage must come from the producer. BED223 computes `this+4c`;
BED227 sets `-1` before GetActivePwrScheme at BED22D. The function tests AL
(the import returns BOOLEAN). On success ReadPwrScheme at BED23D writes the
original `POWER_POLICY` at `+50`. Its result is ignored. BED242 captures the
scheme; BED244..BED251 copies 24h dwords to `+e0`. BED258 and BED25E zero the
modified policy's VideoTimeoutDc and VideoTimeoutAc, respectively, at native
platform `+11c/+118`, then BED264 calls SetActivePwrScheme with that modified
policy and null global policy. The final SystemParametersInfoA at BED271 is
unconditional, `(0x11,0,null,0)`, disabling the screensaver. No host call has a
default successful implementation; the production import factories bind Win32.

The SDK layout is asserted: POWER_POLICY size `90h`, user offset zero,
VideoTimeoutAc `38h`, VideoTimeoutDc `3ch`. Native constructor BECDA0 does not
write scheme/policy storage. The older projection's scheme=0 initialization is
not evidence of a captured OS policy. The owner must execute the producer before
using its saved storage for restoration; no zero policy is manufactured here.
ReadPwrScheme failure still copies the supplied policy preimage, matching the
native ignored-return behavior. This module never turns failure into success.

The destructor's screensaver call is `SystemParametersInfoA(0x11,1,null,0)`:
it unconditionally enables the setting, rather than restoring a prior boolean.
BECE30 has no current Ghidra function definition. Its raw PE listing and live
bytes establish this call, but it must not be attributed to the nearby BECDA0
function. This packet performs no analysis mutation or full destructor port.

## Control arms and receiver identity

| Message | Native inclusive arm | Behavior |
| --- | --- | --- |
| WM_PAINT | `00bed5c1..00bed5dc` | Calls DefWindowProcA, discards its return, returns zero |
| WM_GETMINMAXINFO | `00bed67e..00bed69f` | Stores 100 at LPARAM+18/+1c (`ptMinTrackSize`), returns DefWindowProcA result |
| WM_SETTINGCHANGE | `00bed6b4..00bed6da` | Only full WPARAM 20h or 21h sets explicit receiver's +2c; every value reaches DefWindowProcA |
| WM_EXITSIZEMOVE | `00bed718..00bed730` | Sets captured window-extra receiver's +42 to one, then default |
| WM_ENTERSIZEMOVE | `00bed731..00bed749` | Clears captured window-extra receiver's +42, then default |
| WM_SYSCOMMAND | `00bed74a..00bed761` | Full DWORD WPARAM exactly F140h returns one without default; every other value defaults |

The common prologue's GetWindowLongA at BED3C5 is retained once per selected
message. It may return null before WM_CREATE; the minmax/paint/syscommand and
settings arms do not dereference it. Only the two sizing-loop arms require that
window's stored state. WM_SETTINGCHANGE uses the explicit active receiver at
stack+14h, not the captured ESI. The existing PlatformFocusMessageHost's
reference-return method is therefore not reused for this potentially-null read;
typed borrowed imports preserve the actual API contract without a fake object.

Assembly establishes EBP=HWND, EDI=message, EBX=full WPARAM, and ESI=the captured
window-extra pointer along these paths. ESI is overwritten only in disjoint
activation/create/size arms. Every native message return cleans 14h bytes, five
stdcall arguments including the explicit platform pointer. Imported WINAPI
calls consume their own stack arguments. Common default BED669..BED67D and
dispatch blocks are borrowed; no ledger claim replaces the whole handler.

Excluded arms are activation/create/size `00bed3f3..00bed5c0`, text
`00bed607..00bed668` and `00bed6db..00bed6fe`, and close
`00bed6a0..00bed6b3`. An unselected message returns false without calls or a
result write, allowing its existing handler to run. No tests are committed.

## Application order and binding

Native 73DC25 calls the entire BECEE0 routine. Renderer initialization, render
entry cache initialization, and the power tail all finish before BECEE0 RET at
BED289. Caller 73DC2B then adds 2Ch to ESP (eleven cdecl arguments), and online
construction at 73DC7C occurs later. A projected configure routine returning
deferred renderer arguments is only a prefix; executing online startup before
the deferred renderer/cache/power tail would reverse the native order.

The primary integration owns game_hosts wiring and policy storage. If bound,
the producer belongs after actual renderer/cache startup and stop must use that
same saved policy. However, no native normal-shutdown call to BEBF70 is currently
established; the proven caller is BECEE0's old-window path. BECE30's screensaver
call does not restore the power policy. Production power initialization/restore
binding is therefore deferred until the lifetime contract is established. The
exact fragments and probes do not invent that scheduling. Primary integration
can bind the control-message arms independently.

## Validation

`reports/platform_control_messages_cc10.json` records the build, call checks,
native spans, and focused ignored probe. The probe runs relocated original
BEBF70 and BED223 power bytes against spies, comparing three success/failure
producer paths and matching shutdown. PostQuitMessage is real on an isolated
thread; actual power and screensaver mutations are spied. Ten original/source
control cases cover exact syscommand matching, null early minmax storage,
distinct receiver identities, and WM_PAINT's discarded result. A real Win32
window checks message delivery and paint-region validation. These are fixture
and Win32-message results, not whole-game or native-object ABI validation.

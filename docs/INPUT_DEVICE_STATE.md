# Keyboard and mouse polling state

`input_device_state.hpp/.cpp` implement the keyboard and mouse device queries
needed by the complete `InputBindingPollHost`, the state-update methods that
feed them, and a concrete `KeyboardMouseBindingPollHost` bridge. Devices derive
from the existing `InputDevice`; no second device table is introduced. Native
COM calls use the real Windows SDK `IDirectInputDevice8A` interface.

These are typed C++ interfaces, not binary-compatible device replacements. The
caller supplies a borrowed, already-created and data-format-configured COM
device. The original constructors' `CreateDevice`, keyboard cooperative-level
setup, data formats, reference counting and destruction are not ported here.
The mouse's deferred cooperative-level setup is implemented in its poll.
Joystick and XInput queries remain a separate reconstruction dependency.

## Vtables and ABI

Constructor stores `00a9a411` and `00a9a2ee` establish vtables `00d5b904`
(keyboard) and `00d5b8b0` (mouse). Live bytes fix these slots:

| Slot | Keyboard | Mouse | Native contract |
| --- | --- | --- | --- |
| +08h class | `00a96350` | `00a9a0f0` | EAX=0 / EAX=1, RET |
| +10h update | `00a9a4a0` | `00a9a180` | ECX=device, ignored stack float, AL result, RET4 |
| +14h reset | `00a93e80` | `00a93e80` | bare RET; clears nothing |
| +1Ch query | `00a93ea0` | `00a93ea0` | ECX=device, stack code, AL=0, RET4 |
| +20h down | `00a95e70` | `00a99f70` | ECX=device, unsigned stack code, AL byte, RET4 |
| +24h value | `00a95e90` | `00a99fe0` | ECX=device, unsigned stack code, ST0 result, RET4 |

The keyboard and mouse return only 0/1 from +20h. The bridge retains the byte
return type because the complete action poll's required-modifier test is
`!=0` while its forbidden-modifier test is `==1`. The common +1Ch always-zero
method is proven by its two instructions; it is not a missing-device fallback.

The shared history walker `00a99e90` takes ECX=device and returns with a bare
RET at `00a99ec7`. For each code 0..255 it stores current+0Ch to previous+10Ch,
calls virtual +20h, then stores AL to current+0Ch before advancing. The keyboard
poll contains the same walk inline at `00a9a540..562`. Reset +14h does not invoke
this walker or clear either history. History arrays start zero in the native
base-construction fragment of `00a962f0` and the mouse constructor `00a9a290`.

## Keyboard update

Keyboard normalized state is at +20Ch, COM pointer at +30Ch. The poll first
zeroes all 256 state bytes. A null COM pointer returns false. A negative Poll
HRESULT leads to one Acquire call and returns false without shifting history.
A nonnegative Poll calls GetDeviceState with size 256, ignores its HRESULT,
shifts every returned byte right by 7, and clears the state again if platform
byte +170h is nonzero. It then shifts/query-updates history and returns true.
Partial bytes written by a failed GetDeviceState therefore remain observable.

`00a95e70` tests its addressed state byte against zero; it does not itself test
the high bit. `00a95e90` invokes +20h and returns binary32 0 or 1. Native code
does not bounds-check the keyboard code. The typed adapter reports an
out-of-range code rather than reading beyond its 256-byte array.

## Mouse update and values

The mouse COM pointer is +20Ch, validity byte +210h, raw 20-byte DIMOUSESTATE2
at +214h, wrapping accumulated x/y/z at +228h/+22Ch/+230h, cooperative flag
+234h, swapped-button flag +235h, and double-click seconds +238h. The raw
sample is not initialized by the original constructor. The typed constructor
requires an explicit initial sample so failed first reads have defined storage.

Every poll clears validity first. A null COM pointer returns false. If +234h
is zero, the poll calls SetCooperativeLevel with the platform HWND and literal
5 (`DISCL_EXCLUSIVE | DISCL_FOREGROUND`), then sets +234h even if that fails.
Platform +2Ch causes it to clear that platform flag and refresh system settings.
The actual Win32 GetSystemMetrics and GetDoubleClickTime calls remain live.
Unsigned milliseconds use the original x87 FILD/bias/FDIV-1000/FSTP conversion.

Poll failure calls Acquire once and returns false with previous histories and
accumulators retained. On successful Poll, GetDeviceState receives size 20.
Only its nonnegative HRESULT sets validity. Regardless of that HRESULT, the
poll adds all three raw signed deltas into wrapping 32-bit accumulators,
updates all histories through +20h, and returns true. A failed read may retain
or partially change the raw sample; validity makes every +20h query return zero
during that history update. The implementation passes its sample storage
directly to COM to preserve partial-write behavior.

Correction to the earlier `APP_INIT_INPUT.md` description: literal 0x17 is
`SM_SWAPBUTTON`, not `SM_MOUSEWHEELPRESENT`. Current Windows SDK WinUser.h gives
these constants as 23 and 75 respectively. Both query methods swap codes 0/1
when +235h is set. Buttons 0..7 use each raw button's high bit; values are 0/1
and never axis-scaled.

When valid, value codes 8/9/10 convert x/y/z to binary32 using CVTSI2SS. Wheel
code 10 first executes integer NEG, so INT_MIN remains INT_MIN. Y inversion is
controlled by external byte `00f8bc04` and executes binary32 `-0.0 - y`, not a
generic absolute-value or integer operation. The code then loads external
float `00e12fb0`, divides by double 100 at `00d7a220`, multiplies by the binary32
axis on x87, and spills to binary32. These two globals are explicit live caller
state, not invented globals. The image's scale value was 1.0.

Unknown codes also run the scaling path with a positive-zero axis. Consequently
an infinite/NaN scale can make their value NaN. The +20h method queries +24h
only for axes 8..10, testing nonzero including unordered; every other unknown
code returns a zero byte. Invalid devices return positive zero immediately
from +24h and zero from +20h. Inline x87/SSE preserves the specific spills,
integer wrap, signed zero and unordered branches used by these query functions.

## Binding integration and validation

`KeyboardMouseBindingPollHost` dispatches the three device operations through
`InputStateDevice`. Supplying a cached base/joystick/XInput device is an explicit
error. Its required `CrtSqrt` function pointer preserves the existing external
CRT/FPU boundary; library sqrt behavior is not reconstructed here. Polling the
COM devices and synchronizing the supplied platform/globals remains the caller's
per-frame obligation before polling action bindings.

Every Ghidra query/export used `bsp.py`'s verified `bsp` project and
`/battlestationspacific.exe` target (`C:/Users/sqz269/bsp.gpr`). The primary
integrator defined missing function starts under its write lock; this worker
made no Ghidra mutations. Assembly superseded incorrect pseudocode argument
types and x87 expressions. `reports/input_device_state.json` lists boundaries,
evidence, status, uncertainty and build/probe artifacts.

MSVC Win32 Release compiled this source through an ignored local deferred
source registration; the integrator must register it in `cmake/startup.cmake`.
`scripts/build.ps1` passed both existing CTests after verify-seeds. A single
ignored native query probe verified four query code ranges against the installed
executable and passed 6,272 exact binary32-bit/AL comparisons. Coverage included
swapping, invalid state, INT_MIN wheel, signed zero, unknown codes and sampled
infinite/quiet-NaN scale values. No new permanent test suite was added. The
probe did not execute real device polling, verify arbitrary NaN payloads or FPU
exceptions, or establish in-game behavior.

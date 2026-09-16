# Shared platform focus cells and window messages (R67)

Addresses: `00BED3B0`, `00BECDA0`, `00BECEE0`, `00BEC230`, `00B20C50`.

## Production change

The application window callback now executes the complete `WM_CREATE` and
`WM_SIZE` branches of `BED3B0`. Creation stores `CREATESTRUCTA::lpCreateParams`
in window-extra offset zero before setting that object's `+40` flag. Resize
captures that window-associated object, optionally changes fullscreen Z order,
and writes its `+40/+41` flags. Exact DWORD size values 1 and 4 clear the flags;
all other values set them and call `SetFocus`, then `SetForegroundWindow`.
Every size branch returns the real `DefWindowProcA` result. Win32 failures are
ignored as in the original. The injected production methods call real Win32 APIs.

The focus calls use the separate active receiver passed by the window thunk,
not the window-extra receiver. The HWND is reloaded after `SetFocus`, preserving
reentrant changes. The window-extra receiver remains captured across the
`SetWindowPos` callback. Native `BED3B0` takes five stack arguments including its
explicit receiver and ends with `RET 14h`; its old Ghidra prototype is incomplete.
This source fragment exposes a new typed interface, not that original ABI.

`Win32PlatformState::window` and `byte_041` now refer to the actual `+30/+41`
cells of an owned 44h `NativePlatformWindowFocusStorage`. Existing constructor,
window creation, resize and shutdown writes all reach those cells; there is no
synchronization copy. Explicit copy/move construction and assignment retain
references to each destination's own storage. The rest of the platform remains
a typed projection. Padding in this storage is **not** recovered native state;
only `BEC230` and `B20C50` may consume it. It must not be passed to the full
native platform constructor or message handler.

## Evidence and validation

Fresh Ghidra reads from `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`
match the installed original PE for five complete bodies (946, 132, 938, 4 and
31 bytes respectively) and the 24-byte first message jump table: **2,075 bytes**.
The read-only receipts include exact instructions, original bytes, hashes and
call sites. No flow repair or new function definition was needed.

A local MSVC Win32 differential probe executes the original `BED3B0` machine
body for the admitted create/size branches. Its IAT slots route to recording
imports; other external calls trap. The first six-entry jump table is rebased.
Thirteen original/source cases compare results, Win32 argument lists, call
order and flags: creation plus six size values in both fullscreen states.
The receivers are distinct; callbacks change the window-extra publication and
active HWND to verify native capture/reload behavior. Original `BEC230` and
`B20C50` also match the reconstructed helpers on production focus storage,
using the real `GetFocus` import. Copy/move independence and constructor stores
are checked. These are finite fixture results, not full message-handler or
original exception/ABI parity. No permanent tests were added.

The strict Win32 build and all three existing CTests passed. The application
ran with the previously verified private Microsoft XLive/dependency pair and
isolated settings: two frames presented, exit zero. Its native focus reads
reported `hwnd_matches=1 active=1 focused=1`; the saved screenshot shows the
existing front-end art and sign-in/continue prompt. This validates the reached
real-window path, not interactive activation/minimize/fullscreen gameplay.

## Remaining work

`WM_ACTIVATE` is not routed through this fragment. Its ordered XInput, sound,
media, GUI, renderer-focus and render-service calls still need shared application
bindings. Copying only its flag writes would omit those dependencies. Other
message branches remain outside this change, including the existing separate
close handling. The platform singleton registration/full 184h ABI is also open.

The application still uses the projected renderer constructor, device prefix
and milestone frame loop. The R66 full renderer remains fixture-validated.
Its production composition also needs canonical Lua/type/service state,
declaration dependencies, valid COM lifetime through manager drain and failure
handling. This packet provides the actual window/focus cells needed by its reset
context; it does not claim that full renderer composition or gameplay is done.

Detailed receipts and local artifact archive references are in
`reports/native_platform_focus_r67.json`. The archive preserves the tested
artifacts and direct fixture compiler inputs; it is not a complete toolchain,
all-loaded-module, game-asset or reproducible-build closure.

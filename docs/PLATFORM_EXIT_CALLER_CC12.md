# Platform exit caller and close-request transport (CC12)

`GameFrameHost` now executes the recovered window-close policy using the
application's actual online-manager publication and current game state. It no
longer turns every pending close into unconditional global exit. This packet
does **not** establish a normal-exit call to `BEBF70`; power-policy activation
remains dependent on that specific missing caller.

## Bounded native caller evidence

Live batches used project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. The saved `BED3B0` body records `WM_CLOSE` at
platform `+180` and returns zero. `BEC1A0` checks `+181` after the message/frame
iteration and sets `+43` on exit. Neither body calls platform slot `+8`.

The outstanding getter lead in `PLATFORM_STOP_SCHEDULE_CC10.md` was checked
against every raw executable-section byte for direct `E8/E9` targets and every
raw PE section for exact stored pointers. Getters `4B4BC0`, `B210D0`, `B23680`,
and `B25F30` have no such users. `BEBF70` has no direct relative call/jump and
only the stored pointer at `D68CCC`. The `SetActivePwrScheme` thunk `C2DDD0`
has exactly the two direct calls at `BEBF82` and `BED264` and no stored pointer.
This closes those direct/getter leads; it does **not** exclude an indirect
virtual caller through another platform alias. The genuine missing dependency
is the ordinary-exit caller/alias that dispatches platform slot `+8`, if such a
caller exists. The already proved old-HWND call at `BECF27` remains unchanged.

## Concrete close policy

The complete `4CA2F0` listing has 88 instructions without listing gaps. Its
prefix consumes and clears platform `+180` before reading the current
`F8ABE8` manager's `+3E8` byte. That byte is the existing
`NativeOnlineManagerStorage::notification9_3e8`: the real constructor initializes
it at `A40EB5`, and the native notification-9 branch updates it at `A401A9`.
Nonzero suppresses the consumed request. States `1`, `2`, and `4` set global
exit `E1AE75`; other states require the shared confirmation screen.

`WindowCloseTransportContext` retains references to the platform, global exit,
and the actual mutable online publication. The concrete host invokes the
existing `cc13_sys::game_process_window_close_request_004ca2f0` body. It reloads
the current online owner after consuming close, preserves nonboolean gate
bytes, and does not set loop exit `+181`. Application code runs it before
`menu_->frame`, matching close call `4E4A79` before the native front-end update.
The application supplies `settings_process_->online_00f8abe8()`, shared with
the online constructor and notification producers.

Source admission checks return an explicit missing-owner result **before**
native effects. A missing online owner or a missing required confirmation
owner leaves close pending and global exit unchanged. A prompt owner is not
required for no request, a suppressed request, or states `1/2/4`. These checks
are source integration boundaries, not native null checks or alternate native
behavior. They require the owning UI thread and no concurrent publication
retirement between admission and the synchronous policy call.

`RetainedWindowClosePromptCalls` composes the already recovered prompt bodies:
title, message, and owned countdown strings are constructed in native order;
the real supplied `425D10` getter is called at `4CA3A2`; `531B00` receives slot
6, Yes/No kind 1, callback `4BBC50`, flag 1, zero timeout/result and dismissible
1; caller strings are released message then title; the getter runs again at
`4CA41B` before `530C20` selects the last navigation item. Both getter results
must carry their actual matching screen/host, and both string providers must
refer to the same underlying storage owner. The getter's lazy construction,
registration and GUI dependencies are not fabricated by this adapter.

`GameFrameHost::dispatch_window_close_callback` accepts `4BBC50` only, clears
close and sets the sticky global exit only for response 1. The prompt host must
route that address there and retain the frame host throughout callback delivery.
Unrecognized callbacks are left to their own dispatcher.

The current application has no actual shared confirmation-screen/GUI owner;
its non-front-end path therefore reports `CloseRequestPolicy::confirmation_owner`
at `425D10/531B00` and leaves the request pending. It must be bound through
`bind_window_close_prompt` when that actual owner exists. This is a precise
remaining prompt dependency, not a completed confirmation UI or shutdown.

## Validation and limits

MSVC Win32 `/O2` compiled the new transport and modified application host. A
focused `/MANIFEST:EMBED` probe relocates the installed image's complete
326-byte `4CA2F0` and 25-byte `4BBC50`, replacing only globals/imported callees
with controlled storage and spies. Fifteen original/source scenarios compare
no request, nonboolean suppression, all three direct-exit states, confirmation
arguments/focus order, and callback responses `-1/0/1/2` with both initial exit
values. They pass. Additional source admission checks verify pending-close
preservation and that suppressed/direct-exit branches need no prompt owner.
The probe does not call OS power APIs or touch the game installation.

The complete build requires the integrator's registration of
`src/window_close_transport.cpp` in `bsp_core`; the shared CMake file was held
by another packet during worker validation. The concrete prompt composition
compiled, but its GUI lifecycle was not exercised by this policy probe. Native
SEH cleanup, native platform ABI, the actual prompt owner, normal-exit power
restoration, and whole-game startup/gameplay remain unproved. Existing Ghidra
names/comments were preserved; the `4CA2F0` evidence was appended under the
write lock and the project saved. No new CTest cases were added.

# Application raw input composition

Addresses: source composition, consuming 0073DD6C..0073DDB3, 00A900F0,
004B4630, 00A90490, 00A918A0, 00A91620, 00A90EE0, 00BEBF30, 00A983C0,
00BECB20, 00BECA40, 00BECE70, 00A92840 and 004BA6D0.

`GameInputRuntime` binds the recovered raw backend, device, action storage and
cursor providers. It owns source service contexts and the DirectInput reference
tracker; native backend/action allocations remain registered with the application's
existing raw singleton manager. The source facade has no original function address
or original ABI claim. Each consumed routine's registers, stack cleanup, body end,
call-site evidence and source domain are recorded in the corresponding
`native_input_backend_startup`, `native_input_backend_bindings`,
`native_input_cursor` and `native_input_device_runtime` reports.

`GameStartupHost::InputServices` supplies the canonical application platform and
clock publications, actual F8BBF4/F8BBF8 cells and source SDK services. Startup
constructs the actual F8h backend, reloads its publication to store callback4B4630
at+D8, then reloads it for A900F0 reset. The callback is the verified C3 target.
Device lookup uses the separately reconstructed `gui_raw_input_device_004ba6d0`
over actual active vectors, rather than the fixed attachment table.

Sound's cursor events now borrow the same facade/publications. Existing source
online/platform owners retain their own types; raw input allocations are never
cast to typed input owners. A reached listener-zero callback requires the actual
application listener provider. No replacement listener or empty active vector is
created by the facade. Captured groups-profile D5B5F8 slot0C is A90ED0's C3;
slot10 is the purecall used by the tick prepass. The DirectInput profile's slot0C
dispatches A983C0, capturing+E0 before clearing+F4.

DLLs, publication cells, action/storage callbacks and HWND remain alive through
the shared raw BD0400 drain. Explicit SDK release follows that drain (device
references first, backend interface references second); native destructors do not
acquire an invented COM-release responsibility. Normal window destruction follows
the drain. On exceptional startup, a native registration failure can preserve a
dangling backend publication, or an unregistered action allocation. The fallback
destructor logs an unmet SDK-release precondition without dereferencing/repairing
such cells or claiming resource cleanup succeeded.

`--xinput-dll` selects an absolute DLL path. The default uses the actual Windows
system directory and XINPUT1_3.dll, preserving the version/ordinal contract.
Image-value initialization uses the verified bit patterns recorded in the input
reports; keyboard/mouse and joystick share the same float1.0 and negative-zero
storage. The legacy action-storage API borrows sound's immutable1.0 DWORD literal
representation, avoiding a C++ float/integer alias. CRT conversion mode uses the
OS CPU+OS SSE2 capability service as a source binding, rather than claiming a port
of the original CRT initializer C27B7C/C27B1C. The joystick ignored-result Xbox
branch receives an explicit zero source stack baseline; arbitrary original
uninitialized-stack failure behavior remains outside the demonstrated domain.

## Remaining application work

The native OnInitOnce requested-count/accepted-ID producer A917E0 is now bound
by the startup fragment documented in GAME_INPUT_STARTUP_SETTINGS.md. Action/frame
processing, actual online-owner construction and listener implementation remain
required boundaries. The real settings commit must bind the live rumble
word; mouse scale/invert producers are not established merely by initializing their
image words. Source construction and cleanup do not establish interactive input,
hardware effects, native ABI compatibility or a runnable game rebuild. Validation
and its precise source/archive revision are recorded in `reports/game_input_runtime.json`.

The original running game and its single-instance mutex remain untouched.

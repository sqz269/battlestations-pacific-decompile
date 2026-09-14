# Raw online notification dispatcher

Addresses: `00A40110..00A404F3`; switch table `00A404F4`; FH3 handler
`00CB41D8`, funclets `00CB41C0/00CB41C8/00CB41D0`, metadata `00DE9E84`.

`drain_native_online_notifications_00a40110` implements the complete normal
dispatcher over the captured actual 3F0h manager. The caller supplies the same
raw pool, sign-in/profile/update providers, achievement SDK/CRT pair, and live
publication cells used by the application. It supplies defined stack preimages
for the SDK outputs; no projected manager or zero-filled invite is substituted.
The C++ name is descriptive, not a recovered symbol. Original entry is ECX
manager, no stack arguments, RET. This interface does not reproduce that ABI.

The listener is created only when manager+1C is -1, with areas2Fh; null does not
trigger recreation. Sign-in debounce precedes the loop. Every GetNext reloads
the current listener and reuses the same id/parameter output cells. Failed
GetNext outputs remain in those cells and are not dispatched. Bare-RET
004254B0 diagnostic calls have no application effect.

| Notification | Native site | Result |
|---|---|---|
| 9 | A401A9 | Capture current F8ABEC, pass SETNZ(parameter) in CL, reread parameter after callback, then write captured manager+3E8 |
| 10 | A401CF | Same-manager A3F440 sign-in toggle |
| 11 | A401EA | Invoke current optional F8ABF0 |
| 14 | A401E0 | A3E600 with parameter low byte |
| 15h | A40218 | Title update path, append `\setup.exe` if nonempty, invoke launcher with `update.exe`, Sleep200, _exit0 |
| 16h | A404BF | System path, raw byte-to-UTF16 header, SDK update call, _exit0 |
| 02000001h | A40358 | Force achievements on 1510F0h; set manager+128 on 80151005h |
| 02000002h | A40328 | Ignore accepted-invite SDK status, set manager+31, copy exactly84 bytes to unaligned manager+32 |
| 02000007h | A40409 | Set manager+30 before diagnostic |
| 04000002h/3h | A403F9 | Reload current F8A2FC, then its current vtable+28 |

The invite source is the first54h bytes of the original stack window at ESP30.
There are5Ch bytes before the EH record; that capacity is not an inferred SDK
record size. Partial SDK writes and failure preserve the caller's other bytes.
The operation and manager must have distinct, live storage.

Title-path normal inline frees capture pointers/sizes and do not clear headers.
The underlying BF7680 CRT copy admits overlap, so suffix copying uses memmove
with the native captured length and current destination pointer. The root's
inline returns resolve00419CC0 on each call and use the same raw pool/gate.
The leaf string adapter retains its established noexcept-release boundary.

FH3 metadata selects header28 alone in state0, header20 alone in state1, and
header28 then header20 in state2. The wide header20 in the system-update branch
is not an unwind obligation. `_exit` bypasses all cleanup. A source exception
retains the operation as active with surviving headers and refuses replay;
native FH3 delivery and double-exception handling are not established.

Use `NativeOnlineNotificationLeafRuntime(library, raw_profile_context)` for the
normal manager+18 identity737D60. The SDK-only constructor cannot satisfy that
callback. Its context must borrow current F8ABE8, E188A8 and actual string pool.
The UI's locale resolver remains an explicit semantic provider.

F8ABF0 has no image writer in the saved xrefs; the established image domain is
null. A nonnull C++ binding must be a supplied no-argument cdecl thunk, without
claiming an unknown native register ABI. F8A2FC is published by992A50 after
2218h allocation and A47EE0; the final primary vtableD24878 has slot28=A43180.
The source calls this borrowed actual virtual. A43180/A430E0 gameplay bodies
are still external; no synthetic friends callback supplies their behavior.

Evidence and exact call sites are in `reports/native_online_notification_dispatch.json`.
Seven saved/installed-PE spans total1346 bytes across dispatcher, pump and
metadata. A focused fixture verifies partial failed invite output, current
slots, source state0 cleanup/replay, and isolated child `_exit` paths with fake
SDK/Shell calls. It does not execute original A40110 or a real updater. Exact
registered-library build/fixture evidence is recorded after integration.

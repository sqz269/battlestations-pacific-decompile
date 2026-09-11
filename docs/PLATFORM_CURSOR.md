# Platform cursor and focus policy

Addresses: 00becb20, 00bece70, 004ba6d0.

`update_platform_cursor_focus_00becb20` implements the complete normal cursor
policy using the canonical `Win32PlatformState`, the current
`PlatformManagerFlags::system_ui_visible`, and the backend's existing
`InputBindingDeviceGroups`. The three process bytes at 0109DB8E/F/90 are bound
by reference; no second set of manager flags or device vectors is created.
The original image initializes those bytes to zero, but binding the interface
preserves their current values.

Required services remain explicit: the XLive manager pump 00a409f0, focus/input
reset 00beca40, mouse cooperative-level change 00a9a140, backend virtual +4 input
update, and Windows `ShowCursor`. The parent combines the independent input and
XLive implementations with this policy. No service has a default implementation.
`run_platform_application_service_00bece70` additionally requires the actual
application virtual +10 operation, whose substantive implementation is 00737a50.
It passes the current `platform.application` and then runs cursor policy with
`loading=false` after that call returns. Application exceptions skip cursor work.

## ABI and device getter correction

| Address | Native ABI | Final instruction / inclusive end |
| --- | --- | --- |
| 004ba6d0 | ECX=backend; stack signed class, unsigned index; EAX=device/null; RET8 | 004ba718 RET8, 3 bytes / 004ba71a |
| 00becb20 | ECX=platform; stack loading byte; RET4 | 00beccc5 RET4, 3 bytes / 00beccc7 |
| 00bece70 | ECX=platform; no stack args; RET | 00bece87 RET, 1 byte / 00bece87 |

004ba6d0 was labeled `STL_inst_004ba6d0` with a high-confidence STL tag. Its
assembly computes `backend + 6Ch + class*24h`, reads vector begin/end at +4/+8,
and performs an unsigned index comparison. A null begin or out-of-range index
returns null. A second checked access follows the first guard, then reloads
begin before reading the device pointer. There are no calls or permitted
container mutations between these two checks on valid single-threaded input.
The reconstruction uses the existing class vectors and one index guard. Class
indexing itself is unchecked in native code and remains a caller precondition.
Class 1/index 0 selects the first active mouse device; this is distinct from the
fixed 3x8 `InputDeviceTable` attachment slots. The descriptive replacement name
is a hypothesis, not a recovered symbol. The saved STL tag/name correction is
left to the primary agent, with its old classification retained in the report.

## Native transitions and callback ordering

The initial guards test current F8ABE8, current F8BBF4, and class 1/index 0.
Missing values return without touching policy globals or invoking the pump.
When loading is nonzero, 00becb57 reloads the current manager and calls 00a409f0.
Only after that pump does the policy read platform+41 and current manager+3E8.
The UI byte and desired cursor state are captured for the rest of the call:
show the cursor when focus byte+41 is false or system UI is visible.

If UI is visible and prior UI byte DB90 was zero, the policy resets focus/input,
reloads F8BBF4 and obtains the first active mouse again, sets cooperative flags
to 6, and, while loading, invokes current backend virtual +4 with 0.1f. The
constant at D7A2F0 is `CD CC CC 3D`; native FLD/FSTP transfers that finite binary32
value unchanged. The backend is reloaded at each update call, rather than cached
before reset. A reset or update may change the manager flags, but the captured
UI value still determines this call's final DB90 write.

If UI has just closed while platform focus remains absent, DB8F becomes 1 and
reset is deferred. When focus permits hiding and either prior UI or DB8F is
nonzero, reset runs, the optional loading update follows, then DB8F is cleared.
The hide path resets once more only if no reset has already run this call.
Thus a deferred reset can occur even when DB8E says the cursor is already hidden;
the extra hide-only reset requires DB8E to be nonzero.

To show, write DB8E=1 before calling `ShowCursor(TRUE)` repeatedly until its
signed return is nonnegative. To hide, perform any required reset/update, write
DB8E=0, then call `ShowCursor(FALSE)` until its signed return is negative. These
are count-adjusting loops, not single visibility setters. The three globals are
read or written at their original decision points and are not restored if a
required callback throws. Finally DB90 receives the UI value captured earlier.

## Boundaries and validation

The policy's pointer accessors are plain reads of the caller's current singleton
bindings. After the initial guards, callbacks must preserve the validity needed
by subsequent native dereferences. The first mouse may change during reset,
which is why the second lookup is explicit. Supported backend groups must contain
class 1 even when its vector is empty; inventing a fixed attachment-table fallback
would alter native behavior. `PlatformManagerFlags` already represents +3E8 as
bool; arbitrary malformed non-boolean native flag bytes are not an additional
supported representation.

This packet does not infer initialization of platform+2C from this policy.
Neither 00becb20 nor 00bece70 accesses it. The independently recovered focus-reset
and mouse-setting bodies own its relevant behavior. These C++ interfaces remain
typed projections, not physical native objects, vtables, calling conventions or
arbitrary SEH substitutes.

All three entries already exist in saved Ghidra metadata with complete displayed
body endings. No missing starts or false no-return tails were found in this
packet. The worker verified the configured BSP project/program and used only
read-only Ghidra exports, listings and byte queries. The parent performs reviewed
annotation and save operations. `reports/platform_cursor.json` records provenance,
native bounds, the stale STL classification, and verification results.

MSVC Win32 Release build and both existing CTests passed after all eight native
seed ranges matched the installed image. One ignored focused fixture exercises
UI opening, UI closing while unfocused, later focus restoration, both signed
display-count loops, singleton/mouse reloads across callbacks, captured UI state
despite input-update mutation, application-wrapper ordering and the missing-manager
entry gate. It also checks empty vectors and an unsigned maximum device index.
The fixture uses explicit test services and does not call real `ShowCursor`,
change the user's desktop focus or run XLive. Source/commands/output are in
`local/platform_cursor_fixture.cpp`, `local/run_platform_cursor_fixture.cmd`,
`local/platform-cursor-fixture.log` and `local/platform-cursor-build.log`.

These checks establish reconstructed, build-tested and fixture-tested behavior.
Concrete cross-packet runtime composition, original ABI compatibility, live
focus/cursor effects and gameplay remain unvalidated by this worker.

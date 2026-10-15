# Raw platform stop on the actual owner

`stop_native_win32_platform_00bebf70` supplies all 37 bytes of BEBF70 through
the raw platform construction module. It consumes the same 184h-byte owner
used by construction and queue production; no projected `Win32PlatformState`
or copied policy owner is introduced. The previous typed implementation and
its evidence remain intact. Report: `reports/native_platform_raw_stop.json`.

## Captured owner and real imports

Original entry receives the owner in ECX, consumes no stack arguments, ignores
incoming EDX, preserves ESI and returns with RET. Source exposes that entry as
MSVC Win32 fastcall with a documented unconsumed EDX argument. Both the strict
probe object and normal CMake object emit the original 37-byte instruction
sequence except two named relocation operands. The linked power call reaches
the import library's six-byte jump through the real PowrProf IAT; the second
call directly uses the real User32 PostQuitMessage IAT.

The body captures ECX in ESI and reads owner+4C once. FFFFFFFF skips power
restoration. Otherwise it passes the captured scheme, null global policy and
the exact same owner's +50 address to SetActivePwrScheme. It ignores the
result, pushes zero, clears only captured owner+42, then calls
PostQuitMessage(0). The original instruction order is retained, including the
byte store after the power import. This operation adds no policy producer,
HWND destruction, queue cleanup, +43/+180/+181 store or publication change.
The caller must supply a valid produced policy for a non-sentinel scheme.

The stop body does not read canonical0109CF04 or window-extra storage. A
returning import can replace the canonical publication while the captured
owner remains the receiver of the later byte store. D68CCC points to BEBF70;
BECEE0's old-HWND path calls that slot at BECF27. Fresh BEC3B0 bytes confirm
that its different, per-invocation publication operand is **0109CF04**.
These identities must not be merged when the message pipeline is composed.

## Evidence and focused fixture

Seven fresh live Ghidra/installed-PE spans total 108 bytes: stop body, power
import thunk, two IAT slots, D68CCC, old-window caller slice and BEC3B0. The
saved BEBF70 listing equals the live listing. Every live query verifies the
configured bsp project and `/battlestationspacific.exe`; configuration names
`C:/Users/sqz269/bsp.gpr`. Installed executable SHA-256 remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The manifested local fixture uses the real Source constructor and canonical
CRT to produce a raw 388-byte owner, queue and singleton manager. It executes
copied Original37 and current Source on the same owner for a sentinel scheme,
a FALSE power-import return and a TRUE return. It compares all 388 bytes,
checks the exact +50 pointer and ordered calls, and replaces the fixture's
single borrowed publication cell during the returning power import. The
captured original owner's +42 is cleared; the replacement owner is untouched.
The cell models the canonical binding contract; it is not the game's actual
process address0109CF04, and the fixture does not construct a second platform.

Power writes are intercepted in the fixture process. The fixture uses a
patterned policy at owner+50; BED223 is not executed or credited as a policy
producer. The post-import spy
forwards to actual PostQuitMessage on an isolated thread; all six calls
produce WM_QUIT with zero and each message is drained. Both process IAT hooks
are restored. Fixture cleanup retires the raw queue, title/base/publication
and manager without invoking the separate derived screensaver-setting path.
Actual active-scheme, complete POWER_POLICY and screensaver readbacks all
succeeded and were unchanged before/after. No real power-scheme write ran.

Normal `./scripts/build.ps1` Release and its three tests passed. The strict
probe used `/W4 /WX /fp:strict /MANIFEST:EMBED`. COFF and linked-code checks
prove the bounded stop entry/stack schedule; imported Win32 behavior is
limited to the real WM_QUIT observation and settings readback described above.
No binary was installed, and no application/window/game activation is claimed.

## Next message frontier

BED3B0 remains incomplete as a composed raw handler. Its GUI/media singleton
and sound/movie dispatch bodies already exist, and the raw BED370 queue now
exists. B24FB0 still accepts abstract `NativeRendererActivationBindings`:
the tree has no concrete implementation of resource_reload_slot_08 and
refresh_material_textures_00b19000 on the same canonical resource domains.
B19000 Source exists but is runtime-unvalidated; B469A0's recovered reload
has a callable-descriptor domain that is not an automatic raw registry
binding. B22030/B21F70 resource targets and their real providers must be
composed and validated before admitting that activation chain.

This packet adds no WM_ACTIVATE admission, BED3B0 replacement, BEC3B0 binding,
BECEE0 full-window configuration, native exception compatibility or gameplay
credit. DeviceGraph's private copied publication cell remains a separate
binding problem. The next complete handler must preserve current0109CF04,
explicit active receiver and captured WindowExtra receiver independently.

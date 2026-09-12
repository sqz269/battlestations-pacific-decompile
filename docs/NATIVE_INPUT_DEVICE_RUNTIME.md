# Raw input device runtime composition

Addresses: none (source composition). Provider addresses and original ABI evidence
remain in `NATIVE_INPUT_DEVICE_VIRTUALS.md`, `NATIVE_GAMEPAD_FORCE_REQUESTS.md`,
`NATIVE_INPUT_GUID_STORAGE.md` and the AD raw device reports.

`NativeInputDeviceRuntime` supplies the backend and device callback contracts with
the reconstructed providers over actual raw allocations. It borrows strings,
SDK reference tracking, an explicitly selected XInput DLL, device tables/constants,
and the existing canonical source clock/window publications. It creates no second
singleton manager, device table, GUID owner, SDK reference owner or native clock.

## Dispatch and lifetime

Methods receiving a captured native profile select that target without recapturing
the profile. Methods without that argument read the actual receiver's first word
on entry; nested native virtual calls capture again at their original sites.
The admitted profiles are keyboard `00D5B904`, mouse `00D5B8B0`, XInput `00D5BB48`,
joystick `00D5B7F0`, and the common gamepad base `00D5B670`. This is a finite source
dispatcher, not an emulator for arbitrary native vtables. An unknown profile or
wrong slot signature is a source binding error. Known base pure slots invoke the
actual CRT `_purecall`.

The backend supplies its actual receiver to real DirectInput enumeration. The
callback constructs the existing raw keyboard, mouse or joystick allocation and
uses the stateless GUID buffer adapter. Four raw XInput allocations use the same
gamepad request providers. Device class, attachment, history, polling, reset and
zero-reference deletion resolve to the existing raw functions. The backend has
already decremented the device reference before slot00; `BD30E0` performs the
captured slot04(flags1) dispatch without another decrement.

The force request superclass implements all five current-profile request slots
over the actual 14h/18h/28h request allocations. The runtime supplies XInput and
joystick device force/value methods; the common gamepad's force slot remains a
real purecall. Mouse slot38 has a different, no-argument float result signature.

The dispatcher and every borrowed service must outlive raw backend/device drain.
After raw borrowers are gone, the caller explicitly releases tracked device/effect
SDK references and then backend DirectInput references. No destructor here silently
releases an SDK object. XInput DLL lifetime spans all callbacks.

## Canonical source publications

The clock call reloads the borrowed publication and invokes `00BEE050`'s existing
current-timestamp getter; it does not sample a new clock. Window access reloads the
existing platform publication and calls the reconstructed `00BEC230` getter.
These are the reconstruction's canonical `FrameClock`/`Win32PlatformState` owners,
not original singleton/vtable ABI casts. Null publications are explicit source
binding failures. Shared D7A24C and D7A208 inputs must refer to identical storage;
constructor checks catch conflicting source bindings before constructing devices.

Global words and literal addresses remain borrowed. A fixture initialized from
original image values is not installation into the original executable's globals.
The joystick's XInput stack preimage remains explicit and independent of untouched
object bytes.

## Evidence and remaining work

The first integrated strict Win32 Release build and both existing CTests passed.
The 35 force/virtual provider prototypes were saved and read back under the Ghidra
write lock; prior annotations and typed reconstruction records were retained.
`reports/native_input_provider_prototypes.json` records the native signatures;
`reports/native_input_virtuals_function_definitions.json` records three previously
missing leaf starts. The GUID adapter claims no original STL implementation.

Review confirmed the currently connected slots but found further table entries:
XInput and joystick control names at3Ch, and mouse fields at3Ch through50h. Their
coverage is tracked separately while the supplemental providers are integrated.
The dispatcher therefore does not yet claim complete virtual table coverage.

The application has not yet attached this composition to its frame/input-action
path. Raw event submission and request-tree insertion remain separate work. A
source build or isolated backend lifecycle is not hardware polling, controller
delivery, original binary ABI compatibility or gameplay validation. The existing
game's single-instance guard remains intact; this packet does not launch a game.

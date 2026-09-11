# Concrete DirectInput runtime composition

`DirectInputRuntime` implements both existing interfaces, `DirectInputHost` and
`InputEnumerationDeviceFactory`, using the actual SDK and reconstructed device
constructors. It binds the caller's existing `IDirectInput8A*` slot by reference,
the live mouse globals and explicit initial sample, joystick services, and actual
XInput API/globals. It does not create a second table, active-device group list,
GUID sequence, clock, window owner, or set of input globals.

This is runtime composition of already reconstructed behavior, not a new native
function reconstruction or a binary-compatible replacement. Evidence was checked
read-only through `bsp.py ghidra` against `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. No Ghidra mutations were made.

## Binding and startup

Construct the runtime with the actual interface slot initially null. Construct
the existing `InputFocusBackendState` with that runtime as its `direct_input`
host, then construct `InputEnumerationContext` with the same runtime as its
`devices` factory. Call `bind_enumeration_context(context)` before enumeration.
Binding rejects either a different factory or a different backend host, including
the distinct base-subobject identities created by multiple inheritance.

Then call the existing `create_direct_input_devices(backend.slots, runtime,
DirectInputCreateParams{}, error)`. Its order remains creation, AddRef,
EnumDevices(type 0, flags 1), then four XInput devices attached to first-free
class-2 slots. Context, runtime, services, API DLLs and referenced storage must
remain alive for every operation that uses them. The adapter does not initialize
the native backend's other fields; the caller supplies that existing initialized
backend. Subsequent canonical A983C0 enumeration clears the Xbox-detected flag
before calling this host and keeps the persistent GUID sequence.

`create_interface` calls real `DirectInput8Create(GetModuleHandleA(nullptr),
version, IID_IDirectInput8A, &actual_slot, nullptr)`. It preserves the actual
HRESULT for inspection and returns whether an interface output was supplied,
matching the native ignored-HRESULT behavior when output is valid. It rejects
reinitialization over a nonnull slot. `add_ref` calls the actual current interface
and records that additional reference. `enum_devices` invokes the existing
`enumerate_direct_input_devices` bridge with the bound context and actual current
interface. That bridge handles the genuine SDK callback ABI and transports typed
callback exceptions back outside the DLL. The enumeration HRESULT is recorded;
normal HRESULT failures are otherwise ignored as in native code.

The factory delegates directly to the existing keyboard A9A3E0, mouse A9A290 and
joystick A99940 helpers. The keyboard uses the same current-window service as
the joystick; the mouse uses the caller's current explicit sample preimage on
each construction. The pad factory uses `new (nothrow) XInputDevice(index, api,
globals)`. These are concrete device classes. Constructors that return null on
allocation failure keep that behavior; the adapter does not fabricate a device.

## Actual reference lifetime

`DirectInputReferenceRegistry` is a noncopyable record of acquired COM references.
It records the interface creation reference, each actual AddRef, each device
reference exposed by a successful helper return, and each nonnull joystick
effect reference. Recording does not call AddRef. Multiple references to the
same interface pointer remain separate entries.

The runtime and registry do not implicitly Release during destruction. Existing
keyboard/mouse/joystick wrapper deletion still follows its recovered behavior:
borrowed pointers remain borrowed, and joystick destruction unloads its effects
without releasing them. After all wrappers and callbacks have finished using
those references, the caller explicitly invokes `release_tracked_references`.
It clears the bound interface slot if that object is tracked, then calls actual
IUnknown::Release in reverse acquisition order. Thus normally returned effect
references are released before their joystick device, and devices before the
initial interface references. An externally supplied untracked interface slot
is not cleared unless an AddRef on it was recorded.

This registry models a caller-selected final ownership boundary; it is not a
claim that native focus reset secretly releases COM objects. Calling the registry
directly through `release_all` instead leaves any external pointer slots for the
caller to clear. Neither runtime nor registry is copyable. Construction and
final release are single-owner operations and must not be reentered or run
concurrently. Bookkeeping capacity is reserved before ordinary factory calls.

The existing constructor helpers can throw after an SDK call but before returning
their wrapper (for example, later joystick allocation or a required window
service failure). Such a helper can conceal an already-created COM output from
this adapter. Those exceptional-path references are not falsely claimed as
tracked. Closing that separate helper-unwind boundary would require changes to
the helper contracts; this packet does not add COM proxy implementations or edit
the existing device modules. The existing bring-up also has no rollback of prior
devices after later allocation/attachment failure.

## Native evidence and validation

A982D0 is thiscall ECX=backend, no stack arguments, EAX=this, ending with RET at
A983B0. A98322 calls the GetModuleHandleA import with null. A98328..A9833B passes
null outer, the actual +E0 output slot, IID at D78D8C, version 800h and that module
handle to the C2E016 DirectInput8Create import thunk (IAT CE2050). A98340..A98348
immediately calls AddRef through +4; A9834A reloads the output before EnumDevices
through +10. A98360..A9839B constructs and attaches four native 240h XInput
objects. The IID bytes at D78D8C equal the SDK IID byte for byte.

Native code ignores the creation HRESULT and dereferences the output unchecked.
The existing typed `create_direct_input_devices` at `src/input_settings.cpp:478`
returns false when `create_interface` supplies no object, before AddRef. It also
reports null pad allocation instead of passing null into native attachment's
vtable dereference. These are explicit typed failure guards, not native failure
or SEH-unwind equivalence. No new Ghidra definition or flow gap was found in this
already complete constructor.

MSVC Win32 Release and both existing CTests pass; all eight native seed checks
match. One ignored local fixture calls actual DirectInput8Create and AddRef, then
two explicit Releases. The loaded module reports
`C:\WINDOWS\SYSTEM32\DINPUT8.dll`; creation returns HRESULT 0. It also verifies
the native IID, both context identity checks, the unbound-enumeration guard, and
four actual XInput wrappers attaching to the existing canonical table without
polling. Deleting those wrappers leaves the tracked COM references untouched.

The fixture does not invoke real EnumDevices, device acquisition, cooperative
levels, hardware samples, windows, focus changes or haptics. Existing constructor
fixtures cover their recorded SDK protocols; this packet's hardware proof is
limited to actual interface creation/reference lifetime. It does not establish
complete game startup or gameplay validation. Exact commands/results are in
`reports/directinput_runtime.json`.

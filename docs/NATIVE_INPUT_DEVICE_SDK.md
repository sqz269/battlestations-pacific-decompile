# Raw input device SDK acquisition

Addresses: none (source host for recovered COM call sites).

NativeInputDeviceSdk is the concrete acquisition boundary for raw keyboard,
mouse and joystick constructors. It calls IDirectInput8A::CreateDevice and
IDirectInputDevice8A::CreateEffect with the actual caller-supplied output slots,
GUIDs, descriptor and outer object. It returns the original HRESULT and preserves
every SDK-written pointer. It does not create a device projection or add a COM
reference beyond the SDK's returned reference.

One null tracking slot is allocated before each SDK call. Its numeric index
survives nested acquisitions; recording the returned pointer after the SDK call
does not allocate. Thus a later native constructor step can throw without hiding
an already returned COM reference behind an unreturned wrapper. Host bookkeeping
allocation can throw before the SDK call. Normal COM return semantics are required:
every nonnull output names one valid returned reference, including an error
HRESULT with a valid nonnull output. Arbitrary unmodified poison outputs or C++
exceptions crossing a COM boundary are outside that contract.

The host explicitly releases tracked references in reverse acquisition order,
only after all native owners, member cleanup and callbacks have stopped borrowing.
Release pops before the COM call. It is rejected while acquisition is active;
the class destructor does not release automatically. Native constructors retain
their own output-slot preimages and native cleanup order.

Initial validation: MSVC Win32 Release and both existing CTests passed. This
bootstrap supplies a shared concrete API for the independently reconstructed
raw device families. Device/effect runtime and constructor-failure evidence is
pending their integration; this document does not claim hardware enumeration,
successful full backend startup, original ABI compatibility or gameplay proof.

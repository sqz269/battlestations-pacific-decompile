# XLive device notifications

The two game entries C2F1C0 and C2F1C6 are six-byte import thunks.
They jump through IAT CE25D0 and CE25D4 to xlive.dll ordinals 5005 and
5006, respectively. The game image imports both by ordinal. The installed
game DLL names these exports XLiveOnCreateDevice and XLiveOnDestroyDevice;
the system Win32 DLL exports the same ordinals without names.

NativeXLiveDeviceAdapter forwards those ordinals through the module owned
by the caller's existing XLiveLibrary. Construction borrows the module;
each call resolves its selected export and invokes the Win32 stdcall
function. Create forwards two unmodified pointers, device then presentation
parameters. Destroy has no arguments. Both expose the returned EAX word
without interpreting it. A missing module or ordinal is a source binding
error. No SDK implementation or fallback result is provided.

The full 1,191-byte B29670 caller establishes the argument and ordering
contract. Its destruction notification is gated by the current F8ABE8 word
and occurs before the old renderer device is released. Its creation
notification is independently gated after device recreation and resource
restoration; it passes the current renderer+1A10 device and the presentation
structure at renderer+1A28. Both return values are ignored by that caller.
The adapter does not implement either gate, retain COM pointers, copy the
presentation structure, initialize XLive, or reconstruct the parent route.

Four fresh guarded reads matched 1,211 installed-PE bytes: both complete
thunks, their eight IAT bytes and the complete caller. Both local Win32 DLL
export tables were inspected as files. Their bytes and hashes are evidence
of available providers; neither DLL was loaded or called during this packet.

Strict MSVC Win32 compilation, both existing CTests and all eight native
seed checks passed. The main library and its exact adapter/library objects
were frozen. The two full compiled forwarding methods are recorded with
their COFF relocations. No new tests, SDK runtime comparison, linked-method
fixture, original IAT-mutation behavior, drop-in ABI or gameplay validation
is claimed. The existing SDK names in Ghidra are preserved.

Evidence: reports/native_xlive_device_adapter.json.



## Primary integration

The adapter is registered in the main Win32 build. Both original import names
were retained in Ghidra, reviewed ordinal/argument/caller evidence was appended
and saved, and both exports were refreshed. Ledger entries explicitly classify
the source as external import forwarding, without an SDK implementation or
runtime claim. The read-only proof retains the complete caller/thunk/IAT bytes,
both inspected DLL export tables, and the exact compiled adapter objects.

The primary library SHA256 is `2c19b1ed17be0e22e41b393ef6a609de785738f9b4707a492ca2e30bf4e5632b`. The read-only bundle is
`local/xlive_device_primary/`, seal `22b01d9f73413d865d497863681a4c7009fba613aff2d5da7c6fb0854d873ff5`.
Evidence is recorded in `reports/native_xlive_device_adapter.json`.

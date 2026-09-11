# Actual eight-byte event owner

The five complete bodies at `BD1970`, `BD19B0`, `BD1910`, `BD17C0` and
`BD1960` now operate on actual eight-byte event storage. Its first DWORD is
the original table identity; its second DWORD is the actual Windows HANDLE.
These are raw address identities, not executable C++ vtables. The existing
one-handle `Win32Event` wrapper remains available as a separate interface.

`BD1970` receives the raw manual-reset byte in CL and allocates eight bytes
through the existing operator-new service. If that call returns null, it returns
null. Otherwise it stores table `D6821C`, calls
`CreateEventA(nullptr, zero_extended_byte, FALSE, nullptr)`, and stores the
returned HANDLE even when null. The existing host allocator retries through the
CRT new handler and throws on terminal failure; its normal contract does not
produce the native factory's returning-null allocation case. No replacement
allocation policy or successful event fallback is introduced.

`BD19B0` captures HANDLE `+04`, installs the concrete table, and calls
`CloseHandle` unconditionally. It tests flags bit zero, stores base table
`D68208`, and frees the owner only when that bit was set. It returns the original
address, including after free. It does not clear the retained HANDLE word.
The `ADD ESP,4` continuation after the original free call had been omitted by
Ghidra's incorrect no-return analysis. That local flow is repaired, its prior
documentation is retained, and the complete function ends at `BD19DF` exclusive.

The other three routines load the current HANDLE once and return the actual
results of `SetEvent`, `WaitForSingleObject(handle, INFINITE)`, and `ResetEvent`.
They do not validate the HANDLE, retry or reinterpret the Windows return value.
Original ABI is ECX owner and RET for these leaves; deletion takes one stack
DWORD and uses RET4/EAX owner. The implementation exposes new C++ declarations
and does not install original binary entry points or tables.

[The audit](../reports/native_event_owner_audit.json) records nine current
Ghidra/installed-PE spans totaling 183 bytes, including all five complete bodies
(137 bytes), the profiles and the allocator/free hook preimages. The existing
project and program were verified before every analysis/annotation batch.

One private fixture runs the original five bodies and the main library against
real Win32 APIs. It compares 41 normalized result/state words across three real
event lifetimes (manual-reset bytes `00`, `80` and `FF`) and one borrowed owner
with a null HANDLE. It checks the actual table/handle fields, retained versus
freed deletion, original return address, infinite wait on a previously signaled
event, auto/manual reset behavior and unconditional null-handle close. The
native allocation/free boundaries forward to the existing concrete services;
the free boundary also checks the base table before releasing storage. Five
absolute IAT operands are relocated to real Windows imports. No API result is
simulated. Real event creation failure and allocation failure were not induced.

The strict Win32 build and both existing CTests pass. No tracked test case was
added. This closes the event owner and API leaves, not the worker constructor,
thread procedure, queue execution, renderer stop/pipeline or gameplay.

# Native particle-clock singleton source, 004DE4B0

The installed `battlestationspacific.exe` (SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`)
and live `/battlestationspacific.exe` in `C:/Users/sqz269/bsp.gpr` identify
`004DE4B0..004DE576`, 199 bytes, as `BSP_ParticleClock_GetSingleton`.
The complete PE span SHA-256 is
`fa1f9c042b0a54c37c68547e0fed4dc02526fdfe6a8911e5b66e46ed9d541ec2`.
The machine entry has no arguments, returns EAX and uses plain `RET`; the
source function adds two borrowed references to **current** application
publication cells, `01090AA0` (manager) and `00F8D420` (particle owner).
It is a callable source reconstruction, not an original-ABI replacement.

The first publication read is captured for an immediate nonnull return. A
miss calls raw `00415350`, captures `[first_manager+10h]`, forms the original
eight-byte guard with profile `00CE37FC`, and, for a nonnull section, calls
`EnterCriticalSection` before incrementing its raw `+18h` DWORD. Native
state 0 arms only after that increment. The slow path tests the current
publication, allocates `1Ch` only after a second miss, and writes the
actual owner in machine order: `+04=00CE7D08`, zero `+08,+0C,+10,+14`,
then `+00=00CE7D38`, finally `+04=00CE7D24`. **No store touches +18.**
Even a null allocation is published. A second raw `00415350` call obtains
the registration manager; `00BD0C30` receives a fresh publication read.
Normal release decrements the first captured section's `+18h` before
`LeaveCriticalSection`; the return reloads publication. C++ exceptional
cleanup calls the existing raw `00411EE0` guard destructor. The native
state-0 action `00D8FCD8 -> 00C66FA0 -> 00411EE0` supplies that evidence.

The six native external caller sites are `004DEED5`, `004DF2C4`,
`00B46CC2`, `00B3B2BA`, `00B33A13`, and `004E53A6`. At `00B46CC2`,
the system-constant builder immediately reads returned `EAX+18h` at
`00B46CC7`; this may be the allocation preimage until the separate
`00B19A10` setter writes it. The source therefore never maps the native
owner to host `ParticleClock` or repairs the first payload read.

Raw providers are `get_native_singleton_manager_00415350` (borrowed
`01090AA0` cell), `register_native_singleton_object_00bd0c30` (actual
manager/owner), and `destroy_native_singleton_guard_00411ee0` (actual
eight-byte guard). `singleton_lifetime_allocate` is a concrete host CRT
service asked for the same native and host `1Ch` byte count; the code also
uses real Win32 critical-section APIs. Its host `malloc`/`_callnewh` retry,
exception type, API import identity, and source C++ catch do not reproduce
the original BF681B CRT, CE2218/CE2210 IAT cells, or C66FA8 private FH3/SEH
handler. The stored vtable DWORDs are native identities, not host-callable
C++ vtables. Mixed-owner destruction, original binary calling convention,
gameplay, and first-use shader value remain unvalidated.

In the generated Win32 object, the compiler emits the second volatile
publication comparison immediately before its C++ EH-state store. The native
body stores state 0 immediately before that comparison. This does not change
the normal C++ exception path, but it is another explicit reason that the
source cannot claim native asynchronous-fault/FH3 behavior. The generated
owner-field stores and provider-call order are reviewed in the report.

`./scripts/build.ps1` built the Win32 Release targets and passed the existing
`reconstructed_math` CTest (1/1). These are source/build checks, not an
installed-game or shader-value observation.

`reports/native_particle_clock_singleton_cp.json` pins original callsite,
EH, source/provider, and generated-object evidence. The pre-existing typed
full-function ledger record remains; this packet is a raw-storage interface
upgrade with **zero new body/byte credit**.

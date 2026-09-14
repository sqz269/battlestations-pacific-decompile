# Native online manager base owner

This packet reconstructs the complete normal bodies of `00A3F530`,
`00A3F5D0` and `00A3F670` over the actual 0x3F0-byte
`NativeOnlineManagerStorage`. It uses the same `00F8ABE8` publication and
`01090AA0` singleton-manager cell as the raw sign-in, UI, notification and
pump modules. Descriptive names are hypotheses. The C++ interfaces add
explicit borrowed references and are not the original register ABI.

| Native body | Original ABI | Coverage and effect |
| --- | --- | --- |
| A3F530..A3F5C0 | ECX raw owner, EAX owner, RET | Complete base construction, section capture, early F8ABE8 publication and current-slot registration |
| A3F5D0..A3F668 | ECX raw owner, RET | Complete base destruction, current-slot unregister, clear publication and root profile |
| A3F670..A3F68D | ECX raw owner, flags DWORD stack, EAX captured owner, RET4 | Complete scalar wrapper; bit0 alone frees captured storage after destruction |

Construction writes base vtable profile `D24138` into raw owner+0 and calls
the existing actual `00415350` getter. It captures that first manager's
section at +10, enters it, increments the tracked DWORD at section+18,
publishes the raw owner to F8ABE8 at A3F586, gets the singleton manager
again at A3F58C, reloads current F8ABE8 at A3F591, then calls the existing
raw BD0C30 register body. It releases the *captured* section even if either
publication changes. No other owner byte is initialized; the application
allocation at 0073DC50 requests 3F0h and A40DF0 later writes derived fields.

Destruction writes `D24138`, captures/enters the first getter's section,
gets the singleton manager again, unregisters the *current* F8ABE8 pointer
through the existing raw BCFCA0 body, clears F8ABE8, releases the captured
section, then writes root profile `CE3818`. If current F8ABE8 differs from
the captured receiver, unregister targets the current pointer. The source
does not repair this native behavior or silently unregister the receiver.

Both native EH maps unwind the captured section before the root-vtable
store: constructor `DE9D90` uses `CB4128 -> 00411EE0` then
`CB4120 -> 00412430`; destructor `DE9DC4` uses `CB4148 -> 00411EE0`
then `CB4140 -> 00412430`. The source calls these established raw cleanup
providers. A registration exception may leave F8ABE8 published and possibly
registered while restoring CE3818; a destructor exception before the clear
likewise leaves the current publication intact. Host recovery must retain
the raw allocation until it resolves such a partial state. Native SEH frame
identity, hardware faults, cross-thread retirement, static-CRT exception and
allocator identity are outside this C++ interface.

The scalar wrapper captures the receiver before calling full base teardown.
It tests only flags bit0, then uses C++ `delete` on the captured raw object
and returns its original address value. A flags1 caller must supply exactly
`new NativeOnlineManagerStorage` (default initialization, without `()`)
or a source allocation with the matching delete domain, and relinquish the
owning pointer afterward. The trivial raw storage destructor frees no SDK,
profile, pump or projected state. Flags without bit0 retain the allocation.

The existing `native_singleton_publication` getter, raw BD0C30/BCFCA0
registration bodies, 00411EE0 guard and 00412430 root-profile cleanup are
the required concrete providers. The packet does not duplicate their
manager domain or add a new map. It does not implement derived A40DF0,
A3F9D0 or A3FDC0, IPC A4C250/A4C030, XLive/Winsock startup/shutdown, or
application allocation/publication at 73DC50/73DC87. The primary integrator
must bind D24138/D2413C singleton deletion to this raw allocation, retain
the same F8ABE8/01090AA0 cells through drain, and use the derived constructor
and destructor before enabling raw pump adoption. The older
`XLiveOwnerAllocation` remains a projected source interface and is not this
owner.

`./scripts/build.ps1` passed MSVC Win32 Release and both configured CTests
after `verify-seeds` confirmed all eight native differential seeds.
The focused local fixture used an actual 14h singleton record, actual raw
BD0C30/BCFCA0 bodies and a Win32 critical section with tracked +18 DWORD;
it verified early publication, untouched +3E8 preimage, current-slot
unregister and scalar bit0 free. No XLive SDK, IPC, account dialog, network
or updater was invoked. See `reports/native_online_manager_owner.json` for
exact PE bytes, native call rows, commands, hashes and evidence limits.

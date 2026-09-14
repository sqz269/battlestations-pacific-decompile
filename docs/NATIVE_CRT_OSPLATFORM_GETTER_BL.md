# Native CRT OS-platform getter BL

Address `00BFBAB2`, complete55-byte body ending before `00BFBAE9`.
This packet implements only the qualified context-bearing getter; heap mode
selection, Watson, PTD initialization and invalid-handler ownership remain external.

Native cdecl takes one DWORD output pointer, returns status0 or22 in EAX and
uses a plain RET. It captures output in ECX before any platform access. Null
output skips the platform read. Otherwise BFBADA loads actual109DD84 **once**;
the nonzero branch stores that same captured EAX at BFBAE3 and returns0. This
is not a second volatile platform read after the test.

On either failure, BFBABD calls BFFB8B with no stack arguments. BFBAC2..C6
push five zero DWORDs from ESI, BFBAC7 writes22 through the returned errno
pointer, and BFBACD calls BF66EF. BFBAD2 cleans14h stack bytes. If the service
returns, the getter returns22 without a getter output store, retry or catch.
Actual provider side effects and propagated source exceptions remain observable.

The new interface reuses `NativeCrtPointerDecodeSupportContext`, exactly the
borrowed context used by existing BFBB61 in native_crt_pointer_decode_support.cpp.
Its winmajor binding is unused here. Existing owning_crt.errno_location_00bffb8b
and invalid_parameters.invalid_parameter(context) supply the established error
and all-five-zero invalid-parameter operations. No new callback interface, fresh
errno cell, synthetic OS word or global/allocator owner was introduced.

The source captures the nonzero platform and copies that captured value. Null
output never reads the platform. Failure calls the owning errno accessor, writes
22, then calls the established invalid service. It adds no output initialization,
error translation, fallback, handler repair, rollback or noreturn assumption.

These service bindings are qualified, not proof of original CRT ownership.
Native BFFB8B calls C051B7 and selects PTD+8 or canonical E159E8; BF66EF decodes
actual109DD64 and dispatches the handler or the Watson path. Host _errno and UCRT
handler domains do not establish those original owners. The caller must supply
the actual established borrowed services appropriate to its execution domain.
The additional source context and service call do not reproduce native outgoing
five-word stack layout, private ABI, registers/EFLAGS, asynchronous fault identity,
Watson termination, original process state or gameplay behavior.

Evidence inputs are accepted heap frontier c7a9ae6877b711c7a7e196b4039b5ae4b71373ee
and Watson/error frontier ed185cb2, plus the existing BFBB61 provider composition.
Current verified BSP queries retain the complete55-byte PE/live match and both
native call rows. The report records exact current compiler command/read/all
matching write groups, sole /Fo directory, current source/header/object hashes,
unique archive-member equality, strict full Win32 build, verify-seeds and both
existing CTests. No new test case or native/game runtime execution was added.

Only this getter source/header, this doc/report, its BFBAB2 ledger record and one
deferred CMake source registration belong to the packet. No Ghidra mutation was
performed. The report inventories the complete retained local evidence with size,
SHA256 and SHA512, independently audited twice outside that local directory.

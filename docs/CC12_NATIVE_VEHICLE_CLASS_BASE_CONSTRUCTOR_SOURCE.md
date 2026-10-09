# Actual vehicle-class base constructor: Source candidate

`construct_native_vehicle_class_base_00749050` reconstructs the complete small
base constructor over borrowed actual class storage. It calls the existing
`construct_native_damageable_class_0087c640`, then performs the observed vehicle
profile and field stores. It does not allocate the class, substitute a semantic
`VehicleClassDescriptor`, create a factory, or load a model. The descriptive name
follows the existing vehicle-class allocation and derived-layout contracts; it
is not a recovered symbol.

This is a qualified Source candidate with a new ordinary MSVC Win32 C++ ABI.
Primary registration and admission remain pending. There is no Original ABI,
fixture, startup, production-owner, or gameplay claim for this candidate.

## Complete Native gate

The sole new Native function inspected is `00749050..00749131`, inclusive:
226 bytes, 46 instructions, and one direct call at `00749069` to `0087C640`.
Original PE bytes, fresh live Ghidra bytes, all saved/live assembly rows, and all
46 independent decode starts agree. The function bytes have SHA-256
`2817ba9c783f5feb52d50141d115f9776fd9678fd234180370acbf142f0a028d`.
The original image has SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The analysis process was verified as the existing `C:/Users/sqz269/bsp.gpr`
project, with sole program `/battlestationspacific.exe` and 64,730 functions.
The saved exports are shared through the main checkout. No new Native child,
profile-table data, or handler body was opened. No function mutation or
annotation write was performed. Existing child Source is a dependency, not a
fresh Native child gate.

Assembly establishes ECX as the actual class pointer, ESI as its preserved local
copy, EAX as the same pointer on return, and a bare `RET`. The routine installs
the original FS exception frame with handler identity `00C874BB` and state -1;
it does not arm a local cleanup state around the sole child call. No x87,
indirect-call, or external post-child call occurs in this body.

## Exact writes and storage extent

After `0087C640` returns, the complete ordered caller schedule is:

1. DWORD `+6Ch = 00CFF378h`, then DWORD `+0h = 00CFF7CCh`, then DWORD
   `+6Ch = 00CFF7C8h`. The first secondary-profile write is transient and retained.
2. Zero DWORDs at `+74h, +78h, +94h, +98h, +9Ch, +B4h, +DCh, +E0h, +E4h,
   +E8h, +ECh, +F0h, +F4h, +F8h, +FCh, +100h, +104h, +108h, +10Ch, +114h,
   +118h, +11Ch`, in that order.
3. Zero only the byte at `+120h`.
4. Zero DWORDs at `+128h, +12Ch, +130h`, then `+80h` last; return the original
   class pointer.

These are 30 writes: three profile DWORDs, 26 zero DWORDs, and one zero byte.
The new Source uses volatile stores so the transient write, widths, and order
survive the reviewed optimizing compilation. The profile numbers are opaque
identities; this code does not dereference or invoke them.

The caller writes through byte `+133h`, establishing a `134h` accessed extent.
It does not itself prove an allocation of `138h`. The `138h` storage requirement
comes from the existing `kVehicleClassDescriptorBaseSize` and derived ship
extension at `+138h` in `include/bsp/vehicle_class.hpp`, together with the
`138h` Shipyard class allocation entry in `src/vehicle_class.cpp`. Those are
existing Source/layout contracts, not newly inspected Native allocator bodies.

All other bytes remain as the real child left them. In particular, this caller
preserves `+110h..+113h`, `+121h..+127h`, and `+134h..+137h`. It does not initialize
the class index at `+70h`, engine count at `+7Ch`, or linked-class index at `+C0h`.
The older `docs/VEHICLE_CLASS_DESCRIPTORS.md` prose broadly groups DWORD indices
`37h..47h` and infers the base size from the highest store. This gate refines
that description: the `+110h` gap is real, and access extent and allocation
extent require separate evidence. The historical document is left unchanged
outside this packet's file ownership.

## Actual child, aliasing, and lifetime contract

The caller must provide fresh, writable, four-byte-aligned actual vehicle-class
storage of at least `138h` bytes. No whole-object zeroing or value initialization
is added. The access object, its two actual child profile identities, and its
real string-pool publication domains must be valid and stable for the complete
call, including child failure cleanup. Their storage must not overlap the class
allocation or the child's tree node. This is raw Win32 storage under the
project's existing actual-layout contract, not a portable owning C++ class.

The real `NativeDamageableClassConstructionAccess` is reused unchanged. Its
existing Source child constructs the damageable base, sets the reference count,
initializes its embedded headers, obtains the real `58h` sentinel through the
canonical allocation/new-handler domain, publishes it at `+64h`, and completes
the sentinel links and count. Its existing failure path releases the current
embedded headers through real services; a cleanup exception terminates within
that child. No callback or replacement allocator was introduced here.

The new caller ignores the child's returned value and continues using the
original actual pointer, as the assembly does. No class retain/release,
ownership transfer, model ownership, registry insertion, or allocation free is
added. Existing child initialization and its partial state remain observable.

If the child throws a C++ exception, no vehicle-specific store has happened;
the new caller propagates the child's existing cleanup result without an outer
catch, rollback, replay, or class-allocation free. On successful child return,
the remaining ordinary stores introduce no C++ throwing operation on valid
storage. Hardware faults, longjmp, the private FS/FH3 mechanism, and identical
binary exception behavior remain outside the demonstrated C++ contract.

## Compilation and complete candidate-object review

The new translation unit and actual existing child translation unit both
compiled successfully with the installed MSVC x86 compiler, C++17, `/EHsc`,
`/O2`, `/W4`, `/WX`, `/permissive-`, `/Zc:__cplusplus`, and `/MD /DNDEBUG`.
They used the real project headers and installed SDK; no fake includes or stub
symbols were supplied. The receipts pin 96 and 113 actual compiler inputs,
respectively, plus compiler, dependency manifests, logs, and objects.

The candidate object is 2,144 bytes, SHA-256
`e6a15cbc3fef22ff426a449f7c4b7d81c80cf0d1dc850add6db4ecd3fb2227df`.
Its unique constructor root is 303 bytes and 39 decoded instructions. Every
emitted object store matches the Native offset, width, value, and order. Its
single code relocation is the genuine `0087C640` C++ symbol, with a positive
definition in the separately compiled 8,895-byte real child object.

All candidate COFF sections, symbols, relocations, and both code COMDATs were
reviewed. The second COMDAT is an unreferenced 16-byte, five-instruction ordinary
DWORD store helper; it has no call. Total candidate code is 319 bytes and 44
instructions. A four-byte compiler/CRT weak `.bss` symbol is also present.
There is no candidate local C++ unwind map or frame-handler reference. The
child's complete emitted body was not re-reviewed. These were object-only
compilations: no probe, linking experiment, fixture, or candidate execution.

The required `scripts/build.ps1` baseline run passed both configured checks,
`reconstructed_math` and `tool_tests` (16 Python cases), and is recorded
separately in the JSON report. This worktree has no `local/seed_reference.hpp`,
so Native math differential tests were not configured; seed scope was not
extended. The candidate is absent from
`CMakeLists.txt` and `cmake/startup.cmake`; that baseline build cannot validate
its integrated inclusion. Primary review must register it, build the resulting
scope, and inspect the resulting integrated artifact before any admission.
No new tests were added for this direct, fully inspected store sequence.

## Context and remaining boundary

Twenty-three existing project/compiler and extent-contract files match the
primary checkout after line-ending normalization at the recorded capture time.
This includes the actual child implementation and required headers; compiler
inclusion of a transitive header is not a fresh behavioral review of that file.

The supplied Source511 primary receipt is recorded only as earlier integration
context: three checks passed, with 44 Core objects, one App object, and 52
positive Core roots. Its separate bounded startup receipt reports three ticks,
two successful Presents, one skipped Present, and zero mission frames. No
primary build artifact was opened or compared, and this candidate was in
neither that receipt nor its startup. Source511/earlier source counts and prior
fixtures do not become candidate validation by being mentioned here.

This closes the small base-constructor Source prerequisite identified in
`CC12_UNIT_CLASS_MODEL_PUBLISHER_OWNER_READINESS.md`. It does not construct the
production vehicle-class owner, complete the Lua class-row loader or Mesh
publication, bind actual resource selection/graph services, or prove mission
execution. Those contracts remain separate from this complete constructor.

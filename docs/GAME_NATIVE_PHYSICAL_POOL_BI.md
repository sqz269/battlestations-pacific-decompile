# Game native physical-provider pool process lifetime

The source projection of the actual `0109DBF0` physical-provider pool now has
one process owner, `bsp::game::game_native_physical_pool_process()`. Its raw
`38h` storage, `NativePhysicalProviderPoolContext`, allocator-list head
corresponding to `00E188B4`, and `AllocatorListDomain` share that lifetime.
`GameNativeVfsRuntimeInputs::physical_provider_pool` borrows the retained
context. Other native allocator owners must borrow this same domain; the VFS
wrapper does not create a second allocator registry.

The game entrypoint must explicitly call `initialize_once_00cd9010()` and
inspect its returned CRT status before admitting VFS startup. The call binds
the actual `D68E68/BF3430` trim profile to the common domain and then invokes
the existing `initialize_static_native_physical_provider_pool_00cd9010()`.
That wrapper runs `BF3250` source construction and registers the existing
`CE10F0` source destructor with `std::atexit`. After the call returns, the
process owner supplies `physical_provider_pool_context_0109dbf0()` for the
VFS input. This accessor performs no lazy startup. The game entrypoint wiring
and source build registration are separate integration work.

The function-local process owner finishes C++ construction before
`initialize_once_00cd9010()` is entered, so its C++ destructor is registered
with CRT exit first. `CD9010` registers native `CE10F0` second. CRT callbacks
run in reverse registration order: `CE10F0` calls `BF33A0` while both the raw
pool and shared `AllocatorListDomain` exist, then C++ destruction releases
only the domain's host bookkeeping and raw storage. Registering `CE10F0`
inside the process owner's constructor would reverse this order and leave
`BF33A0` using a destroyed domain. Other CRT users of the shared domain must
also be registered after construction of the process owner.

One guarded startup attempt is retained. A completed second call returns the
original `std::atexit` status without reconstructing the critical section or
registering another destructor. If the first attempt throws, later calls
reject retry; the source does not guess whether native partial construction
survived. If registration returns nonzero, `BF3250` has still constructed the
pool: the status is cached, the context remains available, and no rollback or
manual `BF33A0` call is invented. The caller decides startup admission from
that status. Destruction of an individual VFS wrapper never trims or destroys
the process pool. The native exit callback alone owns pool teardown after a
successful registration.

## Focused verification

An ignored Win32 child probe (`local/probe.cpp`, `local/probe_build.cmd`) was
built using MSVC `/MD`, linked with `/MANIFEST:EMBED`, and exited with code 0.
It verified that context access before explicit startup throws, both startup
calls return zero, the context references the common domain, and a physical
slot can be acquired and returned. A callback registered after startup saw
the live pool with one block. A callback registered between process-owner
construction and native registration saw the pool unlinked with base profile
restored; it also invoked `AllocatorListDomain::trim_all_004b46b0()` on the
empty list while the domain remained alive. An older callback then ran after
the C++ owner destructor. The observed order was:

```text
main pool=initialized slot=returned once_status=0
before_native_cleanup head=pool blocks=1
after_native_cleanup head=empty domain=alive
older_than_process
exit_code=0
```

This is source and host CRT lifetime evidence. It does not execute the
original game process or establish fixed-image address identity, original ABI
or FH3 equivalence, forced `std::atexit` failure behavior, VFS/gameplay
admission, or gameplay correctness. The underlying native lifecycle and slot
evidence is in `docs/NATIVE_PHYSICAL_PROVIDER_POOL_LIFECYCLE.md` and its report.

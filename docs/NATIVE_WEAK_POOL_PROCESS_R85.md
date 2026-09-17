# Weak-handle pool process integration

Addresses: 00cd8a60, 00ce1040

## Result

The application now explicitly calls the existing CD8A60 reconstruction after
the resource hierarchy pool and before the physical provider pool. One process
owner supplies actual38h storage for 0109CE94, the neighboring 0109CE90 mutex
publication, and the existing shared E188B4 allocator list. No extra slab table,
free list, or allocator-list head is introduced.

GameSingletonHost::weak_owners() lazily composes the existing weak-owner domain
over that process pool and the canonical raw14h singleton manager. It installs
the D190B4/925430 deletion binding before returning the domain to any consumer.
The domain remains alive through the host's explicit or destructor-driven drain.
The native mutex itself remains lazy, created only by the recovered weak-base
path. All scene/camera consumers must retire before the application drains.

This is integration of existing reconstructed bodies, not newly recovered
native code or a claim of native entry ABI compatibility.

## Native evidence and lifetime

CD8A60 is 22 bytes: load ECX=0109CE94, call B004B0, push CE1040, call BF6FF5,
discard the argument and return its EAX status. CE1040 is 10 bytes: load the
same ECX and tail-jump to B002C0. Both complete bodies and 0109CE90..0109CECB's
60 loader-zero bytes match the installed PE. The initializer table places
CD82D0 at CE35B0, CD8A60 at CE363C and CD9010 at CE369C. This establishes their
relative order among the currently bound initializers, not complete CRT parity.

The new process accessor finishes C++ static construction before CD8A60
registers native cleanup with real std::atexit. Therefore CE1040 runs before
the companion and common allocator-list owner destructors. The C++ destructor
does not repeat native pool destruction. Repeated successful startup returns
the first registration result without initializing the pool again. A throwing
first attempt is not retried. A nonzero registration result retains the pool
without rollback or an invented cleanup callback, matching the wrapper.

Pool access requires completed explicit startup. The host domain and process
publication are intended for the existing single application lifecycle; they
do not introduce support for independent concurrent application managers.

## Validation

- Strict MSVC Win32 build and all three existing CTests pass. No repository
  tests were added.
- 120 selected bytes match live Ghidra and the PE: 32 complete wrapper bytes,
  28 initializer-table bytes and 60 loader-zero bytes. Two direct calls and
  the explicit tail jump pass the call-site verifier.
- A focused executable reuses R84's six original/source in-place scene cases,
  now using the production process pool and GameSingletonHost domain. Empty,
  12-byte and 513-byte names, whole normalized24h storage and retained weak
  handles match. The raw manager drains the weak mutex and string pool; the
  retained weak handle survives scene destruction with a null target, then
  returns through the same pool. Final D3D API references are zero.
- The probe verifies pre-startup rejection and that repeat startup preserves
  the pool table and shared-list links. It leaves the slab/table for real CRT
  cleanup. A callback registered immediately before CD8A60 runs after CE1040
  and verifies that the shared allocator head is restored while the companion
  still exists and the weak mutex publication is null. This proves normal
  exit ordering/unlinking, not allocator instrumentation or exception parity.
- The current application reaches weak-pool startup with atexit=0, then exits1
  at `FMOD bank raw-length output unavailable`, before window/device creation.
  The immutable R84 executable, hash-checked against its sealed archive, fails
  at the same point under the same run options. Thus the application failure
  predates R85. Earlier two/eight-frame evidence cannot establish a successful
  current run. Both failed runs are preserved in the evidence archive.

The reused probe also executes R83's four real format comparisons and six
controlled HRESULT cases; those are dependency checks, not new pool coverage.
Copied scene bodies call concrete source weak/string adapters and trap native
EH. No full distortion/post20/camera initializer, populated scene roots or
lighting, failure injection, original CRT machine execution, or gameplay was
validated here.

## Next work

Investigate the FMOD bank create/length path against the current file bytes and
installed library, preserving the native error and output contract. Then
assemble the full post/material and camera runtime, bind B107F0 resource
startup, and validate the application and gameplay.

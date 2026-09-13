# Actual Dyn engine world factory

`dyn_engine_create_world_00c420e0` allocates and fully constructs a native-sized
world, then appends it to the actual engine-owned pointer vector. It composes
the existing engine, world and scene owners; it does not construct a settings
projection. The existing name `Dyn_Engine_CreateWorld` is retained as a
descriptive hypothesis.

| Entry | Native contract | Coverage |
| --- | --- | --- |
| 00C420E0..00C421A3 | ECX engine; one stack descriptor pointer; EAX world; RET4 at C421A1 | Complete normal valid-storage, successful-allocation body; native SEH/unwind excluded |

The 196-byte span contains 69 instructions after the root orchestrator repaired
the false-free continuation. The missing inclusive range C42177..C42179 is
`83 C4 04`, `ADD ESP,4`, after `_free` at C42172. The continuation publishes the
replacement vector and proceeds to the append; `_free` does not return from
the factory. The worker made no Ghidra mutation.

## Receiver, result and actual storage

The sole caller is the game constructor 004DDB90 at 004DE1D3. Its ESI receiver
comes from ECX at 004DDBAB. Engine ensure C55F50 returns EAX at 004DE129; the
subsequent descriptor MOVSS stores preserve EAX. At 004DE191 the caller pushes
the descriptor pointer, moves EAX to ECX, and publishes the engine at game+14.
The factory's EAX result is stored at game+18 by 004DE1DB. This packet records
that caller contract without modifying or executing the game constructor.

The public API takes `DynEngineStorage&`, the existing `DynWorldDescriptor&`,
and `DynWorldRuntimeContext&`. Its scene context supplies the existing memory
service and the initialized engine/dispatch/table bindings required by the
full world constructor. Engine storage is the actual 14h object established by
C55EA0; no second layout is declared. Its leading fields are:

| Offset | Producer/meaning |
| --- | --- |
| +0 | C55EA0 initially writes zero; C420E0 publishes the allocated world-pointer buffer |
| +4 | C55EA0 initially writes zero; C420E0 increments the number of appended entries |
| +8 | C55EA0 initially writes zero; C420E0 publishes pointer capacity before growing allocation |

The factory's engine receiver and the world context's engine-global slot are
separate native inputs. The game caller and fixture supply the same owner.
The API does not replace either input or silently alter global publication.

## Native sequence and boundaries

1. Allocate 48Ch bytes through BF681B at **C420FF**. C420F8 is the preceding
   `PUSH 48C`, correcting the older call-address label in the settings evidence.
2. If allocation returned nonnull, push descriptor and world, then call the
   complete C41AD0 constructor at C4211B. Its `RET8` removes both arguments.
   EBX holds the resulting world, or remains zero for the explicit null branch.
3. Compare engine count with capacity. Only equality triggers growth. Compute
   `new_capacity = old_capacity * 2 + 2` and `bytes = new_capacity * 4` with
   native unsigned 32-bit arithmetic. Store capacity at C4213F before BF55BE
   allocation at C42142.
4. Copy the existing count of world pointers in order. Free the old nonnull
   buffer at C42172, then publish the replacement at C4217A.
5. Store the world at `data + count*4` if that computed destination is nonzero,
   increment count, restore the SEH chain, and return the world.

The source preserves the explicit null-world and null-destination tests and
the native capacity mutation before allocation. The supported domain requires
coherent engine storage, valid context services and successful child/storage
construction. Wrapped extents, corrupt count/capacity, failed Win32 setup,
native SEH/OOM unwind and concurrent engine-vector mutation are not supported
or fixture-validated. In particular, no C++ exception cleanup is claimed for
the native CC81B6 handler. This API supplies no engine/world destructor.

## Verification

The ignored fixture extends the preserved real-owner connection fixture. Both
sides run actual engine/profile/task-manager and dispatch startup first. The
original side executes the full original C420E0 and C41AD0 instructions; the
source side calls the new factory and existing full world/scene implementation.
Four worlds exercise count/capacity pairs **1/2, 2/2, 3/6, 4/6**, verifying
initial growth, buffer reuse, growth with copy/free, and reuse after growth.
Each returned pointer equals its actual engine-list entry, and earlier world
pointers retain their order.

All 280 live-buffer comparisons and 111 final allocation/free observations per
side agree. The two 9,380,996-byte streams share SHA256
`75303cf20461c3bed315a6b60554b3d10498a01c190eb9845733bf72eafd54fd`.
Win32 Release, both existing CTests, and all six direct native call checks pass.

Each allocation/free observation records its kind, extent or canonical freed
pointer, engine/profile globals, engine count/capacity and the engine's current
list pointer. This checks publication and free order as well as final bytes.
Snapshots compare all live engine/profile/manager, world/scene/pool and vector
storage, including untouched A5 bytes and spare vector capacity. Only proven
pointer fields/vector words, validated handles and the three established kinds
of `CRITICAL_SECTION.DebugInfo` fields are normalized. Freed old vectors are
removed from comparison regions before memory can be reused.

The fixture retains genuine Win32 objects, original method-table slots, real
CRT `atexit`, the established normal CRT vector-constructor iterator, and the
shared reconstructed BF7030 numerical service. At control word 027F and positive
direction inputs, the fail-fast external `__87except` boundary is never reached.
No collision or solver method executes. Exact byte counts, source/image hashes,
native call checks and test results are in `reports/dyn_world_factory.json`.

After the comparisons, fixture disposal deletes the eight world manifold locks,
closes the two zero-worker managers' 204 handles and deletes their locks, then
frees the quiescent arena allocations. It preserves the native untouched
manager+0 field and therefore does not call those managers' unsafe fresh-zero
destructor path. The exact native/source dispatch callbacks execute through
real process exit. This is factory ownership/storage evidence, not world or
engine destructor validation, game startup, body creation or physics stepping.

No tracked tests or existing owner modules were changed. The CMake registration
only appends the new source. The prior settings interface remains a separate
partial projection; this factory uses the complete runtime world constructor.

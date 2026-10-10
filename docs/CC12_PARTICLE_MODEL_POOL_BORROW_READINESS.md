# Particle model pool borrow readiness

The existing application pool is ready to lend its actual Source storage through
`game_native_particle_pool_process().model_pool_00f8d2d0()`. No pool accessor or
pool implementation change is needed. The smallest later binding connects this
same borrowed pool to raw allocation, constructor-failure return, and the
existing derived-model lifetime access. This does not make model activation
ready: its canonical owner, concrete construction callees, and retirement order
still need an accepted application binding.

This is a read-only readiness packet based on main
`68cfbc7d572951d4fc7bc89bdf2d0a46742a26b1` and the accepted
`Source3352-particle-model-typed-views` epoch. No C++, build, test, fixture,
Native body execution, game run, or Ghidra mutation was performed. The typed
manager patch changed two access declarations only; it did not activate models.

## Existing production owner and startup

`GameNativeParticlePoolProcess` owns aligned, zero-initialized 38h model storage
and a distinct parameter pool. Its constructor borrows the existing physical
process's `AllocatorListDomain` for E188B4; it does not create another list.
The process is a noncopyable function-local static. These are Source projections
of the named original globals, not a claim that C++ storage occupies F8D2D0.

The model startup method holds its startup mutex, marks the native attempt as
`threw`, binds the actual storage and shared list, invokes CD7830, stores the
actual `std::atexit` status, then marks the attempt `returned`. A later call
returns that original status; a native attempt that threw cannot be retried.
CD7830 calls the reconstructed AF6860 initializer, then registers CE0B90 with
the real CRT. The pool already has reconstructed slab allocation, slot return,
trim, destruction, and allocator-list registration.

The accessor requires the model startup to have returned. Both zero and nonzero
CRT-registration statuses leave the initialized pool available; nonzero means
the CRT cleanup callback was not installed. It does not justify retrying,
rolling back, or installing replacement cleanup. The returned pointer is the
address of the same process-owned storage, not a copied pool or publication
cell. Holding that pointer does not retain any model or acquire lifetime
ownership. The startup mutex is released before the pointer is returned.

The actual normal application path calls the model startup and then parameter
startup at `game_main.cpp:1052`, before constructing `GameStartupHost`. Startup
failure exits that path. Existing VFS qualification modes return earlier and do
not establish pool startup. On the normal return path, `host.reset()` runs
before WinMain returns and CRT process teardown begins. Interrupted paths that
use `_Exit` bypass CRT cleanup and do not prove normal model retirement.

The getter's `returned` state is not cleared by CE0B90. It is therefore not a
post-cleanup validity check: a caller must never borrow or use the pool after
native destruction. The process storage and its shared allocator-list domain
must remain alive through that cleanup. Existing process construction precedes
native callback registration; C++ bookkeeping destruction does not substitute
for the native pool drain.

## Exact minimum future binding

There is no declaration mismatch to repair. `model_pool_00f8d2d0()` returns
`void*`, which is already the type required by the following three boundaries:

| Existing boundary | Required use of the same actual pool |
| --- | --- |
| `RegisteredModelEffectCallees::allocate_model_slot_00af6b60()` | Call `allocate_native_particle_model_00af6b60(pool)` and return its raw slot. |
| `RegisteredModelEffectCallees::return_model_slot_00af62f0(raw)` | After constructor unwind, call `return_native_particle_model_00af62f0(raw, pool)` exactly once. Preserve the hook's existing terminal `noexcept` contract. |
| `NativeParticleModelLifetimeAccess::actual_pool_00f8d2d0` | Store the borrowed actual pool address so AF7F40 returns successful models to this pool after derived/base destruction. |

Copying this stable storage address into the existing `void*` access field is a
pool borrow, not a cached manager publication. Keep the separate F8C274 manager
fields as typed const-volatile references to their existing cell; do not replace
them with a captured manager value. No additional accessor, pool wrapper,
context owner, lazy initializer, or publication mirror is indicated.

The returned allocation is an actual 2E0h slot with a 2DCh model payload and its
pool/slab ID at +2DC. AF69E0 produces it from 32-slot slabs. AF60B0 reads that
trailing ID and returns the slot to the same pool. Preserve that ID and all
constructor-unwritten payload bytes. Never use the distinct 188h model pool at
01090054, call the base-model scalar delete, or replace raw return with `free`.

`NativeModelOwner(slot, native_particle_model_slot_bytes, environment)` can
prepare its canonical base views over this larger slot: the existing size
guard is a minimum of 188h, not an exclusive size test. Preparation starts node
and model-tail lifetimes while restoring the first 184h bytes, then binds the
one external scene association. It does not construct the model, inspect its
uninitialized fields, duplicate the native count, or acquire a reference.
The larger slot's +184 bytes are derived data; its real slab ID remains +2DC.

AF74A0 then requires the prepared owner, the selected variant with a live native
string at +08, one shared retained-owner lookup across base/mesh/section/material
accesses, and matching actual name storage. Its profile hook installs the
canonical `NativeParticleModelReference` on the same node binding after D5DA50
publication. Do not create `NativeModelReference`, whose scalar deletion uses
the wrong pool. The existing GUI uses of `NativeModelOwner` demonstrate the
separate 188h base path and are not particle-model producers.

`NativeModelEnvironment` still contains its distinct base pool reference; that
does not authorize allocating or returning a derived slot through it. Its node,
scene, type, name, profile, constant, and retained-owner dependencies must all
be genuine and remain valid through the derived cleanup path.

## Required lifetime and failure order

1. Complete the existing model-pool startup, then obtain the raw pool borrow.
   Keep allocator-list mutation, trim, shutdown, and model clients within their
   existing synchronization contract. The accessor's mutex does not guard later
   allocation or retirement; trim has no internal lock.
2. Allocate from this pool, prepare the canonical base owner, and run AF74A0.
   Any abandoned prepared owner must relinquish its external scene binding.
   Constructor failure must finish the existing base/member unwind and retire
   any derived companion bookkeeping before the caller returns the raw slot.
   `registered_model_effect.cpp:56` already orders constructor throw before its
   AF62F0 hook. No successful-model destructor belongs in that raw-return hook.
3. Keep both access objects, the canonical lookup, scene/attachment runtime,
   profile/type cells, and all reached terminal services alive for callbacks
   and reentrancy. On successful zero-reference retirement, the existing path
   runs AF6C50, base destruction, AF60B0 raw return, runtime unbind, then the
   companion disposal callback. Preserve that order and its no-access-after-
   disposal rule. The future binding must meet the existing domain's ownership
   and synchronization requirements through this whole interval; this review
   does not prove concurrent slot reuse safe.
4. Retire models while the manager pointee is still live. AF6C50 reads the
   current manager after variant/mesh release callbacks, decrements the live
   count, and unregisters the actual model. The manager's weak pointer arrays
   do not keep models alive; its drain frees those arrays without destroying
   their models. A permanent publication cell is not a manager keepalive.
5. Finish these model lifetimes before the application's singleton-manager drain
   and before CE0B90 reaches AF5FF0. Pool destruction frees slabs and table,
   drains positive signed section depth, deletes the critical section, and
   unlinks the same allocator-list element. It does not run model destructors,
   clear all metadata, free the static owner, or retire companions.

AF69E0 allocation exceptions retain its native lock/depth and earlier
publications. No raw slot has necessarily been returned to the caller in that
case. Do not invent an unlock, rollback, retry, or successful empty allocation
as part of a borrow adapter. The existing application has not yet supplied the
concrete particle-model lifetime graph that would establish these obligations
during actual startup, shutdown, or failure.

## What remains before activation

The complete tracked search corpus contains nine construction-access mentions,
eight lifetime-access mentions, and three construction-callee-class mentions;
these are existing declarations, signatures, helpers, and borrowers. It has no
concrete owner/initializer for either access and no implementation of
`NativeParticleModelConstructionCallees`. The effect factory's allocation and
return calls go through its abstract callee interface; they are not a production
binding to the pool. The existing particle parser/runtime borrows F8D344 for
parameters and owns parser/loader contexts; it is not this model-owner bridge.

Required remaining model dependencies include the canonical environment and
owner/companion retirement, current D5DA50 profile and F8D308 type views, live
count F8D2C8, variant/emitter owners and preparation, current resource/shadow/
renderer publications, mesh/material/section/layout and parameter services,
real renderer virtual5C, and canonical RandomThreads arithmetic. These need
their actual owners and callback contracts; the pool borrow supplies none of
them. The next packet should locate and qualify that concrete application
binding, then request separately scoped C++ authorization. Adding an unused
accessor or a success stub would not close this frontier.

## Evidence and scope

All 3,356 accepted epoch pins (3,352 project inputs plus four artifacts) matched
whole frozen and current Root bytes before copying. Seven worker newline
variants are retained separately and agree after LF normalization. Complete Git
commit/root-tree evidence proves `src`, `include`, `cmake`, `scripts`, and
`CMakeLists.txt` unchanged between accepted C++ `99b170231` and captured main
`68cfbc7d5`; the separate current-main recheck is recorded in the report.

The selected Source read set has 3,358 files: the accepted epoch plus the actual
14-TU compiler dependency closure and build inputs. Its 462 tracked dependency
paths are a separate count. Six selected files were outside the old epoch,
including both pool implementation files and the existing process owner; their
complete committed/Root/worker bytes are retained. The exhaustive query corpus
has 4,185 tracked C/C++ files under src/include/tests; the union retained for
queries and selected Source proof is 4,214 paths. Every retained Source path is
bound to its actual Git blob and verified separately against the live checkout.
All 56 compiler tracking logs are retained; the 310 external compiler/SDK paths
are enumerated, without claiming complete SDK/toolchain preimages.

The complete Core archive has 1,991 entries and 1,988 COFF members. The current
App project, `Cl.items`, and object directory agree on 80 objects. Prior Root
metadata retained 70 App objects; that is a distinct reviewed subset, not the
whole current census. The report lists the ten additional members and makes no
missing-before-snapshot comparison. All 80 current whole objects are retained.

Fourteen relevant actual compiler TUs (11 Core and three App) have full COFF
indices: all 6,604 sections, raw primary/aux symbols, labels, payloads, and
relocations. Their 3,593 code sections contain 269,628 bytes; no equality or
behavior claim is made for every function merely because it is retained.
Twenty-five sections with declared storage and raw pointer zero retain empty
payloads correctly. Sixteen selected complete functions are decoded (9,192
bytes, 2,522 instructions), including startup, the getter, raw returns, canonical
preparation, derived retirement, WinMain, and singleton drain.

The compiled getter checks `returned` at observed this+AC and returns this+04;
no total class-size claim follows. CD7830's full Source body calls AF6860 before
`atexit`; AF7F40 calls AF6C50 before AF60B0. A census over every Core member and
all 80 App objects finds one getter definition and zero code-section relocations
to it. This agrees with the Source search: startup exists, model borrowing is
not yet wired. The census is not a dynamic reachability proof.

`local/cc12_particle_model_pool_borrow_readiness/evidence.zip` retains the whole
inputs. Run `python portable_replay.py evidence.zip` with Python and Capstone.
After dependency import, the guard permits only that ZIP to be opened and
rejects writes, subprocesses, networking, and dynamic loading. Replay recomputes
Git chains, compiler membership, Source queries/excerpts, complete object
indices, all-object counts, and selected full decodes without consulting Root
trees, the checkout, installation, or compiler. Both in-place and physically
relocated replay pass identically. `verify_git.py` separately checks actual Git
and current working bytes. This establishes bounded Source readiness and
retained compiled evidence; Original ABI and live model retirement remain
outside the claim.

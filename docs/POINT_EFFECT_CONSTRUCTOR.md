# Complete point-effect constructor orchestration

Packet `orch3_point_effect_constructor_z` joins the recovered stages into
`construct_point_effect_instance_008680b0`. It uses actual node allocation,
construction, transforms, ownership, row dispatch and live-manager insertion.
Canonical node associations and actual current row factories are required
application bindings; no placeholder owner or successful factory is supplied.

The evidence is the existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, and installed executable SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Full bytes, original ABI, validation limits and annotation history are in
`reports/point_effect_constructor.json`. Descriptive names are hypotheses,
not recovered symbols. Previous fragment records and comments remain preserved.

## Native ABI and body

The complete body is **807 bytes**, `008680B0..008683D6`, end-exclusive
`008683D7`. The final instruction begins at `008683D4` and is the complete
three-byte `C2 1C 00` (`RET 1C`). The stored Ghidra extent includes these bytes.
Native ECX is fresh aligned `114h` storage, and EAX returns that same address.
The seven stack DWORDs are:

| Slot | Meaning |
|---|---|
| 1 | Consumed incoming template reference |
| 2 | Parent, nullable |
| 3 | Additional root-registration condition word |
| 4 | Original matrix pointer |
| 5 | Transform selector, low byte |
| 6 | Option selector, low byte |
| 7 | Tail word |

The new C++ interface returns the stable canonical `NativePointEffectReference`;
its `storage()` is the native raw result. It receives the original template's
actual `+1C` name header by borrowed reference and explicit dependencies. That
header must not be a copied value or a projection of a subsequently replaced
effect `+84`. Its contents are consumed by node construction after allocation.
The original matrix reference also remains live through callbacks.

The implementation executes these established operations in native order:

1. Own the consumed argument without an entry retain. Initialize the actual
   point prefix, retain its separate `+84` template member, and increment
   `F87604` then `F87600` with DWORD wrap.
2. Allocate from actual `0108FF58` through `B6ED70`; construct the same physical
   node through `B6F5A0`, using the supplied actual name allocator. Establish its
   canonical plain-node companions without retaining or creating another node.
3. Publish `+110` and retain actual node `+04`. If the third word or parent is
   nonzero, propagate the current root **before** reloading/replacing parent
   `+8C` and retaining the supplied parent.
4. Use `53D9C0` for a zero transform byte or `72AA80` for any nonzero byte;
   cache current world state through the recovered x87-copy stage.
5. Bind the actual point companion before row callbacks or insertion can need
   its terminal path. Resize/read current rows and execute the established
   `8682D5` stage, including required current row virtual `+18` and returned
   temporary ownership. No default factory result exists.
6. Get the actual live manager through `4D1100`, insert the raw owner through
   complete `867500` under the separate actual `F87650` insertion lock, then
   disarm member unwind and release the original consumed argument last.

Normal initial ownership is point count 1, node count 2, and an extra retained
template member. Successful insertion leaves point count 2: caller and manager.
The consumed argument release leaves the template member's retained reference.
Manager removal can reach the recovered immediate/deferred release dispatcher,
point scalar, real node teardown and physical pool return.

## Canonical host associations

`PointEffectNodeCompanions` must register a stable `NativeNodeBinding` and real
final `NativePlainNodeReference` for the exact constructed storage in the existing
scene, node-lifetime and terminal-owner lookups. The constructor checks actual
storage/count/scene identity. The supplied destruction runtime, string storage
and physical pool must be the same dependencies used to construct that node.
This is host metadata, not an unreconstructed native node constructor callback.

The point constructor itself creates its canonical borrowed-count companion in
the existing `PointEffectReleaseRuntime`. Successful scalar disposal retires it
through the established host-only callback. After failed construction and native
member cleanup, `retire_failed_constructor_binding` removes only any remaining
point association. The actual header must be `CEB130`; no scalar, native free,
node release or count operation is performed. An earlier terminal callback may
already have removed that association.

Companion allocation can fail in this new C++ ABI; it is not a new native
allocation site. Partial host metadata must be cleaned without inventing native
destruction. The native owner, captured spans and callback dependencies must
remain alive through their native last access. Callback-induced premature frees
are not repaired with new native rollback or ownership policy.

## Exception evidence

Handler `C95022` selects `FuncInfo DC6E94`, with eight entries at `DC6EB8`:

| State | Next | Native cleanup |
|---|---|---|
| 7 | 5 | `C9501A -> 6CF070`: returned row temporary |
| 6 | 5 | `C95012 -> B6E670`: raw node allocation |
| 5 | 4 | `C95004 -> 605FD0`: current parent member |
| 4 | 3 | `C94FF6 -> 41DE40`: current template member |
| 3 | 2 | `C94FEB -> 8675B0`: auxiliary array |
| 2 | 1 | `C94FE0 -> 8675B0`: entry array |
| 1 | 0 | `C94FD8 -> BD30F0`: base identity |
| 0 | -1 | `C94FD0 -> 41DE40`: consumed incoming argument |

`B6F5A0` handles its own member failure before state 6 returns the raw slot.
The existing row stage uses the nonthrowing retained-assignment domain; a
throwing factory has not transferred a result. The outer state-5 guard releases
parent, template, auxiliary and entry arrays, then restores `CEB130`; argument
release follows all of those operations.

There is **no successful `+110` node cleanup**, counter rollback, physical point
free or insertion rollback in this map. A later factory failure leaves the
constructed node at its current retained count and preserves both increments.
The caller still owns raw point storage on a propagated exception. Native EH
dispatch itself is not emulated or tested by the host exception checks.

## Verification

Strict MSVC Win32 build and both existing CTests passed. One ignored focused
probe (`local/point_constructor_probe_z.cpp`) executed the full original 807-byte
body, including an explicit before/after ESP check for `RET 1C`, and the new C++
constructor. Both transform selectors, `0` and `255`, matched the entire `114h`
owner image after normalizing only template and node identity pointers. Input
preimages, real node/name construction, cached/relative matrices, actual counter
values, consumed template, final insertion counts, real lock depth, manager
removal, node teardown and immediate physical slot reuse were checked.

The original body's relative calls were relocated to existing reconstructed
dependencies, imports to real Interlocked operations, and data references to
actual fixture fields/constants. Matrix bridges temporarily project raw node
identity to its canonical companion and restore it before original code resumes.
No original integer vtable is executed as host code. Template terminal behavior
and the global reference transform are fixture inputs, not reconstructed game
template/camera owners. Normal comparison uses null parent, an empty row extent
and the option gate; successful game component factories are not exercised.

Two C++ failure cases check risks specific to composition: node-name allocation
failure returns the raw node slot before the consumed argument callback, and a
throwing required row factory releases the argument after base/member cleanup
while leaving the successful node and counters intact. Only after those retained
states are asserted does the fixture manually clean leaked native state. The
point companion is absent after failed construction without physical owner free.

No permanent tests or production test hooks were added. Whole-call nonnull-parent
and successful component-factory behavior rely on the separately reviewed stages
and remain outside this focused probe. Concrete application association/factory
routing, original EH, binary ABI replacement, concurrency and gameplay validation
remain outstanding; this is complete dependency-explicit constructor orchestration.

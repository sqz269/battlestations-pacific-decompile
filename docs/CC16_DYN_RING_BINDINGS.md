# Dyn ring family source bindings

`src/cc16_dyn_ring_bindings.cpp` closes the recovered family's typed
provider boundaries. The public entry in
`include/bsp/cc16_dyn_ring_bindings.hpp` accepts the actual native-layout
destination, actual input owner and required `DynLiveCrt`, then calls the
recovered parent. Names are hypotheses. This is source composition, with
no new original body or activation credit.

## Private candidate and dependency custody

The private worktree imported current `origin/main` at
`409689e5e813d4aaaeb656abeb827d06a5842016`, preparation dependency
`d03ef0004dd5a7dadd11f6825f342195b8b851bb`, and ambient-math dependency
`f460b42b4bfedbb6f13dc3efd1828eef06bd788e`. Those named dependencies remain
unaccepted until the primary integrator completes review and the common
build. Importing their commits into this candidate does not publish,
activate or promote them, or the unrelated pending constructor ancestry.
Only the binding overlay and predicate revision are owned by this packet.

## Closed source graph

| Boundary | Concrete composition |
|---|---|
| public entry | `construct_dyn_ring_owner_00c4d5a0` with the identical destination/input/CRT references |
| parent prepare contract | `prepare_chain_00c4d370` with that same owner address and CRT |
| parent next/previous tests | recovered `00C41190` and `00C41230` with the same owner |
| preparation query callback | captured EAX/EBX/four-stack query values copied field-for-field into the recovered query view |
| predicate query contract | recovered `triangle_ring_clear_00c410e0` with unchanged owner/query references |
| query barycentric contract | ambient `00C37860` variant with distinct private u/v/w outputs and eight original float words |
| both predicate orientations | ambient `00C377F0` variant with the unchanged six-float vertex ordering |

The parent continues to compose its initializer and existing
`0040D7B0` vector copy. The initializer, parent, preparation, vector copy
and unwind cleanup all use the caller's same CRT reference and heap pair.
The binding does not create an allocator, copy the owner record, manufacture
geometry, retain an invocation frame or supply a missing-result fallback.
All adapter objects are stateless and stack-scoped. The preparation
callback has no state, so its context field is intentionally unused.

Preparation's raw pointer is the actual typed owner's address passed by
the parent adapter. Only its already captured query input fields are
copied, in the order walk-start, triangle2, stop, triangle0, triangle1.
No cursor or edge is recomputed by the adapter. Recovered routines retain
their own pointer/count reloads around callbacks, allocations and frees.
The input point buffer and returned heap buffers retain their original
ownership/lifetime requirements; binding introduces no ownership transfer.

## Floating-point correspondence

The predicate change preserves all four ordered vertex triples recorded
in `CC16_DYN_RING_PREDICATES.md`; only their CCW target changes from the
qualified fixed-PC24 value helper to the exact ambient variant. The older
worker checkpoint and its precision limitation remain documented there.
The existing `cc13_dyn1.cpp` implementation is untouched.

For the barycentric call, the adapter passes pointers to separate local
u, v and w members, then `(x0,y0,x1,y1,x2,y2,px,py)` without arithmetic.
The native query also uses three distinct caller-frame output slots,
outside the barycentric helper's private argument/scratch area. Its
triangle1 index is captured before its private slot becomes u_out; the
source query already preserves that capture. The ambient helper retains
u/v/w store order and all original float-store epochs. Returned outputs
are observed only after that helper completes, as in the native query.

Preparation's orientation, the query's COMISS/FADD/FCOMIP gates, and the
new CCW/barycentric compositions all use actual ambient hardware controls.
No control word, MXCSR, precision or rounding mode is forced or inferred
from deadness. The original x87 stack-space preconditions remain: four
free entries for CCW and seven for barycentric. This is source/instruction
correspondence, not a fresh native fixture or generic binary-ABI claim.

## Native/public ABI and activation boundary

The native parent takes two stack arguments and uses `RET 8`; preparation
has an ESI receiver, predicates have ECX receivers, and the query/math
bodies have the documented register/stack inputs. The public entry and
typed adapters use ordinary C++ calls and do not install those native
entrypoints or redirect a game caller. Full uint32 predicate/query returns
are passed through their actual contracts.

Original caller reachability remains unresolved. Bounded negative parent
references do not prove complete deadness, and an exposed C++ entry does
not establish a game/root call path. The primary owns dependency acceptance,
registration, common build, central metadata/annotations, actual activation
and runtime/dead-chain evidence. No game, probe or fixture was run here.

Both changed translation units passed optimized MSVC x86 C++17 object
compilation with `/permissive- /EHsc /fp:strict /O2 /W4 /WX`. Generated
machine-code listings show raw MOVSS argument-word copies at the CCW and
barycentric adapters, with no FP conversion/arithmetic inserted. Barycentric
output pointers are the distinct return-storage members at offsets 0, 4
and 8, followed by the original eight float words. The live verifier checked
14 binding-edge rows and six predicate rows with zero failures. Common
build and post-integration checks remain with the primary. No new test
suite or executable is added.

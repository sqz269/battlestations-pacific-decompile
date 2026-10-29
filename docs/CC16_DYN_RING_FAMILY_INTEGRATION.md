# Dyn ring family: reviewed source integration

The recovered owner `00C4D5A0`, preparation `00C4D370`, walk query
`00C410E0` and predicates `00C41190`/`00C41230` are registered directly in
`bsp_core`. `cc16_dyn_ring_bindings.cpp` closes their typed source graph
through the previously accepted `00C33510` initializer, existing vector-copy
body and additional ambient-control math variants. The owner and every
adapter share the actual caller-supplied `DynLiveCrt`; there is no fabricated
heap, points record, default predicate or fallback query.

This is source/build integration with a focused normal-path differential.
Original caller/root reachability, genuine game-side ownership and native
ABI/exception transport remain unresolved. No runtime switch is enabled and
no game-validation credit is granted by this batch. A runtime acceptance
decision must identify a genuine reachable original path or establish the
appropriate original dead-chain boundary; an empty indexed caller list alone
does not establish that boundary.

## Reviewed behavior and build

The primary reviewed the complete owner/binding source and downstream
contracts, including cached versus reloaded pointers, equality-only vector
growth, unsigned wrap, append ordering and full-EAX predicate results.
Preparation retains the original x87 spill epochs; the walk retains COMISS
unordered handling and an unspilled x87 sum. The new math variants execute
under the caller's actual FP controls. The existing fixed-control `cc13_dyn1`
models remain available with their original evidence.

The combined MSVC Win32 Release build on accepted base
`51e6199dab817a68b022aa1ca9438ad3798bda82` plus candidate
`2f76638f6d2d0dfaa3caac8bb85ad7fa7b776cb3` passed. Both registered suites,
`reconstructed_math` and `tool_tests`, passed (12.81 seconds total). Receipt:
`local/cc16_dyn_ring_family_build.log`. Call-report checks passed for owner
(22), preparation (8), query (1), predicates (6) and bindings (14).

Independent inspection of the final CMake ambient-math COFF object confirmed:

| Original body | Bytes / instructions | Exact installed-image SHA256 |
|---|---|---|
| `00C377F0` | 104 / 35 | `a76903724f146a68fc4815a7b131b3cea18247e7aef33fb9a3b1b5f6e5d83450` |
| `00C37860` | 293 / 111 | `a1b1f99bfff1b431e96c393b5035896b0c51414ca59c4492c390835f701ac92c` |

Neither body has a relocation. The public wrappers contain respectively 15
and 16 instructions, one raw-body call each and no FP/SSE arithmetic. Full
object SHA256 is `597398dde08a0956e6348c2c9e5ae42339363acc48d9b1409c5b404bc8a37ab1`.
This proves the inspected object bytes, not a public binary ABI or game path.

## Focused original-machine comparison

Fixture commit `a185278d61a9d76053350e0fc40bdb0201aeffed` preserves the
single fixed concave-five input and exact original-machine reference at
frozen candidate `2f76638`. Under both masked round-nearest PC24 and PC53,
original machine code and bound C++ matched count/cursors, active vector
contents/capacities, logical allocation identities and the ordered eight
allocations plus one free. The native shim checked saved registers, stack
and return pointer; both routes preserved FS registration, x87 TOP and FP
controls. All eight seeds and 34 call sites were verified before execution.
The isolated, manifested probe exited 0 under its 15-second watchdog.

See `CC16_DYN_RING_NATIVE_FIXTURE.md` and its report for exact hashes and
receipts. The same synthetic normal-contract heap callbacks back both routes.
The reference rejects any exception; its handler relocation is a nonreturning
failure path. Original CRT identity, allocation failure, FH3/SEH cleanup,
other geometries/FP modes and public native ABI identity are not tested.
This fixture does not activate the functions.

## Newly defined radix constructor and metadata

The separate `00C37210..00C37236` constructor is compiled, but remains a
partial record pending genuine `00D7A118` vtable custody. Its exact 39-byte
body was defined in the existing BSP program using the locked definition
tool after byte/boundary checks; padding and the separate getter were not
absorbed. The definition receipt is
`reports/cc16_dyn_radix_constructor_definition.json`. The project was saved
and its function count increased by one. This entry is outside the initial
29-function referenced-body census; it is not an additional accepted active
body. No table is invented to complete it.

The five newly recovered ring bodies replace their metadata-only ledger
placeholders as `reconstructed_build_tested_unwired`. The two math variants
add no unique-address credit. Original names/comments were backed up before
locked annotation, the program saved, and all eight affected exports refreshed.
Backup: `local/ghidra-annotations-20261029T041856Z.json`.

The final promotion tree also merges accepted `origin/main` `6c158055f1b7f76a3b7cc47f2d91f5d84374ae9b`
with source integration `d27df3911` and fixture merge `a64b3a76c`. Its fresh
Win32 build and both existing suites passed (12.23 seconds). Receipt:
`local/cc16_dyn_ring_family_main_build.log`. No source edit followed this
combined build; this additional build does not extend gameplay evidence.

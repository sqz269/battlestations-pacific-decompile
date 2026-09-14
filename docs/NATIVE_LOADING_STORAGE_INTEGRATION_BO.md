# Native loading storage integration BO

Six complete ordinary source bodies, totaling 2,019 native bytes, are registered
in three Win32 translation units. Code revision `2a6b82646733f7cdc0d915d09ca9505754f03d63` builds successfully;
both existing math tests pass. The [integration report](../reports/native_loading_storage_integration_bo.json)
records exact source, build, fixture and Ghidra evidence. Full production loading
and gameplay remain incomplete.

| Source | Native bodies | Result |
|---|---|---|
| Resource-cache insertion | B7FF80, B803B0 | Actual nodes and red-black balancing; equivalent keys retain the prior borrowed resource; native owning length-error payload |
| Optional reader buffer | BF05D0, BF0700 | Actual 0Ch header and entries; current-field copies, forward returns, reverse shrink and exact publication order |
| PakRegistry callbacks | BB5770, BB5910 | Actual package resolution, mount/vector append, duplicate removal without release, one name unmount and current-manager counters |

The primary reviewed all six source bodies and all 698 instruction boundaries,
checked the workers' frozen source/dependency/executable hashes, then reran four
focused probe modes successfully. Those probes use explicitly linked new source
objects and pinned BM/BN libraries. Their evidence covers actual cache balancing,
reader string storage and package callback dispatch/cleanup ordering. They do
not execute original native bodies or establish a combined BO gameplay path.
The first registered BO core library hash is `02b33f5248e04e595830d66c353883d0b9d66706e37f653a9d8d8f9545c22444`; BP workers retain that pinned artifact.
The final combined source manifest contains 2586 build inputs.

The whole-report verifier passes 71 direct-call/tail-transfer rows, including
supporting callees and exception handlers. Two indirect reader dependency calls
are explicitly excluded from that verifier. BF05D0's five missing continuation
instructions now belong to its complete 106-instruction body. Four verified
10-byte exception-dispatch handlers were defined. Names and ABI analysis views
for six sources, four handlers and seven unwind actions were saved; correct
compiler/library names and prior comments were preserved and exports refreshed.
All six source flow reports have zero gaps.

The reader report's document hash predated its final call-verifier paragraph.
The primary verified the committed document against the worker copy, reviewed
it in full, corrected that receipt and preserved all three original worker
reports as hashed local preimages. Source and probe hashes required no correction.
Earlier missing-handler observations remain timestamped worker evidence; new
primary rows describe the completed definitions.

The callback source retains outer invocation storage on escaping calls; it does
not implement native FH3 or retain BDD850/BE1740 internal temporary storage.
Those precise dependency boundaries are documented in the
[callback contract](NATIVE_PAK_REGISTRY_BLOCK_CALLBACKS_BO.md). Successful nonnull
archive factory creation was not exercised by its fixture. Reader copy/return
failure effects come from listing/FH3 evidence, not an induced failure fixture.

No game mission was run for BO. Three BP packets advance actual cache erasure,
reader ownership and FileBlock entry/exit. Actual manager lifetime, root/parser
dispatch, resource destruction, production observer binding and the metric
publication contract still precede complete native resource-load composition.

The final publication also merges graphics-pool lifecycle work from main `042bf827c2ec623e4a62819f78c2ea9f5b5d22aa`. The primary reviewed its complete source/header/contract and verified 43 direct-transfer rows. The clean CMake merge preserves all registrations and the deferred placement include. The combined build and both tests pass. The other owner's native-body fixture and 120-frame mission results remain separate evidence; this primary did not rerun them.

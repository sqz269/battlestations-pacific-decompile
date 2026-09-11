# Orch5 native Text ownership integration

Addresses: `00AA83A0`, `00AA8320`, `00AA9F10`, `00AB73B0`, `00AB80C0`,
`00AB8250`, `00AB83D0`, `00AB8400`, `00AB8530`, `00AB9650`, `00ABA8D0`.

The separate `agent/orch5-20260911` worktree combines reviewed worker commits
`28e9cf55` (actual glyph buffers/sections), `c79cb75b` (content prefix and child
clear), and `229fb285` (canonical Text lifetime), plus the primary's actual
native clip binding. The earlier detach, ellipsis and style sources from
`f721a16d` now also enter the normal build through `cmake/startup.cmake`.

Integrated into `main` at `c2d7def9`. The combined build and both existing
tests passed after merging the other orchestrator's current main branch.
The integration recorded 17 applicable annotations and refreshed the two
newly changed exports; `00ABA8D0` remains deferred under the active worker lease.

The buffer path uses the existing native renderer, model, mesh, section,
material and reference-owner domains. The auxiliary Shadow model is published
before temporary-name release. The lifetime owns one Text state, shadow slot,
glyph vector and cached shader identity. It preserves the secondary release
between child and primary-node cleanup, and stops Text dispatch after derived
teardown when native base destruction has replaced the vtable.

The content prefix uses actual buffer initialization and child detachment.
Detached ownership is transported before reloading a glyph slot; actual Text
scalar deletion remains required. Nonempty geometry is explicitly a pending
continuation. Clip registration now writes the actual material parameter table,
borrowing the same widget/ClipBox/aspect fields with native name lifetimes and
the live ordered-zero test. These additions do not enable the Text factory.

The primary reviewed shared-owner edits, native call sites, lifetime phase
ordering and clip assembly. Raw `00AB83D0..00AB83F7` was defined under the Ghidra
write lock after matching all 40 bytes; it ends with the three-byte `RET4` at
`00AB83F5`. Nine affected existing/new names and evidence comments were applied,
with prior values recorded, the correct project saved and exports refreshed.
The content worker still owns `00ABA8D0` for its next continuation; annotation
of its evolving evidence must wait for that lease.

Validation: all seven newly registered sources compile in the normal MSVC
Win32 target with warnings as errors and strict FP. Both existing tests pass.
The existing focused actual-owner detach probe was rerun against the rebuilt
library and passes. Across seven reports, 214 numeric call rows have zero
failures: 208 direct calls and six resolved indirect rows. Twenty-three further
symbolic indirect rows remain explicit contracts; the checker does not prove
their dispatch. No persistent tests were added. New buffer/lifetime/clip paths
have not been executed in the game and are not native ABI replacements.

Follow-up packets are already assigned independently: actual logical stream
mapping and mapped glyph output; Text material/shader and nonempty continuations;
and canonical native font texture/descriptor ownership. Complete builders,
recursive Text deletion, factory and menu wiring, renderer/font lifetime and
gameplay validation remain before the broader reconstruction goal is complete.

# Canonical native renderer scalar process

`GameNativeRendererScalarProcess` owns one source instance of the four
loader-zero scalar domains needed by native renderer and texture work:

* the 8-byte `NativeRendererSynchronizationGlobals` record corresponding to
  `0108D6DC`;
* the shared logical-texture construction serial corresponding to `0108D6E8`;
* the live texture accounting counter corresponding to `0108DAF8`; and
* the live surface accounting counter corresponding to `0108DAFC`.

`game_native_renderer_scalar_process()` constructs the process object once and
returns the same identity for the life of the source process. Each member
accessor returns a stable borrowed reference. Value initialization supplies the
original zero preimage. There is no explicit initializer, reinitialization,
reset, native exit callback, or native cleanup route.

The synchronization record has compile-time checks for its original 8-byte
size and offsets `0`, `1`, `2`, and `4`. The three counters are independent
DWORDs. The class does not imply original address spacing between the four
source members.

## Evidence and ownership boundary

The complete R29 writer census is recorded in
`reports/native_renderer_owner_domains_r29.json`. All four domains fall in the
virtual tail of the original `.data` section and therefore begin as loader-zero
storage with no raw file bytes. No domain has a direct original CRT initializer
or exit callback. Their values change only through the previously reconstructed
renderer guard/worker, logical texture, texture owner, and surface owner paths.

The actual renderer publication at `00F8D394` is outside this class. The current
`main` baseline still has no production owner that constructs the full renderer
and supplies that publication to `GameNativeTextureOwnerServices`; this scalar
process does not close that producer. It also does not instantiate the texture
owner-service bundle, device state, a renderer, COM resources, or fake renderer
aliases. Future renderer work and every named or unnamed texture variant must
borrow these same scalar references rather than create parallel state.

Before adding this process, a bounded read-only `git grep` checked the unmerged
`agent/orch5-native-renderer-base` branch at `b521b0a5988081eaee24e9ab535e6055379bf5a7`
and `agent/orch5-native-renderer-constructor` at
`00a12b5d0fa86494f6e9efccaa90c543c9bd3bef`. Those branches contain raw renderer
base/constructor recovery and many borrowed uses of these addresses, but no
`GameNativeRenderer` process or backing definition for the four scalar cells.
This observation is branch-specific and does not dismiss their renderer-body
work; integration of that work remains a separate orchestrator handoff.

## Validation and limits

`python tools/ghidra_export.py verify-seeds` matched every configured seed.
The strict MSVC Win32 Release build compiled the new source with `/MD`, `/W4`,
`/WX`, and `/fp:strict`; all three existing CTests passed. No new test was added
because the implementation is a compile-time layout plus ordinary stable
process storage, and a zero-initialization fixture would only mirror it.

These checks establish source compilation, layout and process identity design.
They do not establish original ABI or exception-unwind equivalence, original
renderer construction/destruction, worker-thread behavior, live game startup,
rendering, or gameplay parity.

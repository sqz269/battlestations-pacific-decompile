# Raw material entry and batch dispatch (R57)

## Reconstructed behavior

`src/native_material_entry_dispatch.cpp` reconstructs the full native bodies of
`00B45360` (359 bytes, ECX effect, stack entry, RET4) and `00B55550` (70 bytes,
ECX batch, two unused stack words, RET8). Source APIs add explicit borrowed
contexts and persistent pass frames; binary ABI compatibility is not established.

The entry dispatcher preserves the mode-2 special pass, descriptor/fade cull,
COMISS exception flags and unordered branch behavior, and unchecked DWORD-wrapped
mode indexing. It captures the light collection before optional debug-sphere
production. The current model sphere provider returns without consuming the
captured camera/selector arguments; the genuine renderer sphere producer receives
all three arguments. This retains the caller-stack correction documented in R56.

A nonnull light collection repeatedly reads its current front and unsigned count.
Its predicate queries the actual directional type ID at `0109019C`, then reads the
current slot from the captured light profile. Existing Light, DirectionalLight,
and PointLight predicates operate on their shared bootstrap storage. Accepted
lights and selected passes reach the genuine material pass/constant implementation.
Each reached pass uses a fresh caller-prepared frame that survives failure.

The batch checks the current renderer's active-frame getter before clearing the
shared effect cache. Its signed count loop reloads the entry list and resolves
each entry's section, material, and current effect dispatcher.

The source binds established renderer/effect/light profiles and canonical model
services. Unknown targets raise a source contract error. It creates no second
owner graph and does not activate application rendering.

## Evidence and validation

The report records fresh live-Ghidra/installed-PE byte comparisons, function
boundaries, profile slots, call-site rows, original PE hash, linked code, and
same-process handle-resolved I386 module hashes. The unreachable alignment at
`00B4540C` remains unchanged. Runtime type globals have no raw bytes in the PE
region queried; their IDs come from the actual existing bootstrap storage.

- Strict MSVC Win32 `/MD /W4 /WX /fp:strict` build and all three CTests passed.
- Ten copied-full-native/source entry cases compare complete fixture buffers and
  MXCSR, including NaNs, a denormal, fade culling, null special/regular passes,
  and wrapped mode indexing.
- Four batch cases compare the active gate, cache reset, signed count behavior,
  and a two-entry loop. The native batch uses the genuine source active getter
  and the copied full native entry dispatcher.
- Linked machine inspection confirms COMISS/SETA and genuine direct calls into
  pass, model-sphere, sphere-record, and batch/entry implementations.

The ignored probe adapts global addresses and profile representations. Its raw
storage fixtures are not constructed application owners. Native entry external
rel32 calls remain unrelocated because the tested routes do not reach them.

Selected passes, nonnull special passes, sphere production, light-list traversal
and mutation, predicate calls, and returning invalid-parameter handling remain
unexecuted in this fixture. No original FH3/SEH, register/private-frame/fault ABI,
active application, visual, or gameplay parity is claimed. The raw command
executor and queue remain the next integration dependencies.

Details and immutable local artifact receipts:
`reports/native_material_entry_dispatch_r57.json`.

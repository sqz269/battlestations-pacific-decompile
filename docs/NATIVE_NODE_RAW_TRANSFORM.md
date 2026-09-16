# Native node raw transform notification

## Scope

This packet reconstructs three small raw-node operations:

- `invalidate_raw_descendants_00b6da30`, 50 bytes at
  `00B6DA30..00B6DA62`.
- `set_raw_local_matrix_00b6db10`, 73 bytes at
  `00B6DB10..00B6DB59`.
- `notify_raw_bounds_00b6dbc0`, 25 bytes at
  `00B6DBC0..00B6DBD9`.

The source borrows raw node backing and the existing
`NativeTracelineRenderServices::profile()` resolver. It does not project a node
type, own any node, copy a vtable, or add another scene/profile domain.

## Local matrix and notification order

`set_raw_local_matrix_00b6db10` first calls the genuine
`copy_native_camera_matrix_004134f0` into raw `actual+B0`. It then reloads byte
`+5C` and stops unless bits `0A` are present.

On the active path it captures current `actual+A0`, clears bits `30` in
`actual+138`, writes zero to the full DWORD at `actual+5C`, and only then uses
the captured attachment. A nonnull attachment resolves its current raw profile
and accepts only slot `+3C == 00B6DBC0`. An unsupported current target throws
after the matrix and prior flag writes have already happened; source does not
invent rollback or a fallback dispatch.

After the attachment notification returns, the source reloads `actual+34`.
Only a current nonnull child list enters descendant invalidation.

## Descendant walk

`invalidate_raw_descendants_00b6da30` begins from current `actual+34` and walks
siblings through current `child+3C`. Each sibling reloads its full `+5C` flags.
Nodes without bit `02` are skipped, including their entire child subtree.

For a valid node, source clears bits `30` at `+138`, captures whether current
`+34` is nonnull, writes `flags & FFFFFFF5` to `+5C`, and recurses only when
that captured child test succeeded. The sibling link is reloaded after any
recursive call.

## Bounds tail loop

`notify_raw_bounds_00b6dbc0` clears bits `3C` at current `+138`, reloads
current `+A0`, and returns when it is null. Native code otherwise loads the
current profile slot `+3C` and tail-jumps to it.

The source preserves that tail behavior as an iterative loop for the proven
finite closure. Every reached owner is resolved again through
`NativeTracelineRenderServices::profile()`, and every current slot must still
be `00B6DBC0`. It does not use host recursion or call numeric image addresses.

## Evidence boundary

Fresh live memory, PE bytes, and the retained original raw inputs agree for all
three bodies and the 104-byte matrix-copy provider. The scoped fixture executes
relocated original copies of the same three leaves and matrix copy, then compares
all raw matrix bytes and node flag writes against the source. It also verifies
the invalid-child subtree skip, two bounds-tail dispatches, and retained prior
writes at an unsupported current `+3C` target.

The fixture does not execute renderer parent `00B2BB90`, the debug-sphere x87
interior, or gameplay. These C++ functions add explicit service parameters and
do not reproduce the original register ABI or exception machinery.

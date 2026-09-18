# Native game grid application services — R156

The native game constructor needs three grid owners. Their recovered bodies
already existed, but the application renderer exposed no compatible context.
`GameNativeRendererApplication::game_grid_context()` now supplies one using
the application's existing declaration cache, mesh registry, buffer owners,
mapping/locking services, device recreation path and VFS string storage.
This is source composition, not a newly reconstructed native function.

## Ownership and evidence

- `GameGridGraph` owns context metadata and the existing CPU call service.
  It allocates no native grid or replacement renderer. Callers retain actual
  84h grid storage, persistent operation records and explicit descriptor
  preimages, and release grids before renderer teardown.
- The declaration decoder borrows an application context using the VFS string
  wrapper. Its pool, token tables, counts, initialization cell and process
  shutdown callbacks remain canonical. This fixes the wrapper-identity mismatch
  found by the initial application run, without creating another decoder domain.
- F87574/F87578/F8757C use contiguous mutable process storage. The supported PE
  puts their twelve zero bytes beyond the raw part of writable `.data`.
  Ghidra reports 308/189/215 references respectively and no direct WRITE edges;
  95 address references to the first cell prevent an immutability claim. The
  implementation keeps borrowed live references and provides no reset operation.
  Other existing consumers are not migrated by this packet.
- 2,945 live/PE bytes in seventeen spans were rechecked for the seven existing
  grid bodies and their constants. The source interfaces remain explicit C++
  contexts; they are not original binary ABI replacements. Existing names and
  comments are retained and extended at 70BD70, 70B330, 70B220 and 70B280.
- The application's explicit logical-index pool stack preimage is zero; the
  grid's actual flags=1 path never consumes it. The constructor's two unwritten
  descriptor words remain explicit caller inputs.

## Validation

- Strict MSVC Win32 build and all three existing CTests passed.
- The unchanged R120 fixture passed 108 original/source pairs, with 2,933,712
  matching observed bytes, actual D3D9 factories/mapping, 481 companion
  retirements, and both existing retained-failure/replay cases.
- A local diagnostic built from the production renderer translation unit
  inserted only a bounded exercise after device creation. It used ordinary
  application startup to construct three grids, checked canonical context
  identities, changed each live vector cell before construction, checked the
  resulting grid fields, released the grids and verified restored stream counts.
  Explicit descriptor preimages were supplied; their later fields are correctly
  overwritten by the native step calculations, so they are not final-state
  assertions in this diagnostic.
- The diagnostic and unmodified application both completed one loop tick with
  window/device creation, one skipped Present, zero final COM reference counts,
  a joined renderer worker and exit 0. A skipped Present is not rendering proof.

`reports/native_game_grid_services_r156.json` records hashes, original evidence,
commands, annotation receipts and integration validation. Detailed local inputs
and outputs are sealed under `local/evidence-r156/`.

## Remaining limits

The ordinary application still reports 45 unimplemented hosts and does not
construct the native 71A0 game owner. Profile/settings and other game context
bindings, complete application admission, native FH3/failure ABI and gameplay
validation remain open. The diagnostic is not a gameplay or visual result.

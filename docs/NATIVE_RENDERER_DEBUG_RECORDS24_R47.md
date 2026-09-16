# Raw debug sphere record execution

## Scope and provenance

`B2BB90..B2C274` is the 1,765-byte renderer method for the six-DWORD records
at `renderer+1D0C`. The original input is ECX renderer and the method returns
with plain RET. The source adds a borrowed context in EDX and a persistent
frame on the stack. It is not a binary-compatible replacement.

This adoption uses the corrected parent from `a6b2884f2` and the contracts in
`native_renderer_debug_records24_provider_contract_r41.json`. Existing raw
node-transform and sphere providers supply the previously embedded leaves.
The sphere interior is not credited as a separate original function.

## Current providers and ownership

Both generated-model calls explicitly receive `NativeRendererRawModelBinding`.
It borrows the existing AA8/AA4/AA0 string cells and the model's bound constants;
the model environment retains null `actual_names`. The older overload requires
a semantic name provider and cannot compose this raw environment.

The incoming temporary name header is needed during construction. The raw
node constructor resizes its own name header and copies the current source
bytes; it does not retain the caller's header address. Its name-pool context
remains valid through destruction. The generated-model header now states
these distinct lifetimes accurately; its implementation is unchanged.

The caller supplies a separate persistent record frame for every reached
active record, including records appended by callbacks. Each row contains
its own generated acquisition, gather frame, constant-builder frame and pass
frame bound to those constants. The gather receives 1,232 initialized bytes
of caller-provided preimage. There is no automatic allocation, reset or
default clearing inside the native loop.

Pass execution and constant building use the same VS/PS banks and the current
DWORD immediately before each bank. The parent adds no copied banks or owners.

## Recovered schedule

- A zero initial count returns without reading any provider context.
- A missing cached model is constructed and published at `+19E4` before
  normal temporary-string cleanup. The model pointer is reloaded afterward
  for its current local-matrix dispatch.
- Each record captures center/radius and camera, then skips nonzero camera
  modes. An active record gets a new generated model. The selector is reloaded
  after model and string callbacks.
- Current stream and section lookups precede lock, 38-vertex writes, unlock,
  section counts, gather and entry construction. The current first material
  pass executes once per active record.
- Only reached normal `B6DFA0` consumes the transient model creator. Failure
  retains earlier model, lock, entry, counter and child-frame effects; no
  enclosing cleanup, retry or publication repair is invented.
- The loop reloads the live signed count. Afterward, negative capacity reaches
  the existing reserve provider, and the current positive count is drained.

## Validation boundary

The full parent bytes were freshly captured from the verified BSP Ghidra
program and matched independently to the installed PE. Forty direct calls
are checked mechanically; five indirect/import sites have explicit provider
contracts. Source and listing review covers the parent schedule. Existing
sphere and raw-node evidence belongs to those separate providers.

The strict Win32 `/MD /W4 /WX /fp:strict` build and all three existing CTests
passed. Eight native seeds and all forty direct call rows were verified.
The three restored original/source parent cases passed. Exact compiler,
source, object, library, fixture and loaded-module evidence is frozen; the
manifest and archive hashes are recorded in
`reports/native_renderer_debug_records24_main_r47.json`.
The restored parent fixture exercises only empty, skipped-camera and negative
count paths with an existing cache, comparing the complete renderer storage
against a private copy of the original parent bytes. It makes no claim about
the active or cold parent. Native private-stack aliases, FH3/SEH, hardware
faults, renderer appearance and gameplay remain unvalidated.

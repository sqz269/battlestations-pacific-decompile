# First raw caller of bound-definition zero dispatch

No existing raw Source caller is ready to invoke the new bound-zero dispatcher.
The nearest concrete seam is `release_native_ref_counted_handle_0041de40` in
`native_damageable_section.cpp`: capture the current handle, decrement its actual
count once, call its current slot0 only at zero, then clear the original slot.
Its caller does not supply a `GameplayDefinitionReferences` domain. This is an
ownership dependency, not a missing decrement or scalar implementation.

The frozen 1,727-input closure contains 32 domain/lookup/dispatcher matches,
all in `gameplay_point_binding.hpp/.cpp`. The dispatcher occurs only in its
declaration and definition. `GameplayPointRows` and `GameplayPointConstruction`
borrow a domain supplied by their caller; they do not create one, and they do
not receive the raw section handle. The domain itself borrows its context/table
and requires all companions retired before destruction. None of these facts
identifies an application owner that survives the raw section/vector lifetimes.
SceneContents still returns null from its unimplemented effect acquisition and
has a no-op effect release. Existing host-reference releases are a different,
already implemented path, not evidence of this raw caller composition.

The pending source packet is `6c78ecb38de73238b36f7a077208713454b49363`.
Root review was pending when this packet froze. The accepted producer/terminal
readiness remains a prerequisite; this packet does not promote producer,
category, application, or shutdown coverage.

The generic 0041DE40 body admits any supplied owner/current table. Null returns
without a slot write. A non-null owner gets exactly one atomic decrement at+4;
nonzero skips deletion. Zero reloads owner+0 and its slot0 and calls with the
captured owner in ECX and no stack flags. Only normal return clears the original
slot. A callback may replace the slot, but the original slot is still cleared;
the replacement is neither retained nor released. A throwing callback leaves
the slot uncleared and propagates from this generic Source function. No retry,
rollback, profile whitelist or companion retirement is present.

The retained actual compiler object confirms the complete helper is 40 bytes;
its row destructor is 47 bytes and inlines the same sequence after stamping the
row table. Neither section has a relocation to the new dispatcher. The saved
40-byte body captures at+7, performs lock xadd at+13h, loads current table/slot
at+19h/+1Bh, calls at+1Dh and clears at+1Fh. This is Source compiler evidence,
not a new Native instruction export or an original entry-ABI claim.

The same generic helper also releases fake-effect row+0C and soldier base+10/+14.
Those owners are not established as gameplay definitions. Replacing the generic
helper, inventing a global domain, or forwarding every zero to the new dispatcher
would alter unrelated terminals and their exception behavior.

Other section paths are not covered by adapting this helper alone. Assignment
publishes and retains its incoming owner before releasing the captured old one.
Normal insertion cleanup releases its captured temporary without rewriting the
row table or clearing+24. Range destruction calls each row's current deleting
slot with flags0; this is distinct from definition scalar flags1. Copy/fill
unwind retain completed-prefix cleanup; a fill-cleanup exception escapes its
catch. Insertion temporary cleanup instead terminates on a second exception.
Row-begin normal cleanup captures before lowering14-to-13;
its unwind destructor is noexcept. Its access bundle contains vector/table/CRT
dependencies, but no definition domain. These paths remain held.

A future explicitly selected bound-handle route must establish all of the
following before it becomes an implementation packet:

- Name the caller-owned raw-context domain; keep it and its borrowed current
  table, manager publication cells and cleanup services live through all handle
  releases and companion retirement.
  The current public lookup alone does not identify typed versus raw context.
- Prove the captured non-null handle is the exact existing canonical definition,
  bound while positive before publication. Cover callback-installed replacements
  and identify its one owning positive reference. Do not first-bind at zero.
- Keep the handle slot live and writable through cleanup, disjoint from the
  freed owner. Preserve capture/null/decrement/clear order and use exactly the
  captured owner; never reselect a callback replacement for deletion.
- Invoke `dispatch_bound_definition_zero` only on the genuine once-only 1-to-0
  transition. Use its real current0-to-BD30E0-to-fresh-current4 path; add no retain,
  count reset, second decrement, direct-scalar shortcut or cached table word.
- Restrict the selected route to nonthrowing cleanup and exclude concurrent or
  reentrant same-owner terminals and retries. Keep current ID/current weak-tree
  erase and all string/component/free providers valid. Missing ID and partial
  destruction are not recoverable success cases.
- Specify rejection of unknown/unbound owners and other profiles for this new
  selected API. Leave existing generic behavior unchanged. Do not silently fall
  back after a decrement or failed scalar, ignore a missing binding, or retire
  metadata after failure. Selection of a raw domain before mutation needs an
  explicit contract or domain-owned entry; it cannot be inferred from lookup.

The concrete seam files are `native_damageable_section.hpp/.cpp`, with the
existing `gameplay_point_binding.hpp/.cpp` dispatcher as a dependency. No edit
set is ready: the missing caller/domain owner and raw selection contract must
be named first. A separately authorized standalone domain-owned handle API could
encode that contract, but would still not route an existing raw section caller.
Row/vector/row-begin overloads and actual application construction/shutdown each
require separate ownership scopes. The unrelated generic owner families
must not be folded into a definition-only API.

Evidence is retained as complete frozen source inputs, six complete saved COFF
provider objects, 22 whole-file-backed source excerpts, six complete queries
with 201 matches, and the unchanged prior compiler/provider receipts. No Source
file, compiler output, test, Native state or Ghidra state was changed.

Run `python local/gameplay_definition_bound_zero_caller_verify.py` in the worker
worktree. It verifies the new freeze and executes every inherited bound-zero
replay assertion, suppressing only the inherited verifier's two final output
expressions so its original helper and replay stay byte-identical. It performs
no compiler, test, new instruction decode, Native or Ghidra operation. Exact
paths, hashes, contracts and limitations are in the
[report](../reports/cc12_gameplay_definition_bound_zero_caller_readiness.json).

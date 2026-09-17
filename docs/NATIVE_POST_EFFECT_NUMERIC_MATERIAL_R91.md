# Post-effect numeric material cache composition R91

Addresses: `00B4E470`, `00B4E840`. Read-only dependency: `00535320`.

## Problem and resulting behavior

Both reconstructed post-effect constructors admitted the original numeric
`D5F0A8` renderer profile, then invoked the legacy material factory that treats
renderer vtable entries as relocated callable pointers. Their material call
sites now use the existing concrete `00535320 -> B318B0` effect-cache path.
The 20h constructor calls at `B4E5D8`; the 24h constructor calls at `B4E99C`.
The native returned material is still published at owner+14 only after the
factory returns, at `B4E5E1` and `B4E9A5` respectively.

Each construction context now requires `NativeMaterialEffectCacheContext&`.
Admission verifies the same canonical effect/material owners, actual string
storage, renderer publication **cell**, numeric renderer table and cache
backlink. It does not capture a replacement renderer value: the factory still
reads the current renderer at its original acquisition point. The cache retains
its existing VFS, synchronization, profile and loading checks.

## Retained factory state and cleanup limits

Each immovable construction block prepares its own optional
`NativeMaterialFactoryAcquired` before native execution. The diagnostic
`acquired().material_factory` points to that stable frame, which owns the
per-call cache/loader state. Preparation cancellation destroys only an unused
frame. Native constructor failure settles the block without discarding it.

This exposes the completed material identity if `00535320` fails during its
final effect release. It does not publish that identity into owner+14 on a
failed call or introduce a protective release. The factory preserves its
existing raw-slot-only constructor unwind, reference counts and release order.
Existing `material_created` continues to mean the factory returned; the nested
factory record also describes acquisitions before return.

`reset_after_host_quiescence` accepts only unused or completed factory frames.
A completed material must have its companion retirement recorded, including
when host registration failed after factory return. Failed factory/cache frames
must remain alive: this change supplies no native failure recovery API. Existing
owner/reference/viewport quiescence guards remain in force. No failed frame is
made reusable or silently reclaimed.

These are host context and diagnostic changes. Original 20h/24h storage, count
writes, native scheduling, logical twelve-state cleanup and raw allocation
rules are unchanged. C++ interfaces are not the original ECX/RET0Ch ABI or FH3.

## Validation

* Strict MSVC Win32 Release build passed; all three existing CTests passed.
  No new test cases were added.
* Fresh live-Ghidra body ranges/listings and bytes were checked against the
  installed original PE: B4E470 974 bytes, B4E840 948 bytes, 535320 124 bytes.
  The captured listings cover every byte; there are no unresolved gaps.
* Every direct call row from those bodies is mechanically checked with
  `tools/verify_report_calls.py`; indirect calls are recorded separately.
* Actual Win32 COFF relocations show one direct numeric-cache factory call
  inside each post-effect constructor and no legacy callable-factory symbol
  dependency in either object. This proves compiled composition, not execution.
* Current names are retained. Appended evidence preserves prior Ghidra comments,
  is applied under the owning worktree's write lock, saved, read back and exported.
  Exact counts, hashes and sealed build artifacts are in the R91 report.

## Remaining application frontier

This packet does not instantiate the complete post-effect/material compiler
graph in the game executable. Current application source has no constructed
`NativeMaterialEffectCacheContext` or loading context. The two post-effect
constructors have no new whole-body runtime fixture here. The previous R90
cached compiler/sampler runs are separate evidence and are not full post-effect
execution proof. Constructor failure injection, whole original/source parity,
native FH3/SEH, GPU draw parity, full teardown and gameplay remain unproved.

The next work remains the actual material-cache/compiler context and its
model/camera/layout/draw dependencies, followed by full renderer resource
initialization `B107F0`. The present correction closes a concrete source binding
gap and preserves its failure evidence; it does not close that application graph.

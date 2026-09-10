# Retained effect cache and stream lifetime integration

Addresses: 00b318b0, 00b31090, 00b2e940, 00b31750, 00a82250, 00bef750, 008d43c0, 00bf9dc8, 00509190, 007188a0.

The installed font diagnostic now obtains its actual compiled shader pair and
metadata through the recovered effect cache. A miss loads the resolved shader
descriptor through the mounted VFS, compiles the existing supported font program,
and transfers the COM shader owner into the cache. Subsequent descriptor aliases
reuse that owner. The actual material effect size virtual returns zero; cache
accounting uses that verified value.

The cache implements the renderer's fixed `0,1,1` argument combination, including
request normalization, all-alias lookup, first-alias canonical reuse, reference
adoption, and reverse release. The callback owns shader loading. Full native
renderer guards, metadata, recursive `error.shfx` loading, arbitrary flag
combinations, scene integration and native object layouts remain outside this
projection. A failed host loader is explicitly rejected instead of inserting the
native unsafe null entry. No recursion sentinel or synthetic successful effect
was introduced.

Physical-to-memory conversion now accepts an empty file. It still seeks to zero
and performs one read; its logical length and initialized extent are zero, while
the backing retains the native one-byte minimum allocation. A cloned wrapper
keeps that backing alive after the physical source and first wrapper are closed.
The existing positive-size restrictions on bytes-copy and MPKG conversion are
unchanged.

## Validation

`scripts/build.ps1` passed with both existing CTests. The installed-resource
`bsp_d3d9_probe` exited zero. Each of its two font cases compiled once, reused
three aliases for `shaderfx/gui/guifontbilinear.shfx`, and retained the caller's
shader owner after cache clearing. Actual material parameter binding and drawing
used that owner and its reflected metadata. The single-line/wrapped draws had
74/900 lit pixels, zero pixels outside their expected bounds, and restored device
state. This verifies the supported diagnostic font path, not the full renderer
or gameplay. The empty-file scenario passed in the existing probe; no test target
or general test framework was added.

Build, probe, source hashes, and Ghidra readback are recorded in
`reports/lifetime_effect_cache_validation.json` and its linked reports.

## Saved analysis repair

The initial inspection found incorrect per-call `CALL_RETURN` overrides even
while `_free` was reported as returning. Subsequent analysis reinstated a
no-return flag on the actual `_free` at `00bf9dc8` and its thunks. Its complete
assembly includes normal returns. The final repair corrects that library flag,
preserves the `_free` name, and clears thirteen audited call-site overrides in eight
functions. Four previously truncated function definitions were rebuilt from
their already verified instructions, retaining their original names and unknown
prototypes. Their complete sizes are 66, 186, 120 and 95 bytes. Complete
decompilation now includes the inflater's two buffer cleanups and base teardown.

The repair preserves prior annotations and does not modify executable bytes.
`bsp.py ghidra flow` inspects overrides through the server's dry-run query; the
query parameter placement is required by the installed bridge. Forced exports
now request fresh decompilation from Ghidra as well as replacing local exports.
The one-screen `state` command also shows the next packets and their owned
function addresses, alongside the completed packet states.

## Next runtime dependencies

The loader audits establish provider-pump order, current-front callback routing,
state transitions, native stop/wait behavior, worker submission, and the actual
cached resource loader. Incoming EAX at the worker's `007188a0` call is not a
parameter: `007175d0` overwrites it before use. FileBlock setup manages identifier
and tracing state; it does not supply stream ownership. The structured resource
reader, item ownership, queued work removal, synchronization and shutdown still
need concrete implementations before this becomes a running game loader.

See `VFS_LOADING_PUMP_LIFETIME.md`, `VFS_LOAD_PROCESSING_START.md`,
`VFS_LOAD_WORKER_DISPATCH.md`, `FILE_BLOCK_SETUP.md` and the structured-resource
audit documents for the exact contracts and unresolved dependencies.

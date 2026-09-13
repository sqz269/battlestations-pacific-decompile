# Verified original read-only data for source consumers

`GameNativeReadOnlyData` supplies the original numeric table and literal bytes
required by raw VFS and Lua consumers. It accepts a path to the supported original
executable and checks its complete SHA-256 and 12,223,752-byte size before
committing or copying data. The supported digest is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The verified x86 PE has preferred base `00400000`; its `.rdata` is RVA/file offset
`008E2000`, virtual size `00125B24`, raw size `00126000`, characteristics
`40000040`. Callers supply nonempty `GameNativeDataSpan` requests within
`00CE2000..00E07B23`. The service maps the union of their 64 KB address bands,
clipped to the section bounds. The complete original virtual section SHA-256 is
`1965d6b2c0d4cc3675d78c6fa937b0a202d28ea0456b8bef63c4d43b39bcbcc9`.
Committed pages are read-only and non-executable. No original code executes or
imports resolve; numeric code addresses remain data consumed by explicit source
dispatchers. Mutable globals, owners and callable methods need their own services.

The service first reserves each requested band with no access. Reserving before its
large file buffer prevents that allocation from occupying the required addresses.
An occupied range is rejected without changing or releasing its existing owner.
All post-reservation failures release only this service's allocations, including
earlier bands when a later reservation fails. Unrequested bands are untouched.
The mapping must remain alive through every borrowing consumer and singleton drain.
Startup should retain it before large allocations; the rebuilt PE's preferred
base does not guarantee that required bands will be available in every process.
The original full-section approach encountered a pre-existing private allocation
at `00E00000` in the larger fixture. The explicit request contract avoids requiring
unrelated addresses; a collision in a required band still fails.

An ignored manifested Win32 fixture compares the three requested bands at
`00D10000`, `00D50000`, and `00D60000` (196,608 bytes) with the disk source and
checks all 48 committed pages. It rejects a modified same-size image, preserves
sentinel allocations in a required and an unrelated band, checks partial-failure
rollback, bounds, empty-span rejection and complete release. It then substitutes
this service for the retained fixture's manually
mapped table pages. Actual physical HANDLE loading, FileStore memory chunks,
nested `DoFile` with duplicate suffix execution `MCPP`, retained physical and
adopted stream reads, Lua close and zero-counter shutdown pass.

This is source service validation. The fixture retains seeded VFS records,
explicit writable type-descriptor pages and its existing semantic lifetime domain.
It does not construct the production raw VFS manager, initialize archive providers
or establish game startup/gameplay behavior. Physical stream, render-batch and
type-counter lifetime contexts still require review for a single raw manager.
See `reports/game_native_readonly_data_az.json` for component evidence and the
combined publication report for final source/artifact hashes.

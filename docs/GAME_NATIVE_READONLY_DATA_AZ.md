# Verified original read-only data for source consumers

`GameNativeReadOnlyData` supplies the original numeric table and literal bytes
required by raw VFS and Lua consumers. It accepts a path to the supported original
executable and checks its complete SHA-256 and 12,223,752-byte size before
committing or copying data. The supported digest is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The verified x86 PE has preferred base `00400000`; its `.rdata` is RVA/file offset
`008E2000`, virtual size `00125B24`, raw size `00126000`, characteristics
`40000040`. The service copies the virtual section bytes to `00CE2000..00E07B23`.
Their SHA-256 is
`1965d6b2c0d4cc3675d78c6fa937b0a202d28ea0456b8bef63c4d43b39bcbcc9`.
Committed pages are read-only and non-executable. No original code executes or
imports resolve; numeric code addresses remain data consumed by explicit source
dispatchers. Mutable globals, owners and callable methods need their own services.

The service first reserves the fixed range with no access. Reserving before its
large file buffer prevents that allocation from occupying the required addresses.
An occupied range is rejected without changing or releasing its existing owner.
All post-reservation failures release only this service's allocation. The mapped
section must remain alive through every borrowing consumer and singleton drain.
Startup should retain it before large allocations; the rebuilt PE's preferred
`00400000` base and current `001F6000` image size do not by themselves guarantee
that these addresses will be available in an arbitrary process layout.

An ignored manifested Win32 fixture compares the complete mapped section with the
disk source, checks every committed page, rejects a modified same-size image,
preserves a pre-existing sentinel allocation, tests bounds, and verifies complete
release. It then substitutes this service for the retained fixture's manually
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

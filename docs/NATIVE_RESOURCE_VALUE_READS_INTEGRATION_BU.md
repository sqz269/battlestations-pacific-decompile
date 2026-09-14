# Resource value reader integration BU

Addresses: BE4360, BF02C0, BE99D0, B932E0, B93310, B936E0, BE9A00,
BE9FE0 and BEA010.

This batch adds nine complete ordinary bodies (318 bytes) over actual raw
stream/reader/node storage. It closes the scalar and aggregate value readers
needed by the native hierarchy parser. Assembly bridges preserve the x87
load/store boundaries, while existing actual stream and string-pool services
provide transfer and lifetime behavior. No new ownership domain is introduced.

Strict Win32 compilation and one focused actual-service/original-body probe
passed: 612 paired float cases, 60 pointer-seed-aware short reads, an alias case,
one source exception and DWORD/string adapters. The source is a new ABI; native
FH3/SEH, unmasked faults and gameplay are not established. Full details and
evidence limits are in `NATIVE_RESOURCE_VALUE_READS_BU.md`.

The integration report retains the final code revision, exact source-input
manifest, final-library probe and combined build evidence. Production raw
resource dispatch/loading and queue shutdown remain incomplete.

# Native shader and named texture owners

Addresses: 00B38310, 00B38390, 00B38490, 00B5BB40, 00B3A660, 00B3B1E0, 00B3B260, 00B3CED0, 00B3CFA0, 00B34280, 00B342D0

The actual D61810 reflection owner and its 20h-record arrays preserve the same
raw+04 reference count and untouched producer bytes. A retained failed array
operation cannot be silently discarded; explicit diagnostic retirement remains
separate from native completion. The named cube/volume constructors preserve
actual COM ownership, shared serial, native pool metadata and cleanup order.

Both accepted modules are default registered and passed the exact combined
Win32 build at `b18f5477069e6db5da7e11a65ee527aefed3d7f0`, both CTests and their
focused original/source fixtures. Ghidra annotations and post-free owner body
repairs were saved and exports refreshed. `reports/native_owner_extensions.json`
pins the reviewed sources and worker archives; the complete validation record
is `reports/native_loader_integration.json`.

The compiler/reflection continuation, actual texture-cache integration and game
validation remain separate work. No original native FH3 execution is claimed.

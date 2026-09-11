# Storage backend integration
Addresses: 006A51C0, 006ADB50, 004374F0, 00BD48F0, 00BD49B0, 00BD4A70, 00BEB9B0

This batch connects the native archive text writer to actual compressed block
storage, reconstructs the PC save operation backend and profile-manager refresh,
and supplies keyboard settings serialization. The primary integrator works in
`agent/orch2-20260910`; independent worker branches own disjoint functions.
The renderer orchestrator's changes are merged through the shared integration
helper before combined validation.

`GameArchiveSettingsServices` supplies both required settings effects: actual
options.txt persistence through `SettingsTextHost`, and the recovered keyboard
writer over `InputSettings`. Keyboard slot keys remain numeric0 and1. The loader
initializes only runtime fields established by native evidence; the separate
keyboard archive reader and final runtime application remain future work.

`ArchiveCompressedSink` supplies the text writer's buffered route. The storage
backend controls the64KB scratch lifetime and two-stage write progression.
The compressor's output matches existing native player and quick containers
byte for byte; see `ARCHIVE_COMPRESSION_BUFFER.md` for that test's scope.

The PC backend supplies filesystem operations, archive decoding, Lua buffer
execution and operation-state progress. Its Lua-owner service must still create
the actual game environment, including fundamentals and platform/region globals.
An empty substitute interpreter does not establish that startup environment.
`PcProfileManagerRefreshHost` connects the refresh host directly to this PC
backend and the application game-Lua publication slot. It preserves the integer
file kind2 for game records. Profile refresh copies real Lua values and upserts
transient profile records;
the actual game Lua state and storage owner must be supplied by the application.

Validation, worker commits, saved Ghidra annotations and outstanding boundaries
are recorded in `reports/storage_backend_integration.json`. Host compositions
are not native ABI replacements. Game rebuild and gameplay validation remain
incomplete.

Combined code `3c40f08` passed the Win32 build and both existing CTests. The
three worker fixtures were recompiled against that combined library and passed:
keyboard serialization through Lua, profile-manager import through the storage
driver, and actual isolated PC save/read/delete operations including error
paths. Recompression still matched both original native save files exactly.
The fixture Lua owner is controlled test infrastructure; the native game Lua
bootstrap remains required. Thirteen worker artifacts are preserved under
`local/worker-validation/storage-20260910d/`.

After incorporating the other orchestrator's latest renderer code, `e946697`
passed the build, both CTests, all three fixtures and both native-file equality
checks again. All42 reviewed names and comments were saved and read back in
the existing Ghidra project; their exports were forcibly refreshed.

## Follow-up from docs/PROFILE_READ_INTEGRATION.md

The Lua-owner bootstrap/close, cached fundamentals source, keyboard archive
reader/runtime sequence, settings restore/reader, profile content commit and
concrete PcProfileIoHost composition are now reconstructed. The old settings
writer's ClanText type and downloaded-content key/value order are corrected.
Combined Win32, existing tests and real Lua/filesystem fixtures passed at8a5ad85;
original native save hashes remain unchanged. Real VFS override dispatch,
keyboard-device installation and platform/renderer/content/scene services remain
required. See the follow-up report for exact evidence and validation boundaries.

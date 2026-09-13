# Native loader integration

Addresses: 00B43B00, 00BDF4C0, 00B26500, 00B265C0, 00B26680, 00B3C3A0, 00B354D0, 00B35930, 00B3A7E0, 00B3B3C0, 00B319B0, 00B30B40, 00B31BD0, 00B31C20, 00B2C2D0, 00B3B1E0, 00B3B260, 00B3CED0, 00B3CFA0

Exact validated and promoted code: `b18f5477069e6db5da7e11a65ee527aefed3d7f0`.
Seven previously deferred sources are now registered in `cmake/startup.cmake`.
The normal `scripts/build.ps1` Win32 Release build with warnings as errors and
both existing CTests passed, followed by seven focused fixtures against that
exact combined library. All 824 direct and four resolved indirect numeric
call rows passed; remaining indirect calls retain separate evidence.

The batch combines the actual descriptor reader, VFS name resolver, state
caches, compiler builder/wrapper prefix, texture cache, D61810 reflection-owner
lifetime/arrays, and named cube/volume constructors. Persistent child operations
publish before native work. The texture correction keeps actual name headers
alive through failed resolver teardown; the shader array guard prevents silent
discard of a failed operation's unpublished allocation. Compiler and texture
bindings check their actual string and owner domains.

The fixtures cover original compiler builder/lifetime, state caches, installed
shader/VFS parsing, texture record growth, deliberate resolver failure, actual
shader-owner lifetime and a real-HAL named cube/volume comparison. Each keeps
its original scope: actual full texture-cache loading, complete native compiler,
native FH3 execution and gameplay are not established by these checks.

Saved Ghidra analysis includes the full compiler destructor through B3AE9B
(618 instructions, zero gaps), the shader-owner destructor through B3B23C
(28 instructions, zero gaps), and the reviewed post-free reserve/delete repairs.
Annotations preserve prior values and comments; affected exports were refreshed.
Reports and ignored immutable checkpoint archives pin sources, library hashes,
original-byte inputs, fixture recipes/logs and saved-analysis journals.

## Follow-up packets

The active workers are recovering actual shader reflection, unconditional
vertex/pixel system-field appends, and cube/volume texture-loading branches with
canonical companions. These are separate contracts within the remaining native
compiler and texture path; they do not yet provide a complete cold effect load.
Registry production, remaining field/source generation, callbacks and complete
renderer teardown still require concrete reconstruction. The game rebuild and
required gameplay validation remain unfinished.

`reports/native_loader_integration.json` is the current integration record.
The earlier temporary-include checkpoints are historical and superseded.

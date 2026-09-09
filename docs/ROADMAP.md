# Reconstruction milestones

## 1. Establish evidence and a working build — initial slice complete

Existing-project target checks, disk fingerprint, full internal-function inventory,
resumable per-function export, Win32 CMake build, address ledger, and five math routines
with bounded native differential tests are present. Full pseudocode export is available
as an explicit command and has not been run.

## Library discovery and reconstruction priorities

The September 9 discovery import and refreshed exports completed successfully.
[Inventory review](../reports/library_inventory/REVIEW.md) records the import
counts, confidence boundaries and comparison with existing reconstruction names.
All 8,780 rename tags matched the post-import snapshot; the 1,554 bookmark-only
classifications are separate. Naming progress does not reduce the raw coverage
denominator or demonstrate implemented behavior. The original inventory's
candidate counts and calendar projections remain historical estimates.

Reuse source where the evidence supports it:

- **Lua 5.1.1:** already compiled for the script probes. Native runtime integration
  still needs the game allocator at `00a6a1d0`, omission of `dofile`, omission of
  `math.random`/`math.randomseed`, and custom bitmask library opening at `00b6a020`.
  The native library tables differ from stock `luaL_openlibs`; the current host
  fixtures do not establish native scripting-runtime equivalence.
- **zlib 1.2.1:** checksum-pinned stock source now builds as `bsp_zlib121`.
  Recover the game's retained-source, buffering, read/seek and raw-DEFLATE
  adapter before connecting compressed archive entries. See
  [zlib evidence and next boundary](ZLIB_DEPENDENCY.md).
- **CRT, STL and compiler helpers:** use the target toolchain to generate ordinary
  runtime machinery from reconstructed declarations. Recover object layout,
  destruction order, pointer adjustments, comparators and container mutation
  contracts at their callers. A throw string or checked-iterator shape alone
  does not identify an entire routine as replaceable stock code.

In-house Dyn physics, the game's modified MT19937 seeding, engine wrappers,
telemetry and IPC remain behavioral dependencies when reached. Classification
alone is insufficient evidence for removing them or introducing no-op adapters.
RTTI and provisional names guide investigation; they are not recovered source
symbols. Partition or call-graph suggestions can help assign disjoint agent
ranges, but named functions and indirect-call dependencies must remain in scope.
Track completion by working subsystem paths and explicit validation evidence,
not by the number of functions renamed or a projected functions-per-day rate.

## 2. Resolve startup and the first subsystem boundary — in progress

Confirmed the WinMain calling convention, application lifecycle boundaries, and the
per-thread random subsystem. The integer PRNG and registration/lock lifecycle now compile;
the stream matches original code over 1,500 outputs. See `STARTUP_RANDOM.md`.
Useful names and evidence comments are tracked in `config/ghidra_names.json`;
library classifications have their own ledger. The incorrect CRT free noreturn
flag was corrected, though some saved bodies still require raw-tail inspection.
Resolved the concrete Windows vtable at `00d68cc4`; its loop is `00bec1a0`, now ported with
explicit external callback interfaces and a real-message-queue probe. Recovered eight missing
vtable-target functions and named the application frame method. See `PLATFORM_LOOP.md`.
Recovered the window-procedure thunk and concrete D3D9 constructor/device initializer.
Its device-creation prefix now runs against a real D3D9 device; see `D3D9_STARTUP.md`.
Default render/sampler states and their cached setters/guards now compile and pass a real-device
probe; see `D3D9_STATES.md`. Surface binding and dynamic buffer creation now have real-device
checks; surface release/recreation also passes a real host-driven device Reset. Wrapper ownership
and native reset dependencies are traced in `D3D9_RESOURCES.md`. Buffer recreation and partial lock
paths now upload real data with DISCARD/NOOVERWRITE; see `D3D9_BUFFER_UPLOAD.md`.
Non-indexed draw and stream frequency are ported with a diagnostic triangle pixel readback;
logical stream binding and indexed draw now also pass diagnostic readback. Full stream
constructors, hardware declaration translation and registry ownership remain pending (`D3D9_DRAW.md`).
Declaration append/stride/semantic lookup now drive the probe's stream stride (`VERTEX_DECLARATION.md`).
Hardware element conversion and declaration creation now replace FVF in both draw checks
(`HARDWARE_VERTEX_LAYOUT.md`); surrounding diagnostics and full layout lifetime are pending.
Logical vertex/index locks now drive the drawing probe (`LOGICAL_STREAM_UPLOAD.md`);
physical cursor rewind and explicit registry operations are now ported
(`BUFFER_LIFETIME.md`) with real-device reuse readback.
Next, integrate native stream constructor/destructor and base teardown
and integrate the reset/presentation path, and trace the
close-request confirmation UI and update path (`WINDOW_CLOSE.md`) before wiring
a complete window lifecycle.

1. WinMain `008f81f0`: confirmed four stack arguments and `RET 10h`; prototype updated in Ghidra.
2. Export its direct initialization callees, starting with `00737970`, `0073d410`, and
   `00737f30`; classify them from callers, strings, imports, and object accesses.
3. Recover constructors/vtables, sizes, ownership, global initialization order, and shutdown.
4. Map which imports are actual platform/service requirements. Keep stubs explicitly labelled
   in any future experimental harness; do not treat a stub-linked executable as a game rebuild.
5. Build a small executable only when a real initialization path is reconstructed.

## 3. Reconstruct a vertical slice

The diagnostic path now loads the installed atlas, generates and compiles shader
sources, filters interpolators from pixel disassembly, and draws through recovered
buffer/layout/state operations. Camera projection, affine parent-world refresh,
view inverse and combined view-projection caches feed the generated shader draw.
Focused isolated native camera fixtures and pixel readback pass; this remains a
diagnostic host, not a game runtime. Current dependencies include transform-edit
invalidation/ownership, the complete system constant gatherer, shader descriptor
loading, material execution and startup integration. See `CAMERA_TRANSFORM.md`
and `SHADER_CONSTANT_DISPATCH_ANALYSIS.md` for concrete next boundaries.

The font path now loads real Lua descriptors, glyph DAT and image resources,
generates a native-compared glyph quad and draws an installed bilinear font
shader through material slots and indexed buffers (`FONT_MATERIAL_DRAW.md`).
Shader suffix rewriting and font image ownership are recovered. Native texture/
shader search lists, ordered fallback and physical directory resolution now
drive the draw (`VFS_MOUNT_LOOKUP.md`). Recovered mount insertion, virtual `.`
handling and physical factory policy now connect the first two startup mounts
with a supplied root (`PROVIDER_FACTORY_STARTUP.md`). Mounted stream loading now
connects physical providers and the priority-300 FileStore to font ownership and
shader script loading. The existing draw probe explicitly primes three font
resources, then loads GFX/alpha/DAT entirely from the cache with no physical-file
opens during that load (`MOUNTED_RESOURCE_STREAMS.md`). This establishes the
cache-to-render path with host ownership; native preload selection, complete
stream/provider lifetime and archive loading still need integration.
Full text layout/batching, GUI transforms and startup
integration also remain necessary. The draw uses supplied camera constants.
This advances the asset-to-render path but is not a
runnable game target or an original-game visual comparison.

Choose one real path from file loading through decoded data to a visible result. Recover
archive/resource formats and object interfaces, then window/input/device lifecycle and a
minimal render path. Use the user's installed assets as local inputs. Add evidence fixtures
for the loader and compare a controlled output with the original application.

## 4. Expand simulation and gameplay

Recover scene/entity structures, clocks, transforms, scripting, audio, movement, weapons,
damage, AI, mission progression, and UI incrementally. Grow compiled coverage by dependency
order. Record addresses and evidence for each port; prefer independent behavioral fixtures
and original-code comparisons where functions can be safely isolated.

## 5. Establish compatibility and completion

Investigate original register ABIs, x87 control state, allocator behavior, SEH, RTTI, compiler
version, and layout requirements. Separate a source rebuild from a byte-identical executable.
Completion requires startup, rendering, asset loading, and playable scenario comparisons,
including shutdown and save/load. A successful library build or pseudocode export is not
completion of this milestone.

## Next bounded work

1. Recover inflater seek `00bbc060`, read `00bbc140` and output-drain helpers;
   connect them to the identified raw-inflate wrapper without bypassing buffered
   reset, short-read or error behavior. Stock zlib is build-tested only so far.
2. Complete package enumeration, transformed MPKG directory parsing, entry lookup
   and compressed/sliced source ownership (`ARCHIVE_PROVIDER_ENTRY.md`). Then
   exercise a real installed archive through the same mounted stream interface.
3. Recover native FileStore preload policy and stream tracking, replacing explicit
   diagnostic priming only when the actual initialization path is established.
4. Continue independent renderer/window lifetime and font layout/batching work,
   integrating the resulting paths before expanding gameplay. Assign disjoint
   files and address ranges to parallel agents; keep shared metadata and Ghidra
   changes coordinated through the primary agent.

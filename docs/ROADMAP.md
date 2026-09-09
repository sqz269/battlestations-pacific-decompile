# Reconstruction milestones

## 1. Establish evidence and a working build — initial slice complete

Existing-project target checks, disk fingerprint, full internal-function inventory,
resumable per-function export, Win32 CMake build, address ledger, and five math routines
with bounded native differential tests are present. Full pseudocode export is available
as an explicit command and has not been run.

## 2. Resolve startup and the first subsystem boundary — in progress

Confirmed the WinMain calling convention, application lifecycle boundaries, and the
per-thread random subsystem. The integer PRNG and registration/lock lifecycle now compile;
the stream matches original code over 1,500 outputs. See `STARTUP_RANDOM.md`.
Ghidra has 81 useful names with evidence comments; its incorrect CRT free noreturn flag is corrected.
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
constructors, declaration formats and registry ownership remain pending (`D3D9_DRAW.md`).
Next, recover registry/base teardown
and integrate the reset/presentation path, and trace the
close-request-to-application-exit path before wiring a complete window lifecycle.

1. WinMain `008f81f0`: confirmed four stack arguments and `RET 10h`; prototype updated in Ghidra.
2. Export its direct initialization callees, starting with `00737970`, `0073d410`, and
   `00737f30`; classify them from callers, strings, imports, and object accesses.
3. Recover constructors/vtables, sizes, ownership, global initialization order, and shutdown.
4. Map which imports are actual platform/service requirements. Keep stubs explicitly labelled
   in any future experimental harness; do not treat a stub-linked executable as a game rebuild.
5. Build a small executable only when a real initialization path is reconstructed.

## 3. Reconstruct a vertical slice

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

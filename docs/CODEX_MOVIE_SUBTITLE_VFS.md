# Movie subtitle VFS provider closure, 2026-10-28

Packet `codex-movie-subtitle-vfs` follows source dependency
`609ef74158b525c1d860bd5351dbfd195ce2689e`. That parent remains subject to primary
review/acceptance; this packet does not promote it or enable subtitles.
`kMovieSubtitlesBound` remains **OFF**. The loader files and the parent's receipt
are unchanged. This document supersedes only the parent's working-directory
file-provider limitation for this new source revision.

## Original Source contract

The configured target remains `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. `bsp.py ghidra ensure --status` confirmed that
program before the bounded `lookup/show` batch. No Ghidra edits were made.

- `00AB0606..00AB0616` loads `[0109CEEC]`, calls current manager vtable+8 with
  the `.mws` name, then stores AL at movie+104h (`00AB061A`). A false result
  skips loading. `00AB00D1` clears +104h on subtitle clear.
- The production `NativeVfsRuntimeBindings::exists` verifies slot+8 as
  `00BDD440`, whose qualified reconstruction copies/normalizes the input and
  probes actual mounts. It does not perform `00BDF4C0` alias/alternative-name
  resolution. The bridge therefore calls `GameVfsHost::exists(path)` directly.
- Native Lua file reading uses flags 2. `GameVfsHost::open(path, 2)` dispatches
  through its retained actual manager and `00BDF310` open route, obtains the
  complete provider bytes, and completes native stream release before returning
  its checked memory-stream view. No new mount, lookup provider or native
  stream implementation is introduced.

The existing `MovieWidgetState` projects decoder, completion, filename and loop
fields, but has no +104h field. It is unchanged. The bridge's explicit
`subtitle_file_visible_` records the proved existence meaning without claiming
native layout or aliasing a guessed raw field. `subtitle_schedule_ready_`
separately records successful parsing.

## Ownership, load and disposal

`GameMissionFrameHost::Impl` already borrows the application's `GameVfsHost`.
Its sole real `GameMissionResultHost` construction now forwards that same
reference; the result passes it into `MovieBridge`. The bridge neither creates
nor drains a VFS owner. No provider method is called by these constructors.
The result remains owned by the frame for the mission. Existing result and
callback lifetime obligations still require the enclosing VFS owner to survive
all consumers and the shared drain.

When the switch is eventually admitted ON, preparation clears prior projection
state, requires the existing VFS to be ready, performs the direct existence
probe, and returns with both flags clear on a normal missing file. A visible
file must then open through mode 2 and provide complete initialized backing.
The consumer verifies the Lua read domain and the full returned extent before
passing bytes to the existing loader. A legitimately opened empty file remains
distinct from open/read failure.

Provider exceptions propagate unchanged. Visible-but-unopened, incomplete or
short streams produce errors; they are never caught and parsed as invented
empty source. `GameVfsHost` retains its existing native-operation failure and
process-retention policy. The returned memory-stream wrapper is scoped to
preparation and owns no outstanding native provider stream. Clearing only
forgets the subtitle projection; it performs no VFS cleanup or mount mutation.
OFF still executes the original three unimplemented callback log paths.

## Validation and remaining activation work

`scripts/build.ps1` passed the standard MSVC Win32 Release four-node build and
both existing CTest checks (`reconstructed_math`, `tool_tests`; 2/2, 12.73 s).
The result, source hashes and artifact hashes are in
`reports/cc16_movie_subtitle_vfs.json`. No new tests are added and no game or
ad hoc probe is launched by this packet. Root centrally schedules OFF identity
and installed native VFS probes under its run cap.

This is a provider-wiring closure with zero new native-function body credit and
no ABI claim. Full `00AB0520` and subtitle activation still require real page/
callback reachability, the decoder and Bink clock, Text/localisation and widget
ownership, subtitle option/language inputs, extratitles, and the original Lua
library/override/error/owner contract. No fabricated default providers are
added for those dependencies. CMake, VFS implementation, active GUI/Text/raw
Model owner files, ledgers and original install/save/profile files are unchanged.

# Production native VFS admission (BJ)

Addresses: source integration of the existing 0073D604 phase-2 sequence,
0073D94F factory tail, BDF4C0/BDD6E0 resolution, BDD990 enumeration,
BDD340 metadata and BDF310 stream opening. No newly recovered original symbol
or drop-in register ABI is claimed.

The rebuilt executable now retains verified original read-only data and the
actual VFS graph through its existing singleton host. Production Lua, settings,
locale, sound, fonts, frontend, decals and scene readers use that same graph.
The validated run reached the frontend, presented two frames and completed
the shared singleton drain. Gameplay validation remains outstanding.

## Ownership and startup

- WinMain verifies the selected original executable, launches its own suspended
  child, reserves the CF/D1/D5/D6 bands, and waits for the child's mapping ACK.
  The child maps only verified read-only data; no original executable code runs.
  Public argument values using the private handoff prefix are rejected.
- The rebuilt image uses base 30000000 with ASLR disabled so it does not occupy
  those bands. Foreign reservations remain an explicit handoff failure. The
  linker retains `build/win32/bsp_game.map` for fault address lookup.
- `GameNativeVfsApplication` owns stable publication cells, type storage,
  owner services, constants and runtime. It borrows the existing
  `GameSingletonHost` and the process physical-pool owner. The normal teardown
  drains that host before destroying the VFS bundle or unmapping data.
- `GameVfsHost` invokes the actual core, three loose mounts, two fresh package
  scans, full search defaults and archive factory tail at their existing
  application phases. Mount records describe the loose requests only; a null
  result is recorded as failed. Package entry counts are not fabricated.
  Phase-6 parser hosts retain their separately documented source boundaries.

## Consumer compatibility and failures

`NativeVfsAccess` lets existing parser interfaces retain `VfsMountContext&`
without constructing a projected provider manager or copying raw mounts.
The six consumer entry points dispatch before consulting projected containers
or candidate registrations. The unused registration object exists only for
legacy constructor signatures. Pending-I/O fragments still require their
projected provider domain and are not admitted through this bridge.

Production opens accept only the existing read-only modes 2 and 32h. The raw
runtime can preserve other flags for separately reviewed callers. Completed
raw reads become source `MemoryStream` carriers after the original stream has
been released; the new carrier also represents an opened zero-length file.
That carrier is not a reconstruction of the native memory-stream constructor.

An exception inside a native operation prevents further entry into the graph.
The application checks before normal shutdown, raw drain and destruction, and
uses `_Exit` to retain the full owner/data/CRT lifetime until OS reclamation.
Hardware-probe and completed-byte carrier allocation failures are outside that
classification. Interrupted name-resolution frames expose their last site;
other operations may have no recorded site. This terminal policy is source
reviewed; fault injection of a partial acquired frame was not performed.

## Validation

The exact code/build revision, hashes, commands and receipts are recorded in
`reports/game_native_vfs_admission_bj.json`.

- MSVC Win32 Release build and both existing CTests passed after concurrent
  main integration and after the diagnostic-map relink.
- The executable mapped the original data, completed 3/3 loose mounts and two
  package scans, registered four factories, opened the input script tables,
  loaded 6,856 locale keys, six fonts with 19 resource opens, and five GUI pages.
  It presented two 640x480 frames, drained the actual singleton host and exited 0.
- The successful run explicitly selected the existing private GFWL 2.0 runtime
  and its identity-checked dependency, as prior production validation did.
  A preceding run with the installation's XLive replacement reached settings
  then exited with an access violation. An owned debugger reproduction located
  the fault at installed `xlive.dll+319049`: `mov [edi], edx` attempted to write
  90909090 to rebuilt image base +640F5E, beyond its 1F8000 image size. The
  captured child and parent both exited C0000005. The installed DLL was not replaced.
- Settings and logs were isolated under ignored `local/`. The original executable
  hash and live options/save file hashes and metadata were unchanged. The existing
  Documents save directories were required because startup's directory creation
  does not use the settings-root override.

The worker raw-graph fixtures separately cover complete and capacity reads,
ordinary missing names, direct output preservation/aliasing, enumeration flags,
metadata and a 1,387-byte mode-32h descriptor read. Their receipts distinguish
the earlier linked core from this final production run. No permanent tests were
added. Archive payloads, interrupted cleanup, original ABI compatibility and
gameplay are not established by the frontend run.

## Follow-up boundaries

Preserve the explicit validated GFWL runtime selection in startup probes. A
deployment-friendly runtime-selection policy remains separate work. Exercise
mission/content consumers through the new VFS before claiming gameplay. Native
pending I/O requires its own retained interface. Future model-group adoption
requires D7-band storage beyond these four VFS bands and complete external
provider/cleanup contracts; the current mapping must not be treated as enough.

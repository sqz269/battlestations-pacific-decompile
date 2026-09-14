# Raw VFS retirement in the application singleton host (BH)

`GameSingletonHost` now exposes its existing native deletion binding for
retained VFS composition and accepts a borrowed `GameNativeVfsRuntime` before
that runtime registers its core. `shutdown()` retires the bound runtime
immediately after the raw singleton drain, while this host's deletion table
and publication cells remain valid. A shutdown with no current manager also
retires an attached, unregistered runtime. The callback pointer is cleared
before retirement, so later shutdown/destruction cannot call a dead runtime.

This supplies the lifetime hook required by
`docs/GAME_VFS_PROCESS_LIFETIME_BH.md`: the existing application destructor
deletes the singleton host before the VFS host. Delaying raw VFS retirement
until that later destructor would leave it accessing destroyed references.

The caller must retain an attached runtime through shutdown, including failed
core startup. The new binding does not create a lifetime manager, drain one
independently, construct providers, or migrate the still-projected
`GameVfsHost`. Original native shutdown ABI and CRT behavior are not new
claims from this source adapter. The combined native-data/VFS integration
records the exact build and focused real-host drain proof separately in
`reports/native_vfs_handoff_bh_validation.json`.

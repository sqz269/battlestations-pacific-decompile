# Raw VFS owned full-file read adapter (BH)

`GameNativeVfsRuntime::read_all(path)` uses the same initialized raw manager,
retained services, path string pool, and `NativeVfsRuntimeBindings` as `exists`
and the existing caller-capacity `read`. It opens with mode `2`, takes the
stream's 64-bit length from its `+30h` dispatch, reads until that length is
filled, and releases the one caller stream reference exactly once. The binding
already dispatches memory, physical, adopted-substream, and raw-inflate stream
length/read profiles; an unsupported profile remains an exception.

The return contract is `std::optional<std::vector<std::uint8_t>>`: `nullopt`
means the raw open returned no stream, while an opened zero-length file yields
an engaged empty vector. Null paths, an unregistered core, unsupported
dispatch, allocation failure, failed length/read, and failed release throw.
The 64-bit length is rejected before allocation when it exceeds Win32's
`size_t`, one-call `uint32_t` count, or `vector::max_size` limit. A partial
read is retried; zero progress or a returned count larger than requested is
a read failure. On exception during length, allocation, or read, the stream
reference is still released. If release itself throws during unwinding, the
original failure is preserved; on the success path the release error surfaces.
No host filesystem fallback or projected manager is introduced.

## Consumer migration required

Current source was rechecked for this packet. `GameVfsHost::resolve_and_read`
in `src/game_hosts_vfs.cpp` still uses projected `manager_->context()`,
`resolve_existing_resource_00bdf4c0_fragment`, and
`open_resource_memory_00bdf310_fragment`. The new API supplies owned bytes,
but does not supply the separately reported resolved path; a native path
resolution/diagnostic adapter or a deliberate probe-result contract change
is needed when migrating that method. It also does not replace manager-based
enumeration, mount/search records, or parser registration.

Direct projected full-file readers to migrate are `src/game_hosts_fonts.cpp`
and `src/game_hosts_frontend.cpp` (both currently hand `MemoryStream` to
parsers/textures), `src/game_hosts_init_tail.cpp` (decal Lua text),
`src/game_hosts_mission.cpp` (scene document text), and
`src/game_hosts_scene_contents.cpp` (property-library text). Callers needing
`MemoryStream` must adapt the owned bytes without silently treating `nullopt`
or a read exception as an empty successful stream.

`src/game_hosts.cpp` passes projected manager/search contexts into script,
settings, and sound services; `src/game_hosts_lua.cpp` constructs
`VfsLocaleRuntime` with the projected context. Those need explicit raw-backed
read/enumeration interfaces and lifetime wiring before the projected manager
can be removed. The separate property-library enumeration in
`src/game_hosts_scene_contents.cpp` also remains projected. No consumer is
switched in this packet.

The primary lifetime audit also found that `GameStartupHost` currently deletes
`GameSingletonHost` before `GameVfsHost`. Production raw-VFS migration must
retire the raw runtime immediately after the shared singleton shutdown, while
the deletion bindings and publication references are still alive. The
physical-provider pool `0109DBF0` and shared type descriptors need process/CRT
storage rather than copied probe-local storage. See the primary checkout's
`docs/GAME_VFS_PROCESS_LIFETIME_BH.md` for that integration audit.

The API is source-level composition evidence, not a drop-in original binary
ABI or game-process validation. The local Win32 probe compiled and linked,
but its run stopped before raw startup because the required `00D10000`
numeric data band was unavailable in this process. It therefore did not
exercise the new installed-file, missing, empty, repeated-read, or drain
checks. Short-read zero-progress and exactly-once exception cleanup are
explicit source paths here, without injected fault proof. Archive profiles
retain their separate phase-two validation scope.

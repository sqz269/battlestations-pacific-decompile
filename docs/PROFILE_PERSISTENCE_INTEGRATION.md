# Profile text, mission records and storage continuations
Addresses: 006ADB50, 006AD3C0, 007FF100, 007FA710, 007FA670, 007FA220, 007F9500, 007FDF00, 007F9540, 00920000, 0090CB40, 0090BF50, 00920E10

The profile writer now has a concrete text backend and a binding to the recovered
storage driver. `006adb50` drives storage updates and prompts; it can invoke its
continuation before returning. An interactive prompt instead retains the shared
continuation. The previous `register_task_006adb50` interface name implied less
behavior than the native routine performs and has been replaced with
`run_storage_operation_006adb50` in both profile hosts.

`TextProfileWriteHost` creates each temporary `ArchiveTextWriter`, retires it
before the next driver call, and dispatches the four recovered profile callback
addresses. It shares the chosen output sink across these temporary writers. Its
storage host must perform actual progress and render work; settings services must
write the actual keyboard setup and options text. Those effects have no successful
default implementation. Objects captured by a retained continuation must outlive
the operation, including the read and write hosts.

`MissionProgressOwner` manages actual reconstructed mission-record and counter
maps. `MissionProfileArchiveHost` binds allocation, destruction, read, write, sum
and the unlock completion projection to that storage. The hints singleton and
profile-manager refresh remain required external effects. The caller must share
the same owner with its profile-reset host. Failed allocation stays null; attempting
the subsequent native null dereference is reported as a host exception. No profile
records or allocation success are synthesized.

The profile writer now accepts tagged section keys as well as names. Native
`TotalScores` children are numeric keys 0, 1 and 2; stringifying those keys would
change the archive schema. Mission archive details, including the checkpoint
spelling mismatch and write-time flag clearing, are documented in
`MISSION_PROGRESS_ARCHIVE.md`.

`GuiLua51Host` binds the recovered `GuiLuaReader` to the existing Lua 5.1.1
library. It uses registry references and a separate cursor reference because the
reader releases each enumerated key before asking for the next entry. The reader
takes ownership of its root handle. A protected host entry executes generated
archive text and reports real Lua errors while restoring the stack; it opens no
standard libraries. This entry is distinct from the game's GUI loader, which
uses unprotected `lua_call`. Generated ordinary tables are the supported archive
input; this adapter is not a sandbox for arbitrary scripts or metatables.

All these interfaces are C++ host compositions. The native profile functions
retain their individually documented register/stack ABIs; the host classes are
not native object layouts or drop-in binary replacements. Raw Lua text in a
memory sink is uncompressed archive data, not a complete native save container.
Game save compatibility and gameplay remain unvalidated.

Ghidra definitions for response `006ad3c0` and writer deleting destructor
`00435640` were independently checked against the installed image, created
under the shared write lock, and saved in the existing `bsp` project. The archive
buffer flush tail `00bd4979..00bd49a9` was decoded and its returning call override
repaired, but the stored body of `00bd48f0` remains incomplete. The report
`archive_buffer_flow_repair.json` records that partial result. Its raw assembly,
not the incomplete decompilation, is the evidence for the missing tail.

Validation for the combined implementation is recorded in
`reports/profile_persistence_integration.json`.

# Shared VFS owner and MPAK storage integration

`NativeVfsOwnerServices` retains string, physical-stream, render-batch and
type-counter contexts over the caller's existing publication cells and one
borrowed raw `01090AA0` manager. Construction does not fetch a singleton.
Its explicit deletion binding must be installed before registration, and the
bundle, publications and table data must remain live through manager drain.
The owner-services translation unit is now registered in the core CMake target.

The type counter now accepts the same `SoundLifetimeAccess` as the other
services. Its first publication read is the fast return; otherwise it captures
the first manager section, rechecks, initializes the eight-byte owner with
counter zero, publishes, resolves the manager again for registration and reloads
publication after unlock. The `CFB6C4` deletion route uses the popped owner and
clears the original publication without resetting type descriptors or guards.
Two complete native envelopes and the deletion table were verified; saved
ownership at `006FAD60` was restored with prior metadata retained. A transcription
error in the new getter annotation initially said counter one; the saved comment
and generator were corrected from the verified zero store at `006FAC8E`, with
the correction receipt retained. Existing source and earlier evidence used zero.

`NativeMpakStorageServices` retains the concrete lookup, offset-copy and
container implementations and supplies them to `NativeMpakRuntimeInputs`.
It borrows the existing string, stream, allocation and registry/manager owners;
it creates no provider or second manager. Static review checked every forwarded
runtime input against the existing aggregate contract.

Production composition also exposed a missing `CFEA10` MPKG factory deletion
route. The shared dispatcher now calls the existing secondary thunk with the
popped allocation+4 pointer and retained factory context. See
`NATIVE_MPKG_FACTORY_DELETION_BD.md`. The raw-owner fixture checks six
registrations/deletions, including secondary registration and clearing a changed
MPKG publication while preserving its unrelated replacement.

The populated raw VFS/Lua fixture uses the same owner bundle and the existing
raw fundamentals context. One manager drains its six owners after installed
physical HANDLE reads, nested `DoFile` order `MCPP`, memory/physical/adopted
FileStore conversion and Lua close. Publications clear, descriptor guards remain
set, and retained-memory counters return to zero. A separate retained semantic
Lua fixture checks compatibility. All three primary fixtures link the combined
core archive; the new owner source is no longer compiled separately by them.

The populated fixture still seeds valid VFS/provider/FileStore records. It does
not prove production manager construction, package/search setup, host migration
or gameplay. The installed tree has no `.mpak`, `.mpkg` or `.pak` archive, so a
real archive member comparison has no installed input. This limits that
particular check; it does not require delaying loose-asset game startup or
installing a reduced factory implementation. The production composer must bind
all reconstructed profiles, including genuine MPKG/MPAK data outside `.rdata`.

Follow-up packets build the retained game VFS composer, recover actual `+60`
search-group registration and provide verified initial-image VFS constants.
The separate `+54` extension-prefix tree and full startup registration sequence
remain explicit dependencies. Game-host and external `manager()` consumers
must migrate together after those contracts are ready. The exact source/build,
fixture hashes and native evidence are pinned in
`reports/vfs_storage_owner_bb_validation.json`.

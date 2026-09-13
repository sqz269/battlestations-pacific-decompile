# Observer dispatch owner in the raw singleton drain

Addresses:00BD0400,00695F40,00696360,00CCD6A0.

The raw manager now recognizes the actual dispatch owner profileCF7E74 and calls the recovered00695F40 scalar deleter. A live table read at00CF7E74 gives405F6900. The manager pops the original object before its slot-zero call with flags1; the source map passes that popped object directly, without comparing it to the current publication.

`NativeSingletonDeletionBindings` borrows the sameE198DC publication cell used by the getter and CRT producer. `GameSingletonHost::bind_observer_dispatch_owner` installs that cell before registration. The deleting routine clearsE198DC and leaves the separateE198E4 owner+4 alias unchanged. Descriptive names are hypotheses; original ECX/stack/RET4 details remain in `reports/observer_dispatch_owner.json`. The explicit C++ binding is not a drop-in native ABI replacement.

MSVC Win32 and both existing CTests passed. One focused source lifecycle creates the actual dispatch owner throughCCD6A0 first, then lazily creates the actual observer lock in the same raw manager. It checks two registrations, duplicate edge references and cleanup before draining both profiles. Both owning publications clear; the alias address remains unchanged and is never dereferenced after free. This composition check uses real recovered allocations, manager registration and Win32 locks. It is not an original-byte differential test.

The exact probe, source, command, object and three Release libraries are archived with hashes in `local/observer_dispatch_binding_artifact_manifest.json`; the report records source hashes and validation scope. No new permanent tests were added.

GameStartup does not yet instantiate these observers. Runtime integration must place the recovered CRT publication before observer users and complete all edge cleanup before singleton drain. Recreating only the owner getter does not repair the old alias. Native exception, allocation-failure, concurrency and gameplay behavior remain outside this check. Q's Ghidra body-membership gaps remain separately recorded despite complete PE destructor bytes.

## Correction from docs/GAME_OBSERVER_RUNTIME.md

Application startup now instantiates the actual dispatch owner through the existing raw singleton manager. The 120-frame process run records publication and manager teardown; the alias remains unchanged and the unused lock stays lazy. This replaces the earlier startup-instantiation follow-up only; actual unit event delivery remains separate.

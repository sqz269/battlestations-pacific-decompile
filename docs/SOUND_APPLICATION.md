# Sound application integration status

Addresses: 0073DAFD callsite in0073D410; composition of already reconstructed
sound routines. No new application entry or replacement binary ABI.

Native application phase5 creates the178h sound manager at0073DAFD and the
234h streamed-dialog manager at0073DB2B, before phase6/window startup. The
caller derives disabled from the audio-enabled byte. GameSoundRuntime now
composes existing FMOD, VFS, Lua, clock, sample, channel, resource and shutdown
services. SoundAlternateOwner reconstructs the second owner and its table is
loaded by the actual-storage parser in SOUND_DIALOG_TABLE.md.

GameSoundLoadEvents borrows the real selected XLive DLL and existing platform
cursor policy. It invokes XLive pretranslation ordinal5030 and BECB20 using
the supplied platform, publication services and three current cursor bytes.
It creates no independent input/online owners. Counters record actual adapter
invocations and do not substitute external results.

These components are not yet attached to GameStartupHost phase5. Main is
migrating GameSingletonHost to raw native singleton registration/destruction;
its current finite deletion profiles do not include sound. The sound services
still require the existing semantic SingletonLifetimeDomain interface. A
second private domain would bypass application ownership, so the application
entry remains pending the shared manager contract.

Independent integration review found that native event-query scalar deletion
A89B40 does not unregister its shared-domain entry. The composition wrapper
now removes the captured pointer before deleting it. This is explicit host
ownership policy; the recovered native destructor stays unchanged. It is
safe both during explicit runtime shutdown and after a domain drain has
already popped the entry. The combined local fixture creates the query in
both paths and drains the domain after explicit shutdown to detect stale
registrations.

The installed fixture exercises real sound libraries, soundsetup.lua, an FSB,
an event bank, the streamed_dialogs.def table and actual XLive pretranslation.
It constructs no online or input owner, so cursor policy follows its real
early guard. It does not tick or start logical dialog streams: those remain
required dependencies, as does callback230. Build and fixture receipts are
in reports/sound_application.json and the component reports. None establish
audible playback, executable startup, native ABI compatibility or gameplay.

## Follow-up packets

Add sound registration/deletion to the shared raw singleton contract with the
main owner, then attach phase5 using the same VFS/Lua/clock/platform services.
Recover logical updateA78820 and startA783F0 with actual54h stream ownership
before enabling streamed-dialog ticks in the application loop.

# Application sound startup and shared platform services

Addresses: 0073D410, 0073DAFD, 0073DB2B, 0073DC25, 00BEC1A0, 00BEC1D8

The rebuilt executable now starts the actual core sound and streamed-dialog
services before creating its window. It uses the application's existing VFS,
Lua runtime, frame clock, native strings and raw singleton lifetime manager.
The normal message loop and sound-load message service share one selected
`XLiveLibrary`. This is a bounded application composition, not a complete
reconstruction of 0073D410 or an original object ABI replacement.

## Native order and ownership

0073DAFD calls A88770 after settings loading; 0073DB2B calls A79230. Parser
registration follows at 0073DB41..0073DB69, then the current alternate sound
owner is reloaded separately for music/speech stores at +218/+21C. The window
virtual +4 call is at **0073DC25**, with caller stack cleanup at 0073DC2B;
older references to 0073DC0F do not identify that call instruction.

Online initialization A40DF0 is called later at 0073DC7C. Input initialization
A982D0 is called at 0073DD8E, after device initialization. The platform packet
verified the loader-zero F8ABE8/F8BBF4 slots and the exclusive nonzero publication
routes into those later phases. Keeping these actual lifecycle slots null
during sound startup preserves BECB20's pre-online guard. Later online and input
owner construction remains explicitly unimplemented in this executable.
See `GAME_PLATFORM_SERVICES.md` for the publication-xref evidence.

`GameStartupHost::SoundServices` borrows these publication cells and owns the
concrete load adapter, XLive loader, dialog facade and core. The dialog attaches
to the actual core/FMOD services before the raw lifetime dispatcher binds that
same core. Binding is the last constructor operation; the aggregate is assigned
before either startup call. An exception after raw registration therefore
leaves the callbacks and services alive for fallback teardown.

Both ordinary teardown and failed-startup fallback drain the raw singleton
manager before destroying sound, DLLs, Lua or VFS. Core destruction precedes
dialog and load-adapter destruction. The CRT math-error publication has process
lifetime, because arithmetic can still run after the sound aggregate is gone.
No second lifetime manager, input backend or online owner is fabricated.

00BEC1D8, in the successful PeekMessage arm of 00BEC1A0, calls the existing
C2F1D2 import thunk for real XLivePreTranslateMessage (ordinal 5030). The shared
loader now supplies that callback in `GameLoopCallbacks` as well as the load
adapter. A returned handled result follows the existing reconstructed loop.

0073D410 receives the application in ECX and two caller-supplied stack values.
This packet does not revise its incompletely modeled return signature. Names
are descriptive hypotheses. New C++ interfaces expose services explicitly;
native ABI, SEH and full application behavior are not established by this work.

## Selecting actual libraries

Optional `--fmod-dll PATH`, `--fmod-event-dll PATH`, `--xlive-dll PATH` and repeatable
`--xlive-dependency PATH` select real libraries. Explicit paths resolve before
`--game-root` changes the working directory. Defaults are the original filenames
under the selected game directory: fmodex.dll, fmod_event.dll and xlive.dll.
The executable's help text is separately owned and does not yet list these flags.

The recorded run used installed FMOD libraries and a private, matching legacy
XLive/credential-runtime pair under ignored `local/gfwl-private-runtime`. Only
msidcrl40.dll was explicitly preloaded; matching ppcrlconfig.dll and xlivefnt.dll
resided alongside it. The installed AlterBSP XLive and the system XLive without
its matching dependency pair were not valid substitutes in the prior probes.
This run changes no original game or SDK files and initializes no online account.
Library hashes and the exact command are recorded in the report.

## Execution evidence and limits

`./scripts/build.ps1` passed Release Win32 compilation and both existing CTests.
The actual `build/win32/Release/bsp_game.exe` then exited zero after two frames
and two presents, with an isolated settings root and an 800x600 window.
Only settings are isolated; the existing save-directory handling still refers
to the user's normal Documents/Battlestations-Pacific/save directory.

The recorded startup log reports enabled sound, seven classes, one resource,
132 FMOD calls with zero errors and one load-focus call. Startup precedes window
creation. The ordinary message loop made two real XLive pretranslation calls.
Raw teardown drained three singleton registrations and left zero sound classes,
samples, resources and pending file adapters. See
`local/sound_application_ab_run.log`, `local/sound_application_ab_run.json` and
`local/sound_application_ab_build.log` for the local evidence.

This startup path opened no sound files and reported zero resource bytes.
It demonstrates real initialization and teardown, not audible playback or a
loaded sound bank. The installed title stream is tested separately by
`GAME_TITLE_SOUND.md`. `SOUND_FRAME_SERVICE.md` documents the independently
recovered frame/load callers; this application still lacks the real game-render
binding that reaches those callers in native order. No arbitrary extra sound
tick has been added to the application loop.

Follow-up work requires shared raw online/IPC ownership, the actual input
backend and lazy action manager, real render/listener/interface bindings, and
the current menu's canonical native string/stream fields plus MoviePlayer host
for title playback. Gameplay, active Bink playback and audible output remain
unverified. The report's direct-call audit is partial coverage of application
initialization and the platform loop, not their complete call graphs.

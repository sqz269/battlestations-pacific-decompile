# Installed FMOD stream bindings

Addresses: 00A86B40, 00A874D0 (game consumers); installed fmodex.dll RVAs
0001E67C, 0001CF31, 0005F77B, 0005FBD5, 0005FC8F, 0005FCB3.

FmodConfigurationLibrary now implements SoundStreamFmodHost. Existing create,
play, release, length, pause, stop and position-query operations are shared.
Four added C exports implement GetFormat, SetSpeakerLevels, SetPriority and
SetPosition. Every call returns and records the installed DLL's actual result.

GetOpenState and IsPlaying use the original exported C++ methods. The installed
DLL is UPX-packed; a manifested Win32 probe loaded the unchanged file and read
its export instructions after unpacking. IsPlaying at RVA1CF31 takes explicit
this and byte-output stack arguments, RET8. GetOpenState at RVA1E67C takes this,
state, percentage and byte-output pointers, RET10. Both are stdcall, so the
existing typed export-call adapter can invoke them directly.

This matters for native output behavior. A874D0 supplies the low byte of its
reused dt word to IsPlaying. The C wrapper at RVA5FF32 instead calls the C++
method with private byte storage, then converts it to a four-byte FMOD_BOOL.
Its null-handle shortcut leaves the caller's output untouched; the actual C++
method writes a zero byte after failed handle validation. An adapter using
sentinel FMOD_BOOL storage cannot reproduce both behaviors. Passing the actual
byte directly also preserves native writes on error and any unwritten output.
GetOpenState likewise receives the caller's actual output pointers without
initializing or reading the unused percentage/starving values.

Installed fmodex.dll SHA256:
`31e7451aef6115b0aec353e4e508ff9f0fc14ea3a8957e5ddb805ede911700e8`.
Only the loaded DLL was inspected; the installation was not modified. This is
a concrete library binding, not a reconstruction of FMOD. The source stream
and dialog routines retain the ABI and storage limits in their individual
reports. Build, export-output and installed lifecycle validation are recorded
in reports/sound_stream_integration.json as they complete.

Integrated Win32 Release and both existing CTests passed. A second manifested
probe invoked these two original C++ exports through FmodConfigurationLibrary
with invalid handles. Both returned the actual result36; IsPlaying changed
exactly one byte to0, while GetOpenState left initialized state/percentage and
starving outputs untouched. Adjacent canary bytes survived both calls.

## Follow-up application packets

GameStartupHost should compose the dialog facade and core after settings at
0073DAA5, then invoke A88770 at0073DAFD and A79230 at0073DB2B before window
creation. Bind the core in GameSingletonHost before registration. The later
0073DB7E/0073DB8E stores apply music/speech levels at alternate+218/+21C.
The current placeholder after window setup is too late.

Application service work remains: real GameSoundLoadEvents needs the live
PlatformCursorHost, cursor state bytes and selected absolute XLive DLL. The
existing log-only pretranslation callback is not a valid replacement. Shared
CRT access needs its bypass word and legacy_crt_87except_00c27489. Reuse the
application's accepted crt_string_storage service; the actual pool currently
requires a semantic singleton domain and must not create a second manager.

Frame sound update belongs in world-view submission735B50 and job4BBD00:
A87BF0 receives the listener matrix and by-value velocity, deriving dt from
the shared FrameClock. The 68A670 listener provider and application render-host
binding remain missing. Identity/zero describes only735B50's null-interface
arm. Title music creation is separately unimplemented. Keep the core, dialog
facade, load callbacks and all borrowed services alive through raw WinMain
singleton drain, then destroy core, facade and singleton wrapper in that order.

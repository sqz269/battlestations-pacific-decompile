# FMOD audio endpoint and startup recheck

The current Console session still has no active playback endpoint. At
2026-10-10T10:18:19Z, MMDevice creation, active render enumeration and count all
returned `S_OK`, with count **0**. Default console, multimedia and communications
queries each returned `80070490` and no endpoint. Audiosrv and AudioEndpointBuilder
are running automatically; MMCSS is a running automatic **kernel driver**. Its
absence from Win32_Service is not a missing-driver finding. Eight healthy PnP
sound records and registry DeviceState records do not establish active endpoints.

A separate manifested Win32 `audio_probe.exe` loaded the actual installed
`fmodex.dll` and `fmod_event.dll`. Their versions remain 4.18.4 and both full hashes
match the historical receipt. Genuine Create/GetSystem/GetVersion/GetOutput/
GetNumDrivers calls returned 0; version was `0x41804`, pre-init output was
AUTODETECT 0 and driver count was **0**. No driver names/caps calls were reachable
with that count. The real EventSystem release returned 0. This probe did not call
Init, SetOutput, SetDriver, SetSpeakerMode or bank APIs and did not launch the game.
Its process exit 0 qualifies enumeration and release only.

Source contains a genuine disabled-sound path: `GameSoundRuntime::startup(true)`
reaches `System_SetOutput(NOSOUND=2)` before `EventSystem_Init(512,16,null,0)`.
The path still requires real SDK initialization, file callbacks, resources and
configuration; it does not skip banks or supply output. The executable has no
supported selector for this path. The Native settings constructor seeds actual
`+24h=true`; `native_settings_loader.cpp:77` recognizes **SoundEnabled but never
reads its value**, and the settings read view copies that existing flag. The
serializer writes the token, while the archive reader handles volume keys only.
`SoundEnabled 0` and zero volumes therefore cannot select NOSOUND at startup.
The current command-line parser has no no-audio or headless option. `--frames`
limits later loop ticks; normal startup still initializes sound and creates the
window/device. Existing VFS failure qualification modes are separate diagnostics.

Worker Source is pinned at main `898224d3c`, separately from Root's immutable
Source993 freeze: 993 selected inputs, 997 pins and three recorded existing check
passes. Root reported 170 selected whole Core and 10 App objects; final context
COFF review was pending in this assignment. Of ten relevant inputs present in
that freeze, all complete files compare identically after CRLF/LF normalization.
The raw differences in `cmake/startup.cmake` and `src/game_hosts.cpp` are EOL-only;
both raw and normalized hashes and empty complete unified diffs are retained.
No new compile, test, startup activation or final COFF-review claim is made here.

Historical Source581, frozen Source539 control and Source595 attempts retain
their exact commands, executable hashes, options, PIDs, exits and logs. Source595
PID116676 at 00:24:36Z exited 1 before graphics. The real error order remains
Init **61 OUTPUT_INIT**, CreateSound **78 UNINITIALIZED**, GetLength **37
INVALID_PARAM**; `sound/gui/error.fsb` had 2688 bytes, mode 2634 and no bank output.
The unwritten raw length was not consumed. Local retained 4.18 result numbering
was rechecked; newer SDK version labels were not substituted. Earlier successful
smokes and the 03:36 endpoint observation remain historical, separately pinned.

The next normal executable prerequisite is an actual active/default render
endpoint visible to Console session1. A Root-reviewed environment action and
fresh endpoint/SDK/control/selected-build startup checks would be required to
qualify resolution; this receipt changes no devices, services, defaults, configs
or installation. Endpoint absence is not proof of the sole cause of historical
Init61 or a guarantee that later startup succeeds. A new Source selector would
require separate semantic review; silently implementing the ignored token is
not justified. Independent Source reconstruction can continue.

The [complete receipt](../reports/cc12_fmod_audio_endpoint_startup_recheck.json)
retains Source/config/log/DLL/probe inputs, exact commands, complete raw outputs,
provider boundaries and lifetime/failure contracts. Its ignored local replay
script validates frozen hashes, full EOL comparisons, Source queries and DLL
export metadata without live API calls. This packet earns current environment
and SDK enumeration evidence only: no new FMOD init, game startup, graphics,
Native ABI or gameplay credit.

# Current FMOD startup failure

Status: a current Windows audio prerequisite is absent; no fix has been applied.
The installed FMOD 4.18.04 runtime independently reproduces the first application
failure. This packet changes only this document and its JSON report. It performs
no new Native body/data/handler/GPR reads or writes and does not launch the
original game, modify its installation, change services or audio configuration,
or change repository C++, builds or tests.

## Pinned application attempts

Both attempts used the genuine bundled SDK, the same ordinary isolated windowed
settings (options SHA-256
`1a815f6524a92e6db601870b75a5cec0e9ab9f52242663566d9bafe9b746b549`),
and three requested ticks. Their full commands, executable hashes, timestamps,
exit records and log hashes are retained in
`reports/cc12_fmod_current_startup_failure.json`.

| Attempt | PID | Start UTC | Exit | Window / device / Presents |
| --- | ---: | --- | ---: | --- |
| Recorded Source581 build | 110544 | 2026-10-09 23:42:50.625024 | 1 | 0 / 0 / 0 |
| Frozen Source539 control | 73660 | 2026-10-09 23:43:42.570471 | 1 | 0 / 0 / 0 |

The Source581 build passed its three existing checks. Its model and model-base
atexit registration results were both zero before the sound failure. Neither
that build evidence nor the registrations establish current startup success.
The unchanged frozen control produces the same failure chain, so the new model
pools and Comment registration are not required to trigger this failure.

## Exact error and output order

The local established 4.18 result mapping in `config/fmod_ex_types.json` gives:

| Call, in log order | Raw result | 4.18 enum |
| --- | ---: | --- |
| `_FMOD_EventSystem_Init@20` | 61 | `FMOD_ERR_OUTPUT_INIT` |
| `FMOD_System_CreateSound` | 78 | `FMOD_ERR_UNINITIALIZED` |
| `FMOD_Sound_GetLength` | 37 | `FMOD_ERR_INVALID_PARAM` |

Code 37 is invalid parameter; the local invalid-handle code is 36. Newer SDK
result numbering must not be substituted for this established 4.18 mapping.

Both logs then preserve the same diagnostic:

```text
startup failed: FMOD bank raw-length output unavailable: path=sound/gui/error.fsb bytes=2688 mode=2634 create_result=78 length_result=37 bank_returned=0
```

The byte count is the loaded file size, not an FMOD raw-length result.
`sound_resource_asset.cpp` calls CreateSound, checks memory, obtains memory stats,
and calls GetLength with the bank pointer before its existing output guard throws.
It never consumes the unwritten length on this failure. The null bank and later
invalid-parameter result follow the uninitialized-system error; this evidence
does not establish corrupt FSB contents. No length fallback, fabricated output,
success result or sound bypass is proposed.

## Source and installed SDK boundary

Current Source providers are individually pinned in the report. Enabled startup
queries the driver count and scans capabilities from the last driver down to 0,
seeds speaker mode 1, sets the resulting speaker mode, then initializes the event
system with 512 channels, flags `0x10`, null extra driver data and event flags 0.
There is no SetDriver or enabled-path SetOutput call. The local SDK-derived flag
name is `FMOD_INIT_SOFTWARE_HRTF`. Non-memory errors continue through the fragment;
the file callbacks and 3D settings follow initialization.

`game_main.cpp` sets the process directory to the supplied game root before these
providers. With no FMOD override, `selected_library_path` resolves the two DLLs
there. The installed `fmodex.dll` and `fmod_event.dll` both carry file version
4.18.4; their full SHA-256 pins are retained. The diagnostic probe confirms those
actual loaded paths and runtime version `0x00041804`.

## Current Windows and isolated SDK evidence

Windows 11 build 26200, Console session 1, reports Audiosrv,
AudioEndpointBuilder and MMCSS running with automatic start. All eight
Win32_SoundDevice records report Status OK and ConfigManagerErrorCode 0. Four
render endpoint registry records carry DeviceState 1, including HyperX Cloud
Alpha S Game, Realtek Digital Output and Steam streaming endpoints. These are
registry/PnP observations, not proof of live playback availability.

Two read-only live MMDevice default queries return `0x80070490`
(`HRESULT_FROM_WIN32(ERROR_NOT_FOUND)`) for Console, Multimedia and Communications.
The final combined query also returns S_OK from
`EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE)` with **zero live endpoints**.
The report retains the observation time and exact results. A functioning active
render endpoint and a default playback route are presently unavailable to this
process session; healthy services and registry records do not supply them.

One Root-authorized standalone Win32 probe, PID 120852 at 23:50:02.991881 UTC,
loads the actual installed DLLs. It uses the current startup parameters through
EventSystem_Init, with extra read-only SDK queries:

| Probe observation | Result / output |
| --- | --- |
| EventSystem_Create / GetSystemObject | 0 / actual objects returned |
| System_GetVersion | 0 / `0x00041804` |
| System_GetOutput before init | 0 / 0, AUTODETECT |
| System_GetNumDrivers | 0 / **0 drivers** |
| System_SetSpeakerMode | 0 / argument 1, the Source seed |
| EventSystem_Init | **61**, using 512 / `0x10` / null / 0 |
| System_GetOutput after failed init | 0 / 9, local SDK name WASAPI |
| EventSystem_Release | 0; releases its owned actual core system |

Driver-info/caps calls are absent because there are no enumerated drivers.
No output backend or driver is forced. This probe completes with process exit 0;
that means the diagnostic completed, while FMOD initialization itself failed.
It excludes model/base pool startup, Comment, VFS/FSB loading and rendering from
this independent reproduction. It is not a reconstructed-game startup pass or
Native ABI/gameplay proof.

The ignored `local/cc12_fmod_diagnostic/` artifacts retain the source, executable,
stdout, result JSON and build driver; exact source/build text and their hashes
are also embedded in the committed report. Compilation was:

```text
vcvarsall.bat x86
cl /nologo /std:c++17 /EHsc /W4 /MD /O2 /Fe:fmod_probe.exe /Fo:fmod_probe.obj fmod_probe.cpp /link /MANIFEST:EMBED
```

The probe is PE Win32 machine `0x14c` with an embedded `asInvoker` manifest.

## Concrete prerequisite and remaining proof

Restore an actual active Windows playback endpoint and set a functioning default
playback route for the same interactive Console session. This is a Root-reviewed
environment action; this packet does not apply a persistent user setting or
restart a service. No Source behavior change is justified by the present evidence.
The evidence identifies the current missing prerequisite, not the reason Windows
lost the endpoint or a guarantee that the rest of startup will then succeed.

After that action, recheck the live default endpoint and FMOD driver count, then
rerun unchanged frozen Source539 and the recorded Source581 executable with the same genuine
DLLs and settings. Record the real EventSystem_Init result, bank outputs, exit
and window/device/Present summary. This is required before a resolved-startup
claim.

Root's Source534 two-Present run around 22:59 UTC is historical evidence. A
separately checked earlier three-tick smoke at 21:11:44 UTC also had two Presents
and no FMOD errors, with a different executable hash pinned in the report. Neither
earlier run establishes that the current environment or either current executable
can complete startup now. Root subsequently published Source583; this packet
contains no runtime attempt for that later source receipt.

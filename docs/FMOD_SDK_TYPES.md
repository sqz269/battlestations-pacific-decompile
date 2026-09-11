# FMOD Ex SDK types in the Ghidra project

Status: applied to the saved Ghidra project on 2026-09-11 (types and prototypes only; no game
function was renamed). Reproducible with `tools/fmod_sdk_types.py`; the applied data is
`config/fmod_ex_types.json`.

## What the game links

`battlestationspacific.exe` imports 81 C++ member functions and 2 C functions from `fmodex.dll`
and `fmod_event.dll` (FMOD Ex 4.18.04: both DLLs carry version resource 4.18.4 and the runtime
reports 0x41804, see `docs/INSTALLED_SOUND_CONFIGURATION.md`). Ghidra's demangler had already
recovered every import name, but it created the referenced types as empty placeholders
(`FMOD_RESULT` and the other enums with no members, `FMOD_ADVANCEDSETTINGS`,
`FMOD_CREATESOUNDEXINFO`, `FMOD_EVENT_INFO` and `FMOD_VECTOR` as one-byte structs) and its
signatures omitted `this`, because FMOD's Win32 C++ exports are `__stdcall` members that
receive `this` as the first stack argument (mangled `QAG`). Two import stubs, `System::close`
at 00c2dde2 and `Event::getInfo` at 00c2df62, had no function at all, so their callers
decompiled without the call.

FMOD is closed source. The headers are public through the free SDK installers; 4.18.04 itself
is not archived, the closest is the 4.24.16 Programmers API (`fmodapi42416win32-installer.exe`
in archive.org item `fmodapi44452nacl.tar`, an NSIS installer that 7-Zip extracts). The SDK is
kept out of the repository; only the derived JSON is committed.

## Version evidence (4.24.16 headers against the 4.18.04 game)

| Item | Evidence | Verdict |
| --- | --- | --- |
| `FMOD_ADVANCEDSETTINGS` (0x44) | game zeroes a 44h block and stores `cbsize = 44h` before `System::getAdvancedSettings` (`docs/SOUND_CONFIGURATION.md`, step 4); SDK 4.24.16 struct is 17 members = 0x44 with the C alignment; `revision.txt` last changed it in 4.17.00 | identical layout |
| `FMOD_CREATESOUNDEXINFO` (0x6c) | `FUN_00a823f0` memsets 6Ch and stores `cbsize = 6Ch` before `System::createSound`; SDK struct is 27 members = 0x6c; last change 4.09.04 | identical layout |
| `FMOD_EVENT_INFO` (0x34) | SDK size; last change 4.13.00; the game's only reader is `FUN_00a828b0` via `Event::getInfo` | provisional (no size store found in the game) |
| `FMOD_RESULT` | the game inlines `FMOD_ErrorString()` six times (jump tables at 00a7a17c and five identical copies, 89 cases); its case strings were matched to the 4.24.16 `fmod_errors.h` names | game-validated 4.18 numbering: 89 codes, `FMOD_ERR_MEMORY = 43` (the SDK's 44); six codes added after 4.18 are absent (`FMOD_ERR_INVALID_POSITION`, `FMOD_ERR_PRELOADED`, `FMOD_ERR_EVENT_NEEDSSIMPLE`, `FMOD_ERR_EVENT_GUIDCONFLICT`, `FMOD_ERR_EVENT_ALREADY_LOADED`, `FMOD_ERR_MUSIC_UNINITIALIZED`); two messages were reworded and matched by prefix (`FMOD_ERR_OUTPUT_NOHARDWARE`, `FMOD_ERR_EVENT_MAXSTREAMS`) |
| other enums and flag groups | taken from 4.24.16; `revision.txt` 4.19.00 to 4.24.16 records no member insertions for them, but that history also failed to list the six result codes above | provisional; values the game compares against should be checked against the exe when they matter |
| method parameter names and types | `fmod.hpp` and `fmod_event.hpp` 4.24.16; the Event API classes are pure virtual there but the game imports the 4.18 non-virtual wrappers with the same parameter lists | names are SDK names, not recovered symbols |

The exe contains no compiled-in `FMOD_VERSION` check (the one byte match for 0x41804 is a CALL
displacement), so the version pin rests on the DLL resources and the runtime report.

## What was written to Ghidra

`python tools/fmod_sdk_types.py apply` (idempotent, under the Ghidra write lock, previous values
recorded in `local/fmod-types-<stamp>.json`):

- opaque class placeholders `System`, `Event`, `EventSystem`, `SoundGroup` in `/Demangler/FMOD`
  next to the demangler's own `Sound`, `Channel`, `ChannelGroup`, `DSP`, `DSPConnection`,
  `EventParameter`;
- enums replacing the empty placeholders: `FMOD_RESULT` (4.18 numbering), `FMOD_OUTPUTTYPE`,
  `FMOD_SPEAKERMODE`, `FMOD_SPEAKER`, `FMOD_SOUND_TYPE`, `FMOD_SOUND_FORMAT`, `FMOD_OPENSTATE`,
  `FMOD_DSP_TYPE`, `FMOD_CHANNELINDEX`, `FMOD_EVENT_PITCHUNITS`, `FMOD_SPEAKERMAPTYPE`, plus the
  flag groups `FMOD_MODE`, `FMOD_INITFLAGS`, `FMOD_TIMEUNIT`, `FMOD_CAPS`, `FMOD_EVENT_MODE`,
  `FMOD_EVENT_STATE`, `FMOD_EVENT_INITFLAGS` as enums so the decompiler prints
  `FMOD_OPENMEMORY|FMOD_SOFTWARE|FMOD_3D` instead of a constant;
- the seven callback signatures (`FMOD_FILE_*CALLBACK`, `FMOD_SOUND_*CALLBACK`) as function
  definitions with pointer typedefs under the SDK names;
- the four structs filled in place (so every existing pointer reference follows), padded to
  the C alignment where the bridge builds unaligned structs (`FMOD_ADVANCEDSETTINGS` +0x42);
- a `__stdcall` prototype with an explicit `this` and the SDK parameter names on all 83 import
  thunks, for example
  `FMOD_RESULT __stdcall createSound(System * this, char * name_or_data, FMOD_MODE mode, FMOD_CREATESOUNDEXINFO * exinfo, Sound * * sound)`;
- functions defined on the two missing thunks.

Result on the callers: `FUN_00a823f0` now reads
`FMOD_OPENMEMORY|FMOD_CREATECOMPRESSEDSAMPLE|FMOD_SOFTWARE|FMOD_3D|FMOD_LOOP_NORMAL` and
`if (FVar4 == FMOD_ERR_MEMORY)`; every `FMOD::System::*` call shows the `System *` in the first
slot instead of shifting the arguments.

## Known limitation: struct field spelling

The GhidraMCP bridge rewrites every struct field it creates or renames into Hungarian
notation from the field's type name (`cbsize` became `nCbsize`, `debugLogFilename` became
`pDebugLogFilename`; enum- and typedef-typed fields such as `format` and `initialseekpostype`
kept their names). The bridge has no switch for this and script execution is disabled on it.
The names are still recognisable, and `tools/ghidra_scripts/RestoreFmodFieldNames.java`
restores the SDK spellings when run from Ghidra's Script Manager (add `tools/ghidra_scripts` to
the script directories, run it, point it at `config/fmod_ex_types.json`, save). Until that runs,
read `nCbsize` as `cbsize`.

Two SDK struct types have no definition in Ghidra and are typed `void *` in the field lists:
`FMOD_EVENT_WAVEBANKINFO` and `FMOD_GUID` (`FMOD_EVENT_INFO.wavebankinfo`, `.guid`).

## Refreshing and extending

- Re-run `python tools/fmod_sdk_types.py verify` after a new exe or Ghidra image; it re-derives
  the error table and the thunk map and compares them with the JSON.
- `apply` skips enums that already have members and structs that already have fields, so it
  can be re-run after a Ghidra restore; use `--dry-run` first.
- New imports would need their declarations in the JSON (`extract` keeps only the methods the
  exe imports).
- The repository's `SoundFmodAdvancedSettings` (`include/bsp/sound_configuration.hpp`) is still
  the opaque 17-word block the probe passes through; its words can now be named from
  `config/fmod_ex_types.json` when that file's owner next touches it.

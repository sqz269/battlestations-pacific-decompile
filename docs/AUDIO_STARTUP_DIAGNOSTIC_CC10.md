# Pre-window FMOD startup diagnostic (CC10)

This packet investigates the retained `EventSystem_Init` result 61 before
window creation; it does not change the game's audio configuration or add a
fallback. At integrated revision `648081336`, the application log records
`_FMOD_EventSystem_Init@20` result 61, followed by
`FMOD_System_CreateSound` result 78 and `FMOD_Sound_GetLength` result 37 for
the 2,688-byte `sound/gui/error.fsb` bank (mode 2634). The comparison build
`9dcca1289` without early shared XInput lifetime reproduced those results.
Both application runs exited before window creation; sound services and
queried PnP devices were healthy, which did not prove FMOD output readiness.

The installed `fmodex.dll` and `fmod_event.dll` each have file version
4.18.4. Their SHA-256 values are respectively
`31e7451aef6115b0aec353e4e508ff9f0fc14ea3a8957e5ddb805ede911700e8`
and `4b6ae7ba8d3da780c23abb5e72a65ea50e5348a60a1b4aef43196e09ae771890`.
The installed game executable hash is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`;
the fallback bank hash is
`247374fbb9cfcb9e3867f054c72e0663f346ee858ecc1ea1ffa4c4fff32fb555`.
The DLL version query in the direct probe returned `0x41804`. Local
`tools/fmod_sdk_types.py verify` matched all six of the game's 89-case
inlined FMOD error tables and all 83 imports to `config/fmod_ex_types.json`.
For this executable, results 61, 78 and 37 are respectively
`FMOD_ERR_OUTPUT_INIT`, `FMOD_ERR_UNINITIALIZED` and
`FMOD_ERR_INVALID_PARAM`. Those are game-version mappings, not inferred
from the newer 4.24.16 SDK numbering alone.

`00A88770` creates the EventSystem at `00A8881E`, obtains its System via
table slot 7 at `00A88844`, and on the enabled-sound path enumerates
drivers from last to first, retaining driver 0's control-panel speaker
mode. It calls `setSpeakerMode`, then EventSystem table slot 0 at
`00A88931` with `(512, 0x10, nullptr, 0)`. The disabled-sound branch
instead calls `setOutput(2)` before the same init. The user's current
options file has `SoundEnabled 1`; current source passes that to the
enabled-sound path. Native code checks only result `0x2b` (memory) after
these calls and continues on result 61. Later, `00A823F0` asks
`createSound` at `00A82492` and `getLength(RAWBYTES=8)` at `00A824E5`;
the source host guard rejects unavailable raw length rather than reading
an unwritten output. The 78/37 results are downstream failures, not
independent proof of a malformed FSB.

A local Win32 `/MD` probe with an embedded manifest loaded the same two
installed DLLs by absolute path and used the same C exports/signatures and
init arguments as `FmodConfigurationLibrary`. With sound enabled it saw
eight drivers; every `GetDriverCaps` returned zero, driver 0 selected
speaker mode 2, and `SetSpeakerMode` and `EventSystem_Init` returned zero.
`GetOutput` then returned numeric output 9 and driver 0. An independent
no-sound process called `setOutput(2)` and also initialized successfully.
The probe executable hash is
`ca43d0e694bfbb44d27a556fb8fa474e5d72db3035dcff686cdb9203c0de5a44`;
its ignored local logs are `local/output/audio_probe_enabled.log` and
`local/output/audio_probe_nosound.log`. The direct probe has no preceding
game startup, VFS, Lua, renderer or input state; its success establishes
only that the installed FMOD runtime can initialize the output in a fresh
process under this session. It cannot identify the earlier application's
cause or exclude transient output conditions.

The fresh `f155d3379` application was built with strict MSVC Win32 and both
existing CTests passed. Its `bsp_game.exe` SHA-256 is
`d16299bdcdf4b633259f6aa780afff172ba0371d8c83e811617629efa7d4c688`.
One `tools/run_game.ps1 -Log local/cc10-audio-baseline.log -- --frames 2`
invocation waited for a normal wrapper slot, acquired slot 1, and exited 0.
The log says `sound startup before window: enabled=1`, 132 FMOD calls and
zero FMOD errors; it then created a window and D3D device, presented one
frame, skipped one, and finished the finite loop. The options file was
loaded, with graphics resolution 2560x1440; the older failing run logged
640x480. The current file contains `SoundEnabled 1`, but no retained
snapshot proves the earlier file's complete contents. No restart was
issued while the wrapper was queued. A targeted revision diff from
`648081336` to `f155d3379` found no changes in the direct audio startup,
FMOD library, sound runtime/resource, or `game_hosts.cpp` files reviewed
here. Other application changes and the altered options environment keep
that comparison from isolating a cause.

The historical result 61 is an output-initialization failure, with 78/37
following the source's native-compatible continue-on-error schedule. The
same installed DLLs can initialize in a standalone process and this fresh
application build now completes sound startup. These observations do not
identify whether the earlier failure came from transient output state or
another application/configuration condition; no audio source fix is
supported. The one-frame finite application run is not audible playback,
full GUI, gameplay or original-game validation. No Windows audio settings,
installed files, or peer processes were changed.

# Current Source title startup observation

The current MSVC Win32 Source executable completes three title-loop ticks with
the explicitly selected, retained Microsoft XLive runtime: two frames presented,
one skipped, device HRESULT zero and process exit zero. The 1920x1080 screenshot
shows the main-menu banner, Pacific map and login/press-enter prompt. No input,
mission or combat was exercised. The run reports 120 concrete and 44
unimplemented host methods; full faithful startup/gameplay acceptance remains
open. Installed assets include AlterBSP login text, so this is not an original
content visual-parity claim.

The normal build passed the existing reconstructed math, native math
differential and tooling checks. It includes the new real settings float at
+34h, its direct process-retained reference accessor, and the fixed actual World
parent-header initialization/reverse-clear composition. Those new function
roots are emitted in exact core library members but absent from the game map.
This title run does not validate their application callers or establish full
Game/World/GlobalConfig construction.

The initial direct launch selected the installed game-specific XLive shim and
exited with C0000005 immediately after settings loading. Its physical DLL hash
matches the earlier documented shim that patches host RVA640F5E, beyond the
current reconstructed image. The current AV PC was not captured. A separate
debugger attempt stopped at C0000008 in NtClose while that shim loaded; it is
not an AV replay. Existing investigation and unchanged DLL bytes strongly
support recurrence, without a new fault-PC claim.

The successful launch selects `local/gfwl-private-runtime/xlive.dll` and
preloads its matching `msidcrl40.dll` through the existing `--xlive-dll` and
`--xlive-dependency` options. All four retained runtime files were hash-checked
against the earlier Microsoft runtime receipt. No game DLL or original PE was
patched or replaced. Direct launch's default still selects the incompatible
game-local shim in this installation; the explicit runtime route is required.

Each run checked the native `MidwayThreadMutex` and existing game processes
before launch. Distinct ignored output directories retain argument vectors,
executables/maps, logs, isolated options files and the successful screenshot.
The settings personal root is isolated; the save-storage constructor still
selected the existing Documents save path. Its folder preimage was not
captured, so unchanged save-directory contents are not claimed.

See the [startup receipt](../reports/cc12_current_startup_observation.json),
[independent sound-boundary audit](../reports/cc12_sound_startup_crash_frontier.json)
and [Source build review](../reports/cc12_settings_world_source_primary_review.json).
Root's ignored `local/cc12_current_startup_observation/run_genuine.py` is the
retained hash-checked three-frame launcher for this checkout; it fails if the
private runtime identities change.

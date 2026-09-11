# Retained settings startup and process binding

Addresses: 008d8190, 008d6170, 008d7bc0, 008d4ea0, 008d4df0, 00cd2d80,
008d7710, 008d56c0, 00b295c0, 00b200b0, 00b200c0.

`load_game_settings_008d8190` now updates the retained `GameSettingsBlock`.
The former return-by-value partial object discarded constructor state and had
no selected language index. `GameStartupHost` initializes the actual projected
object once through `initialize_static_game_settings_00cd2d80`, then retains it
and the language/capability binding for its lifetime. Other fields survive loads.
This is a new C++ ownership interface, not the native static-object or ABI layout.

## Native sequence and corrections

Native `008d8190..008d88b8` is ECX-this, no stack arguments, RET. It first builds
the language catalog, copies renderer resolution pairs, seeds shader model from
the renderer, and opens the options path in CRT text mode. The missing-file path
sets fullscreen and desktop dimensions, finds the first matching resolution,
optionally selects registry language, then calls `008d6170` at `008d878a`.
That call writes the current settings before the common capability tail; it does
not derive hardware defaults. Output-open failure remains nonfatal in the
existing writer. The real binding now invokes that writer with the actual object
and language table.

Language selection uses the recovered native string rule and resets index zero
on a miss. The previous claim that a miss retained the old index is superseded
by `SETTINGS_INITIAL_STATE.md`. The host-only language string is derived from the
selected entry, never used as a replacement for its index.

The options reader reuses the corrected native scene scanner with delimiter
`;`. Fullscreen, HiResShadow and NoLOD use integer-not-zero. VSync, Clouds,
Foliage, Shadow, Reflection and Firewall use the strict native boolean reader.
Resolution consumes two integers, falls back to 640x480 if unsupported, then
retains the last matching index. Failed typed reads preserve the scanner's cache
and recovery state. Unknown tokens advance once. `SoundEnabled` consumes only
its key; its following token is processed in the next loop. `HardwareReported`
has no handler here. Neither changes the retained audio or hardware-report field.
The binding's old token-recapitalization workaround is removed.

Assembly `008d878f..008d887c` establishes the shared tail:

1. Repair a shader model below one, then clamp against the native renderer
   ceiling; invoke `00b200c0`, whose entire recovered body is RET4.
2. Read renderer virtual+104's record+28. It holds the low word of
   `PixelShaderVersion`, not a constant-buffer limit. Below 0x200, clear shadow
   bytes+84/+85 and old-film dword+90.
3. Rebuild AA samples with format0x15 when byte+8c is zero, otherwise format0x71.
   `00b295c0` enumerates AA support, not post-effect selection.
4. Copy the result, select the last exact sample match, clamp an index at/above
   count to count-1, then index unconditionally. Empty tables and negative final
   indices are rejected by the host instead of reproducing invalid native reads.

See `SETTINGS_CAPABILITIES.md` for original D3D9 query arguments, sorting and the
game's shader ceiling of one or two, and `OPTIONS_TOKEN_READER.md` for typed-read
recovery and malformed-input limits.

## Runtime owner and validation boundary

The binding holds an actual IDirect3D9 interface and delegates capability queries
to the recovered operations. Its VFS language source references the run's mounted
provider manager, search registrations, content-suffix vector and hints owner.
Their lifetimes extend past settings loading. Profile/DLC population and the full
renderer resource owner remain separate integration work.

The native renderer shares its API at+1990 with device creation. The current
process still creates a separate API in `GameDeviceHost::create`; this batch
establishes the settings query behavior and lifetime, not that shared renderer
ownership. The full constructor's intervening adapter-identifier/NVIDIA check
also remains outside the settings projection.

`--settings-personal-root <dir>` supplies an explicit personal-directory override
for isolated checks. It is resolved before `--game-root` changes the working
directory. Normal runs use CSIDL_PERSONAL. Both startup language lookup and the
settings read/write binding use the override. CRT read errors throw; only bytes
actually read after text-mode translation are passed to the scanner, avoiding the
original's indeterminate unused buffer tail. Query failures do not fabricate caps.

Validation and saved annotation evidence are recorded in
`reports/settings_startup_owner.json`. Build/fixture/process validation does not
establish native object ABI compatibility or gameplay completion.

The combined strict Win32 build and both existing CTests pass. An ignored
fixture checks retained sentinel fields, case-insensitive language selection,
typed token failures, exact single-token diagnostics, both resolution-index
rules, AA selection and the pre-tail writer snapshot. Its concrete binding run
loads one installed language descriptor, queries 26 resolutions and four AA
levels, and writes/reloads options only in an isolated personal directory.

The rebuilt executable then ran with that isolated windowed configuration:
640x480 device, 60 frames presented, clean loop completion, exit0, and three VFS
probes resolved. The original installed executable and live personal options
retained their hashes, sizes and modification times. This is startup/process
validation with the milestone clear/present path, not gameplay validation. Input,
GUI, world and scene owners still have unimplemented bindings in the run log.

### Follow-up: shared renderer and startup consumers

[RUNTIME_STARTUP_OWNERS.md](RUNTIME_STARTUP_OWNERS.md) supersedes this packet's
two-API ownership boundary. GameStartupHost now owns one IDirect3D9 interface
and retained renderer parameter/capability state; GameSettingsBinding and
GameDeviceHost borrow them. The complete capability query sequence is in
RENDERER_CAPABILITIES.md. Input script startup runs before options loading, and
the selected catalog language feeds the retained locale tables after device setup.
The earlier packet's validation remains evidence for its recorded source commit.

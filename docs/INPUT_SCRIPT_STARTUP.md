# Input settings script startup

Addresses: 006a7be0, 006ab6b0

`InputScriptStartup` composes the existing VFS, Lua 5.1.1 owner/runtime and input schema
readers into the normal startup path. It owns an `InputSettings` and the persistent
ControlPresets interpreter. Construction loads the tables immediately; the application
initializer's later call to `load_data_tables_006a7be0()` returns without reopening Lua or
changing settings. Descriptive names remain hypotheses, not recovered symbols.

## Native order and lifetime

| Address | Proven behavior |
| --- | --- |
| `006ab6da..006ab7c6` | Set vtable and construct five tree heads and three vectors in the native 0x540-byte object. |
| `006ab7c9..006ab7e3` | Construct Lua owner at `this+0x78`, clear bytes `+4` and `+5`, and call `006a7be0`. |
| `006a7c09..006a7c25` | Return if byte `+4` is nonzero; otherwise set it **before** opening persistent Lua with mask 1. |
| `006a7c2a..006a7c47` | Execute `Scripts/datatables/ControlPresets.lua` with flag zero on the persistent owner. This routine does not parse its tables. |
| `006a7c74..006a7cb7` | Construct a separate stack owner, open mask 1, and execute `Scripts\datatables\KeyboardSetup.lua` with flag zero. |
| `006a7ce2..006a947a` | Clear only the device map and device-order vector; read KeyboardSetup, InputNames, Conflicts and finally DEVINPUTS through the existing schema readers. |
| `006a94a8..006a9789` | Execute `Scripts\datatables\ControllerInputNames.lua` with flag zero in the **same temporary state**, then read ControllerInputNames. |
| `006a978b..006a9829` | Release temporary Lua references and destroy the temporary owner. The persistent presets owner survives. |
| `006a982e..006a9843` | Restore registers/SEH and return, also the early-guard target. |

Both original functions receive `this` in ECX and use plain RET. Constructor `006ab6b0`
returns `this` in EAX at `006ab7ed`; its exclusive end is `006ab7fc`. Loader `006a7be0`
has no recovered return value; its exclusive end is `006a9844`. Assembly was used for
register inputs, stack owner identity, library/obfuscation arguments, string addresses,
guard order and destruction. The stored bodies contain their terminal RETs. The flow
audit reports only two non-CALL alignment gaps in the loader and no constructor gaps;
no repair was required.

The guard lives in `InputScriptStartup::data_tables_started_`, separate from
`InputSettings::runtime_settings_loaded` (native `+5`, set later by archive loading).
It is never reset by the loader. `settings()` returns the owned model for subsequent
runtime/archive use. `control_presets_lua()` borrows the persistent state; callers must
retire Lua references before destroying its owner and must not close the state themselves.

`VfsLuaScriptFiles::owner_environment` supplies the cached installed fundamentals, platform
globals and real DoFile callback. Both script states use the existing mask-1 behavior:
Lua base plus bootstrap globals/fundamentals; unrelated libraries remain closed.
`LuaScriptRuntime::run_file` preserves base-before-override execution and nested DoFile
behavior. The exact mixed path separators are retained because `00bdef90` splits names
using only forward slashes. The runtime/files/VFS/global inputs must outlive this owner;
access is serialized.

## Boundaries

This is a new C++ composition, not the native object layout, singleton getter `005547d0`,
allocator, vtable, checked containers or binary replacement. The constructor record is a
fragment: unused native trees at `+0x54` and `+0x6c` are not invented. Existing
`load_keyboard_setup` and `load_controller_input_names` retain their documented schema
and malformed-input boundaries; no duplicate parser is introduced. Native schema faults
and Lua panic paths become exceptions through those existing host boundaries. The native
Lua wrapper is constructed before the flags are cleared; the C++ owner allocation and
environment capture happen after setting the guard, before opening Lua. This changes
allocation/failure mechanics, not the established normal script order.

No profile/preset application policy, device construction, global singleton lifetime,
window initialization or game-loop wiring is claimed by this packet. The persistent Lua
state is available for those consumers instead of being discarded after table parsing.

## Validation

MSVC Win32 Release build and both existing tests pass. All eight existing native-math
seeds match disk bytes; those tests do not validate the new input path against native code.

One ignored fixture (`local/input_script_startup_fixture.cpp`) mounts the original game
directory read-only through `VfsProviderManager` and adds an in-memory FileStore for
observable script suffixes. It executes installed ControlPresets, KeyboardSetup,
ControllerInputNames and the nested `inputs.lua` through VFS/DoFile. It reads four input
categories, 16 input-name records and 12 installed controller maps, plus one fixture map.
Suffix assertions verify independent persistent/temporary globals and shared temporary
state. A controller suffix mutates KeyboardSetup/DEVINPUTS after parsing; the C++ settings
retain the values parsed earlier. A repeated loader call preserves runtime `+5`, a runtime
edit, the same interpreter and the persistent script's MULTRET result without further
file reads. Lua userdata finalizers call real DoFile into the observed VFS and establish
exactly one temporary close before construction returns and one persistent close at
destruction. The original fundamentals bytes are augmented only in the fixture FileStore
to install those observers; no installation file is modified.

Logs and installed source hashes are recorded in `reports/input_script_startup.json`.
No new permanent test or test framework was added. This proves host composition and
installed-script compatibility; native differential behavior, ABI compatibility and
gameplay validation remain unproven.

### Application binding

[RUNTIME_STARTUP_OWNERS.md](RUNTIME_STARTUP_OWNERS.md) connects this composition
to the executable before settings loading. GameScriptHost owns the globals,
VFS file adapter, Lua runtime and input owner for the application lifetime.
LUA_RUNTIME_GLOBALS.md establishes the initial false/unset platform globals
from image zero-fill and write ordering. Device polling, profile application
and native singleton lifetime remain separate integration requirements.

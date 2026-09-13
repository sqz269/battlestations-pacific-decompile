# Complete raw input configuration loader

Addresses: 00698A10

`load_native_input_configuration_00698a10` supplies the complete normal
4462-byte schedule through the original RET at 00699B7D. It composes the
existing script, modifier and action parser with the final rebind, zero-delta
update, settings application, loaded-byte write and Lua cleanup. The earlier
prefix APIs remain bounded as documented in `NATIVE_INPUT_CONFIGURATION_LOAD.md`,
`NATIVE_INPUT_CONFIGURATION_MODIFIERS.md` and `NATIVE_INPUT_CONFIGURATION_PARSE.md`.

The original receives the embedded 524h configuration in ECX, has no stack
arguments and returns with RET. Its descriptive name remains a hypothesis, not
a recovered symbol. The new source ABI accepts explicit services and does not
provide a binary replacement for the original entry.

## Final sequence

The existing prefix produces actual action records and keeps globals, Inputs,
key and value in four stable-address Lua objects. Those objects remain alive
through all of the following calls:

| Site | Operation |
| --- | --- |
| 00699AD8 | Resolve the current action owner through 004BEC00 |
| 00699ADF | Rebind all actions through A922A0 |
| 00699AE4..00699AE7 | Produce positive zero with FLDZ and spill it as float |
| 00699AEA | Resolve the action owner again |
| 00699AF1 | Update that owner through A92C40 with the captured zero |
| 00699AF6 | Read the current F88A30 byte after the complete update |
| 00699B01, 00699B08 | If zero, get settings through 005547D0 and preserve defaults through 006AB820 |
| 00699B14 | Set configuration byte +520 to one |
| 00699B23, 37, 4B, 62 | Destroy value, key, Inputs and globals, in that order |

The source uses the actual rebind and action-update implementations. The update
may rebind again after observing the backend's dirty byte, poll actions, update
listeners and invoke the current post-tick callback. Its zero delta does not
turn it into a no-op. The settings decision therefore reads the live flag only
after the entire update and its callbacks return.

The settings path uses the complete raw lazy getter from
`NATIVE_INPUT_SETTINGS_LIFETIME.md` and default preservation from
`NATIVE_INPUT_SETTINGS_DEFAULTS.md`. It passes actual settings and action
allocations. No projected `InputSettings` or `InputTickState` replaces them.
The full getter can construct settings and execute their scripts at this point.

## Ownership and service requirements

Parsing failures retain the existing prefix cleanup. After prefix success,
the continuation owns all four Lua objects. A later exception releases their
remaining lifetimes in reverse order while preserving prior action/settings
mutations. The loaded byte is written only after rebind, update and conditional
settings calls return. Earlier values of that byte remain unchanged on a
failure before the write. Normal cleanup lowers its remaining ownership before
each destructor call, matching the native state writes 10h, 0Fh, 3 and -1.

The contexts must share the application's actual action, backend, settings and
manager publication cells and compatible record services. Device, timing,
callback and settings-container providers remain required. This composition
does not create another global, owner, default provider or virtual dispatcher.
Application setup still has to bind these services at the actual game stage.

## Verification

The strict MSVC Win32 build and both existing CTests pass. Eight seed spans
match disk and Ghidra. All 183 direct CALL rows in the full loader pass the
live function-ownership and callee audit. The complete 4462-byte loader capture
matches the installed image, and the reused native helper captures retain
their earlier evidence. `reports/native_input_configuration_runtime.json`
records the exact source/archive validation and all limits.

One ignored manifested differential fixture extends the prior action-parser
fixture to execute the original loader through its real return, without a
synthetic prefix return. It compares 106,214 words against the new source:
actual action/binding/modifier records, populated settings trees/vectors and
defined descriptor/string/float values. Both runs use the same previously
verified concrete Lua, action, tick, settings and lifetime routines, with
captured native library container providers. This is a composition comparison,
not an independent revalidation of every transitive callee.

The fixture performs three complete loads per side, retaining the established
repeated-index/nil-termination/modifier/inversion case. Settings scripts execute
once per side through the real lazy getter. A fixture callback changes F88A30
from one to zero during the third update. The resulting reset of the actual
key1 default-binding count proves the loader takes the post-update settings
branch. Empty physical-device slots still execute the source backend update
schedule; no physical device method is reached. A separate source-only
post-tick exception verifies unchanged byte +520, an empty Lua stack, zero
tracked references and no pooled-string leak. Finally the raw manager drains
the populated settings through its CF81CC deletion binding.

The fixture supplies installed fundamentals.lua, Inputs.lua, ControlPresets.lua,
KeyboardSetup.lua and ControllerInputNames.lua through isolated streams. The
existing third Inputs test mutates the loaded table to exercise observable
lookups, and the post-tick callback is an explicit fixture service, not the
production game callback. Configuration streams close 3/3 on the native side
and 4/4 on the source side including the exception case; each side closes 4/4
settings streams. All pooled strings are released. Descriptor scratch padding
and opaque native storage remain outside the comparison as previously documented.

Original FH3/private-stack aliases, malformed storage, hardware faults,
asynchronous mutation, complete application wiring and gameplay remain
unvalidated. The settings destructor's separately documented saved Ghidra-tail
ownership issue is unchanged. No running game was accessed and no agents were
dispatched.

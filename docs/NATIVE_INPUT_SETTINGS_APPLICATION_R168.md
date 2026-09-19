# Raw input settings in application startup (R168)

Addresses: `0073D410` (calls at `0073DA94` / `0073DA9B`), `005547D0`,
`006AB6B0`, `006A7BE0`, `006AA460`, `006AB800`.

## Application change

`GameScriptHost` now binds the existing complete raw input-settings runtime to
the application's existing VFS, Lua fundamentals, pooled strings, and singleton
manager. It no longer constructs the projected `InputScriptStartup` table owner.
Its file/runtime adapters remain for sound and other consumers of those services.
Application summaries read the actual native container sizes.

`GameNativeInputSettingsApplication` owns stable source contexts and an explicit
phase; the raw 540h allocation belongs to the shared singleton manager. The
application retains this source object before calling the getter. It installs
the CF81CC deletion binding before any native registration, executes under the
existing `NativeLuaServiceBindings::Activation`, and performs the two verified
parent calls:

1. `0073DA94 -> 005547D0`: construct/load the actual input singleton, publish
   it in the existing E198E8 cell, and register with the actual 01090AA0 manager.
2. `0073DA99 MOV ECX,EAX; 0073DA9B -> 006A7BE0`: pass the captured getter result
   to the explicit table load. The constructor has already set byte4, so this
   call preserves the same persistent interpreter and publication.

The next native instruction loads the F88980 settings receiver before the
`008D8190` call. This packet supplies its input-owner prerequisite; migration
of that separate BCh settings block remains open.

## Ownership and source policies

The source wrapper borrows the same `GameSingletonHost` publication cells and
deletion table already used by application input/backend services. Its concrete
`NativeInputKeyboardStorage` and `GameInputSettingsRuntime` borrow the VFS's
`ActualNativeStringPoolStorage`, the current Lua bootstrap, and the current
raw Lua file/override callbacks. It creates no second pool or VFS manager.

The application keeps all these providers alive through the raw manager drain.
The existing CF81CC dispatch reaches `006AB800 -> 006AA460`, closing the actual
persistent Lua owner and destroying native containers. After the drain, while
the singleton host still exists, the wrapper checks that the publication is
null and retires its deletion binding. The script/Lua/VFS providers are destroyed
later. An interrupted load retains the application graph through process exit;
it does not invent cleanup for partially registered native state.

Constants come from the already verified read-only image mapping:
`00D7A24C = 3F800000` and `00CF7FE8 = 322BCC77`. SSE2 conversion uses the host's
CPU/OS capability service, as the other application input bindings do.

The three `GameInputSettingsStackPolicy` words are explicitly zero for this
application route. Those are chosen source preimages, not recovered native
stack values. The descriptor and preset flag high bytes originate in distinct
native frames; the current table API deliberately uses the same chosen seed
for both and logs that policy. Opaque vector word0 remains governed by the
existing source behavior. See `NATIVE_INPUT_PREIMAGE_PROVENANCE_AY.md`; this
packet does not claim native stack-byte identity or universal invisibility of
those choices to later keyboard application.

## Evidence and validation

The local collector verifies the project/program, containing Ghidra bodies,
three adjacent startup CALLs, and both constant words against the exact PE.
The parent input call sequence is 12 bytes; the recorded 22-byte window also
includes the following settings call as context. All three direct calls are
mechanically checked against saved function membership. Existing function names
are retained and application-binding evidence is appended in Ghidra.

Strict MSVC Win32 compilation and all three existing CTests pass. No permanent
test cases or framework were added. A focused component executable links the
actual game host objects and uses canonical data/Lua startup plus the real
mounted VFS. It loads 4 devices, 16 input names and 12 controller names, preserves
the guarded interpreter/publication, then drains the shared manager and retires
the input binding. This component does not create a game window or renderer.

The ordinary application launch was **not run**. Its serialized launcher
waited 900 seconds, then stopped while the peer's live processes 2892/40484 and
`cc8-ai-squadron` lock remained active. The full application check remains open.
Its prepared script uses a workspace-local options directory and can be rerun
when the shared runtime slot is free. The report and receipt bind source and
component artifacts, executable/library hashes, runtime-gate evidence and the
tested integration commit; component success is not ordinary startup proof.

## Remaining work

This is raw table/singleton startup and teardown. It does not establish actual
game+3Ch input-configuration ownership, native keyboard/default-binding
application, action callback/deadline ownership, original ABI/FH3/SEH, or
gameplay parity. The BCh settings process owner still needs to share the input
publication safely across its CRT destruction order, and the ordinary options
loader remains projected until that ownership is composed.

The earlier integration-gap audit is historical: raw VFS/Lua services and the
input-table application binding are now present. Its separate game
configuration and keyboard-application requirements still apply.

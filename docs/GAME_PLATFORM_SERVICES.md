# Application platform load services

Addresses: 00BECA40, 00BECB20, 00A409F0 (existing recovered bodies, reused).
`GamePlatformServices` is a new application adapter, not a native object or ABI.

| Routine | Coverage in this packet | Existing provider |
| --- | --- | --- |
| BECA40 | complete adapter to existing normal reset sequence and its documented valid-backend domain | `reset_focus_input_00beca40` in `input_focus_reset.cpp` |
| BECB20 | complete load-service composition of existing cursor policy, including the first absent-online guard | `GameSoundLoadEvents` and `update_platform_cursor_focus_00becb20` |
| A409F0 | complete current-owner dispatch into existing online pump | `pump_xlive_system_00a409f0` |

Native bodies were inspected again, not duplicated or relabeled as newly
reconstructed. Invalid native pointers/checked-vector trap continuations remain
outside the existing typed valid-storage domain. Later input/action/online owner
construction is **not implemented by this adapter**. The native field/ABI and
individual algorithm evidence remains in `INPUT_FOCUS_RESET.md`,
`PLATFORM_CURSOR.md`, and `XLIVE_SYSTEM_PUMP.md`.

## Actual startup ordering

Verified live against `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.
All Ghidra operations were read only through the CLI, whose client verifies the
target before each live batch. Full capped data-reference queries returned 191
F8ABE8 references and 93 F8BBF4 references without truncation:

| Publication | Initial image bytes | Only nonzero write | Only zero write | Sole constructor chain |
| --- | --- | --- | --- | --- |
| F8ABE8 online owner | `00 00 00 00` | A3F586 | A3F639 | 73DC7C -> A40DF0 -> A3F530 |
| F8BBF4 input backend | `00 00 00 00` | A908F6 | A909A9 | 73DD8E -> A982D0 -> A91570 -> A908A0 |

The current call graph has no earlier lazy producer for either publication.
The normal startup order in 0073D410 is:

1. Sound constructor A88770 at **73DAFD**, then dialog constructor A79230 at
   **73DB2B**.
2. Platform/window virtual+4 at **73DC25**; ADD ESP,2C at73DC2B accounts for
   eleven stack words. Older references to73DC0F as this call are inaccurate.
3. Online constructor A40DF0 at **73DC7C**, two callback arguments, native RET8.
4. Input backend constructor A982D0 at **73DD8E**, no stack arguments. The caller
   then sets current backend+D8 to4B4630 and calls A900F0 at73DDAF.

Therefore the BECB20 load call during initial sound construction legitimately
returns on **its first F8ABE8-null guard**, before input, mouse, lazy actions,
online pumping, or ShowCursor. This is a true lifecycle state, not permission to
replace later live owners with null or reorder their construction before sound.

GameScriptHost's InputScriptStartup is settings singleton006AB6B0 and owns Lua/
settings, not F8BBF4. The private InputTickState in game_hosts_mission_frame.cpp is an unbound
projection, not proof of the actual lazy004BEC00 singleton. Current application
hosts create neither the backend nor the online manager; those remain separate
owner-integration work.

## Public binding and lifetime

Construct `GamePlatformServices` from the shared Win32PlatformState and three
cursor byte references, references to the actual mutable input/online publication
slots, a reference to an optional `InputFocusResetHost*` provider slot, and the
same real `XLiveLibrary` used by message pretranslation. The constructor reads
or writes none of the publication/byte values and creates no owners.

The optional pointer is an application **provider slot**, not another native
singleton or private action table. It is read only when recovered BECA40 reaches
004BEC00. At that point it must name a genuine lazy getter using the established
action-manager lifetime domain. The provider's backend accessor is unused;
every reset backend read reloads the actual input publication directly.

`load_events()` returns the owned GameSoundLoadEvents for GameSoundRuntimeServices.
`devices()` returns one shared concrete dispatch chain:
KeyboardMouseFocusDeviceHost -> XInputFocusDeviceHost -> JoystickFocusDeviceHost.
These adapters call the existing real device polls, identifiers and destructors.
The reset destroys class1 mice; a complete future backend destructor still needs
its genuine keyboard deletion path, which this dispatch chain does not supply.
No action, device, manager, COM reference, DLL, clock or publication is owned by
GamePlatformServices. There is no application-frame callback dependency just to
service resource loads.

Cursor pumping reloads the actual online owner and uses that owner's canonical
context/flags. A stale supplied flags projection is a binding error. Mouse
cooperation calls actual DirectInput SetCooperativeLevel with the same stable
platform HWND; HRESULT remains ignored and configured becomes true as native.
Input update reloads the current backend and calls A918A0 with the same concrete
device chain. ShowCursor forwards the real signed OS count; its native repeated
loops stay in the existing policy. Missing required live bindings throw explicit
host errors; they do not report success or suppress native work.

Keep publication slots, platform, cursor bytes, XLiveLibrary and eventual
published owners/providers alive through all synchronous load callbacks. Sound
must finish shutdown before this adapter or the SDK library is destroyed. The
adapter never drains singleton lifetimes or closes borrowed COM/SDK resources.
Future input ownership must use canonical slots/groups, DirectInputRuntime and
InputEnumerationContext, retain acquired COM references through wrapper use,
and provide actual lazy action ownership. Future XLive ownership must publish
the same owner slot and use the shared application lifetime domain; its existing
semantic-domain constructor is not permission to introduce a second domain.

## Validation limits

The report records the Win32 build, existing two CTests, eight native seed
comparisons and live address/native CALL audit. One ignored manifested probe
uses the selected real private XLive DLL plus msidcrl40 dependency, checks both
loading modes with genuinely absent pre-online owners, preserves preexisting
cursor bytes, invokes ordinal5030 once, and balances a real ShowCursor pair.
It creates no fake input/action/online owner and leaves original SDK files alone.
Later live-backend focus reset, online initialization and application runtime
composition are not exercised by that focused probe; no gameplay claim is made.

## Correction from docs/GAME_INPUT_RUNTIME.md

AF replaces the typed input adapter described above with the application's
shared raw F8BBF4 publication and `GameInputRuntime`. `GameSoundLoadEvents`
borrows a `GameSoundCursorCalls` service; its XLive pretranslation behavior and
counters are unchanged. The facade calls the raw cursor/focus and device bodies,
using the committed GUI raw active-vector getter and actual lazy action owner.
The original pre-online null guard still applies. The old fixture above records
the earlier typed composition, not validation of the new constructor signature.
Current construction, ownership, failure boundaries and validation are recorded
in `docs/GAME_INPUT_RUNTIME.md` and `reports/game_input_runtime.json`.

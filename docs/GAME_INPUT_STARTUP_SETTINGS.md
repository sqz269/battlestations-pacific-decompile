# Input class startup and rumble services

Addresses: 004DD6A8..004DD6DA within 004DD5B0; 00A917E0; 00A949A0;
00A94C50. Source composition in GameInputRuntime and GameStartupHost.

The first-time OnInitOnce arm reloads F8BBF4 at4DD6A8,4DD6B9,4DD6CA and
calls A917E0 at4DD6B4,4DD6C5,4DD6D6. The three argument lists are
`(class0,1,null)`, `(class1,1,null)` and `(class2,1,null)`. This follows the
Xbox-compatibility setter at4DD6A3 and precedes later GUI/input setup. The
helper has ECX backend and three stack DWORDs, RET0Ch. Its full body, captured
range/returning-CRT behavior and library storage contracts are documented in
NATIVE_INPUT_CLASS_CONFIGURATION.md.

`GameInputRuntime::initialize_classes_004dd6a8` performs these three calls on
the current actual raw publication. GameStartupHost invokes this fragment before
its existing OnInitTitle path, instead of leaving all requested counts zero.
Each class receives one wildcard accepted ID and an empty active vector. Fixed
owning device slots, dirty byte, callback and references are unchanged. Activation
belongs to the later backend update. This does not claim the entire OnInitOnce
Lua/UI/settings prefix or tail is implemented at its native stage; the current
application still has deferred Lua initialization and missing action-map setup.

`set_rumble_enabled_00a94c50` on the facade now calls the raw setter. Its mutable
flag must be the same storage as the existing device tables; construction rejects
a different cell. The raw setter performs its native device outputs and refreshes,
not just a boolean assignment. A captured-profile force-output overload selects
the existing XInput/joystick target without recapturing the profile. The new
relative-slot1C overload provides the same contract for the upcoming raw action
binding consumers. Existing callers continue to capture their profile on entry.

The settings consumer at8D5DE2 enables rumble only when game+634 and settings+40
are both zero. Live inspection identifies game+634 as the cinematic/HUD-hidden
field, initialized at4DDD28 (BL=0 from4DDBAD). The current mission frame owns a
later source projection of that field. A complete application settings binding
must borrow the canonical game state rather than create another independent
flag. No full SettingsApplyHost implementation or new game flag was fabricated
for this input packet; the facade setter is ready for that consumer.

E12FB0 (mouse scale1.0) and F8BC04 (invert0) were verified in the image. The
worker's complete literal-address scan and xrefs found consumer reads only;
they do not establish mutable settings setters. In particular, the separate
invertCameraY setting is not evidence for a write to F8BC04.

## Verification

Strict Win32 build and both existing CTests pass. The class-configuration fixture
checks actual headers, source aliasing, captured range erasure after a returning
CRT handler, allocation reuse and cleanup. Four relocated native rumble cases
check call/write order and mutation; those use controlled output providers and
make no hardware force call. Reports list all call sites and original ABI limits.

The extended real SDK facade lifecycle fixture constructs the unfiltered backend
only when the actual controller inventory is zero. It observes requested counts
1/1/1, one wildcard ID per class and empty active vectors after configuration;
the real GUI getter returns null for all three. Toggling the shared rumble word
with no active devices reaches no output. Lazy action creation and raw BD0400
drain still clear all publications, followed by explicit COM reference release.
Its HWND remains hidden and nonactivating. Source/archive hashes and precise
validation revisions are recorded in reports/game_input_startup_settings.json.
The subsequent combined archive, seven focused fixtures and listener-equipped
facade drain are recorded separately in reports/game_input_actions.json.

The running game and single-instance check are untouched. No device polling,
ShowCursor, hardware force, game launch or gameplay validation is claimed.

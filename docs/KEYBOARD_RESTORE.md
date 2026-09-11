# Keyboard archive restore and runtime application

`read_keyboard_setup_006aba50` reads the body of an already-open `keyboardSetup`
section through the existing `GuiLuaReader`, then calls
`apply_keyboard_setup_006aa640`. They reuse `InputSettings`, `DeviceSettings`,
and the writer's `KeyboardInputBinding`; no parallel archive model is created.
Names are hypotheses. These are host C++ interfaces, not native object layouts
or drop-in binary replacements.

| Address and native body | Original ABI | Reconstructed entry |
| --- | --- | --- |
| 006aba50..006ac025 | ECX=settings; stack=reader pointer; RET 4 | `read_keyboard_setup_006aba50` |
| 006aa640..006aac43 | ECX=settings; no stack arguments; RET | `apply_keyboard_setup_006aa640` |

The Ghidra CLI verified configured project `bsp`, program
`/battlestationspacific.exe`, language and image base before each live query
batch. Assembly was necessary because the reader's indirect visitor calls
spoiled register/stack analysis, and runtime application contained overlapping
floating constants, x87 arithmetic and a false no-return boundary.

## Archive compatibility

At 006aba5a the reader sets settings+5. `InputSettings::runtime_settings_loaded`
models this latch: the apply path tests it but never clears it. The name does
not claim that this reader is its only native writer.

The traversal is the existing case-insensitive device map, then each device's
runtime binding map. Lua key lookup itself remains case-sensitive. Device and
action names are presence-tested before entering them. `inputSettings` and
`sensitivitySettings` are entered unconditionally inside a present device;
the existing reader supports missing subtrees as nil values.

Only numeric slots 0 and 1 are considered (006abbd0..006abed0). For a present
slot, required integer fields `deviceType`, `deviceIdx`, and `key` use reader
virtual +10h; `slider` and `reverse` use +0Ch with false defaults. Binding+8
is untouched. `reverse` lives in the independent devRec+6Ch map indexed by
the current input name and current slot (006abd4b..006abe61).

An absent slot resets the binding to `{-1,0,0,-1,false}` at
006abe69..006abebe. It does **not** clear that slot's reverse bit. This default
differs from the data-table loader's initial key of zero. Absent device, action
or sensitivity keys preserve existing values. Unknown archive names and slots
are ignored because the reader does not enumerate archive keys. Float
sensitivities update only present keys (006abf62..006abfcf). The final call to
runtime apply is at 006ac017, after all entered sections have been left.

## Runtime behavior and host calls

The apply path returns before host work if DEVINPUTS at settings+50h is set or
the +5 latch is clear. It obtains the input manager and tests action 128h via
00a92260 once. That helper bounds-checks the 30h-stride action table and reads
the action record's byte +0; `KeyboardRuntimeHost` exposes the predicate.

For each device in map order, 0055b400 copies devRec+50h's nested numeric map.
Outer key is input code, inner key is device class. Sensitivity descriptions
at devRec+18h are walked in case-insensitive name order. Native description
maps are projected from the existing loader's vectors: duplicate names append
codes (006a8117, 006a83eb), while the last sensitivity category overwrites the
previous category (006a8381). Bare input labels do not create native input-map
rows, so they are excluded.

Sensitivity category 0 selects device class 0; 1/3 select 1; 2/4 select 2.
Each selected scale of zero becomes one. The runtime multiplier is applied
unless it lies strictly between -1 and 1 and the code is in Min1SensHacks.
NaN takes the set-lookup branch, matching the ordered COMISS branch sequence.
The multiplication has a float32 store boundary (006aa86c..006aa874). The
temporary map is discarded after the device; original base scales stay intact.

The input-description map then determines action-code traversal. For each
input name, 006aa090 determines whether to use alternate slots. For slots
0/1, each code obtains the temporary scale indexed by binding.device_type;
a zero is replaced with one locally. Its reverse bit negates the scale.
Normal bindings are sent to 00a93750 with slot 0/1, even if disabled.
Alternate bindings use slots 2/3 and negative-zero minus scale. The predicate
00699bf0 suppresses alternate bindings when mouse class 1 has unsigned key
at least 8, or joystick class 2 has unsigned key at least 60; suppression sends
`{-1,0,0,-1,false}` and positive-zero scale. Native unsigned comparisons also
classify negative keys as exceeding those thresholds.

`KeyboardRuntimeHost::use_alternate_axis_slots_006aa090` remains an explicit
dependency. The helper examines AxisPairs and calls 0069e860 on the paired
input-description vectors; its comparison is not replaced with guessed vector
equality. `bind_action_00a93750` is the other effect boundary: native grows the
action's 34h-stride bindings, copies the descriptor and scale, and calls
00a91e80 to resolve device references. Neither host call has a successful
no-op default. The host represents the acquired singleton for the call.

## Evidence correction, limits and verification

The native call to `_free` at 006aac02 had a CALL_RETURN override. This hid
39 bytes at 006aac07..006aac2d and made pseudocode return after the first
device. The integrator cleared only that call-site override under the write
lock, disassembled the bytes, saved and refreshed the export. The recovered
006aac18 iterator increment and 006aac29 jump to 006aa6b0 establish the full
device loop. See `reports/keyboard_restore_flow_repair.json`; no native code
or global CRT signature was changed. No missing function definitions were found
inside either owned body.

Constants were read from the program: 00d7a218=+0f,
00d7a24c=1f, 00d7a250=-1 double, 00d7a260=-1f and 00d7a208=-0f.
Host double intermediates retain the observed float32 multiplication stores;
full x87 control-word, exception, denormal and NaN-payload parity is untested.
The meaning of binding+8 remains unspecified and preserved for present slots.
Short vectors throw `std::out_of_range` instead of continuing after native
checked-container failure. Invalid sensitivity categories with nonempty code
lists throw `std::invalid_argument` instead of consuming a stale or
uninitialized native temporary. Exceptions can leave partially updated state.

Validation uses the existing MSVC Win32 build and tests plus one ignored host
fixture at `local/keyboard_restore_fixture.cpp`. It connects the existing text
writer, Lua 5.1.1 execution, reader and recording runtime host; it covers partial
archive retention, missing-slot reset/reverse preservation, unknown-word
retention, ordering across devices, scaling, alternate suppression and gates.
Results and commands are recorded in `reports/keyboard_restore.json`. This is
host fixture evidence, not native differential or game runtime validation.

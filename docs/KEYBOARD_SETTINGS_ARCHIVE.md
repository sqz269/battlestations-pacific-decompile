# Keyboard settings archive writer
Addresses: `006a51c0`, `006a4ca0`, `006a8067`, `006a8448`; supporting `006a45c0`, `006a0db0`, `0049df50`, `006a5640`, `006aba50`.

`write_keyboard_setup_006a51c0(const InputSettings&, SettingsWriter&)` now writes the keyboard setup body into the existing archive visitor. It uses the existing `InputSettings::devices` map and adds the runtime members absent from the earlier Lua-description projection. Callers open and close `keyboardSetup`; this function neither creates that outer section nor writes options.txt.

The name `BSP_InputSettings_WriteKeyboardArchive` is a descriptive hypothesis, not a recovered symbol. The native body is `006a51c0..006a563d`, with ECX pointing to the settings object, the writer on the stack, and `RET 4`. The host class layouts and visitor vtables are not ABI compatible. No saved analysis was modified by this worker; annotation requests are recorded in the report for the integrator.

## Native storage and archive schema

The device map is at settings+`08h`. Its node key occupies +`0ch..13h`, and the mapped device value begins at node+`14h`. Device traversal uses the case-insensitive map, not the separate device-order vector. Each device emits `inputSettings` followed by `sensitivitySettings`, including empty sections.

| Device-relative offset | Host storage | Evidence |
| --- | --- | --- |
| `+0ch` | `runtime_bindings`, case-insensitive input-name map to vector of `KeyboardInputBinding` | `006a525e..006a5263` selects node+`20h`; `006a45c0` uses the N28 case-insensitive lower bound |
| `+24h` | `runtime_sensitivities`, case-insensitive name-to-float map | `006a5566..006a55ed` selects node+`38h` and emits float tag 2 from inner node+`14h` |
| `+6ch` | `runtime_reverse`, case-insensitive input-name map to packed boolean vector | `006a5400..006a540e` selects device node+`80h`; `006a4ca0` uses the N2CB case-insensitive lower bound |

The binding vector contains 20-byte native elements. Each input name always opens its section. Exactly indices 0 and 1 are visited, even if the vector has more entries. A binding whose +0 field equals `-1` emits no slot section. Enabled bindings open **integer variant** keys 0/1, then emit:

| Binding offset | Archive key | Type and omission |
| --- | --- | --- |
| `+00h` | `deviceType` | integer, always |
| `+04h` | `deviceIdx` | integer, always |
| `+08h` | none | preserved as `unknown_08`; writer never serializes it |
| `+0ch` | `key` | integer, always |
| `+10h` byte | `slider` | boolean, omitted when false |
| separate reverse map, same input and slot | `reverse` | boolean, omitted when false |

The literal at `00cf8070` is `key`, despite its pointer-like label in pseudocode. Assembly at `006a5440..006a544d` advances the reverse iterator by the slot index; `006a5490..006a54ce` validates and reads that exact bit. Pseudocode incorrectly suggests register inputs, invalid globals, and reading bit zero for both slots. The assembly establishes that slot 1 uses reverse bit 1. All runtime sensitivity floats are written, including `1.0`; the Lua sensitivity description's integer field is a separate value.

`SettingsWriter` now requires a variant-key section overload. The existing `ArchiveTextWriter` overload satisfies it. The sole existing test recorder was adapted to the interface without adding permanent cases.

## Initialization and invalid state

The earlier `load_keyboard_setup` did not store the three runtime maps. Disk assembly inside the already established `006a7be0` body proves the following initialization, now reproduced in that loader:

- Table-form inputs resize their binding vector to two at `006a8067..006a8096`, preserving existing entries on repeated case-insensitive names. The fill element is `{-1, 0, 0, 0, false}`. Bare string labels create no runtime binding entry.
- The same input's reverse vector resizes to two with false at `006a809b..006a80b1`. `0049df50` preserves existing bits while resizing.
- Each named sensitivity writes `1.0f` into its runtime map at `006a8448..006a8459`; `00d7a24c` contains `00 00 80 3f`.

The serializer does not invent missing mappings. Short binding vectors or missing/short reverse vectors for enabled slots throw `std::out_of_range`. Native code instead invokes checked-container failure; its reverse-map operator[] first inserts an empty vector before the invalid access. The const host writer avoids that invalid-state mutation. An exception can occur after some visitor output, so callers must reject the archive when serialization fails.

## Evidence and remaining integration

Live reads/exports use `tools/bsp.py ghidra`, whose client verifies project `bsp` and `/battlestationspacific.exe` before each batch. The worker exported the writer and reverse-map accessor and inspected writer assembly, loader disk bytes, helper decompilations, and read-side structure. The adjacent `006a5640` is another writer that opens `Preset`, not the read path. The paired reader is `006aba50..006ac025`; it checks existing known device/input sections and ends by calling `006aa640`. That reader and runtime device rebinding/application remain unreconstructed by this packet.

Verification is reported in `reports/keyboard_settings_archive.json`: Win32 compilation, existing CTests, seed byte checks, and a single ignored host fixture cover the host implementation. This is concrete archive serialization, not a runnable original game, native differential execution of this writer, ABI compatibility, or gameplay validation.

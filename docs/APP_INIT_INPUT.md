# Application initialize, input phase

Addresses: 005547d0, 006a7be0, 00a982d0, 00a900f0, 006ab6b0, 0055c110, 00a904e0, 00a98030,
00a982b0, 00a9a3e0, 00a9a290, 00a99940, 00a9a5a0, 00a95bd0, 00a93e80

Packet `app_init_input`. Everything below is read off Ghidra pseudocode and assembly. No Ghidra
object was mutated; this packet was read-only against the program. Every name is a hypothesis,
not a recovered symbol. The bootstrap packet's outputs (module path, command line, registry game
settings) are treated as inputs and are not re-derived here.

## Position in `cSkeletonAppMidway::Init` (0073d410)

Two separate phases of the application initializer touch input, and they are far apart.

| Init address | Call | Phase |
| ------------ | ---- | ----- |
| 0073da94 | `005547d0` settings singleton | 5, right after the D3D9 renderer is built |
| 0073da9b | `006a7be0` data-table load | 5 |
| 0073dd8e | `00a982d0` DirectInput manager construction | 8 |
| 0073dd9x | `*(DAT_00f8bbf4 + 0xd8) = &DAT_004b4630` | 8 |
| 0073ddaf | `00a900f0` device-slot reset | 8 |

So the settings tables are parsed while the renderer exists but before any device is created, and
the device-slot reset runs immediately after the manager constructor. `006a7be0` is also reached
from the settings constructor itself, so the phase-5 call site is not the only entry.

## 005547d0, the settings singleton

`__cdecl`, no arguments, `RET` with no operand, returns the singleton pointer in EAX. The body is
the standard lazy getter this codebase uses everywhere: test `DAT_00e198e8`, take the critical
section from `BSP_SingletonLifetime_GetManager()+0x10`, re-test under the lock, allocate `0x540`
bytes through `LIBCRT_unmatched_00bf681b`, construct with `FUN_006ab6b0`, then register the
instance with `BSP_SingletonLifetime_Register`. A failed allocation stores a null singleton and
still registers it. The recursion count at `criticalSection[1].DebugInfo` is incremented and
decremented by hand around the guarded region.

State: analyzed. Nothing about the getter is reconstructed; the C++ module models the object it
builds, not the singleton mechanism.

## 006ab6b0, the settings object

`__fastcall` on ECX, returns the object. It sets the vtable `00cf81cc`, builds five red-black tree
heads and three checked vectors, constructs the persistent Lua script wrapper at `+0x78` through
`00b66bd0`, clears the two flag bytes at `+0x04` and `+0x05`, and then calls `006a7be0` on itself.
The load therefore happens inside construction; the phase-5 call at 0073da9b re-enters a routine
whose `+0x04` guard is already set and returns immediately.

Node shapes come from the `_Isnil` offset each head is stamped with, matched against the table in
`docs/INPUT_SETTINGS_TREE_CLUSTER.md`.

| Offset | Member | Evidence |
| ------ | ------ | -------- |
| +0x00 | vtable `00cf81cc` | 006ab6c8 |
| +0x04 | `loaded` flag | 006a7c09 tests it, 006a7c21 sets it |
| +0x05 | second flag, never read in this packet | 006ab7c8 |
| +0x08 | `map<PooledString, DeviceRecord, CaseInsensitiveLess>` | head at +0x0c, `_Isnil` +0x99, size +0x10 |
| +0x14 | checked `vector<PooledString>` device names | `_Myfirst` +0x18, `_Mylast` +0x1c, `_Myend` +0x20 |
| +0x24 | `map<PooledString, InputNameRecord, CaseInsensitiveLess>` | head +0x28, `_Isnil` +0x29, N2C |
| +0x30 | checked vector, Conflicts.Groups | `_Myfirst` +0x34, `_Mylast` +0x38 |
| +0x40 | checked vector, Conflicts.Pairs | `_Myfirst` +0x44, `_Mylast` +0x48 |
| +0x50 | `DEVINPUTS` boolean | 006a947a |
| +0x54 | `map<int, T4>` | head +0x58, `_Isnil` +0x15, N18; not written by 006a7be0 |
| +0x60 | `map<PooledString, map<int, PooledString>, CaseInsensitiveLess>` | head +0x64, `_Isnil` +0x21, N24 whose 12-byte mapped value is exactly a tree object |
| +0x6c | second map with the DeviceRecord instantiation | head +0x70, `_Isnil` +0x99; not written by 006a7be0 |
| +0x78 | Lua script wrapper, holds the ControlPresets state | 006ab7b8, 006a7c1b |

The device map's `_Isnil` at +0x99 pins `sizeof(value_type)` to `0x8c`, so the mapped device record
is `0x84` bytes after the 8-byte pooled-string key. `0055c110` is that map's `operator[]`; it
descends with `BSP_NativeString_LessCaseInsensitive`, so device lookup is case-insensitive. The
same is true of the `InputNames` map and the `ControllerInputNames` map.

State: analyzed. The C++ module mirrors the members this packet proved and nothing else.

## 006a7be0, the data-table load

`__fastcall` on ECX, `RET` with no operand, no return value. Guarded by the byte at `this+0x04`,
which it sets before doing anything, so it runs exactly once per object.

Two Lua states are involved. The persistent wrapper at `this+0x78` is initialised with library mask
1 and runs `Scripts/datatables/ControlPresets.lua`; **nothing in this function reads a table out of
it.** The presets stay resident in that state for later code. A second, stack-local wrapper is
constructed at 006a7c7b, initialised with the same mask, and runs
`Scripts\datatables\KeyboardSetup.lua` and later `Scripts\datatables\ControllerInputNames.lua` in
the same state; it is destroyed at 006a9829. Note the mixed path separators: the presets path uses
forward slashes and the other two use backslashes.

The wrapper API is the one already documented for the shader loader:
`00b66bd0` construct, `00b6a020` initialise with a library mask, `00b69d40` execute a file,
`00b67980` globals reference, `00b67800` string-key index, `00b67720` integer-key index,
`00b67700` release a reference, `00b67690` move-assign a reference into an existing variable,
`00b67080`/`00b67190` begin and advance a `next` walk with `00b66420` as its end test,
`00b65fb0` exact nil test, `00b660a0` exact string test, `00b66000`/`00b66250` boolean test and
read, `00b662b0` `lua_tolstring`, `00b66290` `lua_tonumber` truncated to a signed int through a
float32 spill, `00b66270` the same coercion without the truncation, `00b669a0` destroy the wrapper.

Before the walk the routine clears only two containers: the device map (`006a7540` subtree erase
then a head reset at 006a7cf3) and the device-name vector (checked range erase `004954f0`). No
other member is reset, so a second load would append to `InputNames`, the conflict vectors and
`ControllerInputNames`. The `+0x04` guard is what actually prevents that.

### Schema the three Lua files must provide

`KeyboardSetup.lua`

```lua
KeyboardSetup = {
  { Name = "<device name>",
    Inputs = {
      "<input name>",                       -- bare-string form, no codes
      { "<input name>", { <int>, <int> } }, -- table form: [1] name, [2] codes
    },
    Sensitivities = {
      { "<name>", <int>, { <int>, ... } },  -- [1] name, [2] value, [3] codes
    },
    BaseSensitivities = { [<int>] = { [<int>] = <number> } },
    AxisPairs = { { "<name>", "<name>" } },
    Min1SensHacks = { <int>, ... },         -- optional, nil-guarded at 006a89de
  },
}
InputNames  = { ["<name>"] = { <int>, <int> } }
Conflicts   = { Groups = { { { "<name>", "<name>" } } },
                Pairs  = { { <int>, <int> } } }
DEVINPUTS   = true                          -- or false, or absent
```

`ControllerInputNames.lua`

```lua
ControllerInputNames = { ["<device name>"] = { [<int>] = "<label>" } }
```

`ControlPresets.lua` is executed and left resident; this packet did not establish what it defines.

The device array, `Inputs`, `Sensitivities`, `AxisPairs`, `Min1SensHacks`, `Conflicts.Groups`,
`Conflicts.Pairs` and every inner code list are walked as one-based arrays that stop at the first
nil. `InputNames`, `BaseSensitivities` and both levels of `ControllerInputNames` are walked with
the `next` iterator, so their key order is the Lua hash order, not a declaration order.

### What each read writes

| Source | Destination | Evidence |
| ------ | ----------- | -------- |
| `entry.Name` | `0055c110(this+0x08, name)` creates the record, `00450540(this+0x14, name)` appends the name | 006a7e69, 006a7e7a |
| `Inputs[k]` string | `00450540(devRec+0x30, name)` | 006a7f91 |
| `Inputs[k][1]` | `006a44b0(devRec, name)` then the same `+0x30` append and `006a45c0(devRec+0x0c, name)` | 006a8046, 006a8055, 006a8062 |
| `Inputs[k][2][m]` | `00697f40` appends the int to the record `006a44b0` returned | 006a8117 |
| `Sensitivities[s][1]` | `0055a9a0(devRec+0x18, name)` returns the slot | 006a8344 |
| `Sensitivities[s][2]` | written to that slot's first dword | 006a838b |
| `Sensitivities[s][3][n]` | `00697f40` appends the int | 006a83eb |
| after each sensitivity | `00444be0(devRec+0x24)` slot set to `1.0f` (`DAT_00d7a24c`) | 006a8448 |
| `BaseSensitivities[a][b]` | `006a5aa0` selects the row, `006a1560` indexes it, value stored as float32; a zero becomes `1e-8f` (`DAT_00cf7fe8`) | 006a859a, 006a8608, 006a8616 |
| `AxisPairs[p][1]`, `[2]` | two pooled strings in a 16-byte element appended to the vector at `devRec+0x5c` | 006a87da, 006a8898 |
| `Min1SensHacks[q]` | `0069fa40`, a `set<int>` unique insert | 006a8a6d |
| `InputNames[name][1]`, `[2]` | `006a1e70` indexes the map at `this+0x24`; written to value+0x00 and value+0x0c | 006a8ce4, 006a8d1f |
| `Conflicts.Groups[g][h][1]`, `[2]` | `006a79a0` appends a group, `006a6350` appends an 8-byte-stride two-string element | 006a8e5a, 006a8f19 |
| `Conflicts.Pairs[p][1]`, `[2]` | `006a4710` appends a 4-byte-stride two-int element; **both values have one subtracted** | 006a9327, 006a937d |
| `DEVINPUTS` | byte at `this+0x50`; a non-boolean yields `false` | 006a9453, 006a947a |
| `ControllerInputNames[dev][code]` | `006a6900(this+0x60, dev)` selects the inner map, `006a1f80` indexes it by the integer key, value stored as a pooled string | 006a95c3, 006a9661 |

The three unresolved device-record helpers are `006a44b0` (input record by name), `006a45c0`
(`devRec+0x0c`), `0055a9a0` (`devRec+0x18`), `00444be0` (`devRec+0x24`), `006a4ca0` (`devRec+0x6c`,
called with the constants 2 and 0 alongside a 20-byte stack temporary built at 006a8067) and
`006a0db0` (called with 2 and -1). Their containers were not opened; the device record is
`0x84` bytes and offsets `+0x0c`, `+0x18`, `+0x24`, `+0x30`, `+0x5c` and `+0x6c` are the ones this
packet touched.

State: analyzed for the native containers, reconstructed for the schema. The C++ module reproduces
every table read above against an already-evaluated Lua state.

## 00a982d0, DirectInput bring-up

`__fastcall` on ECX, `RET` with no operand, returns `this` in EAX. It is a constructor: it calls the
base/array helper `00a91570`, stamps the vtable `00d5b72c`, then does the DirectInput work.

```
this+0xe0 = 0; this+0xe8 = this+0xec = this+0xf0 = 0; this+0xf4 = 0;
DirectInput8Create(GetModuleHandleA(nullptr), 0x0800, &IID_IDirectInput8A, &this+0xe0, nullptr);
this+0xe0 -> AddRef();                                     // vtable +0x04
this+0xe0 -> EnumDevices(0, 0x00a982b0, this, 1);           // vtable +0x10
for (i = 0; i < 4; ++i) 00a904e0(this, -1, new(0x240) 00a9a5a0(i));
```

`DAT_00d78d8c` is `30 80 79 bf 3a 48 a2 4d aa 99 5d 64 ed 36 97 00`, that is
`{BF798030-483A-4DA2-AA99-5D64ED369700}`, `IID_IDirectInput8A`. The version is
`DIRECTINPUT_VERSION` 0x0800, the enumeration class is `DI8DEVCLASS_ALL` and the flag is
`DIEDFL_ATTACHEDONLY`. **The HRESULT is discarded and the returned pointer is dereferenced on the
next instruction**, so a failed create faults inside `AddRef`. The same is true of the four
allocations: a null result is passed straight into `00a904e0`, which dereferences it.

The vector at `this+0xe8`/`+0xec` has 16-byte elements and holds the instance GUIDs already seen;
`this+0xf4` is set when an enumerated product name matches Xbox and 360.

### The enumeration callback

`00a982b0` is a three-instruction `__stdcall` thunk, `RET 8`, matching
`DIEnumDevicesCallbackA(LPCDIDEVICEINSTANCEA, LPVOID)`. It forwards to `00a98030(pvRef, lpddi)` and
returns its result, always `DIENUM_CONTINUE`. Ghidra has no function object at `00a982b0`, so it is
read from bytes.

`00a98030` switches on the low byte of `lpddi->dwDevType` at `+0x24`, and reads the product name at
`+0x12c`, which is `DIDEVICEINSTANCEA::tszProductName`, so the ANSI interface is in use throughout.

| `dwDevType` | Guard | Allocation | Constructor | Slot |
| ----------- | ----- | ---------- | ----------- | ---- |
| 0x13 `DI8DEVTYPE_KEYBOARD` | `this+0x04` empty | 0x310 | `00a9a3e0(di8)` | class 0, slot 0 |
| 0x12 `DI8DEVTYPE_MOUSE` | `this+0x24` empty | 0x23c | `00a9a290(di8)` | class 1, slot 0 |
| 0x14/0x15 joystick, gamepad | not already in the GUID vector | 0xb48 | `00a99940(di8, lpddi)` | class 2, first free |
| anything else | | | | ignored |

Because the guard offsets are `this+0x04` and `this+0x24`, and joysticks land at
`this+0x44 + index*4`, the three rows of the slot array are keyboard, mouse and joystick.

### Cooperative levels, data formats and buffer sizes

| Device | `CreateDevice` GUID | `SetDataFormat` | `SetCooperativeLevel` |
| ------ | ------------------- | --------------- | --------------------- |
| keyboard | `GUID_SysKeyboard` `{6F1D2B61-D5A0-11CF-BFC7-444553540000}` at 00d78efc | `00d79804`, the static keyboard format | window, `6` = `DISCL_FOREGROUND | DISCL_NONEXCLUSIVE` |
| mouse | `GUID_SysMouse` `{6F1D2B60-D5A0-11CF-BFC7-444553540000}` at 00d78eec | `00d795fc`, a static mouse format | none in the constructor |
| joystick | the enumerated `guidInstance` | a format built at `device+0x274` from an `EnumObjects` walk with the callback `00a99920` | window, `5` = `DISCL_EXCLUSIVE | DISCL_FOREGROUND` |

The window handle comes from `00bec230`, a `__thiscall` on the global platform object at
`0109cf04` that takes no arguments; Ghidra mis-attributes the cooperative-level flag to it as an
argument, which the assembly at 00a9a445 and 00a99cf1 disproves.

**No device sets `DIPROP_BUFFERSIZE` and none calls `SetEventNotification`**, so every device reads
immediate state rather than buffered data. The only properties touched are on the joystick:
`GetProperty(DIPROP_RANGE)` per axis with `dwSize` 0x18, `dwHeaderSize` 0x10, `dwObj` the byte
offset and `dwHow` `DIPH_BYOFFSET`, whose min and max are cached with their difference at
`device+0x290 + axis*0x1c`; and `SetProperty(DIPROP_AUTOCENTER)` with `dwData` 0, only when the
force-feedback member at `device+0xb18` is set.

The mouse constructor also zeroes two 256-byte state buffers at `+0x0c` and `+0x10c`, records
`GetSystemMetrics(SM_MOUSEWHEELPRESENT)` at `+0x235` and stores `GetDoubleClickTime()/1000.0` as a
float at `+0x238`.

State: analyzed. The C++ module reproduces the ordering and the slot assignment against an injected
host, and carries the parameters above as data. It creates no device.

## 00a904e0, slot assignment

`__thiscall`, two stack arguments, `RET 8`.

```
class = device->vtable[+0x08]();          // 0..2
if (requested == -1) {
    slot = 0;
    while (this[4 + class*0x20 + slot*4] != 0) { ++slot; if (slot >= 8) break; }
}
device->field_0x08 = slot;
this[4 + (slot + class*8)*4] = device;
```

The scan is the source of the 3x8 shape: eight columns per class, and each class searched
independently. When a row is full the counter leaves the loop at 8 and the store lands outside the
row, one element into the next class or past the array for class 2. The reconstruction reports this
case instead of writing.

## 00a900f0, the slot reset

`__fastcall` on ECX, `RET` with no operand, no return value, no stack arguments.

```
p = this + 4;
for (3 rows)  for (8 columns) { if (*p) ((void(__thiscall*)(void*))(*(void**)*p)[5])(*p); ++p; }
```

The virtual is slot index 5, byte offset `+0x14`, invoked with ECX only and no arguments. The base
device implements it as a bare `RET` at `00a93e80`, so the meaning is entirely in the overrides; the
name `reset` is a hypothesis taken from the call site, which is the first thing to run after the
manager is constructed. Ghidra reports a three-byte gap at `00a900fd`, which is alignment padding
between the `JMP` at 00a900fb and the loop head at 00a90100.

The device vtable this reaches is `00d5bb48`: `+0x08` returns 2 for the four devices the constructor
makes (`00a95bd0` is `MOV EAX,2; RET`), `+0x14` is the no-op, `+0x18` is `RET 8` and `+0x1c` is
`XOR EAX,EAX; RET 4`.

## Reconstruction

`include/bsp/input_settings.hpp` and `src/input_settings.cpp` build with `/W4 /WX` into `bsp_core`
and are exercised by one case in `tests/math_tests.cpp`.

- `bsp::load_keyboard_setup` and `bsp::load_controller_input_names` read every table listed above
  from an already-evaluated Lua state through the stock Lua 5.1.1 C API, in the same order and with
  the same coercions as the native routine, including the float32 spill before truncation, the
  `1e-8f` substitution for a zero base sensitivity and the one subtracted from both halves of a
  conflict pair. They report an error where the native routine would carry on over a wrong type.
- `bsp::InputDeviceTable` reproduces `00a904e0` and `00a900f0` with explicit arguments over an
  abstract `InputDevice` exposing only the two virtual slots this packet proved.
- `bsp::create_direct_input_devices` reproduces the ordering of `00a982d0` against an injected
  `DirectInputHost`, and the three `*_device_profile()` accessors carry the recovered GUIDs, data
  formats and cooperative levels as data.

State: reconstructed and build-tested. Nothing here is ABI-compatible with the original and nothing
is game-validated.

## Uncertainties and what remains

- The meaning of the two `InputNameRecord` fields and of the three dwords the load leaves default.
- The device record's `0x84` bytes: `006a44b0`, `006a45c0`, `0055a9a0`, `00444be0`, `006a4ca0` and
  `006a0db0` were not opened, so `devRec+0x0c`, `+0x18`, `+0x24` and `+0x6c` are known only by the
  values written into them.
- The two settings members `+0x54` and `+0x6c` are constructed and never written by this load.
- `ControlPresets.lua` is executed but not consumed here; whoever reads the presets out of the
  wrapper at `this+0x78` is unidentified.
- The flag byte at `this+0x05`.
- The virtual at device vtable `+0x14` is a no-op in the base, so the semantics of the slot reset
  live in an override this packet did not locate.
- `*(DAT_00f8bbf4 + 0xd8) = &DAT_004b4630` between the constructor and the reset is unexplained.
- Whether `Sensitivities[s][2]` is an index, a scale or a mode is not established; only its width
  and its placement are.

## Correction from docs/INPUT_DEVICE_STATE.md (2026-09-10)

The keyboard and mouse query/update methods are now reconstructed, including
actual calls to the borrowed ANSI DirectInput device. Their GetDeviceState
failure behavior differs: the keyboard ignores HRESULT and normalizes whatever
bytes were written, while the mouse records validity but still accumulates its
sample after a successful Poll. Both preserve the native history-update order.

The mouse constructor's `GetSystemMetrics(0x17)` is `SM_SWAPBUTTON`, not
`SM_MOUSEWHEELPRESENT`. Byte +235 swaps button codes 0 and 1. Axis values use the
live scale at 00e12fb0 divided by 100; Y inversion and wheel negation occur in
different arithmetic domains. 6272 native query comparisons cover the resulting
float and byte behavior. The base +14 reset at 00a93e80 is an actual bare RET.

`KeyboardMouseBindingPollHost` connects these queries to full binding polling.
It requires reconstructed keyboard/mouse devices and an actual CRT square-root
service. Device creation/data format/COM ownership, joystick and XInput queries,
and hardware/gameplay verification remain separate dependencies.

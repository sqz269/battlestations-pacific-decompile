# Application init bootstrap and options

Addresses: 00439040, 006ad0d0, 0073ce20, 0073c3b0, 008d8190, 00737c40

Packet `app_init_bootstrap_options`, owner `agent/init-bootstrap`. Parent map:
`docs/APP_INITIALIZE_MAP.md` / `reports/app_initialize_map.json`. Every name below is a
hypothesis, not a recovered symbol. All six routines are called from
`BSP_Application_Initialize` (0073d410) and from nowhere else.

The native string and sized-pool helpers (0041dd40, 0041e870, 00419cc0, 00bd1510, 00438e40)
are owned by `app_init_alloc_string_helpers` and are treated here as external calls with the
interface observed at the call site.

## Two corrections to the parent map

1. **006ad0d0 is not a crash-handler install.** It registers three code pointers, not one, and
   the consumers are the debug-inspection readers in segment 95, not an exception filter. See
   below. Recorded as `BSP_Application_InstallObjectHandleResolvers`.
2. **008d8190 is an options-file loader with a registry fallback,** not a registry-backed
   settings loader. The registry is touched only when the options file cannot be opened, and
   only one value (`language`) is read from it. Recorded as `BSP_GameSettings_Load`.

A third, smaller correction: the hardware probe reads a *second* registry key,
`HKLM\SOFTWARE\Eidos\BSM_HWD`, which the parent map does not mention.

## 00439040 BSP_Application_CaptureModulePath

`void __cdecl f(void)`, RET 0, no arguments, no return value.

`GetModuleFileNameA(NULL, buf, 0x104)`, then `__strlwr(buf)`, then
`_splitpath(buf, drive, dir, fname, ext)`. Only `drive` and `dir` survive: the body inlines
`strcpy(DAT_00e176c8, drive)` (0043907e-0043909f) followed by `strcat(DAT_00e176c8, dir)`
(004390b0-004390dd, the tail being a `rep movsd` plus a byte remainder). The file name and
extension are computed into stack buffers and discarded.

`DAT_00e176c8` is the lower-cased directory of the running executable, with the trailing
separator kept. It has no reader anywhere in the image outside this function, which is the
first thing worth flagging: the value is captured at 0073d458 and, as far as static cross
references go, never consulted again.

Stack buffers are `wchar_t[130]` and three `wchar_t[128]` in Ghidra's view but are used as
`char` throughout; the decompiler's element type is wrong, the byte arithmetic is right.

## 006ad0d0 BSP_Application_InstallObjectHandleResolvers

`void __cdecl f(void)`, RET 0. Five instructions:

```
006ad0d0: PUSH 0x6ad0c0
006ad0d5: MOV EDX,0x888aa0
006ad0da: MOV ECX,0x6ad080
006ad0df: CALL 0x00bd4fc0
006ad0e4: RET
```

`FUN_00bd4fc0` is `__fastcall(ECX, EDX, [stack])` and does nothing but three stores:
`DAT_0109ced4 = ECX`, `DAT_0109ced8 = EDX`, `DAT_0109cedc = [stack]`. The slots are read by
`FUN_00bd63b0` (0109ced4 at 00bd64b3, 0109ced8 at 00bd64e2), `FUN_00bd7180` (0109cedc at
00bd747e) and `FUN_00bd7f50` (0109cedc at 00bd82ef).

The three callbacks:

| Slot | Target | Behaviour |
| --- | --- | --- |
| DAT_0109ced4 | 006ad080 | 16-bit handle to object pointer. Null-checks ECX, compares the 16-bit id against `[00f89a10]`, then indexes either the table at `[00f89a54]` or `[00f89aa8]` with stride 0x10 and returns `+0xc`. |
| DAT_0109ced8 | 00888aa0 | Enters a scripting scope with `FUN_00b67800(.., DAT_00cfad08 = "Ptr")` and returns `FUN_00b662d0()`. |
| DAT_0109cedc | 006ad0c0 | `TEST ECX,ECX; JZ; MOVZX EAX,word ptr [ECX+0x174]; RET` plus `XOR EAX,EAX; RET`. Object pointer to 16-bit handle, 0 for null. |

006ad0c0 is a bare label, not a Ghidra function, so `bsp.py ghidra disasm` refuses it; the
listing above is decoded from `85 c9 74 08 0f b7 81 74 01 00 00 c3 33 c0 c3`.

Nothing in this path writes a crash handler, an exception filter or a signal. The provisional
name in the parent map came from the position in Init, not from the body.

## 0073ce20 BSP_Application_ParseCommandLine

`void __thiscall f(cSkeletonAppMidway* this)`, RET 0. `this` arrives in ECX and is used for
exactly one field, `this+0x1a`. The function takes no explicit string: it reads the duplicated
command line from `DAT_00e1ae78`, which Init assigns five instructions earlier at 0073d945.

Tokenizing: `BSP_CharacterString_AssignCounted(DAT_00e1ae78, strlen(..))` builds a
`std::string`, then `FUN_0094ec70(&vector, &string, " \n\r")` splits it. The delimiter set is
the literal at 00cff164 (`20 0a 0d 00`). The splitter suppresses empty fields, toggles an
in-quotes state on `"` that disables delimiters, and leaves the quote characters inside the
token. Elements are 0x1c-byte MSVC `std::string`s (`_Bx` at +4, `_Mysize` at +0x14,
`_Myres` at +0x18) walked with a checked iterator held at `ESP+0x10`.

Comparison helpers: `FUN_004b3fc0(data, lit, n)` is a counted memcmp, guarded by an explicit
length equality so the first two tokens are exact matches; `FUN_004beb60` is
`std::string::compare(0, size, lit, n)`; `FUN_004c2df0` is `operator==(const string&, const
char*)`; `FUN_004cdbe0(out, 0, n)` is `substr`, used for the four prefix switches.

### Switch table (13 tokens)

| Token | Literal | Match | Effect |
| --- | --- | --- | --- |
| `cachedload` | 00ce8168 | exact | `DAT_00e1ae76 = 1` |
| `nozip` | 00ce80f0 | exact | `DAT_00e0a538 = 0` |
| `fixfps` | 00ce80e0 | exact | `DAT_00e1ae81 = 1` |
| `1frame` | 00ce8158 | exact | `DAT_00e1ae75 = 1` |
| `filelog` | 00ce8160 | exact | `DAT_00e1ae77 = 1` |
| `freecam` | 00ce80e8 | exact | `DAT_00e1ae80 = 1` |
| `ip:` | 00ce811c | prefix, 3 | inline `strcpy(&DAT_00f1af38, token + 3)`, unbounded |
| `localport:` | 00ce8110 | prefix, 10 | `DAT_00f1af30 = _atol(token + 10)` |
| `connectport:` | 00ce8100 | prefix, 12 | `_DAT_00f1af34 = _atol(token + 12)` |
| `debug:` | 00ce80f8 | prefix, 6 | none; 0073d19f jumps straight to the loop tail |
| `auto` | 00ce80b4 | exact | consumes one or two following tokens, see below |
| `memlimit` | 00ce80bc | exact | `this+0x1a = 1` |
| `nomemlimit` | 00ce80c8 | exact | `this+0x1a = 0` |

`debug:` is the only token that is recognised and then deliberately dropped. Unrecognised
tokens fall out of the chain with no diagnostic.

### The `auto` group

`auto` advances the loop's own iterator, so the follower tokens are consumed and never
re-examined by the switch table. ESI is reloaded from the iterator at 0073d364 before the
outer increment, which is what makes the consumption stick.

| Form | Effect |
| --- | --- |
| `auto mpak classes` | `DAT_00e1ae77 = 1` (set at 0073d231, before the third token is read), then `FUN_00425560` on the singleton from `FUN_004c1e90` |
| `auto mpak scenes` | same flag, then `FUN_00425570` |
| `auto test <name>` | `BSP_NativeString_Assign(name)`, then `FUN_004278a0` on the same singleton |

`auto mpak` raising the file-access-log flag is coherent: `files.txt` is the record of which
files the run touched, which is what an automated packaging pass needs.

Every `auto` form stops early if the group runs off the end of the token list.

## 0073c3b0 BSP_SystemOptions_ProbeHardware

`void __cdecl f(void)`, RET 0. Roughly 0x11000 bytes of stack frame, hence the
`__alloca_probe` injection warning.

Four values are read from `HKLM\SOFTWARE\Eidos\BSM_HWD` through `FUN_0098d4e0`, which is
`__fastcall(ECX = value name, EDX = expected type, [stack] data, [stack] &cbData)` and returns
true only when `RegQueryValueExA` succeeds *and* the returned type matches:

| Value | Wrapper | Expected type | Destination |
| --- | --- | --- | --- |
| `MemSize` (00cfe9d4) | 0098d5c0 | 0xb, REG_QWORD | low dword of an 8-byte buffer |
| `CPUSpeed` (00cf3aa8) | 0098d5c0 | 0xb, REG_QWORD | low dword |
| `GPUDeviceID` (00cfe9dc) | 0098d5c0 | 0xb, REG_QWORD | low dword |
| `SoundDevice` (00cfe9e8) | 0098d5f0 | 1, REG_SZ | 0x400-byte buffer |

The gate at 0073c44d sums the four booleans and requires exactly 4. A single missing or
mistyped value makes the whole routine a no-op, silently. The key is written by the separate
hardware-detection pass, not by the game.

With all four present, the body assembles up to four wide message fragments (three guarded by
`FUN_0098c870`, `TRIV_body_0098c7f0` and `TRIV_body_0098c800`, one by `FUN_00449af0`),
formats them with `L"%s%s%s%s\n%s"` (00cff128) and, if any fragment was produced, calls
`MessageBoxW(NULL, text, caption, 0x34)` = `MB_YESNO | MB_ICONEXCLAMATION`. On `IDYES` (6) it
calls `FUN_0098f430("options.txt")`, which is a `fopen(path, "wb")` writer, so answering yes
overwrites the options file with generated defaults.

Localised text comes from `FUN_00996060(lang, index)`, a flat table at
`PTR_u_Battlestations__Pacific_already_r_00d1e918` indexed `[lang + index * 6]`, six languages
per message. That table is available before the VFS exists, which is why this routine can talk
to the user at 0073d610, inside the `DAT_0109ceec == 0` first-time branch.

The routine writes no globals. Its only outward effect is the message box and the possible
rewrite of options.txt.

## 008d8190 BSP_GameSettings_Load

`void __fastcall f(cGameSettings* this)`, RET 0. Init calls it as
`MOV ECX,0xf88980; CALL 0x008d8190`, so `this` is the static object at **00f88980**.

Preamble: `FUN_008d7bc0()` resets the object, `FUN_008d4ea0(TRIV_body_00b1fff0())` seeds a
renderer-derived value, and `this+0x88 = TRIV_body_00b200b0()` seeds the shader model with the
renderer maximum. `FUN_008d5150` then builds the options path from
`SHGetSpecialFolderPathA(NULL, buf, CSIDL_PERSONAL, TRUE)`; when it yields an empty string the
loader falls back to the global path buffer at 00f88a3c (008d8205).

### Path A, options file present

`fopen(path, "rt")`, whole file into a memory backing (`BSP_MemoryBacking_Construct`), memory
stream, then a name/value token loop. `FUN_008d8a70` reads a token, `FUN_008d8960` consumes
it, `FUN_00438e10` and `FUN_00467cc0` are the two comparison helpers, and the value readers
are `FUN_008d99f0` (string), `FUN_008d9ad0` (int) and `FUN_008d9a80` (byte).

| Token | Literal | Reader | Field | Notes |
| --- | --- | --- | --- | --- |
| `Language` | 00d15f1c | string | via `FUN_008d56c0` | not a plain field write |
| `Fullscreen` | 00d15f10 | int | `+0x1e` | stored as `value != 0` |
| `HiResShadow` | 00d15f04 | int | `+0x1d` | stored as `value != 0` |
| `NoLOD` | 00d15efc | int | `+0x1c` | stored as `value != 0` |
| `Resolution` | 00cf3a78 | int, int | `+0x14`, `+0x18` | two values |
| `VSync` | 00d15ef4 | byte | `+0x60` | |
| `ShaderModel` | 00d15ee8 | int | `+0x88` | |
| `Antialias` | 00d15edc | int | `+0x5c = 0`, `+0x58` | index cleared first |
| `Clouds` | 00d15ed4 | byte | `+0x74` | |
| `Foliage` | 00d15ecc | byte | `+0x86` | |
| `Shadow` | 00d15ec4 | byte | `+0x84` and `+0x85` | one value, two fields |
| `Reflection` | 00d15eb8 | byte | `+0x6c` | |
| `TextureDetail` | 00d15ea8 | int | `+0x68` | |
| `ObjectDetail` | 00d15e98 | int | `+0x54` | |
| `SoundEnabled` | 00d15e88 | none | none | recognised, then `JNZ 0x008d82a0` back to the loop head with no store |
| `Firewall` | 00d15e7c | byte | `+0x94` | |
| anything else | | | | `"Options: unknown token %s"` (00d15e60) |

The decompiler's else-chain for this block is shifted by one because `SoundEnabled` has no
handler; the offsets above are read from the listing at 008d846b-008d85ff, not from the
pseudocode.

`Resolution` validates the pair with `FUN_008d46c0`; on -1 it falls back to the literal pair
`0x280 x 0x1e0` (640x480) at 008d841f. It then scans the stride-8 table at `DAT_00f8895c`
(count `DAT_00f88960`) for the index into `+0x78`, **without** breaking, so the last matching
entry wins.

### Path B, options file absent

`this+0x1e = 1` (fullscreen on), then `GetDesktopWindow` + `GetWindowRect` fill `+0x14` and
`+0x18`, logged as `"Destop size=%d %d"` (the misspelling is the original's). The same
resolution table is scanned for `+0x78`, this time **with** a break, so the first match wins.

Only then is the registry touched:

```
RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Eidos\\Battlestations Pacific", 0, 0x20019, &h)
RegQueryValueExA(h, "language", NULL, &type, buf, &cb)   // cb starts at 0x400
```

The value is used only when the call succeeds and `type == 4` (REG_DWORD). The dword is an
LCID mapped by the switch at 008d8236:

| LCID | Name |
| --- | --- |
| 0x407 | german |
| 0x40a | spanish |
| 0x40c | french |
| 0x410 | italian |
| anything else | english |

`FUN_008d6170` then derives the remaining settings from detected hardware.

### Tail, both paths

`+0x88` is re-seeded from `TRIV_body_00b200b0()` when below 1, clamped to that same maximum,
and pushed back with `TRIV_body_00b200c0`. The renderer is queried through
`(*DAT_00f8d394)->vtbl+0x104`; when `caps+0x28 < 0x200` the shadow pair `+0x84`/`+0x85` and
`+0x90` are forced to zero. `FUN_00b295c0` is called with 0x15 or 0x71 depending on `+0x8c`.
Finally the antialias sample count `+0x58` is snapped onto the stride-4 table at
`DAT_00f88968` (count `DAT_00f8896c`): the scan does not break, the index `+0x5c` is clamped
to `count - 1`, and `+0x58` is rewritten from the table.

## 00737c40 BSP_Application_CreateFileAccessLog

`void* __thiscall f(void* this, native_string subject)`, **RET 8**. The subject is a
two-dword native string passed by value; the epilogue releases its pool block through
`BSP_SizedStoragePool_GetSingleton_Provisional` / `BSP_SizedStoragePool_ReturnBlock_Provisional`.

The body is `FUN_007374a0(this, subject)` followed by `*this = &PTR_..._00cfeae0`, so the
object is four bytes: a vtable pointer only. Init allocates it with `operator new(4)` at
0073d99e and keeps the result in a stack slot; it is never stored in a global and never
freed. Registration is the base constructor's job.

The subject string is `"files.txt"`, assigned from `PTR_s__files__txt_00e0a534` through
`BSP_NativeString_Assign` at 0073d9cc.

## Globals written by this packet

| Global | Written at | Read at | Meaning |
| --- | --- | --- | --- |
| DAT_00e176c8 | 00439093, 004390d6 | nowhere | lower-cased module directory |
| DAT_0109ced4 | 00bd4fc4 | 00bd64b3 | handle to object resolver |
| DAT_0109ced8 | 00bd4fca | 00bd64e2 | scripting-context value getter |
| DAT_0109cedc | 00bd4fd0 | 00bd747e, 00bd82ef | object to handle accessor |
| DAT_00e1ae76 | 0073cf5d | 0073d975 | `cachedload`, passed to the VFS provider manager |
| DAT_00e0a538 | 0073cfab | nowhere | `nozip`; the image initialises it to 1 |
| DAT_00e1ae81 | 0073cfca | 0073daaa | `fixfps` |
| DAT_00e1ae75 | 0073cfee | 00737b4e, 00737b65, 0042592e | shared quit-requested flag |
| DAT_00e1ae77 | 0073d012, 0073d231 | 0073d98d | `filelog`, also raised by `auto mpak` |
| DAT_00e1ae80 | 0073d030 | 0068c3e7 | `freecam` |
| DAT_00f1af38 | 0073d08a | nowhere | `ip:` value |
| DAT_00f1af30 | 0073d0f9 | 0076fbd2, 0076fd27 | `localport:` value |
| DAT_00f1af34 | 0073d15a | nowhere | `connectport:` value |
| app+0x1a | 0073d39d, 0073d3b9 | 0073e1dd | memory limit, forwarded to game+0x719d |
| 00f88980 fields | 008d8190 body | across the renderer and UI | game settings |

`DAT_00e1ae75` is not a `1frame` flag in the sense of a counter. It is the same byte that
`BSP_Game_ProcessWindowCloseRequest` (004ca349) and `BSP_Game_OnQuitConfirmation` (004bbc61)
raise, and that `BSP_Application_RunFrame` polls. `1frame` simply asks to quit before the
first frame is drawn.

Three globals are written and never read: `DAT_00e176c8`, `DAT_00e0a538` and `DAT_00f1af34`,
plus `DAT_00f1af38` which has only the write-side data reference. Ghidra sees the whole image,
so these are most likely genuinely dead in the shipped build, but a computed or aliased access
would not show up as a cross reference. Treat it as strong, not conclusive.

## Gates in Init that read a bootstrap global

The parent map lists ten guards. Only three consume something this packet sets, and one more
consumes the string the parser reads.

| Guard | Site | Skipped work | Source |
| --- | --- | --- | --- |
| `DAT_00e1ae77 != 0 && DAT_0109cee8 == 0` | 0073d98d | the files.txt access log | `filelog`, or `auto mpak` |
| `DAT_00e1ae81 != 0` | 0073daaa | the `DAT_01090ab0` vtable+0x24 call | `fixfps` |
| `DAT_00e1ae78 != 0` | 0073e45c | the free of the duplicated command line | Init itself |
| `DAT_0109ceec == 0` | 0073d604 | the whole VFS block, and with it the hardware probe | not a bootstrap flag |
| `DAT_01090ab0 == 0` | 0073d4a5 | timer service construction | not a bootstrap flag |
| `DAT_0109cf00 == 0` | 0073d8ac | the 00737e20 object | not a bootstrap flag |
| `DAT_00f8abdc == 0` | 0073dcb2 | XLive array construction | not a bootstrap flag |
| `DAT_00f871b4 == 0` | 0073dd4e | the 0078c400 array | not a bootstrap flag |
| allocation non-null, 25 sites | throughout | the matching constructor | not a bootstrap flag |
| command line contains `devrr` | 0073d4ce-0073d610 | forces `devshaders` and `reloadresources` | an inline `_strstr` scan, **not** 0073ce20 |

`cachedload` is not a guard: at 0073d975 the byte is pushed into
`FUN_00bd9f90(DAT_0109ceec, byte)`, which stores it at provider-manager+0x78.

### The ordering problem worth recording

`BSP_Application_ParseCommandLine` runs at **0073d94a**, after the entire VFS block
(0073d604-0073d88d). Every flag it sets is therefore invisible to the provider manager
construction, the three mounts and both package scans. The `devrr` / `genshaders` /
`devshaders` / `hiresmode` / `reloadresources` flags that *do* need to be early are set by a
separate inline `_strstr` scan in Init at 0073d4ce-0073d610, which is why they exist as a
second, redundant parsing path. `nozip` having no reader is consistent with this: by the time
the switch is parsed, the archive providers are already registered.

## Callers and callees

All six routines have exactly one caller, 0073d410. Callees that matter:

- 00439040: `GetModuleFileNameA`, `__strlwr` (00bf8f6e), `_splitpath` (00bf8d35).
- 006ad0d0: `FUN_00bd4fc0`.
- 0073ce20: `BSP_CharacterString_AssignCounted` (00408720), `FUN_0094ec70`, `FUN_004b3fc0`,
  `FUN_004beb60`, `FUN_004c2df0`, `FUN_004cdbe0`, `_atol`, `BSP_NativeString_Assign` (0041e870),
  `FUN_004c1e90`, `FUN_00425560`, `FUN_00425570`, `FUN_004278a0`, plus the vector and iterator
  instantiations 004b5970 / 004bcdf0 / 004bce20 / 004bce50 / 004c44d0 / 004116a0 / 004072d0.
- 0073c3b0: `FUN_0098d5c0`, `FUN_0098d5f0` (both to `FUN_0098d4e0`), `FUN_00996060`,
  `FUN_00735450`, `MultiByteToWideChar`, `MessageBoxW`, `FUN_0098f430`.
- 008d8190: `FUN_008d7bc0`, `FUN_008d5150`, `BSP_MemoryBacking_Construct` (008d43c0),
  `BSP_MemoryStream_CreateFromBacking`, `FUN_008d8a70`, `FUN_008d8960`, `FUN_00438e10`,
  `FUN_00467cc0`, `FUN_008d99f0`, `FUN_008d9a80`, `FUN_008d9ad0`, `FUN_008d46c0`,
  `FUN_008d56c0`, `FUN_008d6170`, `FUN_00b295c0`, `RegOpenKeyExA`, `RegQueryValueExA`,
  `RegCloseKey`, `GetDesktopWindow`, `GetWindowRect`.
- 00737c40: `FUN_007374a0`, the two pool helpers.

## Reconstruction

`include/bsp/app_bootstrap.hpp` and `src/app_bootstrap.cpp`, registered through
`cmake/startup.cmake`. The contract's `bsp::parse_command_line` and `bsp::load_game_settings`
are spelled `parse_command_line_0073ce20` and `load_game_settings_008d8190` to match the
address-suffixed convention the rest of `include/bsp` uses.

Reconstructed with explicit arguments: `capture_module_directory_00439040`,
`install_object_handle_resolvers_006ad0d0`, `split_command_line_0094ec70`,
`parse_command_line_0073ce20`, `probe_hardware_0073c3b0`, `language_name_for_lcid_008d8190`,
`load_game_settings_008d8190`, `run_bootstrap_0073d410`.

Injected rather than reimplemented, because they are platform calls or unrecovered logic:
`HardwareProbeHost` (the four registry reads, the requirement comparison, the message box and
the options writer) and `GameSettingsHost` (the options file, the registry language value, the
desktop size, the two lookup tables, the renderer shader-model maximum and `FUN_008d6170`).
No reconstruction reads a real registry or a real file.

## Uncertainties

- The four requirement comparisons inside 0073c3b0 (0073c493-0073c7ff) are not recovered.
  Which fragment corresponds to which of CPU, memory, GPU and sound is unresolved, and the
  thresholds are not read out. The reconstruction delegates the decision to the host.
- The options-file token separator is modelled as `" \t\r\n,"`. The evidence is the literal at
  00d15f2c, which sits immediately after the `"rt"` fopen mode at 00d15f28 used by this
  routine; the tokenizer 008d9f20/008d8a70 was not opened. Provisional.
- `FUN_008d9a80`, the byte value reader, is modelled as `atoi(token) != 0`. Its exact
  acceptance set (whether it takes `true`/`yes`) is unrecovered.
- `this+0x1a` has no initialiser in Init, which sets only `this+0x18 = 1` and `this+0x19 = 0`.
  The reconstruction uses `std::optional<bool>` so that "the switch was absent" stays distinct
  from "the switch said no". Whether the constructor zeroes it was not checked.
- The three `auto` actions `FUN_00425560`, `FUN_00425570` and `FUN_004278a0`, and the
  singleton getter `FUN_004c1e90`, were not opened. The names `classes`, `scenes` and the
  `mpak` grouping are read off the literals.
- `FUN_008d56c0` (apply language) and `FUN_008d46c0` (validate resolution) were not opened.
- `+0x8c`, `+0x90` and the `FUN_00b295c0(0x15 | 0x71)` selection in the settings tail are named
  but not explained.

## What remains

- Open the four requirement comparisons in 0073c3b0 and recover the thresholds.
- Open 008d9f20 / 008d8a70 / 008d8960 and settle the options tokenizer contract, including the
  separator set and the byte reader.
- Open 00425560 / 00425570 / 004278a0 to establish what the `auto` modes actually run.
- Confirm whether `DAT_00e0a538`, `DAT_00f1af34`, `DAT_00f1af38` and `DAT_00e176c8` are truly
  dead, for example by a byte-pattern search for their addresses as immediates.
- Map the remaining fields of the 00f88980 settings object; only the ones this routine writes
  are covered here.

## State reached

| Address | Name | State |
| --- | --- | --- |
| 00439040 | BSP_Application_CaptureModulePath | exported, analyzed, reconstructed, build-tested |
| 006ad0d0 | BSP_Application_InstallObjectHandleResolvers | exported, analyzed, reconstructed, build-tested |
| 0073ce20 | BSP_Application_ParseCommandLine | exported, analyzed, reconstructed, build-tested, fixture-tested |
| 0073c3b0 | BSP_SystemOptions_ProbeHardware | exported, analyzed, partially reconstructed (registry gate and message policy only), build-tested |
| 008d8190 | BSP_GameSettings_Load | exported, analyzed, reconstructed except the renderer-caps tail, build-tested |
| 00737c40 | BSP_Application_CreateFileAccessLog | exported, analyzed; ABI and construction shape recorded, no reconstruction (the object is an empty vtable holder whose behaviour lives in 007374a0) |

None of these is ABI-compatible or game-validated. The single fixture test in
`tests/math_tests.cpp` covers the `auto` token-consumption behaviour, which is the one place
where a plausible per-token reading of the switch table gives the wrong answer.

### Correction from docs/APP_INIT_RENDERER.md

`caps+0x28` behind renderer virtual `+0x104` (`00b1ff50`, an accessor for the capability record at renderer+1B18h) is `LOWORD(D3DCAPS9.PixelShaderVersion)`, so `caps+0x28 < 0x200` means pixel shader model below 2.0, which disables shadows.

### Correction from docs/OPTIONS_SETTINGS_COMMIT.md

The Eidos registry key on the installed game holds `ApplicationDir`, `Patch` and `languages` and no value named `language`, so the loader's registry fallback for the language would not fire on this install; settings persist through the serializer `008d64a0` to the options file, never to the registry.

### Correction from docs/GAME_EXECUTABLE.md (milestone 2a)

The options-file token comparison in `008d8190` goes through `00467cc0`, which calls `BSP_CString_CompareInsensitive`; the game's own writer emits `Vsync` while the reader literal at `00d15ef4` is `VSync`, so a file the game wrote round-trips only because the comparison folds case. The reconstruction compared with `==` until the executable milestone exposed it; `src/app_bootstrap.cpp` now folds case. The supported-resolution table and shader-model ceiling come from the renderer vector that phase 4 fills.

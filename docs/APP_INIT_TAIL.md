# The Init phase tail bsp_game.exe records as unimplemented

Addresses: 0073c3b0, 00735450, 0098c7f0, 0098c800, 0098c870, 0098c890, 0098d3d0, 0098d400,
0098d470, 0098d4e0, 0098d570, 0098d590, 0098d5c0, 0098d5f0, 0098f430, 00736b60, 00736c30,
00bb4fb0, 00be0660, 00736dd0, 00736ea0, 00b80a50, 00af0b10, 00af06a0, 0073fa70, 0073e9a0,
00740410, 00740840

`docs/GAME_EXECUTABLE.md` lists 31 unimplemented host methods after the milestone 2a run. This
packet covers the five of them that belong to `cSkeletonAppMidway::Init` 0073d410 itself: the
hardware probe, the provider factory tail, the phase 6 parser registrations, the phase 8
singleton and the phase 9 decal table. The reconstruction is `include/bsp/app_init_tail.hpp`
and `src/app_init_tail.cpp`; the Lua layer the decal table needs is `docs/LUA_OBJECT_API.md`.

Every descriptive name is a hypothesis, not a recovered symbol.

## A. Phase 2 hardware probe 0073c3b0

`void __cdecl f(void)`, RET 0, an 0x11064-byte frame. Called at 0073d610 inside the
`DAT_0109ceec == 0` first-time branch, before the VFS exists.

`docs/APP_INIT_BOOTSTRAP.md` already covers the four stored reads and the all-four gate, and
`probe_hardware_0073c3b0` in `src/app_bootstrap.cpp` already runs them, delegating the decision
to a host method `failed_requirements` marked "not recovered". It is recovered here.

### The registry accessors

| Routine | ABI | Behaviour |
| --- | --- | --- |
| 0098d4e0 `BSP_HardwareRegistry_ReadValue` | `__fastcall(ECX = value name, EDX = expected REG type)`, stack (data, &cbData), RET 8 | `RegOpenKeyExA(HKLM, SOFTWARE\Eidos\BSM_HWD, 0x20019)` then `RegQueryValueExA`; true only when the call succeeds **and** the returned type equals EDX |
| 0098d5c0 `..._ReadQword` | `__fastcall(ECX = name, EDX = uint32 out)` | expected type 0x0b, 8-byte buffer, keeps the low dword only (0098d5e2) |
| 0098d5f0 `..._ReadString` | `__fastcall(ECX = name, EDX = buffer)`, stack &cbData, RET 4 | expected type 1 |
| 0098d470 `..._WriteValue` | `__fastcall(ECX = name, EDX = REG type)`, stack (data, cbData) | `RegCreateKeyExA(..., 0x20006)` then `RegSetValueExA` |
| 0098d570 `..._WriteDword` | `__fastcall(ECX = name, EDX = value)` | writes **four** bytes |
| 0098d590 `..._WriteString` | `__fastcall(ECX = name, EDX = string)` | writes `strlen + 1` bytes |

The asymmetry is real and worth keeping: the reads demand `REG_QWORD` but the write-back stores
four bytes. A key this routine wrote itself would therefore fail its own type check on the next
run, so the eight-byte values must come from the separate hardware-detection pass.

### The machine side

A detection object is constructed at 0073c467 by `009927d0` from the LCID `00992830` returns,
lives at frame+0Cc430h, and is torn down inline at 0073c8a3. Four accessors read it:

| Accessor | Reads | Returns |
| --- | --- | --- |
| 0098c870 `BSP_HardwareInfo_GpuDeviceId` | `[this+0x28]`, a negative index clamped to 0 | field +0 of the 12-byte record at `00e0cff4 + index * 12`, the device id |
| 0098c800 `BSP_HardwareInfo_MemorySize` | `[this+0x2c]` | that word |
| 0098c7f0 `BSP_HardwareInfo_CpuSpeed` | `[this+0x30]` | that word |
| 0098d3d0 `BSP_HardwareInfo_SoundDeviceName` | nothing | the fixed address `00f8a0e8` |

`0098d3d0` is two instructions, `MOV EAX, 0x00f8a0e8` and `RET`. It ignores the `ECX` the probe
passes it, so the detected sound device is a process-wide buffer, not a field of the detection
object. `0098c890 BSP_HardwareInfo_GpuDeviceName`, `__fastcall(ECX = device id)`, walks the same
12-byte records comparing field +0 with `ECX`, stops at the first record whose name field +4 is
null, and returns field +4 of the match or of record 0 when nothing matched.

### The decision

Four independent comparisons, in this order. Each contributes one localized fragment; the
enumerator value is the message index passed to `00996060`.

| Check | Site | Test | Message arguments |
| --- | --- | --- | --- |
| GPU device | 0073c49f | `0098c870() != GPUDeviceID` | `0098c890(stored)`, `0098c890(current)` |
| Sound device | 0073c5cb | `00449af0` NotEqualCaseInsensitive over the stored string and `0098d3d0()` | stored name, current name |
| CPU speed | 0073c6fc | `0098c7f0() != CPUSpeed` | the two raw integers |
| Memory | 0073c763 | `0098c800() != MemSize` | the two raw integers |

Both string pairs are ordered stored value first, machine second. The two numeric checks push
the integers straight into `_vswprintf_p_l`, so the localized format string owns their spelling.

### The message and the two side effects

If no check fired, 0073c7a4 falls straight to teardown: no message box, and **nothing is written
back**. Otherwise `00735450` formats `L"%s%s%s%s\n%s"` (00cff128) over the four fragment
buffers in the fixed order GPU, sound, CPU, memory, then message 7. A check that did not fire
contributes an empty buffer, so the fragments are not separated from each other and the newline
is always present. `MessageBoxW(NULL, text, message 6, MB_YESNO | MB_ICONEXCLAMATION)` runs at
0073c827.

Two side effects follow, and only the first is conditional:

1. `IDYES` (6) calls `0098f430("options.txt")`, `__thiscall` on the detection object, which is
   `fopen(path, "wb")` and generated defaults.
2. 0073c843..0073c89e writes the machine's four values back into the registry **on both
   answers**, in the order MemSize, CPUSpeed, GPUDeviceID, SoundDevice. So the warning appears
   exactly once per hardware change whether or not the user accepts the rewrite.

Neither the existing `HardwareProbeResult::defaults_declined` path nor the milestone's host
models that write-back; see Corrections.

### Host methods for phase 2

| Host method | Native site | Must do |
| --- | --- | --- |
| `current_gpu_device_id` | 0073c49a | the detected adapter's device id |
| `gpu_device_name(id)` | 0073c4b4, 0073c4cb | the table lookup above |
| `current_sound_device` | 0073c5b5 | the global at 00f8a0e8 |
| `current_cpu_speed` | 0073c6f7 | `[detection+0x30]` |
| `current_memory_size` | 0073c75e | `[detection+0x2c]` |
| `localized_message(index, a, b)` | 0073c55b + 0073c56e | table row `0098d400(LCID) + index * 6`, formatted |
| `ask_write_default_options(text, caption)` | 0073c827 | `MessageBoxW`, true only for 6 |
| `write_default_options_file(path)` | 0073c83e | `fopen(path, "wb")` |
| `write_hardware_dword(name, value)` | 0073c856, 0073c86e, 0073c886 | 4-byte `RegSetValueExA` |
| `write_hardware_string(name, value)` | 0073c89e | `strlen + 1` `RegSetValueExA` |

`run_hardware_probe_tail_0073c3b0` is the sequence over those ten; `HardwareProbeHost::failed_requirements`
in the existing bootstrap can be implemented by calling `hardware_probe_changes_0073c3b0`.

## B. The provider factory tail, 0073d94f-0073d98d

Five calls, in order, each re-reading the manager global rather than caching it:

```
0073d955  00736b60  -> the mpak provider factory singleton
0073d95d  00be0660  -> manager.factories.push_back(that)
0073d968  00736c30  -> the pak archive registry singleton
0073d970  00bd9230  -> manager+0x88 = that
0073d983  00bd9f90  -> manager+0x78 = DAT_00e1ae76, the cachedload flag
0073d988  00bb40b0  -> publish the process-wide pak lock at DAT_010904e0
```

`00736b60`, `__cdecl void*(void)`, is the same lazy double-checked singleton as the already
concrete mpkg factory `00736a90`: an 8-byte object with a primary vtable at +0 and a secondary
base vtable at +4, registered with the singleton lifetime manager at instance+4.

| | mpkg 00736a90 | mpak 00736b60 |
| --- | --- | --- |
| cached global | 010904f4 | 010904d4 |
| primary vtable | 00cfea14 | 00cfea20 |
| secondary vtable | 00cfea10 | 00cfea1c |
| vtable slot 0 | 00737090 dtor | 007370d0 dtor |
| vtable slot 1, Create | 00bb9d90 | 00bb83a0 |

The only behavioural difference is slot 1. The mpak Create `00bb83a0` accepts a path only when
it is longer than four characters and its last five bytes compare equal to `.mpak` without
case; on a match it enters the lock at `DAT_010904e0` and returns the cached provider at
`DAT_010904dc` or builds a 0x44-byte one through `00bb8240`. That Create body is a separate
packet: it pulls in 00bb8240, 00bb82f0 and the provider cache.

`00be0660 BSP_VFS_RegisterProviderFactory`, `__thiscall(ECX = manager, [esp+4] = factory)`,
RET 4, is a plain `std::list::push_back` onto the list at manager+30h/+34h through `00bdab20`
and `00bdece0`. No deduplication, no reference count, no ownership, and the order is the call
order: physical, FileStore, MPKG, then MPAK from this tail. The node is `_Next`, `_Prev`, then
the raw pointer at +8.

`00736c30 BSP_PakArchiveRegistry_GetSingleton` is the same lazy shape over `DAT_010904d8` with
`operator new(0x1c)`, constructor `00bb4fb0` and the lifetime sub-object at instance+**8**
rather than +4. The 1Ch object:

| Offset | Value | Note |
| --- | --- | --- |
| +00 | vtable 00d64190 | six slots, base pair 00ceb130 / 00d64188 first |
| +04 | 1 | |
| +08 | vtable 00d6418c | one adjustor slot, `sub ecx,8; jmp 00bb5410` |
| +0C | 0x64 | copied into every PAK provider by 00bb8240 (provider+18h) |
| +10, +14, +18 | 0 | list fields |

No extension string lives in the registry; `.mpak` belongs to the factory Create.

### Host methods for the factory tail

| Host method | Native site | Must do |
| --- | --- | --- |
| `mpak_factory_00736b60` | 0073d955 | one 8-byte singleton at 010904d4 whose Create matches `.mpak` |
| `register_provider_factory_00be0660` | 0073d95d | append, no dedup, order preserving |
| `pak_archive_registry_00736c30` | 0073d968 | one 1Ch singleton at 010904d8 with the table above |
| `set_manager_pak_registry_00bd9230` | 0073d970 | manager+0x88; reuse `bsp::set_file_manager_provider_policy_00bd9230` in `src/platform_window.cpp` |
| `create_pak_registry_lock_00bb40b0` | 0073d988 | publish one lock; reuse `bsp::create_shared_lock_00bb40b0` in `src/platform_window.cpp` |

Two of the five already have reconstructed fragments and only need wiring. `00bd9f90` is
already concrete in the milestone.

## C. Phase 6, the two resource type parsers

`0073db41..0073db69`, straight-line, with the manager fetched once per registration:

```
0073db41  004c1400   -> resource manager singleton (010901c4)
0073db48  00736dd0   -> the AnimationChannels parser singleton
0073db50  00b80a50   -> register, AL discarded
0073db55  004c1400   -> the manager again, not cached
0073db5c  00736ea0   -> the Bone parser singleton
0073db64  00b80a50   -> register, AL discarded
```

Both parsers are 8-byte singletons with no data: primary vtable at +0, lifetime sub-object
vtable at +4.

| | AnimationChannels | Bone |
| --- | --- | --- |
| accessor | 00736dd0 | 00736ea0 |
| cached global | 01090298 | 0109029c |
| primary vtable | 00cfea38 | 00cfea48 |
| slot 0, dtor | 00737150 | 00737190 |
| slot 4, type name | 00b8b050 | 00b8b080 |
| slot 8, create item | 00b8a910 | 00b8a990 |

The registration key is neither a four-character code nor a file extension: it is the C++
string literal slot +4 returns, `AnimationChannels` and `Bone`. `00b8b050` is 33 bytes, ignores
the incoming `ECX` and returns the hidden output-string pointer, RET 4.

`00b80a50 BSP_ResourceManager_RegisterTypeParser`, `__thiscall(ECX = manager,
[esp+4] = parser)`, RET 4, bool in AL, is append-only into the parser map at manager+8: it
probes with `00b7e740`, returns false and keeps the incumbent on a hit, and on a miss asks the
parser for its name a **second** time before building the pair and inserting. It reports true
without inspecting the insertion result, so a parser whose second name differs can collide and
still be reported as registered. `StructuredResourceRegistry::register_parser` in
`src/structured_resource_registry.cpp` is that routine and is reused, not redone.

`004c1400` is documented only, as the packet requires: `__cdecl void*(void)`, RET, returning the
global at 010901c4, constructing a 0x28-byte manager through `00b81040` with vtable 00d63128 on
first call. It is unleased and no `src/` file reconstructs it.

### Host methods for phase 6

| Host method | Native site |
| --- | --- |
| `resource_manager_004c1400` | 0073db41, 0073db55, and it must return the same object both times |
| `animation_channels_parser_00736dd0` | 0073db48 |
| `bone_parser_00736ea0` | 0073db5c |
| `register_type_parser_00b80a50(manager, parser)` | 0073db50, 0073db64 |

The singleton lifetime registration inside each accessor is observable but not required for
phase 6 to complete; skipping it only changes teardown.

## D. Phase 8, 00af0b10

`00af0b10` is already reconstructed in `src/world_effects_startup.cpp` and is not redone here.
Its base constructor `00af06a0` was not, and it turns out to be the same routine as the decal
system's base constructor `0073fa70`:

```
install the base vtable          00af06c8 = 00d5d7ec   0073fa98 = 00cff218
00415350, enter manager+0x10     00af06e7              0073fab7
publish the instance             00af06f6 -> 00f8c274  0073fac6 -> 00e1aea0
00bd0c30 register, re-reading that global
                                 00af070a/00af0713     0073fad1
leave                            00af0718              0073fae4
```

The publication global is the only handle: `0073e02b` never stores the constructor's `EAX`
anywhere. `publish_startup_singleton_00af06a0` is the four steps; `00bd1860`, `00415350` and
`00bd0c30` are already reconstructed in `src/random_threads.cpp` and `src/singleton_lifetime.cpp`.

Field evidence contradicts the milestone's label for this site. `00af0b10` writes +2Ch from
`[00d7a24c]` = 1.0f, which `00af0450` and `00af0460` feed into shader constant c33, and
`00af0c50` walks the +4/+8 group array from `BSP_Game_OnMove`; a separate settings key
`FoliageGroup` exists at `008d5b50`. The strings `FloatingParticles` and `ParticleFloating`
sit next to the vtable in .rdata but are consumed by `00af0f20`, which is never called with
this instance. It is a foliage group manager; "world effects startup" is the packet name for
the seven phase 9/10 addresses, not a name for this one.

Two host methods are enough for this step: `create_critical_section` (00af0b68 -> 00bd1860) and
the publication above (00af06f6 plus 00af070a).

## E. Phase 9, the decal definition table 00740840

`__thiscall(ECX = the 1Ch decal system allocated at 0073de6d)`, RET, EAX = this. Called at
0073de8c.

`0073fa70` runs first and installs the base vtable and the 00e1aea0 publication; 00740872 then
overwrites +0 with the derived vtable 00cff2a0 and zeroes +04h..+18h. The loader builds its own
Lua state owner at frame+0x90 (00b66bd0 at 007408a0), opens it with mask 1 (00b6a020 at
007408b6), and runs `scripts/datatables/decals.lua` through 00b69d40 at 00740903. The pushed
string length is 0x1d, exactly the 29 bytes of that path.

The walk, in frame-relative terms (`B` = ESP after the prologue): globals object at B+0x7c, the
`Decals` table at B+0x68, the key at B+0x54, the value at B+0x40 and one reused field temporary
at B+0x2c.

- `00b660a0` at 007409b9 tests the **key**, not the value, and the key's string is the record
  name (00b662b0 at 007409f9).
- The 0x28-byte record is allocated and constructed by `00740410` *before* the name is read, so
  an entry with a non-string key allocates nothing.
- `00b66420` at 007409a8 and 00740d6d tests the **value** for the end of iteration.

| Record offset | Key | Accessor | Site |
| --- | --- | --- | --- |
| +00, +04 | the table key | 00b662b0 | 007409f9 |
| +0C | `Size` (00cff278) | 00b66270 | 00740ab9 |
| +10 | `Radius` (00ce5b54) | 00b66270 | 00740aef |
| +08 | `Maxnum` (00cff270) | 00b66290 | 00740b25 |
| +18 | `Texture` (00ce5fec) | 00b662b0 then renderer `00f8d394` vtable +64h with (name, 0) | 00740b5b |
| +1C | `Shader` (00cff268) | 00b662b0 then renderer vtable +48h with (name) | 00740c14 |
| +20 | `LifeTime` (00cff25c) | 00b66270 | 00740cc9 |
| +24 | `FadeOutTime` (00cff250) | 00b66270 | 00740cff |
| +14 | none | `decal.mvfm` through renderer vtable +34h, set by 00740410 | 00740454 |

Presence is never tested, so a record whose table omits a key keeps the zero `00740410` left
there and reads an empty texture or shader name.

Records go into a three-word doubling vector on the system object: base +10h, count +14h,
capacity +18h. At 00740d1d, `count == capacity` asks `0073e9a0 BSP_DecalRecordVector_Reserve`
for `max(1, capacity * 2)`; `0073e9a0` itself floors its argument to 1, returns without work
when the capacity already covers the request, and otherwise mallocs `capacity * 4` bytes,
copies the live elements and frees the old block.

Teardown at 00740d7a releases value, key, the `Decals` table and finally closes the state owner
with `00b669a0`.

### Fixture result

`src/decal_table_probe.cpp` runs the reconstruction over the installed
`scripts/datatables/decals.lua` with the repository's stock Lua 5.1.1 (`cmake/lua.cmake`,
`bsp::GuiLua51Host`). It parses **3 decal definitions**, and every field matches the installed
file exactly:

| Name | Size | Radius | Maxnum | Texture | Shader | LifeTime | FadeOutTime |
| --- | --- | --- | --- | --- | --- | --- | --- |
| machine_gun_decal | 0.33 | 2 | 130 | white.tga | decal.mshd | 15 | 2 |
| explosion_hole | 4 | 2 | 10 | white.tga | decal.mshd | 30 | 2 |
| landscape_hole | 5 | 2 | 100 | white.tga | decal.mshd | 30 | 2 |

The probe returns 5 if any record comes back with an empty name, texture or shader or a zero
count, so a walk that lost a field fails rather than reporting a smaller table. Iteration order
is `lua_next` order and is not the file order.

### Host methods for phase 9

| Host method | Native site | Must do |
| --- | --- | --- |
| `construct_lua_state_owner` | 007408a0 | one owner per load |
| `open_lua_libraries(1)` | 007408b6 | 00b6a020 with mask 1 |
| `run_script(path)` | 00740903 | `scripts/datatables/decals.lua` through the VFS |
| `load_vertex_format("decal.mvfm")` | inside 00740410 | renderer 00f8d394 vtable +34h, once per record |
| `load_texture(name, 0)` | 00740b90 area | renderer vtable +64h |
| `load_shader(name)` | 00740c50 area | renderer vtable +48h |
| `publish_decal_system` | 0073fac6 | the store to 00e1aea0 and the 00bd0c30 registration |

The parse between those calls is `load_decal_definitions_00740840` and needs no host at all.

## no_ghidra_function

Four adjustor thunks reachable only through the vtables above have no Ghidra function. Each is
`sub ecx, N; jmp <destructor>` followed by int3 padding. The integrator can define them with
`python tools/ghidra_define_function.py <start> <end_exclusive>`.

| Start | Inclusive end | Body | Belongs to |
| --- | --- | --- | --- |
| 00735d00 | 00735d07 | `sub ecx,4; jmp 00737090` | mpkg factory secondary vtable 00cfea10 |
| 00735d30 | 00735d37 | `sub ecx,4; jmp 007370d0` | mpak factory secondary vtable 00cfea1c |
| 00735d90 | 00735d97 | `sub ecx,4; jmp 00737150` | AnimationChannels parser lifetime vtable 00cfea34 |
| 00735dc0 | 00735dc7 | `sub ecx,4; jmp 00737190` | Bone parser lifetime vtable 00cfea44 |

One flow gap, not a missing function: Ghidra leaves 00740a0d..00740a0f undisassembled inside
00740840, between the `LEA EDX,[EAX+1]` at 00740a08 and the strlen loop at 00740a10. The
pseudocode still reconstructs the loop correctly, so this is cosmetic;
`python tools/ghidra_flow_repair.py 00740840 --apply` would close it.

## Corrections

- **docs/GAME_EXECUTABLE.md** calls the host method at 00740840 `Phase 9 game_entry`, and its
  "Next milestones" section reads it as "game entry 00740840 into game state 2, the front-end
  screen sets". 00740840 is the decal definition loader, as `docs/APP_INIT_WORLD_EFFECTS.md`
  and the reconstruction here both show; it enters no game state and touches no screen set.
  The label should be `Phase 9 decal_definitions`. `src/game_hosts.cpp:910` logs the same wrong
  label.
- **docs/APP_INIT_WORLD_EFFECTS.md**, the 00740840 section, says the loader makes a record "for
  each element whose value is a table (00b660a0)". 00b660a0 at 007409b9 runs on the **key**
  temporary at frame+0x54 and tests `lua_type == LUA_TSTRING`; the value is never type-tested.
  An entry whose key is a string but whose value is a number would still allocate a record and
  read seven nil fields.
- **docs/APP_INIT_BOOTSTRAP.md** lists `SoundDevice` as read by wrapper `0098d5f0` into a
  0x400-byte buffer, which is right, but its table gives the wrapper for that row as 0098d5f0
  while the routine that reads the *current* device, 0098d3d0, is described as an object
  accessor. 0098d3d0 takes no input: it returns the constant address 00f8a0e8.
- **src/app_bootstrap.cpp**, `probe_hardware_0073c3b0`: it joins the failure lines with no
  separator and appends `'\n'` last. The native format is `L"%s%s%s%s\n%s"`, so a fifth
  localized line (message 7) follows the newline. The function also models no registry
  write-back, and the write-back at 0073c843 runs on the decline path too, so
  `HardwareProbeResult::defaults_declined` currently implies "nothing changed" when the native
  has already updated the stored profile. Both belong to that file's owner, not to this packet.

## Follow-up packets

- `mpak_provider_create`: 00bb83a0, 00bb8240 and 00bb82f0, the `.mpak` provider behind the
  factory this packet only identifies.
- `resource_manager_singleton`: 004c1400 and 00b81040, currently documented only.
- `resource_item_parsers`: slot +8 of the two phase 6 parsers, 00b8a910 and 00b8a990.
- `hardware_detection_object`: 009927d0 and 00992830, the object the probe queries, including
  what fills `[this+0x28]`, `[this+0x2c]` and `[this+0x30]` and who writes 00f8a0e8.
- `decal_system_runtime`: the derived vtable 00cff2a0 and the consumers of the record vector.

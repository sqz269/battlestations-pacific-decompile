# Application initialize: fonts and GUI (packet `app_init_fonts_gui`)

Addresses: 0073bae0, 007371d0, 0073c960, 004c14c0, 004c12b0, 0053bc00, 00aa5e20,
00aa5d70, 00be9620, 00be9760

Every name in this document is a hypothesis, not a recovered symbol. The packet covers the
fonts/GUI half of the map's `app_init_locale_gui`; the language selection and the two
`lockit` loads are in `docs/APP_INIT_LOCALE.md`. Ghidra was read-only for this packet: no
rename, comment, prototype or save was applied. Analysis batches verified project `bsp` and
program `/battlestationspacific.exe`.

## Where the packet sits in `BSP_Application_Initialize`

Disassembled at `0073e030`-`0073e155`:

| Address | Instruction / call | Meaning |
| --- | --- | --- |
| 0073e030 | `push 0x4040; call 00bf681b` | CRT allocation of the locale-table manager |
| 0073e057 | `mov ecx,eax; call 0073c960` | its constructor, skipped when the allocation returned null |
| 0073e06a..0073e135 | `008d4870`, `00aa09d0`, `00aa0d30`, `00aa06d0` | the locale half, `docs/APP_INIT_LOCALE.md` |
| 0073e13c | `call 0073bae0` | the whole fonts + GUI bring-up below |
| 0073e141/0073e146 | `mov ecx,0x00cff1a8; call 004c9c90` | the `After InitGui` memory checkpoint |
| 0073e14b | `call 004c14c0` | diagnostic sink getter, result discarded |
| 0073e150 | `push 0x71a0` | phase 11 begins |

So the GUI manager is created inside `0073bae0`, before the checkpoint, not after it. The
only thing that happens at the checkpoint itself is the label push and one stripped
diagnostic call.

## 0073bae0 `BSP_FontSystem_LoadDefinitions`

`__cdecl void f(void)`, `RET` (0 bytes popped), body `0073bae0`-`0073bc33`. Frame is
`SUB ESP,0x18` plus the three-dword SEH record (handler `00c867f8`), so the epilogue is
`ADD ESP,0x24`. Three native strings live in that frame; call it `S` = ESP after
`PUSH EBX` at `0073baf8`:

| Slot | `{size, data}` | Content |
| --- | --- | --- |
| `S+0x04` / `S+0x08` | 6 | `Fonts\` (`00cfefa8`) |
| `S+0x0c` / `S+0x10` | 0xf | `Fonts/Fonts.lua` (`00cff088`) |
| `S+0x14` / `S+0x18` | from `008d4890` | language font path |

Recovered sequence:

1. `008d4890` `BSP_GameSettings_GetLanguageFontPath` with `ECX = 00f88980`, the settings
   object, and `0041e870` `BSP_NativeString_Assign` copies the result into `S+0x14`. For
   the shipped English build the language descriptor has no `fontpath` key, so this string
   is empty (`docs/APP_INIT_LOCALE.md`).
2. `0041dd40` `BSP_NativeString_Resize` + `_memcpy` build `Fonts/Fonts.lua` and `Fonts\`.
3. `0073bb7e`-`0073bb92` pushes the three string addresses in the order
   *language path, descriptor, root*, then `call 007371d0`. **`007371d0` ends in a plain
   `RET` at `0073728c` and takes no arguments**: the three pushes belong to the *next*
   call. Ghidra's pseudocode shows them as arguments of `007371d0`, which is wrong.
4. `mov ecx,eax; call 00ac3910` `BSP_FontRegistry_LoadLuaDescriptors`, `__thiscall` with
   `ECX` = the font registry and three stack arguments, `RET 0xc` at `00ac3edc`. Argument
   order at the callee is therefore `(root "Fonts\", descriptor "Fonts/Fonts.lua",
   language font path)`. Already reconstructed; see `docs/FONT_GLYPH_SOURCE.md` and
   `src/font_registry.cpp`. Nothing about it is repeated or duplicated here.
5. The three strings are destroyed in reverse order through
   `00419cc0` `BSP_SizedStoragePool_GetSingleton` + `00bd1510` `BSP_SizedStoragePool_ReturnBlock`,
   each with `size+1` bytes.
6. `call 0053bc00; mov ecx,eax; call 00be9620` - the fallback glyph-width singleton and a
   `this+4` base adjustment. **The result is discarded**, so the pair exists only to force
   the singleton into existence at this point in startup.
7. `call 004c12b0; mov ecx,eax; call 00aa5e20` - `BSP_GuiManager_GetOrCreate` followed by
   the GUI manager's resource load.

The function has no branches other than the three null checks in the string destructors,
and no return value.

### GUI-manager dependency

`004c12b0` `BSP_GuiManager_GetOrCreate` (`__cdecl int f(void)`, `RET`, body
`004c12b0`-`004c136f`) is the standard double-checked singleton of this codebase: test
`DAT_00f8bc5c`, take the lifetime manager's optional critical section at
`BSP_SingletonLifetime_GetManager()+0x10`, re-test, `00bf681b(0x88)`, construct with
`00aa5d70`, register with `00bd0c30`. It already carried a reviewed ledger name before this
packet. `0073bae0` is the site that first creates it during startup; the earlier `InitGui`
label in the map names the phase, not a separate call.

### 00aa5d70, the `cGuiManager` constructor (0x88 bytes)

`__fastcall`, `ECX` = the object, returns it, `RET`, body `00aa5d70`-`00aa5e18`. Vtable
`00d5bfcc` (`CG_scalar_deleting_dtor_00aa6540`). It stores a node object from `00aa2920` at
`+0x0c`, sets that node's byte `+0x15` to 1 and self-links its three pointers `+0`/`+4`/`+8`
(a list/tree sentinel), zeroes `+0x10`, `+0x18`..`+0x30`, stores `00aa2820`'s result at
`+0x38`, zeroes `+0x3c`, `+0x40`, byte `+0x48` and `+0x6c`, and copies the float constant
`DAT_00ce3800` into `+0x5c` and `+0x60`. It leaves `+0x4c`, `+0x50`, `+0x54`, `+0x58`,
`+0x74`..`+0x84` uninitialised; those are exactly the fields `00aa5e20` fills, which is the
ordering evidence that the resource load must run once after construction.

### 00aa5e20, the GUI manager resource load

`__fastcall void f(void *this)`, `RET`, body `00aa5e20`-`00aa633e`. Six blocks are reported
unreachable by the decompiler (`00aa613a`, `00aa6172`, `00aa6228`, `00aa6260`, `00aa629f`,
`00aa62da`) and several string literals are dropped from the pseudocode; the literal
addresses were read from the disassembly and the bytes confirm each length.

Two factories appear, and the `ECX` they are called with is the structural fact: `00aa5840`
always takes the **manager** and creates a group; `00aa7e00` always takes a **group** and
creates a child of it. The decompiler's flattened output loses that nesting.

| Order | String (address, length) | Call | `ECX` | Stored at | Follow-up |
| --- | --- | --- | --- | --- | --- |
| 1 | `data/interface/textures/whiteGui.tga` (`00d5c084`, 0x24) | renderer `*(00f8d394)` vtable `+0x64` | renderer | `+0x28` | only when `BSP_VFS_ResolveExistingName` accepts the name |
| 2 | `interface/textures/common/transparent.tga` (`00d5c058`, 0x29) | same vtable `+0x64` | renderer | `+0x2c` | previous value released through `InterlockedDecrement` on `+4`, vtable `+0` at zero |
| 3 | `_Mouse` (`00d5c050`, 6) | `00aa5840(&name, 1, 0)` | manager | `+0x4c` | virtual `+0x34`(1) at `00aa5faf` |
| 4 | `MousePtrFE_Icon` (`00d5c040`, 0xf) | `00aa7e00(&name, 1)` | `[+0x4c]` | `+0x54` **and** `+0x50` | virtual `+0x34`(0) at `00aa60a3`, after entry 5 |
| 5 | `MousePtrGUI_Icon` (`00d5c02c`, 0x10) | `00aa7e00(&name, 1)` | `[+0x4c]` | `+0x58` | virtual `+0x34`(0) at `00aa6098` |
| 6 | `_Highlight` (`00d5c020`, 0xa) | `00aa5840(&name, 1, 0)` | manager | not stored, kept in `EDI` | virtual `+0x34`(1) at `00aa6117` |
| 7 | `hl_FrameBox` (`00d5c014`, 0xb) | `00aa7e00(&name, 1)` | `EDI` | `+0x74` | virtual `+0x34`(0) |
| 8 | `hlCircle_FrameBox` (`00d5c000`, 0x11) | `00aa7e00(&name, 1)` | `EDI` | `+0x78` | virtual `+0x34`(0) |
| 9 | `safezone_43_FrameBox` (`00d5bfe8`, 0x14) | `00aa7e00(&name, 1)` | `EDI` | `+0x7c` | virtual `+0x34`(0) |
| 10 | `safezone_169_FrameBox` (`00d5bfd0`, 0x15) | `00aa7e00(&name, 1)` | `EDI` | `+0x80` | virtual `+0x34`(0) |

So the `_Highlight` group itself is never stored on the manager: only its four children are.
Byte `+0x84` is set to 0 last. Every name string is built with
`BSP_NativeString_Resize` + `_memcpy` and released through the sized storage pool, the same
idiom as `0073bae0`.

### 007371d0, the font registry singleton

`__cdecl int f(void)`, plain `RET` at `0073728c`, body `007371d0`-`0073728c`. Same
double-checked shape as `004c12b0` over the global `DAT_00f8bf44`, allocating 0x10 bytes and
constructing with `00ac3690`. `docs/FONT_GLYPH_SOURCE.md` already establishes the object:
list sentinel at `+8`, count at `+0xc`, and `00ac3570` is the by-name lookup. Its other two
callers are `008d5b50` `BSP_Settings_ApplyAll` and `00ab8c30`, the text-context font setter.

### 0053bc00 / 00be9760 / 00be9620, the fallback glyph-width singleton

`0053bc00`: `__cdecl int f(void)`, `RET`, body `0053bc00`-`0053bcbf`; the same
double-checked singleton over `DAT_0109db78`, allocating 0x204 bytes and constructing with
`00be9760`.

`00be9760` (`__fastcall`, returns the object, body `00be9760`-`00be988b`) sets vtable
`00d68bb0`, builds the 0x11-byte name `fonts/arial19.dat`, opens it through the file
manager `*(DAT_0109ceec)` vtable `+4` with mode 2, and requires the stream length from
virtual `+0x30` to be **exactly 0x200**. On a match it reads 0x200 bytes into `this+4`
through virtual `+0x24` and post-processes them with `00be9630(this+4, 00e15120)`; on a
mismatch it releases the stream and zeroes the first byte at `this+4`. So the object is a
4-byte header plus a fixed 512-byte table loaded from `fonts/arial19.dat`.

`00be9620` is `__fastcall int f(void *this) { return this + 4; }`, body
`00be9620`-`00be9623` - a base/interface pointer adjustment onto that table.

The call pair in `0073bae0` throws the adjusted pointer away, so at this point in startup it
is a preload, not a use. What the 512 bytes mean (256 entries of 2 bytes is the obvious
reading for a width table, and the name is a font data file) was **not** established.

## 0073c960, the locale-table manager constructor

`__fastcall undefined4 * f(void *this)`, returns `this`, plain `RET` at `0073c9ec`, body
`0073c960`-`0073c9ec`. The caller allocates 0x4040 bytes at `0073e030`.

The object is **not** a new GUI structure: `00736540` `BSP_Singleton_Register_00f8bc4c`
publishes it in `DAT_00f8bc4c`, which is the localisation manager `docs/APP_INIT_LOCALE.md`
already documents. `00736540` runs first (with the same `ECX`), stores the singleton base
vtable `00cfea5c` and registers the object under the lifetime manager's lock; `0073c960`
then overwrites `*this` with the derived vtable `00cff140`
(`CG_scalar_deleting_dtor_0073caf0`) and initialises the rest.

### Layout of the 0x4040-byte object

Fields the constructor writes are marked; the three it leaves alone are the leading dword of
each 16-byte vector.

| Offset | Size | Field | Written by 0073c960 | Evidence |
| --- | --- | --- | --- | --- |
| +0x0000 | 4 | vtable, `00cff140` | yes | `MOV [ESI],0xcff140` at `0073c982` |
| +0x0004 | 4 | vector allocator/pool word | no | `00450540` treats `manager+4` as the vector object and reads `+4`/`+8`/`+0xc` of it |
| +0x0008 | 4 | registered table names, first | yes | `0073c988` |
| +0x000c | 4 | registered table names, last | yes | `0073c98b` |
| +0x0010 | 4 | registered table names, capacity end | yes | `0073c98e` |
| +0x0014 | 0x4000 | 0x1000 hash bucket heads, 4 bytes each | yes, `_memset(this+0x14, 0, 0x4000)` | `0073c991`-`0073c99b`; bucket index is `hash & 0xfff` in `00a9fc30`/`00a9ec70` |
| +0x4014 | 4 | live entry count | yes | `0073c9a3` writes `[EBX+0x4000]` where `EBX = this+0x14` |
| +0x4018 | 4 | current language name, size | yes | `0073c9a9` |
| +0x401c | 4 | current language name, data | yes | `0073c9af` |
| +0x4020 | 4 | `.lanx` vector A, allocator word | no | - |
| +0x4024 | 4 | `.lanx` vector A, first | yes | `0073c9b5` |
| +0x4028 | 4 | `.lanx` vector A, last | yes | `0073c9bb` |
| +0x402c | 4 | `.lanx` vector A, capacity end | yes | `0073c9c1` |
| +0x4030 | 4 | `.lanx` vector B, allocator word | no | - |
| +0x4034 | 4 | `.lanx` vector B, first | yes | `0073c9cb` |
| +0x4038 | 4 | `.lanx` vector B, last | yes | `0073c9d1` |
| +0x403c | 4 | `.lanx` vector B, capacity end | yes | `0073c9d7` |

4 + 16 + 0x4000 + 4 + 8 + 16 + 16 = 0x4040 exactly.

**Table identity, element size and count.** The 0x4000-byte region is the bucket array of the
localisation string map, 0x1000 heads of one 4-byte node pointer each. Element size and count
come from two independent places: `0073c240` `BSP_StringMap_Clear` loops 0x1000 chains and
then memsets 0x4000 bytes, and both `00a9fc30` `BSP_StringMap_InsertOrGet` and `00a9ec70`
`BSP_StringMap_Find` mask their hash with `0xfff`. Nodes are 0x1c bytes, chained at the
head. Readers and writers of the object reached through `DAT_00f8bc4c` are, by
`ghidra xrefs 00f8bc4c`: the loaders `00aa09d0` / `00aa06d0` / `00aa0d30`, the lookups behind
`00a9fad0`, `BSP_TextContext_UpdateUtf16Geometry` (`00aba911`),
`BSP_TextContext_EllipsizeUtf16ToWidth` (`00ab8f84`), `FUN_00abaed0`, `FUN_00abb000`,
`BSP_Settings_ApplyAll`, `FUN_008d5030`, and a set of front-end screens
(`FUN_00690fd0`, `FUN_005d0050`, `FUN_00a40510`, `FUN_006003a0`, `FUN_00510c20`,
`FUN_00438020`, `FUN_0057c360`). All of them are text lookups; none of them treats the
0x4000 bytes as anything but the map.

This refines the layout table in `docs/APP_INIT_LOCALE.md`, which lists the two `.lanx`
vectors as `+0x4020..+0x402f` and `+0x4030..+0x403f` without separating the leading word from
the three pointers.

### Lifetime pairing

`docs/APP_SHUTDOWN.md` rows 13 and 17: `DAT_00f8bc4c` is released in the step-13 chain and
then, at `007382df`-`00738304`, the shutdown re-tests the global, unregisters it from the
lifetime manager, destroys it and nulls the global. The unregister half of the constructor's
`00736540` is `007365e0` `BSP_Singleton_Unregister_00f8bc4c`, which the shutdown document
lists as the pair. The class destructor itself is `0073caf0`, reached through vtable slot 0
at `00cff140`.

## 004c14c0, the 4-byte singleton at the `After InitGui` checkpoint

`__cdecl undefined4 * f(void)`, `RET`, body `004c14c0`-`004c1569`. Same double-checked
pattern over `DAT_0109cf14`, but the "constructor" is inlined and is a single store: allocate
4 bytes with `00bf681b(4)` and write `&PTR_LAB_00ce752c`. The object therefore has **no state
at all** beyond its vtable pointer.

**The vtable.** `00ce752c` holds one slot, `0x004bbca0`; the dword after it (`00ce7530`) is
zero and the next (`00ce7534`) is the float `0x40a77000`, i.e. unrelated data, so the table is
one entry long. `004bbca0` is the scalar deleting destructor: `test byte [esp+4],1`,
`mov dword [0x109cf14],0`, `mov [esi],0xce3818` (restore the singleton base vtable),
conditional `free`, `ret 4`. It clears its own global without taking the lifetime lock,
unlike `007365e0`. There is no RTTI: the dword before the vtable is `0x64`, part of the
adjacent literal `Cloud`, so the class name is not recoverable from the image.

**What its construction does.** Nothing but the allocation, the vtable store and
`BSP_SingletonLifetime_Register`. No file, no device, no other object.

**What it is for.** All 20 call sites (`ghidra xrefs 004c14c0`) discard the return value, and
each sits immediately after a formatted message or on a failed precondition:

| Site | Shape |
| --- | --- |
| `004c70ac` in `FUN_004c7070` | `_vsprintf_s(buf,0xff,fmt,va)` then `FUN_004b8410(out,"%s\n",buf)` then the call; the formatted text is dropped |
| `004c7103` in `FUN_004c70c0` | same helper with a 0x1ff8-byte frame |
| `00be3e46` in `FUN_00be3bf0` | `sprintf(buf, "%s %.2f ms count:%d", ...)` - the profiler report line, with `PERF_APP_UPDATE`/`PERF_GAME_ONMOVE` names in the same literal block |
| `00b4b863`, `00b4b89f` in `BSP_D3D9IndexBuffer_Lock` | on `capacity < cursor + offset` and on a non-zero offset in the dynamic path |
| `00b4ba13`, `00b4ba4f` in `BSP_D3D9VertexBuffer_Lock` | the same two preconditions |
| `009406d4`..`0094071a` in `FUN_0093ff10` | eight back-to-back `call 004c14c0`, five of them consecutive with nothing between |
| `0073e14b` | immediately after the `After InitGui` checkpoint |
| `004ca67f`, `00a86a22`, `00a86abe`, `00b1546a`, `00b155ee`, `00b1f0f8`, `00b5106c`, `0094062d` | same shape |

Five identical no-argument calls in a row that consume nothing and produce nothing only make
sense as the residue of statements whose body was compiled away. The reading that fits every
site is a **diagnostic sink**: the shipped build kept the lazy getter (a real function with
side effects, so the optimiser could not delete it) and dropped the reporting method. The
class name is a hypothesis; the "no state, only a virtual destructor, result always
discarded" shape is fact. `docs/D3D9_BUFFER_UPLOAD.md` calls it "the unresolved diagnostic
singleton"; this packet resolves its layout and lifetime but not its output path.

## Reconstruction

`include/bsp/gui_startup.hpp` / `src/gui_startup.cpp`.

- `bsp::run_gui_startup(GuiStartupHost&)` sequences the nine steps of `0073bae0` onto an
  injected host with one method per native call site, in the same style as
  `bsp::run_application_frame`. It reuses `bsp::FontRegistry` for step 4 rather than
  restating the descriptor load, which is already `src/font_registry.cpp`.
- `bsp::GuiManagerResource` and `kGuiManagerResources` carry the ten names of `00aa5e20`
  with their destination offsets, so the order and the offsets are data, not code.
- `bsp::LocaleTableManagerImage` is the 0x4040 layout above as a struct with a
  `static_assert` on each offset, and `construct_locale_table_manager_0073c960` reproduces
  exactly the words the native constructor writes. The bucket array is *not* a new map:
  `bsp::LocaleStringMap` in `include/bsp/locale_tables.hpp` already models it, and the image
  struct only fixes the byte layout the constructor establishes.
- `bsp::DiagnosticSinkSingleton` models `004c14c0` as lazy construction plus lifetime
  registration with no payload, because that is all the native code does.

Nothing here is binary compatible with the original: the native strings, the pooled
allocator and the vtables are not reproduced.

## Uncertainties and what remains

- The class names `cGuiManager`, the font registry, the fallback glyph table and the
  diagnostic sink are hypotheses. Only `cGuiManager` has direct string support, from the
  segment-76 literal `cGuiManager`.
- The 512 bytes of `fonts/arial19.dat` are read but their meaning was not established, and
  `00be9630`'s transform over them was not analysed.
- `00aa5840` and `00aa7e00`, the two widget factories used by `00aa5e20`, were not analysed;
  the virtual `+0x34(bool)` they are followed by is presumably a visibility or enable flag,
  which is a guess.
- The renderer texture entry point in `00aa5e20` is `*(00f8d394)` virtual `+0x64`; the
  renderer class is out of this packet.
- The leading dword of the three 16-byte vectors at `+0x04`, `+0x4020` and `+0x4030` is never
  written by the constructor chain. `00450540` reads only `+4`/`+8`/`+0xc` of the vector, so
  what the leading word holds and who initialises it is open.
- Six blocks of `00aa5e20` are reported unreachable by the decompiler; the string table above
  was read from the disassembly instead, but the branches those blocks belong to were not
  reconstructed.
- `004c14c0`'s output path is not recoverable from this image.
- Ghidra still shows `007371d0` with three parameters at the `0073bae0` call site. The
  correct split (`007371d0` takes none, `00ac3910` takes three, `RET 0xc`) is recorded here
  and in the ledger evidence; the prototype fix needs a writer.

## State reached

| Address | Name | State |
| --- | --- | --- |
| 0073bae0 | BSP_FontSystem_LoadDefinitions | exported, analyzed, reconstructed as a host sequence, build-tested |
| 007371d0 | BSP_FontSystem_GetRegistry | exported, analyzed |
| 0073c960 | BSP_LocaleTableManager_Construct | exported, analyzed, layout reconstructed, build-tested |
| 004c14c0 | BSP_DiagnosticSink_GetOrCreate | exported, analyzed, reconstructed as lazy construction only, build-tested |
| 004c12b0 | BSP_GuiManager_GetOrCreate | exported, analyzed (name predates this packet) |
| 0053bc00 | BSP_FallbackGlyphTable_GetSingleton | exported, analyzed |
| 00be9760 | BSP_FallbackGlyphTable_Construct | exported, analyzed |
| 00be9620 | BSP_FallbackGlyphTable_GetData | exported, analyzed |
| 00aa5d70 | BSP_GuiManager_Construct | exported, analyzed |
| 00aa5e20 | BSP_GuiManager_LoadResources | exported, analyzed, resource list reconstructed, build-tested |
| 00ac3910 | BSP_FontRegistry_LoadLuaDescriptors | reconstructed before this packet, `docs/FONT_GLYPH_SOURCE.md` |

## Exact native byte evidence

SHA256 over the function body bytes as they sit in the installed
`battlestationspacific.exe`. The installation was not modified.

| Range | SHA256 |
| --- | --- |
| 0073bae0..0073bc33 | 5eb4b4a6dd1899633b1b7c8fc26433a6d1de66ff57c3aab409a038894c8d7353 |
| 007371d0..0073728c | ac9c01a13763b515635c492b50258cfec95c5a5c62644a0368ba92a828b434f9 |
| 0073c960..0073c9ec | f59f7defb9cd1b9c99c31a701fd67bd1d4a9e24d9505fa069188a18c4dcae908 |
| 004c14c0..004c1569 | d1ed3a68f63885476351cadb135979cb5433c62f21ddc2778425c5b4a103b505 |
| 004c12b0..004c136f | 7d2554e9a125d67a8f18cc6a652e6914189ce00982f3c08f2f1dc9b72a892a6a |
| 0053bc00..0053bcbf | 6d6ae656c58e0bbba18c3b04cf1f15972ec5b7966322392d6d1005f524dc904c |
| 00aa5e20..00aa633e | cd4758165b406c38cc63c0560fb8e44ae8b1c6ed15b2cfd89a71afcecbf1ffdd |
| 00aa5d70..00aa5e18 | 5343e5b7be77fa95c33c97a324bf886dcdc1563604656dd2949c5deb574ef2ff |
| 00be9620..00be9623 | 4181cb1fec090d6487f323c8ce84a17ff37429613d7843b4f5f40f5eae266b77 |
| 00be9760..00be988b | 37b54d0f17122a891ebe14f1092c42f0962a049cf6e82772d5631e5ddd8e1dc8 |
| 00736540..007365d5 | 3d7f1bf492d1ea9a2384ec4ecfacab2ec3b29aa57d113ad77578d99d95bf7630 |

### Correction from docs/GUI_LAYOUT_LOADER.md

GUI pages are Lua source: a page name resolves to `interface/<name>.lua` (composed by `00ac5600`), the screen constructor runs `interface/_common.lua` first, then the page, then builds the widget tree from the global table `GuiScreen`, with the widget class carried in the key suffix after the last underscore (seventeen types). `00aa7e00` creates nothing: it finds an existing child one level deep, so the names listed above as created by the two factories are keys in the installed `_mouse.lua` and `_highlight.lua`; its documented recursive flag is popped by `RET 8` and never read. `00aaa480` is the MSVC `std::sort` introsort loop.

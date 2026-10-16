# Native GUI/Text mutable state 38

Packet `cc12_native_gui_text_state_38`, prepared from Main `9a727abac`.
Source: `include/bsp/game_native_gui_text_state.hpp` and
`src/game_native_gui_text_state.cpp`. Receipt:
`reports/cc12_native_gui_text_state_38.json`.

This supplies one process-lifetime **typed Source** owner for the scalar and
lazy-white prerequisites of `GameNativeTextApplication`. It does not map these
objects at Original addresses or reproduce an Original global/object ABI.
No native Text factory or normal-host activation is added. The independent
Text lineage/pool packet owns F8BE28[3], F8BDF0, CD7210, CD71F0 and CE0A80.

## Original preimages and producer inventory

The current installed executable is
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`,
12,223,752 bytes, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Its image base is 00400000. Writable `.data` starts at RVA A08000, has
SizeOfRawData 10000 and VirtualSize 297EDC. The following words were independently
read from that PE and compared with live Ghidra memory; BSS values below are
loader zero fill, not bytes claimed to exist in the file.

| Original cell | Initial word(s) | Source role and established producer |
| --- | --- | --- |
| E12FC0 | 3FAAAAAB | Aspect ratio, exact float bits (approximately 4/3); initialized `.data`. |
| E12FD4 | 3F400000 | Vertical scale 0.75; initialized `.data`. |
| E12FD8..E12FE4 | 00000000,00000000,00000000,3F400000 | Shadow RGBA 0,0,0,0.75; initialized `.data`. |
| F8BE38..F8BE44 | four 00000000 words | Shared lazy-white quartet; loader zero fill. |
| F8BE48 | 00000000 | Lazy-white mask; loader zero fill. |
| F8BE54 | 00000000 | Borrowed shadow-material blend source; loader zero fill. |

All selected cells are outside `GameNativeReadOnlyData`'s CE2000..E07B23 range.
The Source owner encodes these verified words directly; it never asks `data_at`
to admit mutable `.data` or obtains a plausible substitute from a descriptor.

Live xrefs and an independent scan of every raw PE section for these exact
DWORD addresses and neighboring four-byte anchors account for their encoded
references. The scan found 89 occurrences and no initialized data/table pointer
to any scanned anchor. No Original CRT/startup writer to aspect, vertical scale,
shadow quartet or blend was found: their established initial producer is the
PE loader. The only recovered writes to the white quartet/mask are the getter
below and four inline lazy blocks in ABB630. The independently reviewed CD7210
initializer addresses its own lineage cells, not this packet's scalars.
This is a static producer/reference inventory, not proof against every possible
computed-address store in an arbitrary running process.

Important consumers checked include AA9F10 (aspect pointer registration),
AB9650 and AB72D0 (shadow quartet), AB98F0/ABA270/AB8910 (vertical scale),
ABB630 (property defaults), and ABA8D0 (F8BE54 parameter source at ABAC37).
Ghidra also reports propagated offcut reads of F8BE54 inside string traversal
functions; the raw PE scan finds only ABAC38's absolute F8BE54 operand. The
separate F8BE50 empty UTF-16 sentinel is not an alias supplied by this owner.

## Recovered shared-white getter

The raw scan exposed an undefined code island at **00AB7060..00AB70A0**, 65
bytes including RET. The locked definition tool first matched all bytes against
the installed executable, defined exactly that range, and saved the canonical
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.
The provisional name is `BSP_GuiText_GetSharedDefaultWhite`, not a recovered
symbol. Annotation history preserves the prior name/comment, and the annotated
export was refreshed. No caller xrefs exist; ABB630 contains inline copies.

| Native site | Exact operation |
| --- | --- |
| AB7060 | EAX=1. |
| AB7065 / AB706B | Test mask bit 0; if set skip directly to AB709B. |
| AB706D | Load the current D7A24C float word; current PE word is 3F800000. |
| AB7075 | OR the DWORD mask with 1, preserving its other bits. |
| AB707B / AB7083 / AB708B / AB7093 | Store that same word into the quartet, in order. |
| AB709B / AB70A0 | EAX=&F8BE38, plain RET. |

There are no register/stack inputs or nested calls. A no-argument cdecl versus
stdcall distinction is unobservable here; the earlier provisional cdecl label
was clarified in the preserved annotation history. The new C++ method instead
takes an explicit borrowed D7A24C reference and returns the same typed array.
The four ABB630 mask-read/store pairs are ABB9B0/ABB9C1, ABBA32/ABBA43,
ABBAAF/ABBAC0 and ABBB2C/ABBB3D. Existing `gui_text_properties.cpp` already
performs their load-before-OR-before-four-stores sequence over borrowed cells.

## One Source backing and its bindings

`game_native_gui_text_state()` returns one intentionally retained owner. It has
no public constructor, copy, reset or teardown path. Its private primitive
members begin with the verified image/loader words. It does not eagerly turn
the zero white quartet into ones. All access and all borrowing services require
the existing serialized GUI/render lifetime contract.

| Owner accessor | Existing service referent |
| --- | --- |
| `aspect_ratio_00e12fc0()` | `GuiMaterialBindingServices`, native clip refresh and clip material parameters. |
| `vertical_scale_00e12fd4()` | `GameNativeTextApplicationBindings`, single-line and wrapped services. |
| `shadow_color_00e12fd8()` | Both constructor `const volatile float[4]` and property `const float[4]` bindings. |
| `default_color_00f8be38()` and `default_color_mask_00f8be48()` | The same `GuiTextPropertyConstants` cells across every Text; also the recovered getter. |
| `blend_factor_00f8be54()` | Content/material blend parameter source. |

The returned arrays are native C++ array references, so their qualification
conversions preserve the quartet identity; no vector or color copy is a backing
owner. Material parameter references can outlive individual Texts, hence the
process lifetime. The existing services remain consumers, and their eventual
normal-host composition must bind these references. This packet does not alter
the separate sprite bridge or activate full native Text.

## Build and integration boundary

Shared CMake was left to its current owner. Required registration:

```cmake
cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL target_sources bsp_game PRIVATE src/game_native_gui_text_state.cpp)
```

An ignored CMake overlay registers that exact source with `/W4 /WX` for focused
Win32 compilation/linking. The normal build script and existing checks are run
separately. The JSON receipt records their final results. No new test suite,
Original ABI differential, game launch, native Text drawing or gameplay result
is claimed. Ignored evidence is under `local/cc12_native_gui_text_state_38/`.

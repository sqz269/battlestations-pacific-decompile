# Segment 66 Hardware Info, Display Options, Stream/Locale & Package Helpers (`argon-v2-seg66`)

## Overview

Packet `argon-v2-seg66` reconstructs 70 functions in Segment 66 spanning `0098c7f0` through `00996060` in:

- Header: `include/bsp/hardware_options_seg66_argon.hpp`
- Source: `src/hardware_options_seg66_argon.cpp`
- Structured report: `reports/argon_seg66_hardware_options.json`

All 70 functions are reconstructed in namespace `bsp::hardware_options_seg66_argon` over explicit `Raw` pointer views and the pure-virtual `Seg66Host` interface without `fn_at` or raw function-pointer casts. All descriptive C++ function and host method names are hypotheses rather than recovered original symbols; the reconstructed interfaces are build-tested C++ projections (`reconstructed_build_tested`), not drop-in binary ABI replacements (`abi_compatible: false`, `game_validated: false`).

## x87 `DC`-Form Audit

Running `x87_dc_form_scan.py` over the address span `0098c7f0` through `00996060` found **0** `DC`-form (`fadd`/`fmul`/`fcom`/`fcomp`/`fsub`/`fsubr`/`fdiv`/`fdivr` `qword ptr`) instructions across all 70 functions.

## Reconstructed Functions (70)

1. **`0x0098c7f0` — `hardware_info_cpu_speed_0098c7f0` (`BSP_HardwareInfo_CpuSpeed`)**
   - **Original ABI**: `__thiscall` (`ECX = self`), `RET`, returns `uint32_t` in `EAX`.
   - **Behavior**: Reads `[ECX + 0x30]` (detected CPU speed). No direct calls.

2. **`0x0098c800` — `hardware_info_memory_size_0098c800` (`BSP_HardwareInfo_MemorySize`)**
   - **Original ABI**: `__thiscall` (`ECX = self`), `RET`, returns `uint32_t` in `EAX`.
   - **Behavior**: Reads `[ECX + 0x2c]` (detected system memory size). No direct calls.

3. **`0x0098c810` — `hardware_info_display_options_ptr_0098c810` (`TRIV_body_0098c810`)**
   - **Original ABI**: `__thiscall` (`ECX = self`), `RET`, returns `void*` in `EAX`.
   - **Behavior**: Returns `ECX + 0x234`. No direct calls.

4. **`0x0098d3d0` — `hardware_info_sound_device_name_0098d3d0` (`BSP_HardwareInfo_SoundDeviceName`)**
   - **Original ABI**: `__cdecl` / ignores `ECX`, `RET`, returns `const char*` in `EAX`.
   - **Behavior**: Returns the global sound device name buffer at `0x00f8a0e8`. No direct calls.

5. **`0x0098da40` — `std_locale_facet_dtor_0098da40` (`TRIV_body_0098da40`)**
   - **Original ABI**: `__thiscall` (`ECX = self`), `RET`.
   - **Behavior**: Stores `std::locale::facet` vtable `0x00d692b8` at `[ECX]`. No direct calls.

6. **`0x0098db80` — `cg_scalar_deleting_dtor_0098db80` (`CG_scalar_deleting_dtor_0098db80`)**
   - **Original ABI**: `__thiscall` (`ECX = self`, stack `uint8_t flags`), `RET 0x4`.
   - **Behavior**: Stores vtable `0x00d692b8` at `[ECX]` and, when `flags & 1` is non-zero, calls `0x00bf65ac` (`0x0098db91`).

7. **`0x0098dbd0` — `std_locale_facet_ctor_0098dbd0` (`TRIV_body_0098dbd0`)**
   - **Original ABI**: `__thiscall` (`ECX = self`), `RET`.
   - **Behavior**: Stores `std::locale::facet` vtable `0x00d692b8` at `[ECX]`. No direct calls.

8. **`0x0098dc50` — `std_codecvt_base_dtor_0098dc50` (`TRIV_body_0098dc50`)**
   - **Original ABI**: `__thiscall` (`ECX = self`), `RET`.
   - **Behavior**: Stores `std::locale::facet` vtable `0x00d692b8` at `[ECX]`. No direct calls.

9. **`0x0098dc60` — `cg_scalar_deleting_dtor_0098dc60` (`CG_scalar_deleting_dtor_0098dc60`)**
   - **Original ABI**: `__thiscall` (`ECX = self`, stack `uint8_t flags`), `RET 0x4`.
   - **Behavior**: Stores vtable `0x00d692b8` at `[ECX]` and conditionally calls `0x00bf65ac` (`0x0098dc71`).

10. **`0x0098dde0` — `cg_scalar_deleting_dtor_0098dde0` (`CG_scalar_deleting_dtor_0098dde0`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `uint8_t flags`), `RET 0x4`.
    - **Behavior**: Stores vtable `0x00d1d208` at `[ECX]`, calls `0x00bf5ec3` (`0x0098ddea`), and conditionally calls `0x00bf65ac` (`0x0098ddfa`).

11. **`0x0098de50` — `std_locale_scalar_deleting_dtor_0098de50` (`FUN_0098de50`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `uint8_t flags`), `RET 0x4`.
    - **Behavior**: If `[ECX]` is non-null, locks locale state via `0x00bf5993` (`0x0098de61`), decrements the reference count at `[ptr + 4]` when neither `0` nor `-1`, unlocks via `0x00bf59b4` (`0x0098de88`), and invokes `vtable[0]` (`1`) when the count reached zero; then conditionally frees `self` via `0x00bf65ac` (`0x0098dea4`).

12. **`0x0098e0e0` — `hardware_info_supports_display_mode_0098e0e0` (`BSP_HardwareInfo_SupportsDisplayMode`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `int32_t width, int32_t height`), `RET 0x8`, returns `bool` in `AL`.
    - **Behavior**: Queries vector length via `0x0098d610` (`0x0098e0e8`, `0x0098e143`), validates checked bounds on the 8-byte `{width, height}` vector at `+0x04`/`+0x08` via `0x00bf6713` (`0x0098e113`, `0x0098e130`), and returns `true` on an exact match.

13. **`0x0098e170` — `cg_scalar_deleting_dtor_0098e170` (`CG_scalar_deleting_dtor_0098e170`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `uint8_t flags`), `RET 0x4`.
    - **Behavior**: Stores vtable `0x00d1d208` at `[ECX]`, calls `0x00bf5ec3` (`0x0098e17a`), and conditionally frees `self` via `0x00bf65ac` (`0x0098e18a`).

14. **`0x0098e4e0` — `std_basic_streambuf_dtor_0098e4e0` (`FUN_0098e4e0`)**
    - **Original ABI**: `__thiscall` (`ECX = self`), tail jump to `0x00bf5f19`.
    - **Behavior**: Stores `basic_streambuf` vtable `0x00d1d214`, releases the heap `std::locale` at `[ECX + 0x38]` under `0x00bf5993` (`0x0098e500`) and `0x00bf59b4` (`0x0098e527`), invokes `vtable[0]` (`1`) when `_Locimp` reference count reaches zero, frees `[ECX + 0x38]` via `0x00bf65ac` (`0x0098e53c`), and destroys the embedded mutex at `ECX + 0x04` via `0x00bf5f19` (`0x0098e54d`).

15. **`0x0098e890` — `cg_scalar_deleting_dtor_0098e890` (`CG_scalar_deleting_dtor_0098e890`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `uint8_t flags`), `RET 0x4`.
    - **Behavior**: Calls `0x0098e4e0` (`0x0098e893`) and conditionally calls `0x00bf65ac` (`0x0098e8a0`).

16. **`0x0098f180` — `std_basic_streambuf_ctor_0098f180` (`FUN_0098f180`)**
    - **Original ABI**: `__thiscall` (`ECX = self`), `RET`.
    - **Behavior**: Stores vtable `0x00d1d214`, constructs mutex at `ECX + 0x04` via `0x00bf5f01` (`0x0098f1a9`), allocates `std::locale` holder via `0x00bf681b` (`0x0098f1b8`), initializes locale via `0x00bf5cc2` (`0x0098f1c7`) and `0x00bf5acc` (`0x0098f1ce`) under `0x00bf5993` (`0x0098f1db`) / `0x00bf59b4` (`0x0098f1f2`), stores it at `[ECX + 0x38]`, and initializes stream buffer pointers via `0x0098e250` (`0x0098f201`).

17. **`0x0098f950` — `cg_scalar_deleting_dtor_0098f950` (`CG_scalar_deleting_dtor_0098f950`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `uint8_t flags`), `RET 0x4`.
    - **Behavior**: Stores `runtime_error` vtable `0x00d1d46c`, frees heap message string at `[ECX + 0x10]` when `[ECX + 0x24] >= 0x10` via `0x00bf65ac` (`0x0098f963`), resets string state, calls `0x00bf6454` (`0x0098f97c`), and conditionally frees `self` via `0x00bf65ac` (`0x0098f989`).

18. **`0x0098f9a0` — `std_string_append_chars_0098f9a0` (`FUN_0098f9a0`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `uint32_t count, char ch`), `RET 0x8`.
    - **Behavior**: Validates string length bounds via `0x00bf5695` (`0x0098f9b2`, `0x0098f9ca`), grows storage via `0x004089e0` (`0x0098f9dd`) when needed, fills characters via `0x0098e1b0` (`0x0098f9f2`), and NUL-terminates.

19. **`0x0098fa90` — `std_string_assign_chars_0098fa90` (`FUN_0098fa90`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `uint32_t count, char ch`), `RET 0x8`.
    - **Behavior**: Validates `count` against `0xffffffff` and `0xfffffffe` via `0x00bf5695` (`0x0098fa9d`, `0x0098faa9`), grows storage via `0x004089e0` (`0x0098fabc`), assigns `count` copies of `ch` at offset `0` via `0x0098e1b0` (`0x0098facf`), and NUL-terminates.

20. **`0x0098fb50` — `std_locinfo_dtor_wrapper_0098fb50` (`FUN_0098fb50`)**
    - **Original ABI**: `__thiscall` (`ECX = self`), `RET`.
    - **Behavior**: Calls `0x00bf5bfd` (`0x0098fb79`), releases four embedded SBO strings via `0x00bf65ac` (`0x0098fb8f`, `0x0098fbb0`, `0x0098fbca`, `0x0098fbe4`), and unlocks via `0x00bf59b4` (`0x0098fbff`).

21. **`0x0098fc20` — `std_basic_filebuf_uflow_0098fc20` (`FUN_0098fc20`)**
    - **Original ABI**: `__thiscall` (`ECX = self`), `RET`.
    - **Behavior**: Reads from buffered get-area if available; otherwise reads from `FILE*` at `[ECX + 0x4c]` via `0x00bfc5f7` (`0x0098fc9b`, `0x0098fcd3`, `0x0098fefa`). When `codecvt` facet `[ECX + 0x3c]` is present, appends input bytes via `0x0098f9a0` (`0x0098fceb`), checks iterator bounds via `0x00bf6713` (`0x0098fd2b`, `0x0098fd56`, `0x0098fd98`, `0x0098fdc1`, `0x0098feb6`, `0x0098fedf`), invokes `vtable[4]` (`+0x10`, `codecvt::in`), and handles converted bytes, `noconv` copy via `0x0098f490` (`0x0098fe1c`, `0x0098ff40`), `0x0098f0c0` (`0x0098fe23`, `0x0098ff47`), `0x00bf672c` (`0x0098fe30`), unget pushback via `0x00bfcc1b` (`0x0098ff75`), prefix erase via `0x004087f0` (`0x0098fef1`), and string teardown via `0x004072d0` (`0x0098fe41`, `0x0098ff0f`, `0x0098ff8a`).

22. **`0x0098ffe0` — `std_basic_filebuf_endwrite_0098ffe0` (`FUN_0098ffe0`)**
    - **Original ABI**: `__thiscall` (`ECX = self`), `RET`.
    - **Behavior**: Flushes active codecvt output state when `[ECX + 0x3c] != 0` and `[ECX + 0x41] != 0`: invokes `vtable[1]` (`+0x04`, `overflow(-1)`), loops over `codecvt::unshift` by invoking `vtable[6]` (`+0x18`) with checked-iterator validations via `0x00bf6713` (`0x009900a6`, `0x009900d1`, `0x00990119`, `0x00990142`, `0x009901e0`, `0x0099020b`, `0x00990253`, `0x0099027c`), writes bytes via `0x00bfb08b` (`0x00990289`), grows buffer via `0x0098f9a0` (`0x009902b2`), and cleans up temporary string via `0x004072d0` (`0x00990175`, `0x009902db`).

23. **`0x00990340` — `std_basic_filebuf_overflow_00990340` (`FUN_00990340`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `int32_t meta`), `RET 0x4`.
    - **Behavior**: Writes `meta` into put-area or `FILE*` at `[ECX + 0x4c]`. Uses `0x00bfc87f` (`0x009903c7`) when `[ECX + 0x3c] == 0`; otherwise converts through `codecvt::out` (invokes `vtable[5]` at `+0x14`) with checked-iterator validations via `0x00bf6713` (`0x00990460`, `0x0099048b`, `0x009904cd`, `0x009904f6`, `0x00990571`, `0x0099059c`, `0x009905e8`, `0x00990611`), writes output via `0x00bfb08b` (`0x00990622`) or `0x0098f580` (`0x009906a7`), expands buffer via `0x0098f9a0` (`0x0099066d`), and destroys temporary string via `0x004072d0` (`0x009906b8`, `0x009906db`, `0x009906fb`, `0x0099071d`).

24. **`0x00990740` — `std_basic_filebuf_seekoff_00990740` (`std_basic_filebuf_seekoff_00990740`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `fpos* out, int32_t way, int32_t off, int32_t mode`), `RET 0x10`.
    - **Behavior**: Adjusts offset for putback char at `ECX + 0x40`, calls `0x0098ffe0` (`0x00990778`), `0x00bfb72b` (`0x00990790`), and `0x00bfcf65` (`0x009907a5`), and populates `*out`.

25. **`0x00990820` — `std_basic_filebuf_seekpos_00990820` (`std_basic_filebuf_seekpos_00990820`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, 32 bytes stack), `RET 0x20`.
    - **Behavior**: Calls `0x0098ffe0` (`0x00990842`), `0x00bfcfd0` (`0x00990858`), `0x00bfb72b` (`0x00990873`), and `0x00bfcf65` (`0x00990888`), updates `[ECX + 0x44]`, and populates `*out`.

26. **`0x00990910` — `std_runtime_error_ctor_00990910` (`std_runtime_error_ctor_00990910`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `const void* msg_str`), `RET 0x4`.
    - **Behavior**: Calls `0x00bf632f` (`0x0099092d`), installs vtable `0x00d1d46c`, and copies `msg_str` into `ECX + 0x0c` via `0x00408120` (`0x00990956`).

27. **`0x00990970` — `std_basic_filebuf_close_00990970` (`std_basic_filebuf_close_00990970`)**
    - **Original ABI**: `__thiscall` (`ECX = self`), `RET`.
    - **Behavior**: Calls `0x0098ffe0` (`0x0099097e`) and `0x00bf8081` (`0x0099098c`) when `[ECX + 0x4c] != 0`, then resets state via `0x0098e250` (`0x009909a2`).

28. **`0x009909e0` — `std_runtime_error_dtor_009909e0` (`std_runtime_error_dtor_009909e0`)**
    - **Original ABI**: `__thiscall` (`ECX = self`), tail jump to `0x00bf6454`.
    - **Behavior**: Stores vtable `0x00d1d46c`, frees message buffer via `0x00bf65ac` (`0x009909f3`) when capacity `>= 0x10`, and tail-jumps to `0x00bf6454` (`0x00990a0d`).

29. **`0x00990a20` — `cg_scalar_deleting_dtor_00990a20` (`CG_scalar_deleting_dtor_00990a20`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `uint8_t flags`), `RET 0x4`.
    - **Behavior**: Stores vtable `0x00d1d46c`, frees message buffer via `0x00bf65ac` (`0x00990a33`), calls `0x00bf6454` (`0x00990a4c`), and conditionally frees `self` via `0x00bf65ac` (`0x00990a59`).

30. **`0x00990a70` — `std_basic_filebuf_dtor_00990a70` (`std_basic_filebuf_dtor_00990a70`)**
    - **Original ABI**: `__thiscall` (`ECX = self`), `RET`.
    - **Behavior**: Stores vtable `0x00d1d434`, calls `0x00990970` (`0x00990aa1`) when `[ECX + 0x48] != 0`, and calls `0x0098e4e0` (`0x00990ab0`).

31. **`0x00990ad0` — `cg_scalar_deleting_dtor_00990ad0` (`CG_scalar_deleting_dtor_00990ad0`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `uint8_t flags`), `RET 0x4`.
    - **Behavior**: Calls `0x00990a70` (`0x00990ad3`) and conditionally calls `0x00bf65ac` (`0x00990ae0`).

32. **`0x00990b60` — `std_locinfo_ctor_00990b60` (`std_Locinfo_ctor_00990b60`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `const char* locname`), `RET 0x4`.
    - **Behavior**: Locks via `0x00bf5993` (`0x00990b83`), initializes four embedded SBO strings, throws `std::runtime_error("bad locale name")` via `0x00408720` (`0x00990bd8`), `0x00990910` (`0x00990beb`), and `0x00bf6885` (`0x00990bfa`) if `locname == nullptr`, otherwise calls `0x00bf5d5c` (`0x00990c01`).

33. **`0x00990c20` — `std_runtime_error_copy_ctor_00990c20` (`std_runtime_error_copy_ctor_00990c20`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `const void* other`), `RET 0x4`.
    - **Behavior**: Calls `0x00bf63a6` (`0x00990c43`), sets vtable `0x00d1d46c`, and copies the message string at `+0x0c` via `0x00408120` (`0x00990c6b`).

34. **`0x00990c90` — `std_ctype_char_ctor_00990c90` (`std_ctype_char_ctor_00990c90`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `const void* table, bool del_flag, uint32_t refs`), `RET 0xc`.
    - **Behavior**: Initializes `ctype<char>` vtable `0x00d1d48c`, constructs `_Locinfo` via `0x00990b60` (`0x00990cd7`), queries `_Getctype` via `0x00bf5879` (`0x00990ce1`), destroys `_Locinfo` via `0x0098fb50` (`0x00990d04`), and when `table != nullptr`, frees the default table via `0x00bf9dc8` (`0x00990d1f`) or `0x00bf6989` (`0x00990d2c`) and installs `table`.

35. **`0x00990d60` — `std_ctype_char_do_tolower_00990d60` (`std_ctype_char_do_tolower_00990d60`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `char ch`), `RET 0x4`.
    - **Behavior**: Calls `0x00bf576a` (`0x00990d6a`) with `(uint8_t)ch` and `ECX + 0x08`.

36. **`0x00990d80` — `std_ctype_char_do_tolower_range_00990d80` (`std_ctype_char_do_tolower_range_00990d80`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `char* first, const char* last`), `RET 0x8`.
    - **Behavior**: Loops over `[first, last)` calling `0x00bf576a` (`0x00990d97`).

37. **`0x00990db0` — `std_ctype_char_do_toupper_00990db0` (`std_ctype_char_do_toupper_00990db0`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `char ch`), `RET 0x4`.
    - **Behavior**: Calls `0x00bf5f87` (`0x00990dba`) with `(uint8_t)ch` and `ECX + 0x08`.

38. **`0x00990dd0` — `std_ctype_char_do_toupper_range_00990dd0` (`std_ctype_char_do_toupper_range_00990dd0`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `char* first, const char* last`), `RET 0x8`.
    - **Behavior**: Loops over `[first, last)` calling `0x00bf5f87` (`0x00990de7`).

39. **`0x00990e00` — `std_ctype_char_do_widen_00990e00` (`TRIV_body_00990e00`)**
    - **Original ABI**: `__thiscall` (stack `char ch`), `RET 0x4`.
    - **Behavior**: Returns `ch` in `AL`. No direct calls.

40. **`0x00990e80` — `std_ctype_char_do_narrow_00990e80` (`TRIV_body_00990e80`)**
    - **Original ABI**: `__thiscall` (stack `char ch, char dflt`), `RET 0x8`.
    - **Behavior**: Returns `ch` in `AL`. No direct calls.

41. **`0x00990f00` — `std_ctype_char_dtor_00990f00` (`std_ctype_char_dtor_00990f00`)**
    - **Original ABI**: `__thiscall` (`ECX = self`), `RET`.
    - **Behavior**: Stores vtable `0x00d1d48c`, frees owned table at `[ECX + 0x10]` via `0x00bf9dc8` (`0x00990f14`) when `[ECX + 0x14] > 0` or `0x00bf6989` (`0x00990f2a`) when `< 0`, and restores facet vtable `0x00d692b8`.

42. **`0x00990f40` — `cg_scalar_deleting_dtor_00990f40` (`CG_scalar_deleting_dtor_00990f40`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `uint8_t flags`), `RET 0x4`.
    - **Behavior**: Inlines `std_ctype_char_dtor` freeing table via `0x00bf9dc8` (`0x00990f54`) or `0x00bf6989` (`0x00990f61`), and conditionally frees `self` via `0x00bf65ac` (`0x00990f77`).

43. **`0x00990f90` — `std_ctype_char_getcat_00990f90` (`std_ctype_char_Getcat_00990f90`)**
    - **Original ABI**: `__cdecl` (stack `void** out_facet`), `RET`.
    - **Behavior**: When `out_facet != nullptr && *out_facet == nullptr`, allocates `0x18` bytes via `0x00bf681b` (`0x00990fb5`) and constructs `ctype<char>` via `0x00990c90` (`0x00990fd5`); returns `2`.

44. **`0x00991000` — `std_ios_base_clear_00991000` (`std_ios_base_clear_00991000`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `uint32_t state, bool reraise`), `RET 0x8`.
    - **Behavior**: Stores `state & 0x17` at `[ECX + 0x08]`; when masked by `[ECX + 0x0c]`, rethrows via `0x00bf6885` (`0x0099103e`) if `reraise` is set, or constructs and throws `ios_base::failure` for `badbit`, `failbit`, or `eofbit` via `0x00407290` (`0x00991051`, `0x00991094`, `0x009910d2`), `0x00990910` (`0x0099106a`, `0x009910ad`, `0x009910eb`), and `0x00bf6885` (`0x00991081`, `0x009910c4`, `0x00991102`).

45. **`0x00991150` — `std_ios_base_init_00991150` (`FUN_00991150`)**
    - **Original ABI**: `__thiscall` (`ECX = self`), `RET`.
    - **Behavior**: Initializes `ios_base` fields, calls `0x00991000` (`0x00991177`), allocates `std::locale` pointer via `0x00bf681b` (`0x0099117e`), and initializes it via `0x00bf5cc2` (`0x0099118d`), `0x00bf5acc` (`0x00991194`), `0x00bf5993` (`0x009911a0`), and `0x00bf59b4` (`0x009911b7`).

46. **`0x009911f0` — `std_use_facet_ctype_char_009911f0` (`std_use_facet_009911f0`)**
    - **Original ABI**: `__cdecl` (stack `const void* locale_obj`), `RET`.
    - **Behavior**: Looks up `ctype<char>` facet under `0x00bf5993` (`0x00991211`, `0x00991237`) and `0x00bf59b4` (`0x0099125b`, `0x009912f6`), checks global locale via `0x00bf5acc` (`0x00991281`), creates on miss via `0x00990f90` (`0x009912a6`), throws `bad_cast` via `0x00bf6477` (`0x009912bc`) and `0x00bf6885` (`0x009912cb`) on failure, and registers via `0x0098d9c0` (`0x009912dc`) and `0x00bf5bc4` (`0x009912e2`).

47. **`0x00991310` — `std_basic_ifstream_dtor_00991310` (`std_basic_ifstream_dtor_00991310`)**
    - **Original ABI**: `__thiscall` (`ECX = vbase_self`), `RET`.
    - **Behavior**: Updates virtual base vtable to `0x00d1d50c`, calls `0x00990a70` (`0x00991346`) on `ECX - 0x50`, and sets base vtable `0x00d1d20c`.

48. **`0x00991610` — `std_basic_ios_widen_00991610` (`FUN_00991610`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `char ch`), `RET 0x4`.
    - **Behavior**: Obtains locale via `0x0098dd80` (`0x0099162f`), resolves `ctype<char>` facet via `0x009911f0` (`0x0099163d`), releases locale reference under `0x00bf5993` (`0x0099165d`) and `0x00bf59b4` (`0x00991684`) (invoking `vtable[0]` if count reaches zero), and invokes `vtable[6]` (`+0x18`, `do_widen`).

49. **`0x009916c0` — `std_facet_ctor_009916c0` (`std_facet_ctor_009916c0`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `uint32_t refs`), `RET 0x4`.
    - **Behavior**: Sets vtable `0x00d1d510`, constructs temporary `_Locinfo` via `0x00990b60` (`0x00991703`), and destroys it via `0x0098fb50` (`0x0099170c`).

50. **`0x00991970` — `std_basic_ios_init_00991970` (`std_basic_ios_init_00991970`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `void* streambuf, bool is_std`), `RET 0x8`.
    - **Behavior**: Calls `0x00991150` (`0x00991973`), widens `' '` via `0x00991610` (`0x0099198a`), sets `badbit` via `0x00991000` (`0x009919a3`) when `streambuf == nullptr`, and calls `0x00bf5e73` (`0x009919b0`) when `is_std` is true.

51. **`0x009919d0` — `std_facet_getcat_009919d0` (`std_facet_Getcat_009919d0`)**
    - **Original ABI**: `__cdecl` (stack `void** out_facet`), `RET`.
    - **Behavior**: Allocates 8 bytes via `0x00bf681b` (`0x009919f5`) and constructs facet via `0x009916c0` (`0x00991a11`) when `*out_facet == nullptr`; returns `2`.

52. **`0x00991a40` — `std_basic_ostream_flush_00991a40` (`std_basic_ostream_flush_00991a40`)**
    - **Original ABI**: `__thiscall` (`ECX = self`), `RET`.
    - **Behavior**: Invokes `vtable[12]` (`+0x30`, `pubsync`) on `rdbuf()` when stream state is good, and sets `badbit` via `0x00991000` (`0x00991a86`) if `pubsync` returns `-1`.

53. **`0x00991a90` — `std_basic_istream_ipfx_00991a90` (`std_basic_istream_Ipfx_00991a90`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `bool noskip`), `RET 0x4`.
    - **Behavior**: Flushes tied ostream via `0x00991a40` (`0x00991ad0`), skips whitespace using `0x0098dd80` (`0x00991af9`), `0x009911f0` (`0x00991b06`), `0x0098daf0` (`0x00991b13`), `vtable[4]` (`+0x10`, `uflow`), and `0x0098f3b0` (`0x00991baa`), updates state via `0x00991000` (`0x00991b65`, `0x00991c02`), and rethrows in catch handler via `0x00bf6885` (`0x00991beb`) when `badbit` is unmasked.

54. **`0x00991ce0` — `std_basic_istream_ctor_00991ce0` (`std_basic_istream_ctor_00991ce0`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `void* streambuf, bool is_std, int32_t init_vbtable`), `RET 0xc`.
    - **Behavior**: Initializes vbtable `0x00d1d530` and vtable `0x00d1d20c`, sets `gcount = 0`, and calls `0x00991970` (`0x00991d44`).

55. **`0x00991d60` — `std_use_facet_codecvt_char_00991d60` (`std_use_facet_00991d60`)**
    - **Original ABI**: `__cdecl` (stack `const void* locale_obj`), `RET`.
    - **Behavior**: Resolves `codecvt` facet under `0x00bf5993` (`0x00991d81`, `0x00991da7`) and `0x00bf59b4` (`0x00991dcb`, `0x00991e66`), checks global locale via `0x00bf5acc` (`0x00991df1`), creates default facet via `0x009919d0` (`0x00991e16`), throws `bad_cast` via `0x00bf6477` (`0x00991e2c`) and `0x00bf6885` (`0x00991e3b`) on failure, and registers via `0x0098d9c0` (`0x00991e4c`) and `0x00bf5bc4` (`0x00991e52`).

56. **`0x00991e80` — `std_basic_istream_sentry_ctor_00991e80` (`FUN_00991e80`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `void* istream_obj, bool noskip`), `RET 0x8`.
    - **Behavior**: Locks `rdbuf()` mutex via `0x00bf5f2e` (`0x00991eb3`) if non-null and calls `0x00991a90` (`0x00991ec7`).

57. **`0x00992370` — `std_basic_filebuf_open_00992370` (`std_basic_filebuf_open_00992370`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `const char* filename, int32_t mode, int32_t prot`), `RET 0xc`.
    - **Behavior**: Opens file via `0x00bf622c` (`0x009923a4`), initializes streambuf pointers via `0x0098e250` (`0x009923bf`, `0x00992439`), queries `codecvt` facet via `0x0098e210` (`0x009923f1`) and `0x00991d60` (`0x009923fb`), invokes `vtable[1]` (`+0x04`, `always_noconv`), and releases temporary locale via `0x0098daf0` (`0x00992419`, `0x00992442`).

58. **`0x00992480` — `std_operator_gt_gt_string_00992480` (`FUN_00992480`)**
    - **Original ABI**: `__cdecl` (stack `void* istream_obj, void* str_obj`), `RET`.
    - **Behavior**: Constructs sentry via `0x00991e80` (`0x009924b2`), resolves `ctype<char>` via `0x0098dd80` (`0x009924cf`), `0x009911f0` (`0x009924d9`), and `0x0098daf0` (`0x009924ea`), clears destination string via `0x004087f0` (`0x009924f5`), extracts non-whitespace characters using `vtable[4]` (`+0x10`, `uflow`), `0x0098f9a0` (`0x009925d3`), and `0x0098f3b0` (`0x009925e8`), updates stream state via `0x00991000` (`0x0099258a`), and unlocks `rdbuf()` mutex via `0x00bf5f37` (`0x009925a9`).

59. **`0x009926a0` — `std_basic_ifstream_ctor_009926a0` (`std_basic_ifstream_ctor_009926a0`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `const char* filename, uint32_t mode, int32_t prot, int32_t init_vbtable`), `RET 0x10`.
    - **Behavior**: Initializes `basic_ifstream` vtables, calls `0x00991150` (`0x00992702`), `0x00991610` (`0x00992714`), `0x00991000` (`0x0099272e`, `0x009927b1`), `0x0098f180` (`0x0099274d`), `0x0098e250` (`0x00992760`), and opens the file via `0x00992370` (`0x0099278d`).

60. **`0x00993060` — `cg_scalar_deleting_dtor_00993060` (`CG_scalar_deleting_dtor_00993060`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `uint8_t flags`), `RET 0x4`.
    - **Behavior**: Returns the `NativeString` buffer at `[ECX + 0x0c]` (size `[ECX + 0x08] + 1`) to the sized storage pool via `0x00419cc0` (`0x00993074`) and `0x00bd1510` (`0x0099307b`), and conditionally frees `self` via `0x00bf65ac` (`0x00993088`).

61. **`0x00993b70` — `stl_xlive_stat_record_vector_erase_range_00993b70` (`FUN_00993b70`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `void* out_iter, void* first_owner, void* first_ptr, void* last_owner, void* last_ptr`), `RET 0x14`.
    - **Behavior**: Validates iterator owners via `0x00bf6713` (`0x00993b83`), shifts trailing records via `0x00993540` (`0x00993baf`), destroys the erased tail via `0x00993990` (`0x00993bc4`), updates `[ECX + 0x08]`, and writes the returned iterator to `*out_iter`.

62. **`0x009942f0` — `cg_scalar_deleting_dtor_009942f0` (`CG_scalar_deleting_dtor_009942f0`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `uint8_t flags`), `RET 0x4`.
    - **Behavior**: Calls `0x00994180` (`0x009942f3`) and conditionally frees `self` via `0x00bf65ac` (`0x00994300`).

63. **`0x00994500` — `return_true_stub_00994500` (`TRIV_body_00994500`)**
    - **Original ABI**: `__stdcall` / `__thiscall` (4 bytes stack), `RET 0x4`.
    - **Behavior**: Returns `1` in `AL`. No direct calls.

64. **`0x00994680` — `cg_scalar_deleting_dtor_00994680` (`CG_scalar_deleting_dtor_00994680`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `uint8_t flags`), `RET 0x4`.
    - **Behavior**: Calls `0x009945e0` (`0x00994683`) and conditionally frees `self` via `0x00bf65ac` (`0x00994690`).

65. **`0x00994750` — `cg_scalar_deleting_dtor_00994750` (`CG_scalar_deleting_dtor_00994750`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `uint8_t flags`), `RET 0x4`.
    - **Behavior**: Calls `0x009946d0` (`0x00994753`) and conditionally frees `self` via `0x00bf65ac` (`0x00994760`).

66. **`0x00994e90` — `return_false_stub_ret4_00994e90` (`TRIV_body_00994e90`)**
    - **Original ABI**: `__stdcall` / `__thiscall` (4 bytes stack), `RET 0x4`.
    - **Behavior**: Returns `0` in `AL`. No direct calls.

67. **`0x00994ea0` — `return_false_stub_00994ea0` (`TRIV_body_00994ea0`)**
    - **Original ABI**: `__cdecl` / `__thiscall` (0 bytes stack), `RET`.
    - **Behavior**: Returns `0` in `AL`. No direct calls.

68. **`0x00994ef0` — `cg_scalar_deleting_dtor_00994ef0` (`CG_scalar_deleting_dtor_00994ef0`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `uint8_t flags`), `RET 0x4`.
    - **Behavior**: Calls `0x00994e10` (`0x00994ef3`) and conditionally frees `self` via `0x00bf65ac` (`0x00994f00`).

69. **`0x00995670` — `cg_scalar_deleting_dtor_00995670` (`CG_scalar_deleting_dtor_00995670`)**
    - **Original ABI**: `__thiscall` (`ECX = self`, stack `uint8_t flags`), `RET 0x4`.
    - **Behavior**: Calls `0x00995150` (`0x00995673`) and conditionally frees `self` via `0x00bf65ac` (`0x00995680`).

70. **`0x00996060` — `localization_already_running_message_00996060` (`BSP_Localization_AlreadyRunningMessage`)**
    - **Original ABI**: `__fastcall` (`ECX = lcid`, `EDX = message_index`), `RET`.
    - **Behavior**: Calls `0x0098d400` (`0x00996063`) to map `lcid` to a language row `0..5` and indexes the wide-string table at `0x00d1e918` by `row + message_index * 6`.

## Skipped

- **`0x0098e910` (`BSP_HardwareInfo_BuildDefaultOptionsText`)**: Excluded from packet `argon-v2-seg66` as a larger multi-branch hardware tier classification and default `options.txt` synthesis routine that calls `0x0098e0e0` (`BSP_HardwareInfo_SupportsDisplayMode`) and formats default video/sound/shader settings based on detected GPU/CPU/memory tiers.

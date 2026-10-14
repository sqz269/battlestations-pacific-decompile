# Argon Segments 31 & 32: Airfield Session Messages, Hangars, Markers, and GUI Highlights (`006d4370` through `006deac0`)

- Packet: `argon-v2-seg31-32`
- Source: `src/airfield_marker_seg31_32_argon.cpp`
- Header: `include/bsp/airfield_marker_seg31_32_argon.hpp`
- Report: `reports/argon_seg31_32_airfield_marker.json`
- Status: `reconstructed_build_tested` (`abi_compatible: false`, `game_validated: false`, `coverage: "complete"`)
- x87 DC-form audit: `x87_dc_form_scan.py` over `006d4370` through `006deac0` found 1 site (`006d58c0` inside skipped function `006d5220`) and 0 sites across the 39 reconstructed functions.

## Scope & Architecture

All 39 reconstructed functions in `006d4370` through `006deac0` are implemented in namespace `bsp::airfield_marker_seg31_32_argon` using `Raw` byte-offset access and the pure-virtual `Seg31_32Host` interface (`no fn_at`, no raw function-pointer address casts). Descriptive C++ names are hypotheses, not recovered symbols, and expose new C++ interfaces rather than drop-in binary replacements.

## Reconstructed Functions (39)

### AirOpsStockSnapshot & SessionMessage_AIRFIELD_CREATE (`006d4370` through `006d5130`)

1. **`0x006d4370` (`air_ops_stock_snapshot_destruct_006d4370` / `BSP_AirOpsStockSnapshot_Destruct`)**
   - Original ABI: `__thiscall(void* self) -> void`, `RET` at `0x006d43b0`.
   - Behavior: Restores vtable `0x00cf8eb8` at `+0x00`, frees the `+0x1c` vector buffer via `0x00bf65ac` (`0x006d4384`) when non-null and zeroes `+0x1c`/`+0x20`/`+0x24`, then frees the `+0x0c` vector buffer via `0x00bf65ac` (`0x006d439d`) when non-null and zeroes `+0x0c`/`+0x10`/`+0x14`.

2. **`0x006d4420` (`session_message_airfield_create_destruct_006d4420` / `BSP_SessionMessage_AIRFIELD_CREATE_Destruct`)**
   - Original ABI: `__thiscall(void* self) -> void`, tail jump `0x007668e0` (`0x006d449d`).
   - Behavior: Restores vtable `0x00cf8ebc` at `+0x00`, frees the hangar array at `+0x13c` via `0x00bf6989` (`0x006d4437`) when non-null and clears `+0x13c`, destroys the embedded `AirOpsStockSnapshot` at `+0x110` (restoring vtable `0x00cf8eb8` and freeing `+0x12c` via `0x00bf65ac` (`0x006d445a`) and `+0x11c` via `0x00bf65ac` (`0x006d447f`)), then tail-calls `0x007668e0` (`0x006d449d`).

3. **`0x006d44b0` (`session_message_airfield_create_stock_snapshot_006d44b0` / `BSP_SessionMessage_AIRFIELD_CREATE_StockSnapshot`)**
   - Original ABI: `__thiscall(void* self) -> void*`, `RET` at `0x006d44b6`.
   - Behavior: Vtable `0x00cf8ebc` slot `+0x14`; returns `self + 0x110`.

4. **`0x006d44c0` (`net_entity_create_message_vslot18_return_zero_006d44c0` / `BSP_NetEntityCreateMessage_VSlot18_ReturnZero`)**
   - Original ABI: `__thiscall(void* self) -> uint32_t`, `RET` at `0x006d44c2`.
   - Behavior: Vtable `0x00cf8ebc` slot `+0x18`; returns `0`.

5. **`0x006d44f0` (`air_ops_stock_snapshot_write_006d44f0` / `BSP_AirOpsStockSnapshot_Write`)**
   - Original ABI: `__thiscall(void* self, void* cursor) -> void`, `RET 0x4` at `0x006d4767`.
   - Behavior: Writes `+0x04` (6 bits) via `0x00429070` (`0x006d4502`), writes the 12-byte-stride vector count at `+0x0c`/`+0x10` (6 bits) via `0x00429070` (`0x006d4536`), and for each record checks bounds (calling `0x00bf6713` (`0x006d4567`), `0x00bf6713` (`0x006d459c`), `0x00bf6713` (`0x006d45d2`) on out-of-range) and writes `+0x00` (10 bits, `0x00429070` (`0x006d4577`)), `+0x04` (8 bits, `0x00429070` (`0x006d45ad`)), and `+0x08` (3 bits, `0x00429070` (`0x006d45e3`)). Next writes the 20-byte-stride vector count at `+0x1c`/`+0x20` (6 bits) via `0x00429070` (`0x006d4628`), and for each record checks bounds (`0x00bf6713` (`0x006d4661`), `0x00bf6713` (`0x006d4697`), `0x00bf6713` (`0x006d46ce`), `0x00bf6713` (`0x006d4705`), `0x00bf6713` (`0x006d473d`)) and writes `+0x00` (10 bits, `0x00429070` (`0x006d4671`)), `+0x04` (6 bits, `0x00429070` (`0x006d46a8`)), `+0x08` (4 bits, `0x00429070` (`0x006d46df`)), `+0x0c` (12-bit word, `0x00429120` (`0x006d4717`)), and `+0x10` (3 bits, `0x00429070` (`0x006d474e`)).

6. **`0x006d47a0` (`session_message_airfield_create_write_006d47a0` / `BSP_SessionMessage_AIRFIELD_CREATE_Write`)**
   - Original ABI: `__thiscall(void* self, void* cursor) -> void`, `RET 0x4` at `0x006d4870`.
   - Behavior: Calls base writer `0x00765000` (`0x006d47aa`) and stock snapshot writer `0x006d44f0` (`0x006d47b6`) on `self + 0x110`, writes floats `+0x140` and `+0x144` via `0x004295c0` (`0x006d47d7`) and `0x004295c0` (`0x006d47f8`) with scale `[0x00d7a248]`, writes hangar count `+0x138` (3 bits) via `0x00429070` (`0x006d4808`), and writes each 6-byte hangar record at `+0x13c` (`+0x00` 12 bits via `0x00429120` (`0x006d482f`), `+0x02` 13 bits via `0x00429120` (`0x006d4844`), `+0x04` 13 bits via `0x00429120` (`0x006d4859`)).

7. **`0x006d4c20` (`air_ops_stock_snapshot_construct_006d4c20` / `BSP_AirOpsStockSnapshot_Construct`)**
   - Original ABI: `__thiscall(void* self) -> void*`, `RET` at `0x006d4c60`.
   - Behavior: Installs vtable `0x00cf8eb8` at `+0x00`, zeroes vector triplets `+0x0c..+0x14` and `+0x1c..+0x24`, and returns `self`.

8. **`0x006d4c70` (`session_message_airfield_create_construct_006d4c70` / `BSP_SessionMessage_AIRFIELD_CREATE_Construct`)**
   - Original ABI: `__thiscall(void* self, void* airfield, float field_140, float field_144) -> void*`, `RET 0xc` at `0x006d4dbf`.
   - Behavior: Calls `0x00766fc0` (`0x006d4ca9`) with message type `0xa3`, class ID `*(*(airfield + 0x538) + 0x70)`, `airfield`, and `1`; sets vtable `0x00cf8ebc` at `+0x00` and embedded snapshot vtable `0x00cf8eb8` at `+0x110`; captures stock from `airfield + 0x72c` via `0x006cc800` (`0x006d4ce4`); stores floats at `+0x140` and `+0x144`; allocates `6 * count` bytes (with `MUL`/`SETO` overflow saturation) via `0x00bf55be` (`0x006d4d22`) for `count = *(airfield + 0x834)` at `+0x138`/`+0x13c`; and populates each 6-byte entry from the 12-byte hangar records at `*(airfield + 0x830)` (`hangar->+0x174` entity ID and vslot `+0x28` path IDs on record `+0x04` and `+0x08`).

9. **`0x006d4dd0` (`air_field_append_intact_hangars_as_sub_entities_006d4dd0` / `BSP_AirField_AppendIntactHangarsAsSubEntities`)**
   - Original ABI: `__thiscall(void* self, void* out_pointer_vec) -> void`, `RET 0x4` at `0x006d4e34`.
   - Behavior: Walks the 12-byte-stride hangar array at `self + 0x830` / `self + 0x834` (reloading base and count each iteration) and appends each non-null hangar whose float at `+0x370` is strictly greater than `[0x00d7a218]` (`0.0f`) into `out_pointer_vec` via `0x004323d0` (`0x006d4e13`).

10. **`0x006d4eb0` (`air_ops_stock_snapshot_read_006d4eb0` / `BSP_AirOpsStockSnapshot_Read`)**
    - Original ABI: `__thiscall(void* self, void* stream) -> void`, `RET 0x4` at `0x006d512a`.
    - Behavior: Advances `cursor = stream + 0x04`, reads `+0x04` (6 bits) via `0x00428d10` (`0x006d4eca`) and first vector count (6 bits) via `0x00428d10` (`0x006d4ed8`), resizes `self + 0x08` via `0x006ca010` (`0x006d4ef6`), and reads each 12-byte record (`+0x00` 10 bits `0x00428d10` (`0x006d4f3f`), `+0x04` 8 bits `0x00428d10` (`0x006d4f75`), `+0x08` 3 bits `0x00428d10` (`0x006d4fab`)) with bounds checks (`0x00bf6713` (`0x006d4f30`), `0x00bf6713` (`0x006d4f64`), `0x00bf6713` (`0x006d4f9a`)). Then reads the second vector count (6 bits) via `0x00428d10` (`0x006d4fcd`), resizes `self + 0x18` via `0x006ca0f0` (`0x006d4ff1`), and reads each 20-byte record (`+0x00` 10 bits `0x00428d10` (`0x006d5034`), `+0x04` 6 bits `0x00428d10` (`0x006d506b`), `+0x08` 4 bits `0x00428d10` (`0x006d50a2`), `+0x0c` 12-bit word `0x00428e30` (`0x006d50d9`), `+0x10` 3 bits `0x00428d10` (`0x006d5110`)) with bounds checks (`0x00bf6713` (`0x006d5025`), `0x00bf6713` (`0x006d505a`), `0x00bf6713` (`0x006d5091`), `0x00bf6713` (`0x006d50c8`), `0x00bf6713` (`0x006d50ff`)).

11. **`0x006d5130` (`session_message_airfield_create_read_006d5130` / `BSP_SessionMessage_AIRFIELD_CREATE_Read`)**
    - Original ABI: `__thiscall(void* self, void* stream) -> void`, `RET 0x4` at `0x006d5217`.
    - Behavior: Calls base reader `0x00767e30` (`0x006d513b`) and stock snapshot reader `0x006d4eb0` (`0x006d5147`) on `self + 0x110`, then uses `cursor = stream + 0x04` to read floats `+0x140` and `+0x144` via `0x004293f0` (`0x006d5169`) and `0x004293f0` (`0x006d5186`), reads hangar count `+0x138` (3 bits) via `0x00428d10` (`0x006d5196`), allocates `6 * count` bytes via `0x00bf55be` (`0x006d51af`), and reads each 6-byte hangar record via `0x00428e30` (`0x006d51d3`), `0x00428e30` (`0x006d51e7`), and `0x00428e30` (`0x006d51fb`).

### Marker Groups, Instances, Classes, and Named Values (`006d6010` through `006d9230`)

12. **`0x006d6010` (`marker_group_vector_clear_006d6010` / `BSP_MarkerGroupVector_Clear`)**
    - Original ABI: `__thiscall(void* self, void* out_vec) -> void`, `RET 0x4` at `0x006d60a1`.
    - Behavior: Validates `out_vec` iterator ordering (`0x00bf6713` (`0x006d6024`), `0x00bf6713` (`0x006d6032`)), erases `[first, last)` via `0x006d3a50` (`0x006d6042`), and walks the 12-byte-stride array at `self + 0x830` / `self + 0x834`, pushing each entry whose pointed object has `+0x5c != 0`, `+0x5d == 0`, `+0x60 == 0`, and `+0x5e == 0` into `out_vec` via `0x006d4e40` (`0x006d607d`).

13. **`0x006d6170` (`vec3_lexicographic_less_006d6170` / `BSP_Vec3_LexicographicLess`)**
    - Original ABI: `__stdcall(const float* lhs, const float* rhs) -> bool`, `RET 0x8` at `0x006d61d0` / `0x006d61dc`.
    - Behavior: Lexicographically compares `lhs[0..2]` against `rhs[0..2]` (`x`, then `y`, then `z`).

14. **`0x006d61f0` (`marker_instance_tick_006d61f0` / `BSP_MarkerInstance_Tick`)**
    - Original ABI: `__thiscall(void* self, float delta) -> void`, `RET 0x4` at `0x006d61f7`.
    - Behavior: Invokes `self` vtable `+0x08` (`vcall_marker_instance_vslot08`).

15. **`0x006d6200` (`triv_body_006d6200` / `TRIV_body_006d6200`)**
    - Original ABI: `void(void)`, `RET` at `0x006d6200`.
    - Behavior: Empty body (`RET`).

16. **`0x006d7100` (`tmpl_owned_ptr_delete_reset_006d7100` / `TMPL_OwnedPtr_DeleteReset_006d7100`)**
    - Original ABI: `__thiscall(void** self_slot) -> void`, `RET` at `0x006d7118`.
    - Behavior: If `*self_slot` is non-null, calls vslot `+0x00` with `1` and zeroes `*self_slot`.

17. **`0x006d7120` (`owned_object_ptr_delete_006d7120` / `BSP_OwnedObjectPtr_Delete_006d7120`)**
    - Original ABI: `__thiscall(void** self_slot) -> void`, `RET` at `0x006d7139`.
    - Behavior: If `*self_slot` is non-null, calls vslot `+0x04` with `1` and zeroes `*self_slot`.

18. **`0x006d7140` (`tmpl_owned_ptr_delete_reset_006d7140` / `TMPL_OwnedPtr_DeleteReset_006d7140`)**
    - Original ABI: `__thiscall(void** self_slot) -> void`, `RET` at `0x006d7158`.
    - Behavior: If `*self_slot` is non-null, calls vslot `+0x00` with `1` and zeroes `*self_slot`.

19. **`0x006d74b0` (`marker_classes_reset_lua_table_006d74b0` / `BSP_MarkerClasses_ResetLuaTable`)**
    - Original ABI: `__cdecl(void) -> void`, `RET` at `0x006d74ce`.
    - Behavior: Loads `lua_machine = *(*([0x00e188a8] + 0x1a08) + 0x04)` and executes `"MarkerClasses=nil\n"` (`0x00cf8f54`) via `0x006b8ad0` (`0x006d74c9`) with `(nullptr, nullptr, 2)`.

20. **`0x006d74d0` (`marker_instance_destruct_006d74d0` / `BSP_MarkerInstance_Destruct`)**
    - Original ABI: `__thiscall(void* self) -> void`, `RET` at `0x006d74f1`.
    - Behavior: Restores vtable `0x00cf8f68` at `+0x00`, and if `+0x18` is non-null, calls its vslot `+0x04` with `1` and zeroes `+0x18`.

21. **`0x006d7fc0` (`marker_instance_update_pose_006d7fc0` / `BSP_MarkerInstance_UpdatePose`)**
    - Original ABI: `__thiscall(void* self) -> void`, `RET` at `0x006d81c6`.
    - Behavior: When descriptor animation mode `*(*(self + 0x14) + 0x18)` is non-zero, resolves world position from `owner + 0xfc` (calling `0x00414db0` (`0x006d7fe9`) when `*(uint8_t*)(owner + 0xc8) == 0`) or `self + 0x08`, builds a 4x4 transform with `phase = 2.0f * *([0x00e188a8] + 0x64c) + *(float*)(self + 0x1c)`, applies mode 1 (`0x00b64640` (`0x006d80c4`), `0x004134f0` (`0x006d80ce`), `0x004134f0` (`0x006d80dc`)), mode 2 (`0x00b646e0` (`0x006d810a`), `0x004134f0` (`0x006d8114`), `0x004134f0` (`0x006d8122`)), mode 3 (`0x0042b4d0` (`0x006d813d`), `0x004134f0` (`0x006d8147`)), or mode 4 (`|sin(phase)| * [0x00ce3880]` added to `y`), and writes the transform to `*(*(self + 0x18) + 0x0c)` via vslot `+0x34`.

22. **`0x006d8750` (`marker_instance_construct_006d8750` / `BSP_MarkerInstance_Construct`)**
    - Original ABI: `__thiscall(void* self, void* descriptor, void* owner, float x, float y, float z) -> void*`, `RET 0x14` at `0x006d87cd` / `0x006d8805`.
    - Behavior: Stores vtable `0x00cf8f68`, `owner` (`+0x04`), position (`+0x08..+0x10`), and `descriptor` (`+0x14`), instantiates the scene resource at `*(descriptor + 0x14)` via vslot `+0x08` `(0, 1.0f)` into `+0x18`, propagates root registration on `*(*(self + 0x18) + 0x0c)` with `*([0x00e188a8] + 0x19ec)` via `0x00b6d890` (`0x006d87ad`), sets phase `+0x1c` to `0.0f` if `*(uint8_t*)(descriptor + 0x1c) != 0` or else from `0x00bd2f10` (`0x006d87e7`) with `ECX = 1` and `(0.0f, [0x00cf8f74])`, and updates the pose via `0x006d7fc0` (`0x006d87c5`) / `0x006d7fc0` (`0x006d87fd`).

23. **`0x006d8c70` (`marker_class_create_instance_006d8c70` / `BSP_MarkerClass_CreateInstance`)**
    - Original ABI: `__thiscall(void* self_descriptor, void* owner, float x, float y, float z) -> void*`, `RET 0x10` at `0x006d8cea` / `0x006d8cfe`.
    - Behavior: Allocates `0x20` bytes via `0x00bf681b` (`0x006d8c8d`) and, if non-null, constructs a `MarkerInstance` via `0x006d8750` (`0x006d8cd6`).

24. **`0x006d8e50` (`native_string_destroy_storage_006d8e50` / `BSP_NativeString_DestroyStorage_006d8e50`)**
    - Original ABI: `__thiscall(void* self_string) -> void`, `RET` at `0x006d8e6c`.
    - Behavior: If `self_string->data` (`+0x04`) is non-null, obtains the string pool singleton via `0x00419cc0` (`0x006d8e60`) (0 args) and returns `(data, length + 1, 1)` via `0x00bd1510` (`0x006d8e67`).

25. **`0x006d8e70` (`native_string_destroy_storage_006d8e70` / `BSP_NativeString_DestroyStorage_006d8e70`)**
    - Original ABI: `__thiscall(void* self_string) -> void`, `RET` at `0x006d8e8c`.
    - Behavior: Byte-identical twin of `0x006d8e50`; calls `0x00419cc0` (`0x006d8e80`) and `0x00bd1510` (`0x006d8e87`).

26. **`0x006d9140` (`named_value_construct_006d9140` / `BSP_NamedValue_Construct_006d9140`)**
    - Original ABI: `__thiscall(void* self, const void* name_str, const uint32_t* value_ptr) -> void*`, `RET 0x8` at `0x006d9188`.
    - Behavior: Zero-initializes `self` (`+0x00`, `+0x04`), copies `name_str` via `0x0041dd40` (`0x006d915e`) and `0x00bf7680` (`0x006d9173`) when `self != name_str`, and writes `*value_ptr` to `+0x08`.

27. **`0x006d9190` (`named_value_construct_006d9190` / `BSP_NamedValue_Construct_006d9190`)**
    - Original ABI: `__thiscall(void* self, const void* name_str, const uint32_t* value_ptr) -> void*`, `RET 0x8` at `0x006d91d8`.
    - Behavior: Byte-identical twin of `0x006d9140`; calls `0x0041dd40` (`0x006d91ae`) and `0x00bf7680` (`0x006d91c3`).

28. **`0x006d91e0` (`named_value_copy_construct_006d91e0` / `BSP_NamedValue_CopyConstruct_006d91e0`)**
    - Original ABI: `__thiscall(void* self, const void* other) -> void*`, `RET 0x4` at `0x006d9225`.
    - Behavior: Copies the `NativeString` prefix of `other` via `0x0041dd40` (`0x006d91fe`) and `0x00bf7680` (`0x006d9213`) and copies the `+0x08` value word.

29. **`0x006d9230` (`named_value_copy_construct_006d9230` / `BSP_NamedValue_CopyConstruct_006d9230`)**
    - Original ABI: `__thiscall(void* self, const void* other) -> void*`, `RET 0x4` at `0x006d9275`.
    - Behavior: Byte-identical twin of `0x006d91e0`; calls `0x0041dd40` (`0x006d924e`) and `0x00bf7680` (`0x006d9263`).

### GuiElement Path/Descendants/Pulse & MarkerManager (`006d9300` through `006deac0`)

30. **`0x006d9300` (`gui_element_build_path_006d9300` / `BSP_GuiElement_BuildPath`)**
    - Original ABI: `__fastcall(void* out_string, void* element) -> void*`, `RET` at `0x006d94ac`.
    - Behavior: Walks the parent chain (`element = *(element + 0x70)`), prepending separator `0x00ce3a70` (`0x0041dd40` (`0x006d935a`), `0x00bf7680` (`0x006d9375`)) and the element name from vslot `+0x28` via `0x004261a0` (`0x006d9395`) and `0x004261a0` (`0x006d93ab`), copying into the accumulator (`0x0041dd40` (`0x006d93c6`), `0x00bf7680` (`0x006d93dd`)) and releasing temporaries via `0x00419cc0` (`0x006d93fd`) / `0x00bd1510` (`0x006d9404`), `0x00419cc0` (`0x006d9421`) / `0x00bd1510` (`0x006d9428`), and `0x00419cc0` (`0x006d9441`) / `0x00bd1510` (`0x006d9448`). Finally strips the leading separator via `0x00469840` (`0x006d946c`) `(1, 0x7fffffff)` and releases the accumulator via `0x00419cc0` (`0x006d948c`) / `0x00bd1510` (`0x006d9493`).

31. **`0x006dae20` (`marker_manager_add_gui_highlight_006dae20` / `BSP_MarkerManager_AddGuiHighlight`)**
    - Original ABI: `__thiscall(void* self, void* gui_element, uint8_t recursive_flag, uint8_t color_mode_flag) -> void`, `RET 0xc` at `0x006dae54` / `0x006dae8d`.
    - Behavior: When `recursive_flag != 0`, inserts `gui_element` into `self + 0x38` via `0x006da5b0` (`0x006dae37`); otherwise selects `self + 0x44` when `color_mode_flag != 0` or `self + 0x2c` when `color_mode_flag == 0` and inserts via `0x006da5b0` (`0x006dae6f`). Updates `[0x00e19970] = *([0x00e188a8] + 0x64c)`.

32. **`0x006db3f0` (`gui_element_collect_descendants_006db3f0` / `BSP_GuiElement_CollectDescendants`)**
    - Original ABI: `__fastcall(void* root_element, void* out_vec) -> void`, `RET` at `0x006db4f2`.
    - Behavior: Seeds `out_vec` with `root_element` via `0x0052ed20` (`0x006db40a`), then performs a breadth-first traversal bounded by `0x00519d50` (`0x006db417`) and `0x00519d50` (`0x006db4de`), checking vector and child-list iterators (`0x00bf6713` (`0x006db43d`), `0x00bf6713` (`0x006db45b`), `0x00bf6713` (`0x006db469`), `0x00bf6713` (`0x006db4ab`), `0x00bf6713` (`0x006db4c8`)) and appending each child from the list at `elem + 0x64` / `elem + 0x68` (fast-path in-place push or grow via `0x0052eb90` (`0x006db4ba`)).

33. **`0x006dbac0` (`gui_element_apply_highlight_pulse_006dbac0` / `BSP_GuiElement_ApplyHighlightPulse`)**
    - Original ABI: custom register + stack (`ECX = element`, `DL = enable`, stack `(uint8_t recursive_flag, uint8_t color_mode_flag, float scale)`), `RET 0xc` at `0x006dbb7b` / `0x006dbc05`.
    - Behavior: Computes `pulse = |cos(2 * (*([0x00e188a8] + 0x64c) - [0x00e19970]))|` when `enable != 0` else `0.0f`. When `color_mode_flag != 0`, reads the 16-byte RGBA color via vslot `+0x54`, sets `rgba[3] = ((enable ? (pulse * [0x00cec9d8] + [0x00d7a348]) : 1.0) * scale)`, and writes it back via vslot `+0x50`. When `color_mode_flag == 0`, writes `scaled_pulse = pulse * [0x00cf8fc8]` to `element + 0x94`, and if `recursive_flag != 0`, collects descendants into a temporary vector via `0x006db3f0` (`0x006dbbb6`), writes `scaled_pulse` to `+0x94` on each descendant (`idx = 1..count-1`), and frees the vector buffer via `0x00bf65ac` (`0x006dbbec`).

34. **`0x006dc5a0` (`marker_manager_remove_gui_highlight_006dc5a0` / `BSP_MarkerManager_RemoveGuiHighlight`)**
    - Original ABI: `__thiscall(void* self, void* gui_element) -> void`, `RET 0x4` at `0x006dc6e9`.
    - Behavior: Looks up `gui_element` in `self + 0x2c` via `0x006d8870` (`0x006dc5cd`) (checking owner with `0x00bf6713` (`0x006dc5e1`)), zeroes `gui_element + 0x94` and erases via `0x006d9bf0` (`0x006dc609`) when found; looks up in `self + 0x38` via `0x006d8870` (`0x006dc61d`) (`0x00bf6713` (`0x006dc63a`)), erases via `0x006d9bf0` (`0x006dc652`) and resets recursive pulse via `0x006dbac0` (`0x006dc665`) when found; and looks up in `self + 0x44` via `0x006d8870` (`0x006dc679`) (`0x00bf6713` (`0x006dc692`)), erases via `0x006d9bf0` (`0x006dc6a6`) and restores `rgba[3] = [0x00d7a24c]` (`1.0f`) through vslots `+0x54`/`+0x50` when found.

35. **`0x006dda40` (`marker_manager_forget_entity_006dda40` / `BSP_MarkerManager_ForgetEntity`)**
    - Original ABI: `__thiscall(void* self, uint32_t entity_id) -> void`, `RET 0x4` at `0x006ddb31`.
    - Behavior: Finds `entity_id` in `self + 0x14` via `0x006d8d10` (`0x006dda62`), validates tree/iterator invariants (`0x00bf6713` (`0x006dda76`), `0x00bf6713` (`0x006dda8b`), `0x00bf6713` (`0x006dda95`), `0x00bf6713` (`0x006ddab9`), `0x00bf6713` (`0x006ddac9`), `0x00bf6713` (`0x006ddad6`), `0x00bf6713` (`0x006ddae0`)), deletes every inner marker instance at `inner_node + 0x14` via vslot `+0x00(1)` while advancing with `0x006d7d20` (`0x006ddaff`), and erases the outer node via `0x006dd1f0` (`0x006ddb25`).

36. **`0x006de3d0` (`marker_manager_bind_unit_marker_stub_006de3d0` / `BSP_MarkerManager_BindUnitMarkerStub`)**
    - Original ABI: `__thiscall(void* self, uint32_t arg0, uint32_t arg1) -> void`, `RET 0x8` at `0x006de3d0`.
    - Behavior: Empty stub (`RET 0x8`).

37. **`0x006de3e0` (`triv_body_006de3e0` / `TRIV_body_006de3e0`)**
    - Original ABI: `__thiscall(void* self, uint32_t arg0, float x, float y, float z) -> void`, `RET 0x10` at `0x006de3e0`.
    - Behavior: Empty stub (`RET 0x10`).

38. **`0x006de4c0` (`marker_manager_clear_position_marker_006de4c0` / `BSP_MarkerManager_ClearPositionMarker`)**
    - Original ABI: `__thiscall(void* self, const void* name_str, float x, float y, float z) -> void`, `RET 0x10` at `0x006de4f1`.
    - Behavior: Looks up the position submap in `self + 0x20` via `0x006de150` (`0x006de4ce`), looks up the marker slot for `name_str` via `0x006dbd40` (`0x006de4d5`), and if non-null deletes the instance via vslot `+0x00(1)` and zeroes the slot.

39. **`0x006deac0` (`marker_manager_destruct_006deac0` / `BSP_MarkerManager_Destruct`)**
    - Original ABI: `__thiscall(void* self) -> void`, `RET` at `0x006dec6b`.
    - Behavior: Restores vtable `0x00cf8ff8` at `+0x00`, drains all three GUI highlight maps (`+0x2c`, `+0x38`, `+0x44`) via `0x006dc5a0` (`0x006deb0c`), `0x006dc5a0` (`0x006deb34`), and `0x006dc5a0` (`0x006deb57`) (with empty-head checks `0x00bf6713` (`0x006deb01`), `0x00bf6713` (`0x006deb29`), `0x00bf6713` (`0x006deb4c`)), erases and frees all five red-black trees (`+0x44` via `0x006da090` (`0x006deb79`) and `0x00bf65ac` (`0x006deb82`); `+0x38` via `0x006da090` (`0x006deba8`) and `0x00bf65ac` (`0x006debb1`); `+0x2c` via `0x006da090` (`0x006debd7`) and `0x00bf65ac` (`0x006debe0`); `+0x20` via `0x006de300` (`0x006dec06`) and `0x00bf65ac` (`0x006dec0f`); `+0x14` via `0x006ddf00` (`0x006dec34`) and `0x00bf65ac` (`0x006dec3d`)), and destroys the base callback owner via `0x00695870` (`0x006dec55`).

## Skipped

The following 3 larger functions in Segments 31 and 32 are outside the 39-function packet and remain deferred:
- `0x006d5220`: `BSP_AirField_InitFromParams` (contains the single x87 DC-form site at `0x006d58c0`).
- `0x006d95d0`: `BSP_MarkerManager_LoadFromLua`.
- `0x006de5c0`: `BSP_MarkerManager_Update`.

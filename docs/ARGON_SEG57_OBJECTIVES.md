# Segment 57 Objectives, Session Messages 55 through 62, Map Pings, Flag Cloth, and Powerup Helpers (`008dd380` through `008e3710`)

## Scope and Status

- **Packet**: `argon-v2-seg57`
- **Address band**: `008dd380` through `008e3710` (51 reconstructed functions)
- **Header**: `include/bsp/objectives_seg57_argon.hpp`
- **Source**: `src/objectives_seg57_argon.cpp`
- **Report**: `reports/argon_seg57_objectives.json`
- **Status**: `reconstructed_build_tested` (`abi_compatible: false`, `game_validated: false`)
- **x87 `DC`-form audit**: `x87_dc_form_scan.py` over `008dd380` through `008e3710` found 0 `DC`-form x87 sites.

All 51 functions are reconstructed 100% completely (`coverage: "complete"`) in namespace `bsp::objectives_seg57_argon` using `Raw` offset views and the abstract `Seg57Host` interface without any `fn_at` or raw address function-pointer casts. Descriptive C++ names are hypotheses and not recovered symbols.

---

## Reconstructed Functions (51)

### 1. `0x008dd380` — `native_string_release_storage_008dd380` (`BSP_NativeString_ReleaseStorage_008dd380`)
- **Original ABI**: `__fastcall (ECX = NativeString* self), RET`
- **Evidence**: Checks `self+0x04`; when non-null, pushes `(data, length + 1, 1)`, obtains the string pool singleton via `0x00419cc0` (`0x008dd390`), and returns the block via `0x00bd1510` (`0x008dd397`).
- **Uncertainty**: None; header fields are not modified after returning the block.

### 2. `0x008dd460` — `objective_sounds_play_throttled_008dd460` (`BSP_ObjectiveSounds_PlayThrottled`)
- **Original ABI**: `__thiscall (ECX = ObjectiveSounds* self, [ESP+4] = int sample_index), RET 0x4`
- **Evidence**: Compares `self+0x18 + [0x00d7a210]` against the clock `[0x00f876a4]`. When elapsed, lazily initializes bit 0 of `[0x00f88c1c]` and resolves sound class `"InGameGUI"` (`0x00ce7854`) into `[0x00f88c18]` via `0x0041e870` (`0x008dd4b7`), `0x00a7acf0` (`0x008dd4cc`), `0x00419cc0` (`0x008dd4ee`), and `0x00bd1510` (`0x008dd4f5`). Then resolves type `"Normal"` (`0x00cecd20`) via `0x0041e870` (`0x008dd50d`) and `0x00a7b0a0` (`0x008dd531`), creates a tracked sound for `self[sample_index]` via `0x00a7e490` (`0x008dd545`), decrements the returned reference at `+0x04` via `[0x00ce2220]` and invokes virtual slot 0 (`0x008dd568`) if zero, releases `"Normal"` via `0x00419cc0` (`0x008dd58f`) and `0x00bd1510` (`0x008dd596`), and updates `self+0x18 = [0x00f876a4]`.
- **Uncertainty**: Descriptive struct/method names are hypotheses.

### 3. `0x008dd5c0` — `objective_construct_008dd5c0` (`BSP_Objective_Construct`)
- **Original ABI**: `__thiscall (ECX = Objective* self, const NativeString* id, const NativeString* text, int kind, uint8_t quiet_flag), RET 0x10`
- **Evidence**: Writes vtable `0x00d16038`, zeroes `id` (`+0x04`/`+0x08`) and `text` (`+0x0c`/`+0x10`), allocates the entry-list sentinel at `+0x24` via `0x008db4f0` (`0x008dd606`) with size `+0x28 = 0`, copies `id` via `0x0041dd40` (`0x008dd62a`) and `0x00bf7680` (`0x008dd640`), copies `text` via `0x0041dd40` (`0x008dd657`) and `0x00bf7680` (`0x008dd66c`), and sets `+0x14 = quiet_flag`, `+0x18 = kind`, and `+0x1c = 0`.
- **Uncertainty**: Field names are hypotheses.

### 4. `0x008dd6a0` — `session_message56_construct_obj_add_008dd6a0` (`BSP_SessionMessage56_ConstructObjAdd`)
- **Original ABI**: `__thiscall (ECX = SessionMessage56* self, const Objective* objective), RET 0x4`
- **Evidence**: Calls `0x0075b430` (`0x008dd6c2`) with type `0x38` (`MT_OBJ_ADD`), sets `+0x04 = 1` and vtable `0x00d16050`, copies `objective+0x04` into `self+0x18` via `0x0041dd40` (`0x008dd702`) and `0x00bf7680` (`0x008dd718`), copies `objective+0x0c` into `self+0x20` via `0x0041dd40` (`0x008dd732`) and `0x00bf7680` (`0x008dd747`), and copies `+0x28 = objective+0x18` and `+0x2c = objective+0x14`.
- **Uncertainty**: None.

### 5. `0x008dd780` — `session_message57_construct_obj_add_entity_008dd780` (`BSP_SessionMessage57_ConstructObjAddEntity`)
- **Original ABI**: `__thiscall (ECX = SessionMessage57* self, const NativeString* name, const Entity* entity), RET 0x8`
- **Evidence**: Calls `0x0075b430` (`0x008dd7a1`) with type `0x39` (`MT_OBJ_ADD_ENTITY`), sets `+0x04 = 1` and vtable `0x00d16064`, copies `name` into `self+0x18` via `0x0041dd40` (`0x008dd7da`) and `0x00bf7680` (`0x008dd7ef`), and copies the 16-bit network handle `entity+0x174` into `self+0x20`.
- **Uncertainty**: None.

### 6. `0x008dd820` — `session_message58_construct_obj_remove_entity_008dd820` (`BSP_SessionMessage58_ConstructObjRemoveEntity`)
- **Original ABI**: `__thiscall (ECX = SessionMessage58* self, const NativeString* name, const Entity* entity), RET 0x8`
- **Evidence**: Calls `0x0075b430` (`0x008dd841`) with type `0x3a` (`MT_OBJ_REMOVE_ENTITY`), sets `+0x04 = 1` and vtable `0x00d16078`, copies `name` into `self+0x18` via `0x0041dd40` (`0x008dd87a`) and `0x00bf7680` (`0x008dd88f`), and copies `entity+0x174` into `self+0x20`.
- **Uncertainty**: None.

### 7. `0x008dd8c0` — `session_message59_construct_obj_add_position_008dd8c0` (`BSP_SessionMessage59_ConstructObjAddPosition`)
- **Original ABI**: `__thiscall (ECX = SessionMessage59* self, const NativeString* name, const float* pos3), RET 0x8`
- **Evidence**: Calls `0x0075b430` (`0x008dd8e1`) with type `0x3b` (`MT_OBJ_ADD_POSITION`), sets `+0x04 = 1` and vtable `0x00d1608c`, copies `name` into `self+0x18` via `0x0041dd40` (`0x008dd91a`) and `0x00bf7680` (`0x008dd92f`), and copies the three floats `pos3[0..2]` into `self+0x20..+0x28`.
- **Uncertainty**: None.

### 8. `0x008dd970` — `session_message60_construct_obj_remove_position_008dd970` (`BSP_SessionMessage60_ConstructObjRemovePosition`)
- **Original ABI**: `__thiscall (ECX = SessionMessage60* self, const NativeString* name, const float* pos3), RET 0x8`
- **Evidence**: Calls `0x0075b430` (`0x008dd991`) with type `0x3c` (`MT_OBJ_REMOVE_POSITION`), sets `+0x04 = 1` and vtable `0x00d160a0`, copies `name` into `self+0x18` via `0x0041dd40` (`0x008dd9ca`) and `0x00bf7680` (`0x008dd9df`), and copies `pos3[0..2]` into `self+0x20..+0x28`.
- **Uncertainty**: None.

### 9. `0x008dda20` — `session_message61_construct_obj_completed_008dda20` (`BSP_SessionMessage61_ConstructObjCompleted`)
- **Original ABI**: `__thiscall (ECX = SessionMessage61* self, const Objective* objective, const NativeString* comment, uint8_t silent_flag), RET 0xc`
- **Evidence**: Calls `0x0075b430` (`0x008dda42`) with type `0x3d` (`MT_OBJ_COMPLETED`), sets `+0x04 = 1` and vtable `0x00d160b4`, copies `objective+0x04` into `self+0x18` via `0x0041dd40` (`0x008dda82`) and `0x00bf7680` (`0x008dda98`), copies `comment` into `self+0x20` via `0x0041dd40` (`0x008ddaaf`) and `0x00bf7680` (`0x008ddac4`), and writes `silent_flag` at `self+0x28`.
- **Uncertainty**: None.

### 10. `0x008ddb00` — `session_message62_construct_obj_failed_008ddb00` (`BSP_SessionMessage62_ConstructObjFailed`)
- **Original ABI**: `__thiscall (ECX = SessionMessage62* self, const Objective* objective, const NativeString* comment, uint8_t silent_flag), RET 0xc`
- **Evidence**: Calls `0x0075b430` (`0x008ddb22`) with type `0x3e` (`MT_OBJ_FAILED`), sets `+0x04 = 1` and vtable `0x00d160c8`, copies `objective+0x04` into `self+0x18` via `0x0041dd40` (`0x008ddb62`) and `0x00bf7680` (`0x008ddb78`), copies `comment` into `self+0x20` via `0x0041dd40` (`0x008ddb8f`) and `0x00bf7680` (`0x008ddba4`), and writes `silent_flag` at `self+0x28`.
- **Uncertainty**: None.

### 11. `0x008ddbe0` — `objective_sync_describe_objectives_008ddbe0` (`BSP_ObjectiveSync_DescribeObjectives`)
- **Original ABI**: `__thiscall (ECX = ObjectiveSet* self, void* archive_writer), RET 0x4`
- **Evidence**: Opens `"objectiveEntities"` (`0x00d160ec`) via `archive_writer` vtable `+0x04`, walks the RB-tree at `self+0x18` with checked-iterator calls to `0x00bf6713` (`0x008ddc36`, `0x008ddc45`, `0x008ddc4f`), writes each node's `+0x0c` entity value via vtable `+0x0c`, and advances via `0x008da530` (`0x008ddc7d`). Closes the block via vtable `+0x08`, opens `"objectiveList"` (`0x00d160dc`) via vtable `+0x04`, walks the list at `self+0x24` with checked-iterator calls to `0x00bf6713` (`0x008ddcce`, `0x008ddcdc`, `0x008ddd0e`), and serializes each entry via `0x008dc210` (`0x008ddcfb`).
- **Uncertainty**: Archive vtable slot names are provisional hypotheses.

### 12. `0x008ddd30` — `objective_set_remove_position_from_named_objective_008ddd30` (`BSP_ObjectiveSet_RemovePositionFromNamedObjective`)
- **Original ABI**: `__thiscall (ECX = ObjectiveSet* self, const NativeString* name, float x, float y, float z), RET 0x10`
- **Evidence**: When `[[0x00e188a8]+0x1fe4] == 1` and the player's peer at `[[0x00e188a8]+0x18cc+slot*4]+0x50` is non-null, constructs a stack `SessionMessage60` via `0x008dd970` (`0x008ddd7a`), sends it via `0x00770b50` (`0x008ddda2`), and destroys it via `0x008dce40` (`0x008dddb3`). Walks `self+0x24` with checked-iterator guards `0x00bf6713` (`0x008dddc7`, `0x008dddd9`, `0x008dde20`, `0x008dde2e`) and case-insensitive compare `0x00bf7fbf` (`0x008dde0a`); on match, erases the position from the objective via `0x008dc500` (`0x008dde3b`), builds `"SzurkeNyil"` (`0x00d16100`) via `0x0041dd40` (`0x008dde58`) and `0x00bf7680` (`0x008dde73`), clears the marker via `0x006de4c0` (`0x008ddebc`), and releases the string via `0x00419cc0` (`0x008ddedc`) and `0x00bd1510` (`0x008ddee3`).
- **Uncertainty**: None.

### 13. `0x008ddf40` — `objective_sync_apply_obj_remove_position_008ddf40` (`BSP_ObjectiveSync_ApplyObjRemovePosition`)
- **Original ABI**: `__thiscall (ECX = ObjectiveSet* self, const SessionMessage60* msg), RET 0x4`
- **Evidence**: Loads `msg+0x18` and the 3-float vector at `msg+0x20..+0x28` and calls `0x008ddd30` (`0x008ddf5e`).
- **Uncertainty**: None.

### 14. `0x008deb40` — `objective_set_erase_named_entries_008deb40` (`BSP_ObjectiveSet_EraseNamedEntries`)
- **Original ABI**: `__thiscall (ECX = void* tree, const void* key_ptr), RET 0x4`
- **Evidence**: Calls `0x008dbba0` (`0x008deb55`) to obtain the equal-range iterator pair, counts the range via `0x008db950` (`0x008deb80`), erases it via `0x008de7e0` (`0x008deb97`), and returns the count.
- **Uncertainty**: None.

### 15. `0x008debc0` — `receive28_range_destroy_forward_008debc0` (`BSP_Receive28Range_Destroy_Forward`)
- **Original ABI**: `__thiscall (ECX = void* self, void* first, void* last), RET 0x8`
- **Evidence**: Forwards `ECX = first`, `EDX = last`, and stack args `(self, last)` to `0x008de660` (`0x008debce`).
- **Uncertainty**: None.

### 16. `0x008dec10` — `session_message55_entry_read_properties_008dec10` (`BSP_SessionMessage55Entry_ReadProperties`)
- **Original ABI**: `__thiscall (ECX = Objective* self, void* archive_reader), RET 0x4`
- **Evidence**: Initializes vtable `0x00d16038`, zeroes string headers at `+0x04` and `+0x0c`, allocates the entry-list sentinel at `+0x24` via `0x008db4f0` (`0x008dec58`), reads `"name"`, `"text"`, `"quiet"`, `"type"`, and `"state"` through `archive_reader` vtable `+0x10`, enters `"positions"` via vtable `+0x04`, and loops while vtable `+0x14` returns non-zero: allocates `0x10` bytes via `0x00bf681b` (`0x008dedc1`), populates the entry via `0x008db990` (`0x008dedd9`), and links it into `self+0x20` via `0x008db650` (`0x008dedfa`) and `0x008de200` (`0x008dee07`).
- **Uncertainty**: Archive vtable slot semantics are provisional.

### 17. `0x008def60` — `objective_sync_destruct_008def60` (`BSP_ObjectiveSync_Destruct`)
- **Original ABI**: `__fastcall (ECX = ObjectiveSet* self), RET`
- **Evidence**: Writes vtable `0x00d1610c`, destroys objective records via `0x008dd1c0` (`0x008def8e`), destroys the pointer list at `+0x24` via `0x008db580` (`0x008def96`), erases the entity RB-tree at `+0x18` via `0x008de7e0` (`0x008defb3`) and frees its head node via `0x00bf65ac` (`0x008defbc`), zeroes `+0x1c` and `+0x20`, and destroys the base `CallbackOwner` via `0x00695870` (`0x008defdc`).
- **Uncertainty**: None.

### 18. `0x008df000` — `objective_sync_read_objectives_008df000` (`BSP_ObjectiveSync_ReadObjectives`)
- **Original ABI**: `__thiscall (ECX = ObjectiveSet* self, void* archive_reader), RET 0x4`
- **Evidence**: Reads `"objectiveEntities"` (`0x00d160ec`), inserting each entity into `self+0x18` via `0x008de6a0` (`0x008df0aa`) and registering `(entity, self)` when not yet registered via `0x00694af0` (`0x008df0b3`) and `0x00694a60` (`0x008df0c0`). Then reads `"objectiveList"` (`0x00d160dc`), allocating `0x2c`-byte objectives via `0x00bf681b` (`0x008df140`), constructing them via `0x008dec10` (`0x008df15b`), and appending to `self+0x24` via `0x008db6a0` (`0x008df182`) and `0x008de2a0` (`0x008df18d`).
- **Uncertainty**: Archive vtable slot semantics are provisional.

### 19. `0x008df1e0` — `objective_sync_detach_observers_008df1e0` (`BSP_ObjectiveSync_DetachObservers`)
- **Original ABI**: `__thiscall (ECX = ObjectiveSet* self, void* entity), RET 0x4`
- **Evidence**: Checks `0x00694af0` (`0x008df1f5`) and unregisters `(entity, self)` via `0x006952a0` (`0x008df202`). Walks `self+0x24` with checked-iterator calls `0x00bf6713` (`0x008df217`, `0x008df225`, `0x008df238`), erasing `entity` from each objective via `0x008dc3e0` (`0x008df22e`). Finally erases `entity` from the RB-tree at `self+0x18` via `0x008dbba0` (`0x008df258`), `0x008db950` (`0x008df283`), and `0x008de7e0` (`0x008df29a`).
- **Uncertainty**: None.

### 20. `0x008df400` — `objective_set_remove_unit_from_named_objective_008df400` (`BSP_ObjectiveSet_RemoveUnitFromNamedObjective`)
- **Original ABI**: `__thiscall (ECX = ObjectiveSet* self, const NativeString* name, void* entity), RET 0x8`
- **Evidence**: Walks `self+0x24` with checked-iterator calls `0x00bf6713` (`0x008df438`, `0x008df44a`, `0x008df491`, `0x008df4aa`, `0x008df4bd`, `0x008df4d6`, `0x008df4f1`) and case-insensitive compare `0x00bf7fbf` (`0x008df47b`). On name match, erases `entity` via `0x008dc3e0` (`0x008df49e`); on non-matching active objectives, accumulates whether `entity` is still referenced via `0x008dc4a0` (`0x008df4e3`). If unreferenced after the loop, unregisters the observer pair via `0x00694af0` (`0x008df512`) and `0x006952a0` (`0x008df521`), erases `entity` from `self+0x18` via `0x008deb40` (`0x008df532`), builds `"SzurkeNyil"` via `0x0041dd40` (`0x008df549`) and `0x00bf7680` (`0x008df564`), drops the unit marker via `0x006de3f0` (`0x008df581`), and releases the temporary string via `0x00419cc0` (`0x008df5a1`) and `0x00bd1510` (`0x008df5a8`).
- **Uncertainty**: None.

### 21. `0x008df800` — `objective_sync_apply_obj_add_position_008df800` (`BSP_ObjectiveSync_ApplyObjAddPosition`)
- **Original ABI**: `__thiscall (ECX = ObjectiveSet* self, const SessionMessage59* msg), RET 0x4`
- **Evidence**: Unpacks `msg+0x18` and the 3-float position at `msg+0x20..+0x28` and calls `0x008df5d0` (`0x008df81e`).
- **Uncertainty**: None.

### 22. `0x008df860` — `receive28_vector_tidy_008df860` (`BSP_Receive28Vector_Tidy_008df860`)
- **Original ABI**: `__fastcall (ECX = void* self), RET`
- **Evidence**: When `self+0x04` is non-null, destroys `[self+0x04, self+0x08)` via `0x008de660` (`0x008df874`) and frees `self+0x04` via `0x00bf65ac` (`0x008df87d`), then zeroes `+0x04`, `+0x08`, and `+0x0c`.
- **Uncertainty**: None.

### 23. `0x008df900` — `objective_set_construct_008df900` (`BSP_ObjectiveSet_Construct`)
- **Original ABI**: `__thiscall (ECX = ObjectiveSet* self, int slot_index), RET 0x4`
- **Evidence**: Zeroes `+0x04..+0x10`, writes vtable `0x00d1610c` and `+0x14 = slot_index`, allocates the RB-tree head node at `+0x1c` via `0x008db7b0` (`0x008df943`), marks `head+0x11 = 1` and self-links `+0x00`/`+0x04`/`+0x08`, zeroes `+0x20`, and allocates the objective list sentinel at `+0x28` via `0x008db560` (`0x008df96d`) with `+0x2c = 0`.
- **Uncertainty**: None.

### 24. `0x008df9b0` — `objective_set_add_unit_expanding_groups_008df9b0` (`BSP_ObjectiveSet_AddUnitExpandingGroups`)
- **Original ABI**: `__thiscall (ECX = ObjectiveSet* self, const NativeString* name, void* unit), RET 0x8`
- **Evidence**: Replicates `MT_OBJ_ADD_ENTITY` on host when peer is non-null via `0x008dd780` (`0x008df9fb`), `0x00770b50` (`0x008dfa1f`), and `0x008dc980` (`0x008dfa30`). Then tests `unit` vtable `+0x5c`:
  - `5`: calls `0x008df2b0` (`0x008dfa4c`) directly.
  - `0x18`: loops `0 .. unit+0x3cc - 1`, passing `unit+0x3d0+i*4` for `i <= 4` and `nullptr` for `i > 4` to `0x008df2b0` (`0x008dfa9b`).
  - `0x1a`: walks the pointer vector `[unit+0x398, unit+0x39c)` with checked-iterator calls `0x00bf6713` (`0x008dfae3`, `0x008dfaf0`, `0x008dfaf9`, `0x008dfb0b`, `0x008dfb24`), calling `0x008df2b0` (`0x008dfb1a`).
  - `0x45`: populates a temporary vector via `0x006d6010` (`0x008dfb58`), calls `0x008df2b0` (`0x008dfb7c`) for each element, and frees the buffer via `0x00bf65ac` (`0x008dfbde`).
  - `0x46`: populates a temporary vector via `0x00848410` (`0x008dfbb0`), calls `0x008df2b0` (`0x008dfbd3`) for each element, and frees the buffer via `0x00bf65ac` (`0x008dfbde`).
- **Uncertainty**: None; the `0x45` and `0x46` loops omitted by Ghidra's dead-code pass were recovered directly from `assembly.txt`.

### 25. `0x008dfc00` — `objective_set_remove_unit_expanding_groups_008dfc00` (`BSP_ObjectiveSet_RemoveUnitExpandingGroups`)
- **Original ABI**: `__thiscall (ECX = ObjectiveSet* self, const NativeString* name, void* unit), RET 0x8`
- **Evidence**: Removal twin of `0x008df9b0`: replicates `MT_OBJ_REMOVE_ENTITY` via `0x008dd820` (`0x008dfc4b`), `0x00770b50` (`0x008dfc6f`), and `0x008dcb40` (`0x008dfc80`), then expands `unit` across vtable `+0x5c` selectors `5`, `0x18`, `0x1a` (with `0x00bf6713` checks at `0x008dfd33`, `0x008dfd40`, `0x008dfd49`, `0x008dfd5b`, `0x008dfd74`), `0x45` (`0x006d6010` at `0x008dfda8`), and `0x46` (`0x00848410` at `0x008dfe00`, `0x00bf65ac` at `0x008dfe2e`) into calls to `0x008df400` (`0x008dfc9c`, `0x008dfceb`, `0x008dfd6a`, `0x008dfdcc`, `0x008dfe23`).
- **Uncertainty**: None.

### 26. `0x008e0060` — `objective_sync_apply_obj_add_entity_008e0060` (`BSP_ObjectiveSync_ApplyObjAddEntity`)
- **Original ABI**: `__thiscall (ECX = ObjectiveSet* self, const SessionMessage57* msg), RET 0x4`
- **Evidence**: Zero-extends the 16-bit handle at `msg+0x20`, selects `[0x00f89a54]` / `[0x00f89a0c]` when `< [0x00f89a10]` or `[0x00f89aa8]` / `[0x00f89a60]` otherwise, loads `table[(handle - base) * 0x10 + 0x0c]`, and calls `0x008df9b0` (`0x008e009a`).
- **Uncertainty**: None.

### 27. `0x008e00b0` — `objective_sync_apply_obj_remove_entity_008e00b0` (`BSP_ObjectiveSync_ApplyObjRemoveEntity`)
- **Original ABI**: `__thiscall (ECX = ObjectiveSet* self, const SessionMessage58* msg), RET 0x4`
- **Evidence**: Resolves `msg+0x20` through the same two-band handle table and calls `0x008dfc00` (`0x008e00ea`).
- **Uncertainty**: None.

### 28. `0x008e0100` — `receive28_vector_tidy_008e0100` (`BSP_Receive28Vector_Tidy_008e0100`)
- **Original ABI**: `__fastcall (ECX = void* self), RET`
- **Evidence**: Identical body to `0x008df860`: calls `0x008de660` (`0x008e0114`) and `0x00bf65ac` (`0x008e011d`) when `self+0x04 != nullptr`, then zeroes `+0x04`, `+0x08`, and `+0x0c`.
- **Uncertainty**: None.

### 29. `0x008e1380` — `session_message55_construct_obj_create_snapshot_008e1380` (`BSP_SessionMessage55_ConstructObjCreateSnapshot`)
- **Original ABI**: `__thiscall (ECX = SessionMessage55* self, const ObjectiveSet* set), RET 0x4`
- **Evidence**: Calls `0x0075b430` (`0x008e13a4`) with `0x37` (`MT_OBJ_CREATE`), sets `+0x04 = 1` and vtable `0x00d16124`, allocates list sentinel at `+0x1c` via `0x008db560` (`0x008e13c1`), zeroes `+0x20` and vector fields `+0x28..+0x30`, and walks `set+0x24` with checked-iterator calls `0x00bf6713` (`0x008e13fb`, `0x008e140b`, `0x008e1451`), appending every objective with `state (+0x1c) == 0` via `0x008db6a0` (`0x008e142c`) and `0x008de2a0` (`0x008e1437`).
- **Uncertainty**: None.

### 30. `0x008e1720` — `objective_set_send_create_snapshot_to_peer_008e1720` (`BSP_ObjectiveSet_SendCreateSnapshotToPeer`)
- **Original ABI**: `__thiscall (ECX = ObjectiveSet* self, const void* peer_owner), RET 0x4`
- **Evidence**: Constructs a stack `SessionMessage55` via `0x008e1380` (`0x008e173d`), sends it to `peer_owner+0x50` via `0x00770b50` (`0x008e175f`), and destroys it via `0x008e1480` (`0x008e176f`).
- **Uncertainty**: None.

### 31. `0x008e1b90` — `objective_set_announce_added_to_local_party_008e1b90` (`BSP_ObjectiveSet_AnnounceAddedToLocalParty`)
- **Original ABI**: `__thiscall (ECX = ObjectiveSet* self, Objective* objective), RET 0x4`
- **Evidence**: When `self+0x14` matches local party `[[0x00e188a8]+0x18ec]` in `0..7` and `objective+0x18` is `0` or `1`, plays sound `0` on `GlobalConfig+0x2c0` via `0x00432650` (`0x008e1bed`) and `0x008dd460` (`0x008e1bfa`). Tests `objective+0x0c` against `""` (`0x00ce3a0c`) via `0x0041e870` (`0x008e1c08`), `0x00449af0` (`0x008e1c17`), `0x00419cc0` (`0x008e1c31`), and `0x00bd1510` (`0x008e1c38`). When non-empty, builds an empty subtitle string (`0x0041e870` at `0x008e1c4e`), optionally prefixes `"missionglobals.secobjprefix|.: |"` (`0x00d16138`) for secondary objectives (`kind == 1`) via `0x0041e870` (`0x008e1c6a`) and `0x004261a0` (`0x008e1c84`), announces via `0x00734870` (`0x008e1cb6`), and cleans up temporary strings via `0x0041dd20` (`0x008e1cd3`, `0x008e1ce9`), `0x00419cc0` (`0x008e1d09`), and `0x00bd1510` (`0x008e1d10`).
- **Uncertainty**: None.

### 32. `0x008e1d30` — `objective_set_announce_state_change_008e1d30` (`BSP_ObjectiveSet_AnnounceStateChange`)
- **Original ABI**: `__thiscall (ECX = ObjectiveSet* self, const Objective* obj, const NativeString* text, uint8_t completed_flag, uint8_t silent_flag), RET 0x10`
- **Evidence**: Under the same local-party and `kind in {0, 1}` gate, when `silent_flag == 0`, plays sound `1` (completed) or `2` (failed) via `0x00432650` (`0x008e1d94`) and `0x008dd460` (`0x008e1da1`). Tests `text` against `""` via `0x0041e870` (`0x008e1db1`), `0x00449af0` (`0x008e1dc1`), `0x00419cc0` (`0x008e1ddb`), and `0x00bd1510` (`0x008e1de2`); when non-empty, announces `text` with an empty second string (`0x0041e870` at `0x008e1df4`) via `0x00734870` (`0x008e1e15`), then releases the temporary string via `0x00419cc0` (`0x008e1e35`) and `0x00bd1510` (`0x008e1e3c`).
- **Uncertainty**: None.

### 33. `0x008e1e60` — `objective_sync_apply_obj_create_008e1e60` (`BSP_ObjectiveSync_ApplyObjCreate`)
- **Original ABI**: `__thiscall (ECX = ObjectiveSet* self, SessionMessage55* msg), RET 0x4`
- **Evidence**: Copies `msg+0x18` into `self+0x24` via `0x008df8a0` (`0x008e1e87`), walks `self+0x24` with checked-iterator calls `0x00bf6713` (`0x008e1e98`, `0x008e1ea6`, `0x008e1ebd`) to announce each objective via `0x008e1b90` (`0x008e1eb3`), and iterates the `0x1c`-byte records in `[msg+0x28, msg+0x2c)` (with bounds check `0x00bf6713` at `0x008e1f1a`), dispatching entity records (`rec+0x08 != 0`) to `0x008df9b0` (`0x008e1f33`) and position records (`rec+0x08 == 0`) to `0x008df5d0` (`0x008e1f5b`).
- **Uncertainty**: None.

### 34. `0x008e1f80` — `objective_set_add_008e1f80` (`BSP_ObjectiveSet_Add`)
- **Original ABI**: `__thiscall (ECX = ObjectiveSet* self, const NativeString* id, const NativeString* text, int kind, uint8_t quiet_flag), RET 0x10`
- **Evidence**: Allocates `0x2c` bytes via `0x00bf681b` (`0x008e1f9e`) and constructs the objective via `0x008dd5c0` (`0x008e1fca`). Replicates `MT_OBJ_ADD` on host when peer is non-null via `0x008dd6a0` (`0x008e200d`), `0x00770b50` (`0x008e2031`), and `0x008dc7a0` (`0x008e203e`). Appends the objective to `self+0x24` via `0x008db6a0` (`0x008e2059`) and `0x008de2a0` (`0x008e2064`), announces when `quiet_flag == 0` via `0x008e1b90` (`0x008e2081`), and zeroes `[[0x00e198c4]+0x60]+0x65`.
- **Uncertainty**: None.

### 35. `0x008e2200` — `objective_set_set_failed_008e2200` (`BSP_ObjectiveSet_SetFailed`)
- **Original ABI**: `__thiscall (ECX = ObjectiveSet* self, const NativeString* id, const NativeString* comment, uint8_t silent_flag), RET 0xc`
- **Evidence**: Walks `self+0x24` with checked-iterator calls `0x00bf6713` (`0x008e222d`, `0x008e223f`, `0x008e2287`) and case-insensitive compare `0x00bf7fbf` (`0x008e2271`). On match, calls `0x008e1d30` (`0x008e229f`) with `completed = 0`, clears markers via `0x008dfe50` (`0x008e22a7`), sets `obj+0x1c = 2`, and replicates `MT_OBJ_FAILED` on host via `0x008ddb00` (`0x008e22d8`), `0x00770b50` (`0x008e2300`), and `0x008dd120` (`0x008e2311`).
- **Uncertainty**: None.

### 36. `0x008e2350` — `objective_sync_apply_obj_completed_008e2350` (`BSP_ObjectiveSync_ApplyObjCompleted`)
- **Original ABI**: `__thiscall (ECX = ObjectiveSet* self, const SessionMessage61* msg), RET 0x4`
- **Evidence**: Calls `0x008e20d0` (`0x008e2361`) with `(msg+0x18, msg+0x20, msg+0x28)`.
- **Uncertainty**: None.

### 37. `0x008e2370` — `objective_sync_apply_obj_failed_008e2370` (`BSP_ObjectiveSync_ApplyObjFailed`)
- **Original ABI**: `__thiscall (ECX = ObjectiveSet* self, const SessionMessage62* msg), RET 0x4`
- **Evidence**: Calls `0x008e2200` (`0x008e2381`) with `(msg+0x18, msg+0x20, msg+0x28)`.
- **Uncertainty**: None.

### 38. `0x008e2390` — `objective_sync_dispatch_received_008e2390` (`BSP_ObjectiveSync_DispatchReceived`)
- **Original ABI**: `__fastcall (ECX = SessionMessage* msg), RET`
- **Evidence**: Queries `msg` vtable `+0x0c` for types `0x37` through `0x3e` and dispatches to the local party's `ObjectiveSet` (`[[0x00e188a8]+0x21a4+party*4]`): `0x008e1e60` (`0x008e23b3`), `0x008e1f80` (`0x008e23ec`), `0x008e0060` (`0x008e2415`), `0x008e00b0` (`0x008e243e`), `0x008df800` (`0x008e2467`), `0x008ddf40` (`0x008e2490`), `0x008e2350` (`0x008e24b9`), and `0x008e2370` (`0x008e24e2`).
- **Uncertainty**: None.

### 39. `0x008e24f0` — `thread_set_debug_name_008e24f0` (`BSP_Thread_SetDebugName`)
- **Original ABI**: `__fastcall (ECX = uint32_t thread_id, EDX = const char* name), RET`
- **Evidence**: Populates `THREADNAME_INFO {0x1000, name, thread_id, 0}` and calls `RaiseException(0x406d1388, 0, 4, &info)` via `[0x00ce2280]` (`0x008e2537`).
- **Uncertainty**: Host projection delegates the SEH-guarded exception raise to `Seg57Host::raise_exception_00ce2280`.

### 40. `0x008e2560` — `flag_cloth_process_frame_008e2560` (`BSP_FlagCloth_ProcessFrame`)
- **Original ABI**: `__stdcall ([ESP+4] = float dt), RET 0x4`
- **Evidence**: Calls `0x0070aa70` (`0x008e256e`, `0x008e2581`, `0x008e2594`) and `0x0070a710` (`0x008e259f`, `0x008e25aa`, `0x008e25b5`) on the three flag instances at `[0x00e19b08]`, `[0x00e19b04]`, and `[0x00e19b0c]`.
- **Uncertainty**: None.

### 41. `0x008e25f0` — `map_ping_reset_cooldown_008e25f0` (`BSP_MapPing_ResetCooldown`)
- **Original ABI**: `__cdecl (void), RET`
- **Evidence**: Stores `0.0f` into `[0x00f88c28]`.
- **Uncertainty**: None.

### 42. `0x008e2b90` — `map_ping_list_construct_008e2b90` (`BSP_MapPingList_Construct`)
- **Original ABI**: `__fastcall (ECX = MapPingList* self), RET`
- **Evidence**: Sets vtable `0x00d16168`, allocates the head node at `+0x08` via `0x008e28f0` (`0x008e2b9f`), and zeroes `+0x0c`.
- **Uncertainty**: None.

### 43. `0x008e2bf0` — `map_ping_list_prune_expired_008e2bf0` (`BSP_MapPingList_PruneExpired`)
- **Original ABI**: `__fastcall (ECX = MapPingList* self), RET`
- **Evidence**: Walks `self+0x04` with checked-iterator calls `0x00bf6713` (`0x008e2c07`, `0x008e2c17`, `0x008e2c22`, `0x008e2c2c`, `0x008e2c45`), unlinking and freeing via `0x00bf65ac` (`0x008e2c5f`) every node whose expiry float at `+0x18` is `<= [0x00f876a4]` and decrementing `self+0x0c`.
- **Uncertainty**: None.

### 44. `0x008e2da0` — `map_ping_handle_received_008e2da0` (`BSP_MapPing_HandleReceived`)
- **Original ABI**: `__fastcall (ECX = void* msg), RET`
- **Evidence**: On host (`[[0x00e188a8]+0x1fe4] == 1`), rebroadcasts `msg` via `0x0076a9f0` (`0x008e2dbe`). When `[[0x00e188a8]+0x21c8]` is non-null, local party `[[0x00e188a8]+0x18ec]` is in `0..7`, and the sender's team (`player+0x28`) equals the local player's team, pushes `{sender_party, msg+0x1c, msg+0x20, msg+0x24, [0x00f876a4] + [0x00ce3db0]}` via `0x008e2d60` (`0x008e2e38`) and sets sound request queue `+0x1a = 1` via `0x004c1b90` (`0x008e2e3d`).
- **Uncertainty**: None.

### 45. `0x008e2e50` — `map_ping_add_local_008e2e50` (`BSP_MapPing_AddLocal`)
- **Original ABI**: `__stdcall (float x, float y), RET 0x8`
- **Evidence**: Builds `{local_party, 0, x, y, [0x00f876a4] + [0x00ce3db0]}`, links a new node into `[[0x00e188a8]+0x21c8]+0x04` via `0x008e2b40` (`0x008e2ea9`) and `0x008e2c80` (`0x008e2eb4`), and sets sound request queue `+0x1a = 1` via `0x004c1b90` (`0x008e2ec1`).
- **Uncertainty**: None.

### 46. `0x008e34f0` — `powerup_class_resolve_air_support_vehicle_classes_008e34f0` (`BSP_PowerupClass_ResolveAirSupportVehicleClasses`)
- **Original ABI**: `__fastcall (ECX = PowerupClass* self), RET`
- **Evidence**: When `self+0x08 == 1`, resolves `+0x2c` from unit type `+0x24` (party 0) via `0x0095ba60` (`0x008e3508`), `0x0046be90` (`0x008e3517`), and `0x00964790` (`0x008e3521`), and resolves `+0x64` from unit type `+0x5c` (party 1) via `0x0095ba60` (`0x008e353b`), `0x0046be90` (`0x008e354a`), and `0x00964790` (`0x008e3554`).
- **Uncertainty**: None.

### 47. `0x008e3570` — `powerup_modifier_matches_message68_008e3570` (`BSP_PowerupModifier_MatchesMessage68`)
- **Original ABI**: `__thiscall (ECX = const PowerupModifier* self, const SessionMessage68* msg), RET 0x4`
- **Evidence**: Compares `self+0x04 == msg+0x18`, `self+0x14 == msg+0x20` (float), `self+0x0c == msg+0x24` (int16), `self+0x0e == msg+0x26` (int16), `self+0x10 == msg+0x28`, `self+0x08 == msg+0x30`, and `self+0x18 == msg+0x34` (float).
- **Uncertainty**: None.

### 48. `0x008e35d0` — `command_building_powerup_binding_construct_008e35d0` (`BSP_CommandBuildingPowerupBinding_Construct`)
- **Original ABI**: `__thiscall (ECX = void* self, void* owner), RET 0x4`
- **Evidence**: Stores vtable `0x00d16194` at `+0x00`, `owner` at `+0x04`, and `0` at `+0x08`.
- **Uncertainty**: None.

### 49. `0x008e35f0` — `powerup_class_scope_accepts_unit_008e35f0` (`BSP_PowerupClass_ScopeAcceptsUnit`)
- **Original ABI**: `__fastcall (ECX = void* unit, EDX = const PowerupClass* cls), RET`
- **Evidence**: Returns `true` when `cls+0x84` is neither `1` nor `2`. Returns `false` if `unit == nullptr`, or `true` if `cls+0x88 == 0x0f`. Otherwise tests `unit` vtable `+0x5c`: `8` -> bit 1, `6` -> bit 0, `0x18` -> bit 2, and `0x009fe0b0` (`0x008e3668`) -> bit 3.
- **Uncertainty**: None.

### 50. `0x008e3680` — `powerup_class_unit_kind_mask_accepts_008e3680` (`BSP_PowerupClass_UnitKindMaskAccepts`)
- **Original ABI**: `__fastcall (ECX = void* unit, EDX = const PowerupClass* cls), RET`
- **Evidence**: Returns `false` if `unit == nullptr`, or `true` if `cls+0x88 == 0x0f`. Tests `unit` vtable `+0x5c`: `8` -> bit 1, `6` -> bit 0, `0x18` or `0x0f` -> bit 2, and `0x009fe0b0` (`0x008e36e9`) -> bit 3.
- **Uncertainty**: None.

### 51. `0x008e3710` — `powerup_slot_record_reset_008e3710` (`BSP_PowerupSlotRecord_Reset`)
- **Original ABI**: `__fastcall (ECX = void* self), RET`
- **Evidence**: Zeroes `+0x08` (u32), `+0x14` (u8), `+0x0c` (f32), `+0x10` (f32), `+0x04` (u32), and `+0x00` (u8).
- **Uncertainty**: None.

---

## Skipped

- **`0x008e2ee0` (`BSP_MapPing_SendLocal`)**: Larger map-ping transmission and chat/sound formatting routine in Segment 57 outside the 51-function `argon-v2-seg57` target list; left for a dedicated follow-up packet.

# Segment 64 Warning, String, and Subscription Helpers (`0096aa00` through `0097aaf0`)

Packet: `argon-v2-seg64`  
Source: `src/warning_string_seg64_argon.cpp`  
Header: `include/bsp/warning_string_seg64_argon.hpp`  
Report: `reports/argon_seg64_warning_string.json`  
Status: `reconstructed_build_tested` (`abi_compatible: false`, `game_validated: false`)

## Scope and Evidence Summary

This document covers the 47 remaining functions in Segment 64 (`0x0096aa00` through `0x0097aaf0`) belonging to packet `argon-v2-seg64`. Every function was verified against both `decompiled.c` and `assembly.txt` in `exports/bsp/functions/` and reconstructed in `bsp::warning_string_seg64_argon` over `Raw` and `Seg64Host` without `fn_at` or raw address function-pointer casts.

### x87 `DC`-Form Audit

Running `x87_dc_form_scan.py` over `0x0096aa00` through `0x0097aaf0` found 1 instruction site (`0x0096ef8a` inside already-reconstructed `0x0096ef60` `BSP_WarningManager_ReportHitFeedback`) and **0 sites** inside the 47 functions of `argon-v2-seg64`. Three functions in this packet use standard `DC C0` (`FADD ST(0), double ptr [...]` with `ST(0)` destination, not reversed `DC` forms):
- `0x00976f10` at `0x00976fc8`: `FLD float ptr [0x00f876a4]; FADD double ptr [0x00d7a308]; FSTP float ptr [ESP + 0x14]`.
- `0x00977820` at `0x0097788a`: `FLD float ptr [0x00f876a4]; ... FADD double ptr [0x00ce7630]; FSTP float ptr [ECX]`.
- `0x00977990` at `0x00977b26`: `FLD float ptr [0x00f876a4]; FADD double ptr [0x00d7a308]; FSTP float ptr [ESI + 0x4]`.

### Sized Storage Pool Release Idiom

Four functions (`0x0096cd90`, `0x0096cdc0`, `0x0096d7b0`, `0x0096ecc0`) release `NativeString` buffers using the two-step singleton pool idiom:
1. Push `(data, length + 1, 1)` for the following `__thiscall`, then call `0x00419cc0` (`BSP_SizedStoragePool_GetSingleton`), which takes **0 arguments** (`RET`) and returns the pool singleton in `EAX`.
2. Move `EAX` to `ECX` and call `0x00bd1510` (`BSP_SizedStoragePool_ReturnBlock`), which is `__thiscall(pool, data, length + 1, 1)` (`RET 0xC`).

---

## Function-by-Function Evidence and ABI Notes

### 1. `0x0096aa00` — `warning_activate_0096aa00` (`TRIV_body_0096aa00`)
- **Original ABI**: `__thiscall(void* this)` (`RET`).
- **Behavior**: Sets `*(uint32_t*)(this + 0x0c) = 1` (warning state = active) and copies `float` clock `0x00f876a4` into `*(float*)(this + 0x10)`. Base `Warning::Activate` virtual slot (`vtable + 0x0c`).
- **Calls**: None.

### 2. `0x0096c280` — `warning_list_remove_by_value_0096c280` (`BSP_WarningList_RemoveByValue`)
- **Original ABI**: `__thiscall(void* list, void* const* value_ptr)` (`RET 0x4`).
- **Behavior**: Walks `std::list<Warning*>` from `head = *(Node**)(list + 0x04)` (`node = head->next`). Preserves all 5 checked-iterator calls to `0x00bf6713` (`0x0096c2a8`), `0x00bf6713` (`0x0096c2b5`), `0x00bf6713` (`0x0096c2bf`), `0x00bf6713` (`0x0096c2d6`), and `0x00bf6713` (`0x0096c309`). For each node where `node->value` (`+0x08`) equals `*value_ptr`, unlinks `node`, calls `0x00bf65ac` (`0x0096c2f0`) (`_free`), and decrements `*(int32_t*)(list + 0x08)`.

### 3. `0x0096cd90` — `vtable_native_string_release_storage_0096cd90` (`BSP_NativeString_ReleaseStorage_0096cd90`)
- **Original ABI**: `__thiscall(void* this)` (`RET`).
- **Behavior**: Installs vtable `0x00d1af88` at `[this + 0x00]`. If `data = *(void**)(this + 0x08)` is non-null, computes `bytes = *(uint32_t*)(this + 0x04) + 1`, calls `0x00419cc0` (`0x0096cda7`), and calls `0x00bd1510` (`0x0096cdae`).

### 4. `0x0096cdc0` — `vtable_native_string_scalar_deleting_dtor_0096cdc0` (`CG_scalar_deleting_dtor_0096cdc0`)
- **Original ABI**: `__thiscall(void* this, uint8_t flags) -> void*` (`RET 0x4`).
- **Behavior**: Installs vtable `0x00d1af88` at `[this + 0x00]`, releases `+0x04`/`+0x08` string storage via `0x00419cc0` (`0x0096cdda`) and `0x00bd1510` (`0x0096cde1`), and if `(flags & 1) != 0` frees `this` via `0x00bf65ac` (`0x0096cdee`).

### 5. `0x0096d330` — `string_list_warning_save_archive_0096d330` (`BSP_StringListWarning_SaveArchive`)
- **Original ABI**: `__thiscall(void* this, void* archive)` (`RET 0x4`).
- **Behavior**: Calls `0x0096cf10` (`0x0096d33a`) (`BSP_Warning_SaveArchive`), invokes `vtable[+0x04]` on `archive` with `(0, 0x00ce4ad8)` (`"msg"`), iterates the `NativeString` vector at `this + 0x70`/`+0x74` (8-byte elements, checked with `0x00bf6713` (`0x0096d383`)), substitutes empty string `0x00f8a0c8` when `elem.data` (`+0x04`) is null, invokes `vtable[+0x0c]` on `archive` with `(1, index, 0, str_ptr)`, and finishes by invoking `vtable[+0x08]` on `archive`.

### 6. `0x0096d7b0` — `native_string_release_storage_0096d7b0` (`BSP_NativeString_ReleaseStorage_0096d7b0`)
- **Original ABI**: `__thiscall(void* this)` (`RET`).
- **Behavior**: Releases a bare 8-byte `NativeString` (`+0x00` length, `+0x04` data): when `data != nullptr`, calls `0x00419cc0` (`0x0096d7c0`) and `0x00bd1510` (`0x0096d7c7`) with `(data, length + 1, 1)`.

### 7. `0x0096db10` — `ref_holder_scalar_deleting_dtor_0096db10` (`CG_scalar_deleting_dtor_0096db10`)
- **Original ABI**: `__thiscall(void* this, uint8_t flags) -> void*` (`RET 0x4`).
- **Behavior**: If `ref = *(void**)(this + 0x18)` is non-null, decrements `*(int32_t*)(ref + 0x04)` via `[0x00ce2220]` (`InterlockedDecrement`), invokes `vtable[+0x00]` on `ref` when the count reaches zero, and clears `*(void**)(this + 0x18) = nullptr`. If `(flags & 1) != 0`, calls `0x00bf65ac` (`0x0096db40`).

### 8. `0x0096dca0` — `warning_event_record_get_kind_0096dca0` (`TRIV_body_0096dca0`)
- **Original ABI**: `__thiscall` / parameterless (`RET`) -> `EAX = 3`.

### 9. `0x0096dd10` — `warning_event_record_scalar_deleting_dtor_0096dd10` (`CG_scalar_deleting_dtor_0096dd10`)
- **Original ABI**: `__thiscall(void* this, uint8_t flags) -> void*` (`RET 0x4`).
- **Behavior**: Calls `0x0096dcb0` (`0x0096dd13`) (`BSP_WarningEventRecord_Destruct`) and conditionally calls `0x00bf65ac` (`0x0096dd20`).

### 10. `0x0096ecc0` — `offset_native_string_scalar_deleting_dtor_0096ecc0` (`CG_scalar_deleting_dtor_0096ecc0`)
- **Original ABI**: `__thiscall(void* this, uint8_t flags) -> void*` (`RET 0x4`).
- **Behavior**: Releases embedded `NativeString` at `+0x0c` (length) / `+0x10` (data) via `0x00419cc0` (`0x0096ecd4`) and `0x00bd1510` (`0x0096ecdb`), then conditionally calls `0x00bf65ac` (`0x0096ece8`).

### 11–36, 38, 44. Warning Virtual Getters and Scalar Deleting Destructors
- **Constant/float getters**:
  - `0x0096f220` (`warning_get_zero_lifetime_0096f220`): `FLDZ; RET` -> `0.0f`.
  - `0x0096f240` (`warning_get_default_cooldown_0096f240`): `FLD float ptr [0x00ce5380]; RET`.
  - `0x00973c50` (`entity_warning_get_kind1_00973c50`): `MOV EAX, 1; RET`.
  - `0x00973cd0` (`entity_warning_get_kind2_00973cd0`): `MOV EAX, 2; RET`.
  - `0x00973ce0` (`entity_warning_kind2_get_cooldown_00973ce0`): `FLD float ptr [0x00ce38c8]; RET`.
  - `0x00973d50` (`warning_get_kind3_00973d50`): `MOV EAX, 3; RET`.
  - `0x00973dc0` (`entity_warning_get_kind4_00973dc0`): `MOV EAX, 4; RET`.
  - `0x00973e40` (`entity_warning_get_kind5_00973e40`): `MOV EAX, 5; RET`.
  - `0x00973ec0` (`entity_warning_get_kind6_00973ec0`): `MOV EAX, 6; RET`.
  - `0x00975710` (`warning_get_kind7_00975710`): `MOV EAX, 7; RET`.
  - `0x00975770` (`warning_get_kind9_00975770`): `MOV EAX, 9; RET`.
  - `0x009758a0` (`warning_get_kind8_009758a0`): `MOV EAX, 8; RET`.
  - `0x00975a80` (`warning_get_kind10_00975a80`): `MOV EAX, 10; RET`.
  - `0x00976280` (`string_list_warning_get_kind0_00976280`): `XOR EAX, EAX; RET`.
- **Scalar deleting destructors (`__thiscall(this, flags)`, `RET 0x4`)**:
  - Base `Warning` destructors calling `0x0096f170` then `0x00bf65ac`:
    - `0x0096f250`: `0x0096f170` (`0x0096f253`), `0x00bf65ac` (`0x0096f260`).
    - `0x00973d70`: `0x0096f170` (`0x00973d73`), `0x00bf65ac` (`0x00973d80`).
    - `0x00975730`: `0x0096f170` (`0x00975733`), `0x00bf65ac` (`0x00975740`).
    - `0x00975790`: `0x0096f170` (`0x00975793`), `0x00bf65ac` (`0x009757a0`).
    - `0x009758c0`: `0x0096f170` (`0x009758c3`), `0x00bf65ac` (`0x009758d0`).
    - `0x00975aa0`: `0x0096f170` (`0x00975aa3`), `0x00bf65ac` (`0x00975ab0`).
  - `EntityWarning` destructors calling `0x0096f270` then `0x00bf65ac`:
    - `0x00973b80`: `0x0096f270` (`0x00973b83`), `0x00bf65ac` (`0x00973b90`).
    - `0x00973c80`: `0x0096f270` (`0x00973c83`), `0x00bf65ac` (`0x00973c90`).
    - `0x00973d10`: `0x0096f270` (`0x00973d13`), `0x00bf65ac` (`0x00973d20`).
    - `0x00973df0`: `0x0096f270` (`0x00973df3`), `0x00bf65ac` (`0x00973e00`).
    - `0x00973e70`: `0x0096f270` (`0x00973e73`), `0x00bf65ac` (`0x00973e80`).
    - `0x00973f00`: `0x0096f270` (`0x00973f03`), `0x00bf65ac` (`0x00973f10`).
  - `StringListWarning` destructor calling `0x009762a0` then `0x00bf65ac`:
    - `0x00978fe0`: `0x009762a0` (`0x00978fe3`), `0x00bf65ac` (`0x00978ff0`).

### 28. `0x00974070` — `warning_manager_apply_warning_00974070` (`BSP_WarningManager_ApplyWarning`)
- **Original ABI**: `__thiscall(void* manager, void* warning)` (`RET 0x4`).
- **Behavior**:
  - Checks `warning + 0x50` (positional speaking entity):
    - If non-null, validates that the 12-byte clip vector at `warning + 0x18`/`+0x1c` is non-empty (calling `0x00bf6713` (`0x0097409f`) when empty), forms a 12-byte value `{0x00cf0dd0, first[1], first[2]}` on the stack, loads `voice_mgr = [[0x00e198c4] + 0xa4]`, and calls `0x005bbdc0` (`0x009740cb`) (`BSP_VoiceManager_PlayPositionalLine`).
    - If null, loads `voice_mgr = [[0x00e198c4] + 0xa4]` and calls `0x005bbc10` (`0x009740e9`) (`BSP_VoiceManager_PlayLine`) with `(warning + 0x14, *(uint32_t*)(warning + 0x38), *(void**)(warning + 0x68))`.
  - Sets `*(uint32_t*)(warning + 0x0c) = 2` and `*(float*)(warning + 0x10) = [0x00f876a4]`.
  - Removes `warning` from the pending list at `manager + 0xe0` via `0x0096c280` (`0x0097410d`).
  - Appends `warning` to the active list at `manager + 0xec` (head at `manager + 0xf0`) by calling `0x0096b510` (`0x0097412a`) and `0x0096f960` (`0x00974135`).

### 37. `0x00975d00` — `ship_noise_map_subscript_00975d00` (`FUN_00975d00`)
- **Original ABI**: `__thiscall(void* map, const uint32_t* key_ptr) -> float*` (`RET 0x4`).
- **Behavior**: Lower-bound lookup in the `std::map<uint32_t, ShipNoiseInfo>` RB-tree (`isnil` byte at `node + 0x1d`, key at `node + 0x0c`, mapped 12-byte `ShipNoiseInfo` at `node + 0x10`). On miss, constructs a default entry `{*key_ptr, [0x00ce4adc], [0x00ce4adc], nullptr}`, inserts via `0x00973410` (`0x00975d98`), releases any leftover ref-counted pointer in the stack temporary via `[0x00ce2220]` and `vtable[+0x00]`, validates the returned iterator with `0x00bf6713` (`0x00975dc9`) and `0x00bf6713` (`0x00975dd3`), and returns `node + 0x10`.

### 39. `0x00976cb0` — `warning_manager_report_first_string_00976cb0` (`BSP_WarningManager_ReportFirstString`)
- **Original ABI**: `__thiscall(void* manager, void* string_vector, uint32_t unused2, uint32_t unused3)` (`RET 0xC`).
- **Behavior**: Checks `*(uint8_t*)(manager + 0xd0) == 0`, `0 <= [[0x00e188a8] + 0x18ec] <= 7`, and `[0x00f876a4] > [0x00ce3d34]`. Locks `*(void**)(manager + 0x24)` (`[0x00ce2218]` and `+0x18` recursion count), allocates `0x6c` bytes via `0x00bf681b` (`0x00976d33`), checks that `string_vector` (`+0x04`/`+0x08`) has at least one 8-byte `NativeString` (else `0x00bf6713` (`0x00976d60`)), constructs the `MessageWarning` via `0x00975ac0` (`0x00976d6b`), reports it via `0x009763e0` (`0x00976d7d`), and unlocks `*(void**)(manager + 0x24)` (`[0x00ce2210]`).

### 40. `0x00976db0` — `string_list_warning_activate_00976db0` (`BSP_StringListWarning_Activate`)
- **Original ABI**: `__thiscall(void* warning)` (`RET`).
- **Behavior**: Sets `*(uint32_t*)(warning + 0x0c) = 1` and `*(float*)(warning + 0x10) = [0x00f876a4]`. Walks the `NativeString` vector at `warning + 0x70`/`+0x74` (checked via `0x00bf6713` (`0x00976df3`)), resolves each string through `0x00705e00` (`0x00976e15`) on `[[0x00e188a8] + 0x21dc]`, and pushes `{0x00cf0dd0, record, 3}` onto `warning + 0x14` via `0x005bb020` (`0x00976e2e`).

### 41. `0x00976f10` — `warning_manager_cancel_by_target_00976f10` (`BSP_WarningManager_CancelByTarget`)
- **Original ABI**: `__thiscall(void* manager, uint32_t target_id)` (`RET 0x4`).
- **Behavior**: Walks `manager + 0xe0` (`head` at `manager + 0xe4`) with all 5 checked-iterator calls (`0x00bf6713` (`0x00976f3b`), `0x00bf6713` (`0x00976f49`), `0x00bf6713` (`0x00976f58`), `0x00bf6713` (`0x00976f62`), `0x00bf6713` (`0x00976f93`)). For any warning where `vtable[+0x10]()` returns `4` and `*(uint32_t*)(warning + 0x80) == target_id`, invokes `vtable[+0x00](1)` on `warning`, unlinks the node, frees it via `0x00bf65ac` (`0x00976fad`), and decrements `*(int32_t*)(manager + 0xe8)`. After the loop completes, unconditionally stores `float([0x00f876a4] + double([0x00d7a308]))` into the `std::map` slot returned by `0x00975c40` (`0x00976fdd`) on `manager + 0x15c`.

### 42. `0x00977820` — `warning_manager_play_collision_sound_00977820` (`BSP_WarningManager_PlayCollisionSound`)
- **Original ABI**: `__thiscall(void* manager, void* ship_entity)` (`RET 0x4`).
- **Behavior**: Locks `*(void**)(manager + 0x24)`, looks up `ship_entity` in `manager + 0x184` via `0x00975d00` (`0x0097786e`), and when `[0x00f876a4] > record[0]`, updates `record[0] = float([0x00f876a4] + double([0x00ce7630]))`, builds a 4x4 diagonal matrix with `[0x00d7a24c]`, increments the refcount on `*(void**)(manager + 0x190)` (`[0x00ce221c]`), calls `0x008687c0` (`0x00977938`) (`BSP_PointEffect_CreateWithParentMatrix`) with `parent = *(void**)(ship_entity + 0x4a4)`, releases the temporary output reference (`[0x00ce2220]` / `vtable[+0x00]`), and sets `*(uint8_t*)(created + 0x09) = 1`.

### 43. `0x00977990` — `warning_manager_scan_proximity_00977990` (`BSP_WarningManager_ScanProximity`)
- **Original ABI**: `__thiscall(void* manager)` (`RET`). Note: `0x00977998` spills `ECX` (`manager`) to `[ESP + 0x04]` and reloads it at `0x00977afb` and `0x00977b15`.
- **Behavior**: Walks `[[0x00e188a8] + 0x19cc] + 0x64` (`ship = node->payload` at `+0x08`). For active qualifying ships (`+0x5c != 0`, `+0x5d == 0`, `+0x60 == 0`, `+0x5e == 0`, and at least 2 dwords in `+0x348`/`+0x34c`), refreshes pose via `0x00414db0` (`0x00977a15`) if `+0xc8 == 0`, then scans `[[0x00e188a8] + 0x19cc] + 0x13c` for an identified hostile unit (`0x00803ce0` (`0x00977a6c`) `== 1`, pose at `other + 0x3d0` refreshed via `0x00414db0` (`0x00977a87`)) within squared distance `double([0x00d09fe8])`. Looks up the ship's record via `0x00975d00` (`0x00977b0a`): on hit, calls `0x0096c070` (`0x00977b1b`) and sets `record + 0x04 = float([0x00f876a4] + double([0x00d7a308]))`; on miss past `record + 0x04`, stops the effect at `record + 0x08` via `0x00867b10` (`0x00977b52`), sets `effect + 0x09 = 1`, and releases/clears `record + 0x08`.

### 45. `0x00979140` — `subscription_set_matches_or_empty_00979140` (`BSP_SubscriptionSet_MatchesOrEmpty`)
- **Original ABI**: `__thiscall(void* this, const void* candidate) -> uint32_t` (`RET 0x4`).
- **Behavior**: Returns `1` if `*(uint32_t*)(this + 0x0c) == 0`; otherwise calls `0x009790d0` (`0x00979161`) on `this + 0x04` with `candidate + 0x04`, validates the iterator container via `0x00bf6713` (`0x00979172`), and returns `0` if `iter.node == *(void**)(this + 0x08)`, else `1`.

### 46. `0x0097a130` — `subscription_tree_owner_scalar_deleting_dtor_0097a130` (`CG_scalar_deleting_dtor_0097a130`)
- **Original ABI**: `__thiscall(void* this, uint8_t flags) -> void*` (`RET 0x4`).
- **Behavior**: Erases the tree at `this + 0x10` via `0x00492a00` (`0x0097a14a`), frees the head node via `0x00bf65ac` (`0x0097a153`), zeroes `+0x14` and `+0x18`, and conditionally frees `this` via `0x00bf65ac` (`0x0097a16b`).

### 47. `0x0097aaf0` — `fire_subscription_construct_0097aaf0` (`BSP_FireSubscription_Construct`)
- **Original ABI**: `__thiscall(void* this) -> void*` (`RET`).
- **Behavior**: Initializes `+0x04 = 0`, `+0x08 = 0`, vtable `+0x00 = 0x00d1b6dc`, embedded set vtable `+0x0c = 0x00d1b670`, allocates the tree head node via `0x0096ba00` (`0x0097ab2d`), links the empty RB-tree head at `+0x14`, sets `+0x18 = 0` and `+0x1c = 0x00d1af44`, and returns `this`.

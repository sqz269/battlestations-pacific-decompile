# Segment 54 Simulation, Fixed-Step, and Unit Damage Reconstruction (`argon-v2-seg54`)

## Scope and Artifacts

This document covers the 38 functions reconstructed in packet `argon-v2-seg54` across Segment 54 (`0x008746a0` through `0x008782a0` in `/battlestationspacific.exe`):

- Header: `include/bsp/sim_damage_seg54_argon.hpp`
- Source: `src/sim_damage_seg54_argon.cpp`
- Report: `reports/argon_seg54_sim_damage.json`

All 38 functions are reconstructed as complete C++ routines over `bsp::sim_damage_seg54_argon::Raw` and the pure-virtual `bsp::sim_damage_seg54_argon::Seg54Host` interface with zero `fn_at` or raw address casts. Names are descriptive hypotheses and not recovered symbols. These routines expose new C++ interfaces (`abi_compatible: false`, `game_validated: false`, `status: "reconstructed_build_tested"`).

## x87 `DC`-Form Audit

Running `python tools/x87_dc_form_scan.py` over `0x008746a0` through `0x008782a0` returns 1 `DC`-form instruction (`0x00875eee` inside the already-reconstructed fixed-step driver `0x00875bb0` `BSP_Game_RunFixedStep`) and **0 `DC`-form instructions** inside the 38 functions of `argon-v2-seg54`. Note that `0x00877cd0` (`unit_instance_set_health_from_scale255_00877cd0`) uses `FDIV double ptr [0x00ce4b48]` at `0x00877ce2` (`DC 35 48 4B CE 00`, standard forward `FDIV ST(0), m64fp`, not `FDIVR`), which divides the unsigned-corrected 32-bit integer value by `255.0` before multiplying by `float ptr [ECX + 0x36c]`.

## Function Inventory and Verified Evidence

1. **`0x008746a0` (`sim_object_run_fixed_steps_provisional_008746a0` / `BSP_SimObject_RunFixedSteps_Provisional`)**
   - Original ABI: `__thiscall void(void* this_ptr, int count)`, `RET 0x4`.
   - Loops `count` times when `count != 0`: invokes `vtable[+0x04](0.05f)` at `0x008746c1` with `[0x00d0de84]`, invokes `vtable[+0x0c]()` at `0x008746ca`, and invokes `vtable[+0x08](0.05f)` at `0x008746dd` with `[0x00d0de84]`.

2. **`0x008746f0` (`triv_body_008746f0` / `TRIV_body_008746f0`)**
   - Original ABI: `__thiscall void(void* this_ptr, uint32_t arg0)`, `RET 0x4`.
   - Single-instruction `RET 0x4` stub.

3. **`0x00874700` (`triv_body_00874700` / `TRIV_body_00874700`)**
   - Original ABI: `__thiscall void(void* this_ptr, uint32_t arg0)`, `RET 0x4`.
   - Single-instruction `RET 0x4` stub.

4. **`0x00874740` (`fixed_step_callback_registry_clear_singleton_unwind_00874740` / `BSP_FixedStepCallbackRegistry_ClearSingleton_Unwind`)**
   - Original ABI: `__thiscall void(void* this_ptr)` (`ECX`), `RET`.
   - Zeroes `[0x00f878c8]` and restores the base singleton vtable `0x00ce3818` at `[ECX + 0x0]`.

5. **`0x00874930` (`cg_scalar_deleting_dtor_00874930` / `CG_scalar_deleting_dtor_00874930`)**
   - Original ABI: `__thiscall void*(void* this_ptr, uint8_t flags)`, `RET 0x4`.
   - Zeroes `[0x00f878c8]`, stores vtable `0x00ce3818` at `[ECX]`, and when `(flags & 1) != 0` calls `0x00bf65ac` (`0x0087494b`).

6. **`0x00874a80` (`cg_scalar_deleting_dtor_00874a80` / `CG_scalar_deleting_dtor_00874a80`)**
   - Original ABI: `__thiscall void*(void* this_ptr, uint8_t flags)`, `RET 0x4`.
   - Zeroes `[0x00f878d0]`, stores vtable `0x00ce3818` at `[ECX]`, and when `(flags & 1) != 0` calls `0x00bf65ac` (`0x00874a9b`).

7. **`0x00874ac0` (`singleton_holder_f878d4_deleting_dtor_00874ac0` / `BSP_SingletonHolder_F878D4_DeletingDtor`)**
   - Original ABI: `__thiscall void*(void* this_ptr, uint8_t flags)`, `RET 0x4`.
   - Zeroes `[0x00f878d4]`, stores vtable `0x00ce3818` at `[ECX]`, and when `(flags & 1) != 0` calls `0x00bf65ac` (`0x00874adb`).

8. **`0x00874b00` (`singleton_holder_f878d8_deleting_dtor_00874b00` / `BSP_SingletonHolder_F878D8_DeletingDtor`)**
   - Original ABI: `__thiscall void*(void* this_ptr, uint8_t flags)`, `RET 0x4`.
   - Zeroes `[0x00f878d8]`, stores vtable `0x00ce3818` at `[ECX]`, and when `(flags & 1) != 0` calls `0x00bf65ac` (`0x00874b1b`).

9. **`0x00874b40` (`cg_scalar_deleting_dtor_00874b40` / `CG_scalar_deleting_dtor_00874b40`)**
   - Original ABI: `__thiscall void*(void* this_ptr, uint8_t flags)`, `RET 0x4`.
   - Zeroes `[0x00f878dc]`, stores vtable `0x00ce3818` at `[ECX]`, and when `(flags & 1) != 0` calls `0x00bf65ac` (`0x00874b5b`).

10. **`0x00874c10` (`cg_scalar_deleting_dtor_00874c10` / `CG_scalar_deleting_dtor_00874c10`)**
    - Original ABI: `__thiscall void*(void* this_ptr, uint8_t flags)`, `RET 0x4`.
    - Zeroes `[0x00f878cc]`, stores vtable `0x00ce3818` at `[ECX]`, and when `(flags & 1) != 0` calls `0x00bf65ac` (`0x00874c2b`).

11. **`0x00874c40` (`fixed_step_callback_registry_construct_00874c40` / `BSP_FixedStepCallbackRegistry_Construct`)**
    - Original ABI: `__thiscall void*(void* this_ptr)` (`ECX`), `RET`.
    - Stores vtable `0x00d0dea4` at `[ECX + 0x0]`, calls `0x00bd1860` (`0x00874c6b`) to allocate/initialize the tracked critical section, and stores it at `[ECX + 0x4]`.

12. **`0x00874d00` (`game_run_extra_fixed_step_00874d00` / `BSP_Game_RunExtraFixedStep`)**
    - Original ABI: `__fastcall void(uint8_t mode)` (`CL`), `RET`.
    - Drains queued Lua calls on `[[0x00e188a8] + 0x1a08]` via `0x00888230` (`0x00874d0f`). When `CL == 0`, runs due entity thinks via `0x00929460` (`0x00874d22`) with `[0x00d0de84]`. When `[0x00e188a8]` is non-null and `[[0x00e188a8] + 0x19cc]->byte[0x4ac]` is non-zero, runs `0x00778450` (`0x00874d68`), `0x0077ec20` (`0x00874d6d`), `0x00874c90` (`0x00874d72`), `0x00925f20` (`0x00874d79`), and `0x0076ffc0` (`0x00874daf`) on `[0x00e188a8] + 0x1ef0`. Finally, when `CL == 0`, drains deferred entity events via `0x00926700` (`0x00874db9`), flushes pending entity event queues via `0x009273a0` (`0x00874dbe`), and tail-jumps to `0x00903610` (`0x00874dd1`) on `[[0x00e188a8] + 0x19cc]`.

13. **`0x00874f00` (`unit_tick_element_clear_sub_list_00874f00` / `BSP_UnitTickElement_ClearSubList`)**
    - Original ABI: `__thiscall void(void* this_ptr)` (`ECX`), `RET`.
    - Drains the intrusive list header at `ECX` (`head` at `+0x00`, `tail` at `+0x04`, `count` at `+0x08`) over node links `+0x08` (`prev`) and `+0x0c` (`next`), and invokes `vtable[+0x00](1)` at `0x00874f5a` on each head node until `count == 0`.

14. **`0x00874fb0` (`cg_adjustor_thunk_00874fb0` / `CG_adjustor_thunk_00874fb0`)**
    - Original ABI: `__thiscall void*(void* this_ptr, uint8_t flags)`, `RET 0x4`.
    - Adjusts `ECX -= 4` and tail-jumps to `0x00875010` (`0x00874fb3`).

15. **`0x00875010` (`cg_scalar_deleting_dtor_00875010` / `CG_scalar_deleting_dtor_00875010`)**
    - Original ABI: `__thiscall void*(void* this_ptr, uint8_t flags)`, `RET 0x4`.
    - Zeroes `[0x00f878d0]`, writes base vtable `0x00ce3818` into `[this_ptr + 0x04]`, and when `(flags & 1) != 0` calls `0x00bf65ac` (`0x00875036`).

16. **`0x00875070` (`cg_adjustor_thunk_00875070` / `CG_adjustor_thunk_00875070`)**
    - Original ABI: `__thiscall void*(void* this_ptr, uint8_t flags)`, `RET 0x4`.
    - Adjusts `ECX -= 4` and tail-jumps to `0x00875100` (`0x00875073`).

17. **`0x00875100` (`cg_scalar_deleting_dtor_00875100` / `CG_scalar_deleting_dtor_00875100`)**
    - Original ABI: `__thiscall void*(void* this_ptr, uint8_t flags)`, `RET 0x4`.
    - Zeroes `[0x00f878d4]`, writes base vtable `0x00ce3818` into `[this_ptr + 0x04]`, and when `(flags & 1) != 0` calls `0x00bf65ac` (`0x00875126`).

18. **`0x00875180` (`cg_adjustor_thunk_00875180` / `CG_adjustor_thunk_00875180`)**
    - Original ABI: `__thiscall void*(void* this_ptr, uint8_t flags)`, `RET 0x4`.
    - Adjusts `ECX -= 4` and tail-jumps to `0x008751b0` (`0x00875183`).

19. **`0x008751b0` (`cg_scalar_deleting_dtor_008751b0` / `CG_scalar_deleting_dtor_008751b0`)**
    - Original ABI: `__thiscall void*(void* this_ptr, uint8_t flags)`, `RET 0x4`.
    - Zeroes `[0x00f878d8]`, writes base vtable `0x00ce3818` into `[this_ptr + 0x04]`, and when `(flags & 1) != 0` calls `0x00bf65ac` (`0x008751d6`).

20. **`0x00875210` (`cg_adjustor_thunk_00875210` / `CG_adjustor_thunk_00875210`)**
    - Original ABI: `__thiscall void*(void* this_ptr, uint8_t flags)`, `RET 0x4`.
    - Adjusts `ECX -= 4` and tail-jumps to `0x00875240` (`0x00875213`).

21. **`0x00875240` (`cg_scalar_deleting_dtor_00875240` / `CG_scalar_deleting_dtor_00875240`)**
    - Original ABI: `__thiscall void*(void* this_ptr, uint8_t flags)`, `RET 0x4`.
    - Zeroes `[0x00f878dc]`, writes base vtable `0x00ce3818` into `[this_ptr + 0x04]`, and when `(flags & 1) != 0` calls `0x00bf65ac` (`0x00875266`).

22. **`0x00875340` (`fixed_step_callback_registry_get_singleton_00875340` / `BSP_FixedStepCallbackRegistry_GetSingleton`)**
    - Original ABI: `__cdecl void*(void)`, `RET`.
    - Double-checked lazy initialization of `[0x00f878c8]`: acquires the singleton lifetime manager (`0x00415350` at `0x00875366`) lock, allocates 8 bytes via `0x00bf681b` (`0x0087539c`), constructs the registry via `0x00874c40` (`0x008753b3`), and registers `[0x00f878c8]` with `0x00415350` (`0x008753c6`) and `0x00bd0c30` (`0x008753d4`).

23. **`0x00875420` (`fixed_step_callback_registry_singleton_holder_dtor_00875420` / `BSP_FixedStepCallbackRegistry_SingletonHolderDtor`)**
    - Original ABI: `__thiscall void(void* this_ptr)` (`ECX`), `RET`.
    - Sets `[ECX] = 0x00d0dea4`, destroys the critical section at `ECX + 0x04` via `0x0041cc80` (`0x0087542c`), clears `[0x00f878c8]`, and sets `[ECX] = 0x00ce3818`.

24. **`0x00875450` (`cg_scalar_deleting_dtor_00875450` / `CG_scalar_deleting_dtor_00875450`)**
    - Original ABI: `__thiscall void*(void* this_ptr, uint8_t flags)`, `RET 0x4`.
    - Sets `[ECX] = 0x00d0dea4`, destroys the critical section at `ECX + 0x04` via `0x0041cc80` (`0x0087545c`), clears `[0x00f878c8]`, sets `[ECX] = 0x00ce3818`, and when `(flags & 1) != 0` calls `0x00bf65ac` (`0x00875479`).

25. **`0x00875490` (`unit_tick_element_destruct_00875490` / `BSP_UnitTickElement_Destruct`)**
    - Original ABI: `__thiscall void(void* this_ptr)` (`ECX`), `RET`.
    - Sets `[ECX] = 0x00d0dec8`, unlinks `ECX` from its intrusive list (`+0x04` prev, `+0x08` next), advances `ECX += 0x1c`, and tail-jumps to `0x00874f00` (`0x008754ab`).

26. **`0x008754d0` (`step_job_wave3_get_descriptor_008754d0` / `BSP_StepJobWave3_GetDescriptor`)**
    - Original ABI: `__cdecl void*(void)`, `RET`.
    - Double-checked lazy initialization of `[0x00f878d0]` under `0x00415350` (`0x008754f6`): allocates 8 bytes via `0x00bf681b` (`0x0087552c`), writes vtables `0x00d0deac` (`+0x00`) and `0x00d0dea8` (`+0x04`, after initial `0x00d0de90`), and registers the `+0x04` base subobject via `0x00415350` (`0x0087556e`) and `0x00bd0c30` (`0x00875575`).

27. **`0x008755a0` (`step_job_wave1_get_descriptor_008755a0` / `BSP_StepJobWave1_GetDescriptor`)**
    - Original ABI: `__cdecl void*(void)`, `RET`.
    - Double-checked lazy initialization of `[0x00f878d4]` under `0x00415350` (`0x008755c6`): allocates 8 bytes via `0x00bf681b` (`0x008755fc`), writes vtables `0x00d0deb4` (`+0x00`) and `0x00d0deb0` (`+0x04`, after initial `0x00d0de94`), and registers the `+0x04` base subobject via `0x00415350` (`0x0087563e`) and `0x00bd0c30` (`0x00875645`).

28. **`0x00875750` (`step_job_wave2_get_descriptor_00875750` / `BSP_StepJobWave2_GetDescriptor`)**
    - Original ABI: `__cdecl void*(void)`, `RET`.
    - Double-checked lazy initialization of `[0x00f878dc]` under `0x00415350` (`0x00875776`): allocates 8 bytes via `0x00bf681b` (`0x008757ac`), writes vtables `0x00d0dec4` (`+0x00`) and `0x00d0dec0` (`+0x04`, after initial `0x00d0de9c`), and registers the `+0x04` base subobject via `0x00415350` (`0x008757ee`) and `0x00bd0c30` (`0x008757f5`).

29. **`0x00875820` (`pending_registry_singleton_holder_dtor_00875820` / `BSP_PendingRegistry_SingletonHolderDtor`)**
    - Original ABI: `__thiscall void(void* this_ptr)` (`ECX`), `RET`.
    - Sets `[ECX] = 0x00d0dea0`, destroys the critical section at `ECX + 0x04` via `0x0041cc80` (`0x0087582c`), clears `[0x00f878cc]`, and sets `[ECX] = 0x00ce3818`.

30. **`0x00875920` (`tick_element_scalar_deleting_dtor_00875920` / `BSP_TickElement_ScalarDeletingDtor`)**
    - Original ABI: `__thiscall void*(void* this_ptr, uint8_t flags)`, `RET 0x4`.
    - Sets `[ECX] = 0x00d0dec8`, unlinks `ECX` from its intrusive list (`+0x04` prev, `+0x08` next), clears the sub-list at `ECX + 0x1c` via `0x00874f00` (`0x0087593e`), and when `(flags & 1) != 0` calls `0x00bf65ac` (`0x0087594b`).

31. **`0x00875ae0` (`fixed_step_callback_unregister_00875ae0` / `BSP_FixedStepCallback_Unregister`)**
    - Original ABI: `__thiscall void(void* this_ptr)` (`ECX`), `RET`.
    - Acquires the callback registry singleton via `0x00875340` (`0x00875ae4`) and its critical section at `+0x04`, unlinks `ECX` over `+0x0c` (`prev`) and `+0x10` (`next`), clears bytes `+0x18` and `+0x19`, and releases the critical section.

32. **`0x008761e0` (`unit_instance_set_numbering_008761e0` / `BSP_UnitInstance_SetNumbering`)**
    - Original ABI: `__thiscall void(void* this_ptr, int32_t number)`, `RET 0x4`.
    - When `[ECX + 0x35c] != number`, updates `[ECX + 0x35c] = number`; if `[ECX + 0x360]` is non-null and `vtable[+0x5c](6)` or `vtable[+0x5c](0x1b)` returns non-zero, forwards `number` to `0x00711be0` (`0x00876222`) on `[ECX + 0x360]`.

33. **`0x00876e20` (`unit_model_group_find_entry_00876e20` / `FUN_00876e20`)**
    - Original ABI: `__thiscall void*(void* this_ptr, int32_t key_a, int32_t key_b)`, `RET 0x8`.
    - Walks the `0x30`-byte stride array at `[ECX + 0x1c]..[ECX + 0x20]` with checked-iterator bounds validation via `0x00bf6713` (`0x00876e72`, `0x00876eb1`), returning the first entry where `[entry + 0x04] == key_a` and `[entry + 0x08] == key_b` or the front element at `[ECX + 0x1c]` if none matches.

34. **`0x008776e0` (`unit_tickable_entity_write_state_008776e0` / `BSP_UnitTickableEntity_WriteState`)**
    - Original ABI: `__thiscall void(void* this_ptr, void* writer)`, `RET 0x4`.
    - Calls `0x0077c160` (`0x008776ee`), opens `"_fragile"` (`0x00d0def8`) via `writer->vtable[+0x04]`, compares non-zero `[ECX + 0x358]` against default `{1, 0}` via `0x00bd5680` (`0x00877739`) and writes `"damagedGFXLevel"` (`0x00d0dee8`) via `writer->vtable[+0x0c]` when distinct, then opens `"damage"` (`0x00cfd7c8`), walks the pointer vector `[ECX + 0x348]..[ECX + 0x34c]` with bounds check `0x00bf6713` (`0x0087779c`), and closes both tables via `writer->vtable[+0x08]`.

35. **`0x00877b00` (`unit_tickable_entity_set_damaged_gfx_level_00877b00` / `BSP_UnitTickableEntity_SetDamagedGfxLevel`)**
    - Original ABI: `__thiscall void(void* this_ptr, int32_t level)`, `RET 0x4`.
    - When `[ECX + 0x358] != level`, calls `0x00876ec0` (`0x00877b29`) and, if `[[0x00e188a8] + 0x1fe4] == 1`, constructs session message `0x56` (`vtable 0x00d02d6c`) via `0x0075b430` (`0x00877b42`) and broadcasts it via `0x0077c7b0` (`0x00877b76`).

36. **`0x00877cd0` (`unit_instance_set_health_from_scale255_00877cd0` / `BSP_UnitInstance_SetHealthFromScale255`)**
    - Original ABI: `__thiscall void(void* this_ptr, int32_t scale_val)`, `RET 0x4`.
    - Converts unsigned 32-bit `scale_val` to floating point (adding `[0x00ce3978]` when `scale_val < 0`), divides by `double [0x00ce4b48]` (`255.0`), multiplies by `float [ECX + 0x36c]`, and calls `0x00877b90` (`0x00877cfa`).

37. **`0x008781f0` (`unit_tickable_entity_set_scene_nodes_visible_008781f0` / `BSP_UnitTickableEntity_SetSceneNodesVisible`)**
    - Original ABI: `__thiscall void(void* this_ptr, uint8_t visible)`, `RET 0x4`.
    - Iterates the holder vector `[ECX + 0x380]..[ECX + 0x384]` (bounds check `0x00bf6713` at `0x00878243`), calling `0x00b6da70` (`0x0087825e`) on each `[holder + 0x0c]` with `[0x00d7a24c]` (`1.0f`) or `0.0f` and `recurse = 0`, then walks child entities at `[ECX + 0x48]` linked through `+0x44` and recurses via `0x008781f0` (`0x00878282`) on children satisfying `vtable[+0x5c](4)`.

38. **`0x008782a0` (`unit_tickable_entity_find_damage_section_008782a0` / `BSP_UnitTickableEntity_FindDamageSection`)**
    - Original ABI: `__thiscall void**(void* this_ptr, int32_t key_a, int32_t key_b)`, `RET 0x8`.
    - Iterates the pointer vector `[ECX + 0x348]..[ECX + 0x34c]` with checked-iterator validation via `0x00bf6713` (`0x008782b7`, `0x008782c8`, `0x008782d5`, `0x008782e2`, `0x008782ec`, `0x00878301`, `0x00878316`, `0x00878325`), returning the slot pointer whose element has `[elem + 0x04] == key_a` and `[elem + 0x08] == key_b`, or `nullptr` at end.

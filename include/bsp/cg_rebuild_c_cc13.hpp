#pragma once
// cc13-cgbuildC: seventeen constructors/destructors that the old CG_* producer tag
// filed as compiler helpers (cc13-cgretag, a46023c5f, re-filed them as game code).
// Each routine is rebuilt over raw object memory; every call the binary makes is a
// CgRebuildCHost hook invoked at the binary's point with the binary's arguments.
//
// Library code is bound, not rebuilt: the CRT EH vector iterators (00BF7C6E,
// 00BF7CD1, 00BF9A96), memcpy (00BF7680), the two free thunks (00BF65AC, 00BF6989)
// and the InterlockedIncrement/Decrement imports ([00CE221C]/[00CE2220]). The
// iterator callbacks are native code addresses, not callable pointers here.
//
// EH: every routine except 00B04B10 runs under an SEH frame whose state byte names the
// members still alive. The C++ below keeps the binary's order but does not model the
// unwind funclets: a host call that throws simply propagates. The per-function comments
// give the state sequence from the listing.
//
// Names are hypotheses, not recovered symbols. Reconstructed and build-tested only;
// nothing here is called from a live host path, and none is fixture- or game-validated.

#include <cstdint>

namespace bsp::cg_rebuild_c_cc13 {

struct CgRebuildCHost {
    virtual ~CgRebuildCHost() = default;

    // --- library bindings (no reconstruction credit) ---
    // 00BF7C6E `eh vector destructor iterator'(first, size, count, dtor), __stdcall.
    virtual void eh_vector_destruct_00bf7c6e(void* first, std::uint32_t size, std::uint32_t count,
        std::uint32_t destructor) = 0;
    // 00BF7CD1 `eh vector constructor iterator'(first, size, count, ctor, dtor), __stdcall.
    virtual void eh_vector_construct_00bf7cd1(void* first, std::uint32_t size, std::uint32_t count,
        std::uint32_t constructor, std::uint32_t destructor) = 0;
    // 00BF9A96 `eh vector copy constructor iterator'(dst, src, size, count, ctor, dtor).
    virtual void eh_vector_copy_construct_00bf9a96(void* dst, const void* src, std::uint32_t size,
        std::uint32_t count, std::uint32_t constructor, std::uint32_t destructor) = 0;
    virtual void memcpy_00bf7680(void* dst, const void* src, std::uint32_t bytes) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void free_00bf6989(void* block) = 0;
    virtual std::int32_t interlocked_increment_00ce221c(void* counter) = 0;
    virtual std::int32_t interlocked_decrement_00ce2220(void* counter) = 0;

    // --- virtual dispatch through an object's own vtable (ECX = object) ---
    // vtable[slot/4](), no stack arguments.
    virtual void virtual_call_0(void* object, std::uint32_t slot_offset) = 0;
    // vtable[slot/4](arg), one stack argument.
    virtual void virtual_call_1(void* object, std::uint32_t slot_offset, std::uint32_t arg) = 0;

    // --- globals, read and written as dwords at their image address ---
    virtual std::uint32_t& global_dword(std::uint32_t address) = 0;

    // --- game code, called not rebuilt here ---
    // NativeString: 0041DD40 BSP_NativeString_Resize(ECX = header; length, preserve) RET 8.
    // A release is 00419CC0 (no arguments, EAX = pool) then 00BD1510
    // ReturnBlock(ECX = pool; data, length + 1, 1) RET 0Ch, skipped when data is null.
    virtual void native_string_resize_0041dd40(void* header, std::uint32_t length,
        std::uint32_t preserve) = 0;
    virtual void* sized_pool_00419cc0() = 0;
    virtual void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t size,
        std::uint32_t flag) = 0;

    // __thiscall(ECX = member), no stack arguments.
    virtual void sound_record_set_destruct_004c9870(void* member) = 0;
    virtual void scene_record_header_destruct_004c6520(void* member) = 0;
    // __thiscall(ECX = vector; new_size) RET 4.
    virtual void pod_pointer_array_resize_00468050(void* vector, std::uint32_t size) = 0;
    virtual void effect_handle_vector_resize_004cb160(void* vector, std::uint32_t size) = 0;
    virtual void native_string_vector_resize_00427110(void* vector, std::uint32_t size) = 0;
    virtual void native_dword_vector_resize_0086aa10(void* vector, std::uint32_t size) = 0;
    virtual void sensor_gui_range_list_resize_00807e60(void* vector, std::uint32_t size) = 0;
    // 004D1A50 BSP_RawStringTree_EraseRange, __thiscall(ECX = tree; out, tree, first,
    // tree, last) RET 14h.
    virtual void raw_string_tree_erase_range_004d1a50(void* tree, void* out, void* tree_a,
        void* first, void* tree_b, void* last) = 0;

    // XLive system members.
    virtual void xlive_system_shutdown_00a46ee0(void* system) = 0;
    virtual void stl_list_tidy_004c1f70(void* list) = 0;
    virtual void xlive_session_terminate_xnet_00a4b020(void* member) = 0;
    // 004CE520 / 00571AF0 range destroy: ECX = first, EDX = last, stack (vector, dummy)
    // where dummy is the value the caller kept in its first local (the system pointer).
    virtual void string_range_destroy_004ce520(void* first, void* last, void* vector,
        void* dummy) = 0;
    virtual void session_record_destroy_range_00571af0(void* first, void* last, void* vector,
        void* dummy) = 0;
    virtual void xlive_voice_destroy_00a493a0(void* member) = 0;
    virtual void trivial_body_00a48f00(void* member) = 0;
    virtual void xlive_host_qos_cancel_00a4b410(void* member) = 0;
    virtual void xlive_matchmaking_reset_00a4b460(void* member) = 0;
    virtual void stl_inst_00a45510(void* list) = 0;
    virtual void stl_list_clear_strings_00688780(void* list) = 0;
    virtual void game_participant_pairs_destroy_storage_004c5860(void* member) = 0;
    virtual void xlive_stats_block_destruct_00994180(void* system) = 0;

    // Front-end screens.
    virtual void front_end_screen_construct_004f7180(void* screen) = 0;
    virtual void front_end_screen_destruct_004f71a0(void* screen) = 0;
    virtual void* gui_manager_get_or_create_004c12b0() = 0;
    // __thiscall(ECX = manager; page) RET 4.
    virtual void gui_manager_remove_and_destroy_page_00aa31f0(void* manager, std::uint32_t page) = 0;
    // 00A83FD0 BSP_SoundSampleCache_AcquireByName, __thiscall(ECX = cache; out, name)
    // RET 8 -> EAX = out.
    virtual void** sound_sample_cache_acquire_00a83fd0(void* cache, void** out, const void* name) = 0;
    virtual void observed_owner_base_destroy_0064a610(void* member) = 0;

    // Effects, controllers, tasks.
    virtual void ref_counted_destroy_base_00bd30f0(void* object) = 0;
    virtual void callback_owner_destroy_00695870(void* member) = 0;
    virtual void tick_subnode_base_cleanup_00875b30(void* object) = 0;
    virtual void bot_observer_holder_destruct_full_007b3f70(void* member) = 0;
    virtual void ai_load_high_level_globals_00a335d0(void* object) = 0;
};

// 005FE010, __thiscall(ECX = dst; const src*) RET 4 -> EAX = dst. 128h-byte record.
void* hint_definition_copy_construct_005fe010(void* dst, const void* src, CgRebuildCHost& host);
// 004C8F00, __thiscall(ECX = record), RET (the 005FE010 record's teardown).
void hint_definition_destruct_004c8f00(void* record, CgRebuildCHost& host);
// 004D74E0, __thiscall(ECX = record), RET.
void scene_record_destruct_004d74e0(void* record, CgRebuildCHost& host);
// 00A475A0, __thiscall(ECX = system), RET.
void xlive_system_destruct_00a475a0(void* system, CgRebuildCHost& host);
// 006821C0, __thiscall(ECX = screen), EBP frame, RET -> EAX = screen.
void* vt_cf7184_construct_006821c0(void* screen, CgRebuildCHost& host);
// 00682030, __thiscall(ECX = screen), RET.
void vt_cf7184_destruct_00682030(void* screen, CgRebuildCHost& host);
// 0064B780, __thiscall(ECX = screen), RET -> EAX = screen.
void* vt_cf5e30_construct_0064b780(void* screen, CgRebuildCHost& host);
// 0064D740, __thiscall(ECX = screen), RET.
void vt_cf5e30_destruct_0064d740(void* screen, CgRebuildCHost& host);
// 0086E5A0, __thiscall(ECX = flare), RET.
void effect_flare_destruct_0086e5a0(void* flare, CgRebuildCHost& host);
// 0071D480, __thiscall(ECX = controller), RET.
void command_controller_base_destruct_0071d480(void* controller, CgRebuildCHost& host);
// 00A37130, __thiscall(ECX = object), RET -> EAX = object.
void* vt_d232c4_construct_00a37130(void* object, CgRebuildCHost& host);
// 00A33020, __thiscall(ECX = object), RET.
void vt_d232c4_destruct_00a33020(void* object, CgRebuildCHost& host);
// 007B4030, __thiscall(ECX = task), RET.
void bot_task_destruct_base_007b4030(void* task, CgRebuildCHost& host);
// 00808230, __thiscall(ECX = object), RET.
void unknown_dtor_00808230(void* object, CgRebuildCHost& host);
// 004CCB70, __thiscall(ECX = object), RET.
void unknown_dtor_004ccb70(void* object, CgRebuildCHost& host);
// 00808D20, __thiscall(ECX = object), RET.
void unknown_dtor_00808d20(void* object, CgRebuildCHost& host);
// 00B04B10, __thiscall(ECX = holder), RET, no EH frame.
void owned_array_release_00b04b10(void* holder, CgRebuildCHost& host);

}  // namespace bsp::cg_rebuild_c_cc13

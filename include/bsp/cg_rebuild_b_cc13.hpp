#pragma once
// Packet cc13-cgbuildB: six bodies the old producer tags called compiler helpers
// (CG_vector_deleting_dtor / CG_array_ctor_helper). cc13-cgretag (a46023c5f)
// showed they are game class constructors and destructors; this file builds
// them. Names are hypotheses, not recovered symbols.
//
// Objects are raw image storage (Raw::at at the image's byte offsets). Every
// routine the image calls is a host method invoked at the same point; library
// code (CRT free/new/memcpy, the EH vector iterators, STL tree/list/vector
// instantiations, kernel32 imports) is bound, never rebuilt. Build-only: none
// of these is wired into a live host path.
//
// EH state numbers are the values the body stores into its frame's state slot
// ([ESP+..] after the FS:[0] registration); each function's comment lists them
// in order. They decide which members the unwind funclets destroy if a call
// throws; the C++ here has no unwinding of its own (the bound calls are not
// expected to throw), so the states are documentation only.
#include <cstddef>
#include <cstdint>

namespace bsp::cg_rebuild_b_cc13 {

// Raw view of an image object: at<T>(off) is the field at that byte offset.
struct Raw {
    std::uint8_t* p;
    template <class T>
    T& at(std::size_t off) const noexcept { return *reinterpret_cast<T*>(p + off); }
    void*& ptr(std::size_t off) const noexcept { return at<void*>(off); }
};
inline Raw raw(void* p) noexcept { return Raw{static_cast<std::uint8_t*>(p)}; }

// The engine's {length, data} string (0041DD40 resizes it; storage comes from
// the sized pool and goes back as length + 1 bytes with flag 1).
struct NativeStringImage {
    std::uint32_t length;
    char* data;
};
static_assert(sizeof(void*) != 4 || sizeof(NativeStringImage) == 8, "Win32 layout");

// A checked STL iterator as the image passes it: {container*, node*}.
struct CheckedIterImage {
    void* container;
    void* node;
};

struct CgRebuildBHost {
    virtual ~CgRebuildBHost() = default;

    // ---- image globals ----
    // The dword stored at an image global address (a pointer for the singletons
    // [00F8BBCC], [00F8BBD8], [00F8BBE8]).
    virtual void* global_ptr(std::uint32_t address) = 0;
    virtual void global_store_u32(std::uint32_t address, std::uint32_t value) = 0;
    // 00E0865C..00E08663: 00525A90's two-entry table of sound paths (.data, so
    // read at run time; the image holds 00CECC90 "sound/gui/click_01.fsb" and
    // 00CECC74 "sound/gui/onmouse_01.fsb").
    virtual const char* image_cstring_table_entry(std::uint32_t address) = 0;

    // ---- library (bound, no credit) ----
    virtual void free_00bf65ac(void* block) = 0;
    // 00BF6989: a second CRT free entry (Ghidra thunk), cdecl(block).
    virtual void free_00bf6989(void* block) = 0;
    // 00BF681B operator new, cdecl(bytes) -> EAX.
    virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
    // 00BF7680 memcpy, cdecl(dst, src, count).
    virtual void crt_memcpy_00bf7680(void* destination, const void* source, std::uint32_t count) = 0;
    // 00BF6713 _invalid_parameter_noinfo (checked-iterator failures).
    virtual void invalid_parameter_00bf6713() = 0;
    // 00BF7CD1 `eh vector constructor iterator', __stdcall(first, size, count,
    // ctor, dtor) RET 14h; 00BF7C6E `eh vector destructor iterator',
    // __stdcall(first, size, count, dtor) RET 10h. Callbacks by image address.
    virtual void eh_vector_constructor_iterator_00bf7cd1(void* first, std::uint32_t size, std::uint32_t count,
                                                         std::uint32_t ctor, std::uint32_t dtor) = 0;
    virtual void eh_vector_destructor_iterator_00bf7c6e(void* first, std::uint32_t size, std::uint32_t count,
                                                        std::uint32_t dtor) = 0;
    // [00CE221C] InterlockedIncrement / [00CE2220] InterlockedDecrement
    // (__stdcall, EAX), [00CE2210] LeaveCriticalSection, [00CE2214]
    // DeleteCriticalSection (__stdcall).
    virtual std::int32_t interlocked_increment_00ce221c(std::int32_t* value) = 0;
    virtual std::int32_t interlocked_decrement_00ce2220(std::int32_t* value) = 0;
    virtual void leave_critical_section_00ce2210(void* section) = 0;
    virtual void delete_critical_section_00ce2214(void* section) = 0;
    // STL tree erase-range instantiations, __thiscall(ECX = tree; iterator* out,
    // CheckedIter first, CheckedIter last) RET 14h; `callee` names which one.
    virtual void stl_tree_erase_range(std::uint32_t callee, void* tree, CheckedIterImage* out,
                                      CheckedIterImage first, CheckedIterImage last) = 0;
    // STL tree iterator ++ instantiations, __thiscall(ECX = iterator), RET.
    virtual void stl_tree_iterator_increment(std::uint32_t callee, CheckedIterImage* iterator) = 0;

    // ---- object virtuals ----
    // vtable +0h called __thiscall(ECX = object) with no stack argument: the
    // release of a reference-counted object whose count at +4h reached zero.
    virtual void refcounted_vtable0_release(void* object) = 0;
    // vtable +0h called __thiscall(ECX = object; dword flags), RET 4: a scalar
    // deleting destructor (flags 1 deletes, 0 destroys in place).
    virtual void object_vtable0_scalar_dtor(void* object, std::uint32_t flags) = 0;

    // ---- engine (shared) ----
    // 00419CC0 BSP_SizedStoragePool_GetSingleton: no arguments; EAX.
    virtual void* sized_pool_get_00419cc0() = 0;
    // 00BD1510 BSP_SizedStoragePool_ReturnBlock: __thiscall(ECX = pool; data, bytes, dword), RET 0Ch.
    virtual void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t bytes,
                                                  std::int32_t flag) = 0;
    // 0041DD40 BSP_NativeString_Resize: __thiscall(ECX = string; length, dword), RET 8.
    virtual void native_string_resize_0041dd40(NativeStringImage* string, std::uint32_t length,
                                               std::int32_t arg) = 0;
    // 00A83FD0 BSP_SoundSampleCache_AcquireByName: __thiscall(ECX = cache; ref* out,
    // const NativeString*), RET 8; EAX = out.
    virtual void** sound_sample_cache_acquire_00a83fd0(void* cache, void** out, const NativeStringImage* name) = 0;
    // 004F7180 BSP_FrontEndScreen_Construct / 004F71A0 BSP_FrontEndScreen_Destruct:
    // __thiscall(ECX = screen), RET.
    virtual void front_end_screen_construct_004f7180(void* screen) = 0;
    virtual void front_end_screen_destruct_004f71a0(void* screen) = 0;
    // 004C12B0 BSP_GuiManager_GetOrCreate: no arguments; EAX.
    // 00AA31F0 BSP_GuiManager_RemoveAndDestroyPage: __thiscall(ECX = manager; page), RET 4.
    virtual void* gui_manager_get_or_create_004c12b0() = 0;
    virtual void gui_manager_remove_and_destroy_page_00aa31f0(void* manager, void* page) = 0;

    // ---- 00524BA0 / 00525A90 (screen 29h, vtable 00CECCF8) ----
    // 005234F0 TMPL_NodeList_Clear: __thiscall(ECX = list at +3Ch), RET.
    virtual void node_list_clear_005234f0(void* list) = 0;
    // 00695870 BSP_CallbackOwner_Destroy / 00695760 BSP_ObserverEndpoint_Destruct:
    // __thiscall(ECX = subobject), RET.
    virtual void callback_owner_destroy_00695870(void* owner) = 0;
    virtual void observer_endpoint_destruct_00695760(void* endpoint) = 0;
    // 00525A10 (unnamed; zeroes {+0,+8,+0Ch,+10h}, erases [+4h] range, +14h = -1):
    // __thiscall(ECX = +C0h), RET; EAX = ECX.
    virtual void container_init_00525a10(void* container) = 0;
    // 00A7ACF0 BSP_SoundManager_FindClassByName / 00A7B0A0
    // BSP_SoundManager_FindTypeByName: __thiscall(ECX = [00F8BBD8]; const
    // NativeString*), RET 4; EAX.
    virtual std::uint32_t sound_manager_find_class_00a7acf0(void* manager, const NativeStringImage* name) = 0;
    virtual std::uint32_t sound_manager_find_type_00a7b0a0(void* manager, const NativeStringImage* name) = 0;

    // ---- 005B9D40 / 005BA7B0 (HUD narrative screen, vtable 00CF0ED8) ----
    // 005B7840 BSP_OwnedObjectList_Clear, 005B7570 STL_List_Tidy, 005B77F0
    // STL_List_Clear: __thiscall(ECX = list), RET.
    virtual void owned_object_list_clear_005b7840(void* list) = 0;
    virtual void list_tidy_005b7570(void* list) = 0;
    virtual void list_clear_005b77f0(void* list) = 0;
    // 005B7550 STL_List_BuyHeadNode: __thiscall(ECX = list), RET; EAX = head.
    virtual void* list_buy_head_node_005b7550(void* list) = 0;
    // 007026F0 BSP_VoiceSlot_Stop: __thiscall(ECX = slot), RET.
    virtual void voice_slot_stop_007026f0(void* slot) = 0;
    // 005B8FA0 BSP_EffectSoundSamples_Resize: __thiscall(ECX = samples; count), RET 4.
    virtual void effect_sound_samples_resize_005b8fa0(void* samples, std::uint32_t count) = 0;
    // 005B9AF0 BSP_EffectSoundSamples_AppendRetained: __thiscall(ECX = samples;
    // const ref* slot), RET 4.
    virtual void effect_sound_samples_append_005b9af0(void* samples, void** slot) = 0;

    // ---- 00590600 (main menu screen) ----
    // 00543E00 BSP_DebugTextList_DestroyRows, 00544A70 (unnamed), 00590250
    // BSP_DebugTextList_Destruct, 0051F910 BSP_CheatMenuScreen_DestroyMembers,
    // 005822A0 STL_List_Tidy, 00A97140 BSP_TextListenerBase_Destroy:
    // __thiscall(ECX = member), RET. 005830A0
    // BSP_MainMenuScreen_ReleaseMissionPreview: __thiscall(ECX = screen), RET.
    virtual void debug_text_list_destroy_rows_00543e00(void* list) = 0;
    virtual void member_teardown_00544a70(void* member) = 0;
    virtual void debug_text_list_destruct_00590250(void* list) = 0;
    virtual void cheat_menu_destroy_members_0051f910(void* member) = 0;
    virtual void list_tidy_005822a0(void* list) = 0;
    virtual void text_listener_base_destroy_00a97140(void* listener) = 0;
    virtual void main_menu_release_mission_preview_005830a0(void* screen) = 0;
    // 00432050 STL_NativeStringRange_Destroy: __fastcall(ECX = first, EDX =
    // last; allocator*, dword), RET 8. 00590600 passes +4F0h and its spilled
    // `this` ([ESP+18h]) as the two stack dwords.
    virtual void native_string_range_destroy_00432050(void* first, void* last, void* allocator,
                                                      void* unused) = 0;
    // 00427110 BSP_NativeStringVector_Resize: __thiscall(ECX = vector; count), RET 4.
    virtual void native_string_vector_resize_00427110(void* vector, std::uint32_t count) = 0;

    // ---- 00989A80 (warning manager) ----
    // 0096BF70 BSP_WarningManager_ClearWaitingQueue: __thiscall(ECX = manager), RET.
    virtual void warning_manager_clear_waiting_queue_0096bf70(void* manager) = 0;
    // 0097A190 (unnamed): __thiscall(ECX = member at +11Ch / +130h), RET.
    virtual void member_teardown_0097a190(void* member) = 0;
};

// 00524BA0 __thiscall(ECX = screen), RET: screen 29h (vtable 00CECCF8) body destructor.
void unit_pick_screen_destruct_00524ba0(void* screen, CgRebuildBHost& host);
// 00525A90 __thiscall(ECX = screen), RET; EAX = screen: its constructor.
void* unit_pick_screen_construct_00525a90(void* screen, CgRebuildBHost& host);
// 005B9D40 __thiscall(ECX = screen), RET: HUD narrative screen (vtable 00CF0ED8) body destructor.
void hud_narrative_screen_destruct_005b9d40(void* screen, CgRebuildBHost& host);
// 005BA7B0 __thiscall(ECX = screen), RET; EAX = screen: its constructor.
void* hud_narrative_screen_construct_005ba7b0(void* screen, CgRebuildBHost& host);
// 00590600 __thiscall(ECX = screen), RET: main menu screen body destructor.
void main_menu_screen_destruct_00590600(void* screen, CgRebuildBHost& host);
// 00989A80 __thiscall(ECX = manager), RET: warning manager body destructor.
void warning_manager_destruct_00989a80(void* manager, CgRebuildBHost& host);

}  // namespace bsp::cg_rebuild_b_cc13

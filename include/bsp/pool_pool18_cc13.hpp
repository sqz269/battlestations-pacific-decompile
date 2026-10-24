#pragma once
// Packet cc13-pool18 (small segments 16, 20, 21): the HUD hints screen's
// constructor and destructor, the main menu screen's constructor, and the
// mission record's default and copy constructors. Names are hypotheses, not
// recovered symbols. Objects are raw image storage; every routine the image
// calls is a host method invoked at the same point. CRT (memcpy, the EH vector
// constructor/destructor/copy iterators), kernel32 interlocked imports and STL
// instantiations are bound with no credit. Build-only: nothing here is wired
// into a live host path.
#include <cstddef>
#include <cstdint>

namespace bsp::pool_pool18_cc13 {

// The engine's {length, data} string.
struct NativeStringImage {
    std::uint32_t length;
    char* data;
};

struct Pool18Host {
    virtual ~Pool18Host() = default;

    // ---- image globals ----
    // Returns the pointer dword STORED at `address` (a load such as
    // MOV ECX,[00F8BBE8]), not the address itself.
    virtual void* global_ptr(std::uint32_t address) = 0;
    virtual void global_store_u32(std::uint32_t address, std::uint32_t value) = 0;
    // A dword of a .data table read at run time (the two-entry sound path
    // tables at 00E08698 and friends).
    virtual const char* image_cstring_table_entry(std::uint32_t address) = 0;

    // ---- library (bound) ----
    virtual void crt_memcpy_00bf7680(void* destination, const void* source, std::uint32_t count) = 0;
    // 00BF7CD1 `eh vector constructor iterator' (first, size, count, ctor, dtor) RET 14h;
    // 00BF7C6E `eh vector destructor iterator' (first, size, count, dtor) RET 10h;
    // 00BF9A96 `eh vector copy constructor iterator' (dst, src, size, count, copy, dtor) RET 18h.
    virtual void eh_vector_constructor_iterator_00bf7cd1(void* first, std::uint32_t size, std::uint32_t count,
                                                         std::uint32_t ctor, std::uint32_t dtor) = 0;
    virtual void eh_vector_destructor_iterator_00bf7c6e(void* first, std::uint32_t size, std::uint32_t count,
                                                        std::uint32_t dtor) = 0;
    virtual void eh_vector_copy_constructor_iterator_00bf9a96(void* destination, const void* source,
                                                              std::uint32_t size, std::uint32_t count,
                                                              std::uint32_t copy, std::uint32_t dtor) = 0;
    // [00CE221C] InterlockedIncrement / [00CE2220] InterlockedDecrement (__stdcall, EAX).
    virtual std::int32_t interlocked_increment_00ce221c(std::int32_t* value) = 0;
    virtual std::int32_t interlocked_decrement_00ce2220(std::int32_t* value) = 0;
    // STL head-node buyers, __thiscall(ECX = container), RET; EAX = node:
    // 00582280 (list sentinel), 0056A710 (tree head).
    virtual void* stl_buy_head_node(std::uint32_t callee, void* container) = 0;
    // STL copy constructors, __thiscall(ECX = destination; const source*), RET 4:
    // 00557BC0 (checked NativeString vector), 004F6210 (checked dword vector),
    // 005C69A0 (int tree).
    virtual void stl_copy_construct(std::uint32_t callee, void* destination, const void* source) = 0;

    // ---- object virtuals ----
    // vtable +0h, __thiscall(ECX = object), no stack argument: the release of a
    // reference-counted object whose count at +4h reached zero.
    virtual void refcounted_vtable0_release(void* object) = 0;
    // vtable +8h, __thiscall(ECX = object; dword), RET 4.
    virtual void object_vtable8_call(void* object, std::uint32_t arg) = 0;

    // ---- engine ----
    virtual void* sized_pool_get_00419cc0() = 0;
    virtual void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t bytes,
                                                  std::int32_t flag) = 0;
    virtual void native_string_resize_0041dd40(NativeStringImage* string, std::uint32_t length,
                                               std::int32_t arg) = 0;
    // 00A83FD0 BSP_SoundSampleCache_AcquireByName: __thiscall(ECX = [00F8BBE8]; ref* out,
    // const NativeString*), RET 8; EAX = out.
    virtual void** sound_sample_cache_acquire_00a83fd0(void* cache, void** out, const NativeStringImage* name) = 0;
    // 004F7180 / 004F71A0 BSP_FrontEndScreen_Construct / _Destruct: __thiscall(ECX = screen), RET.
    virtual void front_end_screen_construct_004f7180(void* screen) = 0;
    virtual void front_end_screen_destruct_004f71a0(void* screen) = 0;
    // 004C12B0 BSP_GuiManager_GetOrCreate: no arguments; EAX.
    // 00AA31F0 BSP_GuiManager_RemoveAndDestroyPage: __thiscall(ECX = manager; page), RET 4.
    virtual void* gui_manager_get_or_create_004c12b0() = 0;
    virtual void gui_manager_remove_and_destroy_page_00aa31f0(void* manager, void* page) = 0;
    // 00A97260 BSP_TextListenerBase_Construct, 00683610 BSP_GuiScroller_Construct:
    // __thiscall(ECX = member), RET.
    virtual void text_listener_base_construct_00a97260(void* listener) = 0;
    virtual void gui_scroller_construct_00683610(void* scroller) = 0;
};

// 0054D550 __thiscall(ECX = screen), RET: HUD hints screen body destructor.
void hud_hints_screen_destruct_0054d550(void* screen, Pool18Host& host);
// 0054D680 __thiscall(ECX = screen), RET; EAX = screen.
void* hud_hints_screen_construct_0054d680(void* screen, Pool18Host& host);
// 005902E0 __thiscall(ECX = screen), RET; EAX = screen.
void* main_menu_screen_construct_005902e0(void* screen, Pool18Host& host);
// 005C81F0 __thiscall(ECX = record; const record*), RET 4; EAX = record.
void* mission_record_copy_construct_005c81f0(void* record, const void* source, Pool18Host& host);
// 005C89B0 __thiscall(ECX = record), RET; EAX = record.
void* mission_record_construct_005c89b0(void* record, Pool18Host& host);

}  // namespace bsp::pool_pool18_cc13

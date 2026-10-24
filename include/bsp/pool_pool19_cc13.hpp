#pragma once

#include <cstdint>

// cc13 pool19 (small segments): the mission record destructor 005C8AE0, the
// multiplayer error screen constructor 005D2F90 and the HUD powerup carousel
// label refresh 00618980. Evidence, ABIs, EH states and uncertainty are in
// src/pool_pool19_cc13.cpp. Names are hypotheses, not recovered symbols.
//
// These are new C++ interfaces over host memory, not drop-in binary
// replacements. Every image routine the bodies call is reached through
// Pool19Host, each virtual named by the address it stands for; nothing is
// stubbed. Build-tested only; nothing here is wired into a host. No routine
// here draws from the shared generator 00BD2F10.

namespace bsp::cc13_pool19 {

// 00CEC380 (double 960.0) and 00CED090 (double 0.013000000268220901, which
// is 0.013f exactly), the carousel label constants.
inline constexpr double kLabelWidthDivisor_00cec380 = 960.0;
inline constexpr float kLabelPad_00ced090 = 0.013f;

// The error-screen button label keys pushed by 005D2F90, in order.
inline constexpr const char* kMultiErrorButtonKeys[5] = {
    "globals.yes", "globals.no", "globals.accept", "globals.marketplace_pc", "globals.back"};
inline constexpr const char* kMultiErrorDialogKeys[5] = {
    "globals.dialog_yes", "globals.dialog_no", "globals.dialog_accept",
    "globals.dialog_marketplace_pc", "globals.dialog_back"};

struct Pool19Host {
    virtual ~Pool19Host() = default;

    // ---- library bindings (no reconstruction credit) ----
    // 00BF7CD1 `eh vector constructor iterator'(first, size, count, ctor, dtor), __stdcall.
    virtual void eh_vector_construct_00bf7cd1(void* first, std::uint32_t size, std::uint32_t count,
        std::uint32_t constructor, std::uint32_t destructor) = 0;
    // 00BF7C6E `eh vector destructor iterator'(first, size, count, dtor), __stdcall.
    virtual void eh_vector_destruct_00bf7c6e(void* first, std::uint32_t size, std::uint32_t count,
        std::uint32_t destructor) = 0;
    virtual void memcpy_00bf7680(void* dst, const void* src, std::uint32_t bytes) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    // 00BF6713, cdecl, no arguments: the checked-iterator invalid-parameter
    // report. The image continues after it returns.
    virtual void invalid_parameter_00bf6713() = 0;
    virtual std::int32_t interlocked_decrement_00ce2220(void* counter) = 0;
    // 00576740 std::_Tree erase(first, last): __thiscall(ECX = tree; out,
    // {tree, first}, {tree, last}) RET 14h, returns out.
    virtual void* tree_erase_range_00576740(void* tree, void* out, void* first_container, void* first_node,
        void* last_container, void* last_node) = 0;
    // 00617870 case-insensitive string map operator[]: __thiscall(ECX = map;
    // const NativeString* key) RET 4, returns the value (node + 14h).
    virtual void* string_map_ci_subscript_00617870(void* map, const void* key) = 0;
    // 00450540 checked NativeString vector push_back: __thiscall(ECX = vector;
    // const NativeString*) RET 4.
    virtual void native_string_vector_append_00450540(void* vector, const void* value) = 0;
    // 00432050: ECX = first, EDX = last, two unused stack arguments, RET 8.
    virtual void native_string_range_destroy_00432050(void* first, void* last, void* vector, void* owner) = 0;

    // ---- NativeString {uint length +0, char* data +4} ----
    // 0041DD40 Resize(ECX = header; length, preserve) RET 8.
    virtual void native_string_resize_0041dd40(void* header, std::uint32_t length, std::uint32_t preserve) = 0;
    // A release is 00419CC0 (no arguments, EAX = pool) then 00BD1510
    // ReturnBlock(ECX = pool; data, length + 1, 1) RET 0Ch, skipped when data is null.
    virtual void* sized_pool_00419cc0() = 0;
    virtual void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t size,
        std::uint32_t flag) = 0;

    // ---- virtual dispatch through an object's own vtable (ECX = object) ----
    virtual void virtual_call_0(void* object, std::uint32_t slot_offset) = 0;
    virtual void virtual_call_1(void* object, std::uint32_t slot_offset, std::uint32_t arg) = 0;
    // vtable[88h](std::uint32_t id, 0, float 1.0f): the icon image setter.
    virtual void virtual_call_icon_set_88(void* object, std::uint32_t id, std::uint32_t zero, float scale) = 0;

    // ---- game code, called not rebuilt here ----
    // 004F7180 BSP_FrontEndScreen_Construct, __thiscall(ECX = screen), RET.
    virtual void front_end_screen_construct_004f7180(void* screen) = 0;
    // 00ABAED0 BSP_GuiText_SetLocalisedSource, __thiscall(ECX = text; source, flag) RET 8.
    virtual void gui_text_set_localised_source_00abaed0(void* text, const void* source, std::uint32_t flag) = 0;
    // 00AA6740 BSP_GuiWidget_GetSize, __thiscall(ECX = widget) -> float[2] (widget + 20h).
    virtual const float* gui_widget_get_size_00aa6740(void* widget) = 0;

    // ---- globals, read as dwords at their image address ----
    virtual std::uint32_t& global_dword(std::uint32_t address) = 0;
};

// 005C8AE0, __thiscall(ECX = record), plain RET.
void mission_record_destroy_005c8ae0(void* record, Pool19Host& host);
// 005D2F90, __thiscall(ECX = screen), EBP frame aligned to 8, plain RET, EAX = screen.
void* multi_error_screen_construct_005d2f90(void* screen, Pool19Host& host);
// 00618980, __thiscall(ECX = carousel), plain RET.
void hud_powerup_carousel_refresh_selected_label_00618980(void* carousel, Pool19Host& host);

}  // namespace bsp::cc13_pool19

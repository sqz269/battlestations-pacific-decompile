// Pool packet pool17 (small segments). See include/bsp/pool_pool17_cc13.hpp for the model. Each body
// follows the disk listing (exports/bsp/functions/<addr>/assembly.txt) in instruction order; field
// offsets are raw image offsets. Names are hypotheses, not recovered symbols. Marker: pool_pool17_cc13.

#include "bsp/pool_pool17_cc13.hpp"

#include <cstring>

namespace bsp::pool_pool17_cc13 {

using cc13_seg33a::add;
using cc13_seg33a::at;
using cc13_seg33a::ptr;

namespace {

// Image routines passed by address to the eh vector constructor iterator.
constexpr std::uint32_t kSceneRecordPlayerSlotConstruct = 0x004C9800;
constexpr std::uint32_t kSceneRecordPlayerSlotDestroy = 0x004C9820;
constexpr std::uint32_t kNativeStringHeaderInitializeEmpty = 0x00415270;
constexpr std::uint32_t kNativeStringDestroyStorage = 0x0041DD20;

std::uint32_t float_bits(float value) noexcept {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

// The 8-byte NativeString the image builds on the stack ({length +0, data +4}).
struct TempNativeString {
    std::uint32_t length;
    char* data;
};

// The inlined "build a temporary NativeString from a literal, hand it to a manager, release it"
// sequence 005098B0 repeats five times: Resize(&tmp, n, 1); memcpy(data, literal, length + 1) when
// data is non-null; call; return length + 1 bytes to the sized storage pool when data is non-null.
// The EH state of the call (14h..18h) unwinds the same release.
template <class Call>
void with_temp_string(Pool17Host& host, const char* literal, std::uint32_t n, Call&& call) {
    TempNativeString tmp{0, nullptr};
    host.native_string_resize_0041dd40(&tmp, n, 1);
    if (tmp.data != nullptr) {
        host.memcpy_00bf7680(tmp.data, literal, tmp.length + 1);
    }
    call(static_cast<void*>(&tmp));
    if (tmp.data != nullptr) {
        void* pool = host.sized_storage_pool_get_singleton_00419cc0();
        host.sized_storage_pool_return_block_00bd1510(pool, tmp.data, tmp.length + 1, 1);
    }
}

}  // namespace

// 004B0312 SoldierRegistryNode_AllocationCatch_004B0312 (7 instructions).
// Evidence: disk listing read whole. MOV EDX,[EBP-14h]; PUSH EDX; CALL 00BF65AC (free); ADD ESP,4;
// PUSH 0; PUSH 0; CALL 00BF6885 (_CxxThrowException(0, 0), the rethrow). It is the catch(...)
// funclet of 004B02A0 allocate_native_soldier_registry_node_004b02a0, whose credited body in
// src/native_soldier_registry_tree.cpp already expresses it as `catch (...) { free(node); throw; }`;
// this is the funclet as a separate function for the record, not a second implementation.
// ABI: EH catch funclet, runs on the parent frame (EBP = 004B02A0's frame); no return.
// EH: none of its own. Uncertainty: [EBP-14h] is the node pointer only by the parent's store order.
[[noreturn]] void soldier_registry_node_allocation_catch_004b0312(Pool17Host& host, void* node_at_ebp_minus_14) {
    host.free_00bf65ac(node_at_ebp_minus_14);
    host.rethrow_00bf6885();
}

// 004BA0D0 BSP_PointerSlot_ConstructNull_004ba0d0 (3 instructions).
// Evidence: disk listing read whole. MOV EAX,ECX; MOV dword [EAX],0; RET.
// ABI: __thiscall(ECX = slot), EAX = slot. EH: none. Uncertainty: none for the body; the "pointer
// slot" reading is a hypothesis (no callers recorded; reached through data references).
void* pointer_slot_construct_null_004ba0d0(void* slot) noexcept {
    at<std::uint32_t>(slot, 0) = 0;
    return slot;
}

// 004DA2A0 BSP_SceneRecord_Construct (182 instructions). Caller 004E1D70 BSP_Game_BuildSceneRecord.
// Evidence: disk listing read whole. Member layout agrees with the credited destructor 004D74E0
// (scene_record_destruct_004d74e0 in src/cg_rebuild_c_cc13.cpp).
// ABI: __thiscall(ECX = this), EAX = this, RET. SEH frame with handler 00C66801.
// EH states (store order): 0 after the player-slot array; 3 after the +928h string array; 5 after
// the environment block; 0Dh after the +C78h tree; 0Fh after +D5Ch; 14h after the cloud colours;
// 15h / 16h around the two operator new(0A0h) calls (back to 14h after each).
// Uncertainty: member names are hypotheses; the 0x3F1C61AB pair at +100C/+1010 is loaded from
// 00CE74F4 (about 0.611) and stored as bits; the four floats of the function-static at 00E18AF4 are
// 1.0f by the first-use initializer and modelled through global_dword.
void* scene_record_construct_004da2a0(Pool17Host& host, void* record) {
    at<std::uint32_t>(record, 0x0) = 0x00CE7950;  // vtable
    host.eh_vector_constructor_iterator_00bf7cd1(add(record, 0x4), 0x120, 8, kSceneRecordPlayerSlotConstruct,
                                                kSceneRecordPlayerSlotDestroy);
    for (std::size_t off : {0x90Cu, 0x910u, 0x918u, 0x91Cu, 0x920u, 0x924u}) at<std::uint32_t>(record, off) = 0;
    host.eh_vector_constructor_iterator_00bf7cd1(add(record, 0x928), 8, 0x0B, kNativeStringHeaderInitializeEmpty,
                                                kNativeStringDestroyStorage);
    at<std::uint32_t>(record, 0x980) = 0;
    at<std::uint32_t>(record, 0x984) = 0;
    host.scene_environment_block_construct_default_004cb420(add(record, 0x990));
    for (std::size_t off = 0xC24; off <= 0xC40; off += 4) at<std::uint32_t>(record, off) = 0;
    at<std::uint32_t>(record, 0xC54) = 0;  // NativeString "sky" name: length
    at<std::uint32_t>(record, 0xC58) = 0;  //   data
    for (std::size_t off : {0xC64u, 0xC68u, 0xC6Cu, 0xC70u, 0xC74u}) at<std::uint32_t>(record, off) = 0;

    // +C78h: tree header {alloc +0, head +4, size +8}; the head is its own sentinel (byte +15h = 1).
    void* tree = add(record, 0xC78);
    void* head = host.raw_tree_node_allocate18_004c26b0(tree);
    at<void*>(tree, 0x4) = head;
    at<std::uint8_t>(head, 0x15) = 1;
    at<void*>(head, 0x4) = head;
    at<void*>(head, 0x0) = head;
    at<void*>(head, 0x8) = head;
    at<std::uint32_t>(tree, 0x8) = 0;

    at<std::uint32_t>(record, 0xD50) = 0;
    at<std::uint32_t>(record, 0xD54) = 0;
    at<std::uint32_t>(record, 0xD58) = 0;
    host.array_ctor_helper_004ccb00(add(record, 0xD5C));
    host.lighting_configuration_construct_default_004b6800(add(record, 0xF48));
    at<std::uint32_t>(record, 0x100C) = 0x3F1C61AB;  // [00CE74F4]
    at<std::uint32_t>(record, 0x1010) = 0x3F1C61AB;
    host.cloud_light_colours_construct_default_004b6980(add(record, 0x1014));
    for (std::size_t off : {0x1044u, 0x1048u, 0x104Cu, 0x1050u, 0x1058u, 0x105Cu, 0x1060u}) {
        at<std::uint32_t>(record, off) = 0;
    }
    for (std::size_t off = 0x1068; off <= 0x1094; off += 4) at<std::uint32_t>(record, off) = 0;

    at<std::uint8_t>(record, 0x914) = 1;
    at<std::uint32_t>(record, 0x988) = 0;

    // "sky_001" (00CE7AB0) into the +C54h NativeString.
    void* sky = add(record, 0xC54);
    host.native_string_resize_0041dd40(sky, 7, 0);
    if (ptr(record, 0xC58) != nullptr) {
        host.memcpy_00bf7680(ptr(record, 0xC58), "sky_001", at<std::uint32_t>(record, 0xC54));
    }

    at<std::uint32_t>(record, 0x908) = 3;
    at<std::uint32_t>(record, 0x98C) = 0;
    host.memset_00bf79f0(add(record, 0x4), 0, 0x900);

    void* layer_a = host.operator_new_00bf681b(0xA0);  // state 15h
    at<void*>(record, 0xC18) =
        layer_a != nullptr ? host.scene_record_texture_layer_construct_a_004c62a0(layer_a) : nullptr;
    void* layer_b = host.operator_new_00bf681b(0xA0);  // state 16h
    at<void*>(record, 0xC1C) =
        layer_b != nullptr ? host.scene_record_texture_layer_construct_b_004c63e0(layer_b) : nullptr;

    at<std::uint32_t>(record, 0xC5C) = float_bits(0.0f);
    at<std::uint32_t>(record, 0xC60) = float_bits(1.0f);  // [00D7A24C]
    at<std::uint8_t>(record, 0x904) = 1;
    at<std::uint8_t>(record, 0x905) = 0;

    // Function-static vec4 at 00E18AF4 guarded by bit 0 (mask 1) of 00E18B04.
    std::uint32_t& guard = host.global_dword(0x00E18B04);
    if ((guard & 1u) == 0) {
        guard |= 1u;
        for (std::uint32_t i = 0; i < 4; ++i) host.global_dword(0x00E18AF4 + 4 * i) = float_bits(1.0f);
    }
    for (std::uint32_t i = 0; i < 4; ++i) {
        at<std::uint32_t>(record, 0xC44 + 4 * i) = host.global_dword(0x00E18AF4 + 4 * i);
    }

    at<std::uint8_t>(record, 0xC20) = 0;
    at<std::int32_t>(record, 0x1098) = -1;
    return record;
}

// 005098B0 BSP_TacticalLibraryScreen_Construct (294 instructions). Caller 00686380 BSP_MainMenu_Init;
// the screen table in include/bsp/main_menu_screens.hpp lists it with vtable 00CEB9B4 and size 5D0h.
// Destructor 0050AF20 is reconstructed in src/frontend_seg12_17_cc13.cpp.
// Evidence: disk listing read whole.
// ABI: __thiscall(ECX = this), EAX = this, RET. SEH frame with handler 00C69812.
// EH states: 0 after the FrontEndScreen base; 2 after the 3D scene and its two interface vptrs; 3 / 4
// after the +318h / +340h string arrays; 13h after the field clears; 14h..18h around the five
// temporary-string calls (each back to 13h after its release).
// Uncertainty: 00B1BD20 is recorded as BSP_LuaGlobals_SetFieldsNamedByStringEntries (PROVISIONAL);
// the vptr values at +64h / +68h are first set to the base pair (00CEB0FC, 00CEB110) and then
// overwritten with the derived set, as the listing does.
void* tactical_library_screen_construct_005098b0(Pool17Host& host, void* screen) {
    host.frontend_screen_construct_004f7180(screen);
    host.simple3d_scene_construct_0093cb10(add(screen, 0x8));
    at<std::uint32_t>(screen, 0x64) = 0x00CEB0FC;
    at<std::uint32_t>(screen, 0x68) = 0x00CEB110;
    at<std::uint32_t>(screen, 0x0) = 0x00CEB9B4;
    at<std::uint32_t>(screen, 0x8) = 0x00CEB9B0;
    at<std::uint32_t>(screen, 0x64) = 0x00CEB99C;
    at<std::uint32_t>(screen, 0x68) = 0x00CEB97C;
    host.gui_scroller_construct_00683610(add(screen, 0xCC));
    host.eh_vector_constructor_iterator_00bf7cd1(add(screen, 0x318), 8, 5, kNativeStringHeaderInitializeEmpty,
                                                kNativeStringDestroyStorage);
    host.eh_vector_constructor_iterator_00bf7cd1(add(screen, 0x340), 8, 3, kNativeStringHeaderInitializeEmpty,
                                                kNativeStringDestroyStorage);
    host.eh_vector_constructor_iterator_00bf7cd1(add(screen, 0x358), 8, 0x0D, kNativeStringHeaderInitializeEmpty,
                                                kNativeStringDestroyStorage);

    for (std::size_t off : {0x3C8u, 0x3CCu, 0x3D0u, 0x3D8u, 0x3DCu, 0x3E0u, 0x3E8u, 0x3ECu, 0x3F0u, 0x3F8u,
                            0x3FCu, 0x400u, 0x408u, 0x40Cu, 0x410u, 0x418u, 0x41Cu, 0x420u, 0x428u, 0x42Cu,
                            0x430u, 0x448u, 0x44Cu, 0x450u}) {
        at<std::uint32_t>(screen, off) = 0;
    }
    at<std::uint32_t>(screen, 0x438) = float_bits(0.0f);
    at<std::uint32_t>(screen, 0x434) = 0;
    for (std::size_t off : {0x460u, 0x464u, 0x468u, 0x470u, 0x474u, 0x478u, 0x480u, 0x484u, 0x488u, 0x494u,
                            0x498u, 0x49Cu, 0x4A4u, 0x4A8u, 0x4ACu, 0x5C4u, 0x5C8u, 0x5CCu}) {
        at<std::uint32_t>(screen, off) = 0;
    }
    at<std::uint32_t>(screen, 0x4BC) = 0;
    at<std::uint8_t>(screen, 0x4CC) = 0;
    for (std::size_t off : {0x508u, 0x50Cu, 0x510u, 0x5A0u, 0x5A4u, 0x5A8u, 0x51Cu, 0x520u, 0x524u, 0x3C0u,
                            0x90u, 0x4B8u}) {
        at<std::uint32_t>(screen, off) = 0;
    }
    at<std::uint32_t>(screen, 0x8C) = float_bits(0.0f);
    at<std::int32_t>(screen, 0x4B4) = -1;
    at<std::uint32_t>(screen, 0x70) = 0;
    at<std::uint32_t>(screen, 0x74) = 0;
    at<std::uint32_t>(screen, 0x78) = 0;
    at<std::uint32_t>(screen, 0x6C) = 0;

    // "white.tga" (00CE77A4) to the manager at [00F8D434] (state 14h).
    with_temp_string(host, "white.tga", 9, [&](void* name) {
        host.lua_globals_set_fields_named_by_string_entries_00b1bd20(
            reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_dword(0x00F8D434))), name);
    });

    at<std::uint32_t>(screen, 0xC4) = 0;
    for (std::size_t off : {0x88u, 0x84u, 0x80u, 0x7Cu}) at<std::uint32_t>(screen, off) = float_bits(100.0f);  // [00CE3D08]

    // "ColourRemap.tga" (00CEB588) into the four remap texture slots of [00F8D39C] (states 15h..18h).
    auto render_resources = [&]() {
        return reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_dword(0x00F8D39C)));
    };
    with_temp_string(host, "ColourRemap.tga", 0x0F,
                     [&](void* name) { host.render_resources_set_remap_texture0_00b0fd70(render_resources(), name); });
    with_temp_string(host, "ColourRemap.tga", 0x0F,
                     [&](void* name) { host.render_resources_set_remap_texture1_00b0fdc0(render_resources(), name); });
    with_temp_string(host, "ColourRemap.tga", 0x0F,
                     [&](void* name) { host.render_resources_set_remap_texture2_00b0fe10(render_resources(), name); });
    with_temp_string(host, "ColourRemap.tga", 0x0F,
                     [&](void* name) { host.render_resources_set_remap_texture3_00b0fe60(render_resources(), name); });
    return screen;
}

// 00530F40 BSP_MenuPromptRecord_Construct (21 instructions).
// Evidence: disk listing read whole. Stores 0 to +0, +4, +10h, +14h, +20h, +24h, +2Ch, +38h, +3Ch,
// +8, +0Ch; +28h = 1; byte +18h = 0; +30h = 0.0f (XORPS / MOVSS); byte +40h = 1 (DL); +44h = 7.
// +34h is not written. Mirrors FrontEndPromptRecord in include/bsp/frontend_prompts.hpp, which keeps
// its own member initializers; nothing there is switched to this body.
// ABI: __thiscall(ECX = this), EAX = this, RET. EH: none. Uncertainty: field meanings are hypotheses.
void* menu_prompt_record_construct_00530f40(void* record) noexcept {
    for (std::size_t off : {0x0u, 0x4u, 0x10u, 0x14u, 0x20u, 0x24u, 0x2Cu, 0x38u, 0x3Cu, 0x8u, 0xCu}) {
        at<std::uint32_t>(record, off) = 0;
    }
    at<std::uint32_t>(record, 0x28) = 1;
    at<std::uint8_t>(record, 0x18) = 0;
    at<std::uint32_t>(record, 0x30) = float_bits(0.0f);
    at<std::uint8_t>(record, 0x40) = 1;
    at<std::uint32_t>(record, 0x44) = 7;
    return record;
}

}  // namespace bsp::pool_pool17_cc13

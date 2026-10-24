// Pool packet pool17 (small segments): the soldier-registry node allocation catch funclet, the
// null pointer-slot constructor, the scene record and tactical library screen constructors, and the
// front-end prompt record constructor. Bodies left in the progress-board pool after the oct24
// credit audit (or never recorded).
//
// Model as in cc13_seg33a: raw image offsets over a Pool17Host whose methods name the image routine
// they stand for. CRT/EH helpers (operator new, memset, memcpy, free, the eh vector constructor
// iterator, the rethrow) are library code bound as host calls. New C++ interfaces, not drop-in
// binary replacements; nothing here is bound into bsp_game. Names are hypotheses, not recovered
// symbols. Marker: pool_pool17_cc13.

#ifndef BSP_POOL_POOL17_CC13_HPP
#define BSP_POOL_POOL17_CC13_HPP

#include <cstddef>
#include <cstdint>

#include "bsp/cc13_seg33a.hpp"

namespace bsp::pool_pool17_cc13 {

struct Pool17Host {
    virtual ~Pool17Host() = default;

    // A dword global of the image (the scene record's function-static vec4 00E18AF4..00E18B04, the
    // manager pointers 00F8D434 / 00F8D39C).
    virtual std::uint32_t& global_dword(std::uint32_t address) = 0;

    // CRT / EH (bound). 00BF65AC free (cdecl); 00BF6885 _CxxThrowException(0, 0) = rethrow, no
    // return; 00BF681B operator new (cdecl); 00BF79F0 memset; 00BF7680 memcpy;
    // 00BF7CD1 `eh vector constructor iterator'(array, size, count, ctor, dtor), stdcall.
    virtual void free_00bf65ac(void* block) = 0;
    [[noreturn]] virtual void rethrow_00bf6885() = 0;
    virtual void* operator_new_00bf681b(std::size_t size) = 0;
    virtual void memset_00bf79f0(void* destination, int value, std::size_t size) = 0;
    virtual void memcpy_00bf7680(void* destination, const void* source, std::size_t size) = 0;
    virtual void eh_vector_constructor_iterator_00bf7cd1(void* array, std::size_t size, std::size_t count,
                                                         std::uint32_t ctor, std::uint32_t dtor) = 0;

    // 0041DD40 BSP_NativeString_Resize, __thiscall(ECX = string, length, keep), RET 8.
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, std::uint32_t keep) = 0;
    // 00419CC0 BSP_SizedStoragePool_GetSingleton(), EAX = pool; 00BD1510 ReturnBlock,
    // __thiscall(ECX = pool, data, bytes, 1), RET 0Ch.
    virtual void* sized_storage_pool_get_singleton_00419cc0() = 0;
    virtual void sized_storage_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t bytes,
                                                          std::uint32_t count) = 0;

    // Scene record members (__fastcall(ECX = member), RET unless noted).
    virtual void scene_environment_block_construct_default_004cb420(void* block) = 0;
    virtual void* raw_tree_node_allocate18_004c26b0(void* tree) = 0;  // EAX = head node
    virtual void array_ctor_helper_004ccb00(void* block) = 0;         // CG_array_ctor_helper_004ccb00
    virtual void lighting_configuration_construct_default_004b6800(void* block) = 0;
    virtual void cloud_light_colours_construct_default_004b6980(void* block) = 0;
    virtual void* scene_record_texture_layer_construct_a_004c62a0(void* layer) = 0;  // EAX = layer
    virtual void* scene_record_texture_layer_construct_b_004c63e0(void* layer) = 0;  // EAX = layer

    // Tactical library screen members.
    virtual void frontend_screen_construct_004f7180(void* screen) = 0;
    virtual void simple3d_scene_construct_0093cb10(void* scene) = 0;
    virtual void gui_scroller_construct_00683610(void* scroller) = 0;
    // __thiscall(ECX = manager, const NativeString* name), RET 4; return value unused here.
    virtual void lua_globals_set_fields_named_by_string_entries_00b1bd20(void* manager, void* name) = 0;
    virtual void render_resources_set_remap_texture0_00b0fd70(void* manager, void* name) = 0;
    virtual void render_resources_set_remap_texture1_00b0fdc0(void* manager, void* name) = 0;
    virtual void render_resources_set_remap_texture2_00b0fe10(void* manager, void* name) = 0;
    virtual void render_resources_set_remap_texture3_00b0fe60(void* manager, void* name) = 0;
};

// The catch(...) funclet of 004B02A0 (allocate_native_soldier_registry_node_004b02a0): frees the
// node held in the parent frame's [EBP-14h] and rethrows.
[[noreturn]] void soldier_registry_node_allocation_catch_004b0312(Pool17Host& host, void* node_at_ebp_minus_14);
void* pointer_slot_construct_null_004ba0d0(void* slot) noexcept;
void* scene_record_construct_004da2a0(Pool17Host& host, void* record);
void* tactical_library_screen_construct_005098b0(Pool17Host& host, void* screen);
void* menu_prompt_record_construct_00530f40(void* record) noexcept;

}  // namespace bsp::pool_pool17_cc13

#endif

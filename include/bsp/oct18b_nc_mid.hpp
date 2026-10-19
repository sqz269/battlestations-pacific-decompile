#pragma once

#include <cstdint>
#include <functional>

#include "bsp/cc13_seg68.hpp"
#include "bsp/cc13_seg70.hpp"

// Additions to the existing non-host families, isolated because their principal
// files have other active owners. Names are hypotheses. Original Win32 offsets
// and calling conventions are evidence, not a binary-compatibility claim.
namespace bsp::cc13_seg68 {
// 009F6C10 / 009F7070: AL = 1 / 0, plain RET; no inputs read.
bool filter_enabled_009f6c10() noexcept;
bool filter_disabled_009f7070() noexcept;
}

namespace bsp::cc13_seg69 {
// 00A00800: __cdecl(const uint32_t*), plain RET, EAX = first word.
std::uint32_t first_word_callback_00a00800(const std::uint32_t* value) noexcept;
// 00A04C70: __thiscall(cache owner; index), RET 4, EAX = 0/1. The
// 00437BD0 hook receives owner+4, grows to mapped_index+1, and may move data.
bool mapped_cache_entry_present_00a04c70(void* owner, std::int32_t index,
    const std::function<void(void*, std::int32_t)>& resize_cache_00437bd0);
}

namespace bsp::cc13_seg70 {
// Native bodies are tail JMPs through +24h / +4h. No incoming register or
// stack arguments are read or changed. The continuation owns the entire native
// invocation frame; these new C++ interfaces only identify object and slot.
using TailDispatch = std::function<void(void*, std::uint32_t)>;
void planner_forward_slot24_00a18570(void* planner, const TailDispatch& continuation);
void group_forward_slot04_00a2bd40(void* group, const TailDispatch& continuation);
// 00A2DA60: __thiscall(ECX = group+10h; entity), RET 4. Search members
// first, then remove, unregister the observer, and queue a newly empty group.
// Like this family's existing list walkers, assumes a valid native list.
void group_observer_remove_entity_00a2da60(Seg70Host& host, Raw observer, void* entity);
// 00A31D60: __thiscall(singleton; byte flags), RET 4 -> original pointer.
// Clears 00F8AAFC, installs 00CE3818, and calls free when flags bit 0 is set.
void* coordinator_singleton_scalar_delete_00a31d60(Seg70Host& host, Raw singleton,
    std::uint8_t flags);
}

namespace bsp::cc13_gui1 {
struct ListboxPageHooks {
    virtual ~ListboxPageHooks() = default;
    virtual void rebuild_page_00a9d870(void* listbox) = 0;
    virtual void select_index_00a9c7c0(void* listbox, std::int32_t index) = 0;
    virtual std::int32_t selected_index_00a9c920(void* listbox) = 0;
};
// 00A9DE50: __thiscall(listbox; absolute index, byte center), RET 8.
void listbox_reveal_index_00a9de50(ListboxPageHooks& host, void* listbox,
    std::int32_t index, std::uint8_t center);
// 00AA1CF0: ECX = GUI object; AX = word at +ECh, plain RET.
std::uint16_t gui_field_ec_00aa1cf0(const void* object) noexcept;
struct ListboxCreateHooks {
    virtual ~ListboxCreateHooks() = default;
    // 00A9B7E0 is currently named CG_static_dtor_stub in Ghidra; the caller
    // actually supplies ECX=17Ch and uses its result as constructor storage.
    virtual void* allocate_00a9b7e0(std::uint32_t bytes) = 0;
    virtual void* construct_00a9df40(void* storage) = 0;
    virtual void* copy_construct_00aa5b40(void* storage, const void* source) = 0;
    // EHHandler_00CB7350 -> unwind map 00DEDB84: both states call 00A9B330
    // with ECX = saved constructor storage (pool deallocation).
    virtual void deallocate_00a9b330(void* storage) = 0;
};
// 00AA64B0: ECX = nullable copy source, plain RET -> listbox or null.
void* create_listbox_00aa64b0(ListboxCreateHooks& host, const void* source);
// 00AB0FE0: __thiscall(float[4]; const float[4]), RET 4, EAX = 0/1.
// Ordered equality of all components: any NaN is unequal, signed zero equal.
bool gui_color_equal_00ab0fe0(const float* first, const float* second) noexcept;
// 00AB6A70: __thiscall(text; byte flag), tail JMP to slot +80h with
// the original argument replaced by 3 when nonzero, else 0 (callee RET 4).
void gui_text_set_flag77_00ab6a70(void* text, std::uint8_t flag,
    const std::function<void(void*, std::int32_t)>& slot80);
}

namespace bsp::cc13_terrain {
// 00AD56E0: FLDZ / RET 8, ECX and two stack words unused, ST0 = +0.
float terrain_zero_sample_00ad56e0(std::uint32_t, std::uint32_t) noexcept;
void terrain_empty_slot_00ad9cb0() noexcept; // plain RET
// 00AD9CC0 / 00AD9CD0: ECX = terrain render object; EAX = +4h / +10h.
std::uint32_t terrain_object_word04_00ad9cc0(const void* object) noexcept;
std::uint32_t terrain_object_word10_00ad9cd0(const void* object) noexcept;
// 00ADC080: __thiscall(owner; pointer), RET 4. Unique insert into the
// pointer array at +44h {data,count,capacity}; reserve hook may move data.
void terrain_add_unique_pointer_00adc080(void* object, void* value,
    const std::function<void(void*, std::int32_t)>& reserve_00adb050);
void terrain_empty_three_args_00adc4b0(std::uint32_t, std::uint32_t, std::uint32_t) noexcept;
// Each below is FLD dword [00CE4ADC / 00CE4970] / RET, result in ST0.
float terrain_min_height_00adc520() noexcept;
float terrain_max_height_00adc530() noexcept;
float terrain_min_height_00adfb00() noexcept;
float terrain_max_height_00adfb10() noexcept;
float terrain_max_height_00aedb50() noexcept;
// 00ADC580 / 00ADFA90: __thiscall(block; float height, int i, int j),
// RET 0Ch. height <= double -1000 -> all-ones sentinel; otherwise truncate
// x87 (height-offset_2c)*multiplier_30 toward zero, retain its low 16/8 bits.
// Native FISTP indefinite/out-of-range behavior is retained on MSVC Win32.
void terrain_u16_block_set_00adc580(void* block, float height, std::int32_t i, std::int32_t j);
void terrain_u8_block_set_00adfa90(void* block, float height, std::int32_t i, std::int32_t j);
// 00ADF090: __thiscall(tile; int x, int z), RET 8. Writes +Ch/+10h,
// builds scale/translation from [[tile+8]+18h], then node+24h slot38(matrix).
void terrain_tile_set_position_00adf090(void* tile, std::int32_t x, std::int32_t z,
    const std::function<void(void*, const float*)>& node_slot38);
void terrain_empty_three_args_00aed1a0(std::uint32_t, std::uint32_t, std::uint32_t) noexcept;
void terrain_empty_three_args_00aed1c0(std::uint32_t, std::uint32_t, std::uint32_t) noexcept;
void terrain_empty_three_args_00aed200(std::uint32_t, std::uint32_t, std::uint32_t) noexcept;
// 00AED6F0 / 00AED8E0 / 00AED910: __thiscall(block; value/out, i, j),
// RET 0Ch. Float grid at +2Ch/+34h; float3 grid at +18h/+20h. No checks.
void terrain_float_layer_set_00aed6f0(void* block, float value, std::int32_t i, std::int32_t j);
void terrain_vector_layer_set_00aed8e0(void* block, const float* value, std::int32_t i, std::int32_t j);
void terrain_vector_layer_get_00aed910(const void* block, float* out, std::int32_t i, std::int32_t j);
void terrain_empty_one_arg_00aeda70(std::uint32_t) noexcept; // RET 4
// 00AF37A0: ECX = source float3, plain RET. Three sequential MOVSS copies
// into globals 00E13028/2C/30. Semantic identity of that global is provisional.
void terrain_store_global_vector_00af37a0(const float* source, float* global_e13028) noexcept;
}

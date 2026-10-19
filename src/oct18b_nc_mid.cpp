#include "bsp/oct18b_nc_mid.hpp"

#include "bsp/camera_projection.hpp"

#include <cmath>
#include <cstring>
#include <limits>

namespace {
template<class T> T& field(void* object, std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(object) + offset);
}
template<class T> const T& field(const void* object, std::size_t offset) {
    return *reinterpret_cast<const T*>(static_cast<const std::uint8_t*>(object) + offset);
}
std::int32_t wrapped_add(std::int32_t a, std::int32_t b) {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(a) + static_cast<std::uint32_t>(b));
}
std::int32_t wrapped_sub(std::int32_t a, std::int32_t b) {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(a) - static_cast<std::uint32_t>(b));
}
std::int32_t cell_index(std::int32_t width, std::int32_t i, std::int32_t j) {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(width) *
        static_cast<std::uint32_t>(j) + static_cast<std::uint32_t>(i));
}
// 00ADC5AA..00ADC5DA and 00ADFAB8..00ADFAE7: keep the subtraction and
// multiplication on the x87 stack; change only rounding-control bits for FISTP.
std::int32_t encode_height(float height, float offset, float multiplier) {
#if defined(_MSC_VER) && defined(_M_IX86)
    std::uint16_t previous_control, truncation_control;
    std::int32_t encoded;
    __asm {
        fld height
        fsub offset
        fnstcw previous_control
        mov ax, previous_control
        fmul multiplier
        or ax, 0c00h
        mov truncation_control, ax
        fldcw truncation_control
        fistp encoded
        fldcw previous_control
    }
    return encoded;
#else
    const long double value = (static_cast<long double>(height) - offset) * multiplier;
    if (!std::isfinite(value) || value < -2147483648.0L || value >= 2147483648.0L)
        return (std::numeric_limits<std::int32_t>::min)();
    return static_cast<std::int32_t>(value);
#endif
}
float tile_scale(float spacing) {
    // Exact little-endian bytes 00 00 00 20 10 10 C0 3F at 00D5D408.
    const std::uint64_t divisor_bits = 0x3fc0101020000000ull;
    double divisor;
    std::memcpy(&divisor, &divisor_bits, sizeof divisor);
    float result;
#if defined(_MSC_VER) && defined(_M_IX86)
    __asm {
        fld spacing
        fdiv divisor
        fstp result
    }
#else
    result = static_cast<float>(static_cast<double>(spacing) / divisor);
#endif
    return result;
}
float tile_translation(float spacing, std::int32_t coordinate) {
    float result;
#if defined(_MSC_VER) && defined(_M_IX86)
    __asm {
        fild coordinate
        fmul spacing
        fstp result
    }
#else
    result = static_cast<float>(static_cast<double>(coordinate) * spacing);
#endif
    return result;
}
void copy_float3_x87(float* out, const float* in) {
#if defined(_MSC_VER) && defined(_M_IX86)
    __asm {
        mov eax, in
        mov edx, out
        fld dword ptr [eax]
        fstp dword ptr [edx]
        fld dword ptr [eax + 4]
        fstp dword ptr [edx + 4]
        fld dword ptr [eax + 8]
        fstp dword ptr [edx + 8]
    }
#else
    out[0] = in[0]; out[1] = in[1]; out[2] = in[2];
#endif
}
}

namespace bsp::cc13_seg68 {
bool filter_enabled_009f6c10() noexcept { return filter_enabled_slot4(SquadronFilter::At0C); }
bool filter_disabled_009f7070() noexcept { return filter_enabled_slot4(SquadronFilter::At14); }
}

namespace bsp::cc13_seg69 {
std::uint32_t first_word_callback_00a00800(const std::uint32_t* value) noexcept { return *value; }
bool mapped_cache_entry_present_00a04c70(void* owner, std::int32_t index,
    const std::function<void(void*, std::int32_t)>& resize_cache) {
    const auto mapped = field<std::int32_t>(owner, 0x10 + index * 4);
    if (mapped >= field<std::int32_t>(owner, 8))
        resize_cache(static_cast<std::uint8_t*>(owner) + 4, wrapped_add(mapped, 1));
    // Reload after a possible resize. NEG/SBB/AND 00F8A0BD/NEG/SBB/NEG is
    // simply 0/1 for a nonzero cache word; it never dereferences 00F8A0BD.
    return field<std::uint32_t*>(owner, 4)[mapped] != 0;
}
}

namespace bsp::cc13_seg70 {
void planner_forward_slot24_00a18570(void* planner, const TailDispatch& continuation) {
    continuation(planner, 0x24);
}
void group_forward_slot04_00a2bd40(void* group, const TailDispatch& continuation) {
    continuation(group, 4);
}
void group_observer_remove_entity_00a2da60(Seg70Host& host, Raw observer, void* entity) {
    void* head = observer.ptr(0x5630);
    for (void* node = Raw{static_cast<std::uint8_t*>(head)}.ptr(0); node != observer.ptr(0x5630);
         node = Raw{static_cast<std::uint8_t*>(node)}.ptr(0)) {
        if (Raw{static_cast<std::uint8_t*>(node)}.ptr(8) != entity) continue;
        // 00A2DABE..00A2DB40 duplicates 00A2D9D0(entity,1), with this
        // adjusted from its observer base at group+10h to the group itself.
        group_remove_entity_00a2d9d0(host, Raw{observer.p - 0x10}, entity, 1);
        return;
    }
}
void* coordinator_singleton_scalar_delete_00a31d60(Seg70Host& host, Raw singleton,
    std::uint8_t flags) {
    host.global(kCoordinatorSingleton).at<std::uint32_t>(0) = 0;
    singleton.at<std::uint32_t>(0) = kSingletonBaseVtable;
    if ((flags & 1) != 0) host.free_00bf65ac(singleton.p);
    return singleton.p;
}
}

namespace bsp::cc13_gui1 {
void listbox_reveal_index_00a9de50(ListboxPageHooks& host, void* listbox,
    std::int32_t index, std::uint8_t center) {
    if (center != 0) {
        const auto half = wrapped_add(wrapped_add(field<std::int32_t>(listbox, 0x158), 1) / 2, -1);
        if (index <= half) {
            listbox_reveal_index_00a9de50(host, listbox, 0, 0);
            host.select_index_00a9c7c0(listbox, index);
        } else {
            listbox_reveal_index_00a9de50(host, listbox, wrapped_sub(index, half), 0);
            host.select_index_00a9c7c0(listbox, wrapped_add(host.selected_index_00a9c920(listbox), half));
        }
        return;
    }
    auto maximum = wrapped_sub(field<std::int32_t>(listbox, 0x154),
        field<std::int32_t>(listbox, 0x158));
    if (maximum < 0) maximum = 0;
    if (index > maximum) {
        field<std::int32_t>(listbox, 0x164) = maximum;
        host.rebuild_page_00a9d870(listbox);
        host.select_index_00a9c7c0(listbox, wrapped_sub(index, field<std::int32_t>(listbox, 0x164)));
    } else {
        field<std::int32_t>(listbox, 0x164) = index;
        host.rebuild_page_00a9d870(listbox);
        host.select_index_00a9c7c0(listbox, 0);
    }
}
std::uint16_t gui_field_ec_00aa1cf0(const void* object) noexcept {
    return field<std::uint16_t>(object, 0xec);
}
void* create_listbox_00aa64b0(ListboxCreateHooks& host, const void* source) {
    void* storage = host.allocate_00a9b7e0(0x17c);
    if (storage == nullptr) return nullptr;
    try {
        return source != nullptr ? host.copy_construct_00aa5b40(storage, source)
                                 : host.construct_00a9df40(storage);
    } catch (...) {
        host.deallocate_00a9b330(storage);
        throw;
    }
}
bool gui_color_equal_00ab0fe0(const float* first, const float* second) noexcept {
    return first[0] == second[0] && first[1] == second[1] &&
        first[2] == second[2] && first[3] == second[3];
}
void gui_text_set_flag77_00ab6a70(void* text, std::uint8_t flag,
    const std::function<void(void*, std::int32_t)>& slot80) {
    field<std::uint8_t>(text, 0x77) = flag;
    slot80(text, flag != 0 ? 3 : 0);
}
}

namespace bsp::cc13_terrain {
float terrain_zero_sample_00ad56e0(std::uint32_t, std::uint32_t) noexcept { return 0.0f; }
void terrain_empty_slot_00ad9cb0() noexcept {}
std::uint32_t terrain_object_word04_00ad9cc0(const void* object) noexcept { return field<std::uint32_t>(object, 4); }
std::uint32_t terrain_object_word10_00ad9cd0(const void* object) noexcept { return field<std::uint32_t>(object, 0x10); }
void terrain_add_unique_pointer_00adc080(void* object, void* value,
    const std::function<void(void*, std::int32_t)>& reserve) {
    const auto count = field<std::int32_t>(object, 0x48);
    auto* data = field<void**>(object, 0x44);
    for (std::int32_t at = 0; at < count; ++at) if (data[at] == value) return;
    const auto capacity = field<std::int32_t>(object, 0x4c);
    if (count == capacity) {
        auto doubled = wrapped_add(capacity, capacity);
        if (doubled <= 1) doubled = 1;
        reserve(static_cast<std::uint8_t*>(object) + 0x44, doubled);
    }
    // Native placement store is skipped when the computed destination is null;
    // its count increment still happens. Reload both fields after the callback.
    auto* slot = reinterpret_cast<void**>(reinterpret_cast<std::uintptr_t>(field<void**>(object, 0x44)) +
        static_cast<std::uint32_t>(field<std::int32_t>(object, 0x48)) * 4u);
    if (slot != nullptr) *slot = value;
    field<std::int32_t>(object, 0x48) = wrapped_add(field<std::int32_t>(object, 0x48), 1);
}
void terrain_empty_three_args_00adc4b0(std::uint32_t, std::uint32_t, std::uint32_t) noexcept {}
float terrain_min_height_00adc520() noexcept { return -10000000000.0f; }
float terrain_max_height_00adc530() noexcept { return 10000000000.0f; }
float terrain_min_height_00adfb00() noexcept { return terrain_min_height_00adc520(); }
float terrain_max_height_00adfb10() noexcept { return terrain_max_height_00adc530(); }
float terrain_max_height_00aedb50() noexcept { return terrain_max_height_00adc530(); }
void terrain_u16_block_set_00adc580(void* block, float height, std::int32_t i, std::int32_t j) {
    auto& cell = field<std::uint16_t*>(block, 0x40)[cell_index(field<std::int32_t>(block, 0x38), i, j)];
    cell = height <= -1000.0 ? 0xffffu : static_cast<std::uint16_t>(
        encode_height(height, field<float>(block, 0x2c), field<float>(block, 0x30)));
}
void terrain_u8_block_set_00adfa90(void* block, float height, std::int32_t i, std::int32_t j) {
    auto& cell = field<std::uint8_t*>(block, 0x40)[cell_index(field<std::int32_t>(block, 0x38), i, j)];
    cell = height <= -1000.0 ? 0xffu : static_cast<std::uint8_t>(
        encode_height(height, field<float>(block, 0x2c), field<float>(block, 0x30)));
}
void terrain_tile_set_position_00adf090(void* tile, std::int32_t x, std::int32_t z,
    const std::function<void(void*, const float*)>& node_slot38) {
    void* owner = field<void*>(tile, 8);
    field<std::int32_t>(tile, 0x10) = z;
    field<std::int32_t>(tile, 0xc) = x;
    const float spacing = field<float>(owner, 0x18);
    const float scale = tile_scale(spacing);
    CameraMatrix source{};
    source[0] = scale; source[5] = 1.0f; source[10] = scale; source[15] = 1.0f;
    CameraMatrix matrix;
    copy_camera_matrix_004134f0(matrix, source);
    // The original reloads spacing after the matrix copy and rounds each
    // FILD/FMUL translation to float before inserting it into the matrix.
    const float reloaded_spacing = field<float>(owner, 0x18);
    matrix[12] = tile_translation(reloaded_spacing, x);
    matrix[13] = 0.0f;
    matrix[14] = tile_translation(reloaded_spacing, z);
    node_slot38(field<void*>(tile, 0x24), matrix.data());
}
void terrain_empty_three_args_00aed1a0(std::uint32_t, std::uint32_t, std::uint32_t) noexcept {}
void terrain_empty_three_args_00aed1c0(std::uint32_t, std::uint32_t, std::uint32_t) noexcept {}
void terrain_empty_three_args_00aed200(std::uint32_t, std::uint32_t, std::uint32_t) noexcept {}
void terrain_float_layer_set_00aed6f0(void* block, float value, std::int32_t i, std::int32_t j) {
    // MOVSS is a bit-preserving store, including NaN payloads.
    float* cell = field<float*>(block, 0x34) + cell_index(field<std::int32_t>(block, 0x2c), i, j);
    std::memcpy(cell, &value, sizeof value);
}
void terrain_vector_layer_set_00aed8e0(void* block, const float* value, std::int32_t i, std::int32_t j) {
    copy_float3_x87(field<float*>(block, 0x20) + cell_index(field<std::int32_t>(block, 0x18), i, j) * 3, value);
}
void terrain_vector_layer_get_00aed910(const void* block, float* out, std::int32_t i, std::int32_t j) {
    copy_float3_x87(out, field<float*>(block, 0x20) + cell_index(field<std::int32_t>(block, 0x18), i, j) * 3);
}
void terrain_empty_one_arg_00aeda70(std::uint32_t) noexcept {}
void terrain_store_global_vector_00af37a0(const float* source, float* global) noexcept {
    // Keep individual load/store ordering rather than a bulk memcpy: the
    // source may overlap the destination by one component.
    for (int i = 0; i != 3; ++i) {
        std::uint32_t word;
        std::memcpy(&word, source + i, sizeof word);
        std::memcpy(global + i, &word, sizeof word);
    }
}
}

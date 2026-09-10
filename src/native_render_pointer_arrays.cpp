#include "bsp/native_render_pointer_arrays.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>

namespace bsp {
namespace {
void reserve_pointers(NativeRenderPointerArrayStorage& array, std::int32_t requested) {
    if (requested < 1) requested = 1;
    if (array.capacity_08 >= requested) return;
    auto** replacement = static_cast<void**>(singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, static_cast<std::size_t>(requested) * 4,
        static_cast<std::size_t>(requested) * sizeof(void*)}));
    for (std::int32_t i = 0; i < array.count_04; ++i)
        replacement[i] = array.data_00[i];
    singleton_lifetime_free(array.data_00);
    array.data_00 = replacement;
    array.capacity_08 = requested;
}

void resize_pointers(NativeRenderPointerArrayStorage& array, std::int32_t requested) {
    if (requested > array.capacity_08) reserve_pointers(array, requested);
    for (std::int32_t i = array.count_04; i < requested; ++i)
        array.data_00[i] = nullptr;
    while (requested < array.count_04) --array.count_04;
    array.count_04 = requested;
}

void destroy_group_pointers(NativeRenderPointerArrayStorage& array) {
    resize_pointers(array, 0);
    singleton_lifetime_free(array.data_00);
}

using ReservePointers = void (*)(NativeRenderPointerArrayStorage&, std::int32_t);

void append_pointer(NativeRenderPointerArrayStorage& array,
    const void* source_pointer_cell, ReservePointers reserve) {
    volatile auto& actual = array;
    const auto capacity = actual.capacity_08;
    if (actual.count_04 == capacity) {
        const auto doubled_bits = static_cast<std::uint32_t>(capacity) * 2u;
        std::int32_t doubled;
        std::memcpy(&doubled, &doubled_bits, sizeof(doubled));
        if (doubled <= 1) doubled = 1;
        reserve(array, doubled);
    }
    // Native LEA uses wrapping DWORD arithmetic, followed by a null test.
    const auto count = static_cast<std::uint32_t>(actual.count_04);
    const auto base = reinterpret_cast<std::uintptr_t>(actual.data_00);
    const auto destination = base + count * 4u;
    if (destination != 0) {
        void* value;
        std::memcpy(&value, source_pointer_cell, sizeof(value));
        std::memcpy(reinterpret_cast<void*>(destination), &value, sizeof(value));
    }
    const auto next_bits = static_cast<std::uint32_t>(actual.count_04) + 1u;
    std::int32_t next;
    std::memcpy(&next, &next_bits, sizeof(next));
    actual.count_04 = next;
}
} // namespace

NativeRenderPointerArrayStorage* initialize_native_instance_entry_pointers_00b1c4f0(
    NativeRenderPointerArrayStorage& array) noexcept {
    array.data_00 = nullptr;
    array.count_04 = 0;
    array.capacity_08 = 0;
    return &array;
}
void reserve_native_instance_entry_pointers_00b1c500(
    NativeRenderPointerArrayStorage& array, std::int32_t requested) {
    reserve_pointers(array, requested);
}
void resize_native_instance_entry_pointers_00b1c770(
    NativeRenderPointerArrayStorage& array, std::int32_t requested) {
    resize_pointers(array, requested);
}
void destroy_native_instance_entry_pointers_00b1d1d0(NativeRenderPointerArrayStorage& array) {
    destroy_group_pointers(array);
}

void reserve_native_render_group_pointers_00b1c660(
    NativeRenderPointerArrayStorage& array, std::int32_t requested) {
    reserve_pointers(array, requested);
}
void resize_native_render_group_pointers_00b1c7c0(
    NativeRenderPointerArrayStorage& array, std::int32_t requested) {
    resize_pointers(array, requested);
}
void reserve_native_render_command_pointers_00b1c6c0(
    NativeRenderPointerArrayStorage& array, std::int32_t requested) {
    reserve_pointers(array, requested);
}
void resize_native_render_command_pointers_00b1cc80(
    NativeRenderPointerArrayStorage& array, std::int32_t requested) {
    resize_pointers(array, requested);
}

void destroy_native_render_command_pointers_00b1d590(NativeRenderPointerArrayStorage& array) {
    resize_native_render_command_pointers_00b1cc80(array, 0);
    const volatile auto& actual = array;
    singleton_lifetime_free(actual.data_00);
}

void append_native_render_group_pointer_00b1cbe0(
    NativeRenderPointerArrayStorage& array, const void* source_pointer_cell) {
    if (array.count_04 == array.capacity_08) {
        std::int32_t capacity = array.capacity_08 * 2;
        if (capacity < 2) capacity = 1;
        reserve_pointers(array, capacity);
    }
    void** const destination = array.data_00 + array.count_04;
    void* value;
    std::memcpy(&value, source_pointer_cell, sizeof(value));
    *destination = value;
    ++array.count_04;
}

void append_native_instance_entry_pointer_00b1cb80(
    NativeRenderPointerArrayStorage& array, const void* source_pointer_cell) {
    append_pointer(array, source_pointer_cell, reserve_native_instance_entry_pointers_00b1c500);
}

void append_native_render_command_pointer_00b1cc20(
    NativeRenderPointerArrayStorage& array, const void* source_pointer_cell) {
    append_pointer(array, source_pointer_cell, reserve_native_render_command_pointers_00b1c6c0);
}

void destroy_native_ordered_group_pointers_00b1d1f0(NativeRenderPointerArrayStorage& array) {
    destroy_group_pointers(array);
}
void destroy_native_indexed_group_pointers_00b1d260(NativeRenderPointerArrayStorage& array) {
    destroy_group_pointers(array);
}
} // namespace bsp

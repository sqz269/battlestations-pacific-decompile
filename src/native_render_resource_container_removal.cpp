#include "bsp/native_render_resource_container_removal.hpp"

#include "bsp/native_pooled_resource_path.hpp"
#include "bsp/native_render_resource_record.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render resource removal requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeRenderResourceRecord) == 0x2c);

// DWORD arithmetic retains native wrap and these isolated MOVs preserve the
// current-field load/store order without inventing a typed owner overlay.
const void* address(const void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
void* address(void* base, std::uint32_t offset) noexcept {
    return const_cast<void*>(address(static_cast<const void*>(base), offset));
}
__forceinline std::uint32_t read_word(const void* location) noexcept {
    std::uint32_t value;
    __asm { mov eax, location }
    __asm { mov eax, dword ptr [eax] }
    __asm { mov value, eax }
    return value;
}
__forceinline void write_word(void* location, std::uint32_t value) noexcept {
    __asm { mov eax, location }
    __asm { mov edx, value }
    __asm { mov dword ptr [eax], edx }
}
void* pointer(const void* location) noexcept {
    return reinterpret_cast<void*>(read_word(location));
}
void invalid(const SingletonLifetimeCallbacks& callbacks) {
    callbacks.invalid_parameter(callbacks.context); // BF6713 may return.
}
std::uint32_t accounted_size(void* resource,
    const NativeRenderResourceAccountingTables& tables) noexcept {
    const auto profile = read_word(resource);
    const volatile std::uint32_t* table;
    switch (profile) {
    case 0x00d61948u: table = tables.texture_2d_00d61948; break;
    case 0x00d61870u: table = tables.texture_cube_00d61870; break;
    case 0x00d618b0u: table = tables.texture_volume_00d618b0; break;
    default: __assume(0); // Explicit supported-profile precondition.
    }
    const auto entry = table[3]; // Native captured table's current slot+0C.
    switch (entry) {
    case 0x00b3ce30u: return native_resource_accounted_size_00b3ce30(resource);
    case 0x00a82250u: return native_resource_zero_accounted_size_00a82250();
    default: __assume(0); // No guessed implementation for another entry.
    }
}
// 004254B0 is the complete one-byte C3 function. Native calls evaluate these
// string pointer arguments but never dereference them or produce diagnostics.
void diagnostic_004254b0(const void*, const void* = nullptr) noexcept {}
const void* diagnostic_text(const void* data) noexcept {
    return data ? data : reinterpret_cast<const void*>(0x0108d5a4u);
}
} // namespace

std::uint32_t native_resource_accounted_size_00b3ce30(const void* actual_resource) noexcept {
    return read_word(address(actual_resource, 0x24));
}
std::uint32_t native_resource_zero_accounted_size_00a82250() noexcept {
    return 0;
}

void remove_native_render_resource_by_alias_00b31dc0(void* container,
    const void* original_name, SizedStoragePool& pool,
    const SingletonLifetimeCallbacks& callbacks,
    const NativeRenderResourceAccountingTables& tables) {
    PooledStringStorage storage(pool);
    alignas(4) unsigned char name[8];
    const bool distinct = static_cast<const void*>(name) != original_name;
    write_word(name, 0);
    write_word(name + 4, 0);
    if (distinct) {
        resize_native_string_header_0041dd40(name, storage, read_word(original_name), true);
        if (read_word(original_name) != 0) {
            const auto count = read_word(name);
            const auto* source = pointer(address(original_name, 4));
            auto* destination = pointer(name + 4);
            if (count != 0) std::memcpy(destination, source, count);
        }
    }
    // Only completed original-name construction arms state 0. Failed initial
    // resize/copy receives no local cleanup from this owner.
    try {
        alignas(4) unsigned char normalized[8];
        copy_construct_native_resource_path_header_00bee780(normalized, name, storage);
        destroy_native_string_header_0041dd20(normalized, storage);

        const auto count = read_word(address(container, 8));
        auto* cursor = pointer(address(container, 4));
        const auto* const end = address(cursor, count * 0x2cu);
        void* matched = nullptr;
        while (cursor != end) {
            auto* const sentinel_cell = address(cursor, 0x0c);
            const auto* const captured_end = pointer(sentinel_cell);
            auto* node = pointer(captured_end);
            // Native CMP EAX,EAX owner check is always equal; no error edge.
            while (node != captured_end) {
                if (node == pointer(sentinel_cell)) invalid(callbacks);
                const auto node_length = read_word(address(node, 8));
                const auto name_length = read_word(name);
                if (node_length == name_length) {
                    bool equal = true;
                    if (node_length != 0) {
                        const auto* name_data = static_cast<const char*>(pointer(name + 4));
                        const auto* node_data = static_cast<const char*>(pointer(address(node, 0x0c)));
                        equal = _stricmp(node_data, name_data) == 0;
                    }
                    if (equal) { matched = cursor; break; }
                }
                if (node == pointer(sentinel_cell)) invalid(callbacks);
                node = pointer(node);
            }
            if (matched) break;
            cursor = address(cursor, 0x2c);
        }
        if (!matched) {
            diagnostic_004254b0(diagnostic_text(pointer(address(original_name, 4))));
        } else {
            auto* const resource = pointer(address(matched, 0x28));
            const auto size = accounted_size(resource, tables);
            auto* const accounting = address(container, 0x10);
            write_word(accounting, read_word(accounting) - size);
            const auto* record_name = diagnostic_text(pointer(address(matched, 4)));
            const auto* local_name = diagnostic_text(pointer(name + 4));
            diagnostic_004254b0(local_name, record_name);

            const auto current_count = read_word(address(container, 8));
            auto* const current_data = pointer(address(container, 4));
            auto* const last = address(current_data, (current_count - 1u) * 0x2cu);
            if (matched != last) {
                assign_native_render_resource_record_00b30510(
                    *static_cast<NativeRenderResourceRecord*>(matched),
                    *static_cast<NativeRenderResourceRecord*>(last), pool, callbacks);
            }
            const auto destroy_count = read_word(address(container, 8));
            auto* const destroy_data = pointer(address(container, 4));
            destroy_native_render_resource_record_00b2f990(
                *static_cast<NativeRenderResourceRecord*>(address(
                    destroy_data, destroy_count * 0x2cu - 0x2cu)), pool);
            write_word(address(container, 8), read_word(address(container, 8)) - 1u);
        }
    } catch (...) {
        destroy_native_string_header_0041dd20(name, storage);
        throw;
    }
    // Native disarms state 0 before normal original-name storage release.
    destroy_native_string_header_0041dd20(name, storage);
}


void remove_native_render_resource_by_alias_00b31dc0(void* container,
    const void* original_name, ActualNativeStringPoolStorage& pool,
    const SingletonLifetimeCallbacks& callbacks,
    const NativeRenderResourceAccountingTables& tables) {
    auto& storage = pool;
    alignas(4) unsigned char name[8];
    const bool distinct = static_cast<const void*>(name) != original_name;
    write_word(name, 0);
    write_word(name + 4, 0);
    if (distinct) {
        resize_native_string_header_0041dd40(name, storage, read_word(original_name), true);
        if (read_word(original_name) != 0) {
            const auto count = read_word(name);
            const auto* source = pointer(address(original_name, 4));
            auto* destination = pointer(name + 4);
            if (count != 0) std::memcpy(destination, source, count);
        }
    }
    // Only completed original-name construction arms state 0. Failed initial
    // resize/copy receives no local cleanup from this owner.
    try {
        alignas(4) unsigned char normalized[8];
        copy_construct_native_resource_path_header_00bee780(normalized, name, storage);
        destroy_native_string_header_0041dd20(normalized, storage);

        const auto count = read_word(address(container, 8));
        auto* cursor = pointer(address(container, 4));
        const auto* const end = address(cursor, count * 0x2cu);
        void* matched = nullptr;
        while (cursor != end) {
            auto* const sentinel_cell = address(cursor, 0x0c);
            const auto* const captured_end = pointer(sentinel_cell);
            auto* node = pointer(captured_end);
            // Native CMP EAX,EAX owner check is always equal; no error edge.
            while (node != captured_end) {
                if (node == pointer(sentinel_cell)) invalid(callbacks);
                const auto node_length = read_word(address(node, 8));
                const auto name_length = read_word(name);
                if (node_length == name_length) {
                    bool equal = true;
                    if (node_length != 0) {
                        const auto* name_data = static_cast<const char*>(pointer(name + 4));
                        const auto* node_data = static_cast<const char*>(pointer(address(node, 0x0c)));
                        equal = _stricmp(node_data, name_data) == 0;
                    }
                    if (equal) { matched = cursor; break; }
                }
                if (node == pointer(sentinel_cell)) invalid(callbacks);
                node = pointer(node);
            }
            if (matched) break;
            cursor = address(cursor, 0x2c);
        }
        if (!matched) {
            diagnostic_004254b0(diagnostic_text(pointer(address(original_name, 4))));
        } else {
            auto* const resource = pointer(address(matched, 0x28));
            const auto size = accounted_size(resource, tables);
            auto* const accounting = address(container, 0x10);
            write_word(accounting, read_word(accounting) - size);
            const auto* record_name = diagnostic_text(pointer(address(matched, 4)));
            const auto* local_name = diagnostic_text(pointer(name + 4));
            diagnostic_004254b0(local_name, record_name);

            const auto current_count = read_word(address(container, 8));
            auto* const current_data = pointer(address(container, 4));
            auto* const last = address(current_data, (current_count - 1u) * 0x2cu);
            if (matched != last) {
                assign_native_render_resource_record_00b30510(
                    *static_cast<NativeRenderResourceRecord*>(matched),
                    *static_cast<NativeRenderResourceRecord*>(last), pool, callbacks);
            }
            const auto destroy_count = read_word(address(container, 8));
            auto* const destroy_data = pointer(address(container, 4));
            destroy_native_render_resource_record_00b2f990(
                *static_cast<NativeRenderResourceRecord*>(address(
                    destroy_data, destroy_count * 0x2cu - 0x2cu)), pool);
            write_word(address(container, 8), read_word(address(container, 8)) - 1u);
        }
    } catch (...) {
        destroy_native_string_header_0041dd20(name, storage);
        throw;
    }
    // Native disarms state 0 before normal original-name storage release.
    destroy_native_string_header_0041dd20(name, storage);
}

} // namespace bsp

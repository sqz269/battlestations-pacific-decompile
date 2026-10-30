#include "bsp/native_mesh_section_layout_storage.hpp"

#include "bsp/native_hardware_layout_factory.hpp"
#include "bsp/native_mesh_remaining_fields.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_renderer_binding_getters.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4 && sizeof(long) == 4);

Word bits(const void* pointer) {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(pointer));
}
void* pointer(Word value) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
Word load(Word address) {
    return *reinterpret_cast<const volatile Word*>(static_cast<std::uintptr_t>(address));
}
void store(Word address, Word value) {
    *reinterpret_cast<volatile Word*>(static_cast<std::uintptr_t>(address)) = value;
}
std::int32_t signed_word(Word value) {
    return static_cast<std::int32_t>(value); // MSVC Win32 signed DWORD interpretation.
}
struct CurrentTarget { Word profile, target; };
CurrentTarget current(Word receiver, Word slot,
    const NativeMeshSectionLayoutStorageContext& c) {
    const Word profile = load(receiver);
    const volatile Word* const table = c.bindings.current_table(pointer(receiver), profile);
    const Word target = table[slot / 4u];
    return {profile, target};
}
void release_captured(Word captured_reference,
    const NativeMeshSectionLayoutStorageContext& c) {
    const auto decrement = c.decrement_00ce2220;
    if (decrement(reinterpret_cast<volatile long*>(
        static_cast<std::uintptr_t>(captured_reference + 4u))) == 0) {
        const auto call = current(captured_reference, 0, c);
        if (call.target == 0x00bd30e0u)
            invoke_native_ref_counted_delete_00bd30e0(pointer(captured_reference), c.scalar_delete);
        else
            c.bindings.other_reference_current00(pointer(captured_reference), call.profile, call.target);
    }
}
} // namespace

__declspec(noinline) void clear_native_mesh_section_vertex_streams_storage_00b86550(
    void* captured_actual_section, const NativeMeshSectionLayoutStorageContext& c) {
    const Word section = bits(captured_actual_section);
    Word index = 0;
    if (signed_word(load(section + 0x4cu)) > 0) {
        Word slot = section + 0x3cu;
        do {
            const Word captured_stream = load(slot);
            if (captured_stream != 0) {
                release_captured(captured_stream, c);
                store(slot, 0); // Includes a callback replacement; only after return.
            }
            index += 1u;
            slot += 4u;
        } while (signed_word(index) < signed_word(load(section + 0x4cu)));
    }
    store(section + 0x4cu, 0);
}

__declspec(noinline) void rebuild_native_mesh_section_vertex_layout_storage_00b865a0(
    void* captured_actual_section, const volatile std::uint32_t& actual_mesh_dword_home,
    void* actual_key_home, const NativeMeshSectionLayoutStorageContext& c,
    void** acquired_before_insertion) {
    const Word section = bits(captured_actual_section);
    if (load(section + 0x50u) != 0) {
        clear_native_mesh_section_vertex_streams_storage_00b86550(captured_actual_section, c);
        const Word captured_layout = load(section + 0x50u); // AFTER the whole clear.
        if (captured_layout != 0) {
            release_captured(captured_layout, c);
            store(section + 0x50u, 0);
        }
        if (load(section + 0x50u) != 0) return;
    }
    if (load(section + 0x4cu) == 0) {
        const Word current_mesh = actual_mesh_dword_home;
        void* const captured_stream = native_mesh_vertex_stream_unchecked_00b73260(
            pointer(current_mesh), 0);
        const auto increment = c.increment_00ce221c;
        increment(reinterpret_cast<volatile long*>(
            static_cast<std::uintptr_t>(bits(captured_stream) + 4u)));
        const Word current_index = load(section + 0x4cu); // AFTER increment call.
        store(section + 0x3cu + current_index * 4u, bits(captured_stream));
        // ADD [section+4C],1 reads AFTER append; append itself can alias +4C.
        store(section + 0x4cu, load(section + 0x4cu) + 1u);
    }
    const Word key = bits(actual_key_home);
    Word index = 0;
    const auto initial_count = signed_word(load(section + 0x4cu)); // CMP first.
    store(key + 0x10u, 0); // MOV preserves the CMP result used by Native JLE.
    if (initial_count > 0) {
        Word slot = section + 0x3cu;
        do {
            const Word captured_stream = load(slot);
            const auto call = current(captured_stream, 0x24u, c);
            void* const descriptor = call.target == 0x00b48ce0u
                ? native_logical_vertex_stream_get_declaration_00b48ce0(pointer(captured_stream))
                : c.bindings.other_stream_descriptor_current24(
                    pointer(captured_stream), call.profile, call.target);
            const Word current_key_index = load(key + 0x10u); // AFTER callback.
            store(key + current_key_index * 4u, bits(descriptor));
            store(key + 0x10u, load(key + 0x10u) + 1u); // AFTER descriptor store.
            index += 1u;
            slot += 4u;
        } while (signed_word(index) < signed_word(load(section + 0x4cu)));
    }
    void* const captured_renderer = c.actual_renderer_00f8d394;
    const auto call = current(bits(captured_renderer), 0x40u, c);
    void* const layout = call.target == 0x00b2f710u
        ? get_or_create_native_hardware_layout_00b2f710(
            actual_key_home, c.hardware_layouts, acquired_before_insertion)
        : c.bindings.other_renderer_layout_current40(
            captured_renderer, call.profile, call.target, actual_key_home, acquired_before_insertion);
    store(section + 0x50u, bits(layout));
}
} // namespace bsp

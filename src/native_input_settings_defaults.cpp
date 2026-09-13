#include "bsp/native_input_settings_defaults.hpp"
#include "bsp/native_input_binding_slots.hpp"
#include "bsp/native_input_deadline_map_lookup_adapter.hpp"
#include "bsp/native_int_pointer_tree18_leaves.hpp"
#include <array>
#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
using Word = std::uint32_t;
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
template<class T = Word> T read(Word p, Word off = 0) noexcept {
    T result; std::memcpy(&result, pointer(p + off), sizeof result); return result;
}
template<class T = Word> void write(Word p, Word off, T value) noexcept {
    std::memcpy(pointer(p + off), &value, sizeof value);
}
struct Iterator { void* owner; void* node; };
void spill_scale(float& scale) noexcept {
    float* storage = &scale;
    __asm {
        mov eax, storage
        fld dword ptr [eax]
        fstp dword ptr [eax]
    }
}
void validate_node(Word owner, Word node) {
    if (node == read(owner, 4)) _invalid_parameter_noinfo();
}
} // namespace

std::int32_t* subscript_native_input_settings_binding_counts(
    void* tree, const std::int32_t* key) {
    // Both native scalar subscript families use signed keys and an all-zero
    // mapped DWORD on insertion. The shared adapter returns storage only.
    return reinterpret_cast<std::int32_t*>(subscript_input_deadline_map_storage(tree, key));
}

void preserve_native_input_default_bindings_006ab820(
    void* actual_settings, NativeInputSettingsDefaultsServices& services) {
    const auto settings = address(actual_settings);
    if (read<std::uint8_t>(settings, 0x50) != 0) return;
    void* const owner = get_native_input_action_owner_004bec00(services.action_owner);
    const auto tree = settings + 0x54u;
    for (std::int32_t key : {0x4a, 0x4b, 0x46, 0x47, 0x4c, 0x4d, 1})
        subscript_native_input_settings_binding_counts(pointer(tree), &key);
    Iterator iterator{pointer(tree), pointer(read(read(tree, 4)))};
    for (;;) {
        const auto container = address(iterator.owner), node = address(iterator.node);
        const auto endpoint = read(settings, 0x58); // captured before owner validation
        if (container == 0 || container != tree) _invalid_parameter_noinfo();
        if (node == endpoint) break;
        if (container == 0) _invalid_parameter_noinfo();
        if (node == read(container, 4)) {
            _invalid_parameter_noinfo();
            validate_node(container, node);
        }
        write(node, 0x10, count_native_input_binding_slots_00a92820(owner, read(node, 0xc)));
        Word slot = 0;
        for (;;) {
            validate_node(container, node);
            if (static_cast<std::int32_t>(slot) >= read<std::int32_t>(node, 0x10)) break;
            std::array<Word, 5> descriptor;
            float scale;
            validate_node(container, node);
            read_native_input_binding_slot_00a92790(owner, read(node, 0xc),
                static_cast<std::int32_t>(slot), descriptor.data(), &scale,
                services.reader_missing_flag_preimage);
            validate_node(container, node);
            spill_scale(scale);
            install_native_input_binding_slot_00a93750(owner, read(node, 0xc),
                static_cast<std::int32_t>(slot + 4u), descriptor, scale,
                services.binding_storage, services.backend_00f8bbf4);
            validate_node(container, node);
            install_native_input_binding_slot_00a93750(owner, read(node, 0xc),
                static_cast<std::int32_t>(slot),
                {0xffffffffu, 0u, 0u, 0xffffffffu,
                    services.cleared_descriptor_flag_preimage & 0xffffff00u}, 0.0f,
                services.binding_storage, services.backend_00f8bbf4);
            ++slot;
        }
        increment_native_int_pointer_tree18_00869a20(&iterator);
    }
    services.keyboard.apply_keyboard_bindings_006aa640(actual_settings);
}
} // namespace bsp

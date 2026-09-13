#include "bsp/native_input_keyboard_apply.hpp"
#include "bsp/native_input_binding_slots.hpp"
#include "bsp/native_input_configuration_cleanup.hpp"
#include "bsp/native_input_deadline_map_lookup_adapter.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <array>
#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
template<class T = Word> T read(Word p, Word off = 0) noexcept {
    T value; std::memcpy(&value, pointer(p + off), sizeof value); return value;
}
template<class T = Word> void write(Word p, Word off, T value) noexcept {
    std::memcpy(pointer(p + off), &value, sizeof value);
}
Word distance(Word first, Word last, unsigned shift) noexcept {
    return static_cast<Word>(static_cast<std::int32_t>(last - first) >> shift);
}
void node_valid(Word owner, Word node) {
    if (node == read(owner, 4)) _invalid_parameter_noinfo();
}
NativeKeyboardTreeIterator begin_tree(Word tree) {
    return {pointer(tree), pointer(read(read(tree, 4)))};
}
bool tree_done(const NativeKeyboardTreeIterator& it, Word tree) {
    const auto owner = address(it.owner), node = address(it.node), end = read(tree, 4);
    if (!owner || owner != tree) _invalid_parameter_noinfo();
    return node == end;
}
void tree_dereference(const NativeKeyboardTreeIterator& it) {
    const auto owner = address(it.owner), node = address(it.node);
    if (!owner) _invalid_parameter_noinfo();
    node_valid(owner, node);
}
bool checked_index_available(Word header, Word index, unsigned shift) {
    const auto first = read(header, 4);
    return first && index < distance(first, read(header, 8), shift);
}
void validate_checked_index(Word header, Word index, unsigned shift) {
    if (!checked_index_available(header, index, shift)) _invalid_parameter_noinfo();
}
void replace_zero_scale(float* scale, const NativeInputKeyboardConstants& constants) noexcept {
    const volatile float* zero = &constants.zero_00d7a218;
    const volatile float* one = &constants.one_00d7a24c;
    __asm {
        mov edx, scale
        mov ecx, zero
        movss xmm0, dword ptr [edx]
        ucomiss xmm0, dword ptr [ecx]
        lahf
        test ah, 44h
        jp unchanged
        mov ecx, one
        movss xmm0, dword ptr [ecx]
        movss dword ptr [edx], xmm0
    unchanged:
    }
}
void load_scale(float* result, const float* source, const NativeInputKeyboardConstants& constants) noexcept {
    // MOVSS capture first; ordered-zero replacement changes only the local.
    std::memcpy(result, source, sizeof(float));
    replace_zero_scale(result, constants);
}
bool magnitude_at_least_one(const float* multiplier,
    const NativeInputKeyboardConstants& constants) noexcept {
    const volatile float* one = &constants.one_00d7a24c;
    const volatile float* negative_one = &constants.negative_one_00d7a260;
    std::uint8_t result;
    __asm {
        mov eax, multiplier
        mov ecx, one
        movss xmm0, dword ptr [eax]
        comiss xmm0, dword ptr [ecx]
        jnc yes
        mov ecx, negative_one
        movss xmm1, dword ptr [ecx]
        comiss xmm1, xmm0
        setnc result
        jmp done
    yes:
        mov result, 1
    done:
    }
    return result != 0;
}
void multiply_scale(float* scale, const float* multiplier) noexcept {
    __asm {
        mov eax, multiplier
        mov edx, scale
        fld dword ptr [eax]
        fmul dword ptr [edx]
        fstp dword ptr [edx]
    }
}
void reverse_scale(float& scale, const volatile double& sign) noexcept {
    float* value = &scale; const volatile double* factor = &sign;
    __asm {
        mov eax, value
        mov edx, factor
        fld dword ptr [eax]
        fmul qword ptr [edx]
        fstp dword ptr [eax]
    }
}
void spill_scale(float& scale) noexcept {
    float* value = &scale;
    __asm {
        mov eax, value
        fld dword ptr [eax]
        fstp dword ptr [eax]
    }
}
void alternate_scale(float& scale, const volatile float& negative_zero) noexcept {
    float* value = &scale; const volatile float* zero = &negative_zero;
    __asm {
        mov eax, value
        mov edx, zero
        movss xmm0, dword ptr [edx]
        subss xmm0, dword ptr [eax]
        movss dword ptr [eax], xmm0
    }
}
std::array<Word, 5> descriptor_words(Word record) noexcept {
    return {read(record), read(record, 4), read(record, 8), read(record, 12), read(record, 16)};
}
struct ScaleTree {
    struct Storage { Word opaque; void* head; Word count; } storage;
    NativeInputKeyboardLibrary& library;
    bool owned = false;
    ScaleTree(NativeInputKeyboardLibrary& calls, Word opaque) : storage{opaque, nullptr, 0}, library(calls) {}
    ~ScaleTree() noexcept { if (owned) library.destroy_scales_0055b490(&storage); }
    void copy(const void* source) { library.copy_scales_0055b400(&storage, source); owned = true; }
    void release() {
        owned = false; // native disarms state0 before the normal erase call
        const auto tree = address(&storage), head = read(tree, 4), first = read(head);
        NativeKeyboardTreeIterator output;
        library.erase_scales_0055b230(&storage, &output, {&storage, pointer(first)}, {&storage, pointer(head)});
        singleton_lifetime_free(storage.head);
        storage.head = nullptr; storage.count = 0;
    }
};
} // namespace

std::uint8_t equal_native_input_code_ranges(const void* first_begin, const void* first_end,
    const void* second_owner, const void* second_begin) {
    const auto first = address(first_begin), end = address(first_end), owner = address(second_owner), second = address(second_begin);
    const auto count = distance(first, end, 2);
    if (!owner) _invalid_parameter_noinfo();
    const auto second_end = second + count * 4u;
    if (second_end > read(owner, 8) || second_end < read(owner, 4)) _invalid_parameter_noinfo();
    auto current = first;
    while (current != end && read(current) == read(second + current - first)) current += 4;
    return static_cast<std::uint8_t>(current == end);
}
std::uint32_t equal_native_input_code_vectors_0069e860(const void* first, const void* second) {
    const auto a = address(first), b = address(second), ab = read(a, 4);
    const auto ac = ab ? distance(ab, read(a, 8), 2) : 0;
    const auto bb = read(b, 4), bc = bb ? distance(bb, read(b, 8), 2) : 0;
    if (ac != bc) return 0;
    if (bb > read(b, 8)) _invalid_parameter_noinfo();
    const auto ae = read(a, 8);
    if (read(a, 4) > ae) _invalid_parameter_noinfo();
    const auto begin = read(a, 4);
    if (begin > read(a, 8)) _invalid_parameter_noinfo();
    return equal_native_input_code_ranges(pointer(begin), pointer(ae), second, pointer(bb)) != 0;
}
std::uint8_t use_native_input_alternate_axis_slots_006aa090(void* settings,
    const void* device_name, const void* input_name, NativeInputKeyboardLibrary& library) {
    const auto device = address(library.device_0055c110(pointer(address(settings) + 8u), device_name));
    const auto pairs = device + 0x5cu;
    auto row = read(pairs, 4);
    if (row > read(pairs, 8)) _invalid_parameter_noinfo();
    for (;;) {
        const auto end = read(pairs, 8);
        if (read(pairs, 4) > end) _invalid_parameter_noinfo();
        // Native CMP EBP,EBP at6AA0C3 makes its diagnostic unreachable.
        if (row == end) return 0;
        if (row >= read(pairs, 8)) _invalid_parameter_noinfo();
        validate_checked_index(row, 1, 3);
        auto second = read(row, 4) + 8u;
        const auto length = read(address(input_name)), second_length = read(second);
        if (length == second_length && (length == 0 ||
            _stricmp(static_cast<const char*>(pointer(read(address(input_name), 4))),
                     static_cast<const char*>(pointer(read(second, 4)))) == 0)) {
            validate_checked_index(row, 1, 3);
            const auto begin = read(row, 4);
            second = begin + 8u;
            if (!begin || distance(begin, read(row, 8), 3) == 0) _invalid_parameter_noinfo();
            const auto first = read(row, 4);
            void* const second_codes = library.input_codes_006a44b0(pointer(device), pointer(second));
            void* const first_codes = library.input_codes_006a44b0(pointer(device), pointer(first));
            if (equal_native_input_code_vectors_0069e860(first_codes, second_codes)) return 1;
        }
        if (row >= read(pairs, 8)) _invalid_parameter_noinfo();
        row += 0x10u;
    }
}
std::uint32_t suppress_native_input_alternate_binding_00699bf0(const void* descriptor) noexcept {
    const auto p = address(descriptor), kind = read(p);
    return (kind == 1 && read(p, 0xc) >= 8) || (kind == 2 && read(p, 0xc) >= 60);
}

void apply_native_input_keyboard_bindings_006aa640(void* actual_settings,
    NativeInputKeyboardApplyContext& context) {
    const auto settings = address(actual_settings);
    if (read<std::uint8_t>(settings, 0x50) || !read<std::uint8_t>(settings, 5)) return;
    void* const input_owner = get_native_input_action_owner_004bec00(context.action_owner);
    if (!(query_native_input_action_registered_00a92260(input_owner, 0x128) & 0xffu)) return;
    auto& library = context.library;
    auto category_class = static_cast<std::int32_t>(context.initial_sensitivity_class_preimage);
    auto devices = begin_tree(settings + 8u);
    while (!tree_done(devices, settings + 8u)) {
        tree_dereference(devices);
        const auto device_node = address(devices.node), device_name = device_node + 0xcu;
        node_valid(address(devices.owner), device_node);
        const auto device = device_node + 0x14u;
        ScaleTree scales(library, context.temporary_map_opaque_preimage);
        scales.copy(pointer(device + 0x50u));
        auto sensitivities = begin_tree(device + 0x18u);
        while (!tree_done(sensitivities, device + 0x18u)) {
            tree_dereference(sensitivities);
            const auto node = address(sensitivities.node);
            node_valid(address(sensitivities.owner), node);
            float* const multiplier = library.multiplier_00444be0(pointer(device + 0x24u), pointer(node + 0xcu));
            switch (read(node, 0x14)) {
            case 0: category_class = 0; break;
            case 1: case 3: category_class = 1; break;
            case 2: case 4: category_class = 2; break;
            default: break; // retain prior local/private stack value, as native
            }
            for (Word index = 0; checked_index_available(node + 0x18u, index, 2); ++index) {
                validate_checked_index(node + 0x18u, index, 2);
                auto code = read<std::int32_t>(read(node, 0x1c) + index * 4u);
                void* const classes = library.code_classes_006a5aa0(&scales.storage, &code);
                float* const scale = subscript_input_deadline_map_storage(classes, &category_class);
                replace_zero_scale(scale, context.constants);
                bool multiply = magnitude_at_least_one(multiplier, context.constants);
                if (!multiply) {
                    const auto hacks = device + 0x78u, end = read(hacks, 4);
                    NativeKeyboardTreeIterator result;
                    auto* returned = library.find_hack_00546840(pointer(hacks), &result, &code);
                    const auto owner = address(returned->owner);
                    if (!owner || owner != hacks) _invalid_parameter_noinfo();
                    multiply = address(returned->node) == end;
                }
                if (multiply) multiply_scale(scale, multiplier);
            }
            library.next_sensitivity_00552770(&sensitivities);
        }
        auto inputs = begin_tree(device);
        while (!tree_done(inputs, device)) {
            tree_dereference(inputs);
            const auto node = address(inputs.node), input_name = node + 0xcu;
            const auto alternate = use_native_input_alternate_axis_slots_006aa090(actual_settings,
                pointer(device_name), pointer(input_name), library);
            for (Word slot = 0; slot < 2; ++slot) {
                const auto bindings = address(library.bindings_006a45c0(pointer(device + 0xcu), pointer(input_name)));
                const auto first = read(bindings, 4);
                const auto count = static_cast<Word>(static_cast<std::int32_t>(read(bindings, 8) - first) / 20);
                if (!first || slot >= count) _invalid_parameter_noinfo();
                const auto binding = read(bindings, 4) + slot * 0x14u;
                Word index = 0;
                for (;;) {
                    node_valid(address(inputs.owner), node);
                    if (!checked_index_available(node + 0x14u, index, 2)) break;
                    node_valid(address(inputs.owner), node);
                    validate_checked_index(node + 0x14u, index, 2);
                    auto code = read<std::int32_t>(read(node, 0x18) + index * 4u);
                    void* const classes = library.code_classes_006a5aa0(&scales.storage, &code);
                    float* const source_scale = subscript_input_deadline_map_storage(classes, static_cast<const std::int32_t*>(pointer(binding)));
                    float scale; load_scale(&scale, source_scale, context.constants);
                    auto reverse = address(library.reverse_006a4ca0(pointer(device + 0x6cu), pointer(input_name)));
                    const auto bits = read(reverse, 8);
                    if (bits > read(reverse, 0xc)) _invalid_parameter_noinfo();
                    if (!reverse) _invalid_parameter_noinfo();
                    NativeKeyboardBitIterator bit{pointer(reverse), pointer(bits), 0};
                    if (!reverse) _invalid_parameter_noinfo();
                    library.advance_bit_0048d3b0(&bit, static_cast<std::int32_t>(slot));
                    reverse = address(bit.owner);
                    if (!reverse) { _invalid_parameter_noinfo(); _invalid_parameter_noinfo(); _invalid_parameter_noinfo(); }
                    else if (!bit.word) _invalid_parameter_noinfo();
                    const auto current_bits = read(reverse, 8);
                    if (current_bits > read(reverse, 0xc)) _invalid_parameter_noinfo();
                    const auto position = distance(current_bits, address(bit.word), 2) * 32u + bit.bit;
                    if (position >= read(reverse)) _invalid_parameter_noinfo();
                    if (read(address(bit.word)) & (1u << (bit.bit & 31u))) reverse_scale(scale, context.constants.reverse_sign_00d7a250);
                    if (!alternate) {
                        spill_scale(scale);
                        install_native_input_binding_slot_00a93750(input_owner, static_cast<Word>(code), static_cast<std::int32_t>(slot),
                            descriptor_words(binding), scale, context.binding_storage, context.backend_00f8bbf4);
                    } else if (!suppress_native_input_alternate_binding_00699bf0(pointer(binding))) {
                        alternate_scale(scale, context.constants.negative_zero_00d7a208);
                        install_native_input_binding_slot_00a93750(input_owner, static_cast<Word>(code), static_cast<std::int32_t>(slot + 2u),
                            descriptor_words(binding), scale, context.binding_storage, context.backend_00f8bbf4);
                    } else {
                        install_native_input_binding_slot_00a93750(input_owner, static_cast<Word>(code), static_cast<std::int32_t>(slot + 2u),
                            {0xffffffffu, 0u, 0u, 0xffffffffu, context.suppressed_flag_stack_preimage & 0xffffff00u},
                            0.0f, context.binding_storage, context.backend_00f8bbf4);
                    }
                    ++index;
                }
            }
            library.next_input_00552d40(&inputs);
        }
        scales.release();
        library.next_device_005540c0(&devices);
    }
}
void NativeInputKeyboardApplication::apply_keyboard_bindings_006aa640(void* settings) {
    apply_native_input_keyboard_bindings_006aa640(settings, context_);
}
} // namespace bsp

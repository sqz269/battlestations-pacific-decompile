#include "bsp/native_input_action_configuration.hpp"
#include "bsp/native_input_action_listener_owner.hpp"
#include <cstddef>
#include <initializer_list>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
using Word = std::uint32_t;
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
template<class T = Word> volatile T& field(Word p, Word offset = 0) noexcept {
    return *reinterpret_cast<volatile T*>(p + offset);
}
std::uint8_t positive_hold(Word action, Word latch_offset, Word value_offset) noexcept {
    if (!field<std::uint8_t>(action, latch_offset)) return 0;
    const auto* value = reinterpret_cast<const float*>(action + value_offset);
    std::uint8_t result;
    // COMISS/JBE: unordered is false and preserves the SSE exception behavior.
    __asm {
        mov eax, value
        xorps xmm0, xmm0
        movss xmm1, dword ptr [eax]
        comiss xmm1, xmm0
        seta result
    }
    return result;
}
void copy_current_to_previous(Word action) noexcept {
    __asm {
        mov ecx, action
        fld dword ptr [ecx + 24h]
        mov al, byte ptr [ecx + 28h]
        fstp dword ptr [ecx + 1ch]
        mov byte ptr [ecx + 20h], al
    }
}
} // namespace

void activate_native_input_action_context_00a92d40(void* actual_action,
    const void* actual_context_header, std::uint8_t context_enabled,
    std::int32_t active_context, NativeInputActionConfigurationContext& context) {
    const auto action = address(actual_action);
    auto current = field(action, 4);
    field<std::uint8_t>(action, 1) = 0;
    if (current != current + field(action, 8) * 4u) {
        const auto end = current + field(action, 8) * 4u;
        do {
            const auto index = field(current);
            if (index == 0xffffffffu || (context_enabled &&
                field<std::int32_t>(field(address(actual_context_header)) + index * 4u)
                    == active_context)) {
                field<std::uint8_t>(action, 1) = 1;
                break;
            }
            current += 4u;
        } while (current != end);
    }
    if (!field<std::uint8_t>(action, 1)) {
        const auto listener = field(action, 0x2c);
        field(action, 0x1c) = 0;
        field<std::uint8_t>(action, 0x20) = 0;
        field(action, 0x24) = 0;
        field<std::uint8_t>(action, 0x28) = 0;
        // TEST(-!!listener,F8BC00) tests the pointer, never the global's value.
        if (listener) {
            for (const Word offset : {8u, 10u, 11u, 13u, 14u, 15u, 16u,
                                      9u, 12u, 17u, 18u, 19u})
                field<std::uint8_t>(listener, offset) = 0;
            field(listener, 0x14) = 0;
            field(listener, 0x18) = 0;
            field(listener, 0x1c) = 0;
            field(listener, 0x20) = 0;
        }
        return;
    }
    poll_native_input_action_00a92370(actual_action, context.bindings);
    const auto listener = field(action, 0x2c);
    if (listener) {
        const auto current_down = positive_hold(action, 0x28, 0x24);
        const auto previous_down = positive_hold(action, 0x20, 0x1c);
        float seconds;
        __asm {
            fld1
            fstp seconds
        }
        update_native_input_action_listener_00a91a50(pointer(listener), seconds,
            previous_down, current_down, context.timing);
    }
    copy_current_to_previous(action);
}

void configure_native_input_action_00a93c80(void* actual_owner,
    std::uint32_t action_index, const void* actual_context_source,
    std::uint8_t replace_listener, NativeInputActionConfigurationContext& context) {
    const auto owner = address(actual_owner);
    if (action_index >= field(owner, 8))
        resize_native_input_actions_00a93c10(pointer(owner + 4u),
            static_cast<std::int32_t>(action_index + 1u), context.records);
    const auto action = field(owner, 4) + action_index * 0x30u;
    field<std::uint8_t>(action) = 1;
    if (replace_listener)
        replace_native_input_action_listener_00a92b70(pointer(action), context.records.listeners);
    assign_native_input_context_words_00a92e70(pointer(action + 4u), actual_context_source);
    const auto source = address(actual_context_source);
    auto current = field(source);
    if (current != current + field(source, 4) * 4u) {
        do {
            const auto index = field<std::int32_t>(current);
            if (index != -1) {
                const auto old_count = field<std::int32_t>(owner, 0x14);
                if (index >= old_count) {
                    const auto count = static_cast<std::int32_t>(static_cast<Word>(index) + 1u);
                    resize_native_input_dwords_0086a430(pointer(owner + 0x10u), count);
                    auto slot = old_count;
                    while (slot < count) {
                        field(field(owner, 0x10) + static_cast<Word>(slot) * 4u) = 0;
                        slot = static_cast<std::int32_t>(static_cast<Word>(slot) + 1u);
                    }
                }
            }
            // Native restores the captured current pointer after resize, then
            // reloads source count/base before incrementing and comparing it.
            const auto count = field(source, 4);
            const auto base = field(source);
            current += 4u;
            if (current == base + count * 4u) break;
        } while (true);
    }
    const auto enabled = field<std::uint8_t>(owner, 0x1c);
    const auto active = field<std::int32_t>(owner, 0x20);
    activate_native_input_action_context_00a92d40(pointer(action), pointer(owner + 0x10u),
        enabled, active, context);
}
} // namespace bsp

#include "bsp/native_input_action_contexts.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
template<class T = Word> volatile T& field(Word p, Word offset = 0) noexcept {
    return *reinterpret_cast<volatile T*>(p + offset);
}
}
void refresh_native_input_action_contexts_00a93020(void* actual_owner,
    NativeInputActionConfigurationContext& context) {
    const auto owner = address(actual_owner);
    const auto initial_count = field(owner, 8);
    auto action = field(owner, 4);
    if (action == action + initial_count * 0x30u) return;
    void* const context_header = pointer(owner + 0x10u);
    do {
        const auto active = field<std::int32_t>(owner, 0x20);
        const auto enabled = field<std::uint8_t>(owner, 0x1c);
        activate_native_input_action_context_00a92d40(pointer(action), context_header,
            enabled, active, context);
        const auto count = field(owner, 8);
        const auto end = count * 0x30u + field(owner, 4);
        action += 0x30u;
        if (action == end) break;
    } while (true);
}
void set_native_input_context_level_00a933f0(void* actual_owner, Word index,
    Word level, NativeInputActionConfigurationContext& context) {
    const auto owner = address(actual_owner);
    const auto initial_base = field(owner, 0x10);
    field(initial_base + index * 4u) = level;
    auto current = field(owner, 0x10);
    field(owner, 0x20) = 1;
    const auto initial_count = field(owner, 0x14);
    if (current != current + initial_count * 4u) {
        do {
            const auto value = field(current);
            if (value > field(owner, 0x20)) field(owner, 0x20) = value;
            const auto count = field(owner, 0x14);
            const auto base = field(owner, 0x10);
            current += 4u;
            if (current == base + count * 4u) break;
        } while (true);
    }
    refresh_native_input_action_contexts_00a93020(actual_owner, context);
}
} // namespace bsp

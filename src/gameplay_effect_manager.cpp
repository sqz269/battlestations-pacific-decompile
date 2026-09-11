#include "bsp/gameplay_effect_manager.hpp"

#include <new>

namespace bsp {
namespace {
struct EffectManagerCapturedSection {
    SystemSingletonCriticalSection* section;
    explicit EffectManagerCapturedSection(SystemSingletonCriticalSection* captured)
        : section(captured) {
        if (section) {
            singleton_enter_critical_section(*section);
            ++section->recursion_18;
        }
    }
    ~EffectManagerCapturedSection() {
        if (section) {
            --section->recursion_18;
            singleton_leave_critical_section(*section);
        }
    }
};
} // namespace

GameplayEffectManager& construct_gameplay_effect_manager_00870370(
    GameplayEffectManager& owner, const GameplayEffectManagerAllocationWords& words) {
    owner.vtable_00 = 0x00d0da64;
    owner.allocator_04 = words.allocator_04;
    owner.definitions.emplace();
    return owner;
}
GameplayEffectManager* get_gameplay_effect_manager_004c1650(
    GameplayEffectManagerContext& context) {
    if (auto* const existing = context.singleton_00f87664) return existing;
    {
        auto& first_manager = context.lifetime.get_manager_00415350()->system_owner();
        EffectManagerCapturedSection lock(first_manager.section_10);
        if (!context.singleton_00f87664) {
            void* const storage = singleton_lifetime_allocate(
                {SingletonAllocationKind::object, 0x10, sizeof(GameplayEffectManager)});
            GameplayEffectManager* owner = nullptr;
            if (storage) {
                owner = ::new (storage) GameplayEffectManager;
                construct_gameplay_effect_manager_00870370(*owner, context.allocation_words);
            }
            context.singleton_00f87664 = owner;
            auto* const second_manager = context.lifetime.get_manager_00415350();
            second_manager->register_object(context.singleton_00f87664);
        }
    } // Captured recursion decrement and leave precede the final global reload.
    return context.singleton_00f87664;
}
void destroy_gameplay_effect_manager_0086fe20(GameplayEffectManager& owner,
    GameplayEffectManagerContext& context) noexcept {
    owner.vtable_00 = 0x00d0da64;
    // 0086AA60 frees tree nodes without inspecting their raw definition values.
    // The subsequent native erase-range sees an already empty tree. reset()
    // includes head destruction; disengagement represents native head/count=0.
    owner.definitions.reset();
    context.singleton_00f87664 = nullptr;
    owner.vtable_00 = 0x00ce3818;
}
GameplayEffectManager* scalar_delete_gameplay_effect_manager_008703e0(
    GameplayEffectManager* owner, std::uint32_t flags,
    GameplayEffectManagerContext& context) noexcept {
    auto* const original = owner;
    destroy_gameplay_effect_manager_0086fe20(*owner, context);
    if ((flags & 1u) != 0) {
        owner->~GameplayEffectManager();
        singleton_lifetime_free(owner);
    }
    return original;
}
void probe_gameplay_effect_registry_0086b0b0(const GameplayEffectManager& owner,
    const char* label) {
    (void)label;
    const auto& definitions = *owner.definitions;
    for (auto it = definitions.begin(); it != definitions.end(); ++it) {
        // Native instrumentation was stripped; no payload is consumed.
    }
}
} // namespace bsp

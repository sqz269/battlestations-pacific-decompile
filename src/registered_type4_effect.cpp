#include "bsp/registered_type4_effect.hpp"

#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Registered type4 effect lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
void restore_base(NativeRegisteredEffectPrefixStorage& prefix) noexcept {
    volatile auto& actual = prefix;
    actual.table_00 = 0x00d0c88c;
    actual.secondary_table_08 = 0x00d0c888;
    actual.table_00 = 0x00ceb130; // Complete BD30F0; count/borrowed fields untouched.
}
class BaseUnwind final {
public:
    explicit BaseUnwind(NativeRegisteredEffectPrefixStorage& prefix) noexcept : prefix_(prefix) {}
    ~BaseUnwind() { if (armed) restore_base(prefix_); }
    bool armed{true};
private:
    NativeRegisteredEffectPrefixStorage& prefix_;
};
} // namespace

NativeRegisteredType4EffectStorage& construct_registered_type4_effect_008744a0(
    NativeRegisteredType4EffectStorage& event, const void* definition, void* subject,
    LiveEffectEventRegistryBindings bindings) {
    volatile auto& actual = event;
    actual.prefix_00.table_00 = 0x00ceb130;
    event.prefix_00.references_04.store(1, std::memory_order_relaxed);
    actual.prefix_00.secondary_table_08 = 0x00d0c8c0;
    actual.prefix_00.active_0c = 1;
    actual.prefix_00.borrowed_10 = subject;
    actual.prefix_00.borrowed_14 = const_cast<void*>(definition);
    BaseUnwind unwind(event.prefix_00); // DC847C: state0 -> C963A0 -> 858150.
    actual.prefix_00.table_00 = 0x00d0de50;
    actual.prefix_00.secondary_table_08 = 0x00d0de4c;
    actual.prefix_00.type_18 = 4;
    actual.node_34 = nullptr;
    actual.state_1c = 1;
    actual.value_24 = 0.0f;
    actual.value_28 = 0.0f;
    actual.value_30 = 0.0f;
    actual.value_2c = 0.0f;
    actual.owned_38 = nullptr;
    auto* const manager = live_effect_manager_singleton_004d1100(
        bindings.actual_manager_00f8765c, bindings.domain);
    register_live_effect_event_00866a10(*manager, &event,
        bindings.actual_lock_00f87654, bindings.lock_lifetime);
    unwind.armed = false;
    return event;
}

NativeRegisteredType4EffectStorage* create_registered_type4_effect_00868d70(
    const void* definition, void* subject, LiveEffectEventRegistryBindings bindings) {
    void* const raw = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x3c, 0x3c});
    if (!raw) return nullptr;
    auto* const event = ::new (raw) NativeRegisteredType4EffectStorage;
    try { construct_registered_type4_effect_008744a0(*event, definition, subject, bindings); }
    catch (...) {
        event->~NativeRegisteredType4EffectStorage();
        singleton_lifetime_free(raw); // DC7090 -> C951C0, after constructor cleanup.
        throw;
    }
    return event;
}

void destroy_registered_type4_effect_00874540(
    NativeRegisteredType4EffectStorage& event, RegisteredType4EffectLifetimeBindings bindings) {
    volatile auto& actual = event;
    actual.prefix_00.table_00 = 0x00d0de50;
    actual.prefix_00.secondary_table_08 = 0x00d0de4c;
    BaseUnwind unwind(event.prefix_00); // DC84A8: state0 -> C963C0 -> 858150.
    void* const node = actual.node_34;
    if (node) {
        auto* const lifetime = bindings.nodes.find_actual_node(
            static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(node)));
        if (!lifetime)
            throw std::invalid_argument("registered type4 effect requires its actual node lifetime binding");
        unlink_and_release_render_model_00b6dfa0(*lifetime);
        actual.node_34 = nullptr; // Only after current virtual18 returns.
    }
    void* const owned = actual.owned_38; // Reload after node terminal reentry.
    if (owned) {
        release_native_render_actual_owner(bindings.actual_owners, owned);
        actual.owned_38 = nullptr;
    }
    auto* const manager = live_effect_manager_singleton_004d1100(
        bindings.registry.actual_manager_00f8765c, bindings.registry.domain);
    unregister_live_effect_event_00866b00(*manager, &event,
        bindings.registry.actual_lock_00f87654, bindings.registry.lock_lifetime);
    unwind.armed = false;
    restore_base(event.prefix_00);
}

NativeRegisteredType4EffectStorage* delete_registered_type4_effect_00874610(
    NativeRegisteredType4EffectStorage* event, std::uint32_t flags,
    RegisteredType4EffectLifetimeBindings bindings) {
    destroy_registered_type4_effect_00874540(*event, bindings);
    if ((flags & 1u) != 0) singleton_lifetime_free(event);
    return event;
}
} // namespace bsp

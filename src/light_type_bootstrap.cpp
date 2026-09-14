#include "bsp/light_type_bootstrap.hpp"

#include <cstddef>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native type bootstrap requires MSVC Win32 pointer widths.
#endif

namespace bsp {
static_assert(sizeof(TypeIdCounterStorage) == 8);
static_assert(offsetof(TypeIdCounterStorage, next_id_04) == 4);
static_assert(sizeof(RootTypeDescriptor) == 8);
static_assert(sizeof(NodeTypeDescriptor) == 12);
static_assert(sizeof(LightTypeDescriptor) == 16);
static_assert(sizeof(DirectionalLightTypeDescriptor) == 20);

TypeIdCounterLifetime::TypeIdCounterLifetime(SoundLifetimeAccess lifetime,
    TypeIdCounterStorage* volatile& actual_global) noexcept
    : lifetime_(lifetime), global_0109db7c_(actual_global) {}

TypeIdCounterStorage* TypeIdCounterLifetime::get_006fac20() {
    auto* existing = global_0109db7c_;
    if (existing) return existing; // Fast path returns its first global load.
    {
        CapturedSoundLifetimeSection guard(lifetime_);
        if (!global_0109db7c_) {
            void* raw = singleton_lifetime_allocate(
                {SingletonAllocationKind::object, 8, sizeof(TypeIdCounterStorage)});
            auto* allocated = raw ? ::new (raw) TypeIdCounterStorage : nullptr;
            if (allocated) {
                volatile auto* initialized = allocated;
                initialized->native_vtable_00 = 0x00cfb6c4u;
                initialized->next_id_04 = 0;
            }
            global_0109db7c_ = allocated;
            // Native calls the manager getter a second time, then reloads the
            // published slot for registration, even if allocation returned null.
            auto registration_manager = lifetime_.get_manager_00415350();
            registration_manager->register_object(global_0109db7c_);
        }
    } // Release the originally captured section before the final slot reload.
    return global_0109db7c_;
}

TypeIdCounterStorage* TypeIdCounterLifetime::deleting_destructor_006fad40(
    TypeIdCounterStorage* owner, std::uint32_t flags) noexcept {
    auto* original_address = owner;
    global_0109db7c_ = nullptr;
    static_cast<volatile TypeIdCounterStorage*>(owner)->native_vtable_00 = 0x00ce3818u;
    if ((flags & 1u) != 0) {
        owner->~TypeIdCounterStorage();
        singleton_lifetime_free(owner);
    }
    return original_address;
}

LightTypeBootstrap::LightTypeBootstrap(TypeIdCounterLifetime& counter,
    LightTypeBootstrapStorage storage) noexcept : counter_(counter), storage_(storage) {}

std::uint32_t LightTypeBootstrap::consume_type_id() {
    volatile auto* owner = counter_.get_006fac20();
    const auto result = owner->next_id_04;
    owner->next_id_04 = result + 1u;
    return result;
}

void LightTypeBootstrap::initialize_root_00bea780(volatile RootTypeDescriptor& target) {
    if (storage_.root_guard_0109db80 == 0) {
        storage_.root_guard_0109db80 = 1;
        target.own_id = consume_type_id();
        target.native_name_address = 0x00d68bbcu;
    }
}

void LightTypeBootstrap::initialize_node_00b6f110(volatile NodeTypeDescriptor& target) {
    if (storage_.node_guard_0108ff54 == 0) {
        storage_.node_guard_0108ff54 = 1;
        target.native_name_address = 0x00d62c7cu;
        initialize_root_00bea780(storage_.root_0109db84);
        target.root_id = storage_.root_0109db84.own_id;
        target.own_id = consume_type_id();
    }
}

void LightTypeBootstrap::initialize_light_after_guard_check() {
    storage_.light_guard_0109010d = 1;
    storage_.light_0109018c.native_name_address = 0x00d62f14u;
    initialize_node_00b6f110(storage_.node_0108ff90);
    // Both parent values are captured before either destination store.
    const auto node_id = storage_.node_0108ff90.own_id;
    const auto root_id = storage_.node_0108ff90.root_id;
    storage_.light_0109018c.node_id = node_id;
    storage_.light_0109018c.root_id = root_id;
    storage_.light_0109018c.own_id = consume_type_id();
}

void LightTypeBootstrap::initialize_light_00cd80a0() {
    if (storage_.light_guard_0109010d == 0) initialize_light_after_guard_check();
}

void LightTypeBootstrap::initialize_directional_00cd80f0() {
    if (storage_.directional_guard_0109010e == 0) {
        // 00CD80FD tests the light guard BEFORE setting the directional guard
        // and name. Preserve that captured decision rather than calling the
        // standalone initializer, which would read the light guard later.
        const bool initialize_light = storage_.light_guard_0109010d == 0;
        storage_.directional_guard_0109010e = 1;
        storage_.directional_0109019c.native_name_address = 0x00d62f1cu;
        if (initialize_light) initialize_light_after_guard_check();
        const auto light_id = storage_.light_0109018c.own_id;
        const auto node_id = storage_.light_0109018c.node_id;
        const auto root_id = storage_.light_0109018c.root_id;
        storage_.directional_0109019c.light_id = light_id;
        storage_.directional_0109019c.node_id = node_id;
        storage_.directional_0109019c.root_id = root_id;
        storage_.directional_0109019c.own_id = consume_type_id();
    }
}

bool LightTypeBootstrap::node_is_type_00b6f570(std::uint32_t token) const noexcept {
    return storage_.node_0108ff90.own_id == token || storage_.node_0108ff90.root_id == token;
}
bool LightTypeBootstrap::light_is_type_00b7c580(std::uint32_t token) const noexcept {
    return storage_.light_0109018c.own_id == token || storage_.light_0109018c.node_id == token ||
        storage_.light_0109018c.root_id == token;
}
bool LightTypeBootstrap::directional_is_type_00b7c6d0(std::uint32_t token) const noexcept {
    return storage_.directional_0109019c.own_id == token ||
        storage_.directional_0109019c.light_id == token ||
        storage_.directional_0109019c.node_id == token ||
        storage_.directional_0109019c.root_id == token;
}
} // namespace bsp

// Focused host fixture: cache publication survives relocation, replacement,
// and shrink after the registry's real constructor. This is not a game run.
#include "bsp/game_host_vehicle_class.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include "bsp/native_singleton_vector_leaves.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdio>
#include <stdexcept>

int main() {
    bsp::NativeStringPoolStorage* volatile strings = nullptr;
    volatile std::uint32_t returns_disabled = 0;
    void* volatile manager = nullptr;
    bsp::NativeStringRawPoolContext context{strings, returns_disabled, manager};
    bsp::NativeSingletonDeletionBindings deletions{};
    bsp::VehicleClassDescriptor first, second;
    auto require = [](bool value) {
        if (!value) throw std::runtime_error("vehicle registry cache publication regression");
    };
    {
        bsp::game::GameVehicleClassDescriptorCache cache(context, deletions);
        require(manager != nullptr && deletions.vehicle_class_registry != nullptr);
        void* const registered = deletions.vehicle_class_registry->registered_owner;
        require(registered != nullptr &&
            bsp::count_native_singleton_slots_00bcf910(manager, nullptr) == 1);
        require(cache.cache_size() == 0 && cache.cached(0) == nullptr);
        first.ref_count = 2;
        cache.store_cached(0, &first);
        cache.resize_cache(2048); // Forces growth beyond the initial single slot.
        require(cache.cache_size() == 2048 && cache.cached(0) == &first);
        require(cache.cached(2047) == nullptr && cache.cached(2048) == nullptr);
        cache.store_cached(0, &second);
        require(first.ref_count == 1 && cache.cached(0) == &second);
        cache.resize_cache(0);
        require(second.ref_count == 0 && cache.cache_size() == 0 && cache.cached(0) == nullptr);
        cache.store_cached(7, &first); // Reuse retained capacity after shrink.
        require(cache.cache_size() == 8 && cache.cached(7) == &first && cache.cached(0) == nullptr);
        cache.resize_cache(0);
        require(deletions.vehicle_class_registry->registered_owner == registered &&
            bsp::count_native_singleton_slots_00bcf910(manager, nullptr) == 1);
    }
    // Cache teardown unregisters its receiver before these stable bindings
    // disappear. Drain the remaining null slot and retire the fixture manager.
    require(deletions.vehicle_class_registry == nullptr && manager != nullptr);
    bsp::destroy_native_singleton_manager_00bd0400(manager, deletions);
    bsp::singleton_lifetime_free(manager);
    manager = nullptr;
    // Cache-only operations do not acquire the fixture's string pool.
    require(strings == nullptr && manager == nullptr && returns_disabled == 0);
    std::puts("c797 vehicle registry cache relocation/replacement/shrink: PASS");
}

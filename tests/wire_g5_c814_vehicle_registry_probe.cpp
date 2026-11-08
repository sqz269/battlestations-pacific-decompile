#include "bsp/game_host_vehicle_class.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include "bsp/native_singleton_vector_leaves.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>
#include <cstdio>

int main() {
    bsp::NativeStringPoolStorage* volatile strings = nullptr;
    volatile std::uint32_t returns_disabled = 0;
    void* volatile manager = nullptr;
    bsp::NativeStringRawPoolContext raw_strings{strings, returns_disabled, manager};
    bsp::NativeSingletonDeletionBindings deletions{};
    bsp::VehicleClassDescriptor descriptor{};
    descriptor.ref_count = 1;
    {
        bsp::game::GameVehicleClassDescriptorCache cache(raw_strings, deletions);
        auto* const binding = deletions.vehicle_class_registry;
        if (binding == nullptr || binding->registered_owner == nullptr || manager == nullptr)
            return 1;
        auto* const words = static_cast<std::uint32_t*>(binding->registered_owner);
        if (words[0] != 0x00ce417cu || cache.cache_size() != 0 ||
            bsp::count_native_singleton_slots_00bcf910(manager, nullptr) != 1)
            return 2;

        // The getter's receiver must be the actual cache header, not a second
        // object used only to obtain a compiled call. Growth and store below
        // must be observable through that registered receiver.
        cache.store_cached(2, &descriptor);
        auto* const slots = reinterpret_cast<void**>(words[1]);
        if (words[2] != 3 || slots[2] != &descriptor || cache.cached(2) != &descriptor)
            return 3;

        // Exercise the risk introduced by registration: shared drain occurs
        // while the C++ cache/vector still live. Its later destructor must
        // neither touch freed registry storage nor release the slot twice.
        bsp::destroy_native_singleton_manager_00bd0400(manager, deletions);
        if (binding->registered_owner != nullptr || descriptor.ref_count != 0 ||
            bsp::count_native_singleton_slots_00bcf910(manager, nullptr) != 0)
            return 4;
        bsp::singleton_lifetime_free(manager);
        manager = nullptr;
    }
    if (deletions.vehicle_class_registry != nullptr || descriptor.ref_count != 0 || strings != nullptr)
        return 5;
    std::puts("c814 vehicle registry: real cache header, shared registration/drain, and later teardown passed");
    return 0;
}

#pragma once

#include "bsp/allocator_list.hpp"
#include "bsp/light_type_bootstrap.hpp"

#include <cstdint>

namespace bsp {

// Borrowed actual storage. Neither binding nor construction resets these words.
struct ModelBaseTypeStorage {
    volatile std::uint8_t& guard_01090031;
    volatile std::uint32_t& own_id_01090044;
    volatile std::uint32_t& node_id_01090048;
    volatile std::uint32_t& root_id_0109004c;
    volatile std::uint32_t& native_name_01090050;
};

class ModelBaseTypeBootstrap final {
public:
    // counter must be the SAME lifetime instance passed to shared_types.
    ModelBaseTypeBootstrap(TypeIdCounterLifetime& counter,
        LightTypeBootstrap& shared_types, ModelBaseTypeStorage storage) noexcept;
    // Complete CD7EB0..CD7EFE; original no-argument RET, no return contract.
    void initialize_static_type_descriptor_00cd7eb0();

private:
    TypeIdCounterLifetime& counter_;
    LightTypeBootstrap& shared_types_;
    ModelBaseTypeStorage storage_;
};

// Host binding for the separate actual38h pool at0109008C. Raw storage and the
// SAME E188B4 domain must survive registration and its process-exit callback.
// Bind once before startup; identity is fixed through process exit. Null or
// different storage/domain is rejected; rebinding the same pair is a no-op.
// This does not construct or publish the pool.
void bind_static_model_base_node_pool_0109008c(void* actual_pool,
    AllocatorListDomain& actual_list);

// Complete CD7F20..CD7F35: native no-arg RET; returns actual atexit status.
int initialize_static_model_base_node_pool_00cd7f20();
// Complete CE0E60..CE0E69: native no-arg tail jump to B6E3D0.
void destroy_static_model_base_node_pool_00ce0e60() noexcept;

} // namespace bsp

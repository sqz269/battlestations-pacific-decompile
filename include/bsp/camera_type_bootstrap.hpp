#pragma once

#include "bsp/light_type_bootstrap.hpp"

namespace bsp {

// The three live tokens at 0108FFA0/A4/A8 followed by native name word 0108FFAC.
// Binding this POD to existing storage must not initialize or reset its fields.
struct CameraTypeDescriptor {
    std::uint32_t own_id;
    std::uint32_t node_id;
    std::uint32_t root_id;
    std::uint32_t native_name_address;
};

struct CameraTypeBootstrapStorage {
    volatile std::uint8_t& guard_0108ff9c;
    volatile CameraTypeDescriptor& camera_0108ffa0;
};

class CameraTypeBootstrap final {
public:
    // counter must be the same object used by shared_types, including its actual
    // 0109DB7C global slot and shared SingletonLifetimeDomain.
    // shared_types supplies the actual node/root descriptors and their guards.
    CameraTypeBootstrap(TypeIdCounterLifetime& counter,
        LightTypeBootstrap& shared_types, CameraTypeBootstrapStorage) noexcept;

    // These original virtual leaves ignore ECX and never initialize a type.
    std::uint32_t type_id_00b6fb60() const noexcept;
    bool is_type_00b71ce0(std::uint32_t token) const noexcept;

    void initialize_static_00cd7d80();
    // Original ECX may designate another descriptor; the guard is process-wide.
    void initialize_00b719e0(volatile CameraTypeDescriptor& target);
    volatile CameraTypeDescriptor* construct_00b71a30(
        volatile CameraTypeDescriptor& target);
    CameraTypeBootstrapStorage storage() const noexcept { return storage_; }

private:
    std::uint32_t consume_type_id();
    void initialize_target(volatile CameraTypeDescriptor& target);
    TypeIdCounterLifetime& counter_;
    LightTypeBootstrap& shared_types_;
    CameraTypeBootstrapStorage storage_;
};

} // namespace bsp

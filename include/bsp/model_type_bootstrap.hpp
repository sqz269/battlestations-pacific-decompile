#pragma once

#include "bsp/light_type_bootstrap.hpp"

#include <cstdint>

namespace bsp {

// Actual four DWORDs at 01090034. Binding storage must not initialize them.
// The name word is a native address, not a host string pointer.
struct ModelTypeDescriptor {
    std::uint32_t own_id;
    std::uint32_t node_id;
    std::uint32_t root_id;
    std::uint32_t native_name_address;
};

struct ModelTypeBootstrapStorage {
    volatile std::uint8_t& guard_01090030;
    volatile ModelTypeDescriptor& model_01090034;
};

class ModelTypeBootstrap final {
public:
    // Pass the same counter instance already used by the shared root/node types.
    ModelTypeBootstrap(TypeIdCounterLifetime&, LightTypeBootstrap&,
        ModelTypeBootstrapStorage) noexcept;

    // Native leaves read the current canonical descriptor, including before
    // initialization. Neither the guard nor a copied token array gates them.
    std::uint32_t type_id_00b74330() const noexcept;
    std::uint32_t type_name_address_00b74340() const noexcept;
    bool is_type_006ef860(std::uint32_t token) const noexcept;

    void initialize_static_00cd7e60();
    // The lazy entry accepts an ECX target but shares the one process guard.
    void initialize_00b74f90(volatile ModelTypeDescriptor& target);
    ModelTypeBootstrapStorage storage() const noexcept { return storage_; }

private:
    std::uint32_t consume_type_id();
    TypeIdCounterLifetime& counter_;
    LightTypeBootstrap& shared_types_;
    ModelTypeBootstrapStorage storage_;
};

} // namespace bsp

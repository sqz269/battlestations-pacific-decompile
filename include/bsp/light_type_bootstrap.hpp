#pragma once

#include "bsp/singleton_lifetime.hpp"
#include "bsp/sound_lifetime_access.hpp"

#include <cstdint>

namespace bsp {

// Actual eight-byte owner published at 0109DB7C, shared by every type family.
// These PODs deliberately have no member initializers: binding live storage
// must not reset either the monotonic counter or the process descriptor guards.
struct TypeIdCounterStorage {
    std::uint32_t native_vtable_00;
    std::uint32_t next_id_04;
};

class TypeIdCounterLifetime final {
public:
    TypeIdCounterLifetime(SoundLifetimeAccess,
        TypeIdCounterStorage* volatile& actual_global_0109db7c) noexcept;
    TypeIdCounterStorage* get_006fac20();
    // Dispatch this from the shared lifetime manager for vtable 00CFB6C4.
    // Clears the global unconditionally; does not unregister or reset guards.
    TypeIdCounterStorage* deleting_destructor_006fad40(
        TypeIdCounterStorage*, std::uint32_t flags) noexcept;

private:
    SoundLifetimeAccess lifetime_;
    TypeIdCounterStorage* volatile& global_0109db7c_;
};

struct RootTypeDescriptor {
    std::uint32_t own_id;
    std::uint32_t native_name_address;
};
struct NodeTypeDescriptor {
    std::uint32_t own_id;
    std::uint32_t root_id;
    std::uint32_t native_name_address;
};
struct LightTypeDescriptor {
    std::uint32_t own_id;
    std::uint32_t node_id;
    std::uint32_t root_id;
    std::uint32_t native_name_address;
};
struct DirectionalLightTypeDescriptor {
    std::uint32_t own_id;
    std::uint32_t light_id;
    std::uint32_t node_id;
    std::uint32_t root_id;
    std::uint32_t native_name_address;
};

// Every reference identifies actual process storage. Name words retain native
// addresses as evidence, not host C-string pointers or dispatchable addresses.
struct LightTypeBootstrapStorage {
    volatile std::uint8_t& root_guard_0109db80;
    volatile RootTypeDescriptor& root_0109db84;
    volatile std::uint8_t& node_guard_0108ff54;
    volatile NodeTypeDescriptor& node_0108ff90;
    volatile std::uint8_t& light_guard_0109010d;
    volatile LightTypeDescriptor& light_0109018c;
    volatile std::uint8_t& directional_guard_0109010e;
    volatile DirectionalLightTypeDescriptor& directional_0109019c;
};

class LightTypeBootstrap final {
public:
    LightTypeBootstrap(TypeIdCounterLifetime&, LightTypeBootstrapStorage) noexcept;
    // Native ECX may target another descriptor, but its guard is process-wide.
    void initialize_root_00bea780(volatile RootTypeDescriptor& target);
    void initialize_node_00b6f110(volatile NodeTypeDescriptor& target);
    void initialize_light_00cd80a0();
    void initialize_directional_00cd80f0();

    // Native leaves read current tokens in order and never run initializers.
    bool node_is_type_00b6f570(std::uint32_t token) const noexcept;
    bool light_is_type_00b7c580(std::uint32_t token) const noexcept;
    bool directional_is_type_00b7c6d0(std::uint32_t token) const noexcept;
    LightTypeBootstrapStorage storage() const noexcept { return storage_; }

private:
    std::uint32_t consume_type_id();
    void initialize_light_after_guard_check();
    TypeIdCounterLifetime& counter_;
    LightTypeBootstrapStorage storage_;
};

} // namespace bsp

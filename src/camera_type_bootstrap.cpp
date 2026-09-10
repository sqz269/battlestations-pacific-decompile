#include "bsp/camera_type_bootstrap.hpp"

#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native camera type bootstrap requires MSVC Win32 pointer widths.
#endif

namespace bsp {

static_assert(sizeof(CameraTypeDescriptor) == 16);
static_assert(offsetof(CameraTypeDescriptor, own_id) == 0);
static_assert(offsetof(CameraTypeDescriptor, node_id) == 4);
static_assert(offsetof(CameraTypeDescriptor, root_id) == 8);
static_assert(offsetof(CameraTypeDescriptor, native_name_address) == 12);

CameraTypeBootstrap::CameraTypeBootstrap(TypeIdCounterLifetime& counter,
    LightTypeBootstrap& shared_types, CameraTypeBootstrapStorage storage) noexcept
    : counter_(counter), shared_types_(shared_types), storage_(storage) {}

std::uint32_t CameraTypeBootstrap::type_id_00b6fb60() const noexcept {
    return storage_.camera_0108ffa0.own_id;
}

bool CameraTypeBootstrap::is_type_00b71ce0(std::uint32_t token) const noexcept {
    return storage_.camera_0108ffa0.own_id == token ||
        storage_.camera_0108ffa0.node_id == token ||
        storage_.camera_0108ffa0.root_id == token;
}

std::uint32_t CameraTypeBootstrap::consume_type_id() {
    volatile auto* counter = counter_.get_006fac20();
    const auto result = counter->next_id_04;
    counter->next_id_04 = result + 1u;
    return result;
}

void CameraTypeBootstrap::initialize_static_00cd7d80() {
    if (storage_.guard_0108ff9c == 0) {
        storage_.guard_0108ff9c = 1;
        storage_.camera_0108ffa0.native_name_address = 0x00d62ce4u;
        auto& node = shared_types_.storage().node_0108ff90;
        shared_types_.initialize_node_00b6f110(node);
        // Unlike the lazy entries, both source values precede either store.
        const auto node_id = node.own_id;
        const auto root_id = node.root_id;
        storage_.camera_0108ffa0.node_id = node_id;
        storage_.camera_0108ffa0.root_id = root_id;
        storage_.camera_0108ffa0.own_id = consume_type_id();
    }
}

void CameraTypeBootstrap::initialize_target(volatile CameraTypeDescriptor& target) {
    if (storage_.guard_0108ff9c == 0) {
        storage_.guard_0108ff9c = 1;
        target.native_name_address = 0x00d62ce4u;
        auto& node = shared_types_.storage().node_0108ff90;
        shared_types_.initialize_node_00b6f110(node);
        target.node_id = node.own_id;
        target.root_id = node.root_id;
        target.own_id = consume_type_id();
    }
}

void CameraTypeBootstrap::initialize_00b719e0(volatile CameraTypeDescriptor& target) {
    initialize_target(target);
}

volatile CameraTypeDescriptor* CameraTypeBootstrap::construct_00b71a30(
    volatile CameraTypeDescriptor& target) {
    initialize_target(target);
    return &target;
}

} // namespace bsp

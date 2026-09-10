#include "bsp/model_type_bootstrap.hpp"

#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native model type bootstrap requires MSVC Win32 pointer widths.
#endif

namespace bsp {

static_assert(sizeof(ModelTypeDescriptor) == 16);
static_assert(offsetof(ModelTypeDescriptor, own_id) == 0);
static_assert(offsetof(ModelTypeDescriptor, node_id) == 4);
static_assert(offsetof(ModelTypeDescriptor, root_id) == 8);
static_assert(offsetof(ModelTypeDescriptor, native_name_address) == 12);

ModelTypeBootstrap::ModelTypeBootstrap(TypeIdCounterLifetime& counter,
    LightTypeBootstrap& shared_types, ModelTypeBootstrapStorage storage) noexcept
    : counter_(counter), shared_types_(shared_types), storage_(storage) {}

std::uint32_t ModelTypeBootstrap::type_id_00b74330() const noexcept {
    return storage_.model_01090034.own_id;
}

std::uint32_t ModelTypeBootstrap::type_name_address_00b74340() const noexcept {
    return storage_.model_01090034.native_name_address;
}

bool ModelTypeBootstrap::is_type_006ef860(std::uint32_t token) const noexcept {
    return storage_.model_01090034.own_id == token ||
        storage_.model_01090034.node_id == token ||
        storage_.model_01090034.root_id == token;
}

std::uint32_t ModelTypeBootstrap::consume_type_id() {
    volatile auto* counter = counter_.get_006fac20();
    const auto result = counter->next_id_04;
    counter->next_id_04 = result + 1u;
    return result;
}

void ModelTypeBootstrap::initialize_static_00cd7e60() {
    if (storage_.guard_01090030 == 0) {
        storage_.guard_01090030 = 1;
        storage_.model_01090034.native_name_address = 0x00d62dd4u;
        auto& node = shared_types_.storage().node_0108ff90;
        shared_types_.initialize_node_00b6f110(node);
        // 00CD7E84..00CD7E94 captures both parents before either store.
        const auto node_id = node.own_id;
        const auto root_id = node.root_id;
        storage_.model_01090034.node_id = node_id;
        storage_.model_01090034.root_id = root_id;
        storage_.model_01090034.own_id = consume_type_id();
    }
}

void ModelTypeBootstrap::initialize_00b74f90(volatile ModelTypeDescriptor& target) {
    if (storage_.guard_01090030 == 0) {
        storage_.guard_01090030 = 1;
        target.native_name_address = 0x00d62dd4u;
        auto& node = shared_types_.storage().node_0108ff90;
        shared_types_.initialize_node_00b6f110(node);
        // Unlike the static entry, each source load precedes its own store.
        target.node_id = node.own_id;
        target.root_id = node.root_id;
        target.own_id = consume_type_id();
    }
}

} // namespace bsp

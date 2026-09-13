#include "bsp/native_model_base_bootstrap.hpp"
#include "bsp/native_node_pool_owner.hpp"

#include <cstdlib>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native model-base bootstrap requires MSVC Win32.
#endif

namespace bsp {

ModelBaseTypeBootstrap::ModelBaseTypeBootstrap(TypeIdCounterLifetime& counter,
    LightTypeBootstrap& shared_types, ModelBaseTypeStorage storage) noexcept
    : counter_(counter), shared_types_(shared_types), storage_(storage) {}

void ModelBaseTypeBootstrap::initialize_static_type_descriptor_00cd7eb0() {
    if (storage_.guard_01090031 != 0) return;
    storage_.guard_01090031 = 1;
    storage_.native_name_01090050 = 0x00d62de0u;
    const auto shared = shared_types_.storage();
    shared_types_.initialize_node_00b6f110(shared.node_0108ff90);
    const auto node_id = shared.node_0108ff90.own_id;
    const auto root_id = shared.node_0108ff90.root_id;
    storage_.node_id_01090048 = node_id;
    storage_.root_id_0109004c = root_id;
    volatile auto* const owner = counter_.get_006fac20();
    const auto old_id = owner->next_id_04;
    owner->next_id_04 = old_id + 1u;
    storage_.own_id_01090044 = old_id;
}

namespace {
constexpr std::uint32_t pool_profile = 0x00d62c78u;
void invoke_trim(void* pool) { trim_native_node_pool_00b6ea60(pool); }
void* actual_model_base_pool;
AllocatorListDomain* actual_model_base_list;
} // namespace

void bind_static_model_base_node_pool_0109008c(void* pool,
    AllocatorListDomain& list) {
    if (!pool) throw std::invalid_argument("model-base pool storage is null");
    if (actual_model_base_pool) {
        if (actual_model_base_pool != pool || actual_model_base_list != &list)
            throw std::logic_error("model-base pool binding cannot change");
        return;
    }
    list.bind_virtual0(*static_cast<AllocatorListElement*>(pool),
        {pool_profile, 0x00b6ea60u, pool, &invoke_trim});
    actual_model_base_pool = pool;
    actual_model_base_list = &list;
}

int initialize_static_model_base_node_pool_00cd7f20() {
    initialize_native_node_pool_00b6e980(actual_model_base_pool, *actual_model_base_list);
    return std::atexit(&destroy_static_model_base_node_pool_00ce0e60);
}

void destroy_static_model_base_node_pool_00ce0e60() noexcept {
    destroy_native_node_pool_00b6e3d0(actual_model_base_pool, *actual_model_base_list);
    actual_model_base_list->unbind_virtual0(
        *static_cast<AllocatorListElement*>(actual_model_base_pool));
}

} // namespace bsp

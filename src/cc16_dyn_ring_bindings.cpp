#include "bsp/cc16_dyn_ring_bindings.hpp"

#include "bsp/cc16_dyn_ambient_math.hpp"
#include "bsp/cc16_dyn_ring_predicates.hpp"
#include "bsp/cc16_dyn_ring_prepare.hpp"
#include "bsp/cc16_dyn_ring_query.hpp"

namespace bsp::cc16_dyn_ring_bindings {
namespace {

class AmbientBarycentricCalls final : public cc16_dyn_ring_query::DynRingQueryCalls {
public:
    cc16_dyn_ring_query::DynRingBarycentricWeights barycentric_00c37860(
        cc16_dyn_ring_predicates::DynRingPoint2 first,
        cc16_dyn_ring_predicates::DynRingPoint2 second,
        cc16_dyn_ring_predicates::DynRingPoint2 third,
        cc16_dyn_ring_predicates::DynRingPoint2 point) override {
        cc16_dyn_ring_query::DynRingBarycentricWeights weights;
        // The native query's three outputs are distinct caller-frame slots,
        // outside the barycentric helper's private argument/scratch area.
        cc16_dyn_ambient_math::barycentric_2d_ambient_00c37860(
            &weights.u, &weights.v, &weights.w,
            first.x_00, first.y_04, second.x_00, second.y_04,
            third.x_00, third.y_04, point.x_00, point.y_04);
        return weights;
    }
};

std::uint32_t run_query(const cc16_dyn_ring_owner::DynRingOwner& owner,
                       const cc16_dyn_ring_predicates::DynRingTriangleQuery& query) {
    AmbientBarycentricCalls math;
    return cc16_dyn_ring_query::triangle_ring_clear_00c410e0(owner, query, math);
}

class PredicateQueryCalls final : public cc16_dyn_ring_predicates::DynRingPredicateCalls {
public:
    std::uint32_t triangle_ring_clear_00c410e0(
        const cc16_dyn_ring_owner::DynRingOwner& owner,
        const cc16_dyn_ring_predicates::DynRingTriangleQuery& query) override {
        return run_query(owner, query);
    }
};

std::uint32_t prepare_query(void*, void* owner,
                           const cc16_dyn_ring_prepare::TriangleQuery& inputs) {
    // Copy only the already captured register/stack values. Do not recompute
    // them from owner fields or substitute a geometry snapshot.
    const cc16_dyn_ring_predicates::DynRingTriangleQuery query{
        inputs.walk_start_eax, inputs.triangle2_ebx, inputs.stop_stack,
        inputs.triangle0_stack, inputs.triangle1_stack};
    return run_query(*static_cast<const cc16_dyn_ring_owner::DynRingOwner*>(owner), query);
}

class OwnerCalls final : public cc16_dyn_ring_owner::DynRingOwnerCalls {
public:
    void prepare_chain_00c4d370(cc16_dyn_ring_owner::DynRingOwner& owner,
                              const cc15_dyn_live::DynLiveCrt& crt) override {
        // The callback is stateless: context is unused, with no global frame.
        const cc16_dyn_ring_prepare::QueryProvider query{nullptr, &prepare_query};
        cc16_dyn_ring_prepare::prepare_chain_00c4d370(&owner, crt, query);
    }

    std::uint32_t test_previous_00c41230(cc16_dyn_ring_owner::DynRingOwner& owner) override {
        PredicateQueryCalls query;
        return cc16_dyn_ring_predicates::test_previous_00c41230(owner, query);
    }

    std::uint32_t test_next_00c41190(cc16_dyn_ring_owner::DynRingOwner& owner) override {
        PredicateQueryCalls query;
        return cc16_dyn_ring_predicates::test_next_00c41190(owner, query);
    }
};

}  // namespace

cc16_dyn_ring_owner::DynRingOwner* construct_dyn_ring_owner_bound_00c4d5a0(
    cc16_dyn_ring_owner::DynRingOwner& destination,
    const cc16_dyn_ring_init::DynRingOwnerCount& input_owner,
    const cc15_dyn_live::DynLiveCrt& crt) {
    OwnerCalls calls;
    return cc16_dyn_ring_owner::construct_dyn_ring_owner_00c4d5a0(
        destination, input_owner, crt, calls);
}

}  // namespace bsp::cc16_dyn_ring_bindings

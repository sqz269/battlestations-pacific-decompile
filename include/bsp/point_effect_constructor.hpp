#pragma once

#include "bsp/native_plain_node.hpp"
#include "bsp/point_effect_matrix_setters.hpp"
#include "bsp/point_effect_release.hpp"
#include "bsp/point_effect_row_ownership.hpp"

namespace bsp {

class PointEffectNodeCompanions {
public:
    virtual ~PointEffectNodeCompanions() = default;
    // Required HOST association step for this exact already-constructed raw
    // node. Install its stable NativeNodeBinding in nodes.scenes and a real
    // NativePlainNodeReference in the existing node terminal-owner lookup.
    // Use the supplied SAME destruction runtime/name allocator/physical pool.
    // No retain, second node, second native count or native initialization.
    // Own these companions until real node terminal retirement. If metadata
    // allocation fails, clean only partial host metadata; do not destroy/return
    // the successfully constructed native node (the outer map has no such step).
    virtual NativePlainNodeReference& bind_actual_constructed_node(
        NativeNodeStorage&, NativeNodeDestructionRuntime&, NativeStringStorage&,
        void* actual_pool_0108ff58) = 0;
};

struct PointEffectConstructorBindings {
    NativeNodeDestructionRuntime& nodes;
    NativeRenderActualOwners& node_terminal_owners;
    NativeStringStorage& strings;
    void* actual_node_pool_0108ff58;
    PointEffectNodeCompanions& node_companions;
    PointEffectInstanceLinks& links;
    PointEffectRowRuntime& rows;
    PointEffectReleaseRuntime& effects;
    PointEffectInstanceCounters counters;
    const volatile std::uint32_t* actual_effect_table_00d0d3ec;
    EffectManager* volatile& actual_insertion_lock_00f87650;
    // Must use the SAME actual singleton domain as effects. This is the
    // separate insertion lock; it must not alias the deletion-lock publication.
    EffectManagerLifetimeAccess& insertion_lifetime;
};

// Complete807 bytes8680B0..8683D6, including RET1C at8683D4. Native ECX raw114h;
// seven stack DWORDs: CONSUMED
// template, parent, third word, matrix pointer, transform lowbyte, option lowbyte,
// tail word. EAX same raw114h, RET1C. This C++ interface returns its canonical
// companion (storage() is that same raw address) and receives explicit bindings.
// captured_template_name_1c MUST borrow the original incoming template's actual
// header, not current effect+84 or a pre-allocation value snapshot. Valid native
// template/source/owner spans and nonnull constructed node are preconditions.
//
// Native body: prefix/template retain/counters; actual node allocation/ctor;
// publish/retain node and root-before-parent registration; world/relative setter;
// world cache; actual row stage; actual4D1100 getter/867500 insertion; release
// incoming consumed argument last. Factory virtual18 remains the REQUIRED real
// current row implementation, with one owned result or null, not a stub.
//
// Native state5 cleanup is parent, template, auxiliary array, entry array, base;
// state6 raw-node cleanup is provided by the allocation/ctor stage. Row state7
// is inside the existing nonthrowing retained-assignment domain. Argument state0
// releases LAST. NO successful-node+110 cleanup, counter rollback, physical
// effect free or insertion rollback is invented. Caller owns raw effect on throw.
NativePointEffectReference& construct_point_effect_instance_008680b0(void* raw114h,
    RenderCommandReference* consumed_template, CameraTransform* parent,
    std::uint32_t third_word, const CameraMatrix& original_matrix,
    std::uint8_t transform_byte, std::uint8_t option_byte, std::uint32_t tail_word,
    const NativeString& captured_template_name_1c, PointEffectConstructorBindings&);

// New C++ ABI/host associations. If callbacks invalidate storage still needed
// by native cleanup, that native precondition violation is not made safe here.
// Native EH, complete application row factories and gameplay remain unvalidated.
} // namespace bsp

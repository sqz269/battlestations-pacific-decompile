#pragma once
#include "bsp/native_model_storage_dispatch.hpp"
#include "bsp/native_node_parenting_storage.hpp"
#include "bsp/model_type_bootstrap.hpp"
#include <cstddef>

namespace bsp {
class NativeNodeDestructionRuntime;
namespace game { class GameNativeReadOnlyData; class GameNativeTypeStorage; }

struct NativeModelStorageNodeCall {
    enum class Kind : std::uint8_t { none, type_0c, attachment_1c, scene_50, scene_54, scene_other };
    using Phase = NativeModelStorageDispatchCallAcquired::Phase;
    Kind kind{Kind::none};
    Phase phase{Phase::fresh};
    void* receiver{};
    std::uint32_t target{}, admitted_profile{}, first_argument{}, second_argument{};
    std::uint8_t result_al{};
    bool body_entered{}, body_returned{};
    NativeNodeAttachmentFrame attachment_frame{};
    NativeNodeParentingAcquired attachment;
    NativeNodeSceneAttachmentFrame scene_frame{};
    NativeNodeSceneAttachmentAcquired scene;
};
struct NativeModelStorageNodeDispatchAcquired {
    bool bound{}, poisoned{}, entry_failed{}, capacity_failed{};
    std::size_t used{}; // handed-out prefix; even completed calls stay retained
    NativeModelStorageNodeCall::Kind failed_kind{NativeModelStorageNodeCall::Kind::none};
    std::uint32_t failed_target{};
    void* failed_receiver{};
};
struct NativeModelStorageNodeDispatchContext {
    NativeModelStorageDispatchContext& record; // SAME e769 / Text85 Model record
    NativeModelStorageDispatch& tree_dispatch;
    NativeNodeParentingContext& parenting;
    NativeNodeBaseDestructionContext& shared_node; // genuine renderer scene-domain borrow
    NativeNodeDestructionRuntime& base_nodes; // SAME Text base/glyph runtime
};

// One private DEFAULT internal188 Model's genuine node/scene/type providers.
// No native owner, second Model record, type initialization or logical scene.
class NativeModelStorageNodeDispatch final : public NativeNodeParentingDispatch,
    public NativeNodeSceneChildDispatch, public NativeSceneRegistrationDerivedTypes {
public:
    // Pure borrowed metadata. The ModelTypeBootstrap member is only a facade
    // over actual type cells, using the SAME VFS counter/common bootstrap.
    NativeModelStorageNodeDispatch(game::GameNativeReadOnlyData&, game::GameNativeTypeStorage&,
        TypeIdCounterLifetime&, LightTypeBootstrap&, NativeModelStorageNodeCall* prepared_calls,
        std::size_t call_count, NativeModelStorageNodeDispatchAcquired&) noexcept;
    ~NativeModelStorageNodeDispatch() override;
    NativeModelStorageNodeDispatch(const NativeModelStorageNodeDispatch&) = delete;
    NativeModelStorageNodeDispatch& operator=(const NativeModelStorageNodeDispatch&) = delete;
    // One pre-pool bind, with null actual/empty canonical optional and fresh
    // records. Both this provider and e769 bind to the SAME record aliases.
    void bind(NativeModelStorageNodeDispatchContext&);
    const volatile std::uint32_t* resolve_profile(std::uint32_t captured_profile) const override;
    std::uint8_t invoke_type_0c(std::uint32_t captured_target, void* actual,
        std::uint32_t captured_token) override;
    void invoke_attachment_1c(std::uint32_t captured_target, void* actual,
        std::uint32_t captured_attachment) override;
    void invoke_child(std::uint32_t captured_target, void* actual,
        std::uint32_t captured_scene, std::uint32_t recursion) override;
private:
    NativeModelStorageNodeDispatchContext& context() const;
    NativeModelStorageNodeCall& next(NativeModelStorageNodeCall::Kind, std::uint32_t, void*);
    std::uint32_t require_active_model(void*) const;
    void fail(NativeModelStorageNodeCall&) noexcept;
    game::GameNativeReadOnlyData& data_;
    game::GameNativeTypeStorage& types_;
    TypeIdCounterLifetime& counter_;
    LightTypeBootstrap& node_types_;
    ModelTypeBootstrap model_types_;
    NativeModelStorageNodeCall* const calls_;
    const std::size_t call_count_;
    NativeModelStorageNodeDispatchAcquired& acquired_;
    NativeModelStorageNodeDispatchContext* context_{};
    const volatile std::uint32_t* model_profile_{};
    const volatile std::uint32_t* node_profile_{};
};

// Call slots and all transitive backing are initialized, address-stable and
// independent of Text/Model slots and companions. No slot recycles, including
// after success. Supply an honest finite call frontier; exhaustion poisons and
// retains the graph after any preceding caller effects, never rolls back.
// Disjoint slots admit nested callbacks; no entry-time allocation/global scratch.
// Canonical Model and e769 were created/bound using EXACTLY these record aliases;
// mutable diagnostics and base sizeof checks do not prove that provenance or
// full opaque extents. All scene/group/array arguments satisfy the existing real
// bodies' accessible-memory, actual3Ch/count/current-import/hierarchy contracts.
// Same raw strings, point-light allocator/bindings, registry and native providers
// survive all credits, calls, entered failures and flags0 physical-slot custody.
// Completed/retired payload rejects BEFORE reads; live D62C88 destruction prefix
// remains admitted until B6F440 ends it. Post-body bookkeeping is metadata only.
// No AA1380/93/122 owner, shared child routing, frontend activation, Original
// register/private-stack/FH3/SEH or native lifecycle acceptance is supplied.
} // namespace bsp

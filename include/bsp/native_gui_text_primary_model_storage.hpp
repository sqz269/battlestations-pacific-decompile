#pragma once
#include "bsp/native_gui_text_default_widget_lifetime.hpp"
#include "bsp/native_model_storage_node_dispatch.hpp"
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Text primary Model production requires MSVC Win32.
#endif

namespace bsp {
namespace game { class GameNativeResourcePoolProcess; }
struct NativeGuiTextPrimaryModelStorageContext {
    game::GameNativeResourcePoolProcess& model_pools; // canonical, explicitly ready
    NativeGuiTextDefaultFactoryStorageContext& factory_context;
    const NativeGuiTextDefaultFactoryStorageAcquired& factory;
    NativeGuiTextDefaultWidgetPublicationState& publication; // SAME owner's PRE-Text cells, never value copies
    NativeGuiTextRawModelRecord& primary; // real mutable actual cell, not another record
    NativeModelStorageNodeDispatchContext& primary_node_context;
    NativeModelStorageNodeDispatch& primary_nodes;
    NativeModelStorageDispatchContext& internal188; // exact existing nested85 aliases
    // SAME caller-owned raw8B header. Pointer is fixed; header/data contents
    // remain current across real string-pool callbacks. Outer caller owns EH.
    const void* const actual_name_header8;
};
class NativeGuiTextPrimaryModelStorageFrame;
struct NativeGuiTextPrimaryModelStorageAcquired;
void produce_native_gui_text_primary_model_00aaad95(NativeGuiTextPrimaryModelStorageContext&,
    NativeGuiTextPrimaryModelStorageFrame&, NativeGuiTextPrimaryModelStorageAcquired&);

// Provenance only: no credit, liveness permission, disposal or payload access.
// Only the producer can issue it after real constructor/bind/publication. Keep
// it and ALL referenced records independently alive through later credits,
// failures and flags0 custody. After retirement identities are diagnostic.
class NativeGuiTextPrimaryModelPublication final {
public:
    NativeGuiTextPrimaryModelPublication(const NativeGuiTextPrimaryModelPublication&) = delete;
    NativeGuiTextPrimaryModelPublication& operator=(const NativeGuiTextPrimaryModelPublication&) = delete;
    void* text_identity() const noexcept { return text_; }
    void* model_identity() const noexcept { return model_; }
    const NativeGuiTextDefaultFactoryStorageContext* factory_context() const noexcept { return factory_context_; }
    const NativeGuiTextDefaultFactoryStorageAcquired* factory_acquisition() const noexcept { return factory_; }
    const NativeGuiTextRawModelRecord* primary_record() const noexcept { return record_; }
    const NativeModelStorageDispatchContext* primary_dispatch_record() const noexcept { return dispatch_; }
    const NativeModelPool* pool() const noexcept { return pool_; }
    const NativeModelStorageConstructAcquired* construction() const noexcept { return construction_; }
private:
    NativeGuiTextPrimaryModelPublication() noexcept = default;
    friend struct NativeGuiTextPrimaryModelStorageAcquired;
    friend class NativeGuiTextPrimaryModelStorageFrame;
    friend void produce_native_gui_text_primary_model_00aaad95(NativeGuiTextPrimaryModelStorageContext&,
        NativeGuiTextPrimaryModelStorageFrame&, NativeGuiTextPrimaryModelStorageAcquired&);
    bool issued_{};
    void* text_{};
    void* model_{};
    const NativeGuiTextDefaultFactoryStorageContext* factory_context_{};
    const NativeGuiTextDefaultFactoryStorageAcquired* factory_{};
    const NativeGuiTextRawModelRecord* record_{};
    const NativeModelStorageDispatchContext* dispatch_{};
    const NativeModelPool* pool_{};
    const NativeModelStorageConstructAcquired* construction_{};
};
enum class NativeGuiTextPrimaryModelStoragePhase : std::uint8_t {
    fresh, preparing, prepared, entered, complete, failed
};
struct NativeGuiTextPrimaryModelStorageAcquired final {
    using Phase = NativeGuiTextPrimaryModelStoragePhase;
    enum class Boundary { none, preparation, tree_bind, node_bind, text_admission,
        allocation, constructor_call, canonical_bind, publication, flags_clear };
    NativeGuiTextPrimaryModelStorageAcquired() noexcept = default;
    NativeGuiTextPrimaryModelStorageAcquired(const NativeGuiTextPrimaryModelStorageAcquired&) = delete;
    NativeGuiTextPrimaryModelStorageAcquired& operator=(const NativeGuiTextPrimaryModelStorageAcquired&) = delete;
    // No destructor performs native cleanup; failed Frame destruction terminates.
    Phase phase{Phase::fresh};
    Boundary boundary{Boundary::none}, escaped_boundary{Boundary::none};
    std::uint32_t active_call_site{}, escaped_call_site{};
    bool tree_bind_entered{}, tree_bound{}, node_bind_entered{}, node_bound{};
    bool allocation_entered{}, allocation_returned{}, constructor_entered{}, constructor_returned{};
    bool canonical_bind_entered{}, canonical_bound{}, publication_entered{}, published{}, flags_cleared{};
    NativeModelPool* actual_pool{};
    void* actual_text{};
    void* raw_slot{}; // diagnostic only on failure; NEVER inferred EH authority
    void* constructor_result{};
    NativeModelStorageReference* canonical_reference{};
    std::exception_ptr failure;
    const NativeGuiTextPrimaryModelPublication* publication() const noexcept {
        return phase == Phase::complete && receipt_.issued_ ? &receipt_ : nullptr;
    }
private:
    friend class NativeGuiTextPrimaryModelStorageFrame;
    friend void produce_native_gui_text_primary_model_00aaad95(NativeGuiTextPrimaryModelStorageContext&,
        NativeGuiTextPrimaryModelStorageFrame&, NativeGuiTextPrimaryModelStorageAcquired&);
    NativeGuiTextPrimaryModelPublication receipt_;
};

// Install Frame/Acquired/whole graph first. Constructor only takes addresses;
// prepare is PURE metadata and must precede Text AA1380 as well as B74D00.
// It performs exact e769.bind then2049.bind on the fresh PRIMARY graph, not
// fresh-looking flag checks over unprovable private bindings. Internal188's
// existing genuine graph is separately prepared by its real Text producer.
// Do not enter either fresh primary dispatcher before completed publication.
class NativeGuiTextPrimaryModelStorageFrame final {
public:
    NativeGuiTextPrimaryModelStorageFrame(NativeGuiTextPrimaryModelStorageContext&,
        NativeGuiTextPrimaryModelStorageAcquired&) noexcept;
    ~NativeGuiTextPrimaryModelStorageFrame() noexcept;
    NativeGuiTextPrimaryModelStorageFrame(const NativeGuiTextPrimaryModelStorageFrame&) = delete;
    NativeGuiTextPrimaryModelStorageFrame& operator=(const NativeGuiTextPrimaryModelStorageFrame&) = delete;
    // Pure identity/private-phase view for a separately authenticated lifetime
    // adapter. This grants no native credit, payload liveness or cleanup right.
    bool matches(const NativeGuiTextPrimaryModelStorageContext&,
        const NativeGuiTextPrimaryModelStorageAcquired&,
        NativeGuiTextPrimaryModelStoragePhase) const noexcept;
    void prepare_before_text_allocation();
private:
    friend void produce_native_gui_text_primary_model_00aaad95(NativeGuiTextPrimaryModelStorageContext&,
        NativeGuiTextPrimaryModelStorageFrame&, NativeGuiTextPrimaryModelStorageAcquired&);
    void require_graph();
    void fail() noexcept;
    NativeGuiTextPrimaryModelStorageContext& context_;
    NativeGuiTextPrimaryModelStorageAcquired& acquired_;
    NativeModelPool* pool_{};
    NativeGuiTextPrimaryModelStoragePhase phase_{NativeGuiTextPrimaryModelStoragePhase::fresh};
};

// Qualified AAA710 interior prefix only: B74D00 -> genuine481/B75030 -> ONE
// canonical481 metadata binding (NO added credit) -> actual4C ->138 low bits.
// B6F5A0 establishes the existing native+4 initial credit. No manual seed,
// retain, new pool/getter, ModelOwner/NodeBinding/InstanceGeometry transport.
// Requires successful real pool/native memory operations and exact actual
// ba2/raw93/raw122 Text, fresh4C, childless/unparented/no-timed DEFAULT state.
// Real raw8B name backing remains caller-owned/current; no std::string copy
// or name cleanup is added. It stays live through this call/its outer stage;
// after the caller consumes it its saved address is diagnostic only.
// Receiver/records/providers remain serialized,
// address-stable, mutually separate and live through ALL failure disposition.
// Known extents do not prove opaque/transitive backing or real provider code.
//
// ALL escapes retain/rethrow. NO B74750, destructor, inferred ctor-slot EH,
// pool unlock, rollback or retry:481 has no Native-origin/failed-node-unwind
// completion proof. Its genuine internal callee failure policy is unchanged.
// Completed ctor plus later bind/publication failure keeps its native creator.
// Frame destruction terminates in preparing/entered/failed state.
// Completion/receipt means PREFIX ONLY; no attach, reparent,74,18,78, outer
// name cleanup, wholeAAA710/child completion, activation or Original ABI/EH.
void produce_native_gui_text_primary_model_00aaad95(NativeGuiTextPrimaryModelStorageContext&,
    NativeGuiTextPrimaryModelStorageFrame&, NativeGuiTextPrimaryModelStorageAcquired&);
} // namespace bsp

#pragma once
#include "bsp/native_gui_text_default_factory_storage.hpp"

namespace bsp {
// One independent publication history, supplied by the retained owner. These
// are diagnostics/admission markers, not native ownership or copied counts.
struct NativeGuiTextDefaultWidgetPublicationState final {
    NativeGuiTextRawModelReference* reference{};
    bool bind_entered{}, bind_complete{}, bind_failed{};
    bool scalar_entered{}, scalar_returned{}, scalar_failed{};
    bool companion_disposal_entered{}, disposed{};
};
struct NativeGuiTextDefaultWidgetLifetimeRecord final {
    NativeGuiTextDefaultFactoryStorageContext& factory_context;
    const NativeGuiTextDefaultFactoryStorageAcquired& factory;
    NativeGuiTextDefaultWidgetPublicationState& publication;
};
struct NativeGuiTextDefaultWidgetLifetimeCall final {
    enum class Operation { none, bind, table, class_query, scalar_admission,
        child_scene, child_delete, timed_delete, remove_all };
    Operation operation{Operation::none};
    void* receiver{}; // diagnostic only after failure/retirement
    const void* pointer_cell{}; // diagnostic after the synchronous list call
    std::uint32_t profile{}, target{}, descriptor{}, flags{};
    bool body_entered{}, body_returned{}, result{};
};
struct NativeGuiTextDefaultWidgetLifetimeAcquired final {
    // Owner's noexcept retention guard checks active || poisoned. Never clear
    // either after an escaping call or reset/rebind this provider.
    bool bound{}, active{}, poisoned{};
    std::size_t completed_calls{};
    NativeGuiTextDefaultWidgetLifetimeCall current, failure;
};

// Private DEFAULT Text binding, directly implementing the actual raw lifetime
// interface. It does not use/cast the legacy NativeGuiWidgetLifetimeDispatch.
// No new native owner, type startup, body credit, child transport or ABI.
class NativeGuiTextDefaultWidgetLifetime final : public NativeGuiWidgetLifetimeBindings {
public:
    // Metadata only: ready-only canonical lineage aliases and real numeric
    // table views, no native getter/predicate, table-target snapshot or pool.
    NativeGuiTextDefaultWidgetLifetime(NativeNodeDestructionRuntime&,
        NativeRenderActualOwnerRegistry&, game::GameNativeReadOnlyData&,
        game::GameNativeGuiTextProcess&, NativeGuiTextDefaultWidgetLifetimeAcquired&);
    ~NativeGuiTextDefaultWidgetLifetime() override;
    NativeGuiTextDefaultWidgetLifetime(const NativeGuiTextDefaultWidgetLifetime&) = delete;
    NativeGuiTextDefaultWidgetLifetime& operator=(const NativeGuiTextDefaultWidgetLifetime&) = delete;
    NativeGuiTextDefaultWidgetLifetime(NativeGuiTextDefaultWidgetLifetime&&) = delete;
    NativeGuiTextDefaultWidgetLifetime& operator=(NativeGuiTextDefaultWidgetLifetime&&) = delete;

    // Available before bind so the one raw93/raw118/raw122 graph can refer to
    // this EXACT context. bind follows full stable graph assembly, before pool.
    NativeGuiWidgetLifetimeContext& context() noexcept { return context_; }
    void bind(NativeGuiTextDefaultWidgetLifetimeRecord&);
    // Owner calls BEFORE scalar_entered, literal flags1 and its existing
    // canonical raw122 direct scalar call. No count test/change is added here.
    void require_default_before_scalar();

    const volatile std::uint32_t* current_table(void*, std::uint32_t captured_profile) override;
    bool call_class_query_0c(void*, std::uint32_t captured_target,
        std::uint32_t captured_descriptor) override;
    void call_child_scene_20(void*, std::uint32_t captured_target) override;
    void call_child_delete_04(void*, std::uint32_t captured_target, std::uint32_t flags) override;
    void call_timed_delete_00(void*, std::uint32_t captured_target, std::uint32_t flags) override;
    void remove_all_00a9bd50(void* captured_actual_list, const void* actual_pointer_cell) override;

private:
    enum class Admission { construction, bound, retiring };
    void begin(NativeGuiTextDefaultWidgetLifetimeCall);
    void finish() noexcept;
    [[noreturn]] void reject(const char*);
    void fail() noexcept;
    NativeGuiTextDefaultWidgetLifetimeRecord& record();
    Admission authenticate(void* actual);
    std::uint32_t active_profile(void* actual, Admission);
    void require_empty_branches(void* actual);
    NativeRenderActualOwnerRegistry& registry_;
    game::GameNativeReadOnlyData& data_;
    game::GameNativeGuiTextProcess& process_;
    NativeGuiTextDefaultWidgetLifetimeAcquired& acquired_;
    const volatile std::uint32_t* const base_;
    const volatile std::uint32_t* const text_;
    const volatile std::uint32_t (&text_lineage_)[3];
    NativeGuiWidgetLifetimeContext context_;
    NativeGuiTextDefaultWidgetLifetimeRecord* record_{};
};

// Caller keeps this provider/Acquired/Record/publication state and the complete
// genuine factory/93/122/188 provider graph stable through callbacks/failure.
// Bound/active metadata is not provenance proof. Constructor-prefix admission
// uses the actual factory/raw93 records without requiring premature raw122.
// Owner marks disposal-entered BEFORE placement destruction and disposed AFTER;
// failed/disposing/ended records reject before reference/payload reads.
//
// Only the e75 DEFAULT lifetime entry is admitted. Its before-parent-effects
// and after-callback checks remain mandatory: base table/AA8320 can bypass this
// virtual interface. No parent/children/timed/primary/glyph/content/shader arm
// may be introduced. A genuine true canonical Text-descriptor lifetime answer
// poisons/throws rather than returning into legacy188 release; no early188 read.
// Other descriptor queries preserve real A9E070/AB83D0/ordinary-zero results.
//
// Child scene/scalar/timed calls always poison/throw. Owner scalar remains the
// existing raw122 API over its OWN persistent actual flags cell. A9BD50 uses
// the captured owned AA9390 +64 list and actual pointer-cell, genuine finite
// ring/allocator provenance, and original reload/free order; no fake list,
// reselected header, invented iteration limit, no-op or hierarchy support.
// No automatic native cleanup/disposal, Source failure rollback, FH3/SEH,
// native private-stack/register ABI, production activation or runtime proof.
} // namespace bsp

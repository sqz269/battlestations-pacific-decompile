#pragma once
#include "bsp/native_gui_text_primary_model_storage.hpp"
#include "bsp/native_gui_widget_raw_model_lifetime.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
struct NativeGuiTextPrefixGeometryLifetimeContext;
class NativeGuiTextMainSectionLifetimeStorageFrame;
// One immutable closed choice. Future created-main-section support must extend
// THIS typed choice and require that compound graph's complete history; it must
// never add a selectable shortcut back to geometry-only/empty-prefix admission.
// The choice itself grants no native ownership, scalar or lifetime authority.
class NativeGuiTextPrimaryPrefixOperation final {
public:
    enum class Kind : std::uint8_t { none, geometry_only, geometry_main_section };
    constexpr NativeGuiTextPrimaryPrefixOperation() noexcept = default;
    explicit constexpr NativeGuiTextPrimaryPrefixOperation(
        const NativeGuiTextPrefixGeometryLifetimeContext& geometry) noexcept
        : kind_(Kind::geometry_only), geometry_(&geometry) {}
    constexpr NativeGuiTextPrimaryPrefixOperation(
        const NativeGuiTextPrefixGeometryLifetimeContext& geometry,
        const NativeGuiTextMainSectionLifetimeStorageFrame& main_section) noexcept
        : kind_(Kind::geometry_main_section), geometry_(&geometry), main_section_(&main_section) {}
    Kind kind() const noexcept { return kind_; }
    const NativeGuiTextPrefixGeometryLifetimeContext* geometry() const noexcept { return geometry_; }
    const NativeGuiTextMainSectionLifetimeStorageFrame* main_section() const noexcept { return main_section_; }
private:
    const Kind kind_{Kind::none};
    const NativeGuiTextPrefixGeometryLifetimeContext* const geometry_{};
    const NativeGuiTextMainSectionLifetimeStorageFrame* const main_section_{};
};

// Actual external raw8B input and its independently retained Source progress.
// Constructor binds addresses only. The owner invokes real41E870/41DD20,
// arms each entered marker BEFORE the helper, records returned/failed after it.
// No destructor cleans a name; returned header/data bits are never rediscovered.
struct NativeGuiTextPrimaryPrefixNameProgress final {
    NativeGuiTextPrimaryPrefixNameProgress(const void* header,
        NativeStringRawPoolContext& raw_strings) noexcept : header8(header), strings(raw_strings) {}
    NativeGuiTextPrimaryPrefixNameProgress(const NativeGuiTextPrimaryPrefixNameProgress&) = delete;
    NativeGuiTextPrimaryPrefixNameProgress& operator=(const NativeGuiTextPrimaryPrefixNameProgress&) = delete;
    const void* const header8;
    NativeStringRawPoolContext& strings;
    bool construct_entered{}, construct_returned{}, construct_failed{};
    bool return_entered{}, returned{}, return_failed{};
};

// Immutable concrete array extent for the existing C++17 build. This borrows
// the exact node-dispatch constructor operands; it owns no calls or records.
struct NativeGuiTextPrimaryPrefixCalls final {
    const NativeModelStorageNodeCall* const data;
    const std::size_t count;
};

// Build the whole nonmoving graph before effects. Bind internal tree/node and
// widget history, prepare db55 primary tree/node, then bind2ee slots BEFORE Text
// allocation. All references name the actual objects, never copied progress or
// a second Model record/count. Opaque/transitive extents remain caller proof.
struct NativeGuiTextPrimaryPrefixLifetimeContext final {
    NativeGuiTextDefaultWidgetLifetimeRecord& text;
    NativeGuiTextDefaultWidgetLifetime& authentication;
    NativeGuiTextDefaultWidgetLifetimeAcquired& authentication_acquired;
    NativeGuiTextPrimaryModelStorageContext& producer_context;
    const NativeGuiTextPrimaryModelStorageFrame& producer_frame;
    const NativeGuiTextPrimaryModelStorageAcquired& producer;
    const NativeGuiTextPrimaryPrefixNameProgress& name;
    NativeGuiWidgetRawModelLifetimeContext& widget;
    NativeModelStorageNodeDispatchContext& internal_node_context;
    NativeGuiWidgetRawModelLifetimeAcquired& widget_acquired;
    const NativeGuiTextPrimaryPrefixCalls primary_calls;
    const NativeGuiTextPrimaryPrefixCalls internal_calls;
    const NativeGuiTextPrimaryPrefixOperation operation{};
};

// Metadata-only selection: false ONLY for untouched prepared primary/name;
// true ONLY for db55's issued exact private receipt and completed name return.
// Partial/failed/pending work throws: it is never an empty DEFAULT fallback.
// May be used after scalar for independent metadata only; not a settled-custody
// or liveness proof. Noexcept owner guards must inspect their direct markers.
// An installed operation may still be untouched FRESH here; scalar permission
// separately requires that chosen graph COMPLETE, never a pending-work fallback.
bool native_gui_text_primary_prefix_selected(const NativeGuiTextPrimaryPrefixLifetimeContext&);

// Owner entry BEFORE scalar_entered/count1 admission/persistent actual flags.
// Reuses exact-record auth and genuine current descriptor/base predicate.
// TRUE is unsupported and fails BEFORE any188 payload read. No Text decrement.
void require_native_gui_text_primary_prefix_before_scalar(const NativeGuiTextPrimaryPrefixLifetimeContext&);

// Existing raw118 integration only. Null progress is preentry; later progress
// is the SAME actual scalar.destruction record, never a copied cleanup view.
// No receipt/missing/failed context can authorize4C. After internal188 cleanup,
// inspect independent records only and do not revisit ended188/name payloads.
void require_native_gui_text_primary_prefix_retirement(void* actual,
    const NativeGuiTextPrimaryPrefixLifetimeContext&, const NativeGuiTextRetirementAcquired* progress);
void destroy_native_gui_text_primary_prefix_base(void* actual,
    const NativeGuiTextPrimaryPrefixLifetimeContext&, const NativeGuiTextRetirementAcquired& progress);
} // namespace bsp

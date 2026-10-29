#pragma once
#include "bsp/native_gui_text_primary_model_storage.hpp"
#include "bsp/native_gui_widget_raw_model_lifetime.hpp"
#include <span>

namespace bsp {
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
    NativeGuiWidgetRawModelLifetimeAcquired& widget_acquired;
    const std::span<const NativeModelStorageNodeCall> primary_calls;
    const std::span<const NativeModelStorageNodeCall> internal_calls;
};

// Metadata-only selection: false ONLY for untouched prepared primary/name;
// true ONLY for db55's issued exact private receipt and completed name return.
// Partial/failed/pending work throws: it is never an empty DEFAULT fallback.
// May be used after scalar for independent metadata only; not a settled-custody
// or liveness proof. Noexcept owner guards must inspect their direct markers.
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

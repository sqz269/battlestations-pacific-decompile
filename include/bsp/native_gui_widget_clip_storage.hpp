#pragma once
#include <cstdint>

namespace bsp {
namespace game { class GameNativeReadOnlyData; }
struct NativeStringRawPoolContext;
struct NativeMaterialStorage;
struct NativeMaterialParameterAccess;

// Required borrowed production domains, with no default owner or callback.
// The material name adapter must borrow the SAME raw string publication/gate/
// manager as strings. The scalar refs are the actual D7A24C/D7A218 RO views and
// SAME live E12FC0 aspect cell, not value snapshots or a second state owner.
struct NativeGuiWidgetClipBindings {
    const game::GameNativeReadOnlyData& actual_read_only_data;
    NativeStringRawPoolContext& strings;
    NativeMaterialParameterAccess& materials;
    const volatile std::uint32_t& actual_one_00d7a24c;
    const volatile std::uint32_t& actual_zero_00d7a218;
    const float& actual_aspect_ratio_00e12fc0;
};

enum class NativeGuiWidgetClipPhase : std::uint8_t { fresh, entered, returned, failed };
enum class NativeGuiWidgetClipNameAction : std::uint8_t { none, primary, aspect };
enum class NativeGuiWidgetClipStage : std::uint8_t {
    prepared, ancestor_selection, enabled_store,
    clip_construct, clip_register, clip_name_return, enabled_reload,
    center_construct, center_register, center_name_return,
    border_construct, border_register, border_name_return,
    aspect_construct, aspect_register, aspect_name_return, unwind_name
};

// Source orchestration only: prepare outside the failing scope BEFORE entry or
// any material registration/owner binding. This frame owns two distinct raw8h
// name headers and metadata, NOT the widget, ancestors, material or any graph.
// Independently retain the full SAME live widget/ancestor/material/name/
// effect/provider/backing graph, including partial parameter records and name
// allocations, before effects. Counts/links/163's companion cannot discover or
// retain that graph, establish constructor completion or keep ancestors alive.
//
// Actual widget has aligned live backing through EBh; every traversed parent
// has genuine current+5C=A9E110 and live type60/parent70. The finite ancestor
// chain and any matched ClipBox EC..103 fields stay independently live. If
// callbacks make current E8 active, the captured ancestor must be nonnull and
// genuinely initialized; no rectangle/default ancestor is manufactured.
// Bindings/services and their metadata remain stable/live and disjoint from
// this frame, widget/material and consumed or repurposed scratch. Borrowed
// one/zero/aspect cells and name allocations have valid separate backing;
// E8/EC/F4 parameter sources intentionally borrow the actual fields, and the
// matched ancestor may be the widget itself. Known-region checks do not prove private graph backing,
// legal C++ placement/lifetimes or all scratch/ancestor alias constraints.
// External serialization forbids reentry or callback mutation of control
// metadata/bindings; the raw name headers remain genuine callee storage.
// Callbacks may change actual E8/header/source cells only within the concrete
// raw string/material domains; all scheduled current reads remain current.
// Genuine reached fields may belong to a separately retained partial graph;
// this entry proves no constructor completion or163/B18A40 admission.
// Every reached current name length+1 must be a readable literal span and a
// writable destination span for the genuine raw copy; no stable-header or
// strong failure guarantee is inferred from the scheduled constructor call.
class NativeGuiWidgetClipOperation final {
public:
    NativeGuiWidgetClipOperation(void* actual_widget, NativeMaterialStorage&,
        NativeGuiWidgetClipBindings&);
    ~NativeGuiWidgetClipOperation();
    NativeGuiWidgetClipOperation(const NativeGuiWidgetClipOperation&) = delete;
    NativeGuiWidgetClipOperation& operator=(const NativeGuiWidgetClipOperation&) = delete;
    NativeGuiWidgetClipOperation(NativeGuiWidgetClipOperation&&) = delete;
    NativeGuiWidgetClipOperation& operator=(NativeGuiWidgetClipOperation&&) = delete;

    // Metadata/identity diagnostics only; returned names may already be freed.
    NativeGuiWidgetClipPhase phase() const noexcept { return phase_; }
    NativeGuiWidgetClipStage stage() const noexcept { return stage_; }
    NativeGuiWidgetClipStage failed_stage() const noexcept { return failed_stage_; }
    NativeGuiWidgetClipNameAction pending_name_action() const noexcept { return name_action_; }
    const void* widget_identity() const noexcept { return actual_widget_; }
    const void* captured_ancestor_identity() const noexcept { return captured_ancestor_; }
    const void* primary_name_identity() const noexcept { return &primary_name_; }
    const void* aspect_name_identity() const noexcept { return &aspect_name_; }
    std::uint8_t returned_registration_calls() const noexcept { return returned_registrations_; }

private:
    struct NameHeader { std::uint32_t length; void* data; };
    struct Views { const char* clip; const char* center; const char* border; const char* aspect; };
    static Views prepare(void*, NativeMaterialStorage&, NativeGuiWidgetClipBindings&, const void*);
    void return_name();
    void unwind_name() noexcept;

    const Views views_;
    void* const actual_widget_;
    NativeMaterialStorage& material_;
    NativeGuiWidgetClipBindings& bindings_;
    NameHeader primary_name_; // Uninitialized until the scheduled native clear/ctor.
    NameHeader aspect_name_;
    void* captured_ancestor_ = nullptr;
    NativeGuiWidgetClipPhase phase_ = NativeGuiWidgetClipPhase::fresh;
    NativeGuiWidgetClipStage stage_ = NativeGuiWidgetClipStage::prepared;
    NativeGuiWidgetClipStage failed_stage_ = NativeGuiWidgetClipStage::prepared;
    NativeGuiWidgetClipNameAction name_action_ = NativeGuiWidgetClipNameAction::none;
    std::uint8_t returned_registrations_ = 0;
    friend void register_native_gui_widget_clip_storage_00aa9f10(void*,
        NativeMaterialStorage&, NativeGuiWidgetClipBindings&, NativeGuiWidgetClipOperation&);
};

// Whole AA9F10 schedule, new Source API (Native ECX widget/stack material/RET4).
// Four native string-only unwind actions: no material rollback, ancestor retain
// or E8 restore. Name action arms ONLY after construction and is consumed BEFORE
// normal getter/return or unwind41DD20. Partial pre-arm names stay retained.
// On admitted escape the operation is sticky failed; cleanup escape terminates
// under the explicit Source double-exception policy. No replay/reset or implicit
// graph cleanup. Entered/failed frame destruction fails closed by Source policy;
// fresh/returned destruction reads metadata only. Caller-held failed graph and
// frame must survive outside unwind; markers describe Source scheduling, not
// Native instruction/exception origin, FH3/SEH or handler/ABI equivalence.
// No convenience B18A40 binding or production activation is added.
void register_native_gui_widget_clip_storage_00aa9f10(void* actual_widget,
    NativeMaterialStorage&, NativeGuiWidgetClipBindings&, NativeGuiWidgetClipOperation&);
} // namespace bsp

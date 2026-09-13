#pragma once
#include "bsp/gui_text_factory.hpp"
#include "bsp/gui_text_type_dispatch.hpp"
#include "bsp/gui_text_properties.hpp"
#include "bsp/gui_text_clip_refresh.hpp"
#include "bsp/gui_text_copy.hpp"
#include "bsp/gui_widget_copy.hpp"
#include "bsp/native_gui_text_identity.hpp"
#include <exception>
#include <optional>
#include <stdexcept>
#include <unordered_map>

namespace bsp {
// Borrow the same live constants used by the native x87 bodies. No defaults
// substitute for the engine's current constant storage.
struct GuiTextBoundsConstants {
    const volatile double& half_00d7a280;
    const volatile double& width_divisor_00cec380;
    const volatile double& height_divisor_00cef1b8;
    const volatile double& padding_00d5c5c8;
};
struct GuiTextRuntimeFactoryServices {
    NativeGuiTextPool& pool;
    GuiTextBufferServices& buffers;
    GuiTextGlyphChildCalls& children;
    GuiTextConstructorConstants constructor;
    GuiTextTypeDispatchServices& dispatch;
    GuiTextPropertyServices& properties;
    GuiTextClipRefreshServices& clip;
    GuiTextBoundsConstants bounds;
    const volatile std::uint32_t (&lineage_00f8be28)[3];
};

// A pending operation is retained on its SAME type implementation. Its caller
// must also retain the surrounding page/loader/glyph frame and mapped owners.
// This exception is not successful loading or permission to unwind the page.
class GuiTextRuntimePending final : public std::logic_error {
public:
    using std::logic_error::logic_error;
};
class GuiTextRuntimeFactory;
class GuiTextRuntimeCopyOperation;
class GuiWidgetCopySourceBorrow;

// One implementation and one GuiTextLifetime on the existing widget owner.
// No raw Text slot is cast to a C++ object, no duplicate hierarchy is created.
// Original D5C6C8 ABI and native strings/SEH remain outside this interface.
// The copied factory operation below adopts one copied lifetime and preserves
// its constructor continuation, using required actual clone/retention services.
class GuiTextRuntimeImplementation final : public GuiWidgetTypeImplementation {
public:
    ~GuiTextRuntimeImplementation() noexcept override;
    GuiTextLifetime& lifetime() noexcept { return *lifetime_; }
    // Same factory allocation's actual prefix/count; never the C++ owner address.
    NativeGuiTextIdentityReference& native_identity();
    void constructed74(GuiWidgetOwner&) override;
    void properties_bound(GuiWidgetOwner&, const GuiTable&) override;
    void loaded78(GuiWidgetOwner&) override;
    void set_active60(GuiWidgetOwner&, bool) override;
    bool is_visible38(GuiWidgetOwner&) override;
    void visibility_changed3c(GuiWidgetOwner&, bool) override;
    void before_scalar_deletion4(GuiWidgetOwner&) override;
    void before_scene_release(GuiWidgetOwner&) override;
    void release_secondary_scene_nodes(GuiWidgetOwner&) override;
    void refresh_clip70(GuiWidgetOwner&) override;
    void set_alpha4c(GuiWidgetOwner&, float) override;
    void set_size58(GuiWidgetOwner&, const GuiWidgetSize&) override;

    // Additional real Text profile entries, kept typed until generic widget
    // dispatch exposes these signatures. All operate on this SAME lifetime.
    void set_alpha4c_00ab6ad0(float);
    void set_color50_00ab6b50(const float (&)[4]);
    void resize58_00abbf30(const GuiWidgetSize&);
    void align_bounds64_00ab6d70(float& left, float& top, float& right, float& bottom);
    void set_state80_00ab7200(std::int32_t);
    // Same AB6BD0 kernel used by bounds64: live multiline178 or current font
    // signed lowword14, divided by live CEF1B8 and narrowed once to float.
    float normalized_height_00ab6bd0();
    void submit_utf16_00ab6ab0(std::u16string_view);
    void submit_source_00abaed0(const std::string&, bool localize);
    void submit_ellipsis_00abb000(const std::string&, float width, bool localize);
    void rebuild_content_00abb1d0();

    bool has_pending_operation() const noexcept;
    bool has_active_operation() const noexcept override;
    GuiTextRuntimeContentContinuation* pending_content() noexcept;
    const GuiTextClipRefreshContinuation* pending_clip() const noexcept;
    // These consume only the continuation AFTER its exact required child
    // operation completed. Never call them merely to skip a missing child.
    // A later child can suspend again and throws with its new frame retained.
    void resume_after_glyph_child();
    void resume_clip_after_child70();
private:
    friend class GuiTextRuntimeFactory;
    friend class GuiTextRuntimeCopyOperation;
    struct CopiedAdmission {};
    struct DefaultAdmission {};
    GuiTextRuntimeImplementation(GuiTextRuntimeFactory&, GuiWidgetOwner&, DefaultAdmission);
    // Empty host shell only. initialize_copy installs the ONE copied lifetime
    // while this shell is already retained by its factory operation.
    GuiTextRuntimeImplementation(GuiTextRuntimeFactory&, GuiWidgetOwner&, CopiedAdmission);
    void initialize_copy(const GuiTextLifetime&, GuiTextCursorServices&);
    void initialize_default();
    GuiTextCopyPhase run_copy();
    void require_owner(GuiWidgetOwner&) const;
    void require_idle() const;
    void require_constructor_read_or_idle() const;
    void retain_submission(GuiTextSubmitResult);
    void continue_clip();
    GuiTextRuntimeFactory& factory_;
    GuiWidgetOwner& owner_;
    std::unique_ptr<GuiTextLifetime> lifetime_;
    std::unique_ptr<GuiTextPropertiesContinuation> properties_;
    std::unique_ptr<GuiTextSubmitContinuation> submission_;
    std::optional<GuiTextClipRefreshContinuation> clip_;
    std::optional<GuiTextCopyServices> copy_services_;
    // In-place construction cannot allocate after publishing the lifetime.
    // References stay valid when the implementation's unique_ptr transfers.
    std::optional<GuiTextCopyContinuation> copy_;
    bool copied_admission_{};
    bool default_admission_{}; // retained constructor shell, no extra count
    bool default_completed_{};
    bool copy_runtime_admitted_{};
    bool copy_dispatch_active_{};
    bool source_copy_borrowed_{}; // host mutation guard, not another refcount
    GuiTextRuntimeCopyOperation* copy_operation_{}; // same retained caller until admission
};

struct GuiTextRuntimeCopyServices {
    const GuiWidgetCopyServices& base;
    GuiTextCursorServices& cursor;
};
enum class GuiTextRuntimeCopyPhase {
    ready, running, pending_content, complete, null_allocation, failed
};

// Retained AA1380 nonnull-source caller. This owns the sole destination
// layout and, until admission, its concrete implementation. The implementation
// owns one copied GuiTextLifetime plus its exact derived/content frame. Base
// acquired creators and the Text allocation/prefix remain visible on failure.
// Source borrow prevents retirement across allocation/construction/pending;
// it is released immediately on completion or the native null-allocation arm.
// All services/factory/owner domains outlive this operation and created Text.
// No destructor discards a started pending/failed native operation or retries
// it. Such frames must stay alive for explicit inspection/native intervention.
class GuiTextRuntimeCopyOperation final {
public:
    ~GuiTextRuntimeCopyOperation() noexcept;
    GuiTextRuntimeCopyOperation(const GuiTextRuntimeCopyOperation&) = delete;
    GuiTextRuntimeCopyOperation& operator=(const GuiTextRuntimeCopyOperation&) = delete;
    GuiTextRuntimeCopyPhase run_00aa1380();
    // ONLY after the actual saved glyph-child operation completed. Generic
    // Text resume routes through this SAME caller and completes admission;
    // no native constructor prefix or after-child continuation is replayed.
    GuiTextRuntimeCopyPhase resume_after_glyph_child();
    GuiTextRuntimeCopyPhase phase() const noexcept { return phase_; }
    GuiTextRuntimeImplementation& implementation();
    GuiTextRuntimeContentContinuation* pending_content() noexcept;
    NativeGuiTextModelCloneAcquired& base_acquired() noexcept { return base_acquired_; }
    GuiTextCursorAcquired& cursor_acquired();
    // Same allocation: only its actual first eight bytes are constructed.
    // No native Text body may be accessed through this pointer.
    void* opaque_allocation_slot() const noexcept { return raw_slot_; }
    const std::exception_ptr& failure() const noexcept { return failure_; }
    // Complete transfers the same layout/registered owner. Native allocation
    // null returns nullptr; no unconstructed layout is reported as a clone.
    std::unique_ptr<GuiLayoutWidget> take_completed_layout();
private:
    friend class GuiTextRuntimeFactory;
    GuiTextRuntimeCopyOperation(GuiTextRuntimeFactory&, GuiWidgetOwner&,
        GuiTextRuntimeImplementation&, std::unique_ptr<GuiLayoutWidget>&,
        const GuiWidgetBaseCopyPreimage&, GuiTextRuntimeCopyServices);
    void finish_admission();
    void release_source_borrow() noexcept;
    GuiTextRuntimeFactory& factory_;
    GuiWidgetOwner& source_;
    GuiTextRuntimeImplementation& source_implementation_;
    std::unique_ptr<GuiLayoutWidget> destination_;
    GuiWidgetBaseCopyPreimage preimage_;
    GuiTextRuntimeCopyServices services_;
    std::unique_ptr<GuiWidgetCopySourceBorrow> source_borrow_;
    NativeGuiTextModelCloneAcquired base_acquired_;
    std::unique_ptr<GuiWidgetTypeImplementation> implementation_;
    GuiTextRuntimeImplementation* copied_implementation_{}; // borrowed, same object
    GuiTextRuntimeCopyPhase phase_{GuiTextRuntimeCopyPhase::ready};
    void* raw_slot_{};
    std::exception_ptr failure_;
};

// The same native pool allocation now carries its proven eight-byte identity
// prefix; the owner runtime retains the one type/body implementation. Configure
// make_type to call this factory for Text and the
// existing real factory for other types. Every supplied domain must outlive
// all live Text and completed flags0 storage. Explicit pool startup/shutdown
// remains with the host; constructing this class does not initialize F8BDF0.
class GuiTextRuntimeFactory final : public GuiTextCursorParameterOwnerServices {
public:
    explicit GuiTextRuntimeFactory(GuiTextRuntimeFactoryServices);
    ~GuiTextRuntimeFactory() noexcept;
    GuiTextRuntimeFactory(const GuiTextRuntimeFactory&) = delete;
    GuiTextRuntimeFactory& operator=(const GuiTextRuntimeFactory&) = delete;
    std::unique_ptr<GuiWidgetTypeImplementation> make_type(GuiWidgetOwner&);
    // Concrete cursor adapter, bound to this factory's SAME canonical registry.
    void bind_retained_text_00b18a40(NativeMaterialStorage&, GuiWidgetOwner&,
        NativeRenderActualOwners&) override;
    NativeGuiTextIdentityReference& actual_identity(GuiWidgetOwner&);
    // Material/property readers must resolve to this owner; +08 onward in the
    // raw slot remains unconstructed. Completed flags0 storage is rejected.
    GuiWidgetOwner& canonical_owner(void* actual_identity);
    // Failed native default construction keeps its one implementation/lifetime
    // on the allocation record and the runtime's constructor borrow. No retry,
    // rollback or successful factory result is provided by this diagnostic.
    const std::exception_ptr& construction_failure(GuiLayoutWidget&) const;
    // Prepare a stable nonnull-source AA1380 caller before native allocation.
    // Source must be this factory's existing live Text implementation. The
    // explicit fresh destination preserves its authored_x/visible preimages;
    // base preimage supplies AA9520's other represented unwritten fields.
    // No default lifetime construction, fallback Model26 clone or null/no-op
    // Text-retention provider substitutes for the required actual dependencies.
    std::unique_ptr<GuiTextRuntimeCopyOperation> begin_copy_00aa1380(
        GuiWidgetOwner& source, std::unique_ptr<GuiLayoutWidget>& destination,
        const GuiWidgetBaseCopyPreimage&, GuiTextRuntimeCopyServices);
    // AB9D38/4F prerequisite ONLY: construct primary-null default Text in the
    // SAME runtime. Does not append+198, clone/bind a model, run74/78 or claim
    // the AB98F0 child tail completed. Caller transports the sole allocation.
    std::unique_ptr<GuiLayoutWidget> construct_unbound_glyph_child();
    // Scalar flags0 ended typed lifetime but retained the raw pool slot.
    // Call while the completed layout wrapper still exists, before disposal.
    void release_completed_storage(GuiLayoutWidget&);
    std::size_t retained_allocation_count() const noexcept { return allocations_.size(); }
private:
    friend class GuiTextRuntimeImplementation;
    friend class GuiTextRuntimeCopyOperation;
    struct Allocation {
        void* raw_slot{};
        NativeGuiTextIdentityPrefix* prefix{};
        std::unique_ptr<NativeGuiTextIdentityReference> identity;
        std::unique_ptr<GuiTextRuntimeImplementation> default_construction;
        std::unique_ptr<GuiLayoutWidget> failed_default_layout;
        std::exception_ptr failure;
        bool registered{};
        bool completed_flags0{};
    };
    void bind_identity(GuiWidgetOwner&);
    void return_completed_allocation(GuiLayoutWidget&) noexcept;
    void implementation_destroyed(GuiLayoutWidget&, bool completed_flags0) noexcept;
    GuiTextRuntimeFactoryServices services_;
    // Allocation handles only, not Text state, widget ownership or a tree.
    std::unordered_map<GuiLayoutWidget*, Allocation> allocations_;
};
} // namespace bsp

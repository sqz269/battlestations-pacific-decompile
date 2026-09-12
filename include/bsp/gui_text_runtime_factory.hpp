#pragma once
#include "bsp/gui_text_factory.hpp"
#include "bsp/gui_text_type_dispatch.hpp"
#include "bsp/gui_text_properties.hpp"
#include "bsp/gui_text_clip_refresh.hpp"
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

// One implementation and one GuiTextLifetime on the existing widget owner.
// No raw Text slot is cast to a C++ object, no duplicate hierarchy is created.
// Original D5C6C8 ABI, native strings/SEH and copy construction ABB2C0 remain
// outside this new interface. Ordinary successful constructor/current74 and
// supported completed property/content domains are executable compositions.
class GuiTextRuntimeImplementation final : public GuiWidgetTypeImplementation {
public:
    ~GuiTextRuntimeImplementation() noexcept override;
    GuiTextLifetime& lifetime() noexcept { return *lifetime_; }
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
    GuiTextRuntimeContentContinuation* pending_content() noexcept;
    const GuiTextClipRefreshContinuation* pending_clip() const noexcept;
    // These consume only the continuation AFTER its exact required child
    // operation completed. Never call them merely to skip a missing child.
    // A later child can suspend again and throws with its new frame retained.
    void resume_after_glyph_child();
    void resume_clip_after_child70();
private:
    friend class GuiTextRuntimeFactory;
    GuiTextRuntimeImplementation(GuiTextRuntimeFactory&, GuiWidgetOwner&);
    void require_owner(GuiWidgetOwner&) const;
    void require_idle() const;
    void retain_submission(GuiTextSubmitResult);
    void continue_clip();
    GuiTextRuntimeFactory& factory_;
    GuiWidgetOwner& owner_;
    std::unique_ptr<GuiTextLifetime> lifetime_;
    std::unique_ptr<GuiTextPropertiesContinuation> properties_;
    std::unique_ptr<GuiTextSubmitContinuation> submission_;
    std::optional<GuiTextClipRefreshContinuation> clip_;
};

// Opaque raw-allocation transport only; the owner runtime retains the one type
// implementation. Configure make_type to call this factory for Text and the
// existing real factory for other types. Every supplied domain must outlive
// all live Text and completed flags0 storage. Explicit pool startup/shutdown
// remains with the host; constructing this class does not initialize F8BDF0.
class GuiTextRuntimeFactory final {
public:
    explicit GuiTextRuntimeFactory(GuiTextRuntimeFactoryServices);
    ~GuiTextRuntimeFactory() noexcept;
    GuiTextRuntimeFactory(const GuiTextRuntimeFactory&) = delete;
    GuiTextRuntimeFactory& operator=(const GuiTextRuntimeFactory&) = delete;
    std::unique_ptr<GuiWidgetTypeImplementation> make_type(GuiWidgetOwner&);
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
    struct Allocation { void* raw_slot; bool completed_flags0; };
    void implementation_destroyed(GuiLayoutWidget&, bool completed_flags0) noexcept;
    GuiTextRuntimeFactoryServices services_;
    // Allocation handles only, not Text state, widget ownership or a tree.
    std::unordered_map<GuiLayoutWidget*, Allocation> allocations_;
};
} // namespace bsp

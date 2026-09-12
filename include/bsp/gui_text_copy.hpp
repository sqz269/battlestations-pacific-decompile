#pragma once
#include "bsp/gui_text_lifetime.hpp"
#include "bsp/gui_text_runtime_submit.hpp"

namespace bsp {
// Required actual B18A40 operation, not a renderer replacement or semantic
// reference counter. Implementations resolve this SAME Text owner's actual
// identity, then perform set_native_material_parameter_owner_00b18a40 with
// retain_flag=1 and the supplied canonical actual-owner domain. The identity
// must have the real atomic +04 and current terminal virtual0 binding, and
// outlive the material's final release. A GuiWidgetOwner address or its
// semantic extra_fields().references_04 cannot be used as that raw storage.
// Native order: release the old owner iff old byte10D; clear after callback;
// publish new owner+0C/byte10D=1; increment incoming+04. Equal identity still
// releases/reacquires; no protective retain. Missing binding must throw before
// claiming this call completed; never install a null/no-op successful owner.
struct GuiTextCursorParameterOwnerServices {
    virtual ~GuiTextCursorParameterOwnerServices() = default;
    virtual void bind_retained_text_00b18a40(NativeMaterialStorage&,
        GuiWidgetOwner&, NativeRenderActualOwners&) = 0;
};
struct GuiTextCursorServices {
    GuiTextBufferServices& buffers;
    NativeMeshSectionLayoutServices& layouts;
    GuiTextCursorParameterOwnerServices& parameter_owner;
    const volatile float& one_00d7a24c;
};

enum class GuiTextCursorPhase { not_started, running, complete, failed };
// One native caller's creator references/temporary headers, not duplicate
// model/mesh/material ownership. Keep alive on failure; native publications
// stand. No destructor rolls back/releases or retries interrupted operations.
// Before a possibly terminal release its pointer is cleared here, so even a
// throwing callback is never released twice. Failure cleanup/SEH is outside
// the supported normal-return domain and needs explicit owner intervention.
struct GuiTextCursorAcquired {
    GuiTextCursorPhase phase{GuiTextCursorPhase::not_started};
    NativeMeshStorage* mesh{};
    void* declaration{};
    void* vertex{};
    NativeMeshSectionStorage* section{};
    NativeMaterialStorage* material{};
    NativeString format_name;
    NativeString effect_name;
    bool format_name_live{};
    bool effect_name_live{};
};

// AB8910..AB8C24, ECX Text/no stack arguments/RET. Full supported normal
// sequence, using its SAME canonical cursor+184 slot and actual owner pools.
// Existing nonnull cursor returns without creation or repair. Fresh creation
// requires actual renderer38/5C, material/effect/layout and Text-retain bindings.
// Actual Model current38 must be B6DB10. This is a new C++ ABI, not a callable
// native Text address or factory-registration implementation.
void ensure_gui_text_cursor_00ab8910(GuiTextLifetime&,
    GuiTextCursorServices&, GuiTextCursorAcquired&);

enum class GuiTextCopyPhase {
    admitted, cursor, sections, rebuilding, pending_content, complete, failed
};
struct GuiTextCopyServices {
    GuiTextCursorServices& cursor;
    GuiTextSubmitServices& submit;
};

// ABB5FA/ABB601/ABB608 after the explicit after-AA9520 lifetime constructor.
// Borrows that ONE companion and its services; caller keeps both alive along
// with this frame through pending content/failure. No automatic rollback or
// native completion occurs when a frame is destroyed. Each copied lifetime
// admits one frame only. No replay after a throwing native operation.
//
// AA9520 canonical widget-owner copy construction remains REQUIRED upstream.
// Consequently this does not enable GuiTextRuntimeFactory::clone, substitute
// default lifetime construction, or claim full ABB2C0/AAB4C0 reconstruction.
class GuiTextCopyContinuation final {
public:
    GuiTextCopyContinuation(GuiTextLifetime& already_admitted, GuiTextCopyServices&);
    GuiTextCopyContinuation(const GuiTextCopyContinuation&) = delete;
    GuiTextCopyContinuation& operator=(const GuiTextCopyContinuation&) = delete;
    GuiTextCopyPhase run_derived_00abb2c0();
    // Only after the actual suspended glyph-child tail completed; the inner
    // submission rejects other pending reasons and preserves its frame.
    GuiTextCopyPhase resume_after_child();
    GuiTextCopyPhase phase() const noexcept { return phase_; }
    GuiTextCursorAcquired& cursor_acquired() noexcept { return cursor_; }
    GuiTextSubmitContinuation* pending_content() noexcept { return pending_.get(); }
private:
    GuiTextLifetime& lifetime_;
    GuiTextCopyServices& services_;
    GuiTextCopyPhase phase_{GuiTextCopyPhase::admitted};
    GuiTextCursorAcquired cursor_;
    std::unique_ptr<GuiTextSubmitContinuation> pending_;
};
} // namespace bsp

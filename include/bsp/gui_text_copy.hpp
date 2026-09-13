#pragma once
#include "bsp/gui_text_lifetime.hpp"
#include "bsp/gui_text_runtime_submit.hpp"

namespace bsp {
// Required actual B18A40 operation, not a renderer replacement or semantic
// reference counter. Implementations resolve this SAME Text owner's actual
// identity, then perform set_native_material_parameter_owner_00b18a40 with
// retain_flag=1 and the supplied canonical actual-owner domain. The identity
// must have the real atomic +04 and current terminal virtual0 binding, and
// outlive the material's final release. A GuiWidgetOwner address cannot stand
// in for the canonical factory's actual eight-byte native prefix.
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
// throwing callback is never released twice. The native conditional Model-name
// and raw-slot cleanup is reconstructed; completed Model creators survive host
// registration failure. FH3 states4/5 release the SAME reused name header.
// Factory-internal mesh/material cleanup remains a provider boundary.
struct GuiTextCursorAcquired : GuiTextAuxiliaryModelAcquired {
    GuiTextCursorPhase phase{GuiTextCursorPhase::not_started};
    NativeMeshStorage* mesh{};
    void* declaration{};
    void* vertex{};
    GuiNativeDeclarationAcquired declaration_factory;
    NativeStreamCloneAcquired vertex_factory;
    NativeMeshSectionStorage* section{};
    NativeMaterialStorage* material{};
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
// GuiTextRuntimeCopyOperation composes that producer and owns this frame in
// the ONE copied implementation. This continuation alone does not enable a
// subtree clone, substitute default construction or provide native Text ABI.
class GuiTextCopyContinuation final {
public:
    GuiTextCopyContinuation(GuiTextLifetime& already_admitted, GuiTextCopyServices&);
    GuiTextCopyContinuation(const GuiTextCopyContinuation&) = delete;
    GuiTextCopyContinuation& operator=(const GuiTextCopyContinuation&) = delete;
    GuiTextCopyPhase run_derived_00abb2c0();
    // Only after the actual suspended glyph-child tail completed; the inner
    // submission rejects other pending reasons and preserves its frame. A
    // throwing suffix marks this frame failed, forbidding partially consumed
    // builders from being retried. Native SEH rollback is not reconstructed.
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

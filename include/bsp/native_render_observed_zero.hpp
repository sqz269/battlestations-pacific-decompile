#pragma once
#include "bsp/native_mesh_canonical.hpp"
#include "bsp/native_render_zero_frame.hpp"
#include <optional>
namespace bsp {
class NativePostEffectModelFrame;
class NativePostEffectParentFrame;
class NativeMaterialFrame;
// No public token or call. Only the specific actual-decrement continuations
// below may enter. Existing public zero dispatch remains a valid-input legacy
// interface and does not issue ordinary raw Mesh/Model/parent scalar authority.
class NativeRenderObservedZeroAccess final {
private:
    friend class NativeRenderObservedZeroFrame;
    friend class NativePostEffectModelFrame;
    friend class NativePostEffectParentFrame;
    friend class NativeMaterialFrame;
    friend class NativeModelReference;
    friend struct NativeMatrixStorageAccess;
    friend class NativeMeshCanonicalAccess;
    friend class NativeModelStorageObservedZeroIssuer;
    friend class NativeModelStorageGeometryObservedZeroIssuer;
    friend void release_render_command_reference(RenderCommandReference&);
    friend void release_native_render_actual_owner_observed(NativeRenderActualOwners&,
        void*, NativeRenderActualOwnerReleaseObservation&);
    friend class NativeGuiCurveStorageObservedZeroIssuer;
    friend class NativeGuiFrameBoxConstructedStorageObservedZeroIssuer;
    friend class NativeGuiFrameBoxRecordsObservedZeroIssuer;
    friend class NativeGuiFrameBoxStateStorageObservedZeroIssuer;
    friend class NativeGuiIconConstructedStorageObservedZeroIssuer;
    friend class NativeGuiModelStorageObservedZeroIssuer;
    friend class NativeGuiSoundStorageObservedZeroIssuer;
    friend class NativeGuiTextCursorStorageObservedZeroIssuer;
    friend class NativeGuiTextFieldStorageObservedZeroIssuer;
    friend class NativeGuiTextGlyphBuffersStorageObservedZeroIssuer;
    friend class NativeGuiTextMainGeometryStorageObservedZeroIssuer;
    friend class NativeGuiTextMainSectionStorageObservedZeroIssuer;
    friend class NativeGuiTextShaderStorageObservedZeroIssuer;
    friend class NativeGuiTextStorageConstructObservedZeroIssuer;
    friend class NativeMainSceneSubmissionObservedZeroIssuer;
    friend class NativeMaterialProgramCompilerActualObservedZeroIssuer;
    friend class NativeParticleClockSinkReleaseObservedZeroIssuer;
    friend class NativeParticleClockShutdownObservedZeroIssuer;
    class CppCountUse final {
    public:
        explicit CppCountUse(RenderCommandReference&);
        ~CppCountUse() noexcept;
        CppCountUse(const CppCountUse&) = delete;
        CppCountUse& operator=(const CppCountUse&) = delete;
    private:
        std::optional<NativeMeshCanonicalBorrow> mesh_;
        NativePostEffectModelFrame* model_{};
        NativePostEffectParentFrame* parent_{};
        NativeMaterialFrame* material_{};
    };
    // An empty scope changes only caller metadata, never resolves/samples an
    // owner before its real decrement. Retained holds outlive this active call.
    class Invocation final {
    public:
        explicit Invocation(NativeRenderObservedZeroFrame& f) noexcept : frame_(f) {
            frame_.active_invocations_.fetch_add(1, std::memory_order_acq_rel);
        }
        ~Invocation() noexcept { frame_.active_invocations_.fetch_sub(1, std::memory_order_release); }
        Invocation(const Invocation&) = delete;
        Invocation& operator=(const Invocation&) = delete;
    private:
        NativeRenderObservedZeroFrame& frame_;
    };
    static void attach_hold(NativeRenderObservedZeroFrame&, NativeRenderObservedZeroHold&) noexcept;
    static void fail_frame(NativeRenderObservedZeroFrame&) noexcept;
    // Only real caller last-use destruction or the actual owning quiescence
    // fence may drain; active invocation failure is recorded BEFORE any unlink.
    static bool end_frame(NativeRenderObservedZeroFrame&, bool source_failed) noexcept;
    static void release_ordinary(RenderCommandReference&);
    static void invoke(RenderCommandReference&, void* captured_actual, NativeRenderObservedZeroFrame&);
    static void dispatch(NativeRenderActualOwners&, void* captured_actual, NativeRenderObservedZeroFrame&);
};
} // namespace bsp

#pragma once
#include "bsp/native_render_effect_lifetime.hpp"
#include <array>

namespace bsp {
struct NativeDistortionLifetimeContext;
// Host metadata only. Use the SAME actual-owner registry as the effect's
// canonical post-effect children. Profiles in order: D5E164, D5E178, D5E18C,
// D5E1A0, D5E1B4, D61FE0, D62150, D61F1C; each view covers its first two cells.
struct NativeRenderPassCompanionContext {
    NativeRenderActualOwnerRegistry& registry;
    NativeRenderEffectLifetimeContext& effects;
    std::array<const volatile std::uint32_t*, 8> profiles;
    // Required only for the distortion profile. Its effects member must be
    // this SAME effects object. The remaining camera/frame/scene and child
    // lifetime contracts remain those of the existing raw distortion deleter.
    NativeDistortionLifetimeContext* distortion{};
};

// Address-stable caller-owned companion for one completed service-visible
// pass. The live, aligned actual+04 atomic already exists. Binding adds no
// native allocation, retain, release or store; registry metadata may allocate.
// Binding failure leaves the actual owner untouched and caller-owned.
//
// Bind only once, before exposing the pass to canonical terminal lookup.
// Keep this object and context alive through final release. Once bound, the
// terminal path exclusively owns scalar deletion; do not also call a raw
// deleter. After retired(), external quiescence permits companion destruction.
// The native storage must support its current admitted profile at dispatch.
// Distortion B4F0C0 leaves +34/+38/+3C untouched. Binding never initializes or
// certifies them: terminal dispatch requires established, valid cleanup values,
// including when B4F560 returns false before writing those fields. Constructor
// completion, registration and a capability failure do not prove that contract.
// Nested holders/surfaces/textures retain their existing direct lifetime paths;
// this class does not register them or admit an incomplete resource graph.
class NativeRenderPassReference final : public RenderCommandReference {
public:
    NativeRenderPassReference(void* actual_owner, NativeRenderPassCompanionContext&);
    ~NativeRenderPassReference() override;
    NativeRenderPassReference(const NativeRenderPassReference&) = delete;
    NativeRenderPassReference& operator=(const NativeRenderPassReference&) = delete;
    void release_zero_references() noexcept override;
    bool retired() const noexcept;
private:
    enum class Phase { bound, destroying, retired };
    void* const identity_;
    NativeRenderPassCompanionContext& context_;
    Phase phase_{Phase::bound};
};
} // namespace bsp

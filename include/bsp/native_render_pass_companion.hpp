#pragma once
#include "bsp/native_render_effect_lifetime.hpp"
#include <array>

namespace bsp {
// Host metadata only. Use the SAME actual-owner registry as the effect's
// canonical post-effect children. Profiles in order: D5E164, D5E178, D5E18C,
// D5E1A0, D5E1B4, D61FE0, D62150; each view covers its current first two cells.
struct NativeRenderPassCompanionContext {
    NativeRenderActualOwnerRegistry& registry;
    NativeRenderEffectLifetimeContext& effects;
    std::array<const volatile std::uint32_t*, 7> profiles;
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

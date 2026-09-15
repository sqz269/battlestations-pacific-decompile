#pragma once

#include "bsp/gui_native_geometry.hpp"
#include "bsp/native_frame_target_owner.hpp"
#include "bsp/native_node_destruction.hpp"
#include <atomic>
#include <cstdint>

namespace bsp {

using NativePostEffectDecrement = long (__stdcall*)(volatile long*);
struct NativePostEffectOwnerContext {
    NativeRenderActualOwners& actual_owners;
    NativeNodeDestructionRuntime& nodes;
    NativeFrameTargetOwnerContext& frame_targets;
    // Borrow actual current cells. B4E2F0 captures CE2220 once, after +18.
    NativePostEffectDecrement const volatile& actual_decrement_00ce2220;
    const volatile std::uint32_t* actual_profile_00d61ec8;
    const volatile std::uint32_t* actual_stream_profile_00d61d6c;
    const volatile std::uint32_t* actual_material_profile_00d5e520;
    const volatile std::uint32_t* actual_frame_profile_00d5e600;
};

// A host companion over an already-constructed actual24h owner. The original
// B4E840 producer is evidence, not implemented here. Actual+04 must already
// contain a live atomic<int32_t>; binding never constructs/resets/retains it.
// Member identities, node bindings, profiles and provider contexts must share
// their established actual domains. This adds no native storage or shadow count.
class NativePostEffectOwner final {
public:
    enum class Phase { live, destroying, dead };
    NativePostEffectOwner(void* actual, NativePostEffectOwnerContext&);
    NativePostEffectOwner(const NativePostEffectOwner&) = delete;
    NativePostEffectOwner& operator=(const NativePostEffectOwner&) = delete;
    void* storage() const noexcept { return actual_; }
    Phase phase() const noexcept { return phase_; }
private:
    friend class NativePostEffectReference;
    friend void destroy_native_post_effect_owner_00b4e2f0(NativePostEffectOwner&);
    void* const actual_;
    NativePostEffectOwnerContext& context_;
    Phase phase_{Phase::live};
    bool reference_bound_{};
};

// Full217-byte body: +18 stream, +14 material, +0C/+10 nodes, +1C CRT
// allocation, +08 frame group, base profile. Clear each current field only
// after its captured member returns. The single EH state cleans only the base.
// Direct destruction while a canonical companion is bound is outside the
// contract; such an owner must retire through its actual count-zero terminal.
void destroy_native_post_effect_owner_00b4e2f0(NativePostEffectOwner&);
// Full30-byte body; free iff flags&1 after successful destruction, return the
// original allocation identity even after free. Original ABI ECX/stack flags/RET4.
void* delete_native_post_effect_owner_00b4e450(NativePostEffectOwner&, std::uint32_t flags);

class NativePostEffectReference;
struct NativePostEffectCompanionDisposal {
    void* context;
    void (*retire)(void*, NativePostEffectReference&) noexcept;
};
// Externally stored, unique canonical reference in the SAME geometry registry.
// Registration is transactional and non-retaining. Retirement unbinds without
// reading the freed allocation, then invokes the host-only disposal notification.
// Unbind removes only the registry entry and preserves both host companions
// until the final disposal notification.
class NativePostEffectReference final : public RenderCommandReference {
public:
    NativePostEffectReference(NativePostEffectOwner&, GuiNativeGeometryRegistration,
        NativePostEffectCompanionDisposal);
    ~NativePostEffectReference() override;
    NativePostEffectReference(const NativePostEffectReference&) = delete;
    NativePostEffectReference& operator=(const NativePostEffectReference&) = delete;
    void* storage() const noexcept { return owner_.storage(); }
    void release_zero_references() noexcept override;
private:
    NativePostEffectOwner& owner_;
    GuiNativeGeometryRegistration registration_;
    NativePostEffectCompanionDisposal disposal_;
    bool retired_{};
};

// New source interfaces, not original binary ABI. The inherited canonical
// terminal is noexcept; direct-body C++ cleanup projection does not establish
// native FH3/SEH behavior through that terminal or gameplay equivalence.
} // namespace bsp

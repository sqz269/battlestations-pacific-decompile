#pragma once

#include "bsp/gui_native_geometry.hpp"
#include "bsp/native_frame_target_owner.hpp"
#include "bsp/native_node_destruction.hpp"
#include <atomic>
#include <cstdint>

namespace bsp {

using NativePostEffect20Decrement = long (__stdcall*)(volatile long*);
struct NativePostEffect20OwnerContext {
    NativeRenderActualOwners& actual_owners;
    NativeNodeDestructionRuntime& nodes;
    NativeFrameTargetOwnerContext& frame_targets;
    // Borrow actual current cells. B4E1F0 captures CE2220 once, after material +14.
    NativePostEffect20Decrement const volatile& actual_decrement_00ce2220;
    const volatile std::uint32_t* actual_profile_00d61ec0;
    const volatile std::uint32_t* actual_material_profile_00d5e520;
    const volatile std::uint32_t* actual_frame_profile_00d5e600;
};

// A host companion over an already-constructed actual20h (32-byte) owner. The original
// B4E470 producer is evidence, not implemented here. Actual+04 must already
// contain a live atomic<int32_t>; binding never constructs/resets/retains it.
// Member identities, node bindings, profiles and provider contexts must share
// their established actual domains. This adds no native storage or shadow count.
class NativePostEffect20Owner final {
public:
    enum class Phase { live, destroying, dead };
    NativePostEffect20Owner(void* actual, NativePostEffect20OwnerContext&);
    NativePostEffect20Owner(const NativePostEffect20Owner&) = delete;
    NativePostEffect20Owner& operator=(const NativePostEffect20Owner&) = delete;
    void* storage() const noexcept { return actual_; }
    Phase phase() const noexcept { return phase_; }
private:
    friend class NativePostEffect20Reference;
    friend void destroy_native_post_effect_owner_00b4e1f0(NativePostEffect20Owner&);
    void* const actual_;
    NativePostEffect20OwnerContext& context_;
    Phase phase_{Phase::live};
    bool reference_bound_{};
};

// Full189-byte body: +14 material, +0C/+10 nodes, +18 raw40-byte CRT
// draw record, +08 frame group, base profile. Clear each current field only
// after its captured member returns. The single EH state cleans only the base.
// Direct destruction while a canonical companion is bound is outside the
// contract; such an owner must retire through its actual count-zero terminal.
void destroy_native_post_effect_owner_00b4e1f0(NativePostEffect20Owner&);
// Full30-byte body; free iff flags&1 after successful destruction, return the
// original allocation identity even after free. Original ABI ECX/stack flags/RET4.
void* delete_native_post_effect_owner_00b4e430(NativePostEffect20Owner&, std::uint32_t flags);

class NativePostEffect20Reference;
struct NativePostEffect20CompanionDisposal {
    void* context;
    void (*retire)(void*, NativePostEffect20Reference&) noexcept;
};
// Externally stored, unique canonical reference in the SAME geometry registry.
// Registration is transactional and non-retaining. Retirement unbinds without
// reading the freed allocation, then invokes the host-only disposal notification.
// Unbind removes only the registry entry and preserves both host companions
// until the final disposal notification.
class NativePostEffect20Reference final : public RenderCommandReference {
public:
    NativePostEffect20Reference(NativePostEffect20Owner&, GuiNativeGeometryRegistration,
        NativePostEffect20CompanionDisposal);
    ~NativePostEffect20Reference() override;
    NativePostEffect20Reference(const NativePostEffect20Reference&) = delete;
    NativePostEffect20Reference& operator=(const NativePostEffect20Reference&) = delete;
    void* storage() const noexcept { return owner_.storage(); }
    void release_zero_references() noexcept override;
private:
    NativePostEffect20Owner& owner_;
    GuiNativeGeometryRegistration registration_;
    NativePostEffect20CompanionDisposal disposal_;
    bool retired_{};
};

// New source interfaces, not original binary ABI. The inherited canonical
// terminal is noexcept; direct-body C++ cleanup projection does not establish
// native FH3/SEH behavior through that terminal or gameplay equivalence.
} // namespace bsp

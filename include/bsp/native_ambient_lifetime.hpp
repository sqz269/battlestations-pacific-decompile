#pragma once
#include "bsp/native_render_context.hpp"

namespace bsp {

// Complete B7C450[99B] and B7C7E0[30B] over the SAME actual 98h allocation
// constructed by B7C290. New source ABI; no enlarged ambient/SceneResource
// object, payload allocation, retain, count decrement or registry admission.
// Valid domain: aligned accessible owner; actual12B borrowed-key array at+08
// has nonnegative signed count/capacity, count<=capacity, and null or correctly
// CRT-owned begin storage. Keys are never dereferenced or retained/released.
//
// Reuse the proven B7BC70 count0 path, reread CURRENT begin, and free through
// the existing shared CRT provider. Preserve begin/capacity and every other
// payload byte; stamp D5C104 then CEB130 only AFTER that free returns. The
// source cleanup projection uses genuine AA6E10 on an escaping C++ failure,
// without retrying array cleanup. Native FH3/SEH/CRT fault identity is separate.
void destroy_native_ambient_00b7c450(void* actual_owner);
// Read only the CURRENT low byte of the caller-owned flags cell AFTER the
// complete destructor returns. flags&1 frees the owner; flags0 ends payload
// lifetime but leaves raw storage. Return numeric identity, possibly freed.
// Flags may alias still-accessible owner fields, including the array count.
void* delete_native_ambient_00b7c7e0(void* actual_owner,
    const volatile std::uint32_t& actual_flags_slot);

struct NativeAmbientIdentityContext {
    NativeRenderActualOwnerRegistry& registry;
    // Borrow CURRENT D62F3C first two native-code-token cells. No default or
    // unknown-profile fallback. Context/profile must survive terminal dispatch.
    const volatile std::uint32_t* ambient_profile_00d62f3c;
};

// Separate postconstruction host metadata, never a call inside native B7C290.
// Borrow the already-live actual+04 atomic; same canonical registry as all
// holders of this owner. Bind changes no native byte/count; duplicate refusal
// leaves the owner unchanged. No logical SceneResource/ambient cast is used.
// Keep metadata stable and externally quiescent through dispatch/retirement;
// prevent storage reuse before registry retirement. Once bound, both terminal
// and explicit scalar deletion go through this companion, not a second path.
class NativeAmbientReference final : public RenderCommandReference {
public:
    NativeAmbientReference(void* actual_owner, NativeAmbientIdentityContext&);
    ~NativeAmbientReference() override;
    NativeAmbientReference(const NativeAmbientReference&) = delete;
    NativeAmbientReference& operator=(const NativeAmbientReference&) = delete;
    // Actual count is already zero. Current0 must be BD30E0; that helper
    // reloads the owner profile and invokes current4 B7C7E0 with flags1.
    void release_zero_references() noexcept override;
    // No count test/decrement. Nonzero counts and flags0 are supported.
    // Retire/unbind after flags0 or1, without raw reads after possible free.
    // An entered destruction failure also retires once and escapes, leaving
    // partial payload/physical storage caller-owned; no rollback or retry.
    void* delete_scalar_00b7c7e0(const volatile std::uint32_t& actual_flags_slot);
    bool retired() const noexcept;
private:
    enum class Phase { bound, destroying, retired };
    void* const identity_;
    NativeAmbientIdentityContext& context_;
    Phase phase_{Phase::bound};
    void retire() noexcept;
};

} // namespace bsp

#pragma once
#include "bsp/native_directional_light_construction.hpp"
#include "bsp/native_light_lifetime.hpp"
#include "bsp/native_point_light_pool.hpp"

namespace bsp {
// B7C710[45], ECX actual200h slot, raw8B name header on stack; EAX same slot,
// RET4. Genuine rawB7C4C0/current constants then D63008 and1E0/1E4/1E8 zeros.
// B6F5A0 establishes actual174h prefix/atomic04. Establishes only the trivial
// scene178 descriptor (complete preimage preserved) and actual1E0 backlink
// descriptor lifetime; no logical point owner, binding or new credit. Preserve
// native-unwritten1EC..1FB and live pool1FC. Failure keeps caller slot recovery;
// base construction supplies its own one-shot cleanup, no new native EH here.
void* construct_native_point_light_raw_00b7c710(void* actual_slot,
    std::size_t slot_bytes, const void* actual_name_header,
    NativeStringRawPoolContext&, const NativeNodeRawConstants&,
    const volatile std::uint32_t& sixty_four_00ce7820);

struct NativePointLightStorageContext {
    NativeLightLifetimeContext& light;
    NativePointLightLinksBinding& backlinks;
    // Caller-owned, address-stable actual+1E0 binding already registered in
    // light.node.trees.point_lights. Same genuine allocation provenance and
    // actual scene/tree owner registry/current decrement cell. No copy/adopt.
};
struct NativePointLightStorageFrame { NativeLightLifetimeFrame light; };
struct NativePointLightStorageAcquired {
    bool started{}, complete{}, exception_cleanup_started{};
    std::int32_t native_eh_state{-1};
    std::uint32_t active_call_site{};
    bool backlinks_unlinked{}, array_cleanup_completed{};
    bool backlink_binding_retired{}, pool_returned{};
    NativeLightLifetimeAcquired light;
};

// 5A1610[23], ECX actual12B descriptor, RET. Proven59FCE0(0) domain: no
// growth, current count decrement/store0, capture CURRENT begin, real free.
// Preserve dangling begin/capacity. Nonnegative count<=capacity, accessible
// backing from SAME runtime, live descriptor and quiescence are required.
void destroy_native_point_light_array_storage_005a1610(
    NativePointLightBacklinkArray&, NativePointLightLinksRuntime&);
// B7C770[112], ECX actual200h owner, RET. StampD63008; state1 unlink actual
// nodes, state0 array cleanup, state-1 rawLight. Existing genuine B7C160 and
// rawB7C5B0 providers; no legacy NativePointLightOwner/SceneAttachmentRuntime.
// Separate HOST metadata step retires the backlink association after its
// cleanup while count0/payload remain live, BEFORE Light/pool return. This is
// not a native call/credit. Pre-boundary failure retains caller's binding;
// frames/acquisitions/backing and unresolved credits require disposition.
void destroy_native_point_light_storage_00b7c770(void* actual_light,
    NativePointLightStorageFrame&, NativePointLightStorageContext&,
    NativePointLightStorageAcquired&);
// B7C850[32], ECX actual owner, stacked flags; EAX original identity, RET4.
// No extra entry profile store. After destruction read CURRENT flags lowbyte;
// bit0 returns SAME200h slot via real0109011C pool/current1FC. No later raw
// payload/count access. Explicit scalar permits positive count and flags0.
void* delete_native_point_light_storage_00b7c850(void* actual_light,
    const volatile std::uint32_t& flags, NativePointLightPool&,
    NativePointLightStorageFrame&, NativePointLightStorageContext&,
    NativePointLightStorageAcquired&);

struct NativePointLightStorageIdentityContext {
    NativePointLightStorageContext& lifetime;
    NativeRenderActualOwnerRegistry& owners;
    NativePointLightPool& pool_0109011c;
    const volatile std::uint32_t* actual_profile_00d63008;
};
// Separate postconstruction canonical metadata over the SAME actual+4, no
// stores/retain. Caller guarantees completed raw200h constructor, genuine pool
// provenance and current profiles. Borrowed backlink binding remains external
// and survives pre-retirement failure even if this canonical metadata retires.
// Pure membership validation checks live1FC, SAME slab/200h slot identity and
// current4000h free indices/4040h count before atomic/registry admission; wrong
// pool or returned slot is refused without native writes. Quiescence required.
// Duplicate refusal leaves native bytes and existing backlink binding intact.
// All destruction of an admitted owner must pass through this reference.
class NativePointLightStorageReference final : public RenderCommandReference {
public:
    NativePointLightStorageReference(void* actual_light,
        NativePointLightStorageIdentityContext&, NativePointLightStorageFrame&,
        NativePointLightStorageAcquired&);
    ~NativePointLightStorageReference() override;
    NativePointLightStorageReference(const NativePointLightStorageReference&) = delete;
    NativePointLightStorageReference& operator=(const NativePointLightStorageReference&) = delete;
    // SAME observed actual count0/current0BD30E0 -> fresh current4B7C850,
    // flags1. A thrown source failure terminates at this noexcept boundary.
    void release_zero_references() noexcept override;
    void* delete_scalar_00b7c850(const volatile std::uint32_t& flags);
    bool retired() const noexcept;
private:
    enum class Phase { bound, destroying, retired };
    void* const identity_;
    NativePointLightStorageIdentityContext& context_;
    NativePointLightStorageFrame& frame_;
    NativePointLightStorageAcquired& acquired_;
    Phase phase_{Phase::bound};
    void retire() noexcept;
};
// Fresh initialized persistent frame/acquisition only; source refuses retries.
// Canonical retirement/unbind is metadata-only and never proves destruction or
// slot return completed on failure. Retired companion destruction and registry
// unbind do not read payload. No reentry/slot reuse until retirement finishes.
// Existing rawLight valid-current-array/gate/count contract applies. Null
// shadow or nonzero remaining count closes normally; reached zero shadow still
// needs its genuine family-aware provider. No invented shadow owner/default.
// Source normal paths/cleanup scheduling only, not native FH3/SEH/private-stack
// ABI, full terminal graph, application admission or game compatibility.
} // namespace bsp

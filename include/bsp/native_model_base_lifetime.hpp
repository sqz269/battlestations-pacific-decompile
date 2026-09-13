#pragma once

#include "bsp/native_node_destruction.hpp"

namespace bsp {

// Complete B74B60..B74B7F: ECX actual node, stack flags, EAX captured original
// address, RET4. Destroy through B6F440, return to actual0109008C iff flags&1.
// Native has no local unwind and does not return a slot after destructor failure.
// The same constructed name storage is explicit. This host binding forgets the
// dead scene association before returning physical storage, including after the
// destructor's member cleanup on exception. No logical release is implied.
// For an unbound node only: once a reference below owns terminal dispatch, its
// zero callback must perform deletion. The caller removes any other companions.
void* delete_native_model_base_00b74b60(NativeNodeDestructionRuntime&,
    NativeNodeBinding&, NativeStringRawPoolContext& actual_name_pool,
    void* actual_pool_0109008c, std::uint32_t flags);

// B743C0..B743D8, ECX actual174h owner, stack actual8h name header, RET4.
// Calls the actual-header/current-constant B6F5A0 provider, then stamps D62D78.
NativeNodeStorage& construct_native_model_base_00b743c0(void* actual_slot,
    std::size_t slot_bytes, const void* actual_name_header,
    NativeStringRawPoolContext&, const NativeNodeRawConstants&);

// B74EC0 ignores incoming size ECX=174h, selects actual0109008C and tailcalls
// B6EB00. B748D0 receives the actual slot in ECX and returns it through B6E490.
void* allocate_native_model_base_00b74ec0(void* actual_pool_0109008c);
void return_native_model_base_00b748d0(void* actual_slot, void* actual_pool_0109008c);

// B86720..B8677E, stack actual name header, RET4; incoming ECX is unused.
// Uses actual initialized38h pool0109008C (CD7F20 -> B6E980), actual178h slot,
// current raw strings/constants. Null allocation returns null. Constructor failure
// returns that exact slot through B748D0 (FH3 CC2530), then propagates C++ failure.
// No companion is synthesized: caller binds the returned actual owner once and
// registers a stable NativeModelBaseReference in its existing lifetime runtime.
NativeNodeStorage* create_native_model_base_00b86720(void* actual_pool_0109008c,
    const void* actual_name_header, NativeStringRawPoolContext&,
    const NativeNodeRawConstants&);

// B743E0..B74407, stack token, AL bool, RET4; ECX is unused.
// The descriptor's three actual current cells must already be initialized.
struct NativeModelBaseTypeTokens {
    const volatile std::uint32_t& actual_01090044;
    const volatile std::uint32_t& actual_01090048;
    const volatile std::uint32_t& actual_0109004c;
};
bool native_model_base_is_type_00b743e0(
    const NativeModelBaseTypeTokens&, std::uint32_t token) noexcept;

class NativeModelBaseReference;
struct NativeModelBaseCompanionDisposal {
    void* context;
    // Called after node destruction, scene/lifetime unbinding and pool return.
    // Retire the canonical actual-owner lookup and any owning host companions.
    // May destroy this reference and NativeNodeBinding; no access follows it.
    void (*retire)(void*, NativeModelBaseReference&) noexcept;
};

// Canonical companion over the existing physical174h node and actual+04 count.
// Constructor does not retain/reset it or create another transform/scene map.
// The node must already be constructed with the supplied string storage and
// registered in the SAME SceneAttachmentRuntime. After successful lifetime
// registration, this binds the verified model-base type/matrix/scene callbacks
// to that same companion. The supplied table is the actual current D62D78 view (22 DWORDs);
// integer entries validate dispatch identities and are never called as code.
// Actual pool/storage/runtime/table/token cells and stable binding outlive callbacks.
// This companion owns scene_attachment.context while live and restores its
// original node-phase context immediately before B6F440 terminal transition.
class NativeModelBaseReference final : public GeneratedModelNodeLifetime,
    public RenderCommandReference {
public:
    NativeModelBaseReference(NativeNodeBinding&, NativeNodeDestructionRuntime&,
        NativeStringRawPoolContext& actual_name_pool, void* actual_pool_0109008c,
        const volatile std::uint32_t* actual_vtable_00d62d78,
        NativeModelBaseTypeTokens, NativeModelBaseCompanionDisposal);
    ~NativeModelBaseReference() override;
    NativeModelBaseReference(const NativeModelBaseReference&) = delete;
    NativeModelBaseReference& operator=(const NativeModelBaseReference&) = delete;
    NativeNodeBinding& node_binding() noexcept { return node_; }
    CameraTransform& transform() noexcept override { return node_.transform; }
    SceneNodeAttachment& scene_attachment() noexcept override { return node_.scene_attachment; }
    void remove_scene_virtual54(SceneResource*, bool recurse) noexcept override;
    void release_model_virtual18_00b6f310() noexcept override;
    void release_zero_references() noexcept override;
private:
    enum class Phase { bound, destroying, retired };
    NativeNodeBinding& node_;
    NativeNodeDestructionRuntime& nodes_;
    NativeStringRawPoolContext& strings_;
    void* pool_;
    const volatile std::uint32_t* table_;
    NativeModelBaseTypeTokens tokens_;
    void* const node_phase_context_;
    NativeModelBaseCompanionDisposal disposal_;
    static bool is_type(SceneAttachmentRuntime&, SceneNodeAttachment&, std::uint32_t);
    Phase phase_{Phase::bound};
    void require_slot(std::uint32_t offset, std::uint32_t address) const noexcept;
    static std::uint32_t light_count(void*) noexcept;
    static void remove_light_backlink(void*, std::uint32_t, CameraTransform&) noexcept;
    static void shrink_lights(void*) noexcept;
};

// The native terminal domain must satisfy the existing nonthrowing intrusive
// interface. Missing profiles/bindings or C++ exceptions there terminate; no
// fallback destructor or successful no-op is supplied. This is a new C++ ABI.
} // namespace bsp

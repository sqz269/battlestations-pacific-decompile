#pragma once

#include "bsp/native_node_destruction.hpp"

namespace bsp {

// Complete B6F8D0..B6F8EF: ECX actual node, stack flags, EAX captured original
// address, RET4. Destroy through B6F440, return to actual0108FF58 iff flags&1.
// Native has no local unwind and does not return a slot after destructor failure.
// The same constructed name storage is explicit. This host binding forgets the
// dead scene association before returning physical storage, including after the
// destructor's member cleanup on exception. No logical release is implied.
// For an unbound node only: once a reference below owns terminal dispatch, its
// zero callback must perform deletion. The caller removes any other companions.
void* delete_native_plain_node_00b6f8d0(NativeNodeDestructionRuntime&,
    NativeNodeBinding&, NativeStringStorage& actual_name_storage,
    void* actual_pool_0108ff58, std::uint32_t flags);

class NativePlainNodeReference;
struct NativePlainNodeCompanionDisposal {
    void* context;
    // Called after node destruction, scene/lifetime unbinding and pool return.
    // Retire the canonical actual-owner lookup and any owning host companions.
    // May destroy this reference and NativeNodeBinding; no access follows it.
    void (*retire)(void*, NativePlainNodeReference&) noexcept;
};

// Canonical companion over the existing physical174h node and actual+04 count.
// Constructor does not retain/reset it or create another transform/scene map.
// The node must already be constructed with the supplied string storage and
// registered in the SAME SceneAttachmentRuntime. After successful lifetime
// registration, this binds the verified plain-node type/matrix/scene callbacks
// to that same companion. The supplied table is the actual current D62C88 view (22 DWORDs);
// integer entries validate dispatch identities and are never called as code.
// Actual pool/storage/runtime/table and stable node binding outlive callbacks.
class NativePlainNodeReference final : public GeneratedModelNodeLifetime,
    public RenderCommandReference {
public:
    NativePlainNodeReference(NativeNodeBinding&, NativeNodeDestructionRuntime&,
        NativeStringStorage& actual_name_storage, void* actual_pool_0108ff58,
        const volatile std::uint32_t* actual_vtable_00d62c88,
        NativePlainNodeCompanionDisposal);
    ~NativePlainNodeReference() override;
    NativePlainNodeReference(const NativePlainNodeReference&) = delete;
    NativePlainNodeReference& operator=(const NativePlainNodeReference&) = delete;
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
    NativeStringStorage& strings_;
    void* pool_;
    const volatile std::uint32_t* table_;
    NativePlainNodeCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    void require_slot(std::uint32_t offset, std::uint32_t address) const noexcept;
    static std::uint32_t light_count(void*) noexcept;
    static GeneratedModelPointLightLinks& light_element(void*, std::uint32_t) noexcept;
    static void shrink_lights(void*) noexcept;
};

// The native terminal domain must satisfy the existing nonthrowing intrusive
// interface. Missing profiles/bindings or C++ exceptions there terminate; no
// fallback destructor or successful no-op is supplied. This is a new C++ ABI.
} // namespace bsp

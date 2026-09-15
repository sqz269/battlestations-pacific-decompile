#pragma once
#include "bsp/gui_page_root.hpp"
#include "bsp/light_type_bootstrap.hpp"
#include "bsp/native_group_pool.hpp"
#include "bsp/native_node_parenting.hpp"

namespace bsp {
// Canonical process descriptor0109032C. No default stores replace preimages.
struct NativeGroupTypeDescriptor {
    std::uint32_t own_id;
    std::uint32_t node_id;
    std::uint32_t root_id;
    std::uint32_t native_name_address;
};
struct NativeGroupTypeStorage {
    volatile std::uint8_t& guard_010902e1;
    volatile NativeGroupTypeDescriptor& group_0109032c;
};
class NativeGroupTypes final {
public:
    NativeGroupTypes(TypeIdCounterLifetime&, LightTypeBootstrap&, NativeGroupTypeStorage) noexcept;
    std::uint32_t type_id_00b8e620() const noexcept;
    bool is_type_00b8f650(std::uint32_t token) const noexcept;
    // ECX target descriptor, RET. Shared process guard is set before parent
    // initialization; failure does not roll it back. No automatic lazy call.
    void initialize_00b8f590(volatile NativeGroupTypeDescriptor& target);
    NativeGroupTypeStorage storage() const noexcept { return storage_; }
private:
    TypeIdCounterLifetime& counter_;
    LightTypeBootstrap& shared_types_;
    NativeGroupTypeStorage storage_;
};

struct NativeGroupEnvironment {
    NativeGroupPool& pool_010902f4;
    NativeNodeDestructionRuntime& nodes;
    NativeGroupTypes& types;
    NativeGroupConstants constants;
    // Current native tables, at least22 DWORDs each; no default profile.
    const volatile std::uint32_t* vtable_00d634f8;
    const volatile std::uint32_t* vtable_00d62c88;
};
class NativeGroupOwner final {
public:
    // Stable companion over one actual18Ch slot. Preserve all allocation bytes;
    // bind the SAME scene/hierarchy, reference count and actual backlink array.
    NativeGroupOwner(void* actual_slot, std::size_t slot_bytes, NativeGroupEnvironment&);
    // Adopt one already constructed B8F5E0 group, such as the actual owner
    // returned by B86780. Its prefix/tail lifetimes already exist. Register
    // the canonical scene/backlink companions without reconstructing, retaining,
    // copying or changing native storage. Binding failure leaves the constructed
    // owner alive for its caller; the supplied name domain must match construction.
    NativeGroupOwner(NativeGroupStorageView actual_constructed, NativeGroupEnvironment&);
    ~NativeGroupOwner();
    NativeGroupOwner(const NativeGroupOwner&) = delete;
    NativeGroupOwner& operator=(const NativeGroupOwner&) = delete;
    enum class Phase { prepared, constructing, live, dead };
    NativeGroupStorageView storage;
    NativeGroupEnvironment& environment;
    NativeNodeBinding node;
    GeneratedModelAttachmentLinks attached_nodes;
    Phase phase{Phase::prepared}; // host bookkeeping, not another native field
};

// Complete existing raw constructor plus installation of actual group dispatch.
void* construct_native_group_00b8f5e0(NativeGroupOwner&, const NativeString&);
// ECX group, RET. Shrink count to0 WITHOUT clearing node+A0, free actual178,
// then complete B6F440. Never releases174 or184. Leaves freed pointer/capacity.
void destroy_native_group_00b8f680(NativeGroupOwner&);
// ECX group, flags on stack, EAX original slot, RET4; pool return iff flags&1.
void* delete_native_group_00b8f8c0(NativeGroupOwner&, std::uint32_t flags);
// Current group virtual1C/3C/40 adapters. 1C uses the caller's canonical routing
// runtime; the GUI dispatcher maps actual identity to this stable companion.
void native_group_attachment_virtual1c(NativeGroupOwner&, NativeNodeParentingRuntime&, void* actual_group);
void notify_native_group_bounds_00b6dbc0(NativeGroupOwner&);
void native_group_world_changed_00b8e6b0(SceneAttachmentRuntime&, SceneNodeAttachment&);

// Actual DWORD-pointer array helpers, ECX descriptor, stack signed count, RET4.
// Allocation is real CRT new-handler allocation/free via singleton_lifetime.
// Valid extents are required; overflow/malformed extents are adapter errors.
void reserve_native_group_attached_nodes_0059e5e0(NativeGroupTailStorage&, std::int32_t capacity);
void resize_native_group_attached_nodes_0059fce0(NativeGroupTailStorage&, std::int32_t count);

class NativeGroupReference;
struct NativeGroupCompanionDisposal {
    void* context;
    void (*retire)(void*, NativeGroupReference&) noexcept;
};
class NativeGroupReference final : public GeneratedModelNodeLifetime, public RenderCommandReference {
public:
    // Borrow actual+04; do not increment it. One canonical reference companion.
    NativeGroupReference(NativeGroupOwner&, NativeGroupCompanionDisposal);
    ~NativeGroupReference() override;
    NativeGroupReference(const NativeGroupReference&) = delete;
    NativeGroupReference& operator=(const NativeGroupReference&) = delete;
    NativeGroupOwner& group_owner() noexcept { return owner_; }
    CameraTransform& transform() noexcept override { return owner_.node.transform; }
    SceneNodeAttachment& scene_attachment() noexcept override { return owner_.node.scene_attachment; }
    void remove_scene_virtual54(SceneResource*, bool recurse) noexcept override;
    // Interface name inherited from existing runtime; group current+18 is
    // B8EEC0, which empties backlinks BEFORE tail-calling shared B6F310.
    void release_model_virtual18_00b6f310() noexcept override;
    void release_zero_references() noexcept override;
private:
    enum class Phase { bound, destroying, retired };
    NativeGroupOwner& owner_;
    GeneratedModelLifetimeRuntime& runtime_;
    NativeGroupCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    static std::uint32_t light_count(void*) noexcept;
    static void remove_light_backlink(void*, std::uint32_t, CameraTransform&) noexcept;
    static void shrink_lights(void*) noexcept;
};
} // namespace bsp

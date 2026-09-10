#pragma once
#include "bsp/camera_transform.hpp"
#include "bsp/system_lighting_constants.hpp"
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace bsp {
struct SceneNodeAttachment;
struct SceneResource;
class SceneAttachmentRuntime;
enum class SceneRegistryInitialization { immediate, deferred };

// New C++ storage, not a native object overlay. The registry owns its list nodes
// and borrows the attachment bindings. Keys are original-width pointer values;
// use the stable binding's Win32 address when no native node exists.
class SceneNodeRegistry {
public:
    explicit SceneNodeRegistry(SceneRegistryInitialization = SceneRegistryInitialization::immediate);
    ~SceneNodeRegistry();
    SceneNodeRegistry(const SceneNodeRegistry&) = delete;
    SceneNodeRegistry& operator=(const SceneNodeRegistry&) = delete;
    // Deferred construction lets the actual SceneResource copy its name before
    // allocating the registry. The canonical sentinel retains allocator+8 bytes.
    void initialize_native_00b83600();
    void destroy_native_bucket_storage_00b82ed0() noexcept;
    void destroy_native_list_storage_00b829d0() noexcept;
    const void* native_sentinel() const noexcept { return list_.head; }
    static const void* native_next(const void*) noexcept;
    static std::uint32_t native_key(const void*) noexcept;
    SceneNodeAttachment* binding_for_key(std::uint32_t) const noexcept;
    bool insert_00b83700(SceneNodeAttachment&);
    std::uint32_t erase_00b83e50(std::uint32_t pointer_key);
    bool contains(std::uint32_t pointer_key) const;
    std::vector<SceneNodeAttachment*> members() const;
    std::uint32_t size() const noexcept { return list_.size; }
    std::uint32_t bucket_count() const noexcept { return active_buckets_; }
    std::uint32_t mask() const noexcept { return mask_; }
private:
    struct Entry {
        Entry* next;
        Entry* previous;
        std::uint32_t key;
    };
    struct Iterator {
        const void* owner;
        Entry* node;
    };
    struct ListOwner {
        std::uint32_t untouched_00;
        Entry* head{};
        std::uint32_t size{};
    } list_;
    Iterator* boundaries_{};
    std::uint32_t boundary_count_{};
    std::uint32_t boundary_capacity_{};
    // Associations contain no ordering links and never supply the sentinel key.
    std::unordered_map<std::uint32_t, SceneNodeAttachment*> bindings_;
    std::uint32_t mask_{1};
    std::uint32_t active_buckets_{1};
    std::uint32_t bucket_for(std::uint32_t key) const noexcept;
    void split_before_insert();
    void clear_00b83be0();
    void resize_boundaries(std::uint32_t);
    void require_initialized() const;
};

// Native +4 reference counter and virtual+0 on transition to zero. The callback
// implements the actual owner destruction policy; it may reenter attachment.
// No implicit retain, release, or detachment occurs in C++ destructors.
struct SceneResource {
    SceneResource(std::int32_t initial_references, void (*on_zero)(SceneResource&),
        void* destruction_context = nullptr,
        SceneRegistryInitialization = SceneRegistryInitialization::immediate);
    std::atomic<std::int32_t> references;
    SceneNodeRegistry registry; // native scene+14: cLight and derived types
    void (*destroy_on_zero)(SceneResource&);
    void* context;
};

using SceneTypePredicate = bool (*)(SceneAttachmentRuntime&, SceneNodeAttachment&, std::uint32_t);
using SceneAttachOverride = void (*)(SceneAttachmentRuntime&, SceneNodeAttachment&, SceneResource*, bool);

struct SceneNodeAttachment {
    SceneNodeAttachment(CameraTransform&, std::uint32_t pointer_key,
        SceneTypePredicate virtual_0c, SceneAttachOverride virtual_50, void* context = nullptr);
    SceneNodeAttachment(const SceneNodeAttachment&) = delete;
    SceneNodeAttachment& operator=(const SceneNodeAttachment&) = delete;
    CameraTransform& transform; // same native +34/+3C hierarchy as camera/bounds
    const std::uint32_t pointer_key;
    SceneResource* scene{}; // native node+170, initially detached in this interface
    SceneTypePredicate is_type;
    SceneAttachOverride attach_scene;
    void* context;
    // Explicit projection of this SAME native key when its concrete owner is a
    // directional light. Null is unbound and must not become a light fallback.
    SystemDirectionalLight* system_directional_light{};
};

// Caller supplies recovered runtime values, not the addresses of the globals.
// 0109018C is cLight's unique ID. 006EF860 accepts c3dObject/c3dNode/cRoot;
// it rejects cLight after type initialization. A node is registered only when
// its actual virtual+0C accepts cLight. Bindings and transforms must stay stable and
// alive until explicitly detached and unbound. All traversed children must be
// bound. Hierarchy mutation/reentry is allowed; concurrent mutation is not.
class SceneAttachmentRuntime : public SystemDirectionalLightResolver {
public:
    SceneAttachmentRuntime(std::uint32_t registry_token,
        std::array<std::uint32_t, 3> object_tokens);
    void bind(SceneNodeAttachment&);
    void unbind(SceneNodeAttachment&); // requires explicit prior detach
    SceneNodeAttachment& resolve(CameraTransform&) const;
    SystemDirectionalLight* resolve_light(std::uint32_t actual_key) override;
    std::uint32_t registry_type_token;
    std::array<std::uint32_t, 3> object_type_tokens; // c3dObject/c3dNode/cRoot, 01090034/38/3C
private:
    std::vector<SceneNodeAttachment*> bindings_;
};

std::uint32_t scene_pointer_hash(std::uint32_t pointer_key) noexcept;
std::uint32_t get_scene_registry_type_00b7aa70(const SceneAttachmentRuntime&) noexcept;
bool object_accepts_scene_type_006ef860(SceneAttachmentRuntime&, SceneNodeAttachment&, std::uint32_t);
void add_scene_node_if_type_00b83d50(SceneAttachmentRuntime&, SceneResource&, SceneNodeAttachment&);
void remove_scene_node_if_type_00b83ec0(SceneAttachmentRuntime&, SceneResource&, SceneNodeAttachment&);
// Native thiscall, ECX=node, stack scene and recursion byte, RET 8. This new
// interface preserves detach/reload/publish/retain/release/reload/add and child
// virtual+50 dispatch followed by a fresh next_sibling load. No cache flag writes.
void set_node_scene_00b6ed80(SceneAttachmentRuntime&, SceneNodeAttachment&, SceneResource*, bool recurse);
}

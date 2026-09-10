#pragma once
#include "bsp/camera_transform.hpp"
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace bsp {
struct SceneNodeAttachment;
struct SceneResource;
class SceneAttachmentRuntime;

// New C++ storage, not a native object overlay. The registry owns its list nodes
// and borrows the attachment bindings. Keys are original-width pointer values;
// use the stable binding's Win32 address when no native node exists.
class SceneNodeRegistry {
public:
    SceneNodeRegistry();
    ~SceneNodeRegistry();
    SceneNodeRegistry(const SceneNodeRegistry&) = delete;
    SceneNodeRegistry& operator=(const SceneNodeRegistry&) = delete;
    bool insert_00b83700(SceneNodeAttachment&);
    std::uint32_t erase_00b83e50(std::uint32_t pointer_key);
    bool contains(std::uint32_t pointer_key) const;
    std::vector<SceneNodeAttachment*> members() const;
    std::uint32_t size() const noexcept { return size_; }
    std::uint32_t bucket_count() const noexcept { return active_buckets_; }
    std::uint32_t mask() const noexcept { return mask_; }
private:
    struct Entry {
        Entry* next{};
        Entry* previous{};
        std::uint32_t key{};
        SceneNodeAttachment* binding{}; // host association; native stores key only
    };
    Entry head_;
    std::vector<Entry*> boundaries_; // native iterator pairs reduce to stable nodes
    std::uint32_t size_{};
    std::uint32_t mask_{1};
    std::uint32_t active_buckets_{1};
    std::uint32_t bucket_for(std::uint32_t key) const noexcept;
    void split_before_insert();
    void clear_00b83be0();
};

// Native +4 reference counter and virtual+0 on transition to zero. The callback
// implements the actual owner destruction policy; it may reenter attachment.
// No implicit retain, release, or detachment occurs in C++ destructors.
struct SceneResource {
    SceneResource(std::int32_t initial_references, void (*on_zero)(SceneResource&),
        void* destruction_context = nullptr);
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
};

// Caller supplies recovered runtime values, not the addresses of the globals.
// 0109018C is cLight's unique ID. 006EF860 accepts c3dObject/c3dNode/cRoot;
// it rejects cLight after type initialization. A node is registered only when
// its actual virtual+0C accepts cLight. Bindings and transforms must stay stable and
// alive until explicitly detached and unbound. All traversed children must be
// bound. Hierarchy mutation/reentry is allowed; concurrent mutation is not.
class SceneAttachmentRuntime {
public:
    SceneAttachmentRuntime(std::uint32_t registry_token,
        std::array<std::uint32_t, 3> object_tokens);
    void bind(SceneNodeAttachment&);
    void unbind(SceneNodeAttachment&); // requires explicit prior detach
    SceneNodeAttachment& resolve(CameraTransform&) const;
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

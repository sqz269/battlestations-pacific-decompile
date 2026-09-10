#include "bsp/scene_attachment.hpp"
#include <algorithm>
#include <memory>
#include <stdexcept>

namespace bsp {
std::uint32_t scene_pointer_hash(std::uint32_t pointer_key) noexcept {
    // 00C03DBE is signed div: EAX quotient, EDX remainder. Preserve the
    // caller's wrapping IMUL/SUB and sign-bit correction without signed UB.
    const auto bits = pointer_key ^ 0xdeadbeefu;
    const auto dividend = bits < 0x80000000u ? static_cast<std::int64_t>(bits)
        : static_cast<std::int64_t>(bits) - 0x100000000ll;
    const auto quotient = dividend / 127773;
    const auto remainder = dividend % 127773;
    auto hash = static_cast<std::uint32_t>(remainder * 16807 - quotient * 2836);
    if (hash & 0x80000000u) hash += 0x7fffffffu;
    return hash;
}

SceneNodeRegistry::SceneNodeRegistry() : boundaries_(9, &head_) {
    head_.next = head_.previous = &head_;
}
SceneNodeRegistry::~SceneNodeRegistry() {
    for (auto* entry = head_.next; entry != &head_;) {
        auto* next = entry->next;
        delete entry;
        entry = next;
    }
}
std::uint32_t SceneNodeRegistry::bucket_for(std::uint32_t key) const noexcept {
    auto bucket = scene_pointer_hash(key) & mask_;
    if (bucket >= active_buckets_) bucket -= (mask_ >> 1) + 1;
    return bucket;
}
void SceneNodeRegistry::split_before_insert() {
    // Growth precedes the duplicate check, including a duplicate insertion.
    if (active_buckets_ > (size_ >> 2)) return;
    if (active_buckets_ < boundaries_.size() - 1) {
        if (mask_ < active_buckets_) mask_ = mask_ * 2 + 1;
    } else {
        mask_ = static_cast<std::uint32_t>(boundaries_.size()) * 2 - 3;
        boundaries_.resize(static_cast<std::size_t>(mask_) + 2, &head_);
    }
    const auto source = active_buckets_ - (mask_ >> 1) - 1;
    auto* entry = boundaries_[source];
    while (entry != boundaries_[source + 1]) {
        if ((scene_pointer_hash(entry->key) & mask_) == source) {
            entry = entry->next;
            continue;
        }
        auto* next = entry->next;
        if (next != &head_) {
            for (auto bucket = source;; --bucket) {
                if (boundaries_[bucket] != entry) break;
                boundaries_[bucket] = next;
                if (bucket == 0) break;
            }
            entry->previous->next = next;
            next->previous = entry->previous;
            entry->next = &head_;
            entry->previous = head_.previous;
            head_.previous->next = entry;
            head_.previous = entry;
            boundaries_[active_buckets_ + 1] = &head_;
        }
        for (auto bucket = active_buckets_; bucket > source; --bucket) {
            if (boundaries_[bucket] != &head_) break;
            boundaries_[bucket] = entry;
        }
        entry = next;
    }
    ++active_buckets_;
}
bool SceneNodeRegistry::insert_00b83700(SceneNodeAttachment& binding) {
    split_before_insert();
    auto bucket = bucket_for(binding.pointer_key);
    auto* position = boundaries_[bucket + 1];
    while (position != boundaries_[bucket]) {
        position = position->previous;
        if (binding.pointer_key >= position->key) {
            if (binding.pointer_key == position->key) return false;
            position = position->next;
            break;
        }
    }
    auto entry = std::make_unique<Entry>(); // native 00B823B0 copies a raw key
    if (size_ == 0x3fffffffu) throw std::length_error("list<T> too long");
    ++size_; // 00B82D30 is a checked size increment, not always a throw
    entry->next = position;
    entry->previous = position->previous;
    entry->key = binding.pointer_key;
    entry->binding = &binding;
    auto* inserted = entry.release();
    position->previous->next = inserted;
    position->previous = inserted;
    for (;;) {
        if (boundaries_[bucket] != position) break;
        boundaries_[bucket] = inserted;
        if (bucket == 0) break;
        --bucket;
    }
    return true;
}
void SceneNodeRegistry::clear_00b83be0() {
    auto* entry = head_.next;
    head_.next = head_.previous = &head_;
    size_ = 0;
    while (entry != &head_) {
        auto* next = entry->next;
        delete entry;
        entry = next;
    }
    boundaries_.resize(9);
    std::fill(boundaries_.begin(), boundaries_.end(), &head_);
    mask_ = active_buckets_ = 1;
}
std::uint32_t SceneNodeRegistry::erase_00b83e50(std::uint32_t key) {
    auto bucket = bucket_for(key);
    auto* first = boundaries_[bucket];
    auto* end = boundaries_[bucket + 1];
    while (first != end && first->key < key) first = first->next;
    if (first == end || first->key != key) return 0;
    // Unique insertion makes native equal_range/count at most one. The whole
    // list range goes through clear, resetting bucket growth even after erasure.
    if (first == head_.next && first->next == &head_) {
        clear_00b83be0();
        return 1;
    }
    for (;;) {
        if (boundaries_[bucket] != first) break;
        boundaries_[bucket] = first->next;
        if (bucket == 0) break;
        --bucket;
    }
    first->previous->next = first->next;
    first->next->previous = first->previous;
    delete first; // 00B827C0 frees only the list entry, never the borrowed node
    --size_; // native continuation at 00B828DD, omitted from old pseudocode
    return 1;
}
bool SceneNodeRegistry::contains(std::uint32_t key) const {
    const auto bucket = bucket_for(key);
    auto* entry = boundaries_[bucket];
    const auto* end = boundaries_[bucket + 1];
    while (entry != end && entry->key < key) entry = entry->next;
    return entry != end && entry->key == key;
}
std::vector<SceneNodeAttachment*> SceneNodeRegistry::members() const {
    std::vector<SceneNodeAttachment*> result;
    result.reserve(size_);
    for (auto* entry = head_.next; entry != &head_; entry = entry->next)
        result.push_back(entry->binding);
    return result;
}

SceneResource::SceneResource(std::int32_t initial_references,
    void (*on_zero)(SceneResource&), void* destruction_context)
    : references(initial_references), destroy_on_zero(on_zero), context(destruction_context) {
    if (!on_zero) throw std::invalid_argument("scene virtual+0 destruction callback is required");
}
SceneNodeAttachment::SceneNodeAttachment(CameraTransform& node_transform, std::uint32_t key,
    SceneTypePredicate virtual_0c, SceneAttachOverride virtual_50, void* node_context)
    : transform(node_transform), pointer_key(key), is_type(virtual_0c),
      attach_scene(virtual_50), context(node_context) {
    if (!virtual_0c || !virtual_50)
        throw std::invalid_argument("scene node virtual+0C and virtual+50 are required");
}
SceneAttachmentRuntime::SceneAttachmentRuntime(std::uint32_t registry_token,
    std::array<std::uint32_t, 3> object_tokens)
    : registry_type_token(registry_token), object_type_tokens(object_tokens) {}
void SceneAttachmentRuntime::bind(SceneNodeAttachment& node) {
    for (const auto* binding : bindings_) {
        if (binding == &node) return;
        if (&binding->transform == &node.transform || binding->pointer_key == node.pointer_key)
            throw std::invalid_argument("scene binding transform and pointer key must be unique");
    }
    bindings_.push_back(&node);
}
void SceneAttachmentRuntime::unbind(SceneNodeAttachment& node) {
    if (node.scene) throw std::logic_error("detach the scene node before unbinding it");
    bindings_.erase(std::remove(bindings_.begin(), bindings_.end(), &node), bindings_.end());
}
SceneNodeAttachment& SceneAttachmentRuntime::resolve(CameraTransform& transform) const {
    for (auto* binding : bindings_) if (&binding->transform == &transform) return *binding;
    throw std::logic_error("scene hierarchy child has no live attachment binding");
}
std::uint32_t get_scene_registry_type_00b7aa70(const SceneAttachmentRuntime& runtime) noexcept {
    return runtime.registry_type_token;
}
bool object_accepts_scene_type_006ef860(SceneAttachmentRuntime& runtime,
    SceneNodeAttachment&, std::uint32_t token) {
    for (const auto accepted : runtime.object_type_tokens) if (token == accepted) return true;
    return false;
}
void add_scene_node_if_type_00b83d50(SceneAttachmentRuntime& runtime,
    SceneResource& scene, SceneNodeAttachment& node) {
    const auto predicate = node.is_type; // native loads vtable before token getter
    if (predicate(runtime, node, get_scene_registry_type_00b7aa70(runtime)))
        scene.registry.insert_00b83700(node);
}
void remove_scene_node_if_type_00b83ec0(SceneAttachmentRuntime& runtime,
    SceneResource& scene, SceneNodeAttachment& node) {
    const auto predicate = node.is_type;
    if (predicate(runtime, node, get_scene_registry_type_00b7aa70(runtime)))
        scene.registry.erase_00b83e50(node.pointer_key);
}
void set_node_scene_00b6ed80(SceneAttachmentRuntime& runtime,
    SceneNodeAttachment& node, SceneResource* requested, bool recurse) {
    if (node.scene != requested) {
        if (node.scene) remove_scene_node_if_type_00b83ec0(runtime, *node.scene, node);
        auto* previous = node.scene; // detach/type callback may have reentered
        if (previous != requested) {
            node.scene = requested;
            if (requested) requested->references.fetch_add(1);
            if (previous && previous->references.fetch_sub(1) == 1)
                previous->destroy_on_zero(*previous);
        }
        // Destruction may replace node.scene, so use the freshly published value.
        if (node.scene) add_scene_node_if_type_00b83d50(runtime, *node.scene, node);
    }
    if (recurse) {
        auto* child = node.transform.first_child;
        while (child) {
            auto& binding = runtime.resolve(*child);
            binding.attach_scene(runtime, binding, requested, true);
            child = child->next_sibling; // intentionally after the child callback
        }
    }
}
}

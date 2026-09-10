#include "bsp/scene_attachment.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <algorithm>
#include <cstring>
#include <memory>
#include <new>
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

SceneNodeRegistry::SceneNodeRegistry(SceneRegistryInitialization initialization) {
    if (initialization == SceneRegistryInitialization::immediate)
        initialize_native_00b83600();
}
SceneNodeRegistry::~SceneNodeRegistry() {
    destroy_native_bucket_storage_00b82ed0();
    destroy_native_list_storage_00b829d0();
}
void SceneNodeRegistry::initialize_native_00b83600() {
    static_assert(sizeof(void*) != 4 || sizeof(Entry) == 12);
    static_assert(sizeof(void*) != 4 || sizeof(Iterator) == 8);
    static_assert(sizeof(void*) != 4 || offsetof(ListOwner, head) == 4);
    static_assert(sizeof(void*) != 4 || offsetof(ListOwner, size) == 8);
    if (list_.head || boundaries_)
        throw std::logic_error("scene registry is already initialized");
    void* storage = singleton_lifetime_allocate({SingletonAllocationKind::object, 12, sizeof(Entry)});
    std::uint32_t payload;
    std::memcpy(&payload, static_cast<const unsigned char*>(storage) + offsetof(Entry, key), 4);
    list_.head = ::new (storage) Entry;
    list_.head->next = list_.head->previous = list_.head;
    list_.head->key = payload; // Native 00B82390 leaves allocator+8 untouched.
    list_.size = 0;
    try {
        resize_boundaries(9); // Native source iterator is {list owner, sentinel}.
    } catch (...) {
        // Registry unwind CC2370 -> 00B82B40 -> 00B829D0 frees its sentinel.
        destroy_native_list_storage_00b829d0();
        throw;
    }
    mask_ = active_buckets_ = 1;
}
void SceneNodeRegistry::destroy_native_bucket_storage_00b82ed0() noexcept {
    singleton_lifetime_free(boundaries_);
    boundaries_ = nullptr;
    boundary_count_ = boundary_capacity_ = 0;
}
void SceneNodeRegistry::destroy_native_list_storage_00b829d0() noexcept {
    if (!list_.head) return;
    auto* entry = list_.head->next;
    list_.head->next = list_.head->previous = list_.head;
    list_.size = 0;
    while (entry != list_.head) {
        auto* next = entry->next;
        singleton_lifetime_free(entry);
        entry = next;
    }
    singleton_lifetime_free(list_.head);
    list_.head = nullptr;
    bindings_.clear();
}
void SceneNodeRegistry::require_initialized() const {
    if (!list_.head || !boundaries_)
        throw std::logic_error("scene registry native storage is not initialized");
}
void SceneNodeRegistry::resize_boundaries(std::uint32_t count) {
    if (count > 0x1fffffffU) throw std::length_error("scene iterator vector too long");
    if (count > boundary_capacity_) {
        auto* replacement = static_cast<Iterator*>(singleton_lifetime_allocate({
            SingletonAllocationKind::pointer_slots, static_cast<std::size_t>(count) * 8,
            static_cast<std::size_t>(count) * sizeof(Iterator)}));
        for (std::uint32_t i = 0; i < boundary_count_; ++i)
            ::new (replacement + i) Iterator{boundaries_[i].owner, boundaries_[i].node};
        for (std::uint32_t i = boundary_count_; i < count; ++i)
            ::new (replacement + i) Iterator{&list_, list_.head};
        singleton_lifetime_free(boundaries_);
        boundaries_ = replacement;
        boundary_capacity_ = count;
    } else {
        for (std::uint32_t i = boundary_count_; i < count; ++i)
            boundaries_[i] = {&list_, list_.head};
    }
    boundary_count_ = count;
}
const void* SceneNodeRegistry::native_next(const void* node) noexcept {
    return static_cast<const Entry*>(node)->next;
}
std::uint32_t SceneNodeRegistry::native_key(const void* node) noexcept {
    return static_cast<const Entry*>(node)->key;
}
SceneNodeAttachment* SceneNodeRegistry::binding_for_key(std::uint32_t key) const noexcept {
    const auto found = bindings_.find(key);
    return found == bindings_.end() ? nullptr : found->second;
}
std::uint32_t SceneNodeRegistry::bucket_for(std::uint32_t key) const noexcept {
    auto bucket = scene_pointer_hash(key) & mask_;
    if (bucket >= active_buckets_) bucket -= (mask_ >> 1) + 1;
    return bucket;
}
void SceneNodeRegistry::split_before_insert() {
    // Growth precedes the duplicate check, including a duplicate insertion.
    if (active_buckets_ > (list_.size >> 2)) return;
    if (active_buckets_ < boundary_count_ - 1) {
        if (mask_ < active_buckets_) mask_ = mask_ * 2 + 1;
    } else {
        mask_ = boundary_count_ * 2 - 3;
        resize_boundaries(mask_ + 2);
    }
    const auto source = active_buckets_ - (mask_ >> 1) - 1;
    auto* entry = boundaries_[source].node;
    while (entry != boundaries_[source + 1].node) {
        if ((scene_pointer_hash(entry->key) & mask_) == source) {
            entry = entry->next;
            continue;
        }
        auto* next = entry->next;
        if (next != list_.head) {
            for (auto bucket = source;; --bucket) {
                if (boundaries_[bucket].node != entry) break;
                boundaries_[bucket].node = next;
                if (bucket == 0) break;
            }
            entry->previous->next = next;
            next->previous = entry->previous;
            entry->next = list_.head;
            entry->previous = list_.head->previous;
            list_.head->previous->next = entry;
            list_.head->previous = entry;
            boundaries_[active_buckets_ + 1].node = list_.head;
        }
        for (auto bucket = active_buckets_; bucket > source; --bucket) {
            if (boundaries_[bucket].node != list_.head) break;
            boundaries_[bucket].node = entry;
        }
        entry = next;
    }
    ++active_buckets_;
}
bool SceneNodeRegistry::insert_00b83700(SceneNodeAttachment& binding) {
    require_initialized();
    split_before_insert();
    auto bucket = bucket_for(binding.pointer_key);
    auto* position = boundaries_[bucket + 1].node;
    while (position != boundaries_[bucket].node) {
        position = position->previous;
        if (binding.pointer_key >= position->key) {
            if (binding.pointer_key == position->key) return false;
            position = position->next;
            break;
        }
    }
    void* storage = singleton_lifetime_allocate({SingletonAllocationKind::object, 12, sizeof(Entry)});
    auto free_entry = [](Entry* entry) { singleton_lifetime_free(entry); };
    std::unique_ptr<Entry, decltype(free_entry)> entry(::new (storage) Entry, free_entry);
    if (list_.size == 0x3fffffffu) throw std::length_error("list<T> too long");
    bindings_.emplace(binding.pointer_key, &binding); // Host association, no list links.
    ++list_.size; // 00B82D30 is a checked size increment, not always a throw
    entry->next = position;
    entry->previous = position->previous;
    entry->key = binding.pointer_key;
    auto* inserted = entry.release();
    position->previous->next = inserted;
    position->previous = inserted;
    for (;;) {
        if (boundaries_[bucket].node != position) break;
        boundaries_[bucket].node = inserted;
        if (bucket == 0) break;
        --bucket;
    }
    return true;
}
void SceneNodeRegistry::clear_00b83be0() {
    auto* entry = list_.head->next;
    list_.head->next = list_.head->previous = list_.head;
    list_.size = 0;
    while (entry != list_.head) {
        auto* next = entry->next;
        singleton_lifetime_free(entry);
        entry = next;
    }
    bindings_.clear();
    resize_boundaries(9);
    for (std::uint32_t i = 0; i < boundary_count_; ++i)
        boundaries_[i] = {&list_, list_.head};
    mask_ = active_buckets_ = 1;
}
std::uint32_t SceneNodeRegistry::erase_00b83e50(std::uint32_t key) {
    require_initialized();
    auto bucket = bucket_for(key);
    auto* first = boundaries_[bucket].node;
    auto* end = boundaries_[bucket + 1].node;
    while (first != end && first->key < key) first = first->next;
    if (first == end || first->key != key) return 0;
    // Unique insertion makes native equal_range/count at most one. The whole
    // list range goes through clear, resetting bucket growth even after erasure.
    if (first == list_.head->next && first->next == list_.head) {
        clear_00b83be0();
        return 1;
    }
    for (;;) {
        if (boundaries_[bucket].node != first) break;
        boundaries_[bucket].node = first->next;
        if (bucket == 0) break;
        --bucket;
    }
    first->previous->next = first->next;
    first->next->previous = first->previous;
    singleton_lifetime_free(first); // 00B827C0 never frees the borrowed light.
    bindings_.erase(key);
    --list_.size; // native continuation at 00B828DD, omitted from old pseudocode
    return 1;
}
bool SceneNodeRegistry::contains(std::uint32_t key) const {
    require_initialized();
    const auto bucket = bucket_for(key);
    auto* entry = boundaries_[bucket].node;
    const auto* end = boundaries_[bucket + 1].node;
    while (entry != end && entry->key < key) entry = entry->next;
    return entry != end && entry->key == key;
}
std::vector<SceneNodeAttachment*> SceneNodeRegistry::members() const {
    require_initialized();
    std::vector<SceneNodeAttachment*> result;
    result.reserve(list_.size);
    for (auto* entry = list_.head->next; entry != list_.head; entry = entry->next) {
        auto* binding = binding_for_key(entry->key);
        if (!binding) throw std::logic_error("scene registry raw key has no attachment binding");
        result.push_back(binding);
    }
    return result;
}

SceneResource::SceneResource(std::int32_t initial_references,
    void (*on_zero)(SceneResource&), void* destruction_context,
    SceneRegistryInitialization initialization)
    : references(initial_references), registry(initialization),
      destroy_on_zero(on_zero), context(destruction_context) {
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
SystemDirectionalLight* SceneAttachmentRuntime::resolve_light(std::uint32_t key) {
    if (!key) return nullptr;
    for (const auto* binding : bindings_) {
        if (binding->pointer_key != key) continue;
        if (!binding->system_directional_light)
            throw std::logic_error("native light key has no directional-light projection");
        return binding->system_directional_light;
    }
    throw std::logic_error("native light key has no live attachment binding");
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

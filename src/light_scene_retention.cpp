#include "bsp/light_scene_retention.hpp"
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
std::int32_t signed_word(std::uint32_t word) noexcept {
    std::int32_t result;
    std::memcpy(&result, &word, sizeof(result));
    return result;
}
bool contains_scene(const SystemAmbientBacklinks& array, SceneResource* requested) noexcept {
    for (std::int32_t i = 0; i < array.count_04; ++i)
        if (array.begin_00[i] == requested) return true;
    return false;
}

LightSceneRetention& retained_scenes(SceneNodeAttachment& node) {
    auto* actual = static_cast<LightSceneRetention*>(node.context);
    if (!actual || &actual->node != &node)
        throw std::invalid_argument("light scene dispatch requires its actual retained-scene binding");
    return *actual;
}
}

void attach_light_scene_00b7c020(SceneAttachmentRuntime& runtime,
    LightSceneRetention& light, SceneResource* requested, bool recurse) {
    auto& array = light.scenes_178;
    if (!contains_scene(array, requested)) {
        if (array.count_04 == array.capacity_08) {
            auto capacity = signed_word(
                static_cast<std::uint32_t>(array.capacity_08) * 2u);
            if (capacity < 2) capacity = 1;
            reserve_system_ambient_backlinks_00b7b390(array, capacity);
        }
        // Read pointer/count again after reserve's actual allocator can run a
        // handler. Native construction tests the resulting slot before writing.
        const auto slot_address = reinterpret_cast<std::uintptr_t>(array.begin_00)
            + static_cast<std::uint32_t>(array.count_04) * sizeof(SceneResource*);
        auto** slot = reinterpret_cast<SceneResource**>(slot_address);
        if (slot) *slot = requested;
        array.count_04 = signed_word(static_cast<std::uint32_t>(array.count_04) + 1u);
        // Valid native callers supply a live nonnull scene. No silent null skip
        // or cleanup of a partially published array is substituted here.
        add_scene_node_if_type_00b83d50(runtime, *requested, light.node);
        requested->references.fetch_add(1, std::memory_order_seq_cst);
    }
    if (recurse) {
        auto* child = light.node.transform.first_child;
        while (child) {
            auto& binding = runtime.resolve(*child);
            const auto callback = binding.attach_scene;
            if (!callback) throw std::logic_error("child light scene virtual50 is unbound");
            callback(runtime, binding, requested, true);
            child = child->next_sibling;
        }
    }
}

void remove_light_scene_00b7bd60(SceneAttachmentRuntime& runtime,
    LightSceneRetention& light, SceneResource* requested, bool recurse) {
    auto& array = light.scenes_178;
    if (contains_scene(array, requested)) {
        erase_system_ambient_backlink_00b7b620(array, &requested);
        remove_scene_node_if_type_00b83ec0(runtime, *requested, light.node);
        if (requested->references.fetch_sub(1, std::memory_order_seq_cst) == 1)
            requested->destroy_on_zero(*requested);
    }
    if (recurse) {
        auto* child = light.node.transform.first_child;
        while (child) {
            auto& binding = runtime.resolve(*child);
            const auto callback = binding.remove_scene;
            if (!callback) throw std::logic_error("child light scene virtual54 is unbound");
            callback(runtime, binding, requested, true);
            child = child->next_sibling;
        }
    }
}

void dispatch_light_scene_attach_00b7c020(SceneAttachmentRuntime& runtime,
    SceneNodeAttachment& node, SceneResource* requested, bool recurse) {
    attach_light_scene_00b7c020(runtime, retained_scenes(node), requested, recurse);
}
void dispatch_light_scene_remove_00b7bd60(SceneAttachmentRuntime& runtime,
    SceneNodeAttachment& node, SceneResource* requested, bool recurse) {
    remove_light_scene_00b7bd60(runtime, retained_scenes(node), requested, recurse);
}
}

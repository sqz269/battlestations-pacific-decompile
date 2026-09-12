#include "bsp/native_point_light_links.hpp"
#include "bsp/native_node_construction.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <algorithm>
#include <exception>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native point-light links require MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(NativePointLightBacklinkArray) == 12);
static_assert(offsetof(NativePointLightBacklinkArray, count) == 4);
static_assert(offsetof(NativePointLightBacklinkArray, capacity) == 8);
static_assert(sizeof(void*) == 4);
namespace {
template<class Array> void require_extent(const Array& array) {
    if (array.count < 0 || array.capacity < array.count ||
        static_cast<std::uint32_t>(array.capacity) > 0x3fffffffu ||
        (array.capacity && !array.begin))
        throw std::logic_error("native point-light descriptor has invalid extent");
}
template<class Array> void reserve(NativePointLightLinksRuntime& runtime,
    Array& array, std::int32_t requested) {
    require_extent(array);
    if (requested < 1) requested = 1;
    if (array.capacity >= requested) return;
    runtime.require_owned_backing(array.begin, array.capacity);
    using Pointer = decltype(array.begin);
    auto* const allocated = static_cast<Pointer>(runtime.allocate_backing(requested));
    // Reload source pointer/count after the real CRT new-handler boundary.
    // Borrowed descriptors must remain valid and within requested capacity.
    try {
        require_extent(array);
        runtime.require_owned_backing(array.begin, array.capacity);
        if (array.count > requested)
            throw std::logic_error("point-light count outgrew allocation during new handler");
    } catch (...) {
        runtime.free_backing(allocated); // unpublished host diagnostic allocation
        throw;
    }
    for (std::int32_t i = 0; i < array.count; ++i) allocated[i] = array.begin[i];
    runtime.free_backing(array.begin);
    array.begin = allocated;
    array.capacity = requested;
}
template<class Array, class Pointer> bool erase_first(Array& array, Pointer pointer) noexcept {
    require_extent(array);
    for (std::int32_t i = 0; i < array.count; ++i) {
        if (array.begin[i] != pointer) continue;
        const auto last = array.count - 1;
        if (i != last) array.begin[i] = array.begin[last];
        --array.count;
        return true;
    }
    return false;
}
template<class Array> std::int32_t next_capacity(const Array& array) {
    // Native signed double/clamp; exclude its overflowing allocation extent.
    if (array.capacity > 0x1fffffffu)
        throw std::length_error("native point-light array growth overflows");
    return array.capacity * 2;
}
void require_light_address(void* light, std::size_t bytes) {
    if (!light || bytes < 0x1ecu || reinterpret_cast<std::uintptr_t>(light) % 4u)
        throw std::invalid_argument("point-light binding requires its aligned actual1ECh extent");
}
} // namespace

NativePointLightLinksBinding::NativePointLightLinksBinding(void* actual_light,
    std::size_t bytes, NativePointLightBacklinkArray& descriptor)
    : identity(actual_light), backlinks(descriptor) {
    require_light_address(actual_light, bytes);
    if (static_cast<void*>(&descriptor) != static_cast<std::byte*>(actual_light) + 0x1e0)
        throw std::invalid_argument("point-light backlinks must be the same actual+1E0 descriptor");
    require_extent(descriptor);
}
NativePointLightLinksRuntime::~NativePointLightLinksRuntime() {
    if (!allocations_.empty() || !lights_.empty()) std::terminate();
}
void NativePointLightLinksRuntime::bind_light(NativePointLightLinksBinding& binding) {
    for (auto* current : lights_)
        if (current->identity == binding.identity)
            throw std::logic_error("actual point light already has its canonical binding");
    lights_.push_back(&binding);
}
void NativePointLightLinksRuntime::unbind_light(NativePointLightLinksBinding& binding) {
    if (binding.backlinks.count != 0)
        throw std::logic_error("point-light binding still has native node backlinks");
    const auto found = std::find(lights_.begin(), lights_.end(), &binding);
    if (found == lights_.end()) throw std::logic_error("point-light binding belongs to another runtime");
    lights_.erase(found);
}
NativePointLightLinksBinding& NativePointLightLinksRuntime::light(void* identity) const {
    for (auto* current : lights_) if (current->identity == identity) return *current;
    throw std::logic_error("actual point light has no live descriptor binding");
}
void* NativePointLightLinksRuntime::allocate_backing(std::int32_t capacity) {
    if (capacity <= 0 || static_cast<std::uint32_t>(capacity) > 0x3fffffffu)
        throw std::length_error("native point-light allocation byte extent overflows");
    const auto bytes = static_cast<std::size_t>(capacity) * 4u;
    void* const allocation = singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, bytes, bytes});
    try { allocations_.emplace(allocation, bytes); }
    catch (...) { singleton_lifetime_free(allocation); throw; }
    return allocation;
}
void NativePointLightLinksRuntime::require_owned_backing(const void* pointer, std::int32_t capacity) const {
    if (!pointer && capacity == 0) return;
    const auto found = allocations_.find(const_cast<void*>(pointer));
    if (capacity < 0 || found == allocations_.end() ||
        found->second != static_cast<std::size_t>(capacity) * 4u)
        throw std::logic_error("point-light backing was not allocated by this matching domain");
}
void NativePointLightLinksRuntime::free_backing(void* pointer) noexcept {
    if (!pointer) { singleton_lifetime_free(nullptr); return; }
    const auto found = allocations_.find(pointer);
    if (found == allocations_.end()) std::terminate();
    allocations_.erase(found);
    singleton_lifetime_free(pointer);
}
NativePointLightBacklinkArray& initialize_native_point_light_backlinks_00b7c710_fragment(
    void* light, std::size_t bytes) {
    require_light_address(light, bytes);
    return *::new (static_cast<std::byte*>(light) + 0x1e0) NativePointLightBacklinkArray{nullptr, 0, 0};
}
void reserve_native_node_point_lights_00b6e500(NativePointLightLinksRuntime& runtime,
    NativeNodePointLightArray& array, std::int32_t capacity) {
    reserve(runtime, array, capacity);
}
void append_native_point_light_backlink_00b7be40(NativePointLightLinksRuntime& runtime,
    NativePointLightLinksBinding& light, NativeNodeStorage& node) {
    if (&runtime.light(light.identity) != &light)
        throw std::logic_error("point-light backlink requires its same registered identity");
    auto& array = light.backlinks;
    require_extent(array);
    if (array.count == array.capacity) reserve(runtime, array, next_capacity(array));
    array.begin[array.count] = &node;
    ++array.count;
}
void append_native_node_point_light_00b6eed0(NativePointLightLinksRuntime& runtime,
    NativeNodeStorage& node, void* identity) {
    // Diagnostic lookup before effects requires a real physical light binding.
    auto& light = runtime.light(identity);
    auto& array = node.point_lights_164;
    require_extent(array);
    if (array.count == array.capacity)
        reserve_native_node_point_lights_00b6e500(runtime, array, next_capacity(array));
    array.begin[array.count] = identity;
    ++array.count;
    append_native_point_light_backlink_00b7be40(runtime, light, node);
}
void copy_native_node_point_light_links_00b6f150_fragment(NativePointLightLinksRuntime& runtime,
    NativeNodeStorage& source, NativeNodeStorage& destination) {
    if (&source == &destination) throw std::invalid_argument("native clone light lists require distinct nodes");
    for (std::int32_t i = 0; i < source.point_lights_164.count; ++i) {
        require_extent(source.point_lights_164);
        void* const light = source.point_lights_164.begin[i];
        append_native_node_point_light_00b6eed0(runtime, destination, light);
    }
}
void remove_native_point_light_backlink_00b7c1a0(NativePointLightLinksBinding& light,
    NativeNodeStorage& node) noexcept { (void)erase_first(light.backlinks, &node); }
void remove_native_node_point_light_00b6f3c0(NativeNodeStorage& node, void* identity) noexcept {
    (void)erase_first(node.point_lights_164, identity);
}
void unlink_native_point_light_nodes_00b7c160(NativePointLightLinksBinding& light) noexcept {
    auto& array = light.backlinks;
    require_extent(array);
    std::uint32_t index = 0;
    while (index < static_cast<std::uint32_t>(array.count)) {
        auto* const node = array.begin[index];
        if (!node) std::terminate();
        remove_native_node_point_light_00b6f3c0(*node, light.identity);
        ++index;
    }
    while (array.count > 0) --array.count;
    array.count = 0;
}
void destroy_native_point_light_backlinks_00b7c770_fragment(NativePointLightLinksRuntime& runtime,
    NativePointLightLinksBinding& light) {
    if (&runtime.light(light.identity) != &light)
        throw std::logic_error("point-light destruction requires its same registered identity");
    runtime.require_owned_backing(light.backlinks.begin, light.backlinks.capacity);
    unlink_native_point_light_nodes_00b7c160(light);
    while (light.backlinks.count > 0) --light.backlinks.count;
    light.backlinks.count = 0;
    runtime.free_backing(light.backlinks.begin);
}
} // namespace bsp

#pragma once
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace bsp {
struct NativeNodeStorage;
struct NativeNodePointLightArray;

// The actual light+1E0/+1E4/+1E8 descriptor. Elements are raw node addresses,
// never CameraTransform/GeneratedModelPointLightLinks companion addresses.
struct NativePointLightBacklinkArray {
    NativeNodeStorage** begin;
    std::int32_t count;
    std::int32_t capacity;
};

// Borrow an already live point light and its SAME physical descriptor. The
// constructor checks the offset and minimum extent; it changes no native byte.
// The light and descriptor must survive all linked nodes and this binding.
struct NativePointLightLinksBinding final {
    NativePointLightLinksBinding(void* actual_light, std::size_t actual_bytes,
        NativePointLightBacklinkArray& actual_descriptor);
    void* const identity;
    NativePointLightBacklinkArray& backlinks;
};

// One light association registry and allocation provenance, with no link copy.
// New backing uses the existing CRT allocation/free service. This domain only
// frees backing it allocated itself; foreign buffers are never adopted/freed.
// Bindings and this service must outlive every operation using their arrays.
// Destruction requires explicit native cleanup first; it does not synthesize
// light/node destruction or silently release outstanding backing.
class NativePointLightLinksRuntime final {
public:
    ~NativePointLightLinksRuntime();
    NativePointLightLinksRuntime() = default;
    NativePointLightLinksRuntime(const NativePointLightLinksRuntime&) = delete;
    NativePointLightLinksRuntime& operator=(const NativePointLightLinksRuntime&) = delete;
    void bind_light(NativePointLightLinksBinding&);
    void unbind_light(NativePointLightLinksBinding&);
    NativePointLightLinksBinding& light(void* actual_identity) const;
    void* allocate_backing(std::int32_t capacity);
    void require_owned_backing(const void*, std::int32_t capacity) const;
    void free_backing(void*) noexcept;
private:
    std::vector<NativePointLightLinksBinding*> lights_;
    std::unordered_map<void*, std::size_t> allocations_;
};

// ONLY B7C725..B7C737: after the existing base constructor and profile store.
// Initializes the descriptor in the caller's actual light slot; no light base,
// slot allocator, profile dispatch, point-light reference or value projection.
NativePointLightBacklinkArray& initialize_native_point_light_backlinks_00b7c710_fragment(
    void* actual_light, std::size_t actual_bytes);

// Native ECX descriptor, signed stack capacity, RET4. Grow-only; clamp1;
// allocate/copy/free/publish. Malformed/overflowing extents are diagnostics.
void reserve_native_node_point_lights_00b6e500(NativePointLightLinksRuntime&,
    NativeNodePointLightArray&, std::int32_t capacity);
// ECX light, raw node on stack, RET4. Append one borrowed raw node; no retain.
void append_native_point_light_backlink_00b7be40(NativePointLightLinksRuntime&,
    NativePointLightLinksBinding&, NativeNodeStorage&);
// B6EED0, ECX node, raw light on stack, RET4. Publishes the forward element
// and increments count BEFORE attempting the reverse append. No rollback.
void append_native_node_point_light_00b6eed0(NativePointLightLinksRuntime&,
    NativeNodeStorage&, void* actual_light);
// Positive-light loop B6F164..B6F1BE, signed live source count. Captures each
// source light before destination growth; current source count/slots reload.
void copy_native_node_point_light_links_00b6f150_fragment(NativePointLightLinksRuntime&,
    NativeNodeStorage& source, NativeNodeStorage& destination);
// B7C1A0 -> B7BED0 and B6F3C0 -> B6F090. Erase first equal raw identity by
// swapping the final pointer, preserving backing/capacity and stale last slot.
void remove_native_point_light_backlink_00b7c1a0(NativePointLightLinksBinding&,
    NativeNodeStorage&) noexcept;
void remove_native_node_point_light_00b6f3c0(NativeNodeStorage&, void* actual_light) noexcept;
// B7C160: remove this light once from each current node, then shrink reverse
// count to zero. No node/light reference count or physical allocation changes.
void unlink_native_point_light_nodes_00b7c160(NativePointLightLinksBinding&) noexcept;
// ONLY B7C79C..B7C7BD: unlink, resize0 again, free reverse backing. Leaves
// freed pointer/capacity unchanged. Point-light vtable phase must be installed
// by caller; B7C7C0 Light base destruction and EH continuation remain external.
void destroy_native_point_light_backlinks_00b7c770_fragment(NativePointLightLinksRuntime&,
    NativePointLightLinksBinding&);
} // namespace bsp

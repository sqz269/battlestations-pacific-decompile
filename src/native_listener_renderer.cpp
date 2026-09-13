#include "bsp/native_listener_renderer.hpp"

namespace bsp {

void NativeListenerRendererBindings::propagate_node_root_00b6d890(
    CameraTransform& node, RenderNodeRootList* requested_root)
{
    propagate_native_node_root_00b6d890(nodes_, node, requested_root);
}

void invalidate_native_listener_roots_00b4ec90(NativeListenerInvalidationView view,
    NativeListenerRendererBindings& bindings)
{
    void* const captured_owner = view.root_owner_3c; // 00B4EC93, before flag store
    view.byte_250 = 1;
    if (bindings.root_owner(captured_owner).first != nullptr) {
        do {
            auto* node = bindings.root_owner(view.root_owner_3c).first;
            bindings.propagate_node_root_00b6d890(*node, nullptr);
        } while (bindings.root_owner(view.root_owner_3c).first != nullptr);
    }
}

void update_native_listener_renderer_00b0d7b0(NativeListenerRendererView view,
    void* handle, NativeListenerRendererBindings& bindings)
{
    view.listener_1c0 = handle;
    if (void* owner = view.invalidation_owner_30) {
        invalidate_native_listener_roots_00b4ec90(bindings.invalidation_owner(owner), bindings);
    }
}

} // namespace bsp

#include "bsp/native_listener_renderer.hpp"
#include <cstring>

namespace bsp {

NativeListenerRendererView NativeListenerRendererView::from_raw(void* actual) noexcept {
    static_assert(sizeof(void*) == 4, "native listener slots are DWORDs");
    auto* bytes = static_cast<unsigned char*>(actual);
    return NativeListenerRendererView(bytes + 0x1c0, bytes + 0x30);
}

void NativeListenerRendererView::publish_listener(void* listener) const noexcept {
    std::memcpy(listener_cell_, &listener, sizeof listener);
}

void* NativeListenerRendererView::invalidation_owner() const noexcept {
    void* value;
    std::memcpy(&value, owner_cell_, sizeof value);
    return value;
}

NativeListenerInvalidationView NativeListenerInvalidationView::from_raw(void* actual) noexcept {
    auto* bytes = static_cast<unsigned char*>(actual);
    return NativeListenerInvalidationView(bytes + 0x3c, bytes + 0x250);
}

void* NativeListenerInvalidationView::root_owner() const noexcept {
    void* value;
    std::memcpy(&value, root_cell_, sizeof value);
    return value;
}

void NativeListenerInvalidationView::mark_invalid() const noexcept {
    const std::uint8_t value = 1;
    std::memcpy(flag_cell_, &value, sizeof value);
}

void* NativeListenerRendererBindings::root_head(void* actual_owner) {
    return root_owner(actual_owner).first;
}

void NativeListenerRendererBindings::detach_root_head(void* actual_node) {
    propagate_node_root_00b6d890(*static_cast<CameraTransform*>(actual_node), nullptr);
}

void NativeListenerRendererBindings::propagate_node_root_00b6d890(
    CameraTransform& node, RenderNodeRootList* requested_root)
{
    propagate_native_node_root_00b6d890(nodes_, node, requested_root);
}

void invalidate_native_listener_roots_00b4ec90(NativeListenerInvalidationView view,
    NativeListenerRendererBindings& bindings)
{
    void* const captured_owner = view.root_owner(); // 00B4EC93, before flag store
    view.mark_invalid();
    if (bindings.root_head(captured_owner) != nullptr) {
        do {
            auto* node = bindings.root_head(view.root_owner());
            bindings.detach_root_head(node);
        } while (bindings.root_head(view.root_owner()) != nullptr);
    }
}

void update_native_listener_renderer_00b0d7b0(NativeListenerRendererView view,
    void* handle, NativeListenerRendererBindings& bindings)
{
    view.publish_listener(handle);
    if (void* owner = view.invalidation_owner()) {
        invalidate_native_listener_roots_00b4ec90(bindings.invalidation_owner(owner), bindings);
    }
}

} // namespace bsp

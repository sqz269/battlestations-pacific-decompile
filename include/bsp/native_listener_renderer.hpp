#pragma once

#include "bsp/native_node_destruction.hpp"
#include <cstdint>

namespace bsp {

// References to the same existing renderer's actual fields. +1C0 is the
// listener slot already identified by controlled_unit.hpp. No renderer owner,
// pointer publication, reference count or semantic-to-native cast is added.
struct NativeListenerRendererView {
    void*& listener_1c0;
    void* const& invalidation_owner_30;
};

// The actual object reached through renderer+30. Names describe these accesses;
// no complete native layout or additional scene owner is inferred.
struct NativeListenerInvalidationView {
    void* const& root_owner_3c;
    std::uint8_t& byte_250;
};

class NativeListenerRendererBindings {
public:
    explicit NativeListenerRendererBindings(NativeNodeDestructionRuntime& nodes) noexcept
        : nodes_(nodes) {}
    virtual ~NativeListenerRendererBindings() = default;

    // Required side-effect-free mappings of the supplied live actual identities.
    // The root view must borrow that same owner's actual +0C head and +1C scene
    // fields, and use the existing canonical CameraTransform node bindings.
    virtual NativeListenerInvalidationView invalidation_owner(void* actual_owner) = 0;
    virtual RenderNodeRootList& root_owner(void* actual_owner) = 0;

    // Concrete composition with the complete existing 00B6D890 implementation.
    // Overrides must preserve that complete contract, including actual root
    // unlinking, root publication and child recursion. No guessed list removal.
    virtual void propagate_node_root_00b6d890(CameraTransform&, RenderNodeRootList*);
private:
    NativeNodeDestructionRuntime& nodes_;
};

// Complete normal 00B4EC90..00B4ECBA, 43 bytes. Native ECX=owner, bare RET.
// Capture +3C before setting +250=1, test the captured owner's +0C, then reload
// the current +3C/+0C before each call and again after it for the loop gate.
// Required native call is 00B6D890(node, nullptr), ECX=node, PUSH 0, RET4.
// Head progress belongs to that operation; there is no synthetic erase/advance.
void invalidate_native_listener_roots_00b4ec90(NativeListenerInvalidationView,
    NativeListenerRendererBindings&);

// Complete normal 00B0D7B0..00B0D7C8, 25 bytes. Native ECX=renderer, stack
// handle, RET4. Publish the exact pointer first, then read current renderer+30
// and run 00B4EC90 only when nonnull. Null handles are still published.
void update_native_listener_renderer_00b0d7b0(NativeListenerRendererView,
    void* handle, NativeListenerRendererBindings&);

// New source ABI. All borrowed owners and bindings must remain live. No native
// exception, concurrent mutation, renderer integration or gameplay parity claim.
} // namespace bsp

#pragma once

#include "bsp/native_node_destruction.hpp"
#include <cstdint>

namespace bsp {

// References to the same existing renderer's actual fields. +1C0 is the
// listener slot already identified by controlled_unit.hpp. No renderer owner,
// pointer publication, reference count or semantic-to-native cast is added.
struct NativeListenerRendererView {
    NativeListenerRendererView(void*& listener, void* const& owner) noexcept
        : listener_cell_(&listener), owner_cell_(&owner) {}
    // Borrow the existing B14A10/B107F0 allocation, with no typed overlay or
    // new renderer publication. Each access reads/writes its actual DWORD.
    static NativeListenerRendererView from_raw(void* actual_renderer) noexcept;
    void publish_listener(void*) const noexcept;
    void* invalidation_owner() const noexcept;
private:
    NativeListenerRendererView(void* listener_cell, const void* owner_cell) noexcept
        : listener_cell_(listener_cell), owner_cell_(owner_cell) {}
    void* listener_cell_;
    const void* owner_cell_;
};

// The actual object reached through renderer+30. Names describe these accesses;
// no complete native layout or additional scene owner is inferred.
struct NativeListenerInvalidationView {
    NativeListenerInvalidationView(void* const& root, std::uint8_t& flag) noexcept
        : root_cell_(&root), flag_cell_(&flag) {}
    static NativeListenerInvalidationView from_raw(void* actual_owner) noexcept;
    void* root_owner() const noexcept;
    void mark_invalid() const noexcept;
private:
    NativeListenerInvalidationView(const void* root_cell, void* flag_cell) noexcept
        : root_cell_(root_cell), flag_cell_(flag_cell) {}
    const void* root_cell_;
    void* flag_cell_;
};

class NativeListenerRendererBindings {
public:
    explicit NativeListenerRendererBindings(NativeNodeDestructionRuntime& nodes) noexcept
        : nodes_(nodes) {}
    virtual ~NativeListenerRendererBindings() = default;

    // Required side-effect-free mappings of the supplied live actual identities.
    // Typed root views borrow that same owner's +0C head and +1C scene fields
    // and use the existing canonical CameraTransform node bindings.
    virtual NativeListenerInvalidationView invalidation_owner(void* actual_owner) = 0;
    virtual RenderNodeRootList& root_owner(void* actual_owner) = 0;

    // Raw scene owners override these operations, borrowing actual +0C and
    // calling the existing raw B6D890 null-request entry, which needs no scene
    // attachment dispatch.
    // The typed fixture/provider domain retains its original composition.
    virtual void* root_head(void* actual_owner);
    virtual void detach_root_head(void* actual_node);

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

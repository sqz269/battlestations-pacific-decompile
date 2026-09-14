#pragma once
#include "bsp/native_resource_hierarchy_parser.hpp"
#include <cstdint>

namespace bsp {
struct SingletonLifetimeCallbacks;

// The caller captures each actual object's current vtable entry before invoking
// these source bindings. Numeric original addresses are identities, never calls
// into the original image. Parser ownership and alternate resource profiles must
// be supplied by their actual implementation; this interface creates no items.
class NativeResourceDispatchCalls {
public:
    virtual ~NativeResourceDispatchCalls() = default;
    virtual void renderer_hook(std::uintptr_t target, void* renderer) = 0;
    virtual void* parse_item(std::uintptr_t target, void* parser, void* child_handle) = 0;
    virtual void append_item(std::uintptr_t target, void* resource, void* item) = 0;
};

struct NativeResourceRootDispatchContext {
    NativeResourceHierarchyParserContext& hierarchy;
    void* volatile& renderer_00f8d394;
    const SingletonLifetimeCallbacks& invalid_parameters;
    NativeResourceDispatchCalls& calls;
};

// B87AA0: ECX resource; stacked item; RET4. Actual pointer header10/14/18,
// wrapped capacity+16 with signed minimum16 at equality, existing B872F0
// reserve, current count/data reload, computed-null store skip, count++.
// Does not retain, classify or copy an item, or undo an earlier append.
void append_native_resource_item_00b87aa0(void* resource, void* item);

// B1FE40 is RET only; B28570 installs then removes an inactive exception frame
// without touching the renderer. Both consume no ordinary inputs or stack args.
// These source entries implement normal behavior, not the private SEH frame.
void begin_native_resource_root_00b1fe40() noexcept;
void end_native_resource_root_00b28570() noexcept;

// Bind only these three proven targets, forwarding all other identities to the
// supplied actual implementation. In particular, game-resource classification
// at71BB40 and concrete parser virtual+8 bodies are not default-resource aliases.
class NativeDefaultResourceDispatchCalls final : public NativeResourceDispatchCalls {
public:
    explicit NativeDefaultResourceDispatchCalls(NativeResourceDispatchCalls&);
    void renderer_hook(std::uintptr_t, void*) override;
    void* parse_item(std::uintptr_t, void*, void*) override;
    void append_item(std::uintptr_t, void*, void*) override;
private:
    NativeResourceDispatchCalls& other_;
};

// B7E970: ECX manager28h; stacked Resource-container handle; RET4. Actual
// parser tree at manager+8, counted-name lower bound then equivalence checks.
// Parser current slot8 returns an item; unknown allocates8h fallback and skips.
// CURRENT manager+24 resource current slotC appends before child release.
// Only active child and a still-constructing fallback allocation unwind; no
// completed item rollback. Invalid-parameter callbacks are allowed to return.
void dispatch_native_resource_items_00b7e970(void* manager, void* handle,
    NativeResourceRootDispatchContext&);

// B7F430: ECX manager; stacked root handle; RET4. CURRENT renderer slot50
// precedes control read. Ordered Resource/Hierarchy/BoundingBox tag dispatch;
// six box words are copied into CURRENT manager+24 resource+28..3C. Unknown
// children skip; recognized children get no extra seek. CURRENT renderer slot54
// runs only after normal traversal, including an empty root. Child cleanup is
// disarmed before normal release, so a throwing release is not retried.
void dispatch_native_resource_root_00b7f430(void* manager, void* handle,
    NativeResourceRootDispatchContext&);

// New C++ interfaces over actual storage and existing concrete service domains.
// No raw manager registration, parser/factory bootstrap or B80720 load admission.
// Private-stack aliases, native FH3/SEH and gameplay are not established here.
} // namespace bsp

#pragma once
#include "bsp/structured_hierarchy.hpp"
#include "bsp/structured_resource_registry.hpp"

namespace bsp {
// Retained owning wire values, not the native 0x74-byte GameResource ABI.
// Resource records preserve indices even for unsupported payloads (nullopt).
// Hierarchy references remain serialized DWORDs, without graph resolution.
struct StructuredModel {
    std::vector<DecodedStructuredResource> resources;
    std::vector<HierarchyItem> hierarchy;
    // Base constructor00b88260 initializes these six native words to zero.
    // Dispatch overwrites all six only after a complete BoundingBox read.
    std::array<float, 6> bounding_box{};
    bool has_bounding_box{}; // Host evidence of an encountered BoundingBox.
    // Host diagnostics retain counted tags, including bytes after a NUL.
    std::vector<std::string> root_order;
    std::vector<std::string> skipped_hierarchy_tags;
};

// Explicit borrowed boundary for global00f8d394 virtual+50/+54, each called
// with ECX renderer and no stack arguments. No renderer service is synthesized.
// Implementations must leave the reader, registry and model exclusively bound
// for the traversal. This host API does not emulate a reentrant native manager
// replacing its current-resource pointer at +24 or the global renderer pointer.
class StructuredModelDispatchHooks {
public:
    virtual ~StructuredModelDispatchHooks() = default;
    virtual void begin_root() = 0;
    virtual void end_root() = 0;
};

// Actual normal behavior of the observed D3D9 renderer table00d5f0a8:
// +50 ->00b1fe40 is RET; +54 ->00b28570 only installs/restores an SEH frame.
// Neither body reads/writes renderer fields or calls a service. Explicit use
// selects this concrete contract; it is not a default for arbitrary renderers.
// Native SEH/async-exception behavior and binary ABI are not reproduced.
class RendererRootHooks_00d5f0a8 final : public StructuredModelDispatchHooks {
public:
    void begin_root() noexcept override; //00b1fe40, ECX renderer unused, RET.
    void end_root() noexcept override;   //00b28570, ECX renderer unused, RET.
};

//00b7f100: ECX manager; stack pointer to Hierarchy node handle; RET4.
// Append every case-insensitive Item through the recovered Item parser;
// explicitly skip other tags. Leave the input container attached.
bool parse_hierarchy_00b7f100(StructuredNode& container,
    StructuredModel& output, std::string& error);

//00b7f430: ECX manager; stack pointer to root node handle; RET4.
// begin_root precedes control-word consumption; end_root follows successful
// traversal, including an empty root. The caller supplies actual parser objects
// in registry and explicit hooks; this function installs no parser defaults.
// Resource/Hierarchy append in encounter order; BoundingBox replaces the prior
// bounds; unknown root payloads explicitly skip. C-string tags compare through
// CRT stricmp. No root tag/control-version validation is inferred from native.
// The model is the fixed host destination for native manager+24 writes. Native
// object construction, virtual append/classification, intrusive lifetime and
// cache registration remain separate from these owning wire values.
// Leaves root attached, with no unread-tail seek for recognized children.
// Host errors preserve consumed input and completed output; there is no rollback
// or synthetic end hook on failure. Hook exceptions propagate to the caller.
// Evidence: docs/REGISTERED_ROOT_RESOURCE.md and its audit report. No native
// ABI-compatible replacement or game-validation claim.
bool dispatch_root_00b7f430(StructuredNode& root,
    StructuredResourceRegistry& registry, StructuredModel& output,
    StructuredModelDispatchHooks& hooks, std::string& error);
}

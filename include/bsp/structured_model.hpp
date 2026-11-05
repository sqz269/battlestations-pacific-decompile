#pragma once
#include "bsp/structured_hierarchy.hpp"
#include "bsp/structured_resource_registry.hpp"
#include "bsp/wire_goal4_chain563_switches.hpp"
#include "bsp/wire_goal4_chain575_switches.hpp"
#include <cstddef>
#include <iterator>
#include <type_traits>

namespace bsp {
// Source owner for the hierarchy pointer triplet at resource+1Ch. The append
// body treats pointees as opaque identities: these are the actual retained
// HierarchyItem values parsed from the model, not native 84h record images.
// Readers use this triplet's count/order directly. No parallel pointer vector
// or native resource/vtable/refcount is constructed.
class StructuredHierarchyItems final {
public:
    StructuredHierarchyItems() noexcept = default;
    ~StructuredHierarchyItems();
    StructuredHierarchyItems(StructuredHierarchyItems&&) noexcept;
    StructuredHierarchyItems& operator=(StructuredHierarchyItems&&) noexcept;
    StructuredHierarchyItems(const StructuredHierarchyItems&) = delete;
    StructuredHierarchyItems& operator=(const StructuredHierarchyItems&) = delete;

    void append_native(HierarchyItem&&);
    std::size_t size() const noexcept;
    bool empty() const noexcept { return size() == 0; }
    HierarchyItem& operator[](std::size_t) noexcept;
    const HierarchyItem& operator[](std::size_t) const noexcept;
    HierarchyItem& front() noexcept { return (*this)[0]; }
    const HierarchyItem& front() const noexcept { return (*this)[0]; }
    // The muzzle loader retains value records after its temporary model dies.
    // Extract in the SAME native pointer order, then retire this owner once.
    operator std::vector<HierarchyItem>() &&;

private:
    HierarchyItem** data() const noexcept;
    void release() noexcept;
    // Only +1Ch/+20h/+24h are accessed. The prefix is an unmodelled source
    // view, not fabricated native resource state. 00B88260 zeros the triplet.
    alignas(4) std::array<std::byte, 0x28> resource_fields_{};
};

using StructuredHierarchyStorage = std::conditional_t<kWireGoal4Chain563Bound,
    StructuredHierarchyItems, std::vector<HierarchyItem>>;

// Own the decoded records whose opaque pointers occupy resource+10h/14h/18h.
// Serialized ordinals, including unsupported records, come only from this
// array. Pointees are source decoded values, never native refcounted items.
class StructuredResourceItems final {
public:
    StructuredResourceItems() noexcept = default;
    ~StructuredResourceItems();
    StructuredResourceItems(StructuredResourceItems&&) noexcept;
    StructuredResourceItems& operator=(StructuredResourceItems&&) noexcept;
    StructuredResourceItems(const StructuredResourceItems&) = delete;
    StructuredResourceItems& operator=(const StructuredResourceItems&) = delete;

    void append_native(DecodedStructuredResource&&);
    std::size_t size() const noexcept;
    bool empty() const noexcept { return size() == 0; }
    DecodedStructuredResource& operator[](std::size_t) noexcept;
    const DecodedStructuredResource& operator[](std::size_t) const noexcept;

    template<bool Const> class Iterator {
        using Owner = std::conditional_t<Const, const StructuredResourceItems, StructuredResourceItems>;
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = DecodedStructuredResource;
        using difference_type = std::ptrdiff_t;
        using reference = std::conditional_t<Const, const value_type&, value_type&>;
        using pointer = std::conditional_t<Const, const value_type*, value_type*>;
        Iterator() noexcept = default;
        Iterator(Owner* owner, std::size_t index) noexcept : owner_(owner), index_(index) {}
        reference operator*() const noexcept { return (*owner_)[index_]; }
        pointer operator->() const noexcept { return &**this; }
        Iterator& operator++() noexcept { ++index_; return *this; }
        Iterator operator++(int) noexcept { auto previous = *this; ++*this; return previous; }
        bool operator==(const Iterator& other) const noexcept {
            return owner_ == other.owner_ && index_ == other.index_;
        }
        bool operator!=(const Iterator& other) const noexcept { return !(*this == other); }
    private:
        Owner* owner_{};
        std::size_t index_{};
    };
    Iterator<false> begin() noexcept { return {this, 0}; }
    Iterator<false> end() noexcept { return {this, size()}; }
    Iterator<true> begin() const noexcept { return {this, 0}; }
    Iterator<true> end() const noexcept { return {this, size()}; }
private:
    DecodedStructuredResource** data() const noexcept;
    void release() noexcept;
    // Only the triplet is used. The unmodelled prefix supplies the append
    // body's offsets, not a fabricated native resource/vtable/refcount.
    // 00B88260 initializes the actual triplet to these same zero words.
    alignas(4) std::array<std::byte, 0x1c> resource_fields_{};
};

using StructuredResourceStorage = std::conditional_t<kWireGoal4Chain575Bound,
    StructuredResourceItems, std::vector<DecodedStructuredResource>>;

// Retained owning wire values, not the native 0x74-byte GameResource ABI.
// Resource records preserve indices even for unsupported payloads (nullopt).
// Hierarchy references remain serialized DWORDs, without graph resolution.
struct StructuredModel {
    StructuredResourceStorage resources;
    StructuredHierarchyStorage hierarchy;
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

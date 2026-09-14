#pragma once
#include "bsp/native_unit_lifecycle_services.hpp"
#include "bsp/native_unit_part_entries.hpp"
#include "bsp/render_command_queue.hpp"

namespace bsp {
// Required bindings for the actual retained resource and root identities.
// Reads resolve current native profile data without mutation or allocation;
// the resource terminal is its complete current slot-0 operation. The node
// companion must borrow that same raw node's fields and full lifetime owner.
class NativeResourceInstanceExternalOwners {
public:
    virtual ~NativeResourceInstanceExternalOwners() = default;
    virtual std::uint32_t resource_slot_zero(void* captured_resource,
        std::uint32_t captured_profile) noexcept = 0;
    virtual void dispatch_resource_zero(std::uint32_t captured_entry,
        void* captured_resource) = 0;
    virtual RenderCommandModelLifetime& root_lifetime(void* actual_root) noexcept = 0;
};
struct NativeResourceInstanceLifetimeAccess {
    NativeControlledListenerPublication listener;
    NativeControlledListenerRenderer& renderer;
    NativeResourceInstanceExternalOwners& owners;
};

// B87930[55]: native ECX ignored; EAX actual24h storage, RET. Allocate and
// separately test computed link addresses0/4/8; color20=1,nil21=0. Remaining
// payload retains allocation bytes. The constructor later makes it a head.
void* allocate_native_resource_instance_map_head_00b87930();

// B89F20[158]: ECX actual3Ch-or-larger storage, stack resource, EAX same,RET4.
// Publish CEB130/refcount1 then D63244/resource8; zero14/18/1C,24/28/2C;
// construct the actual34 head and38 count, then retain current resource8.
// Root0C and proxy10/20/30 remain untouched. A throwing allocation unwinds
// the two vector members and ref-counted base, without releasing resource8.
void* __fastcall construct_native_resource_instance_00b89f20(void*, void* unused_edx,
    void* actual_resource);

// 71ACD0[117]: ECX actual8Ch allocation, stack resource, EAX same,RET4.
// Base first, CFD8E0, then zero data/end/capacity triplets at40,50,60,70,80.
// Keep root0C and five proxy words3C/4C/5C/6C/7C. Its EH state stays-1.
void* __fastcall construct_native_game_resource_instance_0071acd0(void*, void* unused_edx,
    void* actual_resource);

// 71AED0[96]: ECX resource; no stack arguments, EAX allocation/null,RET.
// Allocate8Ch and construct; free captured allocation on a source exception.
// This does not populate the node graph or initialize root0C. B891A0 owns that
// later phase; a just-constructed object is not automatically destructor-ready.
void* __fastcall create_native_game_resource_instance_0071aed0(void* actual_resource);

// B89150[80]: ECX map carried through recursion, stack node, RET4.
// Nil21 stops. Recurse right, read current data14, capture current left before
// any free, free data, zero14/18/1C, free node, continue over captured left.
// The map's nodes contain owned vectors, not owning scene-node references.
void __fastcall clear_native_resource_instance_map_nodes_00b89150(void* actual_map,
    void* unused_edx, void* actual_node) noexcept;

// B89DB0 physical223 bytes throughB89E8E; stored Ghidra body endsB89E32.
// ECX actual instance,RET; source EDX adds access. Release captured resource8
// first, then unlink/release current root0C, clear the complete map30, free
// arrays20/10, stampCEB130. Root0C/proxy words remain. Requires an actual valid
// full tree; the B89980 full-range branch is the only reached range operation.
// Source exceptions from resource dispatch unwind map, arrays and base only.
void __fastcall destroy_native_resource_instance_00b89db0(void*,
    const NativeResourceInstanceLifetimeAccess*);

// 718B30[237]: CFD8E0; clear matching actual listener; free/zero vector
// members7C,6C,5C,4C,3C, then base. Listener failure runs those six unwind
// actions. Native FH3/fault dispatch is not supplied by the source catches.
void __fastcall destroy_native_game_resource_instance_00718b30(void*,
    const NativeResourceInstanceLifetimeAccess*);

// Each30-byte scalar wrapper calls its destructor before flags&1, optionally
// actual CRT free, and returns the original address. Native ECX owner, stack
// flags,EAX original,RET4; source EDX adds access. Exceptions retain allocation.
void* __fastcall delete_native_resource_instance_00b89fc0(void*,
    const NativeResourceInstanceLifetimeAccess*, std::uint32_t flags);
void* __fastcall delete_native_game_resource_instance_00718d00(void*,
    const NativeResourceInstanceLifetimeAccess*, std::uint32_t flags);

// Concrete selected-set terminal for source-owned D63244/CFD8E0 profiles:
// captured slot0=BD30E0 -> current slot4=B89FC0/718D00(flags1). No numeric
// process-address dereference. Both profile tables were checked in Ghidra.
// Access survives release; all other profiles are outside this binding domain.
NativeUnitPartSelectedSetCallbacks native_resource_instance_selected_callbacks(
    NativeResourceInstanceLifetimeAccess&) noexcept;
} // namespace bsp

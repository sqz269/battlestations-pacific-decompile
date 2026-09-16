#pragma once

#include "bsp/native_resource_instance_publication.hpp"

namespace bsp {

// Borrow the SAME live profiles and descriptor cells. Each descriptor has
// three ordered type IDs followed by the native type-name address at slot14.
// CD82F0/CD8340 establish cAnimationResource/cBoneResource names. Never copy
// startup values: later getter/predicate calls read the current cells.
struct NativeResourceExtraItemProfileStorage {
    const volatile std::uint32_t* animation_profile_00d6328c;
    const volatile std::uint32_t* bone_profile_00d632b8;
    const volatile std::uint32_t* animation_cells_01090268;
    const volatile std::uint32_t* bone_cells_01090278;
};

// B8A060/B8A140 slot08 and B8A070/B8A150 slot14 each MOV EAX,[cell]; RET.
// Reuse read_native_mesh_binding_type_00b931b0 with cells[0]/cells[3].
// B8A730/B8A870: ECX object ignored, stacked token, AL Boolean, RET4.
// Reuse matches_native_fallback_type_00b86950 over exactly cells[0..2],
// stopping at the first match. Upper EAX is not a full-width Boolean.
// These bindings compose with the existing publication/classification calls;
// unrelated profiles and entries retain their complete supplied providers.
class NativeResourceExtraItemProfileCalls final : public NativeResourceInstancePublicationCalls {
public:
    NativeResourceExtraItemProfileCalls(NativeResourceInstancePublicationCalls&,
        NativeResourceExtraItemProfileStorage) noexcept;
    std::uint32_t item_slot(std::uint32_t captured_profile,
        std::uint32_t byte_offset) noexcept override;
    std::uint32_t item_type_token(std::uint32_t captured_entry, void*) override;
    std::uint8_t matches_type(std::uintptr_t captured_entry, void*,
        std::uint32_t token) override;

private:
    NativeResourceInstancePublicationCalls& other_;
    NativeResourceExtraItemProfileStorage storage_;
};

struct NativeResourceExtraItemAttachContext {
    NativeResourceInstancePublicationContext& publication;
    const volatile std::uint32_t* instance_profile_00d63244;
    const volatile std::uint32_t* game_instance_profile_00cfd8e0;
};

// B8A080/B8A160 have identical complete24-byte bodies. Native ECX=item;
// stack(instance,record,node,creation_word); RET16. Capture instance's current
// slot08 and call it with ECX=instance, stack(item,node). Record and creation
// word are unconsumed. No specified result, item retain, release or rollback.
// This source binds the two actual instance profiles to complete B89E90 or
// 71B710 publication. Unsupported/changed reached terminals are explicit errors.
void attach_native_extra_item_00b8a080(void* actual_item, void* actual_instance,
    void* unused_record, void* actual_node, std::uint32_t unused_creation_word,
    NativeResourceExtraItemAttachContext&);

// Source interfaces over genuine storage, not original callable ABI. The
// arbitrary instance profiles, graph application wiring, hardware faults,
// FH3/SEH and gameplay remain outside this packet.
} // namespace bsp

#pragma once
#include "bsp/native_game_resource_classification.hpp"

namespace bsp {
// Bind actual item type services and table data. Profiles are captured from
// the actual object, and only the selected slot is read. A source-owned numeric
// profile requires its real table resolver; no numeric code address is called.
class NativeResourceInstancePublicationCalls : public NativeResourceItemTypeCalls {
public:
    virtual std::uint32_t item_slot(std::uint32_t captured_profile,
        std::uint32_t byte_offset) noexcept = 0;
    virtual std::uint32_t item_type_token(std::uint32_t captured_entry,
        void* actual_item) = 0;
};
struct NativeResourceInstancePublicationContext {
    const volatile std::uint32_t& type44_00e19be4;
    const volatile std::uint32_t& type54_00e19a98;
    const volatile std::uint32_t& type74_00e19b74;
    const volatile std::uint32_t& type64_00e19b64;
    const volatile std::uint32_t& type54_00e19b54;
    NativeResourceInstancePublicationCalls& calls;
};

// Full82-byte B89E90 caller: append {node,item} to actual instance+20, call
// current item slot8, find/create that unsigned type-token key in map+30,
// append the same pair to its mapped vector. No retain/release or rollback.
// Native ECX instance, stack(item,node),RET8. Source EDX adds context.
void __fastcall publish_native_resource_instance_item_00b89e90(void* instance,
    NativeResourceInstancePublicationContext*, void* item, void* node);

// Full242-byte71B710: base publication first, then five independent slotC
// predicates. Append {item,node} at3C/4C/5C/6C/7C for every match. Capture the
// table before each current token read; capture slotC after that read. A
// predicate may mutate subsequent tables/tokens; failures keep prior appends.
// Its post-base null test does not make null an accepted base input.
void __fastcall publish_native_game_resource_instance_item_0071b710(void* instance,
    NativeResourceInstancePublicationContext*, void* item, void* node);

// Borrows an actual AP-constructed instance, initialized tree and consistent
// checked storage. No ownership is transferred for pointed items/nodes. The
// source storage bindings reuse checked-vector and tree algorithms; general
// original STL insertion/iterator ABI, structural allocator reentry, private
// native stack aliases, native FH3 and executable graph admission remain open.
} // namespace bsp

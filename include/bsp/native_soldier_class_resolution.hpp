#pragma once

#include "bsp/native_string.hpp"

namespace bsp {

// Actual stable table identities, kept outside all constructed bytes. The raw
// string and registry publications are borrowed; this context owns no storage.
struct NativeSoldierClassConstructionAccess {
    const std::uint32_t actual_base_vtable_00ce660c;
    const std::uint32_t actual_soldier_vtable_00ce7150;
    const std::uint32_t actual_registry_vtable_00ce7160;
    const std::uint32_t actual_singleton_base_vtable_00ce3818;
    NativeStringRawPoolContext& strings;
    void* volatile& actual_registry_publication_00e187f0;
};

// Complete55B allocation leaves: no input, EAX node, RET. Allocate1Ch; guarded
// independent zero links0/4/8, color18=1, nil19=0, other bytes untouched.
void* allocate_native_string_value_tree_node_00443e20();
void* allocate_native_soldier_registry_node_004afed0();

// Complete26B base: ECX actual class, EAX same, RET. Table, zero8h name at+4,
// byte+C=0, two handle words+10/+14=0. Bytes+D..F and+18 onward untouched.
void* construct_native_named_class_base_00489e60(
    void* actual_class, std::uint32_t actual_vtable_00ce660c);

// Complete175B destructor: install base table; release CURRENT+10 then+14
// handles through genuine41DE40 semantics, then return current+4 name through
// actual raw pool. FH3 states1/0/-1 retain the remaining member cleanup on a
// C++ exception. No owner free, rollback, native personality or SEH bridge.
void destroy_native_named_class_base_00489e80(
    void* actual_class, const NativeSoldierClassConstructionAccess&);

// Complete17B constructor-unwind/base tail: clear CURRENT publication before
// stamping the base profile. It does not free the owner or any map node.
void unwind_native_soldier_registry_004af950(
    void* actual_owner, const NativeSoldierClassConstructionAccess&) noexcept;

// Complete104B registry constructor: table at+0, sentinel at+8 and count+C.
// Preserve opaque+4; on allocation failure invoke genuine4AF950 effects.
void* construct_native_soldier_registry_004b11a0(
    void* actual_owner, const NativeSoldierClassConstructionAccess&);

// Complete109B SoldierClass constructor: real489E60 base, derived table,
// sentinel at+2C and count+30. Preserve+18..2B; allocation failure invokes the
// complete489E80 base cleanup, including new-handler changes to members.
void* construct_native_soldier_class_004b12a0(
    void* actual_class, const NativeSoldierClassConstructionAccess&);

// Complete3B vslot+Ch target: MOV AL,1; RET. Ignores receiver, no retain or
// other side effect. New C++ bool interface does not preserve upper EAX bits.
bool native_soldier_class_true_004af520() noexcept;

// These are new source interfaces, not original ABI/FH3 replacements. The
// factory, cache insertion, registry teardown and reader still need their full
// source closures; storing actual table identities does not implement them.
} // namespace bsp

#pragma once

#include "bsp/native_string.hpp"

namespace bsp {

// Borrow actual vtable identities; never create replacement tables or retain the
// class. Identities must remain stable during the call. The string context is
// used only by constructor-failure cleanup of the CURRENT embedded headers.
struct NativeDamageableClassConstructionAccess {
    const std::uint32_t actual_ref_counted_vtable_00ceb130;
    const std::uint32_t actual_damageable_vtable_00d0e13c;
    NativeStringRawPoolContext& strings;
};

// Complete std::_Tree::_Buynode 00877FA0. Native no register inputs, EAX raw
// 58h node, RET. Three zero links, color+54=1, isnil+55=0; all other bytes
// remain uninitialized. Uses the canonical malloc/new-handler allocation domain.
void* allocate_native_damageable_tree_node_00877fa0();

// Complete ordinary 0087C640 (ECX actual class, EAX same class, RET), including
// C9686F/DC8CC0 state5's six-action unwind in the C++ exception domain. Preserve
// unspecified object bytes and partial initialization. This interface does not
// install the original FH3 metadata or promise native SEH/binary compatibility.
void* construct_native_damageable_class_0087c640(void* actual_class,
    const NativeDamageableClassConstructionAccess&);

// Complete 0087AB30: ECX first, EDX end, two ignored stack words, RET8.
// Release +Ch owners in each 10h record through atomic refcount+4 and actual
// vslot0 on zero; clear each nonnull owner field after its release returns.
void release_native_damageable_owner_rows_0087ab30(void* first, const void* end);

// Complete 0087C260: ECX header, RET. Release current [begin+4,end+8), reload
// begin after callbacks, free it, then zero +4/+8/+C. Preserve header+0.
void destroy_native_damageable_owner_vector_0087c260(void* actual_header);

// Complete 00879240 and its 00879830 tail thunk. ECX header, RET. Capture end,
// dispatch each inline 30h record's actual vslot0(flags0), reload/free begin,
// then zero +4/+8/+C. Preserve header+0.
void destroy_native_damageable_effect_vector_00879240(void* actual_header);
void destroy_native_damageable_effect_vector_00879830(void* actual_header);

// Complete 0081B0A0 through BF6989's returning-free tail. ECX {data,count,
// capacity}, RET. Negative capacity invokes actual 74D190 reserve(0); decrease
// positive count one at a time, zero count, free captured data. Data/capacity
// remain as left by reserve; this is deliberately not an owning C++ container.
void destroy_native_class_point_array_0081b0a0(void* actual_header);

} // namespace bsp

#pragma once
#include <cstdint>

namespace bsp {
struct NativeSessionMessageStorage;
struct NativeSessionDeferredMessageNode {
    NativeSessionDeferredMessageNode* next_00;
    NativeSessionDeferredMessageNode* previous_04;
    NativeSessionMessageStorage* message_08;
};

// Actual list owner accesses only +4 (sentinel) and +8 (count). These functions
// do not declare its unknown +0 field or full allocation/class layout.
// Native 0077BAC0 ignores ECX, takes three stack arguments, returns its node in
// EAX and RET12. The third argument is a POINTER CELL, read after allocation.
NativeSessionDeferredMessageNode* allocate_native_session_deferred_node_0077bac0(
    NativeSessionDeferredMessageNode* next,
    NativeSessionDeferredMessageNode* previous,
    NativeSessionMessageStorage* const volatile* message_pointer_cell);

// Native no-argument RET/EAX node; sentinel payload +8 is left untouched.
NativeSessionDeferredMessageNode* create_native_session_deferred_head_00780830();
// Native ECX owner, RET/EAX owner; +0 is untouched.
void* construct_native_session_deferred_storage_007808c0(void* actual_owner);
// Native ECX owner, stack increment, RET4. Same complete count/error contract
// as 004CEE30, including the owning Source exception and its different ABI.
void grow_native_session_deferred_count_0077dda0(
    void* actual_owner, std::uint32_t increment);
// Native ECX owner, RET. Frees nodes/sentinel, clears +4/+8, and does not
// inspect, destroy, or free any pointed-to message. Source interfaces differ
// from the original register ABIs. No category-47 dispatcher is supplied.
void destroy_native_session_deferred_storage_00780860(void* actual_owner);
} // namespace bsp

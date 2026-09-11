#include "bsp/native_retained_memory_owners.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native retained memory owners require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(LONG) == 4);

std::uint32_t word(const volatile void* base, std::uint32_t byte_offset = 0) noexcept {
    std::uint32_t result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
void put(void* base, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
void* pointer(std::uint32_t bits) noexcept { return reinterpret_cast<void*>(bits); }
volatile LONG* references(void* owner) noexcept {
    return reinterpret_cast<volatile LONG*>(reinterpret_cast<std::uintptr_t>(owner) + 4u);
}
const volatile std::uint32_t* current_table(void* owner,
    NativeRetainedMemoryOwnerContext& context) noexcept {
    const auto profile = word(owner);
    if (profile == 0x00d15ad8) return context.actual_backing_profile_00d15ad8;
    if (profile == 0x00d642c0) return context.actual_stream_profile_00d642c0;
    __assume(0);
}
void invoke_current_zero_reference_slot(void* owner, NativeRetainedMemoryOwnerContext& context) {
    const auto invoker = word(current_table(owner, context));
    __assume(invoker == 0x00bd30e0);
    // The complete BD30E0 invoker reloads the owner's table for its flag1 call.
    const auto terminal = word(current_table(owner, context), 4);
    if (terminal == 0x008d4470) {
        delete_native_memory_backing_008d4470(owner, 1, context);
        return;
    }
    if (terminal == 0x00bb8f90) {
        delete_native_memory_stream_00bb8f90(owner, 1, context);
        return;
    }
    __assume(0);
}
void destroy_stream_base(void* owner) noexcept {
    // Complete BB86E0 unwind action and the same normal BEF9C0 tail.
    put(owner, 0, 0x00d5c104);
    put(owner, 0, 0x00ceb130);
}
struct BackingConstructionCleanup {
    void* owner;
    bool armed = true;
    ~BackingConstructionCleanup() noexcept {
        if (armed) put(owner, 0, 0x00ceb130);
    }
};
struct StreamCleanup {
    void* owner;
    bool armed = true;
    ~StreamCleanup() noexcept {
        if (armed) destroy_stream_base(owner);
    }
};
} // namespace

void* construct_native_memory_backing_008d43c0(void* owner,
    std::int32_t request, NativeRetainedMemoryOwnerContext& context) {
    put(owner, 0, 0x00ceb130);
    put(owner, 4, 1);
    BackingConstructionCleanup cleanup{owner}; // Native cleanup map, no catch.
    put(owner, 0, 0x00d15ad8);
    const auto allocation_bytes = request > 0 ? static_cast<std::uint32_t>(request) : 1u;
    put(owner, 12, request > 0 ? static_cast<std::uint32_t>(request) : 0u);
    auto* const data = singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, allocation_bytes, allocation_bytes});
    put(owner, 8, reinterpret_cast<std::uintptr_t>(data));
    context.actual_object_count_0109db98 = context.actual_object_count_0109db98 + 1u;
    context.actual_requested_bytes_0109db9c = context.actual_requested_bytes_0109db9c +
        static_cast<std::uint32_t>(request);
    cleanup.armed = false;
    return owner;
}

void destroy_native_memory_backing_008d4440(
    void* owner, NativeRetainedMemoryOwnerContext& context) noexcept {
    auto* const captured_data = pointer(word(owner, 8));
    put(owner, 0, 0x00d15ad8);
    singleton_lifetime_free(captured_data);
    context.actual_object_count_0109db98 = context.actual_object_count_0109db98 - 1u;
    const auto current_length = word(owner, 12);
    context.actual_requested_bytes_0109db9c = context.actual_requested_bytes_0109db9c - current_length;
    put(owner, 0, 0x00ceb130);
}

void* delete_native_memory_backing_008d4470(void* owner,
    std::uint32_t flags, NativeRetainedMemoryOwnerContext& context) noexcept {
    destroy_native_memory_backing_008d4440(owner, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(owner);
    return owner;
}

void* create_native_memory_stream_from_backing_00bef6d0(
    void* backing, NativeRetainedMemoryOwnerContext& context) {
    auto* const owner = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x14, 0x14});
    if (owner) {
        put(owner, 0, 0x00ceb130);
        put(owner, 4, 1);
        put(owner, 0, 0x00d642c0);
        put(owner, 8, 0);
        put(owner, 12, 0);
        put(owner, 16, 0);
    }
    // BEF708 performs this current load even after the allocation-null branch.
    // No safe null result or rollback is present in the complete original body.
    if (auto* const old_backing = pointer(word(owner, 8))) {
        if (InterlockedDecrement(references(old_backing)) == 0)
            invoke_current_zero_reference_slot(old_backing, context);
        put(owner, 8, 0);
    }
    put(owner, 8, reinterpret_cast<std::uintptr_t>(backing));
    InterlockedIncrement(references(backing));
    const auto captured_data = word(backing, 8);
    put(owner, 16, captured_data);
    const auto current_length = word(backing, 12);
    put(owner, 12, captured_data + current_length);
    return owner;
}

void destroy_native_memory_stream_00bef9c0(
    void* owner, NativeRetainedMemoryOwnerContext& context) {
    put(owner, 0, 0x00d642c0);
    auto* const captured_backing = pointer(word(owner, 8));
    StreamCleanup cleanup{owner}; // State0 follows the capture/test; no catch.
    if (captured_backing) {
        if (InterlockedDecrement(references(captured_backing)) == 0)
            invoke_current_zero_reference_slot(captured_backing, context);
        put(owner, 8, 0);
    }
    cleanup.armed = false;
    destroy_stream_base(owner); // Native disarms state0 before this tail.
}

void* delete_native_memory_stream_00bb8f90(void* owner,
    std::uint32_t flags, NativeRetainedMemoryOwnerContext& context) {
    destroy_native_memory_stream_00bef9c0(owner, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(owner);
    return owner;
}

void* assign_native_retained_memory_slot_00b23640(void* destination,
    const void* source, NativeRetainedMemoryOwnerContext& context) {
    auto* const replacement = pointer(word(source));
    auto* const previous = pointer(word(destination));
    if (previous != replacement) {
        put(destination, 0, reinterpret_cast<std::uintptr_t>(replacement));
        if (replacement) InterlockedIncrement(references(replacement));
        if (previous && InterlockedDecrement(references(previous)) == 0)
            invoke_current_zero_reference_slot(previous, context);
    }
    return destination;
}
} // namespace bsp

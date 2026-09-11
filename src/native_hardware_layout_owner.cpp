#include "bsp/native_hardware_layout_owner.hpp"
#include "bsp/native_hardware_layout_tree.hpp"
#include "bsp/native_vertex_declaration_owner.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <unknwn.h>

#include <cstring>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native hardware layout owners require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 24);

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
std::uint16_t half(const void* base, std::uint32_t byte_offset) noexcept {
    std::uint16_t result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ax, word ptr [eax + edx]
        mov result, ax
    }
    return result;
}
void put_half(void* base, std::uint32_t byte_offset, std::uint16_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov cx, value
        mov word ptr [eax + edx], cx
    }
}
void* pointer(std::uint32_t bits) noexcept { return reinterpret_cast<void*>(bits); }
void* at(const void* base, std::uint32_t byte_offset) noexcept {
    return pointer(reinterpret_cast<std::uintptr_t>(base) + byte_offset);
}
std::int32_t signed_word(std::uint32_t bits) noexcept {
    std::int32_t result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

const volatile std::uint32_t* declaration_table(
    void* declaration, NativeHardwareLayoutOwnerContext& context) noexcept {
    const auto current_profile = word(declaration);
    __assume(current_profile == 0x00d61d1c);
    return context.actual_declaration_profile_00d61d1c;
}

// Native BF7C10 and FH3 unwind-action filters terminate during exception
// SEARCH. A C++ catch/rethrow or noexcept alone can run extra nested cleanup.
int terminate_cpp_cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_records(void* begin, unsigned remaining,
    NativeHardwareLayoutOwnerContext& context) noexcept {
    __try {
        while (remaining != 0) {
            --remaining;
            destroy_native_hardware_layout_record_00b483f0(at(begin, remaining * 12u), context);
        }
    } __except (terminate_cpp_cleanup_exception(GetExceptionCode())) {
        __assume(0);
    }
}
void unwind_base(void* owner, unsigned state,
    NativeHardwareLayoutOwnerContext& context) noexcept {
    __try {
        if (state != 0) destroy_native_hardware_layout_records_00b48950(at(owner, 8), context);
        put(owner, 0, 0x00ceb130);
    } __except (terminate_cpp_cleanup_exception(GetExceptionCode())) {
        __assume(0);
    }
}
void unwind_derived(void* owner, NativeHardwareLayoutOwnerContext& context) noexcept {
    __try {
        destroy_native_hardware_layout_base_00b48960(owner, context);
    } __except (terminate_cpp_cleanup_exception(GetExceptionCode())) {
        __assume(0);
    }
}

// Cleanup-only native FH3 maps have no catch handlers. Armed local destructors
// express those maps without adding a catch(...) that wins exception search.
struct BaseCleanup {
    void* owner;
    NativeHardwareLayoutOwnerContext& context;
    unsigned state = 1;
    bool armed = true;
    ~BaseCleanup() noexcept {
        if (armed) unwind_base(owner, state, context);
    }
};
struct DerivedCleanup {
    void* owner;
    NativeHardwareLayoutOwnerContext& context;
    bool armed = true;
    ~DerivedCleanup() noexcept {
        if (armed) unwind_derived(owner, context);
    }
};
} // namespace

void destroy_native_hardware_layout_record_00b483f0(
    void* record, NativeHardwareLayoutOwnerContext& context) {
    auto* const declaration = pointer(word(record));
    if (!declaration) return;
    if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(at(declaration, 4))) == 0) {
        const auto zero_terminal = word(declaration_table(declaration, context));
        __assume(zero_terminal == 0x00bd30e0);
        // BD30E0 rereads the current table before its deleting-slot call.
        const auto deleting_terminal = word(declaration_table(declaration, context), 4);
        __assume(deleting_terminal == 0x00b48ca0);
        delete_native_vertex_declaration_00b48ca0(declaration, 1,
            context.actual_declaration_pool_0108fd38, context.actual_type_sizes_00d61cc0);
    }
    put(record, 0, 0);
}

void destroy_native_hardware_layout_records_00b48950(
    void* begin, NativeHardwareLayoutOwnerContext& context) {
    unsigned remaining = 4;
    bool completed = false;
    // BF7C6E is an SEH finally, not a C++ destructor unwind. Matching it here
    // also avoids adding a C++ ProcessingThrow count while invoking BF7C10.
    __try {
        while (remaining != 0) {
            --remaining;
            destroy_native_hardware_layout_record_00b483f0(at(begin, remaining * 12u), context);
        }
        completed = true;
    } __finally {
        if (!completed) unwind_records(begin, remaining, context);
    }
}

void destroy_native_hardware_layout_base_00b48960(
    void* owner, NativeHardwareLayoutOwnerContext& context) {
    put(owner, 0, 0x00d61d10);
    auto* const renderer = context.actual_renderer_00f8d394;
    const auto renderer_profile = word(renderer);
    __assume(renderer_profile == 0x00d5f0a8);
    const auto notification = word(context.actual_renderer_profile_00d5f0a8, 0x44);
    __assume(notification == 0x00b2f4c0);
    BaseCleanup cleanup{owner, context}; // Native loads precede state1 arming.
    remove_native_hardware_layout_value_00b2f4c0(
        context.actual_tree_0108d530, owner, context.invalid_parameters);
    cleanup.state = 0;
    destroy_native_hardware_layout_records_00b48950(at(owner, 8), context);
    cleanup.armed = false;
    put(owner, 0, 0x00ceb130);
}

void destroy_native_hardware_layout_00b60700(
    void* owner, NativeHardwareLayoutOwnerContext& context) {
    put(owner, 0, 0x00d62af4);
    auto* const captured_com = reinterpret_cast<IUnknown*>(word(owner, 0x40));
    DerivedCleanup cleanup{owner, context}; // State0 follows the capture/test.
    if (captured_com) {
        captured_com->Release();
        put(owner, 0x40, 0);
    }
    resource_support_singleton_00b3e730(
        context.actual_support_0108fedc, context.actual_lifetime_01090aa0);
    cleanup.armed = false;
    // Native disarms state0 before this call; a normal base failure is not
    // retried by the derived owner's cleanup action.
    destroy_native_hardware_layout_base_00b48960(owner, context);
}

void* delete_native_hardware_layout_00b60770(
    void* owner, std::uint32_t flags, NativeHardwareLayoutOwnerContext& context) {
    destroy_native_hardware_layout_00b60700(owner, context);
    if ((flags & 1u) != 0)
        return_native_hardware_layout_slot_00b60110(context.actual_hardware_layout_pool_0108fe9c, owner);
    return owner;
}

void return_native_hardware_layout_slot_00b60110(void* pool, void* slot) {
    auto* const critical_section = reinterpret_cast<CRITICAL_SECTION*>(at(pool, 0x0c));
    EnterCriticalSection(critical_section);
    put(pool, 0x24, word(pool, 0x24) + 1u);
    const auto slab_index = word(slot, 0x44);
    auto* const slab = pointer(word(pointer(word(pool, 0x28)), slab_index * 4u));
    const auto displacement = signed_word(static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(slot) - reinterpret_cast<std::uintptr_t>(slab)));
    const auto slot_index = static_cast<std::uint16_t>(displacement / 0x48);
    const auto old_count = half(slab, 0x940);
    put_half(slab, 0x900u + static_cast<std::uint32_t>(old_count) * 2u, slot_index);
    put_half(slab, 0x940, static_cast<std::uint16_t>(half(slab, 0x940) + 1u));
    if (slab_index < word(pool, 0x34)) put(pool, 0x34, slab_index);
    put(pool, 0x24, word(pool, 0x24) - 1u);
    LeaveCriticalSection(critical_section);
}
} // namespace bsp

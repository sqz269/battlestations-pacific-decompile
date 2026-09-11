#include "bsp/native_hardware_layout_fields.hpp"
#include "bsp/native_vertex_declaration_owner.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstring>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native hardware layout fields require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

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
void* pointer(std::uint32_t value) noexcept { return reinterpret_cast<void*>(value); }
void* at(const void* base, std::uint32_t byte_offset) noexcept {
    return pointer(reinterpret_cast<std::uintptr_t>(base) + byte_offset);
}
std::int32_t signed_word(std::uint32_t value) noexcept {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
void retain(void* declaration) noexcept {
    if (declaration)
        InterlockedIncrement(reinterpret_cast<volatile LONG*>(at(declaration, 4)));
}
void release(void* declaration, NativeHardwareLayoutOwnerContext& context) {
    if (!declaration ||
        InterlockedDecrement(reinterpret_cast<volatile LONG*>(at(declaration, 4))) != 0) return;
    auto current_profile = word(declaration);
    __assume(current_profile == 0x00d61d1c);
    const auto zero_terminal = word(context.actual_declaration_profile_00d61d1c);
    __assume(zero_terminal == 0x00bd30e0);
    // BD30E0 reloads the current profile before the deleting slot.
    current_profile = word(declaration);
    __assume(current_profile == 0x00d61d1c);
    const auto deleting_terminal = word(context.actual_declaration_profile_00d61d1c, 4);
    __assume(deleting_terminal == 0x00b48ca0);
    delete_native_vertex_declaration_00b48ca0(declaration, 1,
        context.actual_declaration_pool_0108fd38, context.actual_type_sizes_00d61cc0);
}
int terminate_cpp_cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_temporary(void* temporary, NativeHardwareLayoutOwnerContext& context) noexcept {
    __try {
        destroy_native_hardware_layout_record_00b483f0(temporary, context);
    } __except (terminate_cpp_cleanup_exception(GetExceptionCode())) {
        __assume(0);
    }
}
struct TemporaryCleanup {
    void* record;
    NativeHardwareLayoutOwnerContext& context;
    bool armed = true;
    ~TemporaryCleanup() noexcept {
        if (armed) unwind_temporary(record, context);
    }
};
} // namespace

void* initialize_native_hardware_layout_record_00b47640(void* record) noexcept {
    put(record, 0, 0);
    put(record, 4, 1);
    put(record, 8, 0);
    return record;
}

void* initialize_native_hardware_layout_base_00b48c00(void* owner) noexcept {
    put(owner, 0, 0x00ceb130);
    put(owner, 4, 1);
    put(owner, 0, 0x00d61d10);
    for (std::uint32_t i = 0; i != 4; ++i)
        initialize_native_hardware_layout_record_00b47640(at(owner, 8u + i * 12u));
    put(owner, 0x38, 0);
    put(owner, 0x3c, 0);
    return owner;
}

void append_native_hardware_layout_stream_00b48a00(
    void* owner, void* declaration, NativeHardwareLayoutOwnerContext& context) {
    std::uint32_t temporary[3]{0, 1, 0};
    TemporaryCleanup cleanup{temporary, context};
    if (declaration) {
        put(temporary, 0, reinterpret_cast<std::uintptr_t>(declaration));
        retain(declaration);
    }
    const auto old_count = word(owner, 0x38);
    auto* const record = at(owner, old_count * 12u + 8u);
    put(owner, 0x38, old_count + 1u);
    auto* const old_declaration = pointer(word(record));
    if (old_declaration != declaration) {
        put(record, 0, reinterpret_cast<std::uintptr_t>(declaration));
        retain(declaration);
        release(old_declaration, context);
    }
    put(record, 4, 1);
    put(record, 8, 0);
    cleanup.armed = false;
    release(declaration, context);
}

void recompute_native_hardware_layout_stride_00b47d60(void* owner) noexcept {
    const auto initial_count = signed_word(word(owner, 0x38));
    put(owner, 0x3c, 0);
    if (initial_count <= 0) return;
    auto* record = at(owner, 8);
    std::uint32_t index = 0;
    do {
        const auto size = word(pointer(word(record)), 0xcc);
        put(owner, 0x3c, word(owner, 0x3c) + size);
        ++index;
        record = at(record, 12);
    } while (signed_word(index) < signed_word(word(owner, 0x38)));
}
} // namespace bsp

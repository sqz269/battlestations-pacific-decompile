#include "bsp/native_memory_stream.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native memory stream leaves require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && native_memory_stream_bytes == 0x14 &&
    native_memory_backing_bytes == 0x10);

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
__declspec(noinline) void* __cdecl copy_bytes(void* destination,
    const void* source, std::size_t count) noexcept {
    return std::memmove(destination, source, count);
}
const volatile std::uint32_t* current_profile(const void* owner,
    NativeRetainedMemoryOwnerContext& context) noexcept {
    const auto profile = word(owner);
    if (profile == 0x00d642c0) return context.actual_stream_profile_00d642c0;
    if (profile == 0x00d15ad8) return context.actual_backing_profile_00d15ad8;
    __assume(0); // Explicit original-class input domain; no fabricated fallback.
}
void require_stream_slot(const void* owner, std::uint32_t offset,
    std::uint32_t identity, NativeRetainedMemoryOwnerContext& context) noexcept {
    const auto profile = word(owner);
    __assume(profile == 0x00d642c0);
    const auto slot = word(context.actual_stream_profile_00d642c0, offset);
    __assume(slot == identity);
}
struct RawBackingConstructionCleanup {
    void* raw;
    bool armed = true;
    ~RawBackingConstructionCleanup() noexcept {
        if (armed) singleton_lifetime_free(raw);
    }
};
} // namespace

__declspec(naked) bool __fastcall native_memory_stream_open_00bef4c0(
    const void*, void*) noexcept {
    __asm {
        mov al, 1
        ret
    }
}

__declspec(naked) std::int64_t __fastcall native_memory_stream_length_00bef600(
    const void*, void*) noexcept {
    __asm {
        mov edx, dword ptr [ecx + 8]
        mov eax, dword ptr [ecx + 0ch]
        sub eax, dword ptr [edx + 8]
        cdq
        ret
    }
}

__declspec(naked) std::uint8_t* __fastcall native_memory_stream_data_00bef610(
    const void*, void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 8]
        mov eax, dword ptr [eax + 8]
        ret
    }
}

__declspec(naked) void __fastcall native_memory_stream_read_00bef590(
    void*, void*, void*, std::uint32_t, std::uint32_t*) noexcept {
    __asm {
        push esi
        mov esi, dword ptr [esp + 0ch]
        push edi
        mov edi, ecx
        mov ecx, dword ptr [edi + 10h]
        mov eax, dword ptr [edi + 0ch]
        sub eax, ecx
        cmp esi, eax
        cmovnc esi, eax
        cmp esi, 4
        jnz read_two
        mov eax, dword ptr [ecx]
        mov ecx, dword ptr [esp + 0ch]
        mov dword ptr [ecx], eax
        jmp read_advance
    read_two:
        cmp esi, 2
        jnz read_general
        mov dx, word ptr [ecx]
        mov eax, dword ptr [esp + 0ch]
        mov word ptr [eax], dx
        jmp read_advance
    read_general:
        push esi
        push ecx
        mov ecx, dword ptr [esp + 14h]
        push ecx
        // BF7680 has backward-overlap handling at BF7844. The host CRT
        // binding is memmove, including zero length, rather than memcpy UB.
        call copy_bytes
        add esp, 0ch
    read_advance:
        mov eax, dword ptr [esp + 14h]
        add dword ptr [edi + 10h], esi
        test eax, eax
        jz read_done
        mov dword ptr [eax], esi
    read_done:
        pop edi
        pop esi
        ret 0ch
    }
}

bool is_native_memory_stream_00d642c0(const void* owner) noexcept {
    return owner && word(owner) == 0x00d642c0;
}
bool dispatch_native_memory_stream_open(void* owner,
    NativeRetainedMemoryOwnerContext& context) noexcept {
    require_stream_slot(owner, 0x18, 0x00bef4c0, context);
    return native_memory_stream_open_00bef4c0(owner, nullptr);
}
std::int64_t dispatch_native_memory_stream_length(void* owner,
    NativeRetainedMemoryOwnerContext& context) noexcept {
    require_stream_slot(owner, 0x30, 0x00bef600, context);
    return native_memory_stream_length_00bef600(owner, nullptr);
}
void dispatch_native_memory_stream_read(void* owner, void* destination,
    std::uint32_t requested, std::uint32_t* actual,
    NativeRetainedMemoryOwnerContext& context) noexcept {
    require_stream_slot(owner, 0x24, 0x00bef590, context);
    native_memory_stream_read_00bef590(owner, nullptr, destination, requested, actual);
}

void dispatch_native_memory_owner_zero_reference(void* owner,
    NativeRetainedMemoryOwnerContext& context) {
    if (!owner) return; // BD30E0's null branch.
    const auto invoker = word(current_profile(owner, context));
    __assume(invoker == 0x00bd30e0);
    // BD30E0 reloads the current table before calling slot4 with flag1.
    const auto terminal = word(current_profile(owner, context), 4);
    if (terminal == 0x00bb8f90) {
        delete_native_memory_stream_00bb8f90(owner, 1, context);
        return;
    }
    if (terminal == 0x008d4470) {
        delete_native_memory_backing_008d4470(owner, 1, context);
        return;
    }
    __assume(0);
}

void* create_native_memory_stream_from_copy_00befa40(const void* source,
    std::uint32_t count_low, std::uint32_t /* count_high */,
    NativeRetainedMemoryOwnerContext& context) {
    if (!source) return nullptr;
    auto* const raw = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x10, 0x10});
    void* backing = nullptr;
    {
        // E022D8 maxState1 -> E022D0 state0 -> CC76D0 scalar-free raw.
        RawBackingConstructionCleanup cleanup{raw};
        if (raw) backing = construct_native_memory_backing_008d43c0(
            raw, static_cast<std::int32_t>(count_low), context);
        cleanup.armed = false; // State -1 before BF7680; no later rollback.
    }
    auto* const bytes = reinterpret_cast<void*>(word(backing, 8));
    std::memmove(bytes, source, count_low);
    auto* const stream = create_native_memory_stream_from_backing_00bef6d0(backing, context);
    auto* const references = reinterpret_cast<volatile LONG*>(
        reinterpret_cast<std::uintptr_t>(backing) + 4u);
    if (InterlockedDecrement(references) == 0)
        dispatch_native_memory_owner_zero_reference(backing, context);
    return stream;
}
} // namespace bsp

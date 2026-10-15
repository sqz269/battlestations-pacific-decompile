#include "bsp/native_pod8_failure.hpp"

#include "bsp/native_allocator_base_cleanup.hpp"
#include "bsp/native_allocator_base_copy_constructor.hpp"
#include "bsp/native_allocator_owned_message_constructor.hpp"
#include "bsp/native_singleton_vector_allocation.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
__declspec(naked) void __fastcall destroy_native_pod8_allocation_exception_004010e0(
    void*, std::uint32_t) {
    __asm {
        mov dword ptr [ecx], 00d6923ch // 004010E0: actual derived profile.
        jmp cleanup_native_allocator_base_00bf6454 // 004010E6: genuine base.
    }
}

__declspec(naked) void* __fastcall copy_native_pod8_allocation_exception_0040d810(
    void*, std::uint32_t, const void*) {
    __asm {
        mov eax, dword ptr [esp + 4] // 0040D810: actual source address.
        push esi // 0040D814.
        push eax // 0040D815: only original stack argument for base copy.
        mov esi, ecx // 0040D816: capture destination before actual child.
        call construct_native_allocator_base_copy_00bf63a6 // 0040D818.
        mov dword ptr [esi], 00d6923ch // 0040D81D: unconditional publication.
        mov eax, esi // 0040D823: original destination, not child's return.
        pop esi // 0040D825.
        ret 4 // 0040D826: original single source stack word.
    }
}

NativePod8AllocationOverflow::NativePod8AllocationOverflow() {
    // Original 004133B9..004133C9: actual null slot, actual uninitialized raw
    // 12-byte backing. The base's null path includes its real +4 DWORD RMW.
    char* null_message = nullptr;
    (void)construct_native_allocator_owned_message_00bf6340(
        &storage_, 0u, &null_message);
    storage_.native_vtable_00 = 0x00d6923c; // Original 004133D8.
}

NativePod8AllocationOverflow::NativePod8AllocationOverflow(
    const NativePod8AllocationOverflow& source) {
    (void)copy_native_pod8_allocation_exception_0040d810(
        &storage_, 0u, &source.storage_);
}

NativePod8AllocationOverflow::~NativePod8AllocationOverflow() noexcept {
    destroy_native_pod8_allocation_exception_004010e0(&storage_, 0u);
}

const char* NativePod8AllocationOverflow::what() const noexcept {
    // Native BF646A uses this same null fallback, without ownership mutation.
    const char* const message = storage_.base_message_04;
    return message != nullptr ? message : "Unknown exception";
}

void* __fastcall allocate_native_pod8_records_00413390(std::uint32_t count, void*) {
    // Keep the original unsigned quotient predicate, including the zero path
    // which bypasses DIV but still reaches the actual allocation(0) provider.
    if (count != 0u && 0xffffffffu / count < 8u) {
        throw NativePod8AllocationOverflow{};
    }
    const std::uint32_t bytes = count * 8u;
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}

[[noreturn]] void __cdecl throw_native_pod8_vector_length_error_004203a0() {
    // Matched complete105B body and one-state FH3 map. The actual provider
    // retains the delayed temporary cleanup,28h owner and owning Source throw.
    native_singleton_length_error_00bd0590();
}
} // namespace bsp

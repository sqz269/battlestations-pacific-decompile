#include "bsp/native_singleton_vector_allocation.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <new>

namespace bsp {
namespace {

// Native state 0 is armed only after 00408720 returns. FuncInfo DFF4BC,
// map DFF4B4: state0 -> -1, action CC5470 -> 004072D0 on [EBP-50h].
struct CompletedMessage {
    NativeLegacySboStringStorage& storage;
    ~CompletedMessage() noexcept {
        native_legacy_sbo_string_destroy_004072d0(storage);
    }
};

} // namespace

NativeSingletonVectorLengthError::NativeSingletonVectorLengthError(
    const NativeLegacySboStringStorage& message) {
    construct_native_legacy_logic_error_00411700(storage_, message);
    storage_.native_vtable_00 = 0x00d69260;
}

NativeSingletonVectorLengthError::NativeSingletonVectorLengthError(
    const NativeSingletonVectorLengthError& source) {
    copy_native_legacy_length_error_00411940(storage_, source.storage_);
}

NativeSingletonVectorLengthError::~NativeSingletonVectorLengthError() noexcept {
    destroy_native_legacy_logic_error_00411780(storage_);
}

void* __fastcall native_singleton_pointer_allocate_00bcfeb0(
    std::uint32_t count, void*) {
    // For positive count, native FFFFFFFF / count >= 4 admits multiplication.
    // Keep 3FFFFFFF admitted even though the CRT may reject FFFFFFFC bytes.
    if (count > 0x3fffffffu) {
        throw std::bad_alloc();
    }
    const std::uint32_t bytes = count * 4u;
    return singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, bytes, bytes});
}

[[noreturn]] void __cdecl native_singleton_length_error_00bd0590() {
    NativeLegacySboStringStorage temporary;
    temporary.capacity_18 = 15;
    temporary.length_14 = 0;
    temporary.buffer_04.inline_bytes[0] = '\0';
    native_legacy_sbo_string_assign_counted_00408720(
        temporary, "vector<T> too long", 18);
    const CompletedMessage completed{temporary};
    throw NativeSingletonVectorLengthError{temporary};
}

} // namespace bsp

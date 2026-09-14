#include "bsp/native_material_record_allocation.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native material record allocation requires MSVC Win32.
#endif

namespace bsp {
namespace {

// B135C0's FuncInfo DF438C has one state. Its DF4384 unwind map sends state0
// to -1 through CBC290, which calls 004072D0 on the temporary at [EBP-50h].
// The state store at B13605 occurs only after 00408720 returns.
struct CompletedMessage {
    NativeLegacySboStringStorage& storage;
    ~CompletedMessage() noexcept {
        native_legacy_sbo_string_destroy_004072d0(storage);
    }
};

} // namespace

void* __fastcall allocate_native_material_records_00b0d3b0(
    std::uint32_t count, void*) {
    // The native positive branch admits FFFFFFFF/count >= 300. The zero
    // branch also reaches BF681B with a zero-byte request.
    if (count > 0xffffffffu / 300u) {
        throw std::bad_alloc();
    }
    const std::uint32_t bytes = count * 300u;
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}

[[noreturn]] void __cdecl STL_xlen_throw_00b135c0() {
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

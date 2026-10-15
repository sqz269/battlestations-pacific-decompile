#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native POD8 failure reconstruction requires MSVC Win32.
#endif

namespace bsp {
// Actual 0Ch base/bad_alloc backing consumed by BF6340/BF63A6/BF6454 and
// E03CC0 catch metadata. No member SBO, callable vtable or implicit writes.
struct NativePod8AllocationExceptionStorage {
    std::uint32_t native_vtable_00;
    char* base_message_04;
    std::uint32_t owns_base_message_08;
};
static_assert(sizeof(NativePod8AllocationExceptionStorage) == 0x0c);
static_assert(offsetof(NativePod8AllocationExceptionStorage, base_message_04) == 4);
static_assert(offsetof(NativePod8AllocationExceptionStorage, owns_base_message_08) == 8);
static_assert(std::is_trivially_default_constructible_v<NativePod8AllocationExceptionStorage>);
static_assert(std::is_trivially_copyable_v<NativePod8AllocationExceptionStorage>);

// Complete 004010E0..004010EA (11B): publish raw D6923C, tail-JMP actual
// BF6454 cleanup. Original ECX actual raw12 owner, no stack input, plain RET
// through the child, no semantic result. Source EDX is an unused binding word.
// Child reads ownership before D69370 publication; nonzero calls actual current
// CRT free with the late message, including null. No extra reset/null guard.
void __fastcall destroy_native_pod8_allocation_exception_004010e0(
    void* actual_raw12_owner, std::uint32_t unused_edx);

// Complete 0040D810..0040D828 (25B): call actual BF63A6 base copy, then publish
// D6923C in captured destination and return it. Original ECX destination,
// stack source, RET4/EAX destination; Source EDX placeholder is not an Original
// parameter. Full base ownership, nullable malloc, alias and copy order retained.
void* __fastcall copy_native_pod8_allocation_exception_0040d810(
    void* actual_raw12_destination, std::uint32_t unused_edx,
    const void* actual_raw12_source);

// Owning Source catch transport for 00413390's direct multiplication overflow.
// Its sole member is the actual 0Ch owner: default construction calls BF6340
// with an actual null pointer-slot address, then publishes D6923C. Copies and
// destruction call the complete derived adapters above. Native null message
// and ownership DWORD1 are retained; what() returns "Unknown exception".
// This is a new Source catch type/RTTI/EH identity, not std::bad_alloc or the
// original E03CC0/BF6885 catch ABI. Raw vtable words remain address data.
class NativePod8AllocationOverflow final {
public:
    NativePod8AllocationOverflow();
    NativePod8AllocationOverflow(const NativePod8AllocationOverflow& source);
    NativePod8AllocationOverflow& operator=(const NativePod8AllocationOverflow&) = delete;
    ~NativePod8AllocationOverflow() noexcept;

    const NativePod8AllocationExceptionStorage& native_storage() const noexcept {
        return storage_;
    }
    const char* what() const noexcept;

private:
    NativePod8AllocationExceptionStorage storage_;
};
static_assert(sizeof(NativePod8AllocationOverflow) == 0x0c);

// Complete 00413390..004133E4. Original ECX uint32 count, no consumed EDX or
// stack argument, plain RET/EAX raw storage. The explicit Source fastcall EDX
// placeholder is not an Original input. Positive FFFFFFFF/count<8 throws the
// owning direct-overflow transport above; all other counts, INCLUDING ZERO,
// request exactly uint32(count*8) native/host bytes from the actual current
// malloc/_callnewh/retry allocation provider. Free via singleton_lifetime_free.
// Provider allocation failure retains its declared current std::bad_alloc
// policy; original static heap/new-handler/global bad_alloc identity is not
// fabricated. That provider failure is separate from the raw direct overflow.
void* __fastcall allocate_native_pod8_records_00413390(
    std::uint32_t count, void* unused_edx);

// Complete 004203A0..00420408 via matched actual 00BD0590 provider: no input
// or normal return. No-init1Ch temporary, capacity15/length0/NUL, assign exact
// 18-byte vector message, THEN arm temporary cleanup and construct/throw the
// actual28h owner through NativeSingletonVectorLengthError. The existing
// provider owns all temporary/copy/destruction work; no second cleanup guard.
// Current Source C++ EH/catch type replaces Original D83F98/FH3/BF6885.
[[noreturn]] void __cdecl throw_native_pod8_vector_length_error_004203a0();
} // namespace bsp

#pragma once

#include "bsp/native_legacy_exception_owner.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native singleton vector allocation requires MSVC Win32.
#endif

namespace bsp {

// Owning source exception transport for 00BD0590's actual 28h legacy payload.
// D69260 is retained as address data, never used as a host C++ vtable. This
// class has a new catch type, RTTI and C++ EH ABI; it is not std::length_error.
class NativeSingletonVectorLengthError final {
public:
    explicit NativeSingletonVectorLengthError(const NativeLegacySboStringStorage& message);
    NativeSingletonVectorLengthError(const NativeSingletonVectorLengthError& source);
    NativeSingletonVectorLengthError& operator=(const NativeSingletonVectorLengthError&) = delete;
    ~NativeSingletonVectorLengthError() noexcept;

    const NativeLegacyExceptionStorage& native_storage() const noexcept {
        return storage_;
    }

private:
    NativeLegacyExceptionStorage storage_;
};

static_assert(sizeof(NativeSingletonVectorLengthError) == 0x28);

// Complete 00BCFEB0..00BCFF04: count in ECX, no consumed EDX or stack input,
// EAX raw storage and RET0. This two-register fastcall declaration explicitly
// reserves EDX without using it; callers need not supply zero there.
// Zero requests zero bytes. Counts above 3FFFFFFF throw source std::bad_alloc;
// admitted counts request count*4 bytes from singleton_lifetime_allocate.
// Storage shares its existing malloc/free domain; no private manager/domain
// or original static-CRT heap/new-handler state is created by this entry.
void* __fastcall native_singleton_pointer_allocate_00bcfeb0(
    std::uint32_t count, void* unused_edx);

// Complete 00BD0590..00BD05F8: no consumed input, no normal return. Initialize
// the native temporary SBO, assign 18 message bytes, then arm its cleanup and
// throw the owning payload above. Source C++ EH replaces native BF6885/FH3;
// original RTTI identity, SEH records and arbitrary unwind-spill aliases are
// outside this declared service boundary.
[[noreturn]] void __cdecl native_singleton_length_error_00bd0590();

} // namespace bsp

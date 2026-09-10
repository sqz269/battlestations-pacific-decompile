#pragma once

#include "bsp/native_legacy_exception_owner.hpp"

#include <cstdint>

namespace bsp {

// Owning host transport for 004CE780's native length-error payload. The only
// member is the actual 28h legacy owner. Its native vtable is address data;
// the C++ class has new host RTTI, catch type and exception ABI.
class NativeAliasListLengthError final {
public:
    explicit NativeAliasListLengthError(const NativeLegacySboStringStorage& message);
    NativeAliasListLengthError(const NativeAliasListLengthError& source);
    NativeAliasListLengthError& operator=(const NativeAliasListLengthError&) = delete;
    ~NativeAliasListLengthError() noexcept;

    const NativeLegacyExceptionStorage& native_storage() const noexcept {
        return storage_;
    }

private:
    NativeLegacyExceptionStorage storage_;
};

static_assert(sizeof(NativeAliasListLengthError) == 0x28);

// 004CE780..004CE813: ECX actual list owner; stack increment; RET4, no semantic
// return. The count is the DWORD at owner+8. Preserve the native unsigned
// subtraction predicate, including its behavior for already-corrupt counts.
// The error branch throws an owning NativeAliasListLengthError; it does not
// write the count. This new interface is not a native binary replacement.
void grow_native_alias_list_count_004ce780(void* actual_owner, std::uint32_t increment);

} // namespace bsp

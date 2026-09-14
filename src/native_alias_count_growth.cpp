#include "bsp/native_alias_count_growth.hpp"

namespace bsp {
namespace {

// 00C65A80: armed only after counted assignment returns. The native funclet
// destroys the completed temporary SBO string during constructor/throw unwind.
struct CompletedTemporary {
    NativeLegacySboStringStorage& storage;
    ~CompletedTemporary() noexcept {
        native_legacy_sbo_string_destroy_004072d0(storage);
    }
};

} // namespace

NativeAliasListLengthError::NativeAliasListLengthError(
    const NativeLegacySboStringStorage& message) {
    construct_native_legacy_logic_error_00411700(storage_, message);
    storage_.native_vtable_00 = 0x00d69260;
}

NativeAliasListLengthError::NativeAliasListLengthError(
    const NativeAliasListLengthError& source) {
    copy_native_legacy_length_error_00411940(storage_, source.storage_);
}

NativeAliasListLengthError::~NativeAliasListLengthError() noexcept {
    destroy_native_legacy_logic_error_00411780(storage_);
}

namespace {
void grow_count(void* actual_owner, std::uint32_t increment, std::uint32_t bound) {
    auto& count = *reinterpret_cast<volatile std::uint32_t*>(
        static_cast<unsigned char*>(actual_owner) + 8);
    const std::uint32_t initial_count = count;
    if (bound - initial_count < increment) {
        NativeLegacySboStringStorage temporary;
        temporary.capacity_18 = 15;
        temporary.length_14 = 0;
        temporary.buffer_04.inline_bytes[0] = '\0';
        native_legacy_sbo_string_assign_counted_00408720(
            temporary, "list<T> too long", 16);
        const CompletedTemporary completed{temporary};
        throw NativeAliasListLengthError{temporary};
    }
    count = initial_count + increment;
}
} // namespace

void grow_native_alias_list_count_004ce780(void* actual_owner, std::uint32_t increment) {
    grow_count(actual_owner, increment, 0x1fffffff);
}

void grow_native_unit_registry_count_004cee30(void* actual_owner, std::uint32_t increment) {
    grow_count(actual_owner, increment, 0x3fffffff);
}

void grow_native_unit_part_shape_count_00711ed0(void* actual_owner, std::uint32_t increment) {
    grow_count(actual_owner, increment, 0x06666666);
}

} // namespace bsp

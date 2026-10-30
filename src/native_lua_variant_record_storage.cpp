#include "bsp/native_lua_variant_record_storage.hpp"

#include <climits>
#include <cstddef>
#include <new>
#include <type_traits>

namespace bsp {
namespace {
using Record = NativeLuaVariantRecordStorage20;
static_assert(CHAR_BIT == 8);
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(std::uint32_t) == 4 && alignof(std::uint32_t) == 4);
static_assert(std::is_standard_layout_v<Record>);
static_assert(std::is_trivial_v<Record>);
static_assert(std::is_trivially_default_constructible_v<Record>);
static_assert(std::is_nothrow_default_constructible_v<Record>);
static_assert(std::is_trivially_destructible_v<Record>);
static_assert(sizeof(Record) == 20 && alignof(Record) == 4);
static_assert(offsetof(Record, profile_word) == 0);
static_assert(offsetof(Record, tag_word) == 4);
static_assert(offsetof(Record, payload_word) == 8);
static_assert(offsetof(Record, name_length_word) == 12);
static_assert(offsetof(Record, name_data_word) == 16);
} // namespace

NativeLuaVariantRecordStorage20*
begin_native_lua_variant_record_lifetime_20(void* actual_storage) noexcept {
    return ::new (actual_storage) NativeLuaVariantRecordStorage20;
}

} // namespace bsp

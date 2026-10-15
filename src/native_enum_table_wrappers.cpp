#include "bsp/native_enum_table_wrappers.hpp"

#include "bsp/native_enum_dictionary_lookup.hpp"
#include "bsp/native_enum_table_insertion.hpp"
#include "bsp/native_string.hpp"

#include <cstddef>
#include <type_traits>

namespace bsp {

static_assert(sizeof(void*) == 4 && sizeof(NativeString) == 8
    && alignof(NativeString) == 4);
static_assert(std::is_standard_layout_v<NativeString>
    && std::is_nothrow_default_constructible_v<NativeString>
    && std::is_trivially_destructible_v<NativeString>,
    "A genuine local key has no implicit owning cleanup.");

void insert_native_cenum_table_name_008f2b90(void* actual_owner,
    const char* key, void* new_actual_cenum_or_null,
    NativeEnumTableWrapperContext& context) {
    NativeString temporary_key;
    auto& strings = context.strings();
    construct_native_string_header_0041e870(&temporary_key,
        strings.raw_context(), key);
    (void)insert_native_cenum_table_008f17e0(
        static_cast<std::byte*>(actual_owner) + 4, temporary_key,
        new_actual_cenum_or_null, context.table_pool(), context.symbol_pool(),
        strings.raw_context(), context.enum_empty(), strings.query_empty_00e17654());
    destroy_native_string_header_0041dd20(&temporary_key, strings.raw_context());
}

bool contains_native_enum_table_name_008f2c10(const void* actual_owner,
    const char* key, NativeEnumTableWrapperContext& context) {
    NativeString temporary_key;
    std::uint32_t bucket;
    auto& strings = context.strings();
    construct_native_string_header_0041e870(&temporary_key,
        strings.raw_context(), key);
    const void* const node = find_native_enum_table_node_0048d4e0(
        static_cast<const std::byte*>(actual_owner) + 4, temporary_key, bucket,
        context.enum_empty(), strings.query_empty_00e17654());
    const bool found = node != nullptr;
    destroy_native_string_header_0041dd20(&temporary_key, strings.raw_context());
    return found;
}

} // namespace bsp

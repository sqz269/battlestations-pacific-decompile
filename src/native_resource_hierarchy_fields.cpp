#include "bsp/native_resource_hierarchy_fields.hpp"

#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native hierarchy fields require MSVC Win32.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);

U address(const void* value) noexcept { return reinterpret_cast<U>(value); }
void* pointer(U value) noexcept { return reinterpret_cast<void*>(value); }
U word(const void* object, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(address(object) + offset);
}
void put(void* object, U offset, U value) noexcept {
    *reinterpret_cast<volatile U*>(address(object) + offset) = value;
}
std::int32_t signed_word(const void* object, U offset) noexcept {
    return static_cast<std::int32_t>(word(object, offset));
}
} // namespace

void reserve_native_hierarchy_reference_array_00b7d640(
    void* header, std::int32_t requested_capacity) {
    const auto capacity = requested_capacity < 8 ? 8 : requested_capacity;
    if (signed_word(header, 8) >= capacity) return;

    const U bytes = static_cast<U>(capacity) * 4u;
    void* const replacement = singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, bytes, bytes});
    U destination = address(replacement);
    for (U index = 0; static_cast<std::int32_t>(index) < signed_word(header, 4);
         ++index, destination += 4u) {
        if (destination != 0)
            put(pointer(destination), 0, word(pointer(word(header)), index * 4u));
    }
    singleton_lifetime_free(pointer(word(header)));
    // B7D691..699 is a returning-free continuation omitted from the old listing.
    put(header, 0, address(replacement));
    put(header, 8, static_cast<U>(capacity));
}

void resize_native_hierarchy_reference_array_00b7d7d0(
    void* header, std::int32_t requested_count) {
    if (requested_count > signed_word(header, 8))
        reserve_native_hierarchy_reference_array_00b7d640(header, requested_count);
    for (U index = word(header, 4);
         static_cast<std::int32_t>(index) < requested_count; ++index) {
        const U destination = word(header) + index * 4u;
        if (destination != 0) put(pointer(destination), 0, 0);
    }
    while (requested_count < signed_word(header, 4))
        put(header, 4, word(header, 4) - 1u);
    put(header, 4, static_cast<U>(requested_count));
}

void destroy_native_hierarchy_fields_00b88180(
    void* record, NativeStringRawPoolContext& strings) {
    void* const array = pointer(address(record) + 0x4cu);
    bool name_cleanup_armed = true; // native state0 before B881AD
    try {
        resize_native_hierarchy_reference_array_00b7d7d0(array, 0);
        singleton_lifetime_free(pointer(word(array)));

        // B881BA reads current name data AFTER array free and BEFORE disarming.
        void* const name_data = pointer(word(record, 8));
        name_cleanup_armed = false; // B881C2: state-1 before normal name return
        if (name_data != nullptr) {
            const U bytes = word(record, 4) + 1u;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                strings.actual_published_01090aa8,
                strings.actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(pool, name_data, bytes,
                strings.actual_small_returns_disabled_01090aa4);
        }
    } catch (...) {
        // DFB9C0 state0 -> CC2570: current name header only, never array/slot.
        if (name_cleanup_armed)
            destroy_native_string_header_0041dd20(
                pointer(address(record) + 4u), strings);
        throw;
    }
}
} // namespace bsp

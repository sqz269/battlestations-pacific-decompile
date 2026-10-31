#include "bsp/native_lua_variant_string_setter.hpp"

#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_string_raw_resize_exact.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <climits>
#include <cstddef>
#include <cstring>
#include <intrin.h>
#include <new>
#include <type_traits>

namespace bsp {
namespace {

static_assert(CHAR_BIT == 8 && sizeof(std::uint32_t) == 4);
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);

struct Header8 {
    std::uint32_t length;
    std::uint32_t data;
};
static_assert(std::is_standard_layout_v<Header8>);
static_assert(std::is_trivially_default_constructible_v<Header8>);
static_assert(std::is_nothrow_default_constructible_v<Header8>);
static_assert(std::is_trivially_destructible_v<Header8>);
static_assert(sizeof(Header8) == 8 && alignof(Header8) == 4);
static_assert(offsetof(Header8, length) == 0 && offsetof(Header8, data) == 4);

std::uint32_t address_bits(const void* pointer, std::uint32_t offset = 0) noexcept {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(pointer))
        + offset;
}

void* pointer_from_bits(std::uint32_t bits) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}

std::uint32_t read_word(const void* base, std::uint32_t offset = 0) noexcept {
    std::uint32_t value;
    _ReadWriteBarrier();
    std::memcpy(&value, pointer_from_bits(address_bits(base, offset)), sizeof value);
    _ReadWriteBarrier();
    return value;
}

void write_word(void* base, std::uint32_t offset, std::uint32_t value) noexcept {
    _ReadWriteBarrier();
    std::memcpy(pointer_from_bits(address_bits(base, offset)), &value, sizeof value);
    _ReadWriteBarrier();
}

} // namespace

NativeLuaVariantRetirementResult
set_native_lua_variant_string_00886a10_cpp_cleanup(
    void* actual_variant, const void* actual_source_pointer_home,
    NativeStringRawPoolContext& strings,
    NativeLuaVariantRetirementOperation& operation,
    const NativeLuaVariantCppCleanupBacking& cleanup,
    const volatile std::uint32_t& actual_feature_word_0109eea4) {
    void* const captured_variant = actual_variant;
    const void* const captured_source_home = actual_source_pointer_home;
    const NativeLuaVariantRetirementResult cleared =
        clear_native_lua_variant_00886920_cpp_cleanup(
            captured_variant, strings, operation, cleanup);
    if (cleared != NativeLuaVariantRetirementResult::completed) {
        return cleared;
    }

    write_word(captured_variant, 4u, 1u);
    void* const allocation = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 8u, 8u});
    if (allocation == nullptr) {
        write_word(captured_variant, 8u, 0u);
        return NativeLuaVariantRetirementResult::completed;
    }

    try {
        const void* const source = pointer_from_bits(read_word(captured_source_home));
        const bool same_header = allocation == source;
        Header8* const header = ::new (allocation) Header8;
        write_word(header, 0u, 0u);
        write_word(header, 4u, 0u);
        if (!same_header) {
            const std::uint32_t source_length = read_word(source);
            std::uint8_t preserve_home = 1u;
            resize_native_string_header_0041dd40_exact(
                header, strings, source_length, &preserve_home,
                actual_feature_word_0109eea4);
            if (read_word(source) != 0u) {
                const std::uint32_t count = read_word(header);
                const void* const source_data = pointer_from_bits(read_word(source, 4u));
                void* const destination_data = pointer_from_bits(read_word(header, 4u));
                copy_native_crt_bytes_00bf7680(destination_data, source_data, count,
                    actual_feature_word_0109eea4);
            }
        }
    } catch (...) {
        singleton_lifetime_free(allocation);
        throw;
    }

    write_word(captured_variant, 8u, address_bits(allocation));
    return NativeLuaVariantRetirementResult::completed;
}

} // namespace bsp

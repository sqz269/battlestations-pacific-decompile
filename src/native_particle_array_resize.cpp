#include "bsp/native_particle_array_resize.hpp"
#include "bsp/native_particle_model_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <limits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle array resize requires MSVC Win32.
#endif

namespace bsp {
namespace {
std::uint32_t allocation_size(std::int32_t count, std::uint32_t stride) noexcept {
    // Native unsigned MUL overflow -> FFFFFFFF, followed by ADD4 saturation.
    const auto full = static_cast<std::uint64_t>(static_cast<std::uint32_t>(count)) * stride + 4u;
    constexpr auto maximum = (std::numeric_limits<std::uint32_t>::max)();
    return full > maximum ? maximum : static_cast<std::uint32_t>(full);
}
std::byte* allocate_cookie(std::int32_t count, std::uint32_t stride) {
    const auto bytes = allocation_size(count, stride);
    return static_cast<std::byte*>(singleton_lifetime_allocate(
        {SingletonAllocationKind::object, bytes, bytes}));
}
} // namespace

void* construct_native_particle_array_byte_00afcd90(void* element) noexcept {
    *static_cast<std::byte*>(element) = std::byte{0};
    return element;
}
void* construct_native_particle_record_00afdac0(
    void* element, const volatile std::uint32_t& one_00d7a24c) noexcept {
    const auto one_bits = one_00d7a24c; // AFDAC0 MOVSS: copy, no FP conversion.
    auto* bytes = static_cast<std::byte*>(element);
    const std::uint32_t zero = 0;
    std::memcpy(bytes + 0xa4, &zero, sizeof(zero));
    std::memcpy(bytes + 0xa0, &zero, sizeof(zero));
    std::memcpy(bytes + 0xb4, &one_bits, sizeof(one_bits));
    return element;
}
void resize_native_particle_model_byte_array_00afd130(
    NativeParticleArrayStorage& array, std::int32_t count) {
    // Native frees its captured pointer without resetting this header. Apply
    // the canonical helper to a local header so its reset stays local too.
    NativeParticleArrayStorage captured{array.data_00, 0};
    destroy_native_particle_model_byte_array_00afd0f0(captured);
    auto* cookie = allocate_cookie(count, 1);
    void* data = nullptr;
    if (cookie) {
        std::memcpy(cookie, &count, sizeof(count));
        auto* elements = cookie + 4;
        // BF7CD1 compares its signed iteration index to the signed count.
        for (std::int32_t i = 0; i < count; ++i)
            construct_native_particle_array_byte_00afcd90(elements + i);
        data = elements;
    }
    // The actual leaf constructors cannot throw for valid storage, so their
    // native partial-construction cleanup has no C++ exceptional path here.
    array.data_00 = data;
    array.count_04 = count;
}
void resize_native_particle_model_record_array_00afd220(
    NativeParticleArrayStorage& array, std::int32_t count,
    const volatile std::uint32_t& one_00d7a24c) {
    NativeParticleArrayStorage captured{array.data_00, 0};
    destroy_native_particle_model_record_array_00afd1e0(captured);
    auto* cookie = allocate_cookie(count, 0x108);
    void* data = nullptr;
    if (cookie) {
        std::memcpy(cookie, &count, sizeof(count));
        auto* elements = cookie + 4;
        for (std::int32_t i = 0; i < count; ++i)
            construct_native_particle_record_00afdac0(
                elements + static_cast<std::size_t>(i) * 0x108, one_00d7a24c);
        data = elements;
    }
    array.data_00 = data;
    array.count_04 = count;
}
} // namespace bsp

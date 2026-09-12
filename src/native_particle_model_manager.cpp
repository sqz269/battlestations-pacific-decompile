#include "bsp/native_particle_model_manager.hpp"
#include "bsp/native_renderer_worker_lifetime.hpp"
#include "bsp/native_tracked_critical_section_release.hpp"
#include <cstring>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle manager storage requires MSVC Win32.
#endif
namespace bsp {
namespace {
static_assert(sizeof(NativeParticleManagerPointerArray) == 0x0c);
static_assert(sizeof(NativeParticleModelManagerStorage) == 0x34);
static_assert(offsetof(NativeParticleModelManagerStorage, models_04) == 4);
static_assert(offsetof(NativeParticleModelManagerStorage, entries_10) == 0x10);
static_assert(offsetof(NativeParticleModelManagerStorage, untouched_1c) == 0x1c);
static_assert(offsetof(NativeParticleModelManagerStorage, scalar_bits_2c) == 0x2c);
static_assert(offsetof(NativeParticleModelManagerStorage, section_30) == 0x30);
std::int32_t signed_bits(std::uint32_t value) noexcept {
    std::int32_t result;
    std::memcpy(&result, &value, 4);
    return result;
}
std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uint32_t>(value);
}
void* read_pointer(std::uint32_t at) noexcept {
    void* result;
    std::memcpy(&result, reinterpret_cast<const void*>(at), 4);
    return result;
}
void write_pointer(std::uint32_t at, void* value) noexcept {
    std::memcpy(reinterpret_cast<void*>(at), &value, 4);
}
std::int32_t next_capacity(std::uint32_t current) noexcept {
    const auto doubled = signed_bits(current + current);
    return doubled > 1 ? doubled : 1;
}
class CapturedSection final {
public:
    explicit CapturedSection(SystemSingletonCriticalSection* value) : value_(value) {
        if (value_) {
            singleton_enter_critical_section(*value_);
            ++value_->recursion_18;
        }
    }
    ~CapturedSection() {
        if (value_) {
            --value_->recursion_18;
            singleton_leave_critical_section(*value_);
        }
    }
    CapturedSection(const CapturedSection&) = delete;
    CapturedSection& operator=(const CapturedSection&) = delete;
private:
    SystemSingletonCriticalSection* value_;
};
}

void reserve_native_particle_manager_pointers_00af0630(NativeParticleManagerPointerArray& array,
    std::int32_t requested) {
    if (requested < 1) requested = 1;
    if (signed_bits(array.capacity_08) >= requested) return;
    const auto bytes = static_cast<std::uint32_t>(requested) * 4u;
    // BF55BE is an exact JMP BF681B. Use the existing real CRT/new-handler
    // service, preserving wrapped native allocation size (not a widened size).
    auto* const replacement = static_cast<void**>(singleton_lifetime_allocate(
        {SingletonAllocationKind::pointer_slots, bytes, bytes}));
    std::uint32_t index = 0;
    auto destination = address(replacement);
    while (signed_bits(index) < signed_bits(array.count_04)) {
        if (destination) write_pointer(destination, read_pointer(address(array.data_00) + index * 4u));
        ++index;
        destination += 4u;
    }
    singleton_lifetime_free(array.data_00);
    array.data_00 = replacement;
    array.capacity_08 = static_cast<std::uint32_t>(requested);
}
void append_native_particle_manager_pointer_00af07e0(NativeParticleManagerPointerArray& array,
    void* const* source) {
    const auto capacity = array.capacity_08;
    if (array.count_04 == capacity)
        reserve_native_particle_manager_pointers_00af0630(array, next_capacity(capacity));
    const auto destination = address(array.data_00) + array.count_04 * 4u;
    if (destination) write_pointer(destination, *source); // source is read AFTER growth.
    array.count_04 = array.count_04 + 1u;
}
void resize_native_particle_manager_pointers_00af0900(NativeParticleManagerPointerArray& array,
    std::int32_t requested) {
    if (requested > signed_bits(array.capacity_08))
        reserve_native_particle_manager_pointers_00af0630(array, requested);
    auto index = array.count_04;
    while (signed_bits(index) < requested) {
        const auto destination = address(array.data_00) + index * 4u;
        if (destination) write_pointer(destination, nullptr);
        ++index;
    }
    while (requested < signed_bits(array.count_04)) array.count_04 = array.count_04 - 1u;
    array.count_04 = static_cast<std::uint32_t>(requested);
}
std::uint8_t remove_native_particle_manager_pointer_00af0a60(
    NativeParticleManagerPointerArray& array, void* const* source) noexcept {
    const auto base = address(array.data_00);
    const auto count = array.count_04;
    const auto end = base + count * 4u;
    auto cursor = base;
    if (cursor >= end) return 0; // native unsigned pointer comparison.
    void* const wanted = *source;
    do {
        if (read_pointer(cursor) == wanted) {
            const auto index = signed_bits(cursor - base) >> 2;
            if (index == -1) return 0;
            if (static_cast<std::uint32_t>(index) != count - 1u)
                write_pointer(base + static_cast<std::uint32_t>(index) * 4u,
                    read_pointer(base + count * 4u - 4u));
            array.count_04 = array.count_04 - 1u;
            return 1;
        }
        cursor += 4u;
    } while (cursor < end);
    return 0;
}
void destroy_native_particle_manager_pointers_00af0af0(NativeParticleManagerPointerArray& array) noexcept {
    resize_native_particle_manager_pointers_00af0900(array, 0);
    singleton_lifetime_free(array.data_00);
}
void register_native_particle_model_00af0950(void* actual_manager, void* actual_model) {
    auto& array = static_cast<NativeParticleModelManagerStorage*>(actual_manager)->models_04;
    const auto capacity = array.capacity_08;
    if (array.count_04 == capacity)
        reserve_native_particle_manager_pointers_00af0630(array, next_capacity(capacity));
    const auto destination = address(array.data_00) + array.count_04 * 4u;
    if (destination) write_pointer(destination, actual_model);
    array.count_04 = array.count_04 + 1u;
}
std::uint8_t unregister_native_particle_model_00af0ae0(void* actual_manager, void* actual_model) noexcept {
    return remove_native_particle_manager_pointer_00af0a60(
        static_cast<NativeParticleModelManagerStorage*>(actual_manager)->models_04, &actual_model);
}
NativeParticleModelManagerStorage* construct_native_particle_manager_base_00af06a0(
    NativeParticleModelManagerStorage* owner, NativeParticleModelManagerAccess& access) {
    owner->native_table_00 = 0x00d5d7ecu;
    try {
        CapturedSection lock(access.lifetime_01090aa0.get_manager_00415350()->system_owner().section_10);
        access.manager_00f8c274 = owner;
        auto* const manager = access.lifetime_01090aa0.get_manager_00415350();
        manager->register_object(access.manager_00f8c274);
    } catch (...) {
        owner->native_table_00 = 0x00ce3818u; // state0 / 00412430; publication survives.
        throw;
    }
    return owner;
}
void destroy_native_particle_manager_base_00af0740(NativeParticleModelManagerStorage* owner,
    NativeParticleModelManagerAccess& access) {
    owner->native_table_00 = 0x00d5d7ecu;
    try {
        CapturedSection lock(access.lifetime_01090aa0.get_manager_00415350()->system_owner().section_10);
        auto* const manager = access.lifetime_01090aa0.get_manager_00415350();
        manager->unregister_object(access.manager_00f8c274);
        access.manager_00f8c274 = nullptr;
    } catch (...) {
        owner->native_table_00 = 0x00ce3818u;
        throw;
    }
    owner->native_table_00 = 0x00ce3818u;
}
NativeParticleModelManagerStorage* construct_native_particle_model_manager_00af0b10(
    NativeParticleModelManagerStorage* owner, NativeParticleModelManagerAccess& access) {
    construct_native_particle_manager_base_00af06a0(owner, access);
    owner->native_table_00 = 0x00d5d7f8u;
    owner->models_04.data_00 = nullptr;
    owner->models_04.count_04 = 0;
    owner->models_04.capacity_08 = 0;
    owner->entries_10.data_00 = nullptr;
    owner->entries_10.count_04 = 0;
    owner->entries_10.capacity_08 = 0;
    const auto one = access.one_00d7a24c;
    owner->word_24 = 0;
    owner->word_28 = 0;
    owner->scalar_bits_2c = one;
    try {
        owner->section_30 = create_native_tracked_critical_section_00bd1860();
    } catch (...) {
        try {
            destroy_native_particle_manager_pointers_00af0af0(owner->entries_10);
            destroy_native_particle_manager_pointers_00af0af0(owner->models_04);
            destroy_native_particle_manager_base_00af0740(owner, access);
        } catch (...) { std::terminate(); }
        throw;
    }
    return owner;
}
void destroy_native_particle_model_manager_00af0b90(NativeParticleModelManagerStorage* owner,
    NativeParticleModelManagerAccess& access) {
    owner->native_table_00 = 0x00d5d7f8u;
    // The canonical release helper clears ITS passed slot. AF0B90 leaves the
    // published +30 word stale, so pass only the captured local section slot.
    auto* captured_section = owner->section_30;
    release_native_tracked_critical_section_0041cc80(&captured_section);
    destroy_native_particle_manager_pointers_00af0af0(owner->entries_10);
    destroy_native_particle_manager_pointers_00af0af0(owner->models_04);
    destroy_native_particle_manager_base_00af0740(owner, access);
}
NativeParticleModelManagerStorage* delete_native_particle_manager_base_00af0870(
    NativeParticleModelManagerStorage* owner, std::uint32_t flags, NativeParticleModelManagerAccess& access) {
    destroy_native_particle_manager_base_00af0740(owner, access);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
NativeParticleModelManagerStorage* delete_native_particle_model_manager_00af1080(
    NativeParticleModelManagerStorage* owner, std::uint32_t flags, NativeParticleModelManagerAccess& access) {
    destroy_native_particle_model_manager_00af0b90(owner, access);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp

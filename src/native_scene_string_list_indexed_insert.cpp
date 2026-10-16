#include "bsp/native_scene_string_list_indexed_insert.hpp"

#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_scene_string_list_storage.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);

// Preserve measured raw-field access/publication order in the mandatory
// coherent, serialized, nonfaulting domain. These add no synchronization.
template<class T>
T read_field(const void* base, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile T*>(
        static_cast<const unsigned char*>(base) + offset);
}

template<class T>
void write_field(void* base, std::size_t offset, T value) noexcept {
    *reinterpret_cast<volatile T*>(
        static_cast<unsigned char*>(base) + offset) = value;
}

std::int32_t shift_and_publish(void* actual_list12, void* captured_record8,
    std::int32_t native_index) noexcept {
    std::int32_t index = native_index;
    if (index < 0) {
        index = 0;
    } else {
        const auto current_count = read_field<std::int32_t>(actual_list12, 4);
        if (index > current_count) index = current_count;
    }

    auto cursor = read_field<std::int32_t>(actual_list12, 4);
    while (cursor > index) {
        void* const slots = read_field<void*>(actual_list12, 0);
        void* const captured_record = read_field<void*>(slots,
            static_cast<std::uint32_t>(cursor - 1) * 4u);
        void* const captured_destination = static_cast<unsigned char*>(slots)
            + static_cast<std::uint32_t>(cursor) * 4u;
        --cursor;
        write_field<void*>(captured_destination, 0, captured_record);
    }
    void* const slots = read_field<void*>(actual_list12, 0);
    write_field<void*>(slots, static_cast<std::uint32_t>(index) * 4u,
        captured_record8);
    write_field<std::uint32_t>(actual_list12, 4,
        read_field<std::uint32_t>(actual_list12, 4) + 1u);
    return index;
}

// Current-C++ header-only ownership policy, independent of Original private EH.
class FreshHeaderGuard final {
public:
    explicit FreshHeaderGuard(void* fresh_header8) noexcept : header_(fresh_header8) {}
    ~FreshHeaderGuard() noexcept {
        if (header_ != nullptr) singleton_lifetime_free(header_);
    }
    FreshHeaderGuard(const FreshHeaderGuard&) = delete;
    FreshHeaderGuard& operator=(const FreshHeaderGuard&) = delete;
    void disarm() noexcept { header_ = nullptr; }
private:
    void* header_;
};

} // namespace

std::int32_t insert_native_scene_string_list_indexed_0094f2f0(
    void* actual_list12, void* actual_record8, std::int32_t native_index,
    std::uint32_t native_flags, NativeStringRawPoolContext& raw) {
    const auto count = read_field<std::uint32_t>(actual_list12, 4);
    const auto capacity = read_field<std::uint32_t>(actual_list12, 8);
    if (count == capacity)
        grow_native_scene_string_list_storage_0094f010(actual_list12);

    if ((native_flags & 0xffu) == 0) {
        void* const fresh_header = singleton_lifetime_allocate(
            {SingletonAllocationKind::object, 8, 8});
        FreshHeaderGuard guard(fresh_header);
        void* const copied_record = copy_construct_native_string_header_00426060(
            fresh_header, actual_record8, raw);
        const auto result = shift_and_publish(actual_list12, copied_record, native_index);
        guard.disarm();
        return result;
    }
    return shift_and_publish(actual_list12, actual_record8, native_index);
}

} // namespace bsp

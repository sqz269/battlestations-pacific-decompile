#include "bsp/native_scene_string_list_ownership.hpp"

#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_scene_string_list_storage.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);

// Volatile raw-field accesses preserve the measured reads/publications across
// reached providers. Alignment, live storage and disjoint ownership are caller
// preconditions; these helpers add no synchronization or object wrapper.
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

std::uint32_t publish_record(void* actual_list12, void* captured_record8) noexcept {
    const auto index = read_field<std::uint32_t>(actual_list12, 4);
    void* const slots = read_field<void*>(actual_list12, 0);
    write_field<void*>(slots, index * 4u, captured_record8);
    write_field<std::uint32_t>(actual_list12, 4,
        read_field<std::uint32_t>(actual_list12, 4) + 1u);
    return read_field<std::uint32_t>(actual_list12, 4);
}

// Root-selected current-C++ policy, separate from Original private FH3.
// Owns only the actual freshly allocated raw8 header, never its data or any
// adopted header, slot allocation or list12. No original handler is called.
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

std::uint32_t append_native_scene_string_list_0094f270(void* actual_list12,
    void* actual_record8, std::uint32_t native_flags,
    NativeStringRawPoolContext& raw) {
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
        const auto result = publish_record(actual_list12, copied_record);
        guard.disarm();
        return result;
    }
    return publish_record(actual_list12, actual_record8);
}

void clear_native_scene_string_list_0094f3e0(void* actual_list12,
    NativeStringRawPoolContext& raw) {
    std::uint32_t index = 0;
    if (read_field<std::int32_t>(actual_list12, 4) > 0) {
        do {
            void* const slots = read_field<void*>(actual_list12, 0);
            void* const captured_slot = static_cast<unsigned char*>(slots) + index * 4u;
            void* const captured_record = read_field<void*>(captured_slot, 0);
            if (captured_record != nullptr) {
                destroy_native_string_header_0041dd20(captured_record, raw);
                singleton_lifetime_free(captured_record);
                write_field<void*>(captured_slot, 0, nullptr);
            }
            ++index;
        } while (static_cast<std::int32_t>(index) <
            read_field<std::int32_t>(actual_list12, 4));
    }
    write_field<std::uint32_t>(actual_list12, 4, 0);
}

void retire_native_scene_string_list_0094f540(void* actual_list12,
    NativeStringRawPoolContext& raw) {
    clear_native_scene_string_list_0094f3e0(actual_list12, raw);
    void* const slots = read_field<void*>(actual_list12, 0);
    if (slots != nullptr) {
        singleton_lifetime_free(slots);
        write_field<void*>(actual_list12, 0, nullptr);
    }
}

} // namespace bsp

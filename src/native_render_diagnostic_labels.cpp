#include "bsp/native_render_diagnostic_labels.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeString) == 8);
static_assert(std::is_trivially_destructible_v<NativeString>);

void* label_header(void* actual_service) noexcept {
    return reinterpret_cast<void*>(
        reinterpret_cast<std::uintptr_t>(actual_service) + 0x684u);
}

template<class T> T read_header(const void* header, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const char*>(header) + offset, sizeof(value));
    return value;
}

constexpr char reset_label[] = "X"; // Actual CE9A38 bytes 58 00.

} // namespace

void set_native_render_diagnostic_label_00b13030(void* actual_service,
    const void* actual_source_header, NativeStringStorage& storage) {
    void* const destination = label_header(actual_service);
    if (destination == actual_source_header) return;

    const auto requested = read_header<std::uint32_t>(actual_source_header, 0);
    resize_native_string_header_0041dd40(destination, storage, requested, true);
    if (read_header<std::uint32_t>(actual_source_header, 0) != 0) {
        const auto copied = read_header<std::uint32_t>(destination, 0);
        const auto* const source = read_header<char*>(actual_source_header, 4);
        auto* const target = read_header<char*>(destination, 4);
        // As with the existing actual-string helpers, omit zero-byte memcpy.
        if (copied != 0) std::memcpy(target, source, copied);
    }
}

void reset_native_render_diagnostic_label_00b13510(
    void* const volatile& actual_global_00f8d39c, NativeStringStorage& storage) {
    if (actual_global_00f8d39c == nullptr) return;

    NativeString temporary;
    temporary.assign_0041e870(storage, reset_label); // Before native state0.

    void* const current_service = actual_global_00f8d39c;
    auto* const captured_data = read_header<char*>(&temporary, 4);
    const auto captured_length = read_header<std::uint32_t>(&temporary, 0);
    void* const destination = label_header(current_service);
    const bool same_header = destination == &temporary;

    try { // B1355C arms only after construction, capture and alias comparison.
        if (!same_header) {
            resize_native_string_header_0041dd40(
                destination, storage, captured_length, true);
            if (captured_length != 0) {
                const auto copied = read_header<std::uint32_t>(destination, 0);
                auto* const target = read_header<char*>(destination, 4);
                if (copied != 0) std::memcpy(target, captured_data, copied);
            }
        }
    } catch (...) {
        // CBC270 uses the current header, not the earlier normal-path capture.
        destroy_native_string_header_0041dd20(&temporary, storage);
        throw;
    }

    // B13586 disarms before the native sized release. No header clearing.
    if (captured_data != nullptr)
        storage.release(captured_data, captured_length + 1u);
}

} // namespace bsp

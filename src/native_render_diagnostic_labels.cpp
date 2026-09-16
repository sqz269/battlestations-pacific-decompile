#include "bsp/native_render_diagnostic_labels.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <exception>

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

template<class T> T read_raw_header(const void* header, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile T*>(
        static_cast<const char*>(header)+offset);
}

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

void set_native_render_diagnostic_label_00b13030(void* actual_service,
    const void* actual_source_header, NativeStringRawPoolContext& context) {
    void* const destination=label_header(actual_service);
    if(destination==actual_source_header) return;
    const auto requested=read_raw_header<std::uint32_t>(actual_source_header,0);
    resize_native_string_header_0041dd40(destination,context,requested,true);
    if(read_raw_header<std::uint32_t>(actual_source_header,0)!=0) {
        const auto copied=read_raw_header<std::uint32_t>(destination,0);
        const auto* const source=read_raw_header<char*>(actual_source_header,4);
        auto* const target=read_raw_header<char*>(destination,4);
        // Original BF7680 supports overlap; retain all argument captures even
        // when omitting the zero-byte library operation.
        if(copied!=0) std::memmove(target,source,copied);
    }
}

void reset_native_render_diagnostic_label_00b13510(
    void* const volatile& actual_global_00f8d39c, NativeStringRawPoolContext& context) {
    if(actual_global_00f8d39c==nullptr) return;
    std::uint32_t temporary[2]; // original callee initializes both fields
    construct_native_string_header_0041e870(temporary,context,reset_label);
    void* const current_service=actual_global_00f8d39c;
    auto* const captured_data=read_raw_header<char*>(temporary,4);
    const auto captured_length=read_raw_header<std::uint32_t>(temporary,0);
    void* const destination=label_header(current_service);
    const bool same_header=destination==temporary;
    try {
        if(!same_header) {
            resize_native_string_header_0041dd40(destination,context,captured_length,true);
            if(captured_length!=0) {
                const auto copied=read_raw_header<std::uint32_t>(destination,0);
                auto* const target=read_raw_header<char*>(destination,4);
                if(copied!=0) std::memmove(target,captured_data,copied);
            }
        }
    } catch(...) {
        try { destroy_native_string_header_0041dd20(temporary,context); }
        catch(...) { std::terminate(); }
        throw;
    }
    if(captured_data) {
        const auto size=captured_length+1u;
        auto* const pool=native_string_pool_get_or_create_00419cc0(
            context.actual_published_01090aa8,context.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool,captured_data,size,
            context.actual_small_returns_disabled_01090aa4);
    }
}
} // namespace bsp

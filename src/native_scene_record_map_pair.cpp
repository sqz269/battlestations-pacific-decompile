#include "bsp/native_scene_record_map_pair.hpp"

#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstddef>
#include <cstring>

namespace bsp {
namespace {

static_assert(sizeof(std::uintptr_t) == 4);

template<class T> T read_field(const void* storage, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const char*>(storage) + offset, sizeof(value));
    return value;
}
template<class T> void write_field(void* storage, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<char*>(storage) + offset, &value, sizeof(value));
}

void* copy_pair(void* destination, const void* source,
    NativeStringRawPoolContext& context) {
    copy_construct_native_string_header_00426060(destination, source, context);
    const auto mapped = read_field<std::uint32_t>(source, 8);
    write_field<std::uint32_t>(destination, 8, mapped);
    return destination;
}

// Private Source scopes, not independently callable native EBP-frame entries.
void unwind_scene_map_pair_key_00c61be0(void* actual_parameter,
    NativeStringRawPoolContext& context) {
    destroy_native_string_header_0041dd20(actual_parameter, context);
}
void unwind_scene_map_pair_output_00c61be8(bool& constructed,
    void* actual_output, NativeStringRawPoolContext& context) {
    if (constructed) {
        constructed = false; // Native clears bit0 BEFORE the destructor.
        // Full 00468B30 is instruction-identical to this canonical destructor.
        destroy_native_string_header_0041dd20(actual_output, context);
    }
}

struct PairConstructionUnwind {
    void* actual_parameter;
    void* actual_output;
    NativeStringRawPoolContext& context;
    bool parameter_armed = true;
    bool output_constructed = false;
    bool complete = false;

    ~PairConstructionUnwind() noexcept {
        if (!complete) {
            if (parameter_armed)
                unwind_scene_map_pair_key_00c61be0(actual_parameter, context);
            unwind_scene_map_pair_output_00c61be8(output_constructed,
                actual_output, context);
        }
    }
};

} // namespace

void* copy_construct_native_scene_record_map_pair_00469980(void* actual_destination,
    const void* actual_source, NativeStringRawPoolContext& context) {
    return copy_pair(actual_destination, actual_source, context);
}

void* copy_construct_native_scene_record_map_pair_00469a20(void* actual_destination,
    const void* actual_source, NativeStringRawPoolContext& context) {
    return copy_pair(actual_destination, actual_source, context);
}

void* construct_native_scene_record_map_pair_00469fc0(void* actual_destination,
    std::uint32_t mapped_word, void* actual_consumed_key_header,
    NativeStringRawPoolContext& context) {
    // EDI at 00469FEA, EBP at 00469FEE: capture data FIRST, then length.
    auto* const captured_data = read_field<char*>(actual_consumed_key_header, 4);
    const auto captured_length = read_field<std::uint32_t>(actual_consumed_key_header, 0);
    write_field<std::uint32_t>(actual_destination, 0, 0);
    write_field<char*>(actual_destination, 4, nullptr);
    const bool identical = actual_destination == actual_consumed_key_header;
    PairConstructionUnwind unwind{actual_consumed_key_header, actual_destination, context};

    if (!identical) {
        resize_native_string_header_0041dd40(actual_destination, context, captured_length, true);
        if (captured_length != 0) {
            const auto copied = read_field<std::uint32_t>(actual_destination, 0);
            auto* const output = read_field<char*>(actual_destination, 4);
            // BF7680 handles overlap; omit a zero-count host call only after
            // the same current destination fields have been read.
            if (copied != 0) std::memmove(output, captured_data, copied);
        }
    }
    write_field<std::uint32_t>(actual_destination, 8, mapped_word);
    unwind.output_constructed = true; // 0046A028 output completion flag.
    unwind.parameter_armed = false; // 0046A030: state0 BEFORE normal return.

    if (captured_data != nullptr) {
        const auto size = captured_length + 1u;
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            context.actual_published_01090aa8, context.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, captured_data, size,
            context.actual_small_returns_disabled_01090aa4);
    }
    unwind.complete = true;
    return actual_destination;
}

} // namespace bsp

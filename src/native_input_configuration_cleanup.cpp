#include "bsp/native_input_configuration_cleanup.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
using Word = std::uint32_t;
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
template<class T = Word> volatile T& field(Word p, Word offset = 0) noexcept {
    return *reinterpret_cast<volatile T*>(p + offset);
}
void clear_flat_vector(Word header, NativeInputConfigurationStorageCalls& storage) {
    const auto last = field(header, 8);
    if (field(header, 4) > last) _invalid_parameter_noinfo();
    const auto first = field(header, 4);
    if (first > field(header, 8)) _invalid_parameter_noinfo();
    Word iterator[2]; // actual8h output, not a second container
    storage.call_006977f0(pointer(header), iterator,
        pointer(header), pointer(first), pointer(header), pointer(last));
}
} // namespace

std::uint32_t query_native_input_action_registered_00a92260(
    const void* actual_owner, std::uint32_t action_index) noexcept {
    const auto owner = address(actual_owner);
    if (action_index >= field(owner, 8)) return 0;
    const auto base = field(owner, 4);
    return field<std::uint8_t>(base + action_index * 0x30u) ? 1u : 0u;
}

void clear_native_input_action_registration_00a93880(void* actual_action,
    NativeInputActionRecordsContext& context) {
    const auto action = address(actual_action);
    resize_native_input_context_words_00696e70(pointer(action + 4u), 0);
    resize_native_input_bindings_00a93500(pointer(action + 0x10u), 0, context.bindings);
    // Native TEST(-!!pointer,F8BC00) does not read the F8BC00 global.
    if (field(action, 0x2c)) {
        const auto listener = field(action, 0x2c); // distinct native reload
        if (listener) {
            if (InterlockedDecrement(&field<LONG>(listener, 4)) == 0)
                context.listeners.call_listener_slot0(pointer(listener), field(listener));
            field(action, 0x2c) = 0;
        }
        field(action, 0x2c) = 0;
    }
    field<std::uint8_t>(action) = 0;
}

void unregister_native_input_action_00a93920(void* actual_owner,
    std::uint32_t action_index, NativeInputActionRecordsContext& context) {
    const auto action = field(address(actual_owner), 4) + action_index * 0x30u;
    clear_native_input_action_registration_00a93880(pointer(action), context);
}

void clear_native_input_configuration_00698730(void* actual_configuration,
    NativeInputConfigurationCleanupContext& context) {
    for (Word index = 0; index <= 0x128u; ++index) {
        const auto owner = get_native_input_action_owner_004bec00(context.action_owner);
        if (query_native_input_action_registered_00a92260(owner, index)) {
            const auto current_owner = get_native_input_action_owner_004bec00(context.action_owner);
            unregister_native_input_action_00a93920(current_owner, index, context.records);
        }
    }
    const auto configuration = address(actual_configuration);
    auto source = field(configuration, 0x4d8);
    if (field(configuration, 0x4d4) > source) _invalid_parameter_noinfo();
    const auto first = field(configuration, 0x4d4);
    if (first > field(configuration, 0x4d8)) _invalid_parameter_noinfo();
    if (first != source) {
        const auto source_end = field(configuration, 0x4d8);
        const auto retained_count = static_cast<Word>(
            static_cast<std::int32_t>(source_end - source) >> 4);
        const auto resulting_end = first + retained_count * 0x10u;
        if (source != source_end) {
            const auto delta = first - source;
            do {
                context.storage.call_00697bd0(pointer(delta + source), pointer(source));
                source += 0x10u;
            } while (source != source_end);
        }
        const auto current_end = field(configuration, 0x4d8);
        if (resulting_end != current_end) {
            auto row = resulting_end;
            do {
                const auto allocation = field(row, 4);
                if (allocation) singleton_lifetime_free(pointer(allocation));
                field(row, 4) = 0;
                field(row, 8) = 0;
                field(row, 0xc) = 0;
                row += 0x10u;
            } while (row != current_end);
        }
        field(configuration, 0x4d8) = resulting_end;
    }
    clear_flat_vector(configuration + 0x500u, context.storage);
    clear_flat_vector(configuration + 0x510u, context.storage);
    clear_flat_vector(configuration + 0x4e0u, context.storage);
    clear_flat_vector(configuration + 0x4f0u, context.storage);
}
} // namespace bsp

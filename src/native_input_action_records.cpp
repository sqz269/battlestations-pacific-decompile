#include "bsp/native_input_action_records.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <Windows.h>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native input action records require MSVC Win32.
#endif

namespace bsp {
namespace {
template<class T> T get(const void* p, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const unsigned char*>(p) + offset, sizeof value);
    return value;
}
template<class T> void put(void* p, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<unsigned char*>(p) + offset, &value, sizeof value);
}
void* offset(void* p, std::uint32_t bytes) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + bytes);
}
const void* offset(const void* p, std::uint32_t bytes) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(p) + bytes);
}
void zero_header(void* p) noexcept {
    put<void*>(p, 0, nullptr); put<std::int32_t>(p, 4, 0); put<std::int32_t>(p, 8, 0);
}
void copy_float(void* destination, const void* source) noexcept {
    __asm { mov eax, destination }
    __asm { mov edx, source }
    __asm { fld dword ptr [edx] }
    __asm { fstp dword ptr [eax] }
}
struct WordCleanup {
    void* header;
    bool armed{true};
    ~WordCleanup() noexcept {
        if (armed) {
            resize_native_input_context_words_00696e70(header, 0);
            singleton_lifetime_free(get<void*>(header, 0));
        }
    }
};
struct BindingCleanup {
    void* header;
    const NativeInputBindingStorageContext& context;
    bool armed{true};
    ~BindingCleanup() noexcept {
        if (armed) {
            resize_native_input_bindings_00a93500(header, 0, context);
            singleton_lifetime_free(get<void*>(header, 0));
        }
    }
};
}

void* construct_native_input_action_record_00a93940(void* record) noexcept {
    put<std::uint8_t>(record, 0, 0); put<std::uint8_t>(record, 1, 0);
    zero_header(offset(record, 4)); zero_header(offset(record, 0x10));
    put<void*>(record, 0x2c, nullptr);
    void* const listener = get<void*>(record, 0x2c);
    put<std::uint32_t>(record, 0x1c, 0); put<std::uint8_t>(record, 0x20, 0);
    put<std::uint32_t>(record, 0x24, 0); put<std::uint8_t>(record, 0x28, 0);
    // Native retains the just-cleared listener reload and conditional block.
    // No calls intervene; it is unreachable in the valid single-thread domain.
    if (listener) {
        constexpr std::uint32_t bytes[]{8, 10, 11, 13, 14, 15, 16, 9, 12, 17, 18, 19};
        for (auto byte : bytes) put<std::uint8_t>(listener, byte, 0);
        for (std::uint32_t word = 0x14; word <= 0x20; word += 4)
            put<std::uint32_t>(listener, word, 0);
    }
    return record;
}
void destroy_native_input_action_record_00a939c0(void* record, NativeInputActionRecordsContext& context) {
    // DECAE8 states1->0->-1: listener failure cleans both arrays; failure in
    // normal binding cleanup cleans only the earlier DWORD array.
    WordCleanup words{offset(record, 4)};
    BindingCleanup bindings{offset(record, 0x10), context.bindings};
    void* const listener = get<void*>(record, 0x2c);
    if (listener) {
        if (InterlockedDecrement(static_cast<volatile LONG*>(offset(listener, 4))) == 0)
            context.listeners.call_listener_slot0(listener, get<std::uint32_t>(listener, 0));
        put<void*>(record, 0x2c, nullptr);
    }
    bindings.armed = false;
    resize_native_input_bindings_00a93500(bindings.header, 0, context.bindings);
    singleton_lifetime_free(get<void*>(bindings.header, 0));
    words.armed = false;
    resize_native_input_context_words_00696e70(words.header, 0);
    singleton_lifetime_free(get<void*>(words.header, 0));
}
void* delete_native_input_action_record_00a93a60(void* record, std::uint32_t flags,
    NativeInputActionRecordsContext& context) {
    destroy_native_input_action_record_00a939c0(record, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(record);
    return record;
}
void* copy_native_input_action_record_00a93a80(void* destination, const void* source,
    NativeInputActionRecordsContext& context) {
    put<std::uint8_t>(destination, 0, get<std::uint8_t>(source, 0));
    put<std::uint8_t>(destination, 1, get<std::uint8_t>(source, 1));
    void* const words = offset(destination, 4);
    zero_header(words);
    assign_native_input_context_words_00a92e70(words, offset(source, 4));
    WordCleanup cleanup{words};
    void* const bindings = offset(destination, 0x10);
    zero_header(bindings);
    assign_native_input_bindings_00a937e0(bindings, offset(source, 0x10), context.bindings);
    copy_float(offset(destination, 0x1c), offset(source, 0x1c));
    put<std::uint8_t>(destination, 0x20, get<std::uint8_t>(source, 0x20));
    copy_float(offset(destination, 0x24), offset(source, 0x24));
    put<std::uint8_t>(destination, 0x28, get<std::uint8_t>(source, 0x28));
    put<void*>(destination, 0x2c, nullptr);
    void* const listener = get<void*>(source, 0x2c);
    if (listener) {
        put<void*>(destination, 0x2c, listener);
        InterlockedIncrement(static_cast<volatile LONG*>(offset(listener, 4)));
    }
    cleanup.armed = false;
    return destination;
}
void reserve_native_input_actions_00a93b30(void* header, std::int32_t capacity,
    NativeInputActionRecordsContext& context) {
    if (capacity < 1) capacity = 1;
    if (capacity <= get<std::int32_t>(header, 8)) return;
    const std::uint32_t bytes = static_cast<std::uint32_t>(capacity) * 0x30u;
    void* const replacement = singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
    for (std::int32_t i = 0; i < get<std::int32_t>(header, 4); ++i) {
        void* const destination = offset(replacement, static_cast<std::uint32_t>(i) * 0x30u);
        if (destination) copy_native_input_action_record_00a93a80(destination,
            offset(get<void*>(header, 0), static_cast<std::uint32_t>(i) * 0x30u), context);
    }
    for (std::int32_t i = 0; i < get<std::int32_t>(header, 4); ++i)
        destroy_native_input_action_record_00a939c0(
            offset(get<void*>(header, 0), static_cast<std::uint32_t>(i) * 0x30u), context);
    singleton_lifetime_free(get<void*>(header, 0));
    put<void*>(header, 0, replacement);
    put<std::int32_t>(header, 8, capacity);
}
void resize_native_input_actions_00a93c10(void* header, std::int32_t count,
    NativeInputActionRecordsContext& context) {
    if (get<std::int32_t>(header, 8) < count) reserve_native_input_actions_00a93b30(header, count, context);
    for (std::int32_t i = get<std::int32_t>(header, 4); i < count; ++i) {
        void* const record = offset(get<void*>(header, 0), static_cast<std::uint32_t>(i) * 0x30u);
        if (record) construct_native_input_action_record_00a93940(record);
    }
    while (count < get<std::int32_t>(header, 4)) {
        const std::int32_t index = get<std::int32_t>(header, 4) - 1;
        put<std::int32_t>(header, 4, index);
        destroy_native_input_action_record_00a939c0(
            offset(get<void*>(header, 0), static_cast<std::uint32_t>(index) * 0x30u), context);
    }
    put<std::int32_t>(header, 4, count);
}
} // namespace bsp

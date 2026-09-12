#include "bsp/native_input_binding_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native input storage requires MSVC Win32.
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
void* allocate(std::uint32_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
std::int32_t doubled(std::int32_t capacity) noexcept {
    const auto result = static_cast<std::int32_t>(static_cast<std::uint32_t>(capacity) * 2u);
    return result < 2 ? 1 : result;
}
void copy_float(void* destination, const void* source) noexcept {
    __asm { mov eax, destination }
    __asm { mov edx, source }
    __asm { fld dword ptr [edx] }
    __asm { fstp dword ptr [eax] }
}
// Game custom count/capacity arrays, not std::vector's begin/end/end layout.
// Old base/count are reloaded after the allocator/new-handler can run.
void reserve_words(void* header, std::int32_t capacity, std::uint32_t stride) {
    if (capacity < 1) capacity = 1;
    if (capacity <= get<std::int32_t>(header, 8)) return;
    void* const replacement = allocate(static_cast<std::uint32_t>(capacity) * stride);
    for (std::int32_t i = 0; i < get<std::int32_t>(header, 4); ++i) {
        void* const destination = offset(replacement, static_cast<std::uint32_t>(i) * stride);
        if (destination) {
            const void* const source = offset(get<void*>(header, 0), static_cast<std::uint32_t>(i) * stride);
            for (std::uint32_t word = 0; word < stride; word += 4)
                put<std::uint32_t>(destination, word, get<std::uint32_t>(source, word));
        }
    }
    singleton_lifetime_free(get<void*>(header, 0));
    put<void*>(header, 0, replacement);
    put<std::int32_t>(header, 8, capacity);
}
using Reserve = void (*)(void*, std::int32_t);
void resize_words(void* header, std::int32_t count, std::uint32_t stride, Reserve reserve) {
    if (get<std::int32_t>(header, 8) < count) reserve(header, count);
    for (std::int32_t i = get<std::int32_t>(header, 4); i < count; ++i) {
        void* const slot = offset(get<void*>(header, 0), static_cast<std::uint32_t>(i) * stride);
        if (slot) {
            put<std::uint32_t>(slot, 0, stride == 4 ? 0u : 0xffffffffu);
            if (stride == 0x14) {
                put<std::uint32_t>(slot, 4, 0); put<std::uint32_t>(slot, 8, 0);
                put<std::uint32_t>(slot, 12, 0); put<std::uint8_t>(slot, 16, 0);
            }
        }
    }
    while (count < get<std::int32_t>(header, 4))
        put<std::int32_t>(header, 4, get<std::int32_t>(header, 4) - 1);
    put<std::int32_t>(header, 4, count);
}
void* assign_words(void* destination, const void* source, std::uint32_t stride,
    Reserve resize, Reserve reserve) {
    resize(destination, 0);
    reserve(destination, get<std::int32_t>(source, 4));
    for (std::int32_t i = 0; i < get<std::int32_t>(source, 4); ++i) {
        // Native captures this source element before a possible growth callback.
        const void* const incoming = offset(get<void*>(source, 0), static_cast<std::uint32_t>(i) * stride);
        if (get<std::int32_t>(destination, 4) == get<std::int32_t>(destination, 8))
            reserve(destination, doubled(get<std::int32_t>(destination, 8)));
        void* const output = offset(get<void*>(destination, 0),
            static_cast<std::uint32_t>(get<std::int32_t>(destination, 4)) * stride);
        if (output) for (std::uint32_t word = 0; word < stride; word += 4)
            put<std::uint32_t>(output, word, get<std::uint32_t>(incoming, word));
        put<std::int32_t>(destination, 4, get<std::int32_t>(destination, 4) + 1);
    }
    return destination;
}
struct ModifierCleanup {
    void* header;
    bool armed{true};
    ~ModifierCleanup() noexcept {
        if (armed) {
            resize_native_input_modifiers_00697220(header, 0);
            singleton_lifetime_free(get<void*>(header, 0));
        }
    }
};
void destroy_binding_fields(void* binding) {
    ModifierCleanup first{offset(binding, 0x18)};
    void* const second = offset(binding, 0x24);
    resize_native_input_modifiers_00697220(second, 0);
    singleton_lifetime_free(get<void*>(second, 0));
    first.armed = false;
    resize_native_input_modifiers_00697220(first.header, 0);
    singleton_lifetime_free(get<void*>(first.header, 0));
}
}

void reserve_native_input_dwords_0086a220(void* h, std::int32_t n) { reserve_words(h, n, 4); }
void resize_native_input_dwords_0086a430(void* h, std::int32_t n) { resize_words(h, n, 4, reserve_native_input_dwords_0086a220); }
void reserve_native_input_context_words_00696d80(void* h, std::int32_t n) { reserve_words(h, n, 4); }
void resize_native_input_context_words_00696e70(void* h, std::int32_t n) { resize_words(h, n, 4, reserve_native_input_context_words_00696d80); }
void reserve_native_input_modifiers_00696de0(void* h, std::int32_t n) { reserve_words(h, n, 0x14); }
void resize_native_input_modifiers_00697220(void* h, std::int32_t n) { resize_words(h, n, 0x14, reserve_native_input_modifiers_00696de0); }
void* assign_native_input_context_words_00a92e70(void* d, const void* s) { return assign_words(d, s, 4, resize_native_input_context_words_00696e70, reserve_native_input_context_words_00696d80); }
void* assign_native_input_modifiers_00a92ee0(void* d, const void* s) { return assign_words(d, s, 0x14, resize_native_input_modifiers_00697220, reserve_native_input_modifiers_00696de0); }

void* copy_native_input_binding_00a93100(void* destination, const void* source) {
    put<std::uint8_t>(destination, 0, get<std::uint8_t>(source, 0));
    put<std::uint8_t>(destination, 1, get<std::uint8_t>(source, 1));
    for (std::uint32_t word = 4; word <= 0x14; word += 4)
        put<std::uint32_t>(destination, word, get<std::uint32_t>(source, word));
    void* const first = offset(destination, 0x18);
    zero_header(first);
    assign_native_input_modifiers_00a92ee0(first, offset(source, 0x18));
    ModifierCleanup cleanup{first};
    void* const second = offset(destination, 0x24);
    zero_header(second);
    assign_native_input_modifiers_00a92ee0(second, offset(source, 0x24));
    copy_float(offset(destination, 0x30), offset(source, 0x30));
    cleanup.armed = false;
    return destination;
}
void reserve_native_input_bindings_00a93220(void* header, std::int32_t capacity) {
    if (capacity < 1) capacity = 1;
    if (capacity <= get<std::int32_t>(header, 8)) return;
    void* const replacement = allocate(static_cast<std::uint32_t>(capacity) * 0x34u);
    for (std::int32_t i = 0; i < get<std::int32_t>(header, 4); ++i) {
        void* const destination = offset(replacement, static_cast<std::uint32_t>(i) * 0x34u);
        if (destination) copy_native_input_binding_00a93100(destination,
            offset(get<void*>(header, 0), static_cast<std::uint32_t>(i) * 0x34u));
    }
    for (std::int32_t i = 0; i < get<std::int32_t>(header, 4); ++i)
        destroy_binding_fields(offset(get<void*>(header, 0), static_cast<std::uint32_t>(i) * 0x34u));
    singleton_lifetime_free(get<void*>(header, 0));
    put<void*>(header, 0, replacement);
    put<std::int32_t>(header, 8, capacity);
}
void resize_native_input_bindings_00a93500(void* header, std::int32_t count,
    const NativeInputBindingStorageContext& context) {
    if (get<std::int32_t>(header, 8) < count) reserve_native_input_bindings_00a93220(header, count);
    if (get<std::int32_t>(header, 4) < count) {
      const std::uint32_t one = context.one_00d7a24c;
      for (std::int32_t i = get<std::int32_t>(header, 4); i < count; ++i) {
        void* const binding = offset(get<void*>(header, 0), static_cast<std::uint32_t>(i) * 0x34u);
        if (binding) {
            put<std::uint8_t>(binding, 0, 0); put<std::uint8_t>(binding, 1, 0);
            put<std::uint32_t>(binding, 4, 0xffffffffu);
            put<std::uint32_t>(binding, 8, 0); put<std::uint32_t>(binding, 0xc, 0);
            put<std::uint32_t>(binding, 0x10, 0); put<std::uint8_t>(binding, 0x14, 0);
            zero_header(offset(binding, 0x18)); zero_header(offset(binding, 0x24));
            put<std::uint32_t>(binding, 0x30, one);
        }
      }
    }
    while (count < get<std::int32_t>(header, 4)) {
        const std::int32_t index = get<std::int32_t>(header, 4) - 1;
        put<std::int32_t>(header, 4, index);
        destroy_binding_fields(offset(get<void*>(header, 0), static_cast<std::uint32_t>(index) * 0x34u));
    }
    put<std::int32_t>(header, 4, count);
}
void append_native_input_binding_00a93440(void* header, const void* source) {
    if (get<std::int32_t>(header, 4) == get<std::int32_t>(header, 8))
        reserve_native_input_bindings_00a93220(header, doubled(get<std::int32_t>(header, 8)));
    void* const output = offset(get<void*>(header, 0), static_cast<std::uint32_t>(get<std::int32_t>(header, 4)) * 0x34u);
    if (output) copy_native_input_binding_00a93100(output, source);
    put<std::int32_t>(header, 4, get<std::int32_t>(header, 4) + 1);
}
void* assign_native_input_bindings_00a937e0(void* destination, const void* source,
    const NativeInputBindingStorageContext& context) {
    resize_native_input_bindings_00a93500(destination, 0, context);
    reserve_native_input_bindings_00a93220(destination, get<std::int32_t>(source, 4));
    for (std::int32_t i = 0; i < get<std::int32_t>(source, 4); ++i)
        append_native_input_binding_00a93440(destination,
            offset(get<void*>(source, 0), static_cast<std::uint32_t>(i) * 0x34u));
    return destination;
}
} // namespace bsp

#include "bsp/native_input_backend_bindings.hpp"
#include "bsp/native_singleton_vector_allocation.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <new>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
using Word = std::uint32_t;
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(Word p, Word offset = 0) noexcept {
    Word value;
    std::memcpy(&value, pointer(p + offset), 4);
    return value;
}
void write(Word p, Word offset, Word value) noexcept { std::memcpy(pointer(p + offset), &value, 4); }
void write_byte(Word p, Word offset, unsigned char value) noexcept {
    std::memcpy(pointer(p + offset), &value, 1);
}
Word words(Word first, Word last) noexcept {
    return first ? static_cast<Word>(static_cast<std::int32_t>(last - first) >> 2) : 0;
}
void invalid_parameter() { _invalid_parameter_noinfo(); } // real returning CRT service
Word move_words(Word first, Word last, Word destination) {
    const auto bytes = static_cast<Word>(static_cast<std::int32_t>(last - first) >> 2) * 4u;
    if (bytes) (void)memmove_s(pointer(destination), bytes, pointer(first), bytes);
    return destination + bytes;
}
void changed(Word backend, Word device_class, std::int32_t index, NativeInputBackendBindingsCalls& calls) {
    const auto identity = read(backend, 0xd8);
    if (identity) calls.devices_changed_d8(identity, device_class, index);
}
} // namespace

void insert_input_active_pointer_storage(void* output, void* header,
    void* position, const void* value_word) {
    const auto h = address(header), insertion = address(position);
    const auto captured_begin = read(h, 4);
    Word index = 0;
    if (words(captured_begin, read(h, 8)) != 0) {
        if (captured_begin > read(h, 8)) invalid_parameter();
        if (!h) invalid_parameter();
        index = static_cast<Word>(static_cast<std::int32_t>(insertion - captured_begin) >> 2);
    }
    const auto value = read(address(value_word));
    const auto first = read(h, 4);
    const auto capacity = words(first, read(h, 0xc));
    const auto count = words(first, read(h, 8));
    constexpr Word maximum = 0x3fffffffu;
    if (maximum - count < 1u) native_singleton_length_error_00bd0590();
    const auto required = words(first, read(h, 8)) + 1u;
    if (capacity < required) {
        auto next_capacity = maximum - (capacity >> 1) < capacity ? 0u : capacity + (capacity >> 1);
        const auto minimum = words(first, read(h, 8)) + 1u;
        if (next_capacity < minimum) next_capacity = words(first, read(h, 8)) + 1u;
        if (next_capacity > maximum) throw std::bad_alloc();
        const auto bytes = next_capacity * 4u;
        auto* replacement = singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
        const auto replacement_word = address(replacement);
        // The checked-library contract reloads begin AFTER allocation, and
        // end AFTER the first copy/fill, including returning CRT handlers.
        const auto middle = move_words(read(h, 4), insertion, replacement_word);
        write(middle, 0, value);
        (void)move_words(insertion, read(h, 8), middle + 4u);
        const auto current_begin = read(h, 4);
        const auto retained = words(current_begin, read(h, 8)) + 1u;
        if (current_begin) singleton_lifetime_free(pointer(current_begin));
        write(h, 4, replacement_word);
        write(h, 0xc, replacement_word + bytes);
        write(h, 8, replacement_word + retained * 4u);
    } else {
        // Stateless single-element insertion contract, valid contiguous ranges.
        const auto end = read(h, 8);
        (void)move_words(insertion, end, insertion + 4u);
        write(h, 8, read(h, 8) + 4u);
        write(insertion, 0, value);
    }
    const auto result_begin = read(h, 4);
    if (result_begin > read(h, 8)) invalid_parameter();
    const auto result = result_begin + index * 4u;
    if (result > read(h, 8) || result < read(h, 4)) invalid_parameter();
    write(address(output), 4, result);
    write(address(output), 0, h);
}

void activate_native_input_device_slot_00a91620(void* actual_backend, Word device_class,
    Word slot, NativeInputBackendBindingsContext& context) {
    const auto backend = address(actual_backend);
    const auto group = backend + 0x68u + device_class * 0x24u;
    const auto accepted_header = group + 0x14u;
    auto position = read(group, 0x18);
    const auto initial_end = read(group, 0x1c);
    Word candidate = read(backend, 4u + (slot + device_class * 8u) * 4u);
    bool accepted = false;
    if (position > initial_end) invalid_parameter();
    for (;;) {
        const auto captured_end = read(accepted_header, 8);
        if (read(accepted_header, 4) > captured_end) invalid_parameter();
        if (!accepted_header) invalid_parameter();
        if (position == captured_end) break;
        if (!accepted_header) invalid_parameter();
        if (position >= read(accepted_header, 8)) invalid_parameter();
        if (read(position) == 0xffffffffu) accepted = true;
        else {
            if (position >= read(accepted_header, 8)) invalid_parameter();
            const auto profile = read(candidate);
            const auto identifier = context.calls.identifier_vslot34(pointer(candidate), profile);
            if (read(position) == static_cast<Word>(identifier)) accepted = true;
        }
        if (position >= read(accepted_header, 8)) invalid_parameter();
        position += 4u;
    }
    if (!accepted) return;
    const auto active = group + 4u;
    position = read(group, 8);
    if (position > read(group, 0xc)) invalid_parameter();
    for (;;) {
        const auto captured_end = read(active, 8);
        if (read(active, 4) > captured_end) invalid_parameter();
        if (!active) invalid_parameter();
        if (position == captured_end) break;
        if (!active) invalid_parameter();
        if (position >= read(active, 8)) invalid_parameter();
        if (read(position) == candidate) break;
        if (position >= read(active, 8)) invalid_parameter();
        position += 4u;
    }
    const auto captured_end = read(active, 8);
    if (read(active, 4) > captured_end) invalid_parameter();
    if (!active) invalid_parameter();
    if (position != captured_end) return;
    write_byte(backend, 0xd4, 1);
    const auto first = read(active, 4);
    const auto previous_count = words(first, read(active, 8));
    const auto count = words(first, read(active, 8));
    if (first && count < words(first, read(active, 0xc))) {
        const auto destination = read(active, 8);
        write(destination, 0, candidate);
        write(active, 8, destination + 4u);
    } else {
        const auto insertion = read(active, 8);
        if (first > insertion) invalid_parameter();
        Word iterator[2];
        insert_input_active_pointer_storage(iterator, pointer(active), pointer(insertion), &candidate);
    }
    changed(backend, device_class, static_cast<std::int32_t>(previous_count), context.calls);
}

void remove_active_native_input_device_00a90ee0(void* actual_backend, void* actual_device,
    NativeInputBackendBindingsContext& context) {
    const auto backend = address(actual_backend), device = address(actual_device);
    const auto profile = read(device);
    const auto device_class = context.calls.device_class_vslot08(actual_device, profile);
    const auto active = backend + 0x6cu + device_class * 0x24u;
    auto position = read(active, 4);
    if (position > read(active, 8)) invalid_parameter();
    for (;;) {
        const auto captured_end = read(active, 8);
        if (read(active, 4) > captured_end) invalid_parameter();
        // Native CMP ESI,ESI always skips the A90F21 validation call.
        if (position == captured_end) return;
        if (position >= read(active, 8)) invalid_parameter();
        if (read(position) == device) break;
        if (position >= read(active, 8)) invalid_parameter();
        position += 4u;
    }
    write_byte(backend, 0xd4, 1);
    const auto following = position + 4u;
    const auto count = static_cast<std::int32_t>(read(active, 8) - following) >> 2;
    if (count > 0) {
        const auto bytes = static_cast<Word>(count) * 4u;
        (void)memmove_s(pointer(position), bytes, pointer(following), bytes);
    }
    write(active, 8, read(active, 8) - 4u);
    changed(backend, device_class, -1, context.calls);
}

void delete_native_input_device_class_00bebf30(void* actual_backend, Word device_class,
    NativeInputBackendBindingsContext& context) {
    auto cell = address(actual_backend) + 4u + device_class * 0x20u;
    for (Word remaining = 8; remaining != 0; --remaining, cell += 4u) {
        const auto device = read(cell);
        if (device) {
            const auto profile = read(device);
            context.calls.delete_vslot04(pointer(device), profile, 1);
            write(cell, 0, 0);
        }
    }
}
void enumerate_native_input_devices_00a983c0(void* actual_backend,
    NativeInputBackendBindingsContext& context) {
    const auto backend = address(actual_backend);
    const auto captured_interface = read(backend, 0xe0);
    write_byte(backend, 0xf4, 0);
    context.calls.enumerate_devices_vslot10(pointer(captured_interface), 0, actual_backend, 1);
}
} // namespace bsp

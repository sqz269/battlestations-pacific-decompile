#include "bsp/native_vfs_mount_records.hpp"

#include <cstring>

namespace bsp {
namespace {
const void* at(const void* owner, std::uint32_t offset) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(owner) + offset);
}
void* at(void* owner, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(owner) + offset);
}
std::uint32_t word(const void* owner, std::uint32_t offset = 0) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at(owner, offset));
}
void put(void* owner, std::uint32_t offset, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(at(owner, offset)) = value;
}
void construct_payload(void* destination, const void* source, bool same_header,
    NativeStringStorage& storage) {
    put(destination, 0, 0);
    put(destination, 4, 0);
    if (!same_header) {
        resize_native_string_header_0041dd40(destination, storage, word(source), true);
        if (word(source) != 0) {
            const auto length = word(destination);
            const auto* const source_bytes = reinterpret_cast<const void*>(word(source, 4));
            auto* const destination_bytes = reinterpret_cast<void*>(word(destination, 4));
            if (length != 0) std::memmove(destination_bytes, source_bytes, length);
        }
    }
    const auto provider = word(source, 8);
    put(destination, 8, provider);
    const auto flag = *static_cast<const volatile std::uint8_t*>(at(source, 0xc));
    *static_cast<volatile std::uint8_t*>(at(destination, 0xc)) = flag;
}
} // namespace

void trim_native_string_right_00584110(void* header, const char* trim_set,
    NativeStringStorage& storage) {
    if (trim_set == nullptr || *trim_set == '\0') return;
    const auto initial_data = word(header, 4);
    if (initial_data == 0) return;
    const auto initial_length = word(header);
    if (initial_length == 0) return;
    auto cursor = initial_data + initial_length - 1u;
    if (cursor > initial_data) {
        do {
            const int value = *reinterpret_cast<const volatile std::int8_t*>(cursor);
            if (std::strchr(trim_set, value) == nullptr) break;
            --cursor;
        } while (cursor > word(header, 4));
    }
    const auto length = cursor - word(header, 4) + 1u;
    resize_native_string_header_0041dd40(header, storage, length, true);
}

void* copy_native_vfs_mount_record_00bdce40(void* destination,
    const void* source, NativeStringStorage& storage) {
    const auto priority = word(source);
    void* const target_header = at(destination, 4);
    const void* const source_header = at(source, 4);
    const bool same_header = target_header == source_header;
    put(destination, 0, priority);
    construct_payload(target_header, source_header, same_header, storage);
    return destination;
}

void* construct_native_vfs_mount_record_00bdcfc0(void* destination,
    const void* priority_dword, const void* payload, NativeStringStorage& storage) {
    const auto priority = word(priority_dword);
    void* const target_header = at(destination, 4);
    const bool same_header = target_header == payload;
    put(destination, 0, priority);
    construct_payload(target_header, payload, same_header, storage);
    return destination;
}

void* construct_native_vfs_priority_record_00bdeec0(void* destination,
    std::uint32_t priority, void* argument, NativeStringStorage& storage) {
    // E00A6C / E00A5C: state1->0 CC6520 releases EBP+4 argument; state0->-1
    // CC6528 tests/clears completion bit at EBP-14h before BDB570 on EBP-10h.
    bool argument_armed = true;
    bool destination_complete = false;
    try {
        construct_native_vfs_mount_record_00bdcfc0(destination, &priority, argument, storage);
        destination_complete = true;
        argument_armed = false;
        destroy_native_string_header_0041dd20(argument, storage);
    } catch (...) {
        if (argument_armed) destroy_native_string_header_0041dd20(argument, storage);
        if (destination_complete) {
            destination_complete = false;
            destroy_native_string_header_0041dd20(at(destination, 4), storage);
        }
        throw;
    }
    return destination;
}
} // namespace bsp

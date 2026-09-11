#include "bsp/native_logical_texture_named_base.hpp"

#include <cstddef>
#include <cstring>

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4, "Native texture storage requires Win32.");

template<class T> T read(const void* object, std::size_t offset = 0) noexcept {
    T value;
    std::memcpy(&value, static_cast<const char*>(object) + offset, sizeof(value));
    return value;
}
template<class T> void write(void* object, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<char*>(object) + offset, &value, sizeof(value));
}

// Full BD30F0: one vtable store, no count decrement or storage release.
void destroy_refcounted_base(void* owner) noexcept {
    write(owner, 0, std::uint32_t{0x00ceb130});
}

// Both original FuncInfos contain only unwind maps, with no catch map.
// Armed RAII keeps cleanup on the original exception's unwind; a synthetic
// catch/rethrow would introduce an observable extra C++ exception search.
struct BaseUnwindCleanup {
    void* owner;
    bool armed{true};
    ~BaseUnwindCleanup() noexcept { if (armed) destroy_refcounted_base(owner); }
};
struct NameUnwindCleanup {
    void* header;
    NativeStringStorage& storage;
    bool armed{true};
    ~NameUnwindCleanup() noexcept {
        if (armed) destroy_native_string_header_0041dd20(header, storage);
    }
};

} // namespace

void* construct_native_logical_texture_named_base_00b34120(void* owner,
    const void* source, void* borrowed_com, std::uint32_t flags,
    NativeStringStorage& storage, std::uint32_t& serial) {
    write(owner, 0, std::uint32_t{0x00ceb130});
    write(owner, 4, std::uint32_t{1});
    auto* const name = static_cast<char*>(owner) + 8;
    write(owner, 0, std::uint32_t{0x00d5f1f4});
    BaseUnwindCleanup base{owner}; // State 0 at 00B34155.
    write(name, 0, std::uint32_t{0});
    write<char*>(name, 4, nullptr);
    NameUnwindCleanup cleanup{name, storage}; // State 1 at 00B34170.
    if (name != source) {
        resize_native_string_header_0041dd40(name, storage,
            read<std::uint32_t>(source), true);
        // Allocation/release can change either actual header. Native tests
        // CURRENT source length, then copies CURRENT destination length.
        if (read<std::uint32_t>(source) != 0) {
            const auto count = read<std::uint32_t>(name);
            auto* const source_data = read<const char*>(source, 4);
            auto* const destination_data = read<char*>(name, 4);
            // BF7680 detects backward overlap at BF7694..BF769A and copies
            // backward through BF7844. Keep that behavior for buffer aliases.
            // Shared string policy omits the native zero-byte copy.
            if (count != 0) std::memmove(destination_data, source_data, count);
        }
    }
    write(owner, 0x10, borrowed_com);
    write(owner, 0x1c, flags);
    write(owner, 0x20, read<std::uint32_t>(&serial));
    // ADD reads the actual global again after the owner's store, preserving
    // aliases instead of incrementing a snapshot captured before the store.
    write(&serial, 0, read<std::uint32_t>(&serial) + 1u);
    cleanup.armed = false;
    base.armed = false;
    return owner;
}

void* construct_native_logical_texture_named_profile_00b34230(void* owner,
    const void* source, void* borrowed_com, std::uint32_t flags,
    NativeStringStorage& storage, std::uint32_t& serial) {
    construct_native_logical_texture_named_base_00b34120(owner, source,
        borrowed_com, flags, storage, serial);
    write(owner, 0, std::uint32_t{0x00d5f228});
    return owner;
}

void destroy_native_logical_texture_named_base_00b33f50(void* owner,
    NativeStringStorage& storage) {
    write(owner, 0, std::uint32_t{0x00d5f1f4});
    BaseUnwindCleanup base{owner}; // State 0 protects the name release only.
    destroy_native_string_header_0041dd20(static_cast<char*>(owner) + 8, storage);
    base.armed = false; // 00B33F9A clears state before the normal base action.
    destroy_refcounted_base(owner);
}

void unwind_native_logical_texture_named_base_00b34010(void* owner,
    NativeStringStorage& storage) {
    destroy_native_logical_texture_named_base_00b33f50(owner, storage);
}

void* native_logical_texture_name_address_00b33e40(void* owner) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(owner) + 8u);
}

} // namespace bsp

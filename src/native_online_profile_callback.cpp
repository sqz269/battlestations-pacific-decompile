#include "bsp/native_online_profile_callback.hpp"

#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstddef>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native online profile callback requires MSVC Win32 storage and ABI.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(NativeString) == 8);

template<class T> T read(const void* owner, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(owner) + offset, sizeof(value));
    return value;
}

void* header(void* profile, std::size_t offset) noexcept {
    return static_cast<std::byte*>(profile) + offset;
}

void assign_header(void* destination, const NativeString& source,
    NativeStringStorage& storage) {
    if (destination == &source) return;
    const auto requested = read<std::uint32_t>(&source, 0);
    resize_native_string_header_0041dd40(destination, storage, requested, true);
    if (read<std::uint32_t>(&source, 0) != 0) {
        const auto length = read<std::uint32_t>(destination, 0);
        const auto* data = read<const char*>(&source, 4);
        auto* output = read<char*>(destination, 4);
        // The raw string helpers omit only a zero-byte CRT memcpy.
        if (length != 0) std::memcpy(output, data, length);
    }
}

void mirror_selected_name(void* captured_profile, void* volatile& current_game,
    NativeStringStorage& storage) {
    void* chosen = read<std::uint32_t>(header(captured_profile, 0x50), 0) != 0
        ? header(captured_profile, 0x50) : header(captured_profile, 0x3c);
    NativeString substring;
    construct_native_string_substring_00469840(chosen, &substring, 0, 0x1f, storage);
    const char* source = substring.data();
    if (source == nullptr) source = ""; // DAT_00F8745C is the native empty source.
    // The getter is deliberately AFTER substring construction: allocation and
    // return callbacks can publish another game before this mirror write.
    auto* destination = static_cast<char*>(current_game) + 0x1ff0;
    char ch;
    do {
        ch = *source++;
        *destination++ = ch;
    } while (ch != 0);
    destroy_native_string_header_0041dd20(&substring, storage);
}

void set_name_field(void* captured_profile, const NativeString& source,
    std::size_t field_offset, void* volatile& current_game, NativeStringStorage& storage) {
    assign_header(header(captured_profile, field_offset), source, storage);
    mirror_selected_name(captured_profile, current_game, storage);
}
} // namespace

const char* selected_native_online_name_00a3eae0(
    const NativeOnlineManagerStorage& captured_manager) noexcept {
    const auto selected = read<std::uint32_t>(&captured_manager, 0x11c);
    const auto manager = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(&captured_manager));
    const auto address = manager + 0x90u + (selected << 7u);
    return reinterpret_cast<const char*>(static_cast<std::uintptr_t>(address));
}

void set_native_profile_display_name_007f9340(void* captured_profile_650,
    const NativeString& source, void* volatile& current_game_00e188a8,
    NativeStringStorage& storage) {
    set_name_field(captured_profile_650, source, 0x50, current_game_00e188a8, storage);
}

void set_native_profile_name_007f9290(void* captured_profile_650,
    const NativeString& source, void* volatile& current_game_00e188a8,
    NativeStringStorage& storage) {
    set_name_field(captured_profile_650, source, 0x3c, current_game_00e188a8, storage);
}

void apply_native_online_profile_name_00737d60(NativeOnlineProfileCallbackContext& context) {
    ActualNativeStringPoolStorage storage(context.pool.actual_published_01090aa8,
        context.pool.actual_small_returns_disabled_01090aa4,
        context.pool.actual_manager_publication_01090aa0);
    apply_native_online_profile_name_00737d60(context, storage);
}

void apply_native_online_profile_name_00737d60(NativeOnlineProfileCallbackContext& context,
    NativeStringStorage& storage) {
    const auto* manager = context.current_manager_00f8abe8;
    const char* const selected = selected_native_online_name_00a3eae0(*manager);
    NativeString temporary;
    temporary.assign_0041e870(storage, selected);
    try {
        auto* const first_game = context.current_game_00e188a8;
        set_native_profile_display_name_007f9340(
            static_cast<std::byte*>(first_game) + 0x650, temporary,
            context.current_game_00e188a8, storage);
        auto* const second_game = context.current_game_00e188a8;
        set_native_profile_name_007f9290(
            static_cast<std::byte*>(second_game) + 0x650, temporary,
            context.current_game_00e188a8, storage);
    } catch (...) {
        destroy_native_string_header_0041dd20(&temporary, storage);
        throw;
    }
    destroy_native_string_header_0041dd20(&temporary, storage);
}
} // namespace bsp

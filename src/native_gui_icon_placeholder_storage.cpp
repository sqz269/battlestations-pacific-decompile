#include "bsp/native_gui_icon_placeholder_storage.hpp"
#include "bsp/native_gui_icon_storage.hpp"
#include "bsp/native_texture_loading_cache.hpp"
#include "bsp/native_string_raw_resize_exact.hpp"
#include "bsp/native_string_raw_header_return.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_render_context.hpp"
#include <atomic>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw placeholder storage requires MSVC Win32 DWORD contracts.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word live(const Word& p) noexcept { return static_cast<const volatile Word&>(p); }
void set(Word& p, Word v) noexcept { static_cast<volatile Word&>(p) = v; }
Word read(const void* object, Word byte_offset) noexcept {
    Word result;
    __asm {
        mov edx, object
        add edx, byte_offset
        mov eax, dword ptr [edx]
        mov result, eax
    }
    return result;
}
void write(void* object, Word byte_offset, Word value) noexcept {
    __asm {
        mov edx, object
        add edx, byte_offset
        mov eax, value
        mov dword ptr [edx], eax
    }
}
void byte(void* object, Word byte_offset, std::uint8_t value) noexcept {
    *reinterpret_cast<volatile std::uint8_t*>(pointer(bits(object) + byte_offset)) = value;
}
} // namespace

void set_native_gui_icon_placeholder_storage_00ab3b30(void* actual_state,
    NativeGuiIconPlaceholderStorageFrame& f, NativeGuiIconPlaceholderStorageContext& c) {
    set(f.state_home, 0xffffffffu);
    set(f.preserve_home, 1);
    byte(actual_state, 0x3c, 1);
    byte(actual_state, 0x3d, 0);
    set(f.header[0], 0);
    set(f.header[1], 0);
    resize_native_string_header_0041dd40_exact(f.header, c.icon.strings, 0x29,
        &f.preserve_home, c.feature_word_0109eea4);
    const Word constructed_data = live(f.header[1]);
    if (constructed_data != 0) {
        const Word count = live(f.header[0]) + 1u;
        copy_native_crt_bytes_00bf7680(pointer(constructed_data),
            c.transparent_literal_00d5c058, count, c.feature_word_0109eea4);
    }
    try {
        const void* const renderer = c.cache.textures.current_renderer_00f8d394;
        const Word profile = read(renderer, 0);
        const Word target = read(pointer(profile), 0x64);
        (void)static_cast<const volatile Word&>(target);
        set(f.state_home, 0);
        const Word loaded = bits(load_native_renderer_texture_00b319b0(
            pointer(bits(renderer)), f.header, 0, c.cache, &c.load_acquired));
        const Word old = read(actual_state, 0);
        if (old != loaded) {
            write(actual_state, 0, loaded);
            if (loaded != 0) {
                auto& count = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
                    pointer(loaded + 4u)));
                count.fetch_add(1, std::memory_order_seq_cst);
            }
            if (old != 0)
                release_native_render_actual_owner_observed(c.icon.actual_owners,
                    pointer(old), c.old_release);
        }
    } catch (...) {
        return_native_string_header_buffer_0041dd20_exact(f.header, c.icon.strings);
        throw;
    }
    const Word cleanup_data = live(f.header[1]);
    set(f.state_home, 0xffffffffu);
    if (cleanup_data != 0) {
        const Word size = live(f.header[0]) + 1u;
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            c.icon.strings.actual_published_01090aa8,
            c.icon.strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, pointer(cleanup_data), size,
            c.icon.strings.actual_small_returns_disabled_01090aa4);
    }
}
} // namespace bsp

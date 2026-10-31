#include "bsp/native_renderer_texture_find_storage.hpp"
#include "bsp/native_texture_loading_cache.hpp"
#include "bsp/native_renderer_synchronization_actual.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_raw_resize_exact.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <climits>
#include <cstring>
#include <intrin.h>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(CHAR_BIT == 8 && sizeof(void*) == 4 && sizeof(Word) == 4);
Word bits(const void* p) {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(p));
}
void* pointer(Word value) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
const void* address(const void* p, Word offset) {
    return pointer(bits(p) + offset);
}
// Alias-safe raw representations, matching the exact string suppliers. The
// compiler barriers keep separate reached reads at their Native epochs.
Word word(const void* p, Word offset = 0) {
    Word value;
    _ReadWriteBarrier();
    std::memcpy(&value, address(p, offset), sizeof value);
    _ReadWriteBarrier();
    return value;
}
volatile Word& live(Word& value) {
    return value;
}
} // namespace

void* find_native_renderer_texture_storage_00b31ac0(void* actual_renderer,
    volatile Word (&homes)[2], NativeRendererTextureFindStorageContext& c,
    NativeRendererTextureFindStorageFrame& f, NativeRendererTextureFindStorageAcquired& a) {
    void* const renderer = actual_renderer; // B31AE2: captured incoming ECX
    a.renderer = renderer;
    a.home_identity = homes;
    a.phase = NativeRendererTextureFindStorageAcquired::Phase::entered;
    try {
        if (c.cache.synchronization_0108d6dc.mode_00 != 0) { // B31AD8
            const void* const guard_renderer = c.cache.textures.current_renderer_00f8d394;
            live(f.guard_renderer) = bits(guard_renderer); // B31AEC, before enter
            a.active_native_site = 0x00b31af0;
            const std::uint8_t entered = enter_native_renderer_optional_guard_00b33ad0(
                guard_renderer, c.cache.synchronization_0108d6dc);
            *reinterpret_cast<volatile std::uint8_t*>(&f.guard_token) = entered;
            a.guard_enter_returned = true; // AL-only store preserves upper24 bits
        }
        const void* const name = pointer(homes[0]); // B31AF9 AFTER guard returns
        a.name_header = name;
        Word captured_data = 0; // B31AFD XOR ESI,ESI
        const bool same_header = static_cast<const void*>(f.header) == name; // B31B03
        a.native_eh_state = 0; // B31B05, private diagnostic only
        live(f.header[0]) = 0; // B31B0D
        live(f.header[1]) = 0; // B31B15
        if (!same_header) {
            const Word request = word(name); // B31B1B CURRENT source LEN
            live(f.preserve_home) = 1; // B31B1D actual DWORD1/LOWBYTE HOME
            a.active_native_site = 0x00b31b24;
            resize_native_string_header_0041dd40_exact(f.header, c.strings, request,
                &f.preserve_home, c.feature_word_0109eea4);
            a.resize_returned = true;
            const Word current_source_length = word(name); // B31B29 CMP BEFORE ESI
            captured_data = live(f.header[1]); // B31B2B always, before JZ
            a.captured_buffer = pointer(captured_data);
            if (current_source_length != 0) {
                const Word bytes = live(f.header[0]); // B31B31 CURRENT destination LEN
                const void* const source = pointer(word(name, 4)); // B31B35 CURRENT DATA
                a.active_native_site = 0x00b31b3b;
                copy_native_crt_bytes_00bf7680(pointer(captured_data), source, bytes,
                    c.feature_word_0109eea4); // including wrapped/zero count
                a.copy_returned = true;
            }
        }
        a.native_eh_state = 1; // B31B47 after actual normal copy/alias arm
        a.active_native_site = 0x00b31b4c;
        lowercase_native_string_header_004bcc00(f.header);
        const Word post_word = homes[1]; // B31B51 only AFTER lowercase
        a.post_load_word = post_word;
        a.active_native_site = 0x00b31b65;
        void* const result = load_native_cached_texture_00b30b40(
            pointer(bits(renderer) + 0x1a74u), f.header, post_word, 0, 0,
            c.cache, &a.cache_);
        a.cache_returned = true;
        a.result = result; // B31B6C captured identity, not a post-callback read
        a.native_eh_state = 0; // B31B6E BEFORE normal captured-buffer return
        if (captured_data != 0) {
            const Word size = live(f.header[0]) + Word{1}; // B31B75..7B CURRENT LEN/wrap
            a.active_native_site = 0x00b31b80;
            a.buffer_return_entered = true;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                c.strings.actual_published_01090aa8,
                c.strings.actual_manager_publication_01090aa0);
            a.active_native_site = 0x00b31b87;
            return_native_string_pool_00bd1510(pool, pointer(captured_data), size,
                c.strings.actual_small_returns_disabled_01090aa4);
            a.buffer_returned = true; // no header read/store/reset afterwards
        }
        const std::uint8_t exit_mode = c.cache.synchronization_0108d6dc.mode_00; // B31B8C
        a.native_eh_state = -1; // B31B93 precedes leave, including skipped-entry arm
        if (exit_mode != 0) {
            const Word saved_word = live(f.guard_token); // B31B9D FULL DWORD
            const void* const guard_renderer = pointer(live(f.guard_renderer)); // B31BA1
            a.active_native_site = 0x00b31ba6;
            a.guard_leave_entered = true;
            leave_native_renderer_optional_guard_00b33b00(guard_renderer, saved_word,
                c.cache.synchronization_0108d6dc);
            a.guard_leave_returned = true;
        }
        a.phase = NativeRendererTextureFindStorageAcquired::Phase::complete;
        return result; // captured EDI identity even if a later provider ended payload
    } catch (...) {
        a.phase = NativeRendererTextureFindStorageAcquired::Phase::failed;
        a.failure = std::current_exception(); // diagnostic snapshot only
        throw; // retain graph; never infer Native origin or replay FH3 actions
    }
}
} // namespace bsp

#include "bsp/native_particle_bounds_reader.hpp"
#include "bsp/native_particle_text_helpers.hpp"
#include "bsp/native_pooled_text.hpp"
#include "bsp/native_pooled_text_suffix.hpp"
#include <cstdlib>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle bounds reading requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
char* current_text(const void* header) noexcept {
    return *static_cast<char* const volatile*>(header);
}
void store_atof(void* destination, const char* text) {
    // AF4957/49A5/49F5/4A45/4A95 return ST0. Do not spill to a C++ double:
    // each native call is followed immediately by FSTP to its actual field.
    using Atof = double (__cdecl*)(const char*);
    Atof const convert = &std::atof;
    __asm {
        push text
        call convert
        mov ecx, destination
        fstp dword ptr [ecx]
        add esp, 4
    }
}
struct BoundsUnwind {
    NativePooledTextStorage& line;
    NativePooledTextStorage& first_scan;
    NativePooledTextStorage& name;
    NativePooledTextStorage& suffix;
    NativeStringRawPoolContext& strings;
    Word first_scan_flags = 0;
    int state = 0;
    ~BoundsUnwind() noexcept {
        // DF2A64: 3->2 suffix, 2->0 name, 1->0 conditional first token,
        // 0->-1 line. No unwind ownership of the second scan token or any
        // keyword/coordinate token. A secondary getter exception terminates.
        if (state == 3) {
            destroy_native_pooled_text_00aee2a0(&suffix, strings);
            state = 2;
        }
        if (state == 2) {
            destroy_native_pooled_text_00aee2a0(&name, strings);
            state = 0;
        }
        if (state == 1) {
            if (first_scan_flags & 1u) {
                first_scan_flags &= ~1u;
                destroy_native_pooled_text_00aee2a0(&first_scan, strings);
            }
            state = 0;
        }
        if (state == 0) destroy_native_pooled_text_00aee2a0(&line, strings);
    }
};
}

void read_native_particle_bounds_00af4700(void* destination, void* buffer,
    NativeStringRawPoolContext& strings, char* scratch) {
    NativePooledTextStorage line{nullptr};
    NativePooledTextStorage first_scan, name, suffix;
    BoundsUnwind unwind{line, first_scan, name, suffix, strings};
    Word scan_flags = 0;
    for (;;) {
        bool keep_scanning = false;
        NativePooledTextStorage second_scan;
        if (read_native_text_buffer_line_00af5740(buffer, &line, strings, scratch)) {
            const void* const first = get_native_pooled_text_token_00aee3c0(
                &line, &first_scan, 0, strings);
            scan_flags |= 1u;
            const bool present = current_text(first) != nullptr;
            unwind.state = 1;
            unwind.first_scan_flags = scan_flags;
            if (present) {
                const void* const second = get_native_pooled_text_token_00aee3c0(
                    &line, &second_scan, 0, strings);
                scan_flags |= 2u; // Native EBX, not the stored unwind flag.
                keep_scanning = _stricmp(current_text(second), "BoundSphere") != 0;
            }
        }
        if (scan_flags & 2u) {
            scan_flags &= ~2u;
            unwind.first_scan_flags = scan_flags;
            destroy_native_pooled_text_00aee2a0(&second_scan, strings);
        }
        unwind.state = 0; // BEFORE the first token's normal pool return.
        if (scan_flags & 1u) {
            scan_flags &= ~1u;
            destroy_native_pooled_text_00aee2a0(&first_scan, strings);
        }
        if (!keep_scanning) break;
    }

    // Even EOF/null-token exit from the first scan enters this search and
    // performs the subsequent body read. There is no early EOF return here.
    while (read_native_text_buffer_line_00af5740(buffer, &line, strings, scratch)) {
        if (_stricmp(current_text(&line), "{") == 0) break;
    }
    bool have_line = read_native_text_buffer_line_00af5740(buffer, &line, strings, scratch);
    while (have_line) {
        if (_stricmp(current_text(&line), "}") == 0) break;
        if (_stricmp(current_text(&line), "") != 0) {
            NativePooledTextStorage keyword;
            const void* const token = get_native_pooled_text_token_00aee3c0(
                &line, &keyword, 0, strings);
            const bool parameter = _stricmp(current_text(token), "Param") == 0;
            destroy_native_pooled_text_00aee2a0(&keyword, strings);
            if (parameter && count_native_particle_token_fields_00af3e90(&line) >= 3) {
                get_native_pooled_text_token_00aee3c0(&line, &name, 1, strings);
                unwind.state = 2;
                get_native_pooled_text_suffix_00af44c0(&line, &suffix, 2, strings);
                unwind.state = 3;
                if (_stricmp(current_text(&name), "Coords") == 0) {
                    NativePooledTextStorage values[5];
                    for (std::int32_t index = 0; index != 5; ++index) {
                        const void* const value = get_native_pooled_text_token_00aee3c0(
                            &suffix, &values[index], index, strings);
                        void* const field = reinterpret_cast<void*>(
                            reinterpret_cast<Word>(destination) + 0x7cu +
                            static_cast<Word>(index) * 4u);
                        store_atof(field, current_text(value));
                        destroy_native_pooled_text_00aee2a0(&values[index], strings);
                    }
                }
                // Normal native cleanup captures each pointer before dropping
                // its state; it does not regain ownership if that return fails.
                char* const suffix_bytes = current_text(&suffix);
                unwind.state = 2;
                if (suffix_bytes) release_native_pooled_text_bytes_00aee1e0(suffix_bytes, strings);
                char* const name_bytes = current_text(&name);
                suffix.data = nullptr;
                unwind.state = 0;
                if (name_bytes) release_native_pooled_text_bytes_00aee1e0(name_bytes, strings);
                name.data = nullptr;
            }
        }
        have_line = read_native_text_buffer_line_00af5740(buffer, &line, strings, scratch);
    }
    char* const line_bytes = current_text(&line);
    unwind.state = -1;
    if (line_bytes) release_native_pooled_text_bytes_00aee1e0(line_bytes, strings);
}
} // namespace bsp

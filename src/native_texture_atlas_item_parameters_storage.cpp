#include "bsp/native_texture_atlas_item_parameters_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_particle_text_helpers.hpp"
#include "bsp/wire_goal4_chain515_switches.hpp"
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw atlas item parameters require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(NativePooledTextStorage) == 4);
static_assert(offsetof(NativeTextureAtlasItemParameterFrame, parameter) == 4);
static_assert(offsetof(NativeTextureAtlasItemParameterFrame, first_token) == 8);
static_assert(offsetof(NativeTextureAtlasItemParameterFrame, value_u2) == 0x14);
static_assert(offsetof(NativeTextureAtlasItemParameterFrame, value_v2) == 0x18);
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value)); }
char* current(const NativePooledTextStorage& header) noexcept { return header.data; }
void return_captured(char* text, NativeStringRawPoolContext& strings) {
    if (text) release_native_pooled_text_bytes_00aee1e0(text, strings);
}
void store_atof(void* destination, const char* text) {
    // AEE758/7DF/86B/904 -> their immediate FSTP actual item field.
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
void* field(void* raw, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(raw) + offset);
}
} // namespace

NativeTextureAtlasItemParameterContext::NativeTextureAtlasItemParameterContext(
    const game::GameNativeReadOnlyData& data, NativeStringRawPoolContext& strings,
    char* shared_line_scratch)
    : strings_(strings), scratch_(shared_line_scratch), literals_{
        static_cast<const char*>(data.data_at(0x00ce4d38, 2)),
        static_cast<const char*>(data.data_at(0x00ce4cd4, 2)),
        static_cast<const char*>(data.data_at(0x00ce3a0c, 1)),
        static_cast<const char*>(data.data_at(0x00d5d78c, 6)),
        static_cast<const char*>(data.data_at(0x00d5c8a0, 3)),
        static_cast<const char*>(data.data_at(0x00d5d788, 3)),
        static_cast<const char*>(data.data_at(0x00d5d784, 3)),
        static_cast<const char*>(data.data_at(0x00d5d780, 3))},
      packed_scale_00ce5f90_(*static_cast<const volatile double*>(data.data_at(0x00ce5f90, 8))) {}

std::uint8_t read_native_texture_atlas_item_parameters_00aee620(void* actual_item,
    volatile Word& actual_buffer_home, NativeTextureAtlasItemParameterFrame& f,
    NativeTextureAtlasItemParameterContext& c) {
    if (f.phase != NativeTextureAtlasItemParameterPhase::fresh)
        throw std::logic_error("raw atlas item parameter Frame cannot replay");
    f.phase = NativeTextureAtlasItemParameterPhase::entered;
    f.actual_item = actual_item;
    f.line.data = nullptr;                                      // AEE63F
    f.native_state = 0;                                        // AEE643
    try {
        void* seek_receiver;
        for (;;) {
            seek_receiver = pointer(actual_buffer_home);       // AEE647
            f.active_site = 0x00aee652;
            if (!read_native_text_buffer_line_00af5740(seek_receiver, &f.line, c.strings_, c.scratch_)) break;
            f.active_site = 0x00aee665;
            if (_stricmp(current(f.line), c.literals_[0]) == 0) break;
        }
        f.active_site = 0x00aee678;
        bool have_line = read_native_text_buffer_line_00af5740(seek_receiver, &f.line, c.strings_, c.scratch_);
        while (have_line) {
            f.active_site = 0x00aee690;
            if (_stricmp(current(f.line), c.literals_[1]) == 0) break;
            f.active_site = 0x00aee6aa;
            if (_stricmp(current(f.line), c.literals_[2]) != 0) {
                f.active_site = 0x00aee6c4;
                auto* first = static_cast<NativePooledTextStorage*>(get_native_pooled_text_token_00aee3c0(
                    &f.line, &f.first_token, 0, c.strings_));
                f.active_site = 0x00aee6d1;
                const bool is_parameter = _stricmp(current(*first), c.literals_[3]) == 0;
                f.active_site = 0x00aee702;
                return_captured(current(f.first_token), c.strings_);
                f.first_token.data = nullptr;                  // AEE710
                if (is_parameter) {
                    f.active_site = 0x00aee725;
                    get_native_pooled_text_token_00aee3c0(&f.line, &f.parameter, 1, c.strings_);
                    f.native_state = 1;                        // AEE734
                    f.active_site = 0x00aee739;
                    if (_stricmp(current(f.parameter), c.literals_[4]) == 0) {
                        f.active_site = 0x00aee750;
                        auto* value = static_cast<NativePooledTextStorage*>(get_native_pooled_text_token_00aee3c0(
                            &f.line, &f.value_u1, 2, c.strings_));
                        f.active_site = 0x00aee758;
                        if constexpr (kWireGoal4Chain515Bound) {
                            // bsp.py show 00AEDF60 --asm: dereference the actual
                            // token cell, CRT atof, FSTP32/FLD32. AEE758's old
                            // inline conversion has the same binary32 value;
                            // publish it to the SAME item before token release.
                            *static_cast<float*>(field(actual_item, 0x14)) =
                                parse_native_particle_token_float_00aedf60(value);
                        } else {
                        store_atof(field(actual_item, 0x14), current(*value));
                        }
                        f.active_site = 0x00aee782;
                        return_captured(current(f.value_u1), c.strings_);
                        f.value_u1.data = nullptr;              // AEE794
                        f.native_state = 0;                    // AEE798
                        f.active_site = 0x00aee8c9;
                        return_captured(current(f.parameter), c.strings_);
                        f.parameter.data = nullptr;             // AEE8D5
                    } else {
                        f.active_site = 0x00aee7c0;
                        if (_stricmp(current(f.parameter), c.literals_[5]) == 0) {
                            f.active_site = 0x00aee7d7;
                            auto* value = static_cast<NativePooledTextStorage*>(get_native_pooled_text_token_00aee3c0(
                                &f.line, &f.value_v1, 2, c.strings_));
                            f.active_site = 0x00aee7df;
                            if constexpr (kWireGoal4Chain515Bound) {
                                *static_cast<float*>(field(actual_item, 0x18)) =
                                    parse_native_particle_token_float_00aedf60(value);
                            } else {
                            store_atof(field(actual_item, 0x18), current(*value));
                            }
                            f.active_site = 0x00aee809;
                            return_captured(current(f.value_v1), c.strings_);
                            f.value_v1.data = nullptr;          // AEE81B
                            f.native_state = 0;                // AEE81F
                            f.active_site = 0x00aee8c9;
                            return_captured(current(f.parameter), c.strings_);
                            f.parameter.data = nullptr;
                        } else {
                            f.active_site = 0x00aee848;
                            if (_stricmp(current(f.parameter), c.literals_[6]) == 0) {
                                f.active_site = 0x00aee863;
                                auto* value = static_cast<NativePooledTextStorage*>(get_native_pooled_text_token_00aee3c0(
                                    &f.line, &f.value_u2.token, 2, c.strings_));
                                f.active_site = 0x00aee86b;
                                if constexpr (kWireGoal4Chain515Bound) {
                                    *static_cast<float*>(field(actual_item, 0x1c)) =
                                        parse_native_particle_token_float_00aedf60(value);
                                } else {
                                store_atof(field(actual_item, 0x1c), current(*value));
                                }
                                f.active_site = 0x00aee895;
                                return_captured(current(f.value_u2.token), c.strings_);
                                f.value_u2.token.data = nullptr; // AEE8A7
                                f.native_state = 0;             // AEE8AB
                                f.active_site = 0x00aee8c9;
                                return_captured(current(f.parameter), c.strings_);
                                f.parameter.data = nullptr;
                            } else {
                                f.active_site = 0x00aee8e5;
                                if (_stricmp(current(f.parameter), c.literals_[7]) == 0) {
                                    f.active_site = 0x00aee8fc;
                                    auto* value = static_cast<NativePooledTextStorage*>(get_native_pooled_text_token_00aee3c0(
                                        &f.line, &f.value_v2, 2, c.strings_));
                                    f.active_site = 0x00aee904;
                                    if constexpr (kWireGoal4Chain515Bound) {
                                        *static_cast<float*>(field(actual_item, 0x20)) =
                                            parse_native_particle_token_float_00aedf60(value);
                                    } else {
                                    store_atof(field(actual_item, 0x20), current(*value));
                                    }
                                    f.active_site = 0x00aee913;
                                    destroy_native_pooled_text_00aee2a0(&f.value_v2, c.strings_);
                                }
                                f.native_state = 0;             // AEE91C
                                f.active_site = 0x00aee921;
                                destroy_native_pooled_text_00aee2a0(&f.parameter, c.strings_);
                            }
                        }
                    }
                }
            }
            f.active_site = 0x00aee92f;
            have_line = read_native_text_buffer_line_00af5740(pointer(actual_buffer_home),
                &f.line, c.strings_, c.scratch_);
        }
        // The U2 token lifetime has ended before any final packing. A trivial
        // active-member switch writes no byte; the SAME four-byte home follows.
        ::new (static_cast<void*>(&f.value_u2.conversion)) Word;
        Word* const conversion_home = &f.value_u2.conversion;
        volatile Word* const argument_home = &actual_buffer_home;
        const volatile double* const scale = &c.packed_scale_00ce5f90_;
        f.active_site = 0x00aee93d;
        __asm {
            mov edi, actual_item
            mov esi, argument_home
            mov ebx, conversion_home
            mov edx, scale
            fld dword ptr [edi + 14h]
            fstp dword ptr [esi]
            fld dword ptr [esi]
            fld st(0)
            fld qword ptr [edx]
            fmul st(1), st(0)
            fnstcw word ptr [esi]
            movzx eax, word ptr [esi]
            fxch st(1)
            or eax, 0c00h
            mov dword ptr [ebx], eax
            fldcw word ptr [ebx]
            fistp dword ptr [ebx]
            movzx eax, word ptr [ebx]
            mov word ptr [edi + 24h], ax
            fldcw word ptr [esi]
            fld dword ptr [edi + 18h]
            fstp dword ptr [esi]
            fld dword ptr [esi]
            fld st(0)
            fmul st(0), st(2)
            fnstcw word ptr [esi]
            movzx eax, word ptr [esi]
            or eax, 0c00h
            mov dword ptr [ebx], eax
            fldcw word ptr [ebx]
            fistp dword ptr [ebx]
            movzx ecx, word ptr [ebx]
            mov word ptr [edi + 26h], cx
            fldcw word ptr [esi]
            fld dword ptr [edi + 1ch]
            fstp dword ptr [esi]
            fld dword ptr [esi]
            fld st(0)
            fmul st(0), st(3)
            fnstcw word ptr [esi]
            movzx eax, word ptr [esi]
            or eax, 0c00h
            mov dword ptr [ebx], eax
            fldcw word ptr [ebx]
            fistp dword ptr [ebx]
            movzx edx, word ptr [ebx]
            mov word ptr [edi + 28h], dx
            fldcw word ptr [esi]
            fld dword ptr [edi + 20h]
            fstp dword ptr [esi]
            fld dword ptr [esi]
            fnstcw word ptr [esi]
            fld st(0)
            movzx eax, word ptr [esi]
            fmul st(0), st(4)
            or eax, 0c00h
            mov dword ptr [ebx], eax
            fldcw word ptr [ebx]
            fistp dword ptr [ebx]
            movzx eax, word ptr [ebx]
            mov word ptr [edi + 2ah], ax
            fldcw word ptr [esi]
            fnstcw word ptr [esi]
            fsubrp st(2), st(0)
            movzx eax, word ptr [esi]
            fxch st(1)
            or eax, 0c00h
            mov dword ptr [ebx], eax
            fmul st(0), st(2)
            fldcw word ptr [ebx]
            fistp dword ptr [ebx]
            movzx ecx, word ptr [ebx]
            mov word ptr [edi + 2ch], cx
            fldcw word ptr [esi]
            fnstcw word ptr [esi]
            fsubrp st(2), st(0)
            movzx eax, word ptr [esi]
            or eax, 0c00h
            mov dword ptr [ebx], eax
            fmulp st(1), st(0)
            fldcw word ptr [ebx]
            fistp dword ptr [ebx]
            movzx edx, word ptr [ebx]
            mov word ptr [edi + 2eh], dx
            fldcw word ptr [esi]
        }
        f.native_state = -1;                                   // AEEA7B
        f.active_site = 0x00aeeaa2;
        return_captured(current(f.line), c.strings_);            // no line header clear
        f.phase = NativeTextureAtlasItemParameterPhase::complete;
        return 1;
    } catch (...) {
        f.failure = std::current_exception();
        f.phase = NativeTextureAtlasItemParameterPhase::failed;
        throw;
    }
}
} // namespace bsp

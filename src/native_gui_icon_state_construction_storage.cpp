#include "bsp/native_gui_icon_state_construction_storage.hpp"
#include "bsp/native_gui_icon_placeholder_storage.hpp"
#include "bsp/native_gui_icon_state_uv_storage.hpp"
#include "bsp/native_gui_texture_resolver_storage.hpp"
#include "bsp/native_platform_focus_owners.hpp"
#include "bsp/native_string_raw_resize_exact.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include <cstddef>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw Source supplier requires MSVC Win32 inline assembly.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(std::is_trivial<NativeGuiIconStateStorage>::value);
static_assert(sizeof(NativeGuiIconStateStorage) == 0x40);
Word read_word(const void* address, Word byte_offset = 0) {
    Word value;
    __asm {
        mov eax, address
        mov edx, byte_offset
        mov eax, [eax + edx]
        mov value, eax
    }
    return value;
}
void write_word(void* address, Word byte_offset, Word value) {
    __asm {
        mov eax, address
        mov edx, byte_offset
        mov ecx, value
        mov [eax + edx], ecx
    }
}
void* pointer(Word bits) { return reinterpret_cast<void*>(bits); }
void* at(void* address, Word byte_offset) {
    return static_cast<std::byte*>(address) + byte_offset;
}
}

Word add_native_gui_icon_state_storage_00ab66c0(void* actual_icon138,
    volatile Word (&actual_argument_homes)[4],
    NativeGuiIconStateConstructionStorageFrame& frame,
    NativeGuiIconStateConstructionStorageContext& context) {
    void* const captured_icon = actual_icon138;
    volatile Word* const homes = actual_argument_homes;
    Word* const local_homes = frame.local_0c;
    void* const temporary = &frame.temporary_2c;
    Word* const state_home = &frame.state_74;
    const volatile float* const one = &context.actual_one_00d7a24c;
    Word captured_name, self_name;
    __asm {
        mov edi, captured_icon
        mov esi, homes
        mov edx, local_homes
        mov ecx, temporary
        mov ebx, state_home
        mov dword ptr [ebx], -1

        mov eax, [esi + 8]
        test eax, eax
        jnz construct_pivot_supplied
        movss xmm0, [edi + 18h]
        movss [edx + 8], xmm0
        movss xmm0, [edi + 1ch]
        jmp construct_pivot_done
    construct_pivot_supplied:
        movss xmm0, [eax]
        movss [edx + 8], xmm0
        movss xmm0, [eax + 4]
    construct_pivot_done:
        mov eax, [esi + 0ch]
        test eax, eax
        movss [edx + 0ch], xmm0
        jnz construct_size_supplied
        movss xmm0, [edi + 20h]
        movss [edx], xmm0
        movss xmm0, [edi + 24h]
        jmp construct_size_done
    construct_size_supplied:
        movss xmm0, [eax]
        movss [edx], xmm0
        movss xmm0, [eax + 4]
    construct_size_done:
        mov eax, one
        movss xmm1, [eax]
        movss [edx + 4], xmm0
        xorps xmm0, xmm0
        mov dword ptr [ecx], 0
        mov dword ptr [ecx + 4], 0
        mov dword ptr [ecx + 8], 0
        movss [ecx + 0ch], xmm0
        movss [ecx + 10h], xmm0
        movss [ecx + 14h], xmm1
        movss [ecx + 18h], xmm1
        movss [ecx + 1ch], xmm0
        movss [ecx + 20h], xmm0
        movss [ecx + 24h], xmm1
        movss [ecx + 28h], xmm1
        movss [ecx + 2ch], xmm0
        movss [ecx + 30h], xmm0
        movss [ecx + 34h], xmm0
        movss [ecx + 38h], xmm0
        mov byte ptr [ecx + 3ch], 0
        mov byte ptr [ecx + 3dh], 0
        mov esi, [esi]
        mov captured_name, esi
        lea eax, [ecx + 4]
        cmp eax, esi
        mov dword ptr [ebx], 0
        movss [edx + 10h], xmm0
        movss [edx + 14h], xmm0
        movss [edx + 18h], xmm1
        movss [edx + 1ch], xmm1
        sete al
        movzx eax, al
        mov self_name, eax
    }
    Word captured_result;
    try {
        if (!self_name) {
            const Word request = read_word(pointer(captured_name));
            write_word(&frame.preserve_home, 0, 1);
            resize_native_string_header_0041dd40_exact(at(temporary, 4),
                context.growth.storage.strings, request, &frame.preserve_home,
                context.growth.feature_word_0109eea4);
            if (read_word(pointer(captured_name))) {
                const Word bytes = read_word(temporary, 4);
                const Word source = read_word(pointer(captured_name), 4);
                const Word destination = read_word(temporary, 8);
                copy_native_crt_bytes_00bf7680(pointer(destination), pointer(source),
                    bytes, context.growth.feature_word_0109eea4);
            }
        }
        Word delayed;
        __asm {
            mov edi, captured_icon
            mov esi, homes
            mov edx, local_homes
            mov ebx, temporary
            cmp byte ptr [edi + 101h], 0
            setne al
            movzx ecx, al
            mov delayed, ecx
            mov eax, [esi + 4]
            mov ecx, [eax]
            mov esi, [eax + 4]
            movss xmm0, [edx + 8]
            mov [ebx + 1ch], ecx
            mov ecx, [eax + 8]
            mov [ebx + 20h], esi
            mov esi, [eax + 0ch]
            movss [ebx + 2ch], xmm0
            movss xmm0, [edx + 0ch]
            movss [ebx + 30h], xmm0
            mov [ebx + 24h], ecx
            mov [ebx + 28h], esi
        }
        Word captured_size_x;
        if (!delayed) {
            __asm {
                mov ecx, temporary
                mov byte ptr [ecx + 3ch], 0
            }
            (void)get_native_gui_manager_004c12b0(context.gui);
            Word* const resolver_homes = frame.resolver_homes;
            __asm {
                mov eax, resolver_homes
                mov edx, local_homes
                fld1
                fstp dword ptr [eax + 0ch]
                mov [eax + 8], edx
                lea ecx, [edx + 10h]
                mov [eax + 4], ecx
                mov ecx, captured_name
                mov [eax], ecx
            }
            void* const texture = resolve_native_gui_texture_storage_00aa2660(
                frame.resolver_homes, context.resolver);
            Word* const uv_homes = frame.uv_homes;
            __asm {
                mov edx, local_homes
                mov ebx, temporary
                mov esi, uv_homes
                mov ecx, [edx + 10h]
                mov eax, [edx + 14h]
                mov edi, texture
                mov [ebx], edi
                mov [esi], ecx
                mov ecx, [edx + 18h]
                mov [esi + 4], eax
                mov eax, [edx + 1ch]
                mov [esi + 8], ecx
                mov [esi + 0ch], eax
            }
            set_native_gui_icon_state_uv_storage_00ab1680(temporary,
                frame.uv_homes, context.actual_one_00d7a24c);
            Word needs_size;
            __asm {
                mov edx, local_homes
                xor ecx, ecx
                movss xmm0, [edx]
                xorps xmm1, xmm1
                ucomiss xmm0, xmm1
                lahf
                test ah, 44h
                jp construct_size_present
                movss xmm2, [edx + 4]
                ucomiss xmm2, xmm1
                lahf
                test ah, 44h
                jp construct_size_present
                mov ecx, 1
            construct_size_present:
                mov needs_size, ecx
                movd captured_size_x, xmm0
            }
            if (needs_size) {
                const Word texture_bits = read_word(temporary);
                write_word(&frame.native_size_uv_home, 0,
                    reinterpret_cast<Word>(at(temporary, 0x0c)));
                void* const output = write_native_gui_icon_native_size_storage_00ab17b0(
                    local_homes + 2, pointer(texture_bits), frame.native_size_uv_home,
                    frame.native_size, context.resolver.dimensions, context.resolver.constants);
                __asm {
                    mov eax, output
                    mov edx, local_homes
                    movss xmm0, [eax]
                    movss [edx], xmm0
                    movss xmm1, [eax + 4]
                    movss [edx + 4], xmm1
                    movd captured_size_x, xmm0
                }
            }
        } else {
            set_native_gui_icon_placeholder_storage_00ab3b30(temporary,
                frame.placeholder, context.placeholder);
            __asm {
                mov edx, local_homes
                movss xmm0, [edx]
                movd captured_size_x, xmm0
            }
        }
        void* captured_vector;
        Word* const prototype_home = &frame.prototype_home;
        __asm {
            mov ecx, temporary
            mov edx, local_homes
            mov ebx, prototype_home
            movd xmm0, captured_size_x
            movss [ecx + 34h], xmm0
            movss xmm0, [edx + 4]
            mov esi, captured_icon
            add esi, 0f0h
            mov captured_vector, esi
            mov [ebx], ecx
            movss [ecx + 38h], xmm0
        }
        append_native_gui_icon_state_storage_00ab63a0(captured_vector,
            frame.prototype_home, frame.append, context.growth);
        __asm {
            mov esi, captured_vector
            mov ecx, [esi + 4]
            test ecx, ecx
            jnz construct_nonempty
            xor eax, eax
            jmp construct_size_captured
        construct_nonempty:
            mov eax, [esi + 8]
            sub eax, ecx
            sar eax, 6
        construct_size_captured:
            lea esi, [eax - 1]
            mov captured_result, esi
            mov eax, state_home
            mov dword ptr [eax], -1
        }
    } catch (...) {
        destroy_native_gui_icon_state_00ab3280(temporary, context.growth.storage);
        throw;
    }
    destroy_native_gui_icon_state_00ab3280(temporary, context.growth.storage);
    return captured_result;
}
} // namespace bsp

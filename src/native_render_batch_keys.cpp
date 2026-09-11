#include "bsp/native_render_batch_keys.hpp"
#include "bsp/native_render_batch_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render batch keys require MSVC Win32 x87 assembly.
#endif

namespace bsp {

// Full original BF7456..BF74CB. Keep the two stores on the special path:
// they pop both live values and retain native conversion/status side effects.
__declspec(naked) std::uint64_t __cdecl native_x87_truncate_st0_00bf7456() noexcept {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 20h
        and esp, 0fffffff0h
        fld st(0)
        fst dword ptr [esp + 18h]
        fistp qword ptr [esp + 10h]
        fild qword ptr [esp + 10h]
        mov edx, dword ptr [esp + 18h]
        mov eax, dword ptr [esp + 10h]
        test eax, eax
        jz zero_low
    residual:
        fsubp st(1), st(0)
        test edx, edx
        jns nonnegative
        fstp dword ptr [esp]
        mov ecx, dword ptr [esp]
        xor ecx, 80000000h
        add ecx, 7fffffffh
        adc eax, 0
        mov edx, dword ptr [esp + 14h]
        adc edx, 0
        jmp done
    nonnegative:
        fstp dword ptr [esp]
        mov ecx, dword ptr [esp]
        add ecx, 7fffffffh
        sbb eax, 0
        mov edx, dword ptr [esp + 14h]
        sbb edx, 0
        jmp done
    zero_low:
        mov edx, dword ptr [esp + 14h]
        test edx, 7fffffffh
        jnz residual
        fstp dword ptr [esp + 18h]
        fstp dword ptr [esp + 18h]
    done:
        leave
        ret
    }
}

// Moved without floating-instruction changes from gui_group_bounds.cpp so
// GUI radius and frontend progress share one actual mode-dependent converter.
__declspec(naked) std::int32_t __fastcall native_crt_truncate_st0_00bf7420(
    const volatile std::uint32_t*) noexcept {
    __asm {
        cmp dword ptr [ecx], 0
        jz x87_fallback
        push ebp
        mov ebp, esp
        sub esp, 8
        and esp, 0fffffff8h
        fstp qword ptr [esp]
        cvttsd2si eax, qword ptr [esp]
        leave
        ret
    x87_fallback:
        jmp native_x87_truncate_st0_00bf7456
    }
}

__declspec(naked) std::uint32_t __fastcall
native_render_entry_material_depth_less_00b51ab0(const void*, const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 4]
        mov eax, dword ptr [eax + 20h]
        push esi
        mov esi, dword ptr [eax + 7ch]
        mov eax, dword ptr [edx + 4]
        mov eax, dword ptr [eax + 20h]
        mov eax, dword ptr [eax + 7ch]
        mov eax, dword ptr [eax + 0b0h]
        mov esi, dword ptr [esi + 0b0h]
        cmp esi, eax
        jnz unequal
        fld dword ptr [edx + 14h]
        fld dword ptr [ecx + 14h]
        fcomip st(0), st(1)
        fstp st(0)
        jbe false_result
        mov eax, 1
        pop esi
        ret
    false_result:
        xor eax, eax
        pop esi
        ret
    unequal:
        xor ecx, ecx
        cmp esi, eax
        setl cl
        mov al, cl
        pop esi
        ret
    }
}

namespace {
template<class T> T read_field(std::uintptr_t base, std::uint32_t offset) noexcept {
    return *reinterpret_cast<const volatile T*>(base + offset);
}
}

void prepare_native_render_batch_keys_00b51df0(NativeRenderBatchStorage& batch) noexcept {
    const auto owner = reinterpret_cast<std::uintptr_t>(&batch);
    std::int32_t index = 0;
    if (read_field<std::int32_t>(owner, 0x10) <= 0) return;
    do {
        const auto slots = read_field<std::uintptr_t>(owner, 0x0c);
        const auto entry = read_field<std::uintptr_t>(slots,
            static_cast<std::uint32_t>(index) * 4u);
        const auto section = read_field<std::uintptr_t>(entry, 4);
        const auto material = read_field<std::uintptr_t>(section, 0x20);
        const auto material_count = read_field<std::int16_t>(material, 0x34);
        const auto effect = read_field<std::uintptr_t>(material, 0x7c);
        const auto effect_value = read_field<std::uint32_t>(effect, 0xb0);
        std::uint32_t texture_value = 0;
        if (material_count > 0) {
            const auto texture = read_field<std::uintptr_t>(material, 0x10);
            if (texture) texture_value = read_field<std::uint32_t>(texture, 0x20);
        }
        const auto prefix_before_byte = ((effect_value & 0x3fu) * 0x1000u
            + (texture_value & 0xfffu)) * 0x100u;
        const auto prefix = prefix_before_byte + read_field<std::uint8_t>(effect, 0xc0);
        // Original second __allmul operand is high20h/low0 = 2^37.
        const std::uint32_t high = prefix << 5;
        std::uint32_t depth_word;
        __asm {
            mov eax, entry
            fld dword ptr [eax + 14h]
            call native_x87_truncate_st0_00bf7456
            mov depth_word, eax
        }
        *reinterpret_cast<volatile std::uint32_t*>(entry + 0x20) = depth_word;
        *reinterpret_cast<volatile std::uint32_t*>(entry + 0x24) = high;
        ++index;
    } while (index < read_field<std::int32_t>(owner, 0x10));
}
} // namespace bsp

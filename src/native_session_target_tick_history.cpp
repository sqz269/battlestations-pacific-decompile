#include "bsp/native_session_target_tick_history.hpp"
#include "bsp/native_render_batch_keys.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Session target tick history reconstruction requires MSVC Win32.
#endif

namespace bsp {
void update_native_session_target_tick_history_007833e0(
    NativeSessionTargetStorage* target, std::uint32_t original_tick_word,
    const NativeSessionTargetTickHistoryBindings& bindings) noexcept {
    const volatile std::uint32_t* const step = &bindings.step_f876b0;
    std::uint32_t captured_step;
    bool early_reset;
    __asm {
        mov eax, step
        mov ecx, dword ptr [eax]
        mov captured_step, ecx
        cmp ecx, 10
        setl early_reset
    }
    if (early_reset) {
        std::uint32_t zero_word;
        __asm { fldz }
        __asm { fstp dword ptr zero_word }
        reset_native_session_target_history_00783230(target, zero_word);
        return;
    }
    const volatile float* const time = &bindings.time_f876a8;
    const volatile float* const divisor = &bindings.divisor_d0de84;
    const volatile std::uint32_t* const mode = &bindings.conversion_mode_0109eea4;
    std::uint32_t converted_word;
    __asm {
        mov eax, time
        fld dword ptr [eax]
        mov eax, divisor
        fdiv dword ptr [eax]
        mov ecx, mode
        call native_crt_truncate_st0_00bf7420
        mov converted_word, eax
    }
    std::int32_t sample;
    // Use actual modulo32 SUB/ADD/NEG and signed branches. In particular the
    // two half-range endpoints are retained; narrowing to int16 would differ.
    __asm {
        mov edx, captured_step
        mov ecx, original_tick_word
        and edx, 0ffffh
        sub edx, ecx
        mov eax, converted_word
        and eax, 0ffffh
        cmp edx, 8000h
        jle first_not_above
        sub edx, 10000h
        jmp first_done
    first_not_above:
        cmp edx, 0ffff8000h
        jge first_done
        add edx, 10000h
    first_done:
        sub ecx, eax
        mov eax, ecx
        cmp eax, 8000h
        jle second_not_above
        sub eax, 10000h
        jmp second_done
    second_not_above:
        cmp eax, 0ffff8000h
        jge second_done
        add eax, 10000h
    second_done:
        test edx, edx
        jle first_nonpositive
        neg edx
        mov sample, edx
        jmp sample_done
    first_nonpositive:
        test eax, eax
        jle neither_positive
        mov sample, eax
        jmp sample_done
    neither_positive:
        mov sample, 0
    sample_done:
    }
    append_native_session_target_history_007831e0(target, sample);
}
} // namespace bsp

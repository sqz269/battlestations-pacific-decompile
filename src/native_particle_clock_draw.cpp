#include "bsp/native_particle_clock_draw.hpp"
#include "bsp/native_texture_source_time.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle-clock draw requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(float) == 4);

__declspec(naked) void __fastcall set_native_particle_clock_time_00b19a10(
    void*, void*, float) {
    __asm {
        movss xmm0, dword ptr [esp + 4]
        push esi
        push edi
        mov edi, ecx
        mov esi, dword ptr [edi + 8]
        movss dword ptr [edi + 18h], xmm0
        mov eax, dword ptr [edi + 0Ch]
        imul eax, eax, 2Ch
        add eax, esi
        cmp esi, eax
        je complete
        mov edi, edi
    next_record:
        mov ecx, dword ptr [esi + 28h]
        fld dword ptr [esp + 0Ch]
        mov edx, dword ptr [ecx]
        mov eax, dword ptr [edx + 18h]
        push ecx
        fstp dword ptr [esp]
        call eax
        mov ecx, dword ptr [edi + 0Ch]
        imul ecx, ecx, 2Ch
        add ecx, dword ptr [edi + 8]
        add esi, 2Ch
        cmp esi, ecx
        jne next_record
    complete:
        pop edi
        pop esi
        ret 4
    }
}


namespace {
using ClockWord = unsigned int;
static_assert(sizeof(ClockWord) == 4);

__forceinline ClockWord load_clock_word(const void* storage, ClockWord byte_offset) noexcept {
    ClockWord value;
    __asm {
        mov eax, storage
        add eax, byte_offset
        mov eax, dword ptr [eax]
        mov value, eax
    }
    return value;
}
} // namespace

void set_native_particle_clock_time_00b19a10(void* actual_clock,
    float milliseconds, NativeParticleClockDrawContext& context) {
    ClockWord current_record;
    ClockWord count;
    __asm {
        mov ecx, actual_clock
        movss xmm0, milliseconds
        mov eax, dword ptr [ecx + 8]
        mov current_record, eax
        movss dword ptr [ecx + 18h], xmm0
        mov eax, dword ptr [ecx + 0Ch]
        mov count, eax
    }
    ClockWord end = current_record + count * 0x2cu;
    if (current_record == end) return;
    do {
        void* const actual_record = reinterpret_cast<void*>(current_record);
        const auto* const views = &context;
        void* captured_sink;
        ClockWord captured_target = 0;
        ClockWord unsupported = 0;
        float spilled_frame;
        __asm {
            mov eax, actual_record
            mov ecx, dword ptr [eax + 28h]
            mov captured_sink, ecx
            fld milliseconds
            mov edx, dword ptr [ecx]
            mov eax, views
            cmp edx, 00D64478h
            je draw_profile_caustics
            cmp edx, 00D644B4h
            je draw_profile_shore
            cmp edx, 00D79B54h
            je draw_profile_base
            jmp draw_missing
        draw_profile_caustics:
            mov eax, dword ptr [eax]
            jmp draw_read_slot
        draw_profile_shore:
            mov eax, dword ptr [eax + 4]
            jmp draw_read_slot
        draw_profile_base:
            mov eax, dword ptr [eax + 8]
        draw_read_slot:
            test eax, eax
            je draw_missing
            mov eax, dword ptr [eax + 18h]
            mov captured_target, eax
            jmp draw_spill
        draw_missing:
            mov unsupported, 1
        draw_spill:
            fstp spilled_frame
        }
        // No external call/FP work between native FLD and FSTP. This is a
        // regular C++ function, so explicit source errors have an EH boundary.
        if (unsupported != 0)
            throw std::invalid_argument("unsupported current particle-clock time sink profile/view");
        if (captured_target != 0x00c302a0u)
            throw std::invalid_argument("unsupported current particle-clock time sink slot18");
        set_native_texture_source_time_00c302a0(captured_sink,
            context.actual_milliseconds_scale_00ce47a0, spilled_frame);
        const ClockWord current_count = load_clock_word(actual_clock, 0xc);
        const ClockWord current_data = load_clock_word(actual_clock, 8);
        current_record += 0x2cu;
        end = current_data + current_count * 0x2cu;
    } while (current_record != end);
}

} // namespace bsp

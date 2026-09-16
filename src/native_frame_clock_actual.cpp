#include "bsp/native_frame_clock_actual.hpp"
#include "bsp/frame_clock.hpp"
#include "bsp/native_renderer_control_worker.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstring>
#include <exception>
namespace bsp {
namespace {
using Update = void (__fastcall*)(void*);
using Sample = ClockTimestamp* (__fastcall*)(const void*, void*, ClockTimestamp*);
std::uint32_t slot(const void* clock, const NativeFrameClockActualContext* context,
                   std::size_t index) {
    if (!context || !context->profile_d68d50 ||
        *static_cast<const volatile std::uint32_t*>(clock)!=0x00d68d50u)
        std::terminate();
    return context->profile_d68d50[index];
}
Update __fastcall resolve_update(const void* clock, const NativeFrameClockActualContext* context) {
    if (slot(clock,context,2)!=0x00bedc30u) std::terminate();
    return update_native_frame_clock_00bedc30;
}
Sample __fastcall resolve_sample(const void* clock, const NativeFrameClockActualContext* context) {
    if (slot(clock,context,8)!=0x00bee080u) std::terminate();
    return sample_native_frame_clock_00bee080;
}
// Private ABI bridges: all callers below use separate local output storage.
// Copying into real timestamp objects avoids treating the raw allocation as a
// semantic FrameClock. The existing arithmetic retains modulo products and
// signed division (including the original divide fault); no success bool.
void* __fastcall subtract_raw(const void* left, void*,
                            void* destination, const void* right) noexcept {
    ClockTimestamp l, r, result;
    std::memcpy(&l,left,16); std::memcpy(&r,right,16);
    subtract_timestamp_00530890(result,l,r);
    std::memcpy(destination,&result,16);
    return destination;
}
void* __fastcall add_raw(void* left, void*, const void* right) noexcept {
    ClockTimestamp l, r;
    std::memcpy(&l,left,16); std::memcpy(&r,right,16);
    add_timestamp_00bedb70(l,r);
    // Native add leaves frequency untouched; only these tick words are stored.
    auto* words=static_cast<volatile std::uint32_t*>(left);
    std::uint32_t ticks[2]; std::memcpy(ticks,&l.ticks,8);
    words[0]=ticks[0]; words[1]=ticks[1];
    return left;
}
std::int64_t __stdcall fixed_increment(std::int64_t frequency, std::int32_t milliseconds) noexcept {
    ClockTimestamp left{0,frequency};
    const ClockTimestamp right{milliseconds,1000};
    return add_timestamp_00bedb70(left,right).ticks;
}
} // namespace

// BEDC30: exact original stack layout, DWORD schedule, x87 spill and branch.
// Only the QPC import and five timestamp-subtract calls are relocated.
__declspec(naked) void __fastcall update_native_frame_clock_00bedc30(void*) {
    __asm {
        SUB ESP,0x28 // 00bedc30
        PUSH ESI // 00bedc33
        MOV ESI,ECX // 00bedc34
        CMP byte ptr [ESI + 0x68],0x0 // 00bedc36
        JZ label_00bedc67 // 00bedc3a
        ADD ESI,0x40 // 00bedc3c
        PUSH ESI // 00bedc3f
        LEA EAX,[ESP + 0x10] // 00bedc40
        PUSH EAX // 00bedc44
        MOV ECX,ESI // 00bedc45
        CALL subtract_raw // 00bedc47
        MOV ECX,dword ptr [EAX] // 00bedc4c
        MOV dword ptr [ESI],ECX // 00bedc4e
        MOV EDX,dword ptr [EAX + 0x4] // 00bedc50
        MOV dword ptr [ESI + 0x4],EDX // 00bedc53
        MOV ECX,dword ptr [EAX + 0x8] // 00bedc56
        MOV dword ptr [ESI + 0x8],ECX // 00bedc59
        MOV EDX,dword ptr [EAX + 0xc] // 00bedc5c
        MOV dword ptr [ESI + 0xc],EDX // 00bedc5f
        POP ESI // 00bedc62
        ADD ESP,0x28 // 00bedc63
        RET // 00bedc66
    label_00bedc67: ADD dword ptr [ESI + 0x8],0x1
        MOV EAX,dword ptr [ESI + 0x20] // 00bedc6b
        MOV ECX,dword ptr [ESI + 0x24] // 00bedc6e
        MOV EDX,dword ptr [ESI + 0x28] // 00bedc71
        ADC dword ptr [ESI + 0xc],0x0 // 00bedc74
        CMP byte ptr [ESI + 0x69],0x0 // 00bedc78
        PUSH EBX // 00bedc7c
        PUSH EDI // 00bedc7d
        LEA EDI,[ESI + 0x20] // 00bedc7e
        LEA EBX,[ESI + 0x30] // 00bedc81
        MOV dword ptr [EBX],EAX // 00bedc84
        MOV EAX,dword ptr [EDI + 0xc] // 00bedc86
        MOV dword ptr [EBX + 0x4],ECX // 00bedc89
        MOV dword ptr [EBX + 0x8],EDX // 00bedc8c
        MOV dword ptr [EBX + 0xc],EAX // 00bedc8f
        JNZ label_00bedce7 // 00bedc92
        LEA ECX,[ESP + 0xc] // 00bedc94
        PUSH ECX // 00bedc98
        CALL QueryPerformanceCounter // 00bedc99
        MOV EAX,dword ptr [ESP + 0x10] // 00bedc9f
        MOV ECX,dword ptr [ESI + 0x60] // 00bedca3
        MOV EDX,dword ptr [ESP + 0xc] // 00bedca6
        MOV dword ptr [ESP + 0x18],EAX // 00bedcaa
        MOV dword ptr [ESP + 0x1c],ECX // 00bedcae
        LEA EAX,[ESI + 0x10] // 00bedcb2
        PUSH EAX // 00bedcb5
        LEA ECX,[ESP + 0x28] // 00bedcb6
        MOV dword ptr [ESP + 0x18],EDX // 00bedcba
        MOV EDX,dword ptr [ESI + 0x64] // 00bedcbe
        PUSH ECX // 00bedcc1
        LEA ECX,[ESP + 0x1c] // 00bedcc2
        MOV dword ptr [ESP + 0x28],EDX // 00bedcc6
        CALL subtract_raw // 00bedcca
        MOV EDX,dword ptr [EAX] // 00bedccf
        MOV dword ptr [EDI],EDX // 00bedcd1
        MOV ECX,dword ptr [EAX + 0x4] // 00bedcd3
        MOV dword ptr [EDI + 0x4],ECX // 00bedcd6
        MOV EDX,dword ptr [EAX + 0x8] // 00bedcd9
        MOV dword ptr [EDI + 0x8],EDX // 00bedcdc
        MOV EAX,dword ptr [EAX + 0xc] // 00bedcdf
        MOV dword ptr [EDI + 0xc],EAX // 00bedce2
        JMP label_00bedd37 // 00bedce5
    label_00bedce7: MOV ECX,dword ptr [ESI + 0x70]
        ADD dword ptr [ESI + 0x78],ECX // 00bedcea
        MOV EDX,dword ptr [ESI + 0x74] // 00bedced
        MOV EAX,dword ptr [ESI + 0x78] // 00bedcf0
        ADC dword ptr [ESI + 0x7c],EDX // 00bedcf3
        MOV ECX,dword ptr [ESI + 0x7c] // 00bedcf6
        MOV EDX,dword ptr [ESI + 0x60] // 00bedcf9
        MOV dword ptr [ESP + 0x18],ECX // 00bedcfc
        LEA ECX,[ESI + 0x10] // 00bedd00
        MOV dword ptr [ESP + 0x1c],EDX // 00bedd03
        PUSH ECX // 00bedd07
        MOV dword ptr [ESP + 0x18],EAX // 00bedd08
        MOV EAX,dword ptr [ESI + 0x64] // 00bedd0c
        LEA EDX,[ESP + 0x28] // 00bedd0f
        PUSH EDX // 00bedd13
        LEA ECX,[ESP + 0x1c] // 00bedd14
        MOV dword ptr [ESP + 0x28],EAX // 00bedd18
        CALL subtract_raw // 00bedd1c
        MOV ECX,dword ptr [EAX] // 00bedd21
        MOV dword ptr [EDI],ECX // 00bedd23
        MOV EDX,dword ptr [EAX + 0x4] // 00bedd25
        MOV dword ptr [EDI + 0x4],EDX // 00bedd28
        MOV ECX,dword ptr [EAX + 0x8] // 00bedd2b
        MOV dword ptr [EDI + 0x8],ECX // 00bedd2e
        MOV EDX,dword ptr [EAX + 0xc] // 00bedd31
        MOV dword ptr [EDI + 0xc],EDX // 00bedd34
    label_00bedd37: PUSH EBX
        LEA EAX,[ESP + 0x28] // 00bedd38
        PUSH EAX // 00bedd3c
        MOV ECX,EDI // 00bedd3d
        CALL subtract_raw // 00bedd3f
        MOV ECX,dword ptr [EAX] // 00bedd44
        MOV dword ptr [ESI + 0x40],ECX // 00bedd46
        MOV EDX,dword ptr [EAX + 0x4] // 00bedd49
        MOV dword ptr [ESI + 0x44],EDX // 00bedd4c
        MOV ECX,dword ptr [EAX + 0x8] // 00bedd4f
        MOV dword ptr [ESI + 0x48],ECX // 00bedd52
        MOV EDX,dword ptr [EAX + 0xc] // 00bedd55
        MOV dword ptr [ESI + 0x4c],EDX // 00bedd58
        FILD qword ptr [ESI + 0x40] // 00bedd5b
        FILD qword ptr [ESI + 0x48] // 00bedd5e
        FDIVP ST(1),ST(0) // 00bedd61
        FSTP float ptr [ESP + 0xc] // 00bedd63
        FLD float ptr [ESP + 0xc] // 00bedd67
        FLDZ // 00bedd6b
        FCOMIP ST(0),ST(1) // 00bedd6d
        FSTP ST(0) // 00bedd6f
        JBE label_00beddad // 00bedd71
        MOV EAX,dword ptr [EBX] // 00bedd73
        MOV ECX,dword ptr [EBX + 0x4] // 00bedd75
        MOV EDX,dword ptr [EBX + 0x8] // 00bedd78
        MOV dword ptr [EDI],EAX // 00bedd7b
        MOV EAX,dword ptr [EBX + 0xc] // 00bedd7d
        MOV dword ptr [EDI + 0x4],ECX // 00bedd80
        PUSH EBX // 00bedd83
        LEA ECX,[ESP + 0x28] // 00bedd84
        PUSH ECX // 00bedd88
        MOV dword ptr [EDI + 0x8],EDX // 00bedd89
        MOV ECX,EDI // 00bedd8c
        MOV dword ptr [EDI + 0xc],EAX // 00bedd8e
        CALL subtract_raw // 00bedd91
        MOV EDX,dword ptr [EAX] // 00bedd96
        MOV dword ptr [ESI + 0x40],EDX // 00bedd98
        MOV ECX,dword ptr [EAX + 0x4] // 00bedd9b
        MOV dword ptr [ESI + 0x44],ECX // 00bedd9e
        MOV EDX,dword ptr [EAX + 0x8] // 00bedda1
        MOV dword ptr [ESI + 0x48],EDX // 00bedda4
        MOV EAX,dword ptr [EAX + 0xc] // 00bedda7
        MOV dword ptr [ESI + 0x4c],EAX // 00beddaa
    label_00beddad: POP EDI
        POP EBX // 00beddae
        POP ESI // 00beddaf
        ADD ESP,0x28 // 00beddb0
        RET // 00beddb3
    }
}

// BEDB20: retain the flag-before-QPC store and ignored BOOL. Signed 32-bit
// milliseconds is read after QPC; arithmetic reuses the timestamp provider.
__declspec(naked) void __fastcall enable_fixed_native_frame_clock_00bedb20(void*, void*, std::int32_t) {
    __asm {
        push esi
        mov esi,ecx
        lea eax,[esi+78h]
        push eax
        mov byte ptr [esi+69h],1
        call QueryPerformanceCounter
        mov eax,[esp+8]
        mov ecx,[esi+64h]
        push eax
        mov edx,[esi+60h]
        push ecx
        push edx
        call fixed_increment
        mov [esi+70h],eax
        mov [esi+74h],edx
        pop esi
        ret 4
    }
}
__declspec(naked) void __fastcall disable_fixed_native_frame_clock_00bedb60(void*) noexcept {
    __asm { mov byte ptr [ecx+69h],0
        ret }
}

// The extra saved EBX carries the borrowed context; original locals and all
// native stores keep their relative offsets. Resolve each current slot at the
// original load point, including capture before flag changes / origin+1C.
__declspec(naked) void __fastcall initialize_native_frame_clock_00bedbd0(void*, const NativeFrameClockActualContext*) {
    __asm {
        push ebx
        mov ebx,edx
        sub esp,8
        push esi
        mov esi,ecx
        push edi
        xor eax,eax
        lea edi,[esi+60h]
        push edi
        mov byte ptr [esi+68h],al
        mov [esi+8],eax
        mov [esi+0Ch],eax
        call QueryPerformanceFrequency
        lea eax,[esp+8]
        push eax
        call QueryPerformanceCounter
        mov eax,[esp+8]
        mov ecx,[esp+0Ch]
        mov edx,[edi]
        mov edi,[edi+4]
        mov [esi+10h],eax
        mov [esi+14h],ecx
        mov [esi+18h],edx
        mov ecx,esi
        mov edx,ebx
        call resolve_update
        mov ecx,esi
        mov [esi+1Ch],edi
        call eax
        mov ecx,esi
        mov edx,ebx
        call resolve_update
        mov ecx,esi
        call eax
        pop edi
        pop esi
        add esp,8
        pop ebx
        ret
    }
}
__declspec(naked) void __fastcall pause_native_frame_clock_00bedae0(void*, const NativeFrameClockActualContext*) {
    __asm {
        push ebx
        mov ebx,edx
        sub esp,10h
        push esi
        mov esi,ecx
        cmp byte ptr [esi+68h],0
        jnz finished
        mov edx,ebx
        call resolve_sample
        mov edx,eax
        lea ecx,[esp+4]
        push ecx
        mov ecx,esi
        mov byte ptr [esi+68h],1
        call edx
        mov ecx,[eax]
        mov [esi+50h],ecx
        mov edx,[eax+4]
        mov [esi+54h],edx
        mov ecx,[eax+8]
        mov [esi+58h],ecx
        mov edx,[eax+0Ch]
        mov [esi+5Ch],edx
    finished:
        pop esi
        add esp,10h
        pop ebx
        ret
    }
}
__declspec(naked) void __fastcall resume_native_frame_clock_00beddc0(void*, const NativeFrameClockActualContext*) {
    __asm {
        push ebx
        mov ebx,edx
        sub esp,20h
        push esi
        mov esi,ecx
        cmp byte ptr [esi+68h],0
        jz finished
        mov edx,ebx
        call resolve_sample
        mov edx,eax
        lea eax,[esi+50h]
        push eax
        lea ecx,[esp+8]
        push ecx
        lea eax,[esp+1Ch]
        push eax
        mov ecx,esi
        mov byte ptr [esi+68h],0
        call edx
        mov ecx,eax
        call subtract_raw
        push eax
        lea ecx,[esi+10h]
        call add_raw
    finished:
        pop esi
        add esp,20h
        pop ebx
        ret
    }
}
__declspec(naked) const void* __fastcall get_raw_timer_current_00bee050(const void*) noexcept {
    __asm { lea eax,[ecx+20h]
        ret }
}
__declspec(naked) const void* __fastcall get_raw_timer_previous_00bee060(const void*) noexcept {
    __asm { lea eax,[ecx+30h]
        ret }
}
// Same four instruction bytes / interface as the historical orch5 raw getter.
__declspec(naked) const void* __fastcall get_raw_timer_interval_00bee070(const void*) noexcept {
    __asm { lea eax,[ecx+40h]
        ret }
}
} // namespace bsp

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
    label_00bedc30: SUB ESP,0x28
    label_00bedc33: PUSH ESI
    label_00bedc34: MOV ESI,ECX
    label_00bedc36: CMP byte ptr [ESI + 0x68],0x0
    label_00bedc3a: JZ label_00bedc67
    label_00bedc3c: ADD ESI,0x40
    label_00bedc3f: PUSH ESI
    label_00bedc40: LEA EAX,[ESP + 0x10]
    label_00bedc44: PUSH EAX
    label_00bedc45: MOV ECX,ESI
    label_00bedc47: CALL subtract_raw
    label_00bedc4c: MOV ECX,dword ptr [EAX]
    label_00bedc4e: MOV dword ptr [ESI],ECX
    label_00bedc50: MOV EDX,dword ptr [EAX + 0x4]
    label_00bedc53: MOV dword ptr [ESI + 0x4],EDX
    label_00bedc56: MOV ECX,dword ptr [EAX + 0x8]
    label_00bedc59: MOV dword ptr [ESI + 0x8],ECX
    label_00bedc5c: MOV EDX,dword ptr [EAX + 0xc]
    label_00bedc5f: MOV dword ptr [ESI + 0xc],EDX
    label_00bedc62: POP ESI
    label_00bedc63: ADD ESP,0x28
    label_00bedc66: RET
    label_00bedc67: ADD dword ptr [ESI + 0x8],0x1
    label_00bedc6b: MOV EAX,dword ptr [ESI + 0x20]
    label_00bedc6e: MOV ECX,dword ptr [ESI + 0x24]
    label_00bedc71: MOV EDX,dword ptr [ESI + 0x28]
    label_00bedc74: ADC dword ptr [ESI + 0xc],0x0
    label_00bedc78: CMP byte ptr [ESI + 0x69],0x0
    label_00bedc7c: PUSH EBX
    label_00bedc7d: PUSH EDI
    label_00bedc7e: LEA EDI,[ESI + 0x20]
    label_00bedc81: LEA EBX,[ESI + 0x30]
    label_00bedc84: MOV dword ptr [EBX],EAX
    label_00bedc86: MOV EAX,dword ptr [EDI + 0xc]
    label_00bedc89: MOV dword ptr [EBX + 0x4],ECX
    label_00bedc8c: MOV dword ptr [EBX + 0x8],EDX
    label_00bedc8f: MOV dword ptr [EBX + 0xc],EAX
    label_00bedc92: JNZ label_00bedce7
    label_00bedc94: LEA ECX,[ESP + 0xc]
    label_00bedc98: PUSH ECX
    label_00bedc99: CALL QueryPerformanceCounter
    label_00bedc9f: MOV EAX,dword ptr [ESP + 0x10]
    label_00bedca3: MOV ECX,dword ptr [ESI + 0x60]
    label_00bedca6: MOV EDX,dword ptr [ESP + 0xc]
    label_00bedcaa: MOV dword ptr [ESP + 0x18],EAX
    label_00bedcae: MOV dword ptr [ESP + 0x1c],ECX
    label_00bedcb2: LEA EAX,[ESI + 0x10]
    label_00bedcb5: PUSH EAX
    label_00bedcb6: LEA ECX,[ESP + 0x28]
    label_00bedcba: MOV dword ptr [ESP + 0x18],EDX
    label_00bedcbe: MOV EDX,dword ptr [ESI + 0x64]
    label_00bedcc1: PUSH ECX
    label_00bedcc2: LEA ECX,[ESP + 0x1c]
    label_00bedcc6: MOV dword ptr [ESP + 0x28],EDX
    label_00bedcca: CALL subtract_raw
    label_00bedccf: MOV EDX,dword ptr [EAX]
    label_00bedcd1: MOV dword ptr [EDI],EDX
    label_00bedcd3: MOV ECX,dword ptr [EAX + 0x4]
    label_00bedcd6: MOV dword ptr [EDI + 0x4],ECX
    label_00bedcd9: MOV EDX,dword ptr [EAX + 0x8]
    label_00bedcdc: MOV dword ptr [EDI + 0x8],EDX
    label_00bedcdf: MOV EAX,dword ptr [EAX + 0xc]
    label_00bedce2: MOV dword ptr [EDI + 0xc],EAX
    label_00bedce5: JMP label_00bedd37
    label_00bedce7: MOV ECX,dword ptr [ESI + 0x70]
    label_00bedcea: ADD dword ptr [ESI + 0x78],ECX
    label_00bedced: MOV EDX,dword ptr [ESI + 0x74]
    label_00bedcf0: MOV EAX,dword ptr [ESI + 0x78]
    label_00bedcf3: ADC dword ptr [ESI + 0x7c],EDX
    label_00bedcf6: MOV ECX,dword ptr [ESI + 0x7c]
    label_00bedcf9: MOV EDX,dword ptr [ESI + 0x60]
    label_00bedcfc: MOV dword ptr [ESP + 0x18],ECX
    label_00bedd00: LEA ECX,[ESI + 0x10]
    label_00bedd03: MOV dword ptr [ESP + 0x1c],EDX
    label_00bedd07: PUSH ECX
    label_00bedd08: MOV dword ptr [ESP + 0x18],EAX
    label_00bedd0c: MOV EAX,dword ptr [ESI + 0x64]
    label_00bedd0f: LEA EDX,[ESP + 0x28]
    label_00bedd13: PUSH EDX
    label_00bedd14: LEA ECX,[ESP + 0x1c]
    label_00bedd18: MOV dword ptr [ESP + 0x28],EAX
    label_00bedd1c: CALL subtract_raw
    label_00bedd21: MOV ECX,dword ptr [EAX]
    label_00bedd23: MOV dword ptr [EDI],ECX
    label_00bedd25: MOV EDX,dword ptr [EAX + 0x4]
    label_00bedd28: MOV dword ptr [EDI + 0x4],EDX
    label_00bedd2b: MOV ECX,dword ptr [EAX + 0x8]
    label_00bedd2e: MOV dword ptr [EDI + 0x8],ECX
    label_00bedd31: MOV EDX,dword ptr [EAX + 0xc]
    label_00bedd34: MOV dword ptr [EDI + 0xc],EDX
    label_00bedd37: PUSH EBX
    label_00bedd38: LEA EAX,[ESP + 0x28]
    label_00bedd3c: PUSH EAX
    label_00bedd3d: MOV ECX,EDI
    label_00bedd3f: CALL subtract_raw
    label_00bedd44: MOV ECX,dword ptr [EAX]
    label_00bedd46: MOV dword ptr [ESI + 0x40],ECX
    label_00bedd49: MOV EDX,dword ptr [EAX + 0x4]
    label_00bedd4c: MOV dword ptr [ESI + 0x44],EDX
    label_00bedd4f: MOV ECX,dword ptr [EAX + 0x8]
    label_00bedd52: MOV dword ptr [ESI + 0x48],ECX
    label_00bedd55: MOV EDX,dword ptr [EAX + 0xc]
    label_00bedd58: MOV dword ptr [ESI + 0x4c],EDX
    label_00bedd5b: FILD qword ptr [ESI + 0x40]
    label_00bedd5e: FILD qword ptr [ESI + 0x48]
    label_00bedd61: FDIVP
    label_00bedd63: FSTP float ptr [ESP + 0xc]
    label_00bedd67: FLD float ptr [ESP + 0xc]
    label_00bedd6b: FLDZ
    label_00bedd6d: FCOMIP ST0,ST1
    label_00bedd6f: FSTP ST0
    label_00bedd71: JBE label_00beddad
    label_00bedd73: MOV EAX,dword ptr [EBX]
    label_00bedd75: MOV ECX,dword ptr [EBX + 0x4]
    label_00bedd78: MOV EDX,dword ptr [EBX + 0x8]
    label_00bedd7b: MOV dword ptr [EDI],EAX
    label_00bedd7d: MOV EAX,dword ptr [EBX + 0xc]
    label_00bedd80: MOV dword ptr [EDI + 0x4],ECX
    label_00bedd83: PUSH EBX
    label_00bedd84: LEA ECX,[ESP + 0x28]
    label_00bedd88: PUSH ECX
    label_00bedd89: MOV dword ptr [EDI + 0x8],EDX
    label_00bedd8c: MOV ECX,EDI
    label_00bedd8e: MOV dword ptr [EDI + 0xc],EAX
    label_00bedd91: CALL subtract_raw
    label_00bedd96: MOV EDX,dword ptr [EAX]
    label_00bedd98: MOV dword ptr [ESI + 0x40],EDX
    label_00bedd9b: MOV ECX,dword ptr [EAX + 0x4]
    label_00bedd9e: MOV dword ptr [ESI + 0x44],ECX
    label_00bedda1: MOV EDX,dword ptr [EAX + 0x8]
    label_00bedda4: MOV dword ptr [ESI + 0x48],EDX
    label_00bedda7: MOV EAX,dword ptr [EAX + 0xc]
    label_00beddaa: MOV dword ptr [ESI + 0x4c],EAX
    label_00beddad: POP EDI
    label_00beddae: POP EBX
    label_00beddaf: POP ESI
    label_00beddb0: ADD ESP,0x28
    label_00beddb3: RET
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

#include "bsp/native_avoid_zone_group_add.hpp"
#include "bsp/cc13_avoid1.hpp"
#include "bsp/native_avoid_clip_cleanup.hpp"
#include "bsp/native_avoid_zone_construct.hpp"
#include "bsp/native_entity_path_access.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(Cc13AvoidZoneNativeGroup) == 0x14);
static_assert(offsetof(Cc13AvoidZoneNativeGroup, zones) == 4);
static_assert(offsetof(Cc13AvoidZoneNativeGroup, count) == 8);
static_assert(offsetof(Cc13AvoidZoneNativeGroup, capacity) == 0xc);
static_assert(offsetof(Cc13AvoidZoneNativeGroup, layer_key) == 0x10);
static_assert(sizeof(AvoidZoneNativeStorage) == 0x24);

// These are private Source context/phase cells in the enclosing current C++
// frame, not a Native FS registration or substituted incoming entity word.
struct GroupAddWorkingSet {
    void* const volatile* actual_world_pointer_cell;
    std::uint32_t* actual_entity_cell;
    volatile std::int32_t phase;
};
static_assert(offsetof(GroupAddWorkingSet, actual_entity_cell) == 4);
static_assert(offsetof(GroupAddWorkingSet, phase) == 8);
static_assert(sizeof(GroupAddWorkingSet) == 12);

__declspec(noinline) void* __cdecl allocate_group_add_bytes(std::uint32_t bytes) {
    return singleton_lifetime_allocate(
        {SingletonAllocationKind::object, bytes, bytes});
}

// Preserve Original GP register/count/flags operations literally. The extra
// saved EDX word points at persistent Source metadata; it supplies only the
// actual argument-cell address, Source phase and borrowed world-cell address.
__declspec(naked) void __fastcall run_group_add_kernel(void*, GroupAddWorkingSet*) {
    __asm {
        push edx
    // 0041D1E0 MOV EAX,FS:[0x0] [native_eh_frame_removed]
    // 0041D1E6 PUSH -0x1 [native_eh_frame_removed]
    // 0041D1E8 PUSH 0xc5e1bb [native_eh_frame_removed]
    // 0041D1ED PUSH EAX [native_eh_frame_removed]
    // 0041D1EE MOV dword ptr FS:[0x0],ESP [native_eh_frame_removed]
    // 0041D1F5 PUSH EBX [literal]
        _emit 0x53
    // 0041D1F6 PUSH EBP [literal]
        _emit 0x55
    // 0041D1F7 PUSH ESI [literal]
        _emit 0x56
    // 0041D1F8 PUSH EDI [literal]
        _emit 0x57
    // 0041D1F9 MOV EDI,dword ptr [ESP + 0x20] [actual_entity_cell_load]
        mov edi, dword ptr [esp + 10h]
        mov edi, dword ptr [edi + 4]
        mov edi, dword ptr [edi]
    // 0041D1FD MOV ESI,ECX [literal]
        _emit 0x8b
        _emit 0xf1
    // 0041D1FF MOV ECX,EDI [literal]
        _emit 0x8b
        _emit 0xcf
    // 0041D201 CALL 0x007ac9d0 [genuine_provider]
        call get_native_entity_path_interface_007ac9d0
    // 0041D206 CMP dword ptr [EDI + 0x54],0x2 [literal]
        _emit 0x83
        _emit 0x7f
        _emit 0x54
        _emit 0x02
    // 0041D20A MOV EBX,EAX [literal]
        _emit 0x8b
        _emit 0xd8
    // 0041D20C JNZ 0x0041d2fc [branch]
        jne group_add_0041d2fc
    // 0041D212 PUSH 0x24 [literal]
        _emit 0x6a
        _emit 0x24
    // 0041D214 CALL 0x00bf681b [genuine_provider]
        call allocate_group_add_bytes
    // 0041D219 ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041D21C MOV dword ptr [ESP + 0x20],EAX [actual_entity_cell_replace]
        push ecx
        mov ecx, dword ptr [esp + 14h]
        mov ecx, dword ptr [ecx + 4]
        mov dword ptr [ecx], eax
        pop ecx
    // 0041D220 TEST EAX,EAX [literal]
        _emit 0x85
        _emit 0xc0
    // 0041D222 MOV dword ptr [ESP + 0x18],0x0 [current_source_phase]
        push ecx
        mov ecx, dword ptr [esp + 14h]
        mov dword ptr [ecx + 8], 0
        pop ecx
    // 0041D22A JZ 0x0041d23c [branch]
        je group_add_0041d23c
    // 0041D22C MOV ECX,dword ptr [ESI + 0x10] [literal]
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
    // 0041D22F PUSH ECX [literal]
        _emit 0x51
    // 0041D230 PUSH EBX [literal]
        _emit 0x53
    // 0041D231 MOV ECX,EAX [literal]
        _emit 0x8b
        _emit 0xc8
    // 0041D233 CALL 0x0041ccd0 [genuine_constructor_with_borrowed_cell_address]
        mov edx, dword ptr [esp + 18h]
        mov edx, dword ptr [edx]
        call construct_native_avoid_zone_0041ccd0
    // 0041D238 MOV EBP,EAX [literal]
        _emit 0x8b
        _emit 0xe8
    // 0041D23A JMP 0x0041d23e [branch]
        jmp group_add_0041d23e
    // 0041D23C XOR EBP,EBP [literal]
    group_add_0041d23c:
        _emit 0x33
        _emit 0xed
    // 0041D23E MOV EDX,dword ptr [EBP + 0x4] [literal]
    group_add_0041d23e:
        _emit 0x8b
        _emit 0x55
        _emit 0x04
    // 0041D241 TEST EDX,EDX [literal]
        _emit 0x85
        _emit 0xd2
    // 0041D243 MOV dword ptr [ESP + 0x18],0xffffffff [current_source_phase]
        push ecx
        mov ecx, dword ptr [esp + 14h]
        mov dword ptr [ecx + 8], -1
        pop ecx
    // 0041D24B JBE 0x0041d2d5 [branch]
        jbe group_add_0041d2d5
    // 0041D251 MOV EAX,dword ptr [ESI + 0xc] [literal]
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
    // 0041D254 MOV ECX,dword ptr [ESI + 0x8] [literal]
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
    // 0041D257 CMP ECX,EAX [literal]
        _emit 0x3b
        _emit 0xc8
    // 0041D259 JNZ 0x0041d2b3 [branch]
        jne group_add_0041d2b3
    // 0041D25B LEA EAX,[EAX + EAX*0x1 + 0x2] [literal]
        _emit 0x8d
        _emit 0x44
        _emit 0x00
        _emit 0x02
    // 0041D25F CMP EAX,dword ptr [ESI + 0xc] [literal]
        _emit 0x3b
        _emit 0x46
        _emit 0x0c
    // 0041D262 JBE 0x0041d2b3 [branch]
        jbe group_add_0041d2b3
    // 0041D264 XOR ECX,ECX [literal]
        _emit 0x33
        _emit 0xc9
    // 0041D266 MOV dword ptr [ESI + 0xc],EAX [literal]
        _emit 0x89
        _emit 0x46
        _emit 0x0c
    // 0041D269 MOV EDX,0x4 [literal]
        _emit 0xba
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041D26E MUL EDX [literal]
        _emit 0xf7
        _emit 0xe2
    // 0041D270 SETO CL [literal]
        _emit 0x0f
        _emit 0x90
        _emit 0xc1
    // 0041D273 NEG ECX [literal]
        _emit 0xf7
        _emit 0xd9
    // 0041D275 OR ECX,EAX [literal]
        _emit 0x0b
        _emit 0xc8
    // 0041D277 PUSH ECX [literal]
        _emit 0x51
    // 0041D278 CALL 0x00bf55be [genuine_provider]
        call allocate_group_add_bytes
    // 0041D27D ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041D280 CMP dword ptr [ESI + 0x4],0x0 [literal]
        _emit 0x83
        _emit 0x7e
        _emit 0x04
        _emit 0x00
    // 0041D284 MOV EDI,EAX [literal]
        _emit 0x8b
        _emit 0xf8
    // 0041D286 JZ 0x0041d2b0 [branch]
        je group_add_0041d2b0
    // 0041D288 MOV ECX,dword ptr [ESI + 0x8] [literal]
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
    // 0041D28B XOR EAX,EAX [literal]
        _emit 0x33
        _emit 0xc0
    // 0041D28D TEST ECX,ECX [literal]
        _emit 0x85
        _emit 0xc9
    // 0041D28F JBE 0x0041d2a4 [branch]
        jbe group_add_0041d2a4
    // 0041D291 MOV EDX,dword ptr [ESI + 0x4] [literal]
    group_add_0041d291:
        _emit 0x8b
        _emit 0x56
        _emit 0x04
    // 0041D294 MOV ECX,dword ptr [EDX + EAX*0x4] [literal]
        _emit 0x8b
        _emit 0x0c
        _emit 0x82
    // 0041D297 MOV dword ptr [EDI + EAX*0x4],ECX [literal]
        _emit 0x89
        _emit 0x0c
        _emit 0x87
    // 0041D29A MOV EDX,dword ptr [ESI + 0x8] [literal]
        _emit 0x8b
        _emit 0x56
        _emit 0x08
    // 0041D29D ADD EAX,0x1 [literal]
        _emit 0x83
        _emit 0xc0
        _emit 0x01
    // 0041D2A0 CMP EAX,EDX [literal]
        _emit 0x3b
        _emit 0xc2
    // 0041D2A2 JC 0x0041d291 [branch]
        jb group_add_0041d291
    // 0041D2A4 MOV EAX,dword ptr [ESI + 0x4] [literal]
    group_add_0041d2a4:
        _emit 0x8b
        _emit 0x46
        _emit 0x04
    // 0041D2A7 PUSH EAX [literal]
        _emit 0x50
    // 0041D2A8 CALL 0x00bf6989 [genuine_provider]
        call singleton_lifetime_free
    // 0041D2AD ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041D2B0 MOV dword ptr [ESI + 0x4],EDI [literal]
    group_add_0041d2b0:
        _emit 0x89
        _emit 0x7e
        _emit 0x04
    // 0041D2B3 MOV ECX,dword ptr [ESI + 0x8] [literal]
    group_add_0041d2b3:
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
    // 0041D2B6 MOV EDX,dword ptr [ESI + 0x4] [literal]
        _emit 0x8b
        _emit 0x56
        _emit 0x04
    // 0041D2B9 MOV dword ptr [EDX + ECX*0x4],EBP [literal]
        _emit 0x89
        _emit 0x2c
        _emit 0x8a
    // 0041D2BC ADD dword ptr [ESI + 0x8],0x1 [literal]
        _emit 0x83
        _emit 0x46
        _emit 0x08
        _emit 0x01
    // 0041D2C0 MOV ECX,dword ptr [ESP + 0x10] [native_eh_frame_removed]
    // 0041D2C4 MOV dword ptr FS:[0x0],ECX [native_eh_frame_removed]
    // 0041D2CB POP EDI [literal]
        _emit 0x5f
    // 0041D2CC POP ESI [literal]
        _emit 0x5e
    // 0041D2CD POP EBP [literal]
        _emit 0x5d
    // 0041D2CE POP EBX [literal]
        _emit 0x5b
    // 0041D2CF ADD ESP,0xc [native_eh_frame_removed]
    // 0041D2D2 RET 0x4 [private_kernel_return]
        pop edx
        ret
    // 0041D2D5 MOV ECX,EBP [literal]
    group_add_0041d2d5:
        _emit 0x8b
        _emit 0xcd
    // 0041D2D7 CALL 0x004167c0 [genuine_provider]
        call clear_native_avoid_corner_records_004167c0
    // 0041D2DC MOV EAX,dword ptr [EBP] [literal]
        _emit 0x8b
        _emit 0x45
        _emit 0x00
    // 0041D2DF TEST EAX,EAX [literal]
        _emit 0x85
        _emit 0xc0
    // 0041D2E1 JZ 0x0041d2f3 [branch]
        je group_add_0041d2f3
    // 0041D2E3 PUSH EAX [literal]
        _emit 0x50
    // 0041D2E4 CALL 0x00bf6989 [genuine_provider]
        call singleton_lifetime_free
    // 0041D2E9 ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041D2EC MOV dword ptr [EBP],0x0 [literal]
        _emit 0xc7
        _emit 0x45
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    // 0041D2F3 PUSH EBP [literal]
    group_add_0041d2f3:
        _emit 0x55
    // 0041D2F4 CALL 0x00bf65ac [genuine_provider]
        call singleton_lifetime_free
    // 0041D2F9 ADD ESP,0x4 [literal]
        _emit 0x83
        _emit 0xc4
        _emit 0x04
    // 0041D2FC MOV ECX,dword ptr [ESP + 0x10] [native_eh_frame_removed]
    group_add_0041d2fc:
    // 0041D300 POP EDI [literal]
        _emit 0x5f
    // 0041D301 POP ESI [literal]
        _emit 0x5e
    // 0041D302 POP EBP [literal]
        _emit 0x5d
    // 0041D303 MOV dword ptr FS:[0x0],ECX [native_eh_frame_removed]
    // 0041D30A POP EBX [literal]
        _emit 0x5b
    // 0041D30B ADD ESP,0xc [native_eh_frame_removed]
    // 0041D30E RET 0x4 [private_kernel_return]
        pop edx
        ret
    }
}

class CurrentGroupAllocationGuard {
public:
    explicit CurrentGroupAllocationGuard(GroupAddWorkingSet& working) noexcept
        : working_(working) {}
    ~CurrentGroupAllocationGuard() noexcept {
        if (working_.phase == 0) {
            // Native C5E1B0 reads the CURRENT reused incoming word, then frees
            // unconditionally. Do not replace it with a cached allocated zone.
            const std::uint32_t current =
                *reinterpret_cast<volatile const std::uint32_t*>(working_.actual_entity_cell);
            singleton_lifetime_free(reinterpret_cast<void*>(current));
        }
    }
    CurrentGroupAllocationGuard(const CurrentGroupAllocationGuard&) = delete;
    CurrentGroupAllocationGuard& operator=(const CurrentGroupAllocationGuard&) = delete;
private:
    GroupAddWorkingSet& working_;
};

__declspec(noinline) void __cdecl add_current_source_frames(
    void* actual_group, void* const volatile* actual_world_pointer_cell,
    std::uint32_t* actual_entity_cell) {
    GroupAddWorkingSet working{actual_world_pointer_cell, actual_entity_cell, -1};
    CurrentGroupAllocationGuard guard(working);
    // Fixed real entry, never a caller-supplied provider. Its non-noexcept
    // volatile type retains genuine current MSVC cleanup while naked code runs.
    using Kernel = void (__fastcall*)(void*, GroupAddWorkingSet*);
    Kernel volatile kernel = run_group_add_kernel;
    kernel(actual_group, &working);
}

} // namespace

__declspec(naked) void __fastcall add_native_avoid_zone_group_entity_0041d1e0(
    void*, void* const volatile*, void*) {
    __asm {
        lea eax, [esp + 4]
        push eax
        push edx
        push ecx
        call add_current_source_frames
        add esp, 12
        ret 4
    }
}

} // namespace bsp

#include "bsp/native_dyn_world_step.hpp"
#include <type_traits>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native world step requires MSVC Win32 x87/SSE assembly.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;
struct Context {const NativeDynWorldStepContext* owner;NativeDynWorldStepCalls* calls;void* const* profile_slot;void* const* engine_slot;};
static_assert(std::is_standard_layout_v<Context> && offsetof(Context,profile_slot)==8 && offsetof(Context,engine_slot)==12);
Context bind(const NativeDynWorldStepContext& c){return {&c,&c.calls,c.collision.profile.profile_slot_0109e9f8,c.collision.engine_slot_0109e9fc};}
void* __cdecl allocate_bridge(U site,Context* c,U size){return c->calls->allocate_00bf55be(site,size,c->owner->collision.memory);}
void __cdecl free_bridge(U site,Context* c,void* p){c->calls->free_00bf6989(site,p,c->owner->collision.memory);}
void* __cdecl append_bridge(Context* c,DynProfileNodeStorage* p,const char* name,U id){return dyn_profile_node_append_child_00c50390(*p,name,id,c->owner->collision.profile.memory);}
std::uint64_t __cdecl timestamp_bridge(U site,Context* c) noexcept{return c->calls->read_timestamp(site);}
void __cdecl velocity_bridge(Context* c,void* world,float dt){c->calls->integrate_velocities_00c41550(world,dt);}
void __cdecl position_bridge(Context* c,void* world,float dt){c->calls->integrate_positions_00c5b1b0(world,dt,c->owner->crt);}
void __cdecl collision_bridge(Context* c,void* scene){c->calls->collision_pass_00c57070(scene,c->owner->collision);}
void __cdecl create_bridge(Context* c,void* manager){c->calls->create_groups_00c4b610(manager,c->owner->groups);}
void __cdecl sleep_bridge(Context* c,void* manager){c->calls->sleep_groups_00c4b550(manager,c->owner->groups);}
void* __cdecl enter_bridge(Context* c,DynProfileScopeStorage* scope,U id,const char* name){return c->calls->enter_profile_00c57020(*scope,id,name,c->owner->collision.profile);}
void __cdecl clear_bridge(Context* c,void* body){NativeGamePhysicsLifetimeProgress progress;clear_native_dyn_body_manifolds_00c43aa0(body,c->owner->collision.memory,*c->calls,progress);}
void __cdecl storage_bridge(Context* c,void* body){NativeGamePhysicsLifetimeProgress progress;destroy_native_dyn_body_storage_00c43c00(body,c->owner->collision.memory,*c->calls,progress);}
void __cdecl batch_bridge(Context* c,void* manager,void* const* tasks,std::int32_t count){c->calls->run_batch_00c33140(manager,tasks,count,c->owner->collision.memory);}
void substep_kernel();
void simulate_kernel();
void flush_kernel();
void reset_kernel();
alignas(8) const std::uint64_t constant_00d7a398=0x3f0a36e2e0000000ULL;
const char name_00d79f18[]={83,105,109,117,108,97,116,101,0};
const char name_00d79f24[]={67,114,101,97,116,101,71,114,111,117,112,115,0};
const char name_00d79f34[]={83,111,108,118,101,0};
const char name_00d79f3c[]={85,112,100,97,116,101,80,111,115,105,116,105,111,110,0};
const char name_00d79f4c[]={83,108,101,101,112,71,114,111,117,112,115,0};
__declspec(naked) void append_shim(){
    __asm {
        push dword ptr [esp+0ch]
        push dword ptr [esp+0ch]
        push esi
        push dword ptr [esp+10h]
        call append_bridge
        add esp,10h
        ret 0ch
    }
}
__declspec(naked) void batch_shim(){
    __asm {
        push dword ptr [esp+8]
        push eax
        push esi
        push dword ptr [esp+10h]
        call batch_bridge
        add esp,10h
        ret 8
    }
}
__declspec(naked) void velocity_shim(){
    __asm {
        push dword ptr [esp+8]
        push esi
        push dword ptr [esp+0ch]
        call velocity_bridge
        add esp,0ch
        ret 8
    }
}
__declspec(naked) void position_shim(){
    __asm {
        push dword ptr [esp+8]
        push ebx
        push dword ptr [esp+0ch]
        call position_bridge
        add esp,0ch
        ret 8
    }
}
__declspec(naked) void collision_shim(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        call collision_bridge
        add esp,8
        ret 8
    }
}
__declspec(naked) void create_shim(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        call create_bridge
        add esp,8
        ret 8
    }
}
__declspec(naked) void sleep_shim(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        call sleep_bridge
        add esp,8
        ret 8
    }
}
__declspec(naked) void clear_shim(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        call clear_bridge
        add esp,8
        ret 8
    }
}
__declspec(naked) void storage_shim(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        call storage_bridge
        add esp,8
        ret 8
    }
}
__declspec(naked) void enter_shim(){
    __asm {
        push dword ptr [esp+8]
        push eax
        push edi
        push dword ptr [esp+10h]
        call enter_bridge
        add esp,10h
        ret 8
    }
}
__declspec(naked) void substep_shim(){
    __asm {
        push dword ptr [esp+4]
        push dword ptr [esp+10h]
        push dword ptr [esp+10h]
        call substep_kernel
        ret 0ch
    }
}
__declspec(naked) void timestamp_00c5bb9d(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c5bb9dh
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void timestamp_00c5bbcb(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c5bbcbh
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void timestamp_00c5bcd7(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c5bcd7h
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void bridge_00c5bd4f(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c5bd4fh
        call allocate_bridge
        add esp,0ch
        ret 4
    }
}
__declspec(naked) void timestamp_00c5c0f5(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c5c0f5h
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void bridge_00c5c16d(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c5c16dh
        call allocate_bridge
        add esp,0ch
        ret 4
    }
}
__declspec(naked) void bridge_00c5c449(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c5c449h
        call free_bridge
        add esp,0ch
        ret 4
    }
}
__declspec(naked) void timestamp_00c5c47f(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c5c47fh
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void timestamp_00c5c496(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c5c496h
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void timestamp_00c5c4e8(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c5c4e8h
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void timestamp_00c5c508(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c5c508h
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void timestamp_00c5c5df(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c5c5dfh
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void timestamp_00c5c5f3(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c5c5f3h
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void timestamp_00c5c6ce(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c5c6ceh
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
// Full normal native instruction schedule; addresses identify original starts.
__declspec(naked) void substep_kernel(){
    __asm {
        push -1 // 00c5bb30
        mov eax, dword ptr fs:[0] // 00c5bb32
        fld dword ptr [esp + 0ch] // 00c5bb38
        push 0 // 00c5bb3c
        push eax // 00c5bb41
        // 00c5bb42: native FS write omitted; normal-return contract.
        sub esp, 058h // 00c5bb49
        push ebx // 00c5bb4c
        push ebp // 00c5bb4d
        mov ebp, dword ptr [esp + 070h] // 00c5bb4e
        push esi // 00c5bb52
        push edi // 00c5bb53
        push ecx // 00c5bb54
        mov esi, ebp // 00c5bb55
        fstp dword ptr [esp] // 00c5bb57
        push dword ptr [esp+132] // Borrowed context; original frame offsets retained.
        call velocity_shim // 00c5bb5a
        mov eax, dword ptr [ebp + 0444h] // 00c5bb5f
        push eax // 00c5bb65
        push dword ptr [esp+132] // Borrowed context; original frame offsets retained.
        call collision_shim // 00c5bb66
        mov eax,dword ptr [esp+128] // 00c5bb6b: borrowed context.
        mov eax,dword ptr [eax+8]
        mov eax,dword ptr [eax]
        cmp dword ptr [eax + 064h], 0 // 00c5bb70
        lea edi, [eax + 064h] // 00c5bb74
        jne l_00c5bb8f // 00c5bb77
        mov esi, dword ptr [eax + 4] // 00c5bb79
        push 016h // 00c5bb7c
        push offset name_00d79f24 // 00c5bb7e
        push dword ptr [esp+136] // Borrowed context; original frame offsets retained.
        call append_shim // 00c5bb83
        mov dword ptr [edi], eax // 00c5bb88
        mov eax,dword ptr [esp+128] // 00c5bb8a: borrowed context.
        mov eax,dword ptr [eax+8]
        mov eax,dword ptr [eax]
    l_00c5bb8f:
        mov esi, dword ptr [edi] // 00c5bb8f
        mov ecx, dword ptr [eax + 4] // 00c5bb91
        mov dword ptr [esi], ecx // 00c5bb94
        mov dword ptr [esp + 05ch], esi // 00c5bb96
        mov dword ptr [eax + 4], esi // 00c5bb9a
        push dword ptr [esp+128] // Borrowed context; original frame offsets retained.
        call timestamp_00c5bb9d // 00c5bb9d
        mov ebx, edx // 00c5bb9f
        mov edi, eax // 00c5bba1
        mov dword ptr [esp + 050h], ebx // 00c5bba3
        mov dword ptr [esp + 04ch], edi // 00c5bba7
        lea eax, [ebp + 0448h] // 00c5bbab
        push eax // 00c5bbb1
        mov dword ptr [esp + 074h], 0 // 00c5bbb2
        mov dword ptr [esp + 020h], eax // 00c5bbba
        push dword ptr [esp+132] // Borrowed context; original frame offsets retained.
        call create_shim // 00c5bbbe
        mov dword ptr [esp + 070h], 0ffffffffh // 00c5bbc3
        push dword ptr [esp+128] // Borrowed context; original frame offsets retained.
        call timestamp_00c5bbcb // 00c5bbcb
        sub eax, edi // 00c5bbcd
        sbb edx, ebx // 00c5bbcf
        add dword ptr [esi + 038h], eax // 00c5bbd1
        mov dword ptr [esi + 030h], eax // 00c5bbd4
        mov eax,dword ptr [esp+128] // 00c5bbd7: borrowed context.
        mov eax,dword ptr [eax+8]
        mov eax,dword ptr [eax]
        adc dword ptr [esi + 03ch], edx // 00c5bbdc
        mov dword ptr [esi + 034h], edx // 00c5bbdf
        mov ebx, 1 // 00c5bbe2
        add dword ptr [esi + 040h], ebx // 00c5bbe7
        mov edx, dword ptr [eax + 4] // 00c5bbea
        mov ecx, dword ptr [edx] // 00c5bbed
        mov dword ptr [eax + 4], ecx // 00c5bbef
        mov ecx, dword ptr [ebp + 010h] // 00c5bbf2
        sub ecx, 0 // 00c5bbf5
        je l_00c5c02a // 00c5bbf8
        sub ecx, ebx // 00c5bbfe
        jne l_00c5c456 // 00c5bc00
        cmp dword ptr [ebp + 0458h], ecx // 00c5bc06
        je l_00c5bd04 // 00c5bc0c
        push offset name_00d79f34 // 00c5bc12
        lea eax, [ebx + 017h] // 00c5bc17
        lea edi, [esp + 050h] // 00c5bc1a
        push dword ptr [esp+132] // Borrowed context; original frame offsets retained.
        call enter_shim // 00c5bc1e
        mov dword ptr [esp + 070h], 3 // 00c5bc23
        mov eax, dword ptr [ebp + 0458h] // 00c5bc2b
        mov esi, dword ptr [ebp + 0478h] // 00c5bc31
        cmp esi, eax // 00c5bc37
        jge l_00c5bc41 // 00c5bc39
        mov dword ptr [esp + 014h], esi // 00c5bc3b
        jmp l_00c5bc47 // 00c5bc3f
    l_00c5bc41:
        mov esi, eax // 00c5bc41
        mov dword ptr [esp + 014h], eax // 00c5bc43
    l_00c5bc47:
        movss xmm0, dword ptr [esp + 07ch] // 00c5bc47
        cdq  // 00c5bc4d
        lea ebx, [esi - 1] // 00c5bc4e
        xor ecx, ecx // 00c5bc51
        idiv esi // 00c5bc53
        xor edi, edi // 00c5bc55
        test ebx, ebx // 00c5bc57
        jle l_00c5bc98 // 00c5bc59
        xor edx, edx // 00c5bc5b
        mov dword ptr [esp + 078h], ebx // 00c5bc5d
        mov edi, ebx // 00c5bc61
        jmp l_00c5bc70 // 00c5bc63
        _emit 08dh // 00c5bc65
        _emit 0a4h
        _emit 024h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 08dh // 00c5bc6c
        _emit 064h
        _emit 024h
        _emit 000h
    l_00c5bc70:
        mov esi, dword ptr [ebp + 0474h] // 00c5bc70
        lea ebx, [eax + ecx - 1] // 00c5bc76
        mov dword ptr [edx + esi + 0ch], ecx // 00c5bc7a
        mov dword ptr [edx + esi + 010h], ebx // 00c5bc7e
        movss dword ptr [edx + esi + 014h], xmm0 // 00c5bc82
        add ecx, eax // 00c5bc88
        add edx, 018h // 00c5bc8a
        sub dword ptr [esp + 078h], 1 // 00c5bc8d
        jne l_00c5bc70 // 00c5bc92
        mov esi, dword ptr [esp + 014h] // 00c5bc94
    l_00c5bc98:
        mov edx, dword ptr [ebp + 0458h] // 00c5bc98
        mov eax, dword ptr [ebp + 0474h] // 00c5bc9e
        lea edi, [edi + edi*2] // 00c5bca4
        lea eax, [eax + edi*8] // 00c5bca7
        add edx, -1 // 00c5bcaa
        mov dword ptr [eax + 010h], edx // 00c5bcad
        mov edx,dword ptr [esp+128] // 00c5bcb0: borrowed context.
        mov edx,dword ptr [edx+12]
        mov edx,dword ptr [edx]
        mov dword ptr [eax + 0ch], ecx // 00c5bcb6
        movss dword ptr [eax + 014h], xmm0 // 00c5bcb9
        mov ecx, dword ptr [edx + 010h] // 00c5bcbe
        mov eax, dword ptr [ebp + 0480h] // 00c5bcc1
        push esi // 00c5bcc7
        mov esi, ecx // 00c5bcc8
        push dword ptr [esp+132] // Borrowed context; original frame offsets retained.
        call batch_shim // 00c5bcca
        mov dword ptr [esp + 070h], 0ffffffffh // 00c5bccf
        push dword ptr [esp+128] // Borrowed context; original frame offsets retained.
        call timestamp_00c5bcd7 // 00c5bcd7
        sub eax, dword ptr [esp + 04ch] // 00c5bcd9
        sbb edx, dword ptr [esp + 050h] // 00c5bcdd
        mov ecx, eax // 00c5bce1
        mov eax, dword ptr [esp + 05ch] // 00c5bce3
        add dword ptr [eax + 038h], ecx // 00c5bce7
        mov dword ptr [eax + 030h], ecx // 00c5bcea
        mov dword ptr [eax + 034h], edx // 00c5bced
        adc dword ptr [eax + 03ch], edx // 00c5bcf0
        add dword ptr [eax + 040h], 1 // 00c5bcf3
        mov eax,dword ptr [esp+128] // 00c5bcf7: borrowed context.
        mov eax,dword ptr [eax+8]
        mov eax,dword ptr [eax]
        mov ecx, dword ptr [eax + 4] // 00c5bcfc
        mov edx, dword ptr [ecx] // 00c5bcff
        mov dword ptr [eax + 4], edx // 00c5bd01
    l_00c5bd04:
        xor edi, edi // 00c5bd04
        cmp dword ptr [ebp + 024h], edi // 00c5bd06
        je l_00c5c456 // 00c5bd09
        mov dword ptr [esp + 020h], edi // 00c5bd0f
        mov dword ptr [esp + 024h], edi // 00c5bd13
        mov dword ptr [esp + 070h], 4 // 00c5bd17
        mov eax, dword ptr [ebp + 0444h] // 00c5bd1f
        mov eax, dword ptr [eax + 0b0h] // 00c5bd25
        mov eax, dword ptr [eax + 01d0h] // 00c5bd2b
        lea esi, [eax*4] // 00c5bd31
        cmp esi, edi // 00c5bd38
        jle l_00c5bd61 // 00c5bd3a
        xor ecx, ecx // 00c5bd3c
        mov eax, esi // 00c5bd3e
        mov edx, 058h // 00c5bd40
        mul edx // 00c5bd45
        seto cl // 00c5bd47
        neg ecx // 00c5bd4a
        or ecx, eax // 00c5bd4c
        push ecx // 00c5bd4e
        push dword ptr [esp+132] // Borrowed context; original frame offsets retained.
        call bridge_00c5bd4f // 00c5bd4f
        add esp, 4 // 00c5bd54
        mov dword ptr [esp + 020h], eax // 00c5bd57
        mov dword ptr [esp + 024h], esi // 00c5bd5b
        mov edi, eax // 00c5bd5f
    l_00c5bd61:
        mov eax, dword ptr [ebp + 0444h] // 00c5bd61
        mov ecx, dword ptr [eax + 0b0h] // 00c5bd67
        mov esi, dword ptr [ecx + 0ech] // 00c5bd6d
        mov eax, ecx // 00c5bd73
        add eax, 0f0h // 00c5bd75
        xor ebx, ebx // 00c5bd7a
        cmp esi, eax // 00c5bd7c
        je l_00c5c431 // 00c5bd7e
        xorps xmm0, xmm0 // 00c5bd84
        jmp l_00c5bd90 // 00c5bd87
        _emit 08dh // 00c5bd89
        _emit 0a4h
        _emit 024h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    l_00c5bd90:
        xor edi, edi // 00c5bd90
        cmp dword ptr [esi + 0c8h], edi // 00c5bd92
        jle l_00c5c006 // 00c5bd98
        mov ecx, dword ptr [esp + 020h] // 00c5bd9e
        mov eax, ebx // 00c5bda2
        imul eax, eax, 058h // 00c5bda4
        lea eax, [eax + ecx + 040h] // 00c5bda7
        lea ecx, [esi + 01ch] // 00c5bdab
        mov edi, edi // 00c5bdae
    l_00c5bdb0:
        mov edx, dword ptr [esi + 0cch] // 00c5bdb0
        fld dword ptr [ecx - 4] // 00c5bdb6
        fstp dword ptr [esp + 078h] // 00c5bdb9
        fld dword ptr [ecx - 8] // 00c5bdbd
        fstp dword ptr [esp + 014h] // 00c5bdc0
        fld dword ptr [ecx] // 00c5bdc4
        fstp dword ptr [esp + 018h] // 00c5bdc6
        fld dword ptr [edx + 014h] // 00c5bdca
        fld dword ptr [esp + 078h] // 00c5bdcd
        fld st(0) // 00c5bdd1
        fmulp st(2), st(0) // 00c5bdd3
        fld dword ptr [edx + 8] // 00c5bdd5
        fld dword ptr [esp + 014h] // 00c5bdd8
        fld st(0) // 00c5bddc
        fmulp st(2), st(0) // 00c5bdde
        fxch st(3) // 00c5bde0
        faddp st(1), st(0) // 00c5bde2
        fld dword ptr [edx + 020h] // 00c5bde4
        fld dword ptr [esp + 018h] // 00c5bde7
        fld st(0) // 00c5bdeb
        fmulp st(2), st(0) // 00c5bded
        fxch st(2) // 00c5bdef
        faddp st(1), st(0) // 00c5bdf1
        fadd dword ptr [edx + 02ch] // 00c5bdf3
        fstp dword ptr [esp + 078h] // 00c5bdf6
        fld dword ptr [edx + 0ch] // 00c5bdfa
        movss xmm1, dword ptr [esp + 078h] // 00c5bdfd
        fmul st(0), st(3) // 00c5be03
        movss dword ptr [esp + 028h], xmm1 // 00c5be05
        fld dword ptr [edx + 018h] // 00c5be0b
        fmul st(0), st(3) // 00c5be0e
        faddp st(1), st(0) // 00c5be10
        fld dword ptr [edx + 024h] // 00c5be12
        fmul st(0), st(2) // 00c5be15
        faddp st(1), st(0) // 00c5be17
        fadd dword ptr [edx + 030h] // 00c5be19
        fstp dword ptr [esp + 018h] // 00c5be1c
        fld dword ptr [edx + 010h] // 00c5be20
        fmulp st(3), st(0) // 00c5be23
        fld dword ptr [edx + 01ch] // 00c5be25
        fmulp st(2), st(0) // 00c5be28
        fxch st(2) // 00c5be2a
        faddp st(1), st(0) // 00c5be2c
        fld dword ptr [edx + 028h] // 00c5be2e
        fmulp st(2), st(0) // 00c5be31
        faddp st(1), st(0) // 00c5be33
        fadd dword ptr [edx + 034h] // 00c5be35
        mov edx, dword ptr [esp + 028h] // 00c5be38
        mov dword ptr [eax - 040h], edx // 00c5be3c
        fstp dword ptr [esp + 014h] // 00c5be3f
        fld dword ptr [esp + 018h] // 00c5be43
        fstp dword ptr [esp + 02ch] // 00c5be47
        mov edx, dword ptr [esp + 02ch] // 00c5be4b
        fld dword ptr [esp + 014h] // 00c5be4f
        mov dword ptr [eax - 03ch], edx // 00c5be53
        fstp dword ptr [esp + 030h] // 00c5be56
        mov edx, dword ptr [esp + 030h] // 00c5be5a
        mov dword ptr [eax - 038h], edx // 00c5be5e
        fld dword ptr [ecx + 8] // 00c5be61
        mov edx, dword ptr [esi + 0d0h] // 00c5be64
        fstp dword ptr [esp + 078h] // 00c5be6a
        fld dword ptr [ecx + 4] // 00c5be6e
        fstp dword ptr [esp + 018h] // 00c5be71
        fld dword ptr [ecx + 0ch] // 00c5be75
        fstp dword ptr [esp + 014h] // 00c5be78
        fld dword ptr [edx + 014h] // 00c5be7c
        fld dword ptr [esp + 078h] // 00c5be7f
        fld st(0) // 00c5be83
        fmulp st(2), st(0) // 00c5be85
        fld dword ptr [edx + 8] // 00c5be87
        fld dword ptr [esp + 018h] // 00c5be8a
        fld st(0) // 00c5be8e
        fmulp st(2), st(0) // 00c5be90
        fxch st(3) // 00c5be92
        faddp st(1), st(0) // 00c5be94
        fld dword ptr [edx + 020h] // 00c5be96
        fld dword ptr [esp + 014h] // 00c5be99
        fld st(0) // 00c5be9d
        fmulp st(2), st(0) // 00c5be9f
        fxch st(2) // 00c5bea1
        faddp st(1), st(0) // 00c5bea3
        fadd dword ptr [edx + 02ch] // 00c5bea5
        fstp dword ptr [esp + 078h] // 00c5bea8
        fld dword ptr [edx + 0ch] // 00c5beac
        movss xmm1, dword ptr [esp + 078h] // 00c5beaf
        fmul st(0), st(3) // 00c5beb5
        movss dword ptr [esp + 034h], xmm1 // 00c5beb7
        fld dword ptr [edx + 018h] // 00c5bebd
        fmul st(0), st(3) // 00c5bec0
        faddp st(1), st(0) // 00c5bec2
        fld dword ptr [edx + 024h] // 00c5bec4
        fmul st(0), st(2) // 00c5bec7
        faddp st(1), st(0) // 00c5bec9
        fadd dword ptr [edx + 030h] // 00c5becb
        fstp dword ptr [esp + 018h] // 00c5bece
        fld dword ptr [edx + 010h] // 00c5bed2
        fmulp st(3), st(0) // 00c5bed5
        fld dword ptr [edx + 01ch] // 00c5bed7
        fmulp st(2), st(0) // 00c5beda
        fxch st(2) // 00c5bedc
        faddp st(1), st(0) // 00c5bede
        fld dword ptr [edx + 028h] // 00c5bee0
        fmulp st(2), st(0) // 00c5bee3
        faddp st(1), st(0) // 00c5bee5
        fadd dword ptr [edx + 034h] // 00c5bee7
        mov edx, dword ptr [esp + 034h] // 00c5beea
        mov dword ptr [eax - 034h], edx // 00c5beee
        fstp dword ptr [esp + 014h] // 00c5bef1
        fld dword ptr [esp + 018h] // 00c5bef5
        fstp dword ptr [esp + 038h] // 00c5bef9
        mov edx, dword ptr [esp + 038h] // 00c5befd
        fld dword ptr [esp + 014h] // 00c5bf01
        mov dword ptr [eax - 030h], edx // 00c5bf05
        fstp dword ptr [esp + 03ch] // 00c5bf08
        mov edx, dword ptr [esp + 03ch] // 00c5bf0c
        mov dword ptr [eax - 02ch], edx // 00c5bf10
        mov edx, dword ptr [ecx - 014h] // 00c5bf13
        mov dword ptr [eax - 028h], edx // 00c5bf16
        mov edx, dword ptr [ecx - 010h] // 00c5bf19
        mov dword ptr [eax - 024h], edx // 00c5bf1c
        mov edx, dword ptr [ecx - 0ch] // 00c5bf1f
        mov dword ptr [eax - 020h], edx // 00c5bf22
        fld dword ptr [ecx + 010h] // 00c5bf25
        fstp dword ptr [esp + 078h] // 00c5bf28
        fld dword ptr [ecx - 014h] // 00c5bf2c
        fld dword ptr [esp + 078h] // 00c5bf2f
        fld st(0) // 00c5bf33
        fmulp st(2), st(0) // 00c5bf35
        fxch st(1) // 00c5bf37
        fstp dword ptr [esp + 078h] // 00c5bf39
        fld dword ptr [ecx - 010h] // 00c5bf3d
        fmul st(0), st(1) // 00c5bf40
        fstp dword ptr [esp + 018h] // 00c5bf42
        fmul dword ptr [ecx - 0ch] // 00c5bf46
        fstp dword ptr [esp + 014h] // 00c5bf49
        fld dword ptr [esp + 078h] // 00c5bf4d
        fstp dword ptr [esp + 040h] // 00c5bf51
        mov edx, dword ptr [esp + 040h] // 00c5bf55
        fld dword ptr [esp + 018h] // 00c5bf59
        mov dword ptr [eax - 01ch], edx // 00c5bf5d
        fstp dword ptr [esp + 044h] // 00c5bf60
        mov edx, dword ptr [esp + 044h] // 00c5bf64
        fld dword ptr [esp + 014h] // 00c5bf68
        mov dword ptr [eax - 018h], edx // 00c5bf6c
        fstp dword ptr [esp + 048h] // 00c5bf6f
        mov edx, dword ptr [esp + 048h] // 00c5bf73
        mov dword ptr [eax - 014h], edx // 00c5bf77
        fld dword ptr [ecx + 010h] // 00c5bf7a
        fchs  // 00c5bf7d
        fstp dword ptr [esp + 078h] // 00c5bf7f
        fld dword ptr [ecx - 014h] // 00c5bf83
        fld dword ptr [esp + 078h] // 00c5bf86
        fld st(0) // 00c5bf8a
        fmulp st(2), st(0) // 00c5bf8c
        fxch st(1) // 00c5bf8e
        fstp dword ptr [esp + 078h] // 00c5bf90
        fld dword ptr [ecx - 010h] // 00c5bf94
        fmul st(0), st(1) // 00c5bf97
        fstp dword ptr [esp + 018h] // 00c5bf99
        fmul dword ptr [ecx - 0ch] // 00c5bf9d
        fstp dword ptr [esp + 014h] // 00c5bfa0
        fld dword ptr [esp + 078h] // 00c5bfa4
        fstp dword ptr [esp + 04ch] // 00c5bfa8
        mov edx, dword ptr [esp + 04ch] // 00c5bfac
        fld dword ptr [esp + 018h] // 00c5bfb0
        mov dword ptr [eax - 010h], edx // 00c5bfb4
        fstp dword ptr [esp + 050h] // 00c5bfb7
        mov edx, dword ptr [esp + 050h] // 00c5bfbb
        fld dword ptr [esp + 014h] // 00c5bfbf
        mov dword ptr [eax - 0ch], edx // 00c5bfc3
        fstp dword ptr [esp + 054h] // 00c5bfc6
        mov edx, dword ptr [esp + 054h] // 00c5bfca
        mov dword ptr [eax - 8], edx // 00c5bfce
        movss dword ptr [eax + 4], xmm0 // 00c5bfd1
        movss dword ptr [eax], xmm0 // 00c5bfd6
        movss dword ptr [eax - 4], xmm0 // 00c5bfda
        movss dword ptr [eax + 010h], xmm0 // 00c5bfdf
        movss dword ptr [eax + 0ch], xmm0 // 00c5bfe4
        movss dword ptr [eax + 8], xmm0 // 00c5bfe9
        add edi, 1 // 00c5bfee
        add ebx, 1 // 00c5bff1
        add eax, 058h // 00c5bff4
        add ecx, 030h // 00c5bff7
        cmp edi, dword ptr [esi + 0c8h] // 00c5bffa
        jl l_00c5bdb0 // 00c5c000
    l_00c5c006:
        mov eax, dword ptr [ebp + 0444h] // 00c5c006
        mov eax, dword ptr [eax + 0b0h] // 00c5c00c
        mov esi, dword ptr [esi + 0dch] // 00c5c012
        add eax, 0f0h // 00c5c018
        cmp esi, eax // 00c5c01d
        jne l_00c5bd90 // 00c5c01f
        jmp l_00c5c42d // 00c5c025
    l_00c5c02a:
        cmp dword ptr [ebp + 0458h], 0 // 00c5c02a
        je l_00c5c122 // 00c5c031
        push offset name_00d79f34 // 00c5c037
        mov eax, 018h // 00c5c03c
        lea edi, [esp + 050h] // 00c5c041
        push dword ptr [esp+132] // Borrowed context; original frame offsets retained.
        call enter_shim // 00c5c045
        mov dword ptr [esp + 070h], ebx // 00c5c04a
        mov eax, dword ptr [ebp + 0458h] // 00c5c04e
        mov ebx, dword ptr [ebp + 0460h] // 00c5c054
        cmp ebx, eax // 00c5c05a
        jge l_00c5c064 // 00c5c05c
        mov dword ptr [esp + 014h], ebx // 00c5c05e
        jmp l_00c5c06a // 00c5c062
    l_00c5c064:
        mov ebx, eax // 00c5c064
        mov dword ptr [esp + 014h], eax // 00c5c066
    l_00c5c06a:
        movss xmm0, dword ptr [esp + 07ch] // 00c5c06a
        cdq  // 00c5c070
        lea esi, [ebx - 1] // 00c5c071
        xor ecx, ecx // 00c5c074
        idiv ebx // 00c5c076
        xor edi, edi // 00c5c078
        test esi, esi // 00c5c07a
        jle l_00c5c0b8 // 00c5c07c
        xor edx, edx // 00c5c07e
        mov dword ptr [esp + 078h], esi // 00c5c080
        mov edi, esi // 00c5c084
        jmp l_00c5c090 // 00c5c086
        _emit 08dh // 00c5c088
        _emit 0a4h
        _emit 024h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
        nop  // 00c5c08f
    l_00c5c090:
        mov esi, dword ptr [ebp + 045ch] // 00c5c090
        lea ebx, [eax + ecx - 1] // 00c5c096
        mov dword ptr [edx + esi + 0ch], ecx // 00c5c09a
        mov dword ptr [edx + esi + 010h], ebx // 00c5c09e
        movss dword ptr [edx + esi + 014h], xmm0 // 00c5c0a2
        add ecx, eax // 00c5c0a8
        add edx, 018h // 00c5c0aa
        sub dword ptr [esp + 078h], 1 // 00c5c0ad
        jne l_00c5c090 // 00c5c0b2
        mov ebx, dword ptr [esp + 014h] // 00c5c0b4
    l_00c5c0b8:
        mov eax, dword ptr [ebp + 045ch] // 00c5c0b8
        mov edx, dword ptr [ebp + 0458h] // 00c5c0be
        lea esi, [edi + edi*2] // 00c5c0c4
        lea eax, [eax + esi*8] // 00c5c0c7
        mov dword ptr [eax + 0ch], ecx // 00c5c0ca
        mov ecx,dword ptr [esp+128] // 00c5c0cd: borrowed context.
        mov ecx,dword ptr [ecx+12]
        mov ecx,dword ptr [ecx]
        add edx, -1 // 00c5c0d3
        mov dword ptr [eax + 010h], edx // 00c5c0d6
        movss dword ptr [eax + 014h], xmm0 // 00c5c0d9
        mov eax, dword ptr [ebp + 0468h] // 00c5c0de
        mov esi, dword ptr [ecx + 010h] // 00c5c0e4
        push ebx // 00c5c0e7
        push dword ptr [esp+132] // Borrowed context; original frame offsets retained.
        call batch_shim // 00c5c0e8
        mov dword ptr [esp + 070h], 0ffffffffh // 00c5c0ed
        push dword ptr [esp+128] // Borrowed context; original frame offsets retained.
        call timestamp_00c5c0f5 // 00c5c0f5
        sub eax, dword ptr [esp + 04ch] // 00c5c0f7
        sbb edx, dword ptr [esp + 050h] // 00c5c0fb
        mov ecx, eax // 00c5c0ff
        mov eax, dword ptr [esp + 05ch] // 00c5c101
        add dword ptr [eax + 038h], ecx // 00c5c105
        mov dword ptr [eax + 030h], ecx // 00c5c108
        mov dword ptr [eax + 034h], edx // 00c5c10b
        adc dword ptr [eax + 03ch], edx // 00c5c10e
        add dword ptr [eax + 040h], 1 // 00c5c111
        mov eax,dword ptr [esp+128] // 00c5c115: borrowed context.
        mov eax,dword ptr [eax+8]
        mov eax,dword ptr [eax]
        mov edx, dword ptr [eax + 4] // 00c5c11a
        mov ecx, dword ptr [edx] // 00c5c11d
        mov dword ptr [eax + 4], ecx // 00c5c11f
    l_00c5c122:
        xor edi, edi // 00c5c122
        cmp dword ptr [ebp + 024h], edi // 00c5c124
        je l_00c5c456 // 00c5c127
        mov dword ptr [esp + 020h], edi // 00c5c12d
        mov dword ptr [esp + 024h], edi // 00c5c131
        mov dword ptr [esp + 070h], 2 // 00c5c135
        mov eax, dword ptr [ebp + 0444h] // 00c5c13d
        mov edx, dword ptr [eax + 0b0h] // 00c5c143
        mov eax, dword ptr [edx + 01d0h] // 00c5c149
        lea esi, [eax*4] // 00c5c14f
        cmp esi, edi // 00c5c156
        jle l_00c5c17f // 00c5c158
        xor ecx, ecx // 00c5c15a
        mov eax, esi // 00c5c15c
        mov edx, 058h // 00c5c15e
        mul edx // 00c5c163
        seto cl // 00c5c165
        neg ecx // 00c5c168
        or ecx, eax // 00c5c16a
        push ecx // 00c5c16c
        push dword ptr [esp+132] // Borrowed context; original frame offsets retained.
        call bridge_00c5c16d // 00c5c16d
        add esp, 4 // 00c5c172
        mov dword ptr [esp + 020h], eax // 00c5c175
        mov dword ptr [esp + 024h], esi // 00c5c179
        mov edi, eax // 00c5c17d
    l_00c5c17f:
        mov eax, dword ptr [ebp + 0444h] // 00c5c17f
        mov ecx, dword ptr [eax + 0b0h] // 00c5c185
        mov esi, dword ptr [ecx + 0ech] // 00c5c18b
        mov eax, ecx // 00c5c191
        add eax, 0f0h // 00c5c193
        xor ebx, ebx // 00c5c198
        cmp esi, eax // 00c5c19a
        je l_00c5c431 // 00c5c19c
        xorps xmm0, xmm0 // 00c5c1a2
        jmp l_00c5c1b0 // 00c5c1a5
        _emit 08dh // 00c5c1a7
        _emit 0a4h
        _emit 024h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
        mov edi, edi // 00c5c1ae
    l_00c5c1b0:
        xor edi, edi // 00c5c1b0
        cmp dword ptr [esi + 0c8h], edi // 00c5c1b2
        jle l_00c5c40e // 00c5c1b8
        mov ecx, dword ptr [esp + 020h] // 00c5c1be
        mov eax, ebx // 00c5c1c2
        imul eax, eax, 058h // 00c5c1c4
        lea eax, [eax + ecx + 040h] // 00c5c1c7
        lea ecx, [esi + 01ch] // 00c5c1cb
        mov edi, edi // 00c5c1ce
    l_00c5c1d0:
        mov edx, dword ptr [esi + 0cch] // 00c5c1d0
        fld dword ptr [ecx - 4] // 00c5c1d6
        fstp dword ptr [esp + 078h] // 00c5c1d9
        fld dword ptr [ecx - 8] // 00c5c1dd
        fstp dword ptr [esp + 018h] // 00c5c1e0
        fld dword ptr [ecx] // 00c5c1e4
        fstp dword ptr [esp + 014h] // 00c5c1e6
        fld dword ptr [edx + 014h] // 00c5c1ea
        fld dword ptr [esp + 078h] // 00c5c1ed
        fld st(0) // 00c5c1f1
        fmulp st(2), st(0) // 00c5c1f3
        fld dword ptr [esp + 018h] // 00c5c1f5
        fld st(0) // 00c5c1f9
        fmul dword ptr [edx + 8] // 00c5c1fb
        faddp st(3), st(0) // 00c5c1fe
        fld dword ptr [edx + 020h] // 00c5c200
        fld dword ptr [esp + 014h] // 00c5c203
        fld st(0) // 00c5c207
        fmulp st(2), st(0) // 00c5c209
        fxch st(4) // 00c5c20b
        faddp st(1), st(0) // 00c5c20d
        fadd dword ptr [edx + 02ch] // 00c5c20f
        fstp dword ptr [esp + 078h] // 00c5c212
        fld dword ptr [edx + 0ch] // 00c5c216
        movss xmm1, dword ptr [esp + 078h] // 00c5c219
        fmul st(0), st(1) // 00c5c21f
        movss dword ptr [esp + 04ch], xmm1 // 00c5c221
        fld dword ptr [edx + 018h] // 00c5c227
        fmul st(0), st(3) // 00c5c22a
        faddp st(1), st(0) // 00c5c22c
        fld dword ptr [edx + 024h] // 00c5c22e
        fmul st(0), st(4) // 00c5c231
        faddp st(1), st(0) // 00c5c233
        fadd dword ptr [edx + 030h] // 00c5c235
        fstp dword ptr [esp + 018h] // 00c5c238
        fmul dword ptr [edx + 010h] // 00c5c23c
        fld dword ptr [edx + 01ch] // 00c5c23f
        fmulp st(2), st(0) // 00c5c242
        faddp st(1), st(0) // 00c5c244
        fld dword ptr [edx + 028h] // 00c5c246
        fmulp st(2), st(0) // 00c5c249
        faddp st(1), st(0) // 00c5c24b
        fadd dword ptr [edx + 034h] // 00c5c24d
        mov edx, dword ptr [esp + 04ch] // 00c5c250
        mov dword ptr [eax - 040h], edx // 00c5c254
        fstp dword ptr [esp + 014h] // 00c5c257
        fld dword ptr [esp + 018h] // 00c5c25b
        fstp dword ptr [esp + 050h] // 00c5c25f
        mov edx, dword ptr [esp + 050h] // 00c5c263
        fld dword ptr [esp + 014h] // 00c5c267
        mov dword ptr [eax - 03ch], edx // 00c5c26b
        fstp dword ptr [esp + 054h] // 00c5c26e
        mov edx, dword ptr [esp + 054h] // 00c5c272
        mov dword ptr [eax - 038h], edx // 00c5c276
        fld dword ptr [ecx + 8] // 00c5c279
        mov edx, dword ptr [esi + 0d0h] // 00c5c27c
        fstp dword ptr [esp + 078h] // 00c5c282
        fld dword ptr [ecx + 4] // 00c5c286
        fstp dword ptr [esp + 018h] // 00c5c289
        fld dword ptr [ecx + 0ch] // 00c5c28d
        fstp dword ptr [esp + 014h] // 00c5c290
        fld dword ptr [edx + 014h] // 00c5c294
        fld dword ptr [esp + 078h] // 00c5c297
        fld st(0) // 00c5c29b
        fmulp st(2), st(0) // 00c5c29d
        fld dword ptr [esp + 018h] // 00c5c29f
        fld st(0) // 00c5c2a3
        fmul dword ptr [edx + 8] // 00c5c2a5
        faddp st(3), st(0) // 00c5c2a8
        fld dword ptr [edx + 020h] // 00c5c2aa
        fld dword ptr [esp + 014h] // 00c5c2ad
        fld st(0) // 00c5c2b1
        fmulp st(2), st(0) // 00c5c2b3
        fxch st(4) // 00c5c2b5
        faddp st(1), st(0) // 00c5c2b7
        fadd dword ptr [edx + 02ch] // 00c5c2b9
        fstp dword ptr [esp + 078h] // 00c5c2bc
        fld dword ptr [edx + 0ch] // 00c5c2c0
        fmul st(0), st(1) // 00c5c2c3
        fld dword ptr [edx + 018h] // 00c5c2c5
        fmul st(0), st(3) // 00c5c2c8
        faddp st(1), st(0) // 00c5c2ca
        movss xmm1, dword ptr [esp + 078h] // 00c5c2cc
        fld dword ptr [edx + 024h] // 00c5c2d2
        movss dword ptr [esp + 040h], xmm1 // 00c5c2d5
        fmul st(0), st(4) // 00c5c2db
        faddp st(1), st(0) // 00c5c2dd
        fadd dword ptr [edx + 030h] // 00c5c2df
        fstp dword ptr [esp + 018h] // 00c5c2e2
        fmul dword ptr [edx + 010h] // 00c5c2e6
        fld dword ptr [edx + 01ch] // 00c5c2e9
        fmulp st(2), st(0) // 00c5c2ec
        faddp st(1), st(0) // 00c5c2ee
        fld dword ptr [edx + 028h] // 00c5c2f0
        fmulp st(2), st(0) // 00c5c2f3
        faddp st(1), st(0) // 00c5c2f5
        fadd dword ptr [edx + 034h] // 00c5c2f7
        mov edx, dword ptr [esp + 040h] // 00c5c2fa
        mov dword ptr [eax - 034h], edx // 00c5c2fe
        fstp dword ptr [esp + 014h] // 00c5c301
        fld dword ptr [esp + 018h] // 00c5c305
        fstp dword ptr [esp + 044h] // 00c5c309
        mov edx, dword ptr [esp + 044h] // 00c5c30d
        fld dword ptr [esp + 014h] // 00c5c311
        mov dword ptr [eax - 030h], edx // 00c5c315
        fstp dword ptr [esp + 048h] // 00c5c318
        mov edx, dword ptr [esp + 048h] // 00c5c31c
        mov dword ptr [eax - 02ch], edx // 00c5c320
        mov edx, dword ptr [ecx - 014h] // 00c5c323
        mov dword ptr [eax - 028h], edx // 00c5c326
        mov edx, dword ptr [ecx - 010h] // 00c5c329
        mov dword ptr [eax - 024h], edx // 00c5c32c
        mov edx, dword ptr [ecx - 0ch] // 00c5c32f
        mov dword ptr [eax - 020h], edx // 00c5c332
        fld dword ptr [ecx + 010h] // 00c5c335
        fstp dword ptr [esp + 078h] // 00c5c338
        fld dword ptr [esp + 078h] // 00c5c33c
        fld st(0) // 00c5c340
        fmul dword ptr [ecx - 014h] // 00c5c342
        fstp dword ptr [esp + 078h] // 00c5c345
        fld st(0) // 00c5c349
        fmul dword ptr [ecx - 010h] // 00c5c34b
        fstp dword ptr [esp + 018h] // 00c5c34e
        fmul dword ptr [ecx - 0ch] // 00c5c352
        fstp dword ptr [esp + 014h] // 00c5c355
        fld dword ptr [esp + 078h] // 00c5c359
        fstp dword ptr [esp + 034h] // 00c5c35d
        mov edx, dword ptr [esp + 034h] // 00c5c361
        fld dword ptr [esp + 018h] // 00c5c365
        mov dword ptr [eax - 01ch], edx // 00c5c369
        fstp dword ptr [esp + 038h] // 00c5c36c
        mov edx, dword ptr [esp + 038h] // 00c5c370
        fld dword ptr [esp + 014h] // 00c5c374
        mov dword ptr [eax - 018h], edx // 00c5c378
        fstp dword ptr [esp + 03ch] // 00c5c37b
        mov edx, dword ptr [esp + 03ch] // 00c5c37f
        mov dword ptr [eax - 014h], edx // 00c5c383
        fld dword ptr [ecx + 010h] // 00c5c386
        fchs  // 00c5c389
        fstp dword ptr [esp + 078h] // 00c5c38b
        fld dword ptr [esp + 078h] // 00c5c38f
        fld st(0) // 00c5c393
        fmul dword ptr [ecx - 014h] // 00c5c395
        fstp dword ptr [esp + 078h] // 00c5c398
        fld st(0) // 00c5c39c
        fmul dword ptr [ecx - 010h] // 00c5c39e
        fstp dword ptr [esp + 018h] // 00c5c3a1
        fmul dword ptr [ecx - 0ch] // 00c5c3a5
        fstp dword ptr [esp + 014h] // 00c5c3a8
        fld dword ptr [esp + 078h] // 00c5c3ac
        fstp dword ptr [esp + 028h] // 00c5c3b0
        mov edx, dword ptr [esp + 028h] // 00c5c3b4
        fld dword ptr [esp + 018h] // 00c5c3b8
        mov dword ptr [eax - 010h], edx // 00c5c3bc
        fstp dword ptr [esp + 02ch] // 00c5c3bf
        mov edx, dword ptr [esp + 02ch] // 00c5c3c3
        fld dword ptr [esp + 014h] // 00c5c3c7
        mov dword ptr [eax - 0ch], edx // 00c5c3cb
        fstp dword ptr [esp + 030h] // 00c5c3ce
        mov edx, dword ptr [esp + 030h] // 00c5c3d2
        mov dword ptr [eax - 8], edx // 00c5c3d6
        movss dword ptr [eax + 4], xmm0 // 00c5c3d9
        movss dword ptr [eax], xmm0 // 00c5c3de
        movss dword ptr [eax - 4], xmm0 // 00c5c3e2
        movss dword ptr [eax + 010h], xmm0 // 00c5c3e7
        movss dword ptr [eax + 0ch], xmm0 // 00c5c3ec
        movss dword ptr [eax + 8], xmm0 // 00c5c3f1
        add edi, 1 // 00c5c3f6
        add ebx, 1 // 00c5c3f9
        add eax, 058h // 00c5c3fc
        add ecx, 030h // 00c5c3ff
        cmp edi, dword ptr [esi + 0c8h] // 00c5c402
        jl l_00c5c1d0 // 00c5c408
    l_00c5c40e:
        mov eax, dword ptr [ebp + 0444h] // 00c5c40e
        mov eax, dword ptr [eax + 0b0h] // 00c5c414
        mov esi, dword ptr [esi + 0dch] // 00c5c41a
        add eax, 0f0h // 00c5c420
        cmp esi, eax // 00c5c425
        jne l_00c5c1b0 // 00c5c427
    l_00c5c42d:
        mov edi, dword ptr [esp + 020h] // 00c5c42d
    l_00c5c431:
        mov ecx, dword ptr [ebp + 024h] // 00c5c431
        mov eax, dword ptr [ecx] // 00c5c434
        mov edx, dword ptr [eax] // 00c5c436
        push ebx // 00c5c438
        push edi // 00c5c439
        call edx // 00c5c43a
        test edi, edi // 00c5c43c
        mov dword ptr [esp + 070h], 0ffffffffh // 00c5c43e
        je l_00c5c451 // 00c5c446
        push edi // 00c5c448
        push dword ptr [esp+132] // Borrowed context; original frame offsets retained.
        call bridge_00c5c449 // 00c5c449
        add esp, 4 // 00c5c44e
    l_00c5c451:
        mov eax,dword ptr [esp+128] // 00c5c451: borrowed context.
        mov eax,dword ptr [eax+8]
        mov eax,dword ptr [eax]
    l_00c5c456:
        cmp dword ptr [eax + 078h], 0 // 00c5c456
        lea edi, [eax + 078h] // 00c5c45a
        jne l_00c5c475 // 00c5c45d
        mov esi, dword ptr [eax + 4] // 00c5c45f
        push 01bh // 00c5c462
        push offset name_00d79f3c // 00c5c464
        push dword ptr [esp+136] // Borrowed context; original frame offsets retained.
        call append_shim // 00c5c469
        mov dword ptr [edi], eax // 00c5c46e
        mov eax,dword ptr [esp+128] // 00c5c470: borrowed context.
        mov eax,dword ptr [eax+8]
        mov eax,dword ptr [eax]
    l_00c5c475:
        mov esi, dword ptr [edi] // 00c5c475
        mov ecx, dword ptr [eax + 4] // 00c5c477
        mov dword ptr [esi], ecx // 00c5c47a
        mov dword ptr [eax + 4], esi // 00c5c47c
        push dword ptr [esp+128] // Borrowed context; original frame offsets retained.
        call timestamp_00c5c47f // 00c5c47f
        fld dword ptr [esp + 07ch] // 00c5c481
        push ecx // 00c5c485
        mov ebx, ebp // 00c5c486
        fstp dword ptr [esp] // 00c5c488
        mov dword ptr [esp + 054h], edx // 00c5c48b
        mov edi, eax // 00c5c48f
        push dword ptr [esp+132] // Borrowed context; original frame offsets retained.
        call position_shim // 00c5c491
        push dword ptr [esp+128] // Borrowed context; original frame offsets retained.
        call timestamp_00c5c496 // 00c5c496
        sub eax, edi // 00c5c498
        sbb edx, dword ptr [esp + 050h] // 00c5c49a
        add dword ptr [esi + 038h], eax // 00c5c49e
        mov dword ptr [esi + 030h], eax // 00c5c4a1
        mov eax,dword ptr [esp+128] // 00c5c4a4: borrowed context.
        mov eax,dword ptr [eax+8]
        mov eax,dword ptr [eax]
        adc dword ptr [esi + 03ch], edx // 00c5c4a9
        mov dword ptr [esi + 034h], edx // 00c5c4ac
        mov ebp, 1 // 00c5c4af
        add dword ptr [esi + 040h], ebp // 00c5c4b4
        mov edx, dword ptr [eax + 4] // 00c5c4b7
        mov esi, dword ptr [edx] // 00c5c4ba
        lea edi, [eax + 07ch] // 00c5c4bc
        mov dword ptr [eax + 4], esi // 00c5c4bf
        cmp dword ptr [edi], 0 // 00c5c4c2
        jne l_00c5c4da // 00c5c4c5
        push 01ch // 00c5c4c7
        push offset name_00d79f4c // 00c5c4c9
        push dword ptr [esp+136] // Borrowed context; original frame offsets retained.
        call append_shim // 00c5c4ce
        mov dword ptr [edi], eax // 00c5c4d3
        mov eax,dword ptr [esp+128] // 00c5c4d5: borrowed context.
        mov eax,dword ptr [eax+8]
        mov eax,dword ptr [eax]
    l_00c5c4da:
        mov esi, dword ptr [edi] // 00c5c4da
        mov ecx, dword ptr [eax + 4] // 00c5c4dc
        mov dword ptr [esi], ecx // 00c5c4df
        mov dword ptr [esp + 05ch], esi // 00c5c4e1
        mov dword ptr [eax + 4], esi // 00c5c4e5
        push dword ptr [esp+128] // Borrowed context; original frame offsets retained.
        call timestamp_00c5c4e8 // 00c5c4e8
        mov ebx, edx // 00c5c4ea
        mov edi, eax // 00c5c4ec
        mov dword ptr [esp + 050h], ebx // 00c5c4ee
        mov dword ptr [esp + 04ch], edi // 00c5c4f2
        mov edx, dword ptr [esp + 01ch] // 00c5c4f6
        push edx // 00c5c4fa
        mov dword ptr [esp + 074h], 5 // 00c5c4fb
        push dword ptr [esp+132] // Borrowed context; original frame offsets retained.
        call sleep_shim // 00c5c503
        push dword ptr [esp+128] // Borrowed context; original frame offsets retained.
        call timestamp_00c5c508 // 00c5c508
        sub eax, edi // 00c5c50a
        sbb edx, ebx // 00c5c50c
        add dword ptr [esi + 038h], eax // 00c5c50e
        mov dword ptr [esi + 030h], eax // 00c5c511
        mov eax,dword ptr [esp+128] // 00c5c514: borrowed context.
        mov eax,dword ptr [eax+8]
        mov eax,dword ptr [eax]
        adc dword ptr [esi + 03ch], edx // 00c5c519
        mov dword ptr [esi + 034h], edx // 00c5c51c
        add dword ptr [esi + 040h], ebp // 00c5c51f
        mov ecx, dword ptr [eax + 4] // 00c5c522
        mov edx, dword ptr [ecx] // 00c5c525
        mov ecx, dword ptr [esp + 068h] // 00c5c527
        pop edi // 00c5c52b
        pop esi // 00c5c52c
        pop ebp // 00c5c52d
        mov dword ptr [eax + 4], edx // 00c5c52e
        // 00c5c531: native FS write omitted; normal-return contract.
        pop ebx // 00c5c538
        add esp, 064h // 00c5c539
        ret 12 // 00c5c53c
    }
}
// Full normal native instruction schedule; addresses identify original starts.
__declspec(naked) void simulate_kernel(){
    __asm {
        push ebp // 00c5c540
        mov ebp, esp // 00c5c541
        and esp, 0fffffff8h // 00c5c543
        push -1 // 00c5c546
        push 0 // 00c5c548
        mov eax, dword ptr fs:[0] // 00c5c54d
        push eax // 00c5c553
        // 00c5c554: native FS write omitted; normal-return contract.
        sub esp, 038h // 00c5c55b
        mov eax,dword ptr [ebp+12] // 00c5c55e: borrowed context.
        mov eax,dword ptr [eax+12]
        mov eax,dword ptr [eax]
        mov eax, dword ptr [eax + 0ch] // 00c5c563
        push ebx // 00c5c566
        push esi // 00c5c567
        mov esi, dword ptr [eax] // 00c5c568
        mov ebx, dword ptr [esi + 4] // 00c5c56a
        push edi // 00c5c56d
        mov edi, ecx // 00c5c56e
        mov ecx, dword ptr [esi + 8] // 00c5c570
        mov edx, ebx // 00c5c573
        lea ecx, [edx + ecx*4] // 00c5c575
        cmp ebx, ecx // 00c5c578
        mov dword ptr [esp + 010h], edi // 00c5c57a
        mov dword ptr [esp + 014h], eax // 00c5c57e
        je l_00c5c59f // 00c5c582
    l_00c5c584:
        mov ecx, dword ptr [ebx] // 00c5c584
        call reset_kernel // 00c5c586
        mov edx, dword ptr [esi + 8] // 00c5c58b
        mov eax, dword ptr [esi + 4] // 00c5c58e
        add ebx, 4 // 00c5c591
        lea ecx, [eax + edx*4] // 00c5c594
        cmp ebx, ecx // 00c5c597
        jne l_00c5c584 // 00c5c599
        mov eax, dword ptr [esp + 014h] // 00c5c59b
    l_00c5c59f:
        xor ecx, ecx // 00c5c59f
        mov dword ptr [esi + 038h], ecx // 00c5c5a1
        mov dword ptr [esi + 03ch], ecx // 00c5c5a4
        mov dword ptr [esi + 040h], ecx // 00c5c5a7
        add dword ptr [eax + 8], 1 // 00c5c5aa
        mov eax,dword ptr [ebp+12] // 00c5c5ae: borrowed context.
        mov eax,dword ptr [eax+8]
        mov eax,dword ptr [eax]
        cmp dword ptr [eax + 010h], ecx // 00c5c5b3
        lea ebx, [eax + 010h] // 00c5c5b6
        jne l_00c5c5d1 // 00c5c5b9
        mov esi, dword ptr [eax + 4] // 00c5c5bb
        push 1 // 00c5c5be
        push offset name_00d79f18 // 00c5c5c0
        push dword ptr [ebp+12] // Borrowed context; original frame offsets retained.
        call append_shim // 00c5c5c5
        mov dword ptr [ebx], eax // 00c5c5ca
        mov eax,dword ptr [ebp+12] // 00c5c5cc: borrowed context.
        mov eax,dword ptr [eax+8]
        mov eax,dword ptr [eax]
    l_00c5c5d1:
        mov ebx, dword ptr [ebx] // 00c5c5d1
        mov edx, dword ptr [eax + 4] // 00c5c5d3
        mov dword ptr [ebx], edx // 00c5c5d6
        mov dword ptr [esp + 038h], ebx // 00c5c5d8
        mov dword ptr [eax + 4], ebx // 00c5c5dc
        push dword ptr [ebp+12] // Borrowed context; original frame offsets retained.
        call timestamp_00c5c5df // 00c5c5df
        mov esi, eax // 00c5c5e1
        mov dword ptr [esp + 02ch], edx // 00c5c5e3
        mov dword ptr [esp + 028h], esi // 00c5c5e7
        mov dword ptr [esp + 04ch], 0 // 00c5c5eb
        push dword ptr [ebp+12] // Borrowed context; original frame offsets retained.
        call timestamp_00c5c5f3 // 00c5c5f3
        add dword ptr [edi + 02ch], 1 // 00c5c5f5
        mov ecx, edi // 00c5c5f9
        mov dword ptr [esp + 01ch], edx // 00c5c5fb
        push dword ptr [ebp+12] // Borrowed context; original frame offsets retained.
        call flush_kernel // 00c5c5ff
        mov eax, dword ptr [edi + 0204h] // 00c5c604
        lea edx, [edi + 0208h] // 00c5c60a
        cmp eax, edx // 00c5c610
        je l_00c5c639 // 00c5c612
    l_00c5c614:
        mov edi, dword ptr [eax + 4] // 00c5c614
        lea esi, [eax + 8] // 00c5c617
        add edi, 084h // 00c5c61a
        mov ecx, 0ch // 00c5c620
        rep movsd // 00c5c625
        mov eax, dword ptr [eax + 084h] // 00c5c627
        cmp eax, edx // 00c5c62d
        jne l_00c5c614 // 00c5c62f
        mov edi, dword ptr [esp + 010h] // 00c5c631
        mov esi, dword ptr [esp + 028h] // 00c5c635
    l_00c5c639:
        fld dword ptr [ebp + 8] // 00c5c639
        mov eax, dword ptr [edi + 034h] // 00c5c63c
        fadd dword ptr [edi + 048h] // 00c5c63f
        mov dword ptr [esp + 010h], eax // 00c5c642
        fstp dword ptr [esp + 014h] // 00c5c646
        fld dword ptr [esp + 014h] // 00c5c64a
        fst dword ptr [edi + 048h] // 00c5c64e
        fld dword ptr [edi] // 00c5c651
        fstp dword ptr [esp + 014h] // 00c5c653
        fld dword ptr [esp + 014h] // 00c5c657
        fxch st(1) // 00c5c65b
        fcomip st(0), st(1) // 00c5c65d
        jbe l_00c5c698 // 00c5c65f
    l_00c5c661:
        cmp dword ptr [esp + 010h], 0 // 00c5c661
        je l_00c5c6c4 // 00c5c666
        push ecx // 00c5c668
        fstp dword ptr [esp] // 00c5c669
        push edi // 00c5c66c
        push dword ptr [ebp+12] // Borrowed context; original frame offsets retained.
        call substep_shim // 00c5c66d
        fld dword ptr [edi] // 00c5c672
        sub dword ptr [esp + 010h], 1 // 00c5c674
        fstp dword ptr [esp + 014h] // 00c5c679
        fld dword ptr [edi + 048h] // 00c5c67d
        fld dword ptr [esp + 014h] // 00c5c680
        fld st(0) // 00c5c684
        fsubp st(2), st(0) // 00c5c686
        fxch st(1) // 00c5c688
        fstp dword ptr [edi + 048h] // 00c5c68a
        fld dword ptr [edi] // 00c5c68d
        fld dword ptr [edi + 048h] // 00c5c68f
        fcomip st(0), st(1) // 00c5c692
        fstp st(0) // 00c5c694
        ja l_00c5c661 // 00c5c696
    l_00c5c698:
        cmp dword ptr [esp + 010h], 0 // 00c5c698
        fstp st(0) // 00c5c69d
        je l_00c5c6c6 // 00c5c69f
        fld dword ptr [edi + 048h] // 00c5c6a1
        fstp dword ptr [esp + 014h] // 00c5c6a4
        fld qword ptr constant_00d7a398 // 00c5c6a8
        fld dword ptr [esp + 014h] // 00c5c6ae
        fcomi st(0), st(1) // 00c5c6b2
        fstp st(1) // 00c5c6b4
        jbe l_00c5c6c4 // 00c5c6b6
        push ecx // 00c5c6b8
        fstp dword ptr [esp] // 00c5c6b9
        push edi // 00c5c6bc
        push dword ptr [ebp+12] // Borrowed context; original frame offsets retained.
        call substep_shim // 00c5c6bd
        jmp l_00c5c6c6 // 00c5c6c2
    l_00c5c6c4:
        fstp st(0) // 00c5c6c4
    l_00c5c6c6:
        xorps xmm0, xmm0 // 00c5c6c6
        movss dword ptr [edi + 048h], xmm0 // 00c5c6c9
        push dword ptr [ebp+12] // Borrowed context; original frame offsets retained.
        call timestamp_00c5c6ce // 00c5c6ce
        sub eax, esi // 00c5c6d0
        sbb edx, dword ptr [esp + 02ch] // 00c5c6d2
        add dword ptr [ebx + 038h], eax // 00c5c6d6
        mov dword ptr [ebx + 030h], eax // 00c5c6d9
        mov eax,dword ptr [ebp+12] // 00c5c6dc: borrowed context.
        mov eax,dword ptr [eax+8]
        mov eax,dword ptr [eax]
        adc dword ptr [ebx + 03ch], edx // 00c5c6e1
        mov dword ptr [ebx + 034h], edx // 00c5c6e4
        add dword ptr [ebx + 040h], 1 // 00c5c6e7
        mov ecx, dword ptr [eax + 4] // 00c5c6eb
        mov edx, dword ptr [ecx] // 00c5c6ee
        mov ecx, dword ptr [esp + 044h] // 00c5c6f0
        pop edi // 00c5c6f4
        pop esi // 00c5c6f5
        mov dword ptr [eax + 4], edx // 00c5c6f6
        // 00c5c6f9: native FS write omitted; normal-return contract.
        pop ebx // 00c5c700
        mov esp, ebp // 00c5c701
        pop ebp // 00c5c703
        ret 8 // 00c5c704
    }
}
// Full normal native instruction schedule; addresses identify original starts.
__declspec(naked) void flush_kernel(){
    __asm {
        push ecx // 00c4d980
        push ebx // 00c4d981
        mov ebx, ecx // 00c4d982
        mov eax, dword ptr [ebx + 043ch] // 00c4d984
        push ebp // 00c4d98a
        xor ebp, ebp // 00c4d98b
        test eax, eax // 00c4d98d
        mov dword ptr [esp + 8], eax // 00c4d98f
        jle l_00c4da5d // 00c4d993
        push esi // 00c4d999
        push edi // 00c4d99a
        jmp l_00c4d9a0 // 00c4d99b
        _emit 08dh // 00c4d99d
        _emit 049h
        _emit 000h
    l_00c4d9a0:
        mov eax, dword ptr [ebx + 0438h] // 00c4d9a0
        mov esi, dword ptr [eax + ebp*4] // 00c4d9a6
        cmp dword ptr [esi + 070h], 0 // 00c4d9a9
        je l_00c4d9d3 // 00c4d9ad
        mov ecx, dword ptr [esi] // 00c4d9af
        mov eax, dword ptr [ecx + 0444h] // 00c4d9b1
        mov ecx, dword ptr [eax + 0ach] // 00c4d9b7
        mov edx, dword ptr [ecx] // 00c4d9bd
        mov eax, dword ptr [esi + 060h] // 00c4d9bf
        mov edx, dword ptr [edx + 4] // 00c4d9c2
        push eax // 00c4d9c5
        call edx // 00c4d9c6
        and dword ptr [esi + 050h], 0fffffff7h // 00c4d9c8
        mov dword ptr [esi + 060h], 0 // 00c4d9cc
    l_00c4d9d3:
        mov ecx, dword ptr [esi + 070h] // 00c4d9d3
        test ecx, ecx // 00c4d9d6
        je l_00c4d9f5 // 00c4d9d8
        _emit 08dh // 00c4d9da
        _emit 09bh
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    l_00c4d9e0:
        mov eax, dword ptr [ecx] // 00c4d9e0
        mov edx, dword ptr [eax + 4] // 00c4d9e2
        mov edi, dword ptr [ecx + 0208h] // 00c4d9e5
        push 1 // 00c4d9eb
        call edx // 00c4d9ed
        test edi, edi // 00c4d9ef
        mov ecx, edi // 00c4d9f1
        jne l_00c4d9e0 // 00c4d9f3
    l_00c4d9f5:
        push esi // 00c4d9f5
        mov dword ptr [esi + 070h], 0 // 00c4d9f6
        push dword ptr [esp+28] // Borrowed context; original frame offsets retained.
        call clear_shim // 00c4d9fd
        test byte ptr [esi + 050h], 1 // 00c4da02
        lea edi, [ebx + 04ch] // 00c4da06
        jne l_00c4da11 // 00c4da09
        lea edi, [ebx + 0170h] // 00c4da0b
    l_00c4da11:
        push esi // 00c4da11
        push dword ptr [esp+28] // Borrowed context; original frame offsets retained.
        call storage_shim // 00c4da12
        mov eax, dword ptr [esi + 084h] // 00c4da17
        mov ecx, dword ptr [esi + 080h] // 00c4da1d
        mov dword ptr [eax + 080h], ecx // 00c4da23
        mov eax, dword ptr [esi + 084h] // 00c4da29
        mov edx, dword ptr [esi + 080h] // 00c4da2f
        mov dword ptr [edx + 084h], eax // 00c4da35
        mov ecx, dword ptr [edi + 0ch] // 00c4da3b
        mov dword ptr [esi + 084h], ecx // 00c4da3e
        add dword ptr [edi + 0120h], -1 // 00c4da44
        add ebp, 1 // 00c4da4b
        cmp ebp, dword ptr [esp + 010h] // 00c4da4e
        mov dword ptr [edi + 0ch], esi // 00c4da52
        jl l_00c4d9a0 // 00c4da55
        pop edi // 00c4da5b
        pop esi // 00c4da5c
    l_00c4da5d:
        pop ebp // 00c4da5d
        mov dword ptr [ebx + 043ch], 0 // 00c4da5e
        pop ebx // 00c4da68
        pop ecx // 00c4da69
        ret 4 // 00c4da6a
    }
}
// Full normal native instruction schedule; addresses identify original starts.
__declspec(naked) void reset_kernel(){
    __asm {
        push esi // 00c321b0
        mov esi, ecx // 00c321b1
        mov eax, dword ptr [esi + 8] // 00c321b3
        push edi // 00c321b6
        mov edi, dword ptr [esi + 4] // 00c321b7
        mov ecx, edi // 00c321ba
        lea edx, [ecx + eax*4] // 00c321bc
        cmp edi, edx // 00c321bf
        je l_00c321da // 00c321c1
    l_00c321c3:
        mov ecx, dword ptr [edi] // 00c321c3
        call reset_kernel // 00c321c5
        mov eax, dword ptr [esi + 8] // 00c321ca
        mov ecx, dword ptr [esi + 4] // 00c321cd
        add edi, 4 // 00c321d0
        lea edx, [ecx + eax*4] // 00c321d3
        cmp edi, edx // 00c321d6
        jne l_00c321c3 // 00c321d8
    l_00c321da:
        xor eax, eax // 00c321da
        pop edi // 00c321dc
        mov dword ptr [esi + 038h], eax // 00c321dd
        mov dword ptr [esi + 03ch], eax // 00c321e0
        mov dword ptr [esi + 040h], eax // 00c321e3
        pop esi // 00c321e6
        ret  // 00c321e7
    }
}
} // namespace
void NativeDynWorldStepCalls::integrate_velocities_00c41550(void* w,float dt){native_dyn_integrate_velocities_00c41550(w,dt);}
void NativeDynWorldStepCalls::collision_pass_00c57070(void* s,const NativeDynCollisionPassContext& c){run_native_dyn_collision_pass_00c57070(s,c);}
void NativeDynWorldStepCalls::create_groups_00c4b610(void* m,const NativeDynContactGroupContext& c){native_dyn_create_contact_groups_00c4b610(m,c);}
void NativeDynWorldStepCalls::integrate_positions_00c5b1b0(void* w,float dt,const CameraAxesCrtAccess& c){native_dyn_integrate_positions_00c5b1b0(w,dt,c);}
void NativeDynWorldStepCalls::sleep_groups_00c4b550(void* m,const NativeDynContactGroupContext& c){native_dyn_sleep_contact_groups_00c4b550(m,c);}
DynProfileScopeStorage* NativeDynWorldStepCalls::enter_profile_00c57020(DynProfileScopeStorage& s,U id,const char* name,const DynProfileScopeContext& c){return dyn_profile_scope_enter_00c57020(s,id,name,c);}
void reset_native_dyn_profile_tree_00c321b0(void* node){__asm {mov ecx,node} __asm {call reset_kernel}}
void flush_native_dyn_pending_bodies_00c4d980(void* world,const NativeDynWorldStepContext& c){Context context=bind(c);auto* p=&context;__asm {push p} __asm {mov ecx,world} __asm {call flush_kernel}}
void run_native_dyn_world_substep_00c5bb30(void* world,float dt,const NativeDynWorldStepContext& c){Context context=bind(c);auto* p=&context;__asm {push p} __asm {push dt} __asm {push world} __asm {call substep_kernel}}
void simulate_native_dyn_world_00c5c540(void* world,float dt,const NativeDynWorldStepContext& c){Context context=bind(c);auto* p=&context;__asm {push p} __asm {push dt} __asm {mov ecx,world} __asm {call simulate_kernel}}
} // namespace bsp

#include "bsp/native_dyn_collision_pass.hpp"
#include "bsp/dyn_task_manager.hpp"
#include <intrin.h>
#include <type_traits>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native collision pass requires MSVC Win32 x87/SSE assembly.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;
struct Context {const AvoidZoneDynHullMemory* memory;NativeDynCollisionPassCalls* calls;void* const* profile_slot;void* const* engine_slot;const AvoidZoneDynHullMemory* profile_memory;};
static_assert(std::is_standard_layout_v<Context> && offsetof(Context,profile_slot)==8 && offsetof(Context,engine_slot)==12);
Context bind(const NativeDynCollisionPassContext& c){return {&c.memory,&c.calls,c.profile.profile_slot_0109e9f8,c.engine_slot_0109e9fc,&c.profile.memory};}
void* __cdecl allocate_bridge(U site,Context* c,U size){return c->calls->allocate_00bf55be(site,size,*c->memory);}
void __cdecl free_bridge(U site,Context* c,void* p){c->calls->free_00bf6989(site,p,*c->memory);}
void* __cdecl append_bridge(Context* c,DynProfileNodeStorage* p,const char* name,U id){return dyn_profile_node_append_child_00c50390(*p,name,id,*c->profile_memory);}
std::uint64_t __cdecl timestamp_bridge(U site,Context* c) noexcept{return c->calls->read_timestamp(site);}
void __cdecl remove_bridge(Context* c,void* body,void* manifold){NativeGamePhysicsLifetimeProgress progress;remove_native_dyn_manifold_reference_00c37d30(body,manifold,*c->memory,*c->calls,progress);}
void __cdecl batch_bridge(Context* c,void* manager,void* const* tasks,std::int32_t count){c->calls->run_batch_00c33140(manager,tasks,count,*c->memory);}
void pass_kernel();
void update_kernel();
void refresh_kernel();
void events_kernel();
void abs_kernel();
alignas(8) const std::uint64_t constant_00d7a280=0x3fe0000000000000ULL;
alignas(8) const std::uint32_t constant_00d7a2d8=0x3b23d70bU;
alignas(8) const std::uint32_t constant_00d7a320=0xbca3d70aU;
const char name_00d79d50[]={67,111,108,108,105,100,101,0};
const char name_00d79d58[]={66,114,111,97,100,80,104,97,115,101,0};
const char name_00d79d64[]={66,114,111,97,100,80,104,97,115,101,85,112,100,97,116,101,0};
const char name_00d79d78[]={77,97,110,105,102,111,108,100,85,112,100,97,116,101,0};
const char name_00d79d88[]={73,110,116,101,114,115,101,99,116,76,111,111,112,0};
const char name_00d79d98[]={71,101,116,77,97,110,105,102,111,108,100,0};
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
__declspec(naked) void update_shim(){
    __asm {
        push dword ptr [esp+4]
        push dword ptr [esp+0ch]
        call update_kernel
        ret 8
    }
}
__declspec(naked) void remove_shim(){
    __asm {
        push edx
        push esi
        push dword ptr [esp+0ch]
        call remove_bridge
        add esp,0ch
        ret 4
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
__declspec(naked) void timestamp_00c570cb(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c570cbh
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void timestamp_00c57118(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c57118h
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void timestamp_00c574ae(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c574aeh
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void timestamp_00c574cf(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c574cfh
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void timestamp_00c57500(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c57500h
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void timestamp_00c5754f(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c5754fh
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void timestamp_00c57579(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c57579h
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void timestamp_00c575f2(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c575f2h
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void timestamp_00c57636(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c57636h
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void bridge_00c5766c(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c5766ch
        call allocate_bridge
        add esp,0ch
        ret 4
    }
}
__declspec(naked) void bridge_00c576ac(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000c576ach
        call free_bridge
        add esp,0ch
        ret 4
    }
}
__declspec(naked) void timestamp_00c5774a(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c5774ah
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void timestamp_00c577fb(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c577fbh
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
__declspec(naked) void timestamp_00c5782c(){
    __asm {
        pushfd
        push ecx
        push dword ptr [esp+0ch]
        push 000c5782ch
        call timestamp_bridge
        add esp,8
        pop ecx
        popfd
        ret 4
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void pass_kernel(){
    __asm {
        push -1 // 00c57070
        mov eax, dword ptr fs:[0] // 00c57072
        push 0 // 00c57078
        push eax // 00c5707d
        // 00c5707e: original FS write omitted; normal-return contract.
        mov eax,dword ptr [esp+20] // 00c57085: borrowed context.
        mov eax,dword ptr [eax+8] // Actual publication cell.
        mov eax,dword ptr [eax] // Preserve original load and flags.
        sub esp, 0bch // 00c5708a
        cmp dword ptr [eax + 014h], 0 // 00c57090
        push ebx // 00c57094
        push ebp // 00c57095
        mov ebp, dword ptr [esp + 0d4h] // 00c57096
        push esi // 00c5709d
        push edi // 00c5709e
        lea edi, [eax + 014h] // 00c5709f
        jne l_00c570ba // 00c570a2
        mov esi, dword ptr [eax + 4] // 00c570a4
        push 2 // 00c570a7
        push offset name_00d79d50 // 00c570a9
        push dword ptr [esp+232] // Borrowed context; original frame offsets retained.
        call append_shim // 00c570ae
        mov dword ptr [edi], eax // 00c570b3
        mov eax,dword ptr [esp+224] // 00c570b5: borrowed context.
        mov eax,dword ptr [eax+8] // Actual publication cell.
        mov eax,dword ptr [eax] // Preserve original load and flags.
    l_00c570ba:
        mov ecx, dword ptr [edi] // 00c570ba
        mov edx, dword ptr [eax + 4] // 00c570bc
        mov dword ptr [ecx], edx // 00c570bf
        mov dword ptr [esp + 0c0h], ecx // 00c570c1
        mov dword ptr [eax + 4], ecx // 00c570c8
        push dword ptr [esp+224] // Borrowed context; original frame offsets retained.
        call timestamp_00c570cb // 00c570cb
        mov dword ptr [esp + 0b4h], edx // 00c570cd
        mov dword ptr [esp + 0b0h], eax // 00c570d4
        mov dword ptr [esp + 0d4h], 0 // 00c570db
        mov eax,dword ptr [esp+224] // 00c570e6: borrowed context.
        mov eax,dword ptr [eax+8] // Actual publication cell.
        mov eax,dword ptr [eax] // Preserve original load and flags.
        cmp dword ptr [eax + 018h], 0 // 00c570eb
        lea edi, [eax + 018h] // 00c570ef
        jne l_00c5710a // 00c570f2
        mov esi, dword ptr [eax + 4] // 00c570f4
        push 3 // 00c570f7
        push offset name_00d79d58 // 00c570f9
        push dword ptr [esp+232] // Borrowed context; original frame offsets retained.
        call append_shim // 00c570fe
        mov dword ptr [edi], eax // 00c57103
        mov eax,dword ptr [esp+224] // 00c57105: borrowed context.
        mov eax,dword ptr [eax+8] // Actual publication cell.
        mov eax,dword ptr [eax] // Preserve original load and flags.
    l_00c5710a:
        mov edi, dword ptr [edi] // 00c5710a
        mov ecx, dword ptr [eax + 4] // 00c5710c
        mov dword ptr [edi], ecx // 00c5710f
        mov dword ptr [esp + 050h], edi // 00c57111
        mov dword ptr [eax + 4], edi // 00c57115
        push dword ptr [esp+224] // Borrowed context; original frame offsets retained.
        call timestamp_00c57118 // 00c57118
        mov dword ptr [esp + 044h], edx // 00c5711a
        mov dword ptr [esp + 040h], eax // 00c5711e
        mov byte ptr [esp + 0d4h], 1 // 00c57122
        mov eax, dword ptr [ebp + 0a8h] // 00c5712a
        mov esi, dword ptr [eax + 0204h] // 00c57130
        add eax, 0208h // 00c57136
        cmp esi, eax // 00c5713b
        je l_00c5747c // 00c5713d
        fld qword ptr constant_00d7a280 // 00c57143
    l_00c57149:
        mov eax, dword ptr [esi + 050h] // 00c57149
        test al, 8 // 00c5714c
        je l_00c57461 // 00c5714e
        test al, 010h // 00c57154
        jne l_00c57461 // 00c57156
        fld dword ptr [esi + 038h] // 00c5715c
        mov ebx, dword ptr [esi + 060h] // 00c5715f
        fstp dword ptr [esp + 010h] // 00c57162
        add ebx, 4 // 00c57166
        fld dword ptr [esp + 010h] // 00c57169
        fld st(0) // 00c5716d
        fadd dword ptr [esi + 044h] // 00c5716f
        fstp dword ptr [esp + 014h] // 00c57172
        fld dword ptr [esi + 03ch] // 00c57176
        fstp dword ptr [esp + 010h] // 00c57179
        fld dword ptr [esi + 048h] // 00c5717d
        fld dword ptr [esp + 010h] // 00c57180
        fld st(0) // 00c57184
        faddp st(2), st(0) // 00c57186
        fxch st(1) // 00c57188
        fstp dword ptr [esp + 024h] // 00c5718a
        fld dword ptr [esi + 040h] // 00c5718e
        fstp dword ptr [esp + 010h] // 00c57191
        fld dword ptr [esi + 04ch] // 00c57195
        fld dword ptr [esp + 010h] // 00c57198
        fld st(0) // 00c5719c
        faddp st(2), st(0) // 00c5719e
        fxch st(1) // 00c571a0
        fstp dword ptr [esp + 010h] // 00c571a2
        fld dword ptr [esp + 014h] // 00c571a6
        fmul st(0), st(4) // 00c571aa
        fstp dword ptr [esp + 06ch] // 00c571ac
        fld dword ptr [esp + 024h] // 00c571b0
        fmul st(0), st(4) // 00c571b4
        fstp dword ptr [esp + 0ach] // 00c571b6
        fld dword ptr [esp + 010h] // 00c571bd
        fmul st(0), st(4) // 00c571c1
        fstp dword ptr [esp + 068h] // 00c571c3
        fld dword ptr [esi + 044h] // 00c571c7
        fsubrp st(3), st(0) // 00c571ca
        fxch st(2) // 00c571cc
        fstp dword ptr [esp + 024h] // 00c571ce
        fsubr dword ptr [esi + 048h] // 00c571d2
        fstp dword ptr [esp + 010h] // 00c571d5
        fsubr dword ptr [esi + 04ch] // 00c571d9
        fstp dword ptr [esp + 014h] // 00c571dc
        fld dword ptr [esp + 024h] // 00c571e0
        fmul st(0), st(1) // 00c571e4
        fstp dword ptr [esp + 01ch] // 00c571e6
        fld dword ptr [esp + 010h] // 00c571ea
        fmul st(0), st(1) // 00c571ee
        fstp dword ptr [esp + 020h] // 00c571f0
        fmul dword ptr [esp + 014h] // 00c571f4
        fstp dword ptr [esp + 018h] // 00c571f8
        fld dword ptr [esi + 8] // 00c571fc
        fstp dword ptr [esp + 024h] // 00c571ff
        fld dword ptr [esi + 014h] // 00c57203
        fld dword ptr [esp + 0ach] // 00c57206
        fld st(0) // 00c5720d
        fmulp st(2), st(0) // 00c5720f
        fld dword ptr [esp + 024h] // 00c57211
        fld st(0) // 00c57215
        fld dword ptr [esp + 06ch] // 00c57217
        fld st(0) // 00c5721b
        fmulp st(2), st(0) // 00c5721d
        fxch st(4) // 00c5721f
        faddp st(1), st(0) // 00c57221
        fld dword ptr [esi + 020h] // 00c57223
        fld dword ptr [esp + 068h] // 00c57226
        fld st(0) // 00c5722a
        fmulp st(2), st(0) // 00c5722c
        fxch st(2) // 00c5722e
        faddp st(1), st(0) // 00c57230
        fadd dword ptr [esi + 02ch] // 00c57232
        fstp dword ptr [esp + 05ch] // 00c57235
        fld dword ptr [esi + 0ch] // 00c57239
        fstp dword ptr [esp + 014h] // 00c5723c
        fld dword ptr [esi + 018h] // 00c57240
        fmul st(0), st(3) // 00c57243
        fld dword ptr [esp + 014h] // 00c57245
        fmul st(0), st(5) // 00c57249
        faddp st(1), st(0) // 00c5724b
        fld dword ptr [esi + 024h] // 00c5724d
        fmul st(0), st(2) // 00c57250
        faddp st(1), st(0) // 00c57252
        fadd dword ptr [esi + 030h] // 00c57254
        fstp dword ptr [esp + 060h] // 00c57257
        push ecx // 00c5725b
        fld dword ptr [esi + 010h] // 00c5725c
        fstp dword ptr [esp + 014h] // 00c5725f
        fld dword ptr [esi + 01ch] // 00c57263
        fmulp st(3), st(0) // 00c57266
        fld dword ptr [esp + 014h] // 00c57268
        fmulp st(4), st(0) // 00c5726c
        fxch st(2) // 00c5726e
        faddp st(3), st(0) // 00c57270
        fld dword ptr [esi + 028h] // 00c57272
        fmulp st(2), st(0) // 00c57275
        fxch st(2) // 00c57277
        faddp st(1), st(0) // 00c57279
        fadd dword ptr [esi + 034h] // 00c5727b
        fstp dword ptr [esp + 068h] // 00c5727e
        fstp dword ptr [esp] // 00c57282
        call abs_kernel // 00c57285
        fstp dword ptr [esp + 088h] // 00c5728a
        push ecx // 00c57291
        fld dword ptr [esp + 018h] // 00c57292
        fstp dword ptr [esp] // 00c57296
        call abs_kernel // 00c57299
        fstp dword ptr [esp + 08ch] // 00c5729e
        push ecx // 00c572a5
        fld dword ptr [esp + 014h] // 00c572a6
        fstp dword ptr [esp] // 00c572aa
        call abs_kernel // 00c572ad
        fstp dword ptr [esp + 090h] // 00c572b2
        push ecx // 00c572b9
        fld dword ptr [esi + 014h] // 00c572ba
        fstp dword ptr [esp] // 00c572bd
        call abs_kernel // 00c572c0
        fstp dword ptr [esp + 094h] // 00c572c5
        push ecx // 00c572cc
        fld dword ptr [esi + 018h] // 00c572cd
        fstp dword ptr [esp] // 00c572d0
        call abs_kernel // 00c572d3
        fstp dword ptr [esp + 098h] // 00c572d8
        push ecx // 00c572df
        fld dword ptr [esi + 01ch] // 00c572e0
        fstp dword ptr [esp] // 00c572e3
        call abs_kernel // 00c572e6
        fstp dword ptr [esp + 09ch] // 00c572eb
        push ecx // 00c572f2
        fld dword ptr [esi + 020h] // 00c572f3
        fstp dword ptr [esp] // 00c572f6
        call abs_kernel // 00c572f9
        fstp dword ptr [esp + 0a0h] // 00c572fe
        push ecx // 00c57305
        fld dword ptr [esi + 024h] // 00c57306
        fstp dword ptr [esp] // 00c57309
        call abs_kernel // 00c5730c
        fstp dword ptr [esp + 0a4h] // 00c57311
        push ecx // 00c57318
        fld dword ptr [esi + 028h] // 00c57319
        fstp dword ptr [esp] // 00c5731c
        call abs_kernel // 00c5731f
        fstp dword ptr [esp + 0a8h] // 00c57324
        fld dword ptr [esp + 094h] // 00c5732b
        fld dword ptr [esp + 020h] // 00c57332
        fld st(0) // 00c57336
        fmulp st(2), st(0) // 00c57338
        fld dword ptr [esp + 01ch] // 00c5733a
        fld st(0) // 00c5733e
        fmul dword ptr [esp + 088h] // 00c57340
        faddp st(3), st(0) // 00c57347
        fld dword ptr [esp + 0a0h] // 00c57349
        fld dword ptr [esp + 018h] // 00c57350
        fld st(0) // 00c57354
        fmulp st(2), st(0) // 00c57356
        fxch st(4) // 00c57358
        faddp st(1), st(0) // 00c5735a
        fstp dword ptr [esp + 070h] // 00c5735c
        fld dword ptr [esp + 098h] // 00c57360
        fmul st(0), st(2) // 00c57367
        fld dword ptr [esp + 08ch] // 00c57369
        fmul st(0), st(2) // 00c57370
        faddp st(1), st(0) // 00c57372
        fld dword ptr [esp + 0a4h] // 00c57374
        fmul st(0), st(4) // 00c5737b
        faddp st(1), st(0) // 00c5737d
        fstp dword ptr [esp + 074h] // 00c5737f
        fld dword ptr [esp + 09ch] // 00c57383
        fmulp st(2), st(0) // 00c5738a
        fmul dword ptr [esp + 090h] // 00c5738c
        faddp st(1), st(0) // 00c57393
        fld dword ptr [esp + 0a8h] // 00c57395
        fmulp st(2), st(0) // 00c5739c
        faddp st(1), st(0) // 00c5739e
        fstp dword ptr [esp + 078h] // 00c573a0
        fld dword ptr [esp + 05ch] // 00c573a4
        fld st(0) // 00c573a8
        fld dword ptr [esp + 070h] // 00c573aa
        fld st(0) // 00c573ae
        fsubp st(2), st(0) // 00c573b0
        fxch st(1) // 00c573b2
        fstp dword ptr [esp + 018h] // 00c573b4
        fld dword ptr [esp + 060h] // 00c573b8
        fld st(0) // 00c573bc
        fld dword ptr [esp + 074h] // 00c573be
        fld st(0) // 00c573c2
        fsubp st(2), st(0) // 00c573c4
        fxch st(1) // 00c573c6
        fstp dword ptr [esp + 01ch] // 00c573c8
        fld dword ptr [esp + 064h] // 00c573cc
        fld st(0) // 00c573d0
        fld dword ptr [esp + 078h] // 00c573d2
        fld st(0) // 00c573d6
        fsubp st(2), st(0) // 00c573d8
        fxch st(1) // 00c573da
        fstp dword ptr [esp + 020h] // 00c573dc
        fld dword ptr [esp + 018h] // 00c573e0
        fstp dword ptr [esp + 07ch] // 00c573e4
        mov edx, dword ptr [esp + 07ch] // 00c573e8
        fld dword ptr [esp + 01ch] // 00c573ec
        mov dword ptr [ebx], edx // 00c573f0
        fstp dword ptr [esp + 080h] // 00c573f2
        mov eax, dword ptr [esp + 080h] // 00c573f9
        fld dword ptr [esp + 020h] // 00c57400
        mov dword ptr [ebx + 4], eax // 00c57404
        fstp dword ptr [esp + 084h] // 00c57407
        mov ecx, dword ptr [esp + 084h] // 00c5740e
        fxch st(4) // 00c57415
        mov dword ptr [ebx + 8], ecx // 00c57417
        faddp st(5), st(0) // 00c5741a
        fxch st(4) // 00c5741c
        fstp dword ptr [esp + 018h] // 00c5741e
        faddp st(1), st(0) // 00c57422
        fstp dword ptr [esp + 01ch] // 00c57424
        faddp st(1), st(0) // 00c57428
        fstp dword ptr [esp + 020h] // 00c5742a
        fld dword ptr [esp + 018h] // 00c5742e
        fstp dword ptr [esp + 028h] // 00c57432
        mov edx, dword ptr [esp + 028h] // 00c57436
        fld dword ptr [esp + 01ch] // 00c5743a
        mov dword ptr [ebx + 0ch], edx // 00c5743e
        fstp dword ptr [esp + 02ch] // 00c57441
        mov eax, dword ptr [esp + 02ch] // 00c57445
        fld dword ptr [esp + 020h] // 00c57449
        mov dword ptr [ebx + 010h], eax // 00c5744d
        fstp dword ptr [esp + 030h] // 00c57450
        mov ecx, dword ptr [esp + 030h] // 00c57454
        fld qword ptr constant_00d7a280 // 00c57458
        mov dword ptr [ebx + 014h], ecx // 00c5745e
    l_00c57461:
        mov eax, dword ptr [ebp + 0a8h] // 00c57461
        mov esi, dword ptr [esi + 084h] // 00c57467
        add eax, 0208h // 00c5746d
        cmp esi, eax // 00c57472
        jne l_00c57149 // 00c57474
        fstp st(0) // 00c5747a
    l_00c5747c:
        mov eax,dword ptr [esp+224] // 00c5747c: borrowed context.
        mov eax,dword ptr [eax+8] // Actual publication cell.
        mov eax,dword ptr [eax] // Preserve original load and flags.
        cmp dword ptr [eax + 01ch], 0 // 00c57481
        lea ebx, [eax + 01ch] // 00c57485
        jne l_00c574a0 // 00c57488
        mov esi, dword ptr [eax + 4] // 00c5748a
        push 4 // 00c5748d
        push offset name_00d79d64 // 00c5748f
        push dword ptr [esp+232] // Borrowed context; original frame offsets retained.
        call append_shim // 00c57494
        mov dword ptr [ebx], eax // 00c57499
        mov eax,dword ptr [esp+224] // 00c5749b: borrowed context.
        mov eax,dword ptr [eax+8] // Actual publication cell.
        mov eax,dword ptr [eax] // Preserve original load and flags.
    l_00c574a0:
        mov esi, dword ptr [ebx] // 00c574a0
        mov edx, dword ptr [eax + 4] // 00c574a2
        mov dword ptr [esi], edx // 00c574a5
        mov dword ptr [esp + 038h], esi // 00c574a7
        mov dword ptr [eax + 4], esi // 00c574ab
        push dword ptr [esp+224] // Borrowed context; original frame offsets retained.
        call timestamp_00c574ae // 00c574ae
        mov ebx, eax // 00c574b0
        mov dword ptr [esp + 02ch], edx // 00c574b2
        mov dword ptr [esp + 028h], ebx // 00c574b6
        mov byte ptr [esp + 0d4h], 2 // 00c574ba
        mov ecx, dword ptr [ebp + 0ach] // 00c574c2
        mov eax, dword ptr [ecx] // 00c574c8
        mov edx, dword ptr [eax + 0ch] // 00c574ca
        call edx // 00c574cd
        push dword ptr [esp+224] // Borrowed context; original frame offsets retained.
        call timestamp_00c574cf // 00c574cf
        sub eax, ebx // 00c574d1
        sbb edx, dword ptr [esp + 02ch] // 00c574d3
        add dword ptr [esi + 038h], eax // 00c574d7
        mov dword ptr [esi + 030h], eax // 00c574da
        mov eax,dword ptr [esp+224] // 00c574dd: borrowed context.
        mov eax,dword ptr [eax+8] // Actual publication cell.
        mov eax,dword ptr [eax] // Preserve original load and flags.
        adc dword ptr [esi + 03ch], edx // 00c574e2
        mov dword ptr [esi + 034h], edx // 00c574e5
        mov ecx, 1 // 00c574e8
        add dword ptr [esi + 040h], ecx // 00c574ed
        mov edx, dword ptr [eax + 4] // 00c574f0
        mov edx, dword ptr [edx] // 00c574f3
        mov dword ptr [eax + 4], edx // 00c574f5
        mov byte ptr [esp + 0d4h], 0 // 00c574f8
        push dword ptr [esp+224] // Borrowed context; original frame offsets retained.
        call timestamp_00c57500 // 00c57500
        sub eax, dword ptr [esp + 040h] // 00c57502
        sbb edx, dword ptr [esp + 044h] // 00c57506
        add dword ptr [edi + 038h], eax // 00c5750a
        mov dword ptr [edi + 030h], eax // 00c5750d
        mov eax,dword ptr [esp+224] // 00c57510: borrowed context.
        mov eax,dword ptr [eax+8] // Actual publication cell.
        mov eax,dword ptr [eax] // Preserve original load and flags.
        adc dword ptr [edi + 03ch], edx // 00c57515
        mov dword ptr [edi + 034h], edx // 00c57518
        add dword ptr [edi + 040h], ecx // 00c5751b
        mov ecx, dword ptr [eax + 4] // 00c5751e
        mov esi, dword ptr [ecx] // 00c57521
        lea edi, [eax + 044h] // 00c57523
        mov dword ptr [eax + 4], esi // 00c57526
        cmp dword ptr [edi], 0 // 00c57529
        jne l_00c57541 // 00c5752c
        push 0eh // 00c5752e
        push offset name_00d79d78 // 00c57530
        push dword ptr [esp+232] // Borrowed context; original frame offsets retained.
        call append_shim // 00c57535
        mov dword ptr [edi], eax // 00c5753a
        mov eax,dword ptr [esp+224] // 00c5753c: borrowed context.
        mov eax,dword ptr [eax+8] // Actual publication cell.
        mov eax,dword ptr [eax] // Preserve original load and flags.
    l_00c57541:
        mov esi, dword ptr [edi] // 00c57541
        mov edx, dword ptr [eax + 4] // 00c57543
        mov dword ptr [esi], edx // 00c57546
        mov dword ptr [esp + 050h], esi // 00c57548
        mov dword ptr [eax + 4], esi // 00c5754c
        push dword ptr [esp+224] // Borrowed context; original frame offsets retained.
        call timestamp_00c5754f // 00c5754f
        mov ebx, edx // 00c57551
        mov edi, eax // 00c57553
        mov dword ptr [esp + 044h], ebx // 00c57555
        mov dword ptr [esp + 040h], edi // 00c57559
        mov byte ptr [esp + 0d4h], 3 // 00c5755d
        mov eax, dword ptr [ebp + 0b0h] // 00c57565
        push eax // 00c5756b
        push dword ptr [esp+228] // Borrowed context; original frame offsets retained.
        call update_shim // 00c5756c
        mov byte ptr [esp + 0d4h], 0 // 00c57571
        push dword ptr [esp+224] // Borrowed context; original frame offsets retained.
        call timestamp_00c57579 // 00c57579
        sub eax, edi // 00c5757b
        sbb edx, ebx // 00c5757d
        add dword ptr [esi + 038h], eax // 00c5757f
        mov dword ptr [esi + 030h], eax // 00c57582
        mov eax,dword ptr [esp+224] // 00c57585: borrowed context.
        mov eax,dword ptr [eax+8] // Actual publication cell.
        mov eax,dword ptr [eax] // Preserve original load and flags.
        adc dword ptr [esi + 03ch], edx // 00c5758a
        mov dword ptr [esi + 034h], edx // 00c5758d
        add dword ptr [esi + 040h], 1 // 00c57590
        mov ecx, dword ptr [eax + 4] // 00c57594
        mov edx, dword ptr [ecx] // 00c57597
        mov dword ptr [eax + 4], edx // 00c57599
        mov ecx, dword ptr [ebp + 0ach] // 00c5759c
        mov eax, dword ptr [ecx] // 00c575a2
        mov edx, dword ptr [eax + 010h] // 00c575a4
        call edx // 00c575a7
        mov edi, eax // 00c575a9
        xor eax, eax // 00c575ab
        cmp edi, eax // 00c575ad
        mov dword ptr [ebp + 0dch], eax // 00c575af
        mov dword ptr [esp + 014h], eax // 00c575b5
        je l_00c57826 // 00c575b9
        mov ecx,dword ptr [esp+224] // 00c575bf: borrowed context.
        mov ecx,dword ptr [ecx+8] // Actual publication cell.
        mov ecx,dword ptr [ecx] // Preserve original load and flags.
        cmp dword ptr [ecx + 048h], eax // 00c575c5
        lea ebx, [ecx + 048h] // 00c575c8
        jne l_00c575e4 // 00c575cb
        mov esi, dword ptr [ecx + 4] // 00c575cd
        push 0fh // 00c575d0
        push offset name_00d79d88 // 00c575d2
        push dword ptr [esp+232] // Borrowed context; original frame offsets retained.
        call append_shim // 00c575d7
        mov ecx,dword ptr [esp+224] // 00c575dc: borrowed context.
        mov ecx,dword ptr [ecx+8] // Actual publication cell.
        mov ecx,dword ptr [ecx] // Preserve original load and flags.
        mov dword ptr [ebx], eax // 00c575e2
    l_00c575e4:
        mov eax, dword ptr [ebx] // 00c575e4
        mov edx, dword ptr [ecx + 4] // 00c575e6
        mov dword ptr [eax], edx // 00c575e9
        mov dword ptr [esp + 038h], eax // 00c575eb
        mov dword ptr [ecx + 4], eax // 00c575ef
        push dword ptr [esp+224] // Borrowed context; original frame offsets retained.
        call timestamp_00c575f2 // 00c575f2
        mov dword ptr [esp + 02ch], edx // 00c575f4
        mov dword ptr [esp + 028h], eax // 00c575f8
        mov byte ptr [esp + 0d4h], 4 // 00c575fc
        mov eax,dword ptr [esp+224] // 00c57604: borrowed context.
        mov eax,dword ptr [eax+8] // Actual publication cell.
        mov eax,dword ptr [eax] // Preserve original load and flags.
        cmp dword ptr [eax + 04ch], 0 // 00c57609
        lea ebx, [eax + 04ch] // 00c5760d
        jne l_00c57628 // 00c57610
        mov esi, dword ptr [eax + 4] // 00c57612
        push 010h // 00c57615
        push offset name_00d79d98 // 00c57617
        push dword ptr [esp+232] // Borrowed context; original frame offsets retained.
        call append_shim // 00c5761c
        mov dword ptr [ebx], eax // 00c57621
        mov eax,dword ptr [esp+224] // 00c57623: borrowed context.
        mov eax,dword ptr [eax+8] // Actual publication cell.
        mov eax,dword ptr [eax] // Preserve original load and flags.
    l_00c57628:
        mov esi, dword ptr [ebx] // 00c57628
        mov ecx, dword ptr [eax + 4] // 00c5762a
        mov dword ptr [esi], ecx // 00c5762d
        mov dword ptr [esp + 050h], esi // 00c5762f
        mov dword ptr [eax + 4], esi // 00c57633
        push dword ptr [esp+224] // Borrowed context; original frame offsets retained.
        call timestamp_00c57636 // 00c57636
        mov ebx, eax // 00c57638
        mov dword ptr [esp + 044h], edx // 00c5763a
        mov dword ptr [esp + 040h], ebx // 00c5763e
        mov byte ptr [esp + 0d4h], 5 // 00c57642
        cmp edi, dword ptr [ebp + 0b8h] // 00c5764a
        jbe l_00c576ea // 00c57650
        cmp edi, dword ptr [ebp + 0bch] // 00c57656
        jbe l_00c576be // 00c5765c
        lea edx, [edi*4] // 00c5765e
        push edx // 00c57665
        mov dword ptr [ebp + 0bch], edi // 00c57666
        push dword ptr [esp+228] // Borrowed context; original frame offsets retained.
        call bridge_00c5766c // 00c5766c
        mov ecx, eax // 00c57671
        xor eax, eax // 00c57673
        add esp, 4 // 00c57675
        cmp dword ptr [ebp + 0b8h], eax // 00c57678
        mov dword ptr [esp + 018h], ecx // 00c5767e
        jbe l_00c576a1 // 00c57682
    l_00c57684:
        test ecx, ecx // 00c57684
        je l_00c57693 // 00c57686
        mov edx, dword ptr [ebp + 0b4h] // 00c57688
        mov edx, dword ptr [edx + eax*4] // 00c5768e
        mov dword ptr [ecx], edx // 00c57691
    l_00c57693:
        add eax, 1 // 00c57693
        add ecx, 4 // 00c57696
        cmp eax, dword ptr [ebp + 0b8h] // 00c57699
        jb l_00c57684 // 00c5769f
    l_00c576a1:
        mov eax, dword ptr [ebp + 0b4h] // 00c576a1
        test eax, eax // 00c576a7
        je l_00c576b4 // 00c576a9
        push eax // 00c576ab
        push dword ptr [esp+228] // Borrowed context; original frame offsets retained.
        call bridge_00c576ac // 00c576ac
        add esp, 4 // 00c576b1
    l_00c576b4:
        mov eax, dword ptr [esp + 018h] // 00c576b4
        mov dword ptr [ebp + 0b4h], eax // 00c576b8
    l_00c576be:
        mov eax, dword ptr [ebp + 0b8h] // 00c576be
        cmp eax, edi // 00c576c4
        jae l_00c576ea // 00c576c6
        jmp l_00c576d0 // 00c576c8
        _emit 08dh // 00c576ca
        _emit 09bh
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    l_00c576d0:
        mov ecx, dword ptr [ebp + 0b4h] // 00c576d0
        lea ecx, [ecx + eax*4] // 00c576d6
        test ecx, ecx // 00c576d9
        je l_00c576e3 // 00c576db
        mov dword ptr [ecx], 0 // 00c576dd
    l_00c576e3:
        add eax, 1 // 00c576e3
        cmp eax, edi // 00c576e6
        jb l_00c576d0 // 00c576e8
    l_00c576ea:
        mov dword ptr [ebp + 0b8h], edi // 00c576ea
        mov ecx, dword ptr [ebp + 0ach] // 00c576f0
        mov edx, dword ptr [ecx] // 00c576f6
        mov eax, dword ptr [edx + 014h] // 00c576f8
        call eax // 00c576fb
        test edi, edi // 00c576fd
        jle l_00c57742 // 00c576ff
        mov ebx, edi // 00c57701
    l_00c57703:
        mov edx, dword ptr [eax + 4] // 00c57703
        mov edi, dword ptr [edx] // 00c57706
        mov ecx, dword ptr [eax] // 00c57708
        mov ecx, dword ptr [ecx] // 00c5770a
        mov edx, dword ptr [edi + 050h] // 00c5770c
        and edx, dword ptr [ecx + 050h] // 00c5770f
        test dl, 010h // 00c57712
        jne l_00c5772b // 00c57715
        mov edx, dword ptr [esp + 014h] // 00c57717
        mov ecx, dword ptr [ebp + 0b4h] // 00c5771b
        mov dword ptr [ecx + edx*4], eax // 00c57721
        add edx, 1 // 00c57724
        mov dword ptr [esp + 014h], edx // 00c57727
    l_00c5772b:
        mov ecx, dword ptr [ebp + 0ach] // 00c5772b
        mov edx, dword ptr [ecx] // 00c57731
        push eax // 00c57733
        mov eax, dword ptr [edx + 018h] // 00c57734
        call eax // 00c57737
        sub ebx, 1 // 00c57739
        jne l_00c57703 // 00c5773c
        mov ebx, dword ptr [esp + 040h] // 00c5773e
    l_00c57742:
        mov byte ptr [esp + 0d4h], 4 // 00c57742
        push dword ptr [esp+224] // Borrowed context; original frame offsets retained.
        call timestamp_00c5774a // 00c5774a
        sub eax, ebx // 00c5774c
        sbb edx, dword ptr [esp + 044h] // 00c5774e
        add dword ptr [esi + 038h], eax // 00c57752
        mov dword ptr [esi + 030h], eax // 00c57755
        mov eax,dword ptr [esp+224] // 00c57758: borrowed context.
        mov eax,dword ptr [eax+8] // Actual publication cell.
        mov eax,dword ptr [eax] // Preserve original load and flags.
        adc dword ptr [esi + 03ch], edx // 00c5775d
        mov dword ptr [esi + 034h], edx // 00c57760
        add dword ptr [esi + 040h], 1 // 00c57763
        mov ecx, dword ptr [eax + 4] // 00c57767
        mov esi, dword ptr [esp + 014h] // 00c5776a
        test esi, esi // 00c5776e
        mov edx, dword ptr [ecx] // 00c57770
        mov dword ptr [eax + 4], edx // 00c57772
        je l_00c577f3 // 00c57775
        mov ebx, dword ptr [ebp + 0c4h] // 00c57777
        cmp ebx, esi // 00c5777d
        jge l_00c57787 // 00c5777f
        mov dword ptr [esp + 010h], ebx // 00c57781
        jmp l_00c5778d // 00c57785
    l_00c57787:
        mov ebx, esi // 00c57787
        mov dword ptr [esp + 010h], esi // 00c57789
    l_00c5778d:
        mov eax, esi // 00c5778d
        cdq  // 00c5778f
        idiv ebx // 00c57790
        lea edi, [ebx - 1] // 00c57792
        xor ecx, ecx // 00c57795
        xor edx, edx // 00c57797
        test edi, edi // 00c57799
        jle l_00c577c9 // 00c5779b
        mov dword ptr [esp + 018h], edi // 00c5779d
    l_00c577a1:
        mov esi, dword ptr [ebp + 0c0h] // 00c577a1
        lea ebx, [eax + ecx - 1] // 00c577a7
        mov dword ptr [edx + esi + 0ch], ecx // 00c577ab
        mov dword ptr [edx + esi + 010h], ebx // 00c577af
        add ecx, eax // 00c577b3
        add edx, 014h // 00c577b5
        sub edi, 1 // 00c577b8
        jne l_00c577a1 // 00c577bb
        mov ebx, dword ptr [esp + 010h] // 00c577bd
        mov esi, dword ptr [esp + 014h] // 00c577c1
        mov edx, dword ptr [esp + 018h] // 00c577c5
    l_00c577c9:
        mov eax, dword ptr [ebp + 0c0h] // 00c577c9
        lea edx, [edx + edx*4] // 00c577cf
        lea eax, [eax + edx*4] // 00c577d2
        mov dword ptr [eax + 0ch], ecx // 00c577d5
        mov ecx,dword ptr [esp+224] // 00c577d8: borrowed context.
        mov ecx,dword ptr [ecx+12] // Actual publication cell.
        mov ecx,dword ptr [ecx] // Preserve original load and flags.
        add esi, -1 // 00c577de
        mov dword ptr [eax + 010h], esi // 00c577e1
        mov eax, dword ptr [ebp + 0cch] // 00c577e4
        mov esi, dword ptr [ecx + 010h] // 00c577ea
        push ebx // 00c577ed
        push dword ptr [esp+228] // Borrowed context; original frame offsets retained.
        call batch_shim // 00c577ee
    l_00c577f3:
        mov byte ptr [esp + 0d4h], 0 // 00c577f3
        push dword ptr [esp+224] // Borrowed context; original frame offsets retained.
        call timestamp_00c577fb // 00c577fb
        sub eax, dword ptr [esp + 028h] // 00c577fd
        mov ecx, dword ptr [esp + 038h] // 00c57801
        sbb edx, dword ptr [esp + 02ch] // 00c57805
        add dword ptr [ecx + 038h], eax // 00c57809
        mov dword ptr [ecx + 030h], eax // 00c5780c
        mov eax,dword ptr [esp+224] // 00c5780f: borrowed context.
        mov eax,dword ptr [eax+8] // Actual publication cell.
        mov eax,dword ptr [eax] // Preserve original load and flags.
        adc dword ptr [ecx + 03ch], edx // 00c57814
        mov dword ptr [ecx + 034h], edx // 00c57817
        add dword ptr [ecx + 040h], 1 // 00c5781a
        mov edx, dword ptr [eax + 4] // 00c5781e
        mov ecx, dword ptr [edx] // 00c57821
        mov dword ptr [eax + 4], ecx // 00c57823
    l_00c57826:
        push ebp // 00c57826
        call events_kernel // 00c57827
        push dword ptr [esp+224] // Borrowed context; original frame offsets retained.
        call timestamp_00c5782c // 00c5782c
        sub eax, dword ptr [esp + 0b0h] // 00c5782e
        pop edi // 00c57835
        sbb edx, dword ptr [esp + 0b0h] // 00c57836
        mov ecx, eax // 00c5783d
        mov eax, dword ptr [esp + 0bch] // 00c5783f
        add dword ptr [eax + 038h], ecx // 00c57846
        mov dword ptr [eax + 030h], ecx // 00c57849
        mov dword ptr [eax + 034h], edx // 00c5784c
        adc dword ptr [eax + 03ch], edx // 00c5784f
        add dword ptr [eax + 040h], 1 // 00c57852
        mov eax,dword ptr [esp+220] // 00c57856: borrowed context.
        mov eax,dword ptr [eax+8] // Actual publication cell.
        mov eax,dword ptr [eax] // Preserve original load and flags.
        mov edx, dword ptr [eax + 4] // 00c5785b
        mov ecx, dword ptr [edx] // 00c5785e
        pop esi // 00c57860
        mov dword ptr [eax + 4], ecx // 00c57861
        mov ecx, dword ptr [esp + 0c4h] // 00c57864
        pop ebp // 00c5786b
        // 00c5786c: original FS write omitted; normal-return contract.
        pop ebx // 00c57873
        add esp, 0c8h // 00c57874
        ret 8 // 00c5787a
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void update_kernel(){
    __asm {
        push ebx // 00c549d0
        push ebp // 00c549d1
        mov ebp, dword ptr [esp + 0ch] // 00c549d2
        mov edx, dword ptr [ebp + 0ech] // 00c549d6
        push esi // 00c549dc
        lea esi, [ebp + 0f0h] // 00c549dd
        cmp edx, esi // 00c549e3
        push edi // 00c549e5
        je l_00c54a15 // 00c549e6
        jmp l_00c549f0 // 00c549e8
        _emit 08dh // 00c549ea
        _emit 09bh
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    l_00c549f0:
        mov eax, dword ptr [edx + 0d0h] // 00c549f0
        mov ecx, dword ptr [edx + 0cch] // 00c549f6
        mov eax, dword ptr [eax + 050h] // 00c549fc
        and eax, dword ptr [ecx + 050h] // 00c549ff
        test al, 010h // 00c54a02
        jne l_00c54a0b // 00c54a04
        call refresh_kernel // 00c54a06
    l_00c54a0b:
        mov edx, dword ptr [edx + 0dch] // 00c54a0b
        cmp edx, esi // 00c54a11
        jne l_00c549f0 // 00c54a13
    l_00c54a15:
        mov edi, dword ptr [ebp + 0ech] // 00c54a15
        cmp edi, esi // 00c54a1b
        mov ebx, edi // 00c54a1d
        je l_00c54a8d // 00c54a1f
    l_00c54a21:
        cmp dword ptr [edi + 0c8h], 0 // 00c54a21
        mov ebx, dword ptr [ebx + 0dch] // 00c54a28
        jne l_00c54a81 // 00c54a2e
        mov esi, dword ptr [edi + 0cch] // 00c54a30
        mov edx, edi // 00c54a36
        push dword ptr [esp+24] // Borrowed context; original frame offsets retained.
        call remove_shim // 00c54a38
        mov esi, dword ptr [edi + 0d0h] // 00c54a3d
        mov edx, edi // 00c54a43
        push dword ptr [esp+24] // Borrowed context; original frame offsets retained.
        call remove_shim // 00c54a45
        mov ecx, dword ptr [edi + 0dch] // 00c54a4a
        mov edx, dword ptr [edi + 0d8h] // 00c54a50
        mov dword ptr [ecx + 0d8h], edx // 00c54a56
        mov ecx, dword ptr [edi + 0dch] // 00c54a5c
        mov eax, dword ptr [edi + 0d8h] // 00c54a62
        mov dword ptr [eax + 0dch], ecx // 00c54a68
        mov edx, dword ptr [ebp + 0ch] // 00c54a6e
        mov dword ptr [edi + 0dch], edx // 00c54a71
        add dword ptr [ebp + 01d0h], -1 // 00c54a77
        mov dword ptr [ebp + 0ch], edi // 00c54a7e
    l_00c54a81:
        lea eax, [ebp + 0f0h] // 00c54a81
        cmp ebx, eax // 00c54a87
        mov edi, ebx // 00c54a89
        jne l_00c54a21 // 00c54a8b
    l_00c54a8d:
        pop edi // 00c54a8d
        pop esi // 00c54a8e
        pop ebp // 00c54a8f
        pop ebx // 00c54a90
        ret 8 // 00c54a91
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void refresh_kernel(){
    __asm {
        sub esp, 058h // 00c4b9b0
        cmp dword ptr [edx + 0c8h], 0 // 00c4b9b3
        mov dword ptr [esp + 8], 0 // 00c4b9ba
        jle l_00c4bc49 // 00c4b9c2
        movss xmm1, dword ptr constant_00d7a320 // 00c4b9c8
        fld dword ptr constant_00d7a2d8 // 00c4b9d0
        push ebx // 00c4b9d6
        push ebp // 00c4b9d7
        push esi // 00c4b9d8
        lea eax, [edx + 034h] // 00c4b9d9
        lea ecx, [edx + 014h] // 00c4b9dc
        push edi // 00c4b9df
        lea ebx, [edx + 8] // 00c4b9e0
        mov dword ptr [esp + 014h], eax // 00c4b9e3
        mov dword ptr [esp + 010h], ecx // 00c4b9e7
        lea ebp, [edx + 020h] // 00c4b9eb
    l_00c4b9ee:
        fld dword ptr [ebp + 4] // 00c4b9ee
        mov eax, dword ptr [edx + 0d0h] // 00c4b9f1
        fstp dword ptr [esp + 01ch] // 00c4b9f7
        add eax, 8 // 00c4b9fb
        fld dword ptr [ebp] // 00c4b9fe
        fstp dword ptr [esp + 020h] // 00c4ba01
        fld dword ptr [ebp + 8] // 00c4ba05
        fstp dword ptr [esp + 024h] // 00c4ba08
        fld dword ptr [eax + 0ch] // 00c4ba0c
        fld dword ptr [esp + 01ch] // 00c4ba0f
        fld st(0) // 00c4ba13
        fmulp st(2), st(0) // 00c4ba15
        fld dword ptr [esp + 020h] // 00c4ba17
        fld st(0) // 00c4ba1b
        fmul dword ptr [eax] // 00c4ba1d
        faddp st(3), st(0) // 00c4ba1f
        fld dword ptr [eax + 018h] // 00c4ba21
        fld dword ptr [esp + 024h] // 00c4ba24
        fld st(0) // 00c4ba28
        fmulp st(2), st(0) // 00c4ba2a
        fxch st(4) // 00c4ba2c
        faddp st(1), st(0) // 00c4ba2e
        fadd dword ptr [eax + 024h] // 00c4ba30
        fstp dword ptr [esp + 038h] // 00c4ba33
        fld dword ptr [eax + 4] // 00c4ba37
        fmul st(0), st(1) // 00c4ba3a
        fld dword ptr [eax + 010h] // 00c4ba3c
        fmul st(0), st(3) // 00c4ba3f
        faddp st(1), st(0) // 00c4ba41
        fld dword ptr [eax + 01ch] // 00c4ba43
        fmul st(0), st(4) // 00c4ba46
        faddp st(1), st(0) // 00c4ba48
        fadd dword ptr [eax + 028h] // 00c4ba4a
        fstp dword ptr [esp + 03ch] // 00c4ba4d
        fmul dword ptr [eax + 8] // 00c4ba51
        fld dword ptr [eax + 014h] // 00c4ba54
        fmulp st(2), st(0) // 00c4ba57
        faddp st(1), st(0) // 00c4ba59
        fld dword ptr [eax + 020h] // 00c4ba5b
        fmulp st(2), st(0) // 00c4ba5e
        faddp st(1), st(0) // 00c4ba60
        fadd dword ptr [eax + 02ch] // 00c4ba62
        mov eax, dword ptr [edx + 0cch] // 00c4ba65
        add eax, 8 // 00c4ba6b
        fstp dword ptr [esp + 040h] // 00c4ba6e
        fld dword ptr [ecx + 4] // 00c4ba72
        fstp dword ptr [esp + 024h] // 00c4ba75
        fld dword ptr [ecx] // 00c4ba79
        fstp dword ptr [esp + 020h] // 00c4ba7b
        fld dword ptr [ecx + 8] // 00c4ba7f
        fstp dword ptr [esp + 01ch] // 00c4ba82
        fld dword ptr [eax + 0ch] // 00c4ba86
        fld dword ptr [esp + 024h] // 00c4ba89
        fld st(0) // 00c4ba8d
        fmulp st(2), st(0) // 00c4ba8f
        fld dword ptr [esp + 020h] // 00c4ba91
        fld st(0) // 00c4ba95
        fmul dword ptr [eax] // 00c4ba97
        faddp st(3), st(0) // 00c4ba99
        fld dword ptr [eax + 018h] // 00c4ba9b
        fld dword ptr [esp + 01ch] // 00c4ba9e
        fld st(0) // 00c4baa2
        fmulp st(2), st(0) // 00c4baa4
        fxch st(4) // 00c4baa6
        faddp st(1), st(0) // 00c4baa8
        fadd dword ptr [eax + 024h] // 00c4baaa
        fstp dword ptr [esp + 02ch] // 00c4baad
        fld dword ptr [eax + 4] // 00c4bab1
        fmul st(0), st(1) // 00c4bab4
        fld dword ptr [eax + 010h] // 00c4bab6
        fmul st(0), st(3) // 00c4bab9
        faddp st(1), st(0) // 00c4babb
        fld dword ptr [eax + 01ch] // 00c4babd
        fmul st(0), st(4) // 00c4bac0
        faddp st(1), st(0) // 00c4bac2
        fadd dword ptr [eax + 028h] // 00c4bac4
        fstp dword ptr [esp + 030h] // 00c4bac7
        fmul dword ptr [eax + 8] // 00c4bacb
        fld dword ptr [eax + 014h] // 00c4bace
        fmulp st(2), st(0) // 00c4bad1
        faddp st(1), st(0) // 00c4bad3
        fld dword ptr [eax + 020h] // 00c4bad5
        fmulp st(2), st(0) // 00c4bad8
        faddp st(1), st(0) // 00c4bada
        fadd dword ptr [eax + 02ch] // 00c4badc
        mov eax, dword ptr [esp + 014h] // 00c4badf
        fstp dword ptr [esp + 034h] // 00c4bae3
        fld dword ptr [esp + 02ch] // 00c4bae7
        fsub dword ptr [esp + 038h] // 00c4baeb
        fstp dword ptr [esp + 044h] // 00c4baef
        fld dword ptr [esp + 030h] // 00c4baf3
        fsub dword ptr [esp + 03ch] // 00c4baf7
        fstp dword ptr [esp + 048h] // 00c4bafb
        fld dword ptr [esp + 034h] // 00c4baff
        fsub dword ptr [esp + 040h] // 00c4bb03
        fstp dword ptr [esp + 04ch] // 00c4bb07
        fld dword ptr [ebx + 4] // 00c4bb0b
        fld dword ptr [esp + 048h] // 00c4bb0e
        fld st(0) // 00c4bb12
        fmulp st(2), st(0) // 00c4bb14
        fld dword ptr [ebx] // 00c4bb16
        fld dword ptr [esp + 044h] // 00c4bb18
        fld st(0) // 00c4bb1c
        fmulp st(2), st(0) // 00c4bb1e
        fxch st(3) // 00c4bb20
        faddp st(1), st(0) // 00c4bb22
        fld dword ptr [ebx + 8] // 00c4bb24
        fld dword ptr [esp + 04ch] // 00c4bb27
        fld st(0) // 00c4bb2b
        fmulp st(2), st(0) // 00c4bb2d
        fxch st(2) // 00c4bb2f
        faddp st(1), st(0) // 00c4bb31
        fstp dword ptr [esp + 01ch] // 00c4bb33
        movss xmm0, dword ptr [esp + 01ch] // 00c4bb37
        fld dword ptr [esp + 01ch] // 00c4bb3d
        comiss xmm1, xmm0 // 00c4bb41
        fstp dword ptr [eax] // 00c4bb44
        movss dword ptr [esp + 028h], xmm0 // 00c4bb46
        jbe l_00c4bb70 // 00c4bb4c
        add dword ptr [edx + 0c8h], -1 // 00c4bb4e
        fstp st(1) // 00c4bb55
        mov eax, dword ptr [edx + 0c8h] // 00c4bb57
        fstp st(1) // 00c4bb5d
        lea ecx, [eax + eax*2] // 00c4bb5f
        fstp st(0) // 00c4bb62
        shl ecx, 4 // 00c4bb64
        lea esi, [ecx + edx + 8] // 00c4bb67
        jmp l_00c4bbf7 // 00c4bb6b
    l_00c4bb70:
        fld dword ptr [esp + 028h] // 00c4bb70
        fstp dword ptr [esp + 024h] // 00c4bb74
        fld dword ptr [ebx] // 00c4bb78
        fld dword ptr [esp + 024h] // 00c4bb7a
        fld st(0) // 00c4bb7e
        fmulp st(2), st(0) // 00c4bb80
        fxch st(1) // 00c4bb82
        fstp dword ptr [esp + 050h] // 00c4bb84
        fld dword ptr [ebx + 4] // 00c4bb88
        fmul st(0), st(1) // 00c4bb8b
        fstp dword ptr [esp + 054h] // 00c4bb8d
        fmul dword ptr [ebx + 8] // 00c4bb91
        fstp dword ptr [esp + 058h] // 00c4bb94
        fld dword ptr [esp + 050h] // 00c4bb98
        fsubp st(3), st(0) // 00c4bb9c
        fxch st(2) // 00c4bb9e
        fstp dword ptr [esp + 05ch] // 00c4bba0
        fsub dword ptr [esp + 054h] // 00c4bba4
        fstp dword ptr [esp + 060h] // 00c4bba8
        fsub dword ptr [esp + 058h] // 00c4bbac
        fstp dword ptr [esp + 064h] // 00c4bbb0
        fld dword ptr [esp + 060h] // 00c4bbb4
        fld dword ptr [esp + 05ch] // 00c4bbb8
        fld dword ptr [esp + 064h] // 00c4bbbc
        fld st(1) // 00c4bbc0
        fmulp st(2), st(0) // 00c4bbc2
        fld st(2) // 00c4bbc4
        fmulp st(3), st(0) // 00c4bbc6
        fxch st(1) // 00c4bbc8
        faddp st(2), st(0) // 00c4bbca
        fmul st(0), st(0) // 00c4bbcc
        faddp st(1), st(0) // 00c4bbce
        fstp dword ptr [esp + 024h] // 00c4bbd0
        fld dword ptr [esp + 024h] // 00c4bbd4
        fxch st(1) // 00c4bbd8
        fcomi st(0), st(1) // 00c4bbda
        fstp st(1) // 00c4bbdc
        ja l_00c4bc1a // 00c4bbde
        add dword ptr [edx + 0c8h], -1 // 00c4bbe0
        mov eax, dword ptr [edx + 0c8h] // 00c4bbe7
        lea eax, [eax + eax*2] // 00c4bbed
        shl eax, 4 // 00c4bbf0
        lea esi, [eax + edx + 8] // 00c4bbf3
    l_00c4bbf7:
        sub dword ptr [esp + 018h], 1 // 00c4bbf7
        mov eax, 030h // 00c4bbfc
        sub dword ptr [esp + 010h], eax // 00c4bc01
        sub dword ptr [esp + 014h], eax // 00c4bc05
        mov edi, ebx // 00c4bc09
        mov ecx, 0ch // 00c4bc0b
        rep movsd // 00c4bc10
        mov ecx, dword ptr [esp + 010h] // 00c4bc12
        sub ebp, eax // 00c4bc16
        sub ebx, eax // 00c4bc18
    l_00c4bc1a:
        mov eax, dword ptr [esp + 018h] // 00c4bc1a
        add dword ptr [esp + 014h], 030h // 00c4bc1e
        add eax, 1 // 00c4bc23
        add ecx, 030h // 00c4bc26
        add ebp, 030h // 00c4bc29
        add ebx, 030h // 00c4bc2c
        cmp eax, dword ptr [edx + 0c8h] // 00c4bc2f
        mov dword ptr [esp + 018h], eax // 00c4bc35
        mov dword ptr [esp + 010h], ecx // 00c4bc39
        jl l_00c4b9ee // 00c4bc3d
        pop edi // 00c4bc43
        fstp st(0) // 00c4bc44
        pop esi // 00c4bc46
        pop ebp // 00c4bc47
        pop ebx // 00c4bc48
    l_00c4bc49:
        add esp, 058h // 00c4bc49
        ret  // 00c4bc4c
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void events_kernel(){
    __asm {
        sub esp, 050h // 00c35480
        push ebx // 00c35483
        push ebp // 00c35484
        mov ebp, dword ptr [esp + 05ch] // 00c35485
        xor ecx, ecx // 00c35489
        cmp dword ptr [ebp + 0dch], ecx // 00c3548b
        push esi // 00c35491
        push edi // 00c35492
        mov dword ptr [esp + 064h], ecx // 00c35493
        jle l_00c355df // 00c35497
        xor ebx, ebx // 00c3549d
        mov dword ptr [esp + 014h], ecx // 00c3549f
    l_00c354a3:
        mov eax, dword ptr [ebp + 0d8h] // 00c354a3
        mov esi, dword ptr [ebx + eax] // 00c354a9
        mov eax, dword ptr [esi + 0c8h] // 00c354ac
        mov dword ptr [esp + 05ch], eax // 00c354b2
        mov edx, dword ptr [ecx + esi + 8] // 00c354b6
        mov dword ptr [esp + 050h], edx // 00c354ba
        mov edx, dword ptr [ecx + esi + 0ch] // 00c354be
        mov dword ptr [esp + 054h], edx // 00c354c2
        mov ecx, dword ptr [ecx + esi + 010h] // 00c354c6
        xor edi, edi // 00c354ca
        test eax, eax // 00c354cc
        mov dword ptr [esp + 058h], ecx // 00c354ce
        jle l_00c3554b // 00c354d2
        lea edx, [esp + 028h] // 00c354d4
        lea ecx, [esi + 01ch] // 00c354d8
        jmp l_00c354e0 // 00c354db
        _emit 08dh // 00c354dd
        _emit 049h
        _emit 000h
    l_00c354e0:
        mov eax, dword ptr [esi + 0cch] // 00c354e0
        fld dword ptr [eax + 014h] // 00c354e6
        add eax, 8 // 00c354e9
        fmul dword ptr [ecx - 4] // 00c354ec
        add edi, 1 // 00c354ef
        fld dword ptr [ecx - 8] // 00c354f2
        add ecx, 030h // 00c354f5
        fmul dword ptr [eax] // 00c354f8
        add edx, 0ch // 00c354fa
        faddp st(1), st(0) // 00c354fd
        fld dword ptr [eax + 018h] // 00c354ff
        fmul dword ptr [ecx - 030h] // 00c35502
        faddp st(1), st(0) // 00c35505
        fadd dword ptr [eax + 024h] // 00c35507
        fstp dword ptr [edx - 014h] // 00c3550a
        fld dword ptr [eax + 010h] // 00c3550d
        fmul dword ptr [ecx - 034h] // 00c35510
        fld dword ptr [eax + 4] // 00c35513
        fmul dword ptr [ecx - 038h] // 00c35516
        faddp st(1), st(0) // 00c35519
        fld dword ptr [eax + 01ch] // 00c3551b
        fmul dword ptr [ecx - 030h] // 00c3551e
        faddp st(1), st(0) // 00c35521
        fadd dword ptr [eax + 028h] // 00c35523
        fstp dword ptr [edx - 010h] // 00c35526
        fld dword ptr [eax + 014h] // 00c35529
        fmul dword ptr [ecx - 034h] // 00c3552c
        fld dword ptr [eax + 8] // 00c3552f
        fmul dword ptr [ecx - 038h] // 00c35532
        faddp st(1), st(0) // 00c35535
        fld dword ptr [eax + 020h] // 00c35537
        fmul dword ptr [ecx - 030h] // 00c3553a
        faddp st(1), st(0) // 00c3553d
        fadd dword ptr [eax + 02ch] // 00c3553f
        fstp dword ptr [edx - 0ch] // 00c35542
        cmp edi, dword ptr [esp + 05ch] // 00c35545
        jl l_00c354e0 // 00c35549
    l_00c3554b:
        mov edx, dword ptr [esi + 0cch] // 00c3554b
        cmp dword ptr [edx + 068h], 0 // 00c35551
        je l_00c35582 // 00c35555
        mov eax, dword ptr [ebp + 0d8h] // 00c35557
        mov ecx, dword ptr [eax + ebx + 4] // 00c3555d
        add eax, ebx // 00c35561
        mov dword ptr [esp + 018h], ecx // 00c35563
        mov edx, dword ptr [eax + 8] // 00c35567
        mov dword ptr [esp + 01ch], edx // 00c3556a
        mov eax, dword ptr [esi + 0cch] // 00c3556e
        mov ecx, dword ptr [eax + 068h] // 00c35574
        mov edx, dword ptr [ecx] // 00c35577
        mov edx, dword ptr [edx] // 00c35579
        lea eax, [esp + 018h] // 00c3557b
        push eax // 00c3557f
        call edx // 00c35580
    l_00c35582:
        mov eax, dword ptr [esi + 0d0h] // 00c35582
        cmp dword ptr [eax + 068h], 0 // 00c35588
        je l_00c355ba // 00c3558c
        mov ecx, dword ptr [ebp + 0d8h] // 00c3558e
        mov edx, dword ptr [ebx + ecx + 8] // 00c35594
        lea eax, [ebx + ecx] // 00c35598
        mov dword ptr [esp + 018h], edx // 00c3559b
        mov eax, dword ptr [eax + 4] // 00c3559f
        mov dword ptr [esp + 01ch], eax // 00c355a2
        mov ecx, dword ptr [esi + 0d0h] // 00c355a6
        mov ecx, dword ptr [ecx + 068h] // 00c355ac
        mov edx, dword ptr [ecx] // 00c355af
        mov edx, dword ptr [edx] // 00c355b1
        lea eax, [esp + 018h] // 00c355b3
        push eax // 00c355b7
        call edx // 00c355b8
    l_00c355ba:
        mov eax, dword ptr [esp + 064h] // 00c355ba
        mov ecx, dword ptr [esp + 014h] // 00c355be
        add eax, 1 // 00c355c2
        add ecx, 030h // 00c355c5
        add ebx, 0ch // 00c355c8
        cmp eax, dword ptr [ebp + 0dch] // 00c355cb
        mov dword ptr [esp + 064h], eax // 00c355d1
        mov dword ptr [esp + 014h], ecx // 00c355d5
        jl l_00c354a3 // 00c355d9
    l_00c355df:
        pop edi // 00c355df
        pop esi // 00c355e0
        pop ebp // 00c355e1
        pop ebx // 00c355e2
        add esp, 050h // 00c355e3
        ret 4 // 00c355e6
    }
}
// Complete normal instruction schedule; comments identify native starts.
__declspec(naked) void abs_kernel(){
    __asm {
        push ebp // 00401170
        mov ebp, esp // 00401171
        and esp, 0fffffff8h // 00401173
        sub esp, 8 // 00401176
        fld dword ptr [ebp + 8] // 00401179
        fabs  // 0040117c
        fstp dword ptr [esp + 4] // 0040117e
        fld dword ptr [esp + 4] // 00401182
        mov esp, ebp // 00401186
        pop ebp // 00401188
        ret 4 // 00401189
    }
}
} // namespace
void* NativeDynCollisionPassCalls::allocate_00bf55be(U,U n,const AvoidZoneDynHullMemory& m){return m.allocate(m.context,n);}
void NativeDynCollisionPassCalls::free_00bf6989(U,void* p,const AvoidZoneDynHullMemory& m){m.release(m.context,p);}
void NativeDynCollisionPassCalls::run_batch_00c33140(void* manager,void* const* tasks,std::int32_t count,const AvoidZoneDynHullMemory& m){dyn_task_manager_run_batch_00c33140(*static_cast<DynTaskManagerStorage*>(manager),tasks,count,m);}
std::uint64_t NativeDynCollisionPassCalls::read_timestamp(U) noexcept{_ReadWriteBarrier();const auto t=__rdtsc();_ReadWriteBarrier();return t;}
void refresh_native_dyn_manifold_00c4b9b0(void* manifold){
    __asm {mov edx,manifold}
    __asm {call refresh_kernel}
}
void update_native_dyn_manifolds_00c549d0(void* pool,const NativeDynCollisionPassContext& c){
    Context context=bind(c);auto* p=&context;
    __asm {push p}
    __asm {push pool}
    __asm {call update_kernel}
}
void dispatch_native_dyn_contact_events_00c35480(void* scene){
    __asm {push scene}
    __asm {call events_kernel}
}
void run_native_dyn_collision_pass_00c57070(void* scene,const NativeDynCollisionPassContext& c){
    Context context=bind(c);auto* p=&context;
    __asm {push p}
    __asm {push scene}
    __asm {call pass_kernel}
}
} // namespace bsp

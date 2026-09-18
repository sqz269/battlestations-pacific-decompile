#include "bsp/native_game_contact_reports.hpp"
#include <cstdlib>
#include <new>
#include <stdexcept>
#include <type_traits>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native contact reports require MSVC Win32 x87/SSE assembly.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;
using Context=NativeGameContactReportContext;
void* __cdecl allocate_bridge(U site,Context* c,U size){return c->calls.allocate_00bf681b(site,size,c->memory);}
void __cdecl free_bridge(U site,Context* c,void* p){c->calls.free_00bf65ac(site,p,c->memory);}
void __cdecl invalid_bridge(U site,Context* c){c->calls.invalid_parameter_00bf6713(site);}
[[noreturn]] void __cdecl length_bridge(Context* c){c->calls.length_error_0041f870();}
[[noreturn]] void __cdecl bad_alloc_bridge(Context* c){c->calls.bad_alloc_00415720();}
void report_kernel();
void resize_kernel();
void insert_kernel();
void erase_kernel();
void move_kernel();
void size_kernel();
void allocate_kernel();
void copy_kernel();
void fill_kernel();
void fill_loop_kernel();
void copy_wrapper_kernel();
void assign_fill_kernel();
void backward_wrapper_kernel();
void backward_kernel();
__declspec(naked) void resize_shim(){
    __asm {
        push dword ptr [esp+4]
        push dword ptr [esp+24]
        push dword ptr [esp+24]
        push dword ptr [esp+24]
        push dword ptr [esp+24]
        call resize_kernel
        ret 20
    }
}
__declspec(naked) void insert_shim(){
    __asm {
        push dword ptr [esp+4]
        push dword ptr [esp+24]
        push dword ptr [esp+24]
        push dword ptr [esp+24]
        push dword ptr [esp+24]
        call insert_kernel
        ret 20
    }
}
__declspec(naked) void erase_shim(){
    __asm {
        push dword ptr [esp+4]
        push dword ptr [esp+28]
        push dword ptr [esp+28]
        push dword ptr [esp+28]
        push dword ptr [esp+28]
        push dword ptr [esp+28]
        call erase_kernel
        ret 24
    }
}
__declspec(naked) void length_shim(){
    __asm {
        push dword ptr [esp+4]
        call length_bridge
        int 3
    }
}
__declspec(naked) void bad_alloc_shim(){
    __asm {
        push dword ptr [esp+4]
        call bad_alloc_bridge
        int 3
    }
}
__declspec(naked) void bridge_004d4d30(){
    __asm {
        push dword ptr [esp+4]
        push 0004d4d30h
        call invalid_bridge
        add esp,8
        ret 4
    }
}
__declspec(naked) void bridge_004d1562(){
    __asm {
        push dword ptr [esp+4]
        push 0004d1562h
        call invalid_bridge
        add esp,8
        ret 4
    }
}
__declspec(naked) void bridge_004d15a2(){
    __asm {
        push dword ptr [esp+4]
        push 0004d15a2h
        call invalid_bridge
        add esp,8
        ret 4
    }
}
__declspec(naked) void bridge_004d15af(){
    __asm {
        push dword ptr [esp+4]
        push 0004d15afh
        call invalid_bridge
        add esp,8
        ret 4
    }
}
__declspec(naked) void bridge_004d15c9(){
    __asm {
        push dword ptr [esp+4]
        push 0004d15c9h
        call invalid_bridge
        add esp,8
        ret 4
    }
}
__declspec(naked) void bridge_0041fbbe(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 00041fbbeh
        call free_bridge
        add esp,12
        ret 4
    }
}
__declspec(naked) void bridge_004c82c3(){
    __asm {
        push dword ptr [esp+4]
        push 0004c82c3h
        call invalid_bridge
        add esp,8
        ret 4
    }
}
__declspec(naked) void bridge_00415731(){
    __asm {
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        push 000415731h
        call allocate_bridge
        add esp,12
        ret 4
    }
}
// Full normal instruction schedule; comments identify native instruction starts.
__declspec(naked) void report_kernel(){
    __asm {
        push ecx // 004d4ce0
        push ebx // 004d4ce1
        push ebp // 004d4ce2
        push esi // 004d4ce3
        mov esi, dword ptr [esp + 018h] // 004d4ce4
        push edi // 004d4ce8
        sub esp, 0ch // 004d4ce9
        lea ebx, [ecx + 4] // 004d4cec
        mov dword ptr [esp + 01ch], esp // 004d4cef
        push esi // 004d4cf3
        mov ecx, ebx // 004d4cf4
        push dword ptr [esp+48] // Borrowed context; native frame offsets retained.
        call resize_shim // 004d4cf6
        xor edi, edi // 004d4cfb
        test esi, esi // 004d4cfd
        jle l_004d4d5a // 004d4cff
        mov esi, dword ptr [esp + 018h] // 004d4d01
        xor ebp, ebp // 004d4d05
        add esi, 8 // 004d4d07
        _emit 08dh // 004d4d0a
        _emit 09bh
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    l_004d4d10:
        mov eax, dword ptr [ebx + 4] // 004d4d10
        test eax, eax // 004d4d13
        je l_004d4d30 // 004d4d15
        mov ecx, dword ptr [ebx + 8] // 004d4d17
        sub ecx, eax // 004d4d1a
        mov eax, 02aaaaaabh // 004d4d1c
        imul ecx // 004d4d21
        sar edx, 1 // 004d4d23
        mov eax, edx // 004d4d25
        shr eax, 01fh // 004d4d27
        add eax, edx // 004d4d2a
        cmp edi, eax // 004d4d2c
        jb l_004d4d35 // 004d4d2e
    l_004d4d30:
        push dword ptr [esp+32] // Borrowed context; native frame offsets retained.
        call bridge_004d4d30 // 004d4d30
    l_004d4d35:
        mov eax, dword ptr [ebx + 4] // 004d4d35
        fld dword ptr [esi - 8] // 004d4d38
        add eax, ebp // 004d4d3b
        fstp dword ptr [eax] // 004d4d3d
        add edi, 1 // 004d4d3f
        fld dword ptr [esi - 4] // 004d4d42
        add ebp, 0ch // 004d4d45
        fstp dword ptr [eax + 4] // 004d4d48
        add esi, 058h // 004d4d4b
        cmp edi, dword ptr [esp + 01ch] // 004d4d4e
        fld dword ptr [esi - 058h] // 004d4d52
        fstp dword ptr [eax + 8] // 004d4d55
        jl l_004d4d10 // 004d4d58
    l_004d4d5a:
        pop edi // 004d4d5a
        pop esi // 004d4d5b
        pop ebp // 004d4d5c
        pop ebx // 004d4d5d
        pop ecx // 004d4d5e
        ret 12 // 004d4d5f
    }
}
// Full normal instruction schedule; comments identify native instruction starts.
__declspec(naked) void resize_kernel(){
    __asm {
        push ebx // 004d1510
        push ebp // 004d1511
        push esi // 004d1512
        mov esi, ecx // 004d1513
        mov ecx, dword ptr [esi + 4] // 004d1515
        test ecx, ecx // 004d1518
        push edi // 004d151a
        jne l_004d1521 // 004d151b
        xor eax, eax // 004d151d
        jmp l_004d1536 // 004d151f
    l_004d1521:
        mov edx, dword ptr [esi + 8] // 004d1521
        sub edx, ecx // 004d1524
        mov eax, 02aaaaaabh // 004d1526
        imul edx // 004d152b
        sar edx, 1 // 004d152d
        mov eax, edx // 004d152f
        shr eax, 01fh // 004d1531
        add eax, edx // 004d1534
    l_004d1536:
        mov ebx, dword ptr [esp + 014h] // 004d1536
        cmp eax, ebx // 004d153a
        jae l_004d157f // 004d153c
        test ecx, ecx // 004d153e
        jne l_004d1546 // 004d1540
        xor edi, edi // 004d1542
        jmp l_004d155b // 004d1544
    l_004d1546:
        mov edx, dword ptr [esi + 8] // 004d1546
        sub edx, ecx // 004d1549
        mov eax, 02aaaaaabh // 004d154b
        imul edx // 004d1550
        sar edx, 1 // 004d1552
        mov edi, edx // 004d1554
        shr edi, 01fh // 004d1556
        add edi, edx // 004d1559
    l_004d155b:
        mov ebp, dword ptr [esi + 8] // 004d155b
        cmp ecx, ebp // 004d155e
        jbe l_004d1567 // 004d1560
        push dword ptr [esp+36] // Borrowed context; native frame offsets retained.
        call bridge_004d1562 // 004d1562
    l_004d1567:
        lea eax, [esp + 018h] // 004d1567
        push eax // 004d156b
        sub ebx, edi // 004d156c
        push ebx // 004d156e
        push ebp // 004d156f
        push esi // 004d1570
        mov ecx, esi // 004d1571
        push dword ptr [esp+52] // Borrowed context; native frame offsets retained.
        call insert_shim // 004d1573
        pop edi // 004d1578
        pop esi // 004d1579
        pop ebp // 004d157a
        pop ebx // 004d157b
        ret 20 // 004d157c
    l_004d157f:
        test ecx, ecx // 004d157f
        je l_004d15de // 004d1581
        mov edi, dword ptr [esi + 8] // 004d1583
        mov edx, edi // 004d1586
        sub edx, ecx // 004d1588
        mov eax, 02aaaaaabh // 004d158a
        imul edx // 004d158f
        sar edx, 1 // 004d1591
        mov eax, edx // 004d1593
        shr eax, 01fh // 004d1595
        add eax, edx // 004d1598
        cmp ebx, eax // 004d159a
        jae l_004d15de // 004d159c
        cmp ecx, edi // 004d159e
        jbe l_004d15a7 // 004d15a0
        push dword ptr [esp+36] // Borrowed context; native frame offsets retained.
        call bridge_004d15a2 // 004d15a2
    l_004d15a7:
        mov ebp, dword ptr [esi + 4] // 004d15a7
        cmp ebp, dword ptr [esi + 8] // 004d15aa
        jbe l_004d15b4 // 004d15ad
        push dword ptr [esp+36] // Borrowed context; native frame offsets retained.
        call bridge_004d15af // 004d15af
    l_004d15b4:
        lea ecx, [ebx + ebx*2] // 004d15b4
        lea ebx, [ebp + ecx*4] // 004d15b7
        cmp ebx, dword ptr [esi + 8] // 004d15bb
        mov dword ptr [esp + 01ch], ebp // 004d15be
        ja l_004d15c9 // 004d15c2
        cmp ebx, dword ptr [esi + 4] // 004d15c4
        jae l_004d15ce // 004d15c7
    l_004d15c9:
        push dword ptr [esp+36] // Borrowed context; native frame offsets retained.
        call bridge_004d15c9 // 004d15c9
    l_004d15ce:
        push edi // 004d15ce
        push esi // 004d15cf
        push ebx // 004d15d0
        push esi // 004d15d1
        lea edx, [esp + 028h] // 004d15d2
        push edx // 004d15d6
        mov ecx, esi // 004d15d7
        push dword ptr [esp+56] // Borrowed context; native frame offsets retained.
        call erase_shim // 004d15d9
    l_004d15de:
        pop edi // 004d15de
        pop esi // 004d15df
        pop ebp // 004d15e0
        pop ebx // 004d15e1
        ret 20 // 004d15e2
    }
}
// Full normal instruction schedule; comments identify native instruction starts.
__declspec(naked) void insert_kernel(){
    __asm {
        push ebp // 0041fa40
        mov ebp, esp // 0041fa41
        push -1 // 0041fa43
        push 0 // 0041fa45
        mov eax, dword ptr fs:[0] // 0041fa4a
        push eax // 0041fa50
        // 0041fa51: native FS write omitted; normal-return contract.
        sub esp, 010h // 0041fa58
        mov eax, dword ptr [ebp + 014h] // 0041fa5b
        movss xmm0, dword ptr [eax] // 0041fa5e
        push ebx // 0041fa62
        push esi // 0041fa63
        mov esi, ecx // 0041fa64
        mov ecx, dword ptr [esi + 4] // 0041fa66
        test ecx, ecx // 0041fa69
        movss dword ptr [ebp - 01ch], xmm0 // 0041fa6b
        movss xmm0, dword ptr [eax + 4] // 0041fa70
        push edi // 0041fa75
        movss dword ptr [ebp - 018h], xmm0 // 0041fa76
        movss xmm0, dword ptr [eax + 8] // 0041fa7b
        mov dword ptr [ebp - 010h], esp // 0041fa80
        movss dword ptr [ebp - 014h], xmm0 // 0041fa83
        jne l_0041fa8e // 0041fa88
        xor ebx, ebx // 0041fa8a
        jmp l_0041faa3 // 0041fa8c
    l_0041fa8e:
        mov edx, dword ptr [esi + 0ch] // 0041fa8e
        sub edx, ecx // 0041fa91
        mov eax, 02aaaaaabh // 0041fa93
        imul edx // 0041fa98
        sar edx, 1 // 0041fa9a
        mov ebx, edx // 0041fa9c
        shr ebx, 01fh // 0041fa9e
        add ebx, edx // 0041faa1
    l_0041faa3:
        mov edi, dword ptr [ebp + 010h] // 0041faa3
        test edi, edi // 0041faa6
        je l_0041fccf // 0041faa8
        test ecx, ecx // 0041faae
        jne l_0041fab6 // 0041fab0
        xor eax, eax // 0041fab2
        jmp l_0041facb // 0041fab4
    l_0041fab6:
        mov edx, dword ptr [esi + 8] // 0041fab6
        sub edx, ecx // 0041fab9
        mov eax, 02aaaaaabh // 0041fabb
        imul edx // 0041fac0
        sar edx, 1 // 0041fac2
        mov eax, edx // 0041fac4
        shr eax, 01fh // 0041fac6
        add eax, edx // 0041fac9
    l_0041facb:
        mov edx, 015555555h // 0041facb
        sub edx, eax // 0041fad0
        cmp edx, edi // 0041fad2
        jae l_0041fadb // 0041fad4
        push dword ptr [ebp+24] // Borrowed context; native frame offsets retained.
        call length_shim // 0041fad6
    l_0041fadb:
        test ecx, ecx // 0041fadb
        jne l_0041fae3 // 0041fadd
        xor eax, eax // 0041fadf
        jmp l_0041faf8 // 0041fae1
    l_0041fae3:
        mov edx, dword ptr [esi + 8] // 0041fae3
        sub edx, ecx // 0041fae6
        mov eax, 02aaaaaabh // 0041fae8
        imul edx // 0041faed
        sar edx, 1 // 0041faef
        mov eax, edx // 0041faf1
        shr eax, 01fh // 0041faf3
        add eax, edx // 0041faf6
    l_0041faf8:
        add eax, edi // 0041faf8
        cmp ebx, eax // 0041fafa
        jae l_0041fc06 // 0041fafc
        mov eax, ebx // 0041fb02
        shr eax, 1 // 0041fb04
        mov edx, 015555555h // 0041fb06
        sub edx, eax // 0041fb0b
        cmp edx, ebx // 0041fb0d
        jae l_0041fb15 // 0041fb0f
        xor ebx, ebx // 0041fb11
        jmp l_0041fb17 // 0041fb13
    l_0041fb15:
        add ebx, eax // 0041fb15
    l_0041fb17:
        test ecx, ecx // 0041fb17
        jne l_0041fb1f // 0041fb19
        xor eax, eax // 0041fb1b
        jmp l_0041fb34 // 0041fb1d
    l_0041fb1f:
        mov edx, dword ptr [esi + 8] // 0041fb1f
        sub edx, ecx // 0041fb22
        mov eax, 02aaaaaabh // 0041fb24
        imul edx // 0041fb29
        sar edx, 1 // 0041fb2b
        mov eax, edx // 0041fb2d
        shr eax, 01fh // 0041fb2f
        add eax, edx // 0041fb32
    l_0041fb34:
        add eax, edi // 0041fb34
        cmp ebx, eax // 0041fb36
        jae l_0041fb45 // 0041fb38
        mov ecx, esi // 0041fb3a
        call size_kernel // 0041fb3c
        mov ebx, eax // 0041fb41
        add ebx, edi // 0041fb43
    l_0041fb45:
        xor edx, edx // 0041fb45
        mov ecx, ebx // 0041fb47
        push dword ptr [ebp+24] // Borrowed context; native frame offsets retained.
        call allocate_kernel // 0041fb49
        mov ecx, dword ptr [esi + 4] // 0041fb4e
        mov byte ptr [ebp + 010h], 0 // 0041fb51
        mov edx, dword ptr [ebp + 010h] // 0041fb55
        push edx // 0041fb58
        mov dword ptr [ebp + 014h], eax // 0041fb59
        mov edx, dword ptr [ebp + 014h] // 0041fb5c
        push edx // 0041fb5f
        mov edx, dword ptr [ebp + 0ch] // 0041fb60
        push esi // 0041fb63
        push eax // 0041fb64
        mov dword ptr [ebp - 4], 0 // 0041fb65
        call copy_kernel // 0041fb6c
        lea ecx, [ebp - 01ch] // 0041fb71
        push ecx // 0041fb74
        push edi // 0041fb75
        push eax // 0041fb76
        mov ecx, esi // 0041fb77
        call fill_kernel // 0041fb79
        mov edx, dword ptr [esi + 8] // 0041fb7e
        mov byte ptr [ebp + 010h], 0 // 0041fb81
        mov ecx, dword ptr [ebp + 010h] // 0041fb85
        push ecx // 0041fb88
        mov ecx, dword ptr [ebp + 014h] // 0041fb89
        push ecx // 0041fb8c
        mov ecx, dword ptr [ebp + 0ch] // 0041fb8d
        push esi // 0041fb90
        push eax // 0041fb91
        call copy_kernel // 0041fb92
        mov ecx, dword ptr [esi + 4] // 0041fb97
        test ecx, ecx // 0041fb9a
        jne l_0041fba2 // 0041fb9c
        xor eax, eax // 0041fb9e
        jmp l_0041fbb7 // 0041fba0
    l_0041fba2:
        mov edx, dword ptr [esi + 8] // 0041fba2
        sub edx, ecx // 0041fba5
        mov eax, 02aaaaaabh // 0041fba7
        imul edx // 0041fbac
        sar edx, 1 // 0041fbae
        mov eax, edx // 0041fbb0
        shr eax, 01fh // 0041fbb2
        add eax, edx // 0041fbb5
    l_0041fbb7:
        add edi, eax // 0041fbb7
        test ecx, ecx // 0041fbb9
        je l_0041fbc6 // 0041fbbb
        push ecx // 0041fbbd
        push dword ptr [ebp+24] // Borrowed context; native frame offsets retained.
        call bridge_0041fbbe // 0041fbbe
        add esp, 4 // 0041fbc3
    l_0041fbc6:
        mov eax, dword ptr [ebp + 014h] // 0041fbc6
        lea edx, [ebx + ebx*2] // 0041fbc9
        lea ecx, [eax + edx*4] // 0041fbcc
        lea edx, [edi + edi*2] // 0041fbcf
        mov dword ptr [esi + 0ch], ecx // 0041fbd2
        lea ecx, [eax + edx*4] // 0041fbd5
        mov dword ptr [esi + 8], ecx // 0041fbd8
        mov dword ptr [esi + 4], eax // 0041fbdb
        mov ecx, dword ptr [ebp - 0ch] // 0041fbde
        // 0041fbe1: native FS write omitted; normal-return contract.
        pop edi // 0041fbe8
        pop esi // 0041fbe9
        pop ebx // 0041fbea
        mov esp, ebp // 0041fbeb
        pop ebp // 0041fbed
        ret 20 // 0041fbee
        // 0041fbf1..0041fc05: native FH3 catch cleanup/rethrow omitted.
    l_0041fc06:
        mov ecx, dword ptr [esi + 8] // 0041fc06
        mov ebx, dword ptr [ebp + 0ch] // 0041fc09
        mov edx, ecx // 0041fc0c
        sub edx, ebx // 0041fc0e
        mov eax, 02aaaaaabh // 0041fc10
        imul edx // 0041fc15
        sar edx, 1 // 0041fc17
        mov eax, edx // 0041fc19
        shr eax, 01fh // 0041fc1b
        add eax, edx // 0041fc1e
        cmp eax, edi // 0041fc20
        mov dword ptr [ebp + 014h], ecx // 0041fc22
        jae l_0041fc96 // 0041fc25
        lea eax, [edi + edi*2] // 0041fc27
        add eax, eax // 0041fc2a
        add eax, eax // 0041fc2c
        mov dword ptr [ebp + 014h], eax // 0041fc2e
        add eax, ebx // 0041fc31
        push eax // 0041fc33
        push ecx // 0041fc34
        push ebx // 0041fc35
        mov ecx, esi // 0041fc36
        call copy_wrapper_kernel // 0041fc38
        mov ecx, dword ptr [esi + 8] // 0041fc3d
        lea edx, [ebp - 01ch] // 0041fc40
        push edx // 0041fc43
        mov edx, ecx // 0041fc44
        sub edx, ebx // 0041fc46
        mov eax, 02aaaaaabh // 0041fc48
        imul edx // 0041fc4d
        sar edx, 1 // 0041fc4f
        mov eax, edx // 0041fc51
        shr eax, 01fh // 0041fc53
        add eax, edx // 0041fc56
        sub edi, eax // 0041fc58
        push edi // 0041fc5a
        push ecx // 0041fc5b
        mov ecx, esi // 0041fc5c
        mov dword ptr [ebp - 4], 2 // 0041fc5e
        call fill_kernel // 0041fc65
        mov eax, dword ptr [ebp + 014h] // 0041fc6a
        add dword ptr [esi + 8], eax // 0041fc6d
        mov esi, dword ptr [esi + 8] // 0041fc70
        lea ecx, [ebp - 01ch] // 0041fc73
        push ecx // 0041fc76
        sub esi, eax // 0041fc77
        push esi // 0041fc79
        push ebx // 0041fc7a
        call assign_fill_kernel // 0041fc7b
        add esp, 0ch // 0041fc80
        mov ecx, dword ptr [ebp - 0ch] // 0041fc83
        // 0041fc86: native FS write omitted; normal-return contract.
        pop edi // 0041fc8d
        pop esi // 0041fc8e
        pop ebx // 0041fc8f
        mov esp, ebp // 0041fc90
        pop ebp // 0041fc92
        ret 20 // 0041fc93
    l_0041fc96:
        lea edi, [edi + edi*2] // 0041fc96
        push ecx // 0041fc99
        add edi, edi // 0041fc9a
        mov eax, ecx // 0041fc9c
        add edi, edi // 0041fc9e
        sub eax, edi // 0041fca0
        push ecx // 0041fca2
        push eax // 0041fca3
        mov ecx, esi // 0041fca4
        mov dword ptr [ebp + 010h], eax // 0041fca6
        call copy_wrapper_kernel // 0041fca9
        mov edx, dword ptr [ebp + 014h] // 0041fcae
        mov dword ptr [esi + 8], eax // 0041fcb1
        mov eax, dword ptr [ebp + 010h] // 0041fcb4
        push edx // 0041fcb7
        push eax // 0041fcb8
        push ebx // 0041fcb9
        call backward_wrapper_kernel // 0041fcba
        lea ecx, [ebp - 01ch] // 0041fcbf
        push ecx // 0041fcc2
        add edi, ebx // 0041fcc3
        push edi // 0041fcc5
        push ebx // 0041fcc6
        call assign_fill_kernel // 0041fcc7
        add esp, 018h // 0041fccc
    l_0041fccf:
        mov ecx, dword ptr [ebp - 0ch] // 0041fccf
        pop edi // 0041fcd2
        pop esi // 0041fcd3
        // 0041fcd4: native FS write omitted; normal-return contract.
        pop ebx // 0041fcdb
        mov esp, ebp // 0041fcdc
        pop ebp // 0041fcde
        ret 20 // 0041fcdf
    }
}
// Full normal instruction schedule; comments identify native instruction starts.
__declspec(naked) void erase_kernel(){
    __asm {
        push ebx // 004c82b0
        mov ebx, dword ptr [esp + 0ch] // 004c82b1
        test ebx, ebx // 004c82b5
        push esi // 004c82b7
        push edi // 004c82b8
        mov esi, ecx // 004c82b9
        je l_004c82c3 // 004c82bb
        cmp ebx, dword ptr [esp + 01ch] // 004c82bd
        je l_004c82c8 // 004c82c1
    l_004c82c3:
        push dword ptr [esp+36] // Borrowed context; native frame offsets retained.
        call bridge_004c82c3 // 004c82c3
    l_004c82c8:
        mov edi, dword ptr [esp + 018h] // 004c82c8
        mov ecx, dword ptr [esp + 020h] // 004c82cc
        cmp edi, ecx // 004c82d0
        je l_004c82f9 // 004c82d2
        mov eax, dword ptr [esi + 8] // 004c82d4
        mov byte ptr [esp + 014h], 0 // 004c82d7
        mov edx, dword ptr [esp + 014h] // 004c82dc
        push edx // 004c82e0
        mov edx, dword ptr [esp + 014h] // 004c82e1
        push edx // 004c82e5
        mov edx, dword ptr [esp + 018h] // 004c82e6
        push edx // 004c82ea
        push edi // 004c82eb
        push eax // 004c82ec
        push ecx // 004c82ed
        call move_kernel // 004c82ee
        add esp, 018h // 004c82f3
        mov dword ptr [esi + 8], eax // 004c82f6
    l_004c82f9:
        mov eax, dword ptr [esp + 010h] // 004c82f9
        mov dword ptr [eax + 4], edi // 004c82fd
        pop edi // 004c8300
        pop esi // 004c8301
        mov dword ptr [eax], ebx // 004c8302
        pop ebx // 004c8304
        ret 24 // 004c8305
    }
}
// Full normal instruction schedule; comments identify native instruction starts.
__declspec(naked) void move_kernel(){
    __asm {
        push ebx // 00419330
        mov ebx, dword ptr [esp + 010h] // 00419331
        push esi // 00419335
        mov esi, dword ptr [esp + 0ch] // 00419336
        push edi // 0041933a
        mov edi, dword ptr [esp + 014h] // 0041933b
        mov ecx, edi // 0041933f
        sub ecx, esi // 00419341
        mov eax, 02aaaaaabh // 00419343
        imul ecx // 00419348
        sar edx, 1 // 0041934a
        mov eax, edx // 0041934c
        shr eax, 01fh // 0041934e
        add eax, edx // 00419351
        cmp esi, edi // 00419353
        lea eax, [eax + eax*2] // 00419355
        lea eax, [ebx + eax*4] // 00419358
        mov edx, esi // 0041935b
        je l_00419380 // 0041935d
        lea ecx, [ebx + 8] // 0041935f
        sub esi, ebx // 00419362
    l_00419364:
        fld dword ptr [edx] // 00419364
        add edx, 0ch // 00419366
        fstp dword ptr [ecx - 8] // 00419369
        add ecx, 0ch // 0041936c
        cmp edx, edi // 0041936f
        fld dword ptr [edx - 8] // 00419371
        fstp dword ptr [ecx - 010h] // 00419374
        fld dword ptr [esi + ecx - 0ch] // 00419377
        fstp dword ptr [ecx - 0ch] // 0041937b
        jne l_00419364 // 0041937e
    l_00419380:
        pop edi // 00419380
        pop esi // 00419381
        pop ebx // 00419382
        ret  // 00419383
    }
}
// Full normal instruction schedule; comments identify native instruction starts.
__declspec(naked) void size_kernel(){
    __asm {
        mov eax, dword ptr [ecx + 4] // 00415290
        test eax, eax // 00415293
        jne l_00415298 // 00415295
        ret  // 00415297
    l_00415298:
        mov ecx, dword ptr [ecx + 8] // 00415298
        sub ecx, eax // 0041529b
        mov eax, 02aaaaaabh // 0041529d
        imul ecx // 004152a2
        sar edx, 1 // 004152a4
        mov eax, edx // 004152a6
        shr eax, 01fh // 004152a8
        add eax, edx // 004152ab
        ret  // 004152ad
    }
}
// Full normal instruction schedule; comments identify native instruction starts.
__declspec(naked) void allocate_kernel(){
    __asm {
        sub esp, 010h // 00415720
        test ecx, ecx // 00415723
        ja l_0041573d // 00415725
        xor ecx, ecx // 00415727
    l_00415729:
        lea edx, [ecx + ecx*2] // 00415729
        add edx, edx // 0041572c
        add edx, edx // 0041572e
        push edx // 00415730
        push dword ptr [esp+24] // Borrowed context; native frame offsets retained.
        call bridge_00415731 // 00415731
        add esp, 4 // 00415736
        add esp, 010h // 00415739
        ret 4 // 0041573c
    l_0041573d:
        or eax, 0ffffffffh // 0041573d
        xor edx, edx // 00415740
        div ecx // 00415742
        cmp eax, 0ch // 00415744
        jae l_00415729 // 00415747
        push dword ptr [esp+20] // Native bad_alloc construction replaced by source exception service.
        call bad_alloc_shim
    }
}
// Full normal instruction schedule; comments identify native instruction starts.
__declspec(naked) void copy_kernel(){
    __asm {
        cmp ecx, edx // 00419900
        mov eax, dword ptr [esp + 4] // 00419902
        je l_00419926 // 00419906
    l_00419908:
        test eax, eax // 00419908
        je l_0041991c // 0041990a
        fld dword ptr [ecx] // 0041990c
        fstp dword ptr [eax] // 0041990e
        fld dword ptr [ecx + 4] // 00419910
        fstp dword ptr [eax + 4] // 00419913
        fld dword ptr [ecx + 8] // 00419916
        fstp dword ptr [eax + 8] // 00419919
    l_0041991c:
        add ecx, 0ch // 0041991c
        add eax, 0ch // 0041991f
        cmp ecx, edx // 00419922
        jne l_00419908 // 00419924
    l_00419926:
        ret 010h // 00419926
    }
}
// Full normal instruction schedule; comments identify native instruction starts.
__declspec(naked) void fill_kernel(){
    __asm {
        push ecx // 0041e310
        mov edx, dword ptr [esp + 010h] // 0041e311
        push esi // 0041e315
        mov esi, dword ptr [esp + 010h] // 0041e316
        push edi // 0041e31a
        mov edi, dword ptr [esp + 010h] // 0041e31b
        mov byte ptr [esp + 8], 0 // 0041e31f
        mov eax, dword ptr [esp + 8] // 0041e324
        push eax // 0041e328
        mov eax, dword ptr [esp + 01ch] // 0041e329
        push edx // 0041e32d
        push ecx // 0041e32e
        push eax // 0041e32f
        mov edx, esi // 0041e330
        mov ecx, edi // 0041e332
        call fill_loop_kernel // 0041e334
        lea ecx, [esi + esi*2] // 0041e339
        lea eax, [edi + ecx*4] // 0041e33c
        pop edi // 0041e33f
        pop esi // 0041e340
        pop ecx // 0041e341
        ret 0ch // 0041e342
    }
}
// Full normal instruction schedule; comments identify native instruction starts.
__declspec(naked) void fill_loop_kernel(){
    __asm {
        test edx, edx // 0041c7b0
        jbe l_0041c7d6 // 0041c7b2
        mov eax, dword ptr [esp + 4] // 0041c7b4
    l_0041c7b8:
        test ecx, ecx // 0041c7b8
        je l_0041c7cc // 0041c7ba
        fld dword ptr [eax] // 0041c7bc
        fstp dword ptr [ecx] // 0041c7be
        fld dword ptr [eax + 4] // 0041c7c0
        fstp dword ptr [ecx + 4] // 0041c7c3
        fld dword ptr [eax + 8] // 0041c7c6
        fstp dword ptr [ecx + 8] // 0041c7c9
    l_0041c7cc:
        sub edx, 1 // 0041c7cc
        add ecx, 0ch // 0041c7cf
        test edx, edx // 0041c7d2
        ja l_0041c7b8 // 0041c7d4
    l_0041c7d6:
        ret 010h // 0041c7d6
    }
}
// Full normal instruction schedule; comments identify native instruction starts.
__declspec(naked) void copy_wrapper_kernel(){
    __asm {
        push ecx // 0041f480
        mov edx, dword ptr [esp + 010h] // 0041f481
        mov byte ptr [esp], 0 // 0041f485
        mov eax, dword ptr [esp] // 0041f489
        push eax // 0041f48c
        mov eax, dword ptr [esp + 014h] // 0041f48d
        push edx // 0041f491
        mov edx, dword ptr [esp + 014h] // 0041f492
        push ecx // 0041f496
        mov ecx, dword ptr [esp + 014h] // 0041f497
        push eax // 0041f49b
        call copy_kernel // 0041f49c
        pop ecx // 0041f4a1
        ret 0ch // 0041f4a2
    }
}
// Full normal instruction schedule; comments identify native instruction starts.
__declspec(naked) void assign_fill_kernel(){
    __asm {
        mov eax, dword ptr [esp + 4] // 0041c500
        mov edx, dword ptr [esp + 8] // 0041c504
        cmp eax, edx // 0041c508
        je l_0041c528 // 0041c50a
        mov ecx, dword ptr [esp + 0ch] // 0041c50c
    l_0041c510:
        fld dword ptr [ecx] // 0041c510
        add eax, 0ch // 0041c512
        cmp eax, edx // 0041c515
        fstp dword ptr [eax - 0ch] // 0041c517
        fld dword ptr [ecx + 4] // 0041c51a
        fstp dword ptr [eax - 8] // 0041c51d
        fld dword ptr [ecx + 8] // 0041c520
        fstp dword ptr [eax - 4] // 0041c523
        jne l_0041c510 // 0041c526
    l_0041c528:
        ret  // 0041c528
    }
}
// Full normal instruction schedule; comments identify native instruction starts.
__declspec(naked) void backward_wrapper_kernel(){
    __asm {
        push ecx // 0041d950
        mov ecx, dword ptr [esp + 010h] // 0041d951
        mov edx, dword ptr [esp + 010h] // 0041d955
        mov byte ptr [esp], 0 // 0041d959
        mov eax, dword ptr [esp] // 0041d95d
        push eax // 0041d960
        mov eax, dword ptr [esp + 014h] // 0041d961
        push ecx // 0041d965
        mov ecx, dword ptr [esp + 014h] // 0041d966
        push edx // 0041d96a
        mov edx, dword ptr [esp + 014h] // 0041d96b
        push eax // 0041d96f
        push ecx // 0041d970
        push edx // 0041d971
        call backward_kernel // 0041d972
        add esp, 01ch // 0041d977
        ret  // 0041d97a
    }
}
// Full normal instruction schedule; comments identify native instruction starts.
__declspec(naked) void backward_kernel(){
    __asm {
        push ebx // 00419710
        push esi // 00419711
        mov esi, dword ptr [esp + 010h] // 00419712
        mov ebx, dword ptr [esp + 014h] // 00419716
        push edi // 0041971a
        mov edi, dword ptr [esp + 010h] // 0041971b
        mov ecx, esi // 0041971f
        sub ecx, edi // 00419721
        mov eax, 02aaaaaabh // 00419723
        imul ecx // 00419728
        sar edx, 1 // 0041972a
        mov eax, edx // 0041972c
        shr eax, 01fh // 0041972e
        add eax, edx // 00419731
        lea eax, [eax + eax*2] // 00419733
        add eax, eax // 00419736
        add eax, eax // 00419738
        mov ecx, eax // 0041973a
        mov eax, ebx // 0041973c
        sub eax, ecx // 0041973e
        cmp edi, esi // 00419740
        mov edx, esi // 00419742
        je l_0041976b // 00419744
        lea ecx, [ebx + 8] // 00419746
        sub esi, ebx // 00419749
        jmp l_00419750 // 0041974b
        _emit 08dh // 0041974d
        _emit 049h
        _emit 000h
    l_00419750:
        fld dword ptr [edx - 0ch] // 00419750
        sub edx, 0ch // 00419753
        sub ecx, 0ch // 00419756
        cmp edx, edi // 00419759
        fstp dword ptr [ecx - 8] // 0041975b
        fld dword ptr [edx + 4] // 0041975e
        fstp dword ptr [ecx - 4] // 00419761
        fld dword ptr [esi + ecx] // 00419764
        fstp dword ptr [ecx] // 00419767
        jne l_00419750 // 00419769
    l_0041976b:
        pop edi // 0041976b
        pop esi // 0041976c
        pop ebx // 0041976d
        ret  // 0041976e
    }
}
} // namespace
void* NativeGameContactReportCalls::allocate_00bf681b(U,U n,const AvoidZoneDynHullMemory& m){return m.allocate(m.context,n);}
void NativeGameContactReportCalls::free_00bf65ac(U,void* p,const AvoidZoneDynHullMemory& m){m.release(m.context,p);}
void NativeGameContactReportCalls::invalid_parameter_00bf6713(U){_invalid_parameter_noinfo();}
void NativeGameContactReportCalls::length_error_0041f870(){throw std::length_error("vector<T> too long");}
void NativeGameContactReportCalls::bad_alloc_00415720(){throw std::bad_alloc();}
void receive_native_game_contact_reports_004d4ce0(void* receiver,const void* records,std::int32_t count,const NativeGameContactReportContext& c){auto* p=&c;__asm {push p} __asm {push count} __asm {push records} __asm {mov ecx,receiver} __asm {call report_kernel}}
void resize_native_checked_vector12_004d1510(void* vector,U count,const void* fill12,const NativeGameContactReportContext& c){auto* p=&c;__asm {push p} __asm {mov eax,fill12} __asm {push dword ptr [eax+8]} __asm {push dword ptr [eax+4]} __asm {push dword ptr [eax]} __asm {push count} __asm {mov ecx,vector} __asm {call resize_kernel}}
void insert_native_checked_vector12_0041fa40(void* vector,void* owner,void* position,U count,const void* value12,const NativeGameContactReportContext& c){auto* p=&c;__asm {push p} __asm {push value12} __asm {push count} __asm {push position} __asm {push owner} __asm {mov ecx,vector} __asm {call insert_kernel}}
void* erase_native_checked_vector12_004c82b0(void* vector,void* output,void* first_owner,void* first,void* last_owner,void* last,const NativeGameContactReportContext& c){auto* p=&c;void* result;__asm {push p} __asm {push last} __asm {push last_owner} __asm {push first} __asm {push first_owner} __asm {push output} __asm {mov ecx,vector} __asm {call erase_kernel} __asm {mov result,eax} return result;}
std::int32_t native_checked_vector12_size_00415290(const void* vector){std::int32_t result;__asm {mov ecx,vector} __asm {call size_kernel} __asm {mov result,eax} return result;}
NativeGameContactReportRuntime::NativeGameContactReportRuntime(const AvoidZoneDynHullMemory& m,NativeGameContactReportCalls& c) noexcept:methods_{reinterpret_cast<std::uintptr_t>(&receive)},memory_(m),calls_(&c){static_assert(std::is_standard_layout_v<NativeGameContactReportRuntime>);static_assert(offsetof(NativeGameContactReportRuntime,methods_)==0);}
void NativeGameContactReportRuntime::bind(void* receiver) const noexcept{*static_cast<const void**>(receiver)=methods_;}
void __fastcall NativeGameContactReportRuntime::receive(void* receiver,void*,const void* records,std::int32_t count){const auto* owner=reinterpret_cast<const NativeGameContactReportRuntime*>(*static_cast<const void* const*>(receiver));NativeGameContactReportContext c{owner->memory_,*owner->calls_};receive_native_game_contact_reports_004d4ce0(receiver,records,count,c);}
} // namespace bsp

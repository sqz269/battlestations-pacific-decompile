#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_crt_vector_copy.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT memcpy requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);

// BF7680: all 247 native payload instructions are represented. Six constant
// tables and skipped padding are evidence only; eight known-domain ladders
// replace the 16 indexed transfers without stack touches or selector changes.
__declspec(naked) void* __cdecl copy_native_crt_bytes_00bf7680(
    void*, const void*, std::uint32_t, const volatile std::uint32_t&) {
    __asm {
        // Native 00bf7680
        push ebp
        // Native 00bf7681
        mov ebp, esp
        // Native 00bf7683
        push edi
        // Native 00bf7684
        push esi
        // Native 00bf7685
        mov esi, dword ptr [ebp + 0xc]
        // Native 00bf7688
        mov ecx, dword ptr [ebp + 0x10]
        // Native 00bf768b
        mov edi, dword ptr [ebp + 8]
        // Native 00bf768e
        mov eax, ecx
        // Native 00bf7690
        mov edx, ecx
        // Native 00bf7692
        add eax, esi
        // Native 00bf7694
        cmp edi, esi
        // Native 00bf7696
        jbe native_00bf76a0
        // Native 00bf7698
        cmp edi, eax
        // Native 00bf769a
        jb native_00bf7844
    native_00bf76a0:
        cmp ecx, 0x100
        // Native 00bf76a6
        jb native_00bf76c7
        // Native 00bf76a8
        // The completed direction decision has made EAX dead.
        mov eax, dword ptr [ebp + 0x14]
        cmp dword ptr [eax], 0
        // Native 00bf76af
        je native_00bf76c7
        // Native 00bf76b1
        push edi
        // Native 00bf76b2
        push esi
        // Native 00bf76b3
        and edi, 0xf
        // Native 00bf76b6
        and esi, 0xf
        // Native 00bf76b9
        cmp edi, esi
        // Native 00bf76bb
        pop esi
        // Native 00bf76bc
        pop edi
        // Native 00bf76bd
        jne native_00bf76c7
        // Native 00bf76bf
        pop esi
        // Native 00bf76c0
        pop edi
        // Native 00bf76c1
        pop ebp
        // Native 00bf76c2
        jmp copy_native_crt_vector_00c0c82b
    native_00bf76c7:
        test edi, 3
        // Native 00bf76cd
        jne native_00bf76e4
        // Native 00bf76cf
        shr ecx, 2
        // Native 00bf76d2
        and edx, 3
        // Native 00bf76d5
        cmp ecx, 8
        // Native 00bf76d8
        jb native_00bf7704
        // Native 00bf76da
        rep movsd
        // Native 00bf76dc
        jmp dispatch_forward_remainder
    native_00bf76e4:
        mov eax, edi
        // Native 00bf76e6
        mov edx, 3
        // Native 00bf76eb
        sub ecx, 4
        // Native 00bf76ee
        jb native_00bf76fc
        // Native 00bf76f0
        and eax, 3
        // Native 00bf76f3
        add ecx, eax
        // Native 00bf76f5
        jmp dispatch_forward_alignment
    native_00bf76fc:
        jmp dispatch_forward_short
    native_00bf7704:
        jmp dispatch_forward_dwords
    native_00bf7718:
        and edx, ecx
        // Native 00bf771a
        mov al, byte ptr [esi]
        // Native 00bf771c
        mov byte ptr [edi], al
        // Native 00bf771e
        mov al, byte ptr [esi + 1]
        // Native 00bf7721
        mov byte ptr [edi + 1], al
        // Native 00bf7724
        mov al, byte ptr [esi + 2]
        // Native 00bf7727
        shr ecx, 2
        // Native 00bf772a
        mov byte ptr [edi + 2], al
        // Native 00bf772d
        add esi, 3
        // Native 00bf7730
        add edi, 3
        // Native 00bf7733
        cmp ecx, 8
        // Native 00bf7736
        jb native_00bf7704
        // Native 00bf7738
        rep movsd
        // Native 00bf773a
        jmp dispatch_forward_remainder
    native_00bf7744:
        and edx, ecx
        // Native 00bf7746
        mov al, byte ptr [esi]
        // Native 00bf7748
        mov byte ptr [edi], al
        // Native 00bf774a
        mov al, byte ptr [esi + 1]
        // Native 00bf774d
        shr ecx, 2
        // Native 00bf7750
        mov byte ptr [edi + 1], al
        // Native 00bf7753
        add esi, 2
        // Native 00bf7756
        add edi, 2
        // Native 00bf7759
        cmp ecx, 8
        // Native 00bf775c
        jb native_00bf7704
        // Native 00bf775e
        rep movsd
        // Native 00bf7760
        jmp dispatch_forward_remainder
    native_00bf7768:
        and edx, ecx
        // Native 00bf776a
        mov al, byte ptr [esi]
        // Native 00bf776c
        mov byte ptr [edi], al
        // Native 00bf776e
        add esi, 1
        // Native 00bf7771
        shr ecx, 2
        // Native 00bf7774
        add edi, 1
        // Native 00bf7777
        cmp ecx, 8
        // Native 00bf777a
        jb native_00bf7704
        // Native 00bf777c
        rep movsd
        // Native 00bf777e
        jmp dispatch_forward_remainder
    native_00bf77a8:
        mov eax, dword ptr [esi + ecx*4 - 0x1c]
        // Native 00bf77ac
        mov dword ptr [edi + ecx*4 - 0x1c], eax
    native_00bf77b0:
        mov eax, dword ptr [esi + ecx*4 - 0x18]
        // Native 00bf77b4
        mov dword ptr [edi + ecx*4 - 0x18], eax
    native_00bf77b8:
        mov eax, dword ptr [esi + ecx*4 - 0x14]
        // Native 00bf77bc
        mov dword ptr [edi + ecx*4 - 0x14], eax
    native_00bf77c0:
        mov eax, dword ptr [esi + ecx*4 - 0x10]
        // Native 00bf77c4
        mov dword ptr [edi + ecx*4 - 0x10], eax
    native_00bf77c8:
        mov eax, dword ptr [esi + ecx*4 - 0xc]
        // Native 00bf77cc
        mov dword ptr [edi + ecx*4 - 0xc], eax
    native_00bf77d0:
        mov eax, dword ptr [esi + ecx*4 - 8]
        // Native 00bf77d4
        mov dword ptr [edi + ecx*4 - 8], eax
    native_00bf77d8:
        mov eax, dword ptr [esi + ecx*4 - 4]
        // Native 00bf77dc
        mov dword ptr [edi + ecx*4 - 4], eax
        // Native 00bf77e0
        lea eax, [ecx*4]
        // Native 00bf77e7
        add esi, eax
        // Native 00bf77e9
        add edi, eax
    native_00bf77eb:
        jmp dispatch_forward_remainder
    native_00bf7804:
        mov eax, dword ptr [ebp + 8]
        // Native 00bf7807
        pop esi
        // Native 00bf7808
        pop edi
        // Native 00bf7809
        leave
        // Native 00bf780a
        ret
    native_00bf780c:
        mov al, byte ptr [esi]
        // Native 00bf780e
        mov byte ptr [edi], al
        // Native 00bf7810
        mov eax, dword ptr [ebp + 8]
        // Native 00bf7813
        pop esi
        // Native 00bf7814
        pop edi
        // Native 00bf7815
        leave
        // Native 00bf7816
        ret
    native_00bf7818:
        mov al, byte ptr [esi]
        // Native 00bf781a
        mov byte ptr [edi], al
        // Native 00bf781c
        mov al, byte ptr [esi + 1]
        // Native 00bf781f
        mov byte ptr [edi + 1], al
        // Native 00bf7822
        mov eax, dword ptr [ebp + 8]
        // Native 00bf7825
        pop esi
        // Native 00bf7826
        pop edi
        // Native 00bf7827
        leave
        // Native 00bf7828
        ret
    native_00bf782c:
        mov al, byte ptr [esi]
        // Native 00bf782e
        mov byte ptr [edi], al
        // Native 00bf7830
        mov al, byte ptr [esi + 1]
        // Native 00bf7833
        mov byte ptr [edi + 1], al
        // Native 00bf7836
        mov al, byte ptr [esi + 2]
        // Native 00bf7839
        mov byte ptr [edi + 2], al
        // Native 00bf783c
        mov eax, dword ptr [ebp + 8]
        // Native 00bf783f
        pop esi
        // Native 00bf7840
        pop edi
        // Native 00bf7841
        leave
        // Native 00bf7842
        ret
    native_00bf7844:
        lea esi, [ecx + esi - 4]
        // Native 00bf7848
        lea edi, [ecx + edi - 4]
        // Native 00bf784c
        test edi, 3
        // Native 00bf7852
        jne native_00bf7878
        // Native 00bf7854
        shr ecx, 2
        // Native 00bf7857
        and edx, 3
        // Native 00bf785a
        cmp ecx, 8
        // Native 00bf785d
        jb native_00bf786c
        // Native 00bf785f
        std
        // Native 00bf7860
        rep movsd
        // Native 00bf7862
        cld
        // Native 00bf7863
        jmp dispatch_backward_remainder
    native_00bf786c:
        neg ecx
        // Native 00bf786e
        jmp dispatch_backward_dwords
    native_00bf7878:
        mov eax, edi
        // Native 00bf787a
        mov edx, 3
        // Native 00bf787f
        cmp ecx, 4
        // Native 00bf7882
        jb native_00bf7890
        // Native 00bf7884
        and eax, 3
        // Native 00bf7887
        sub ecx, eax
        // Native 00bf7889
        jmp dispatch_backward_alignment
    native_00bf7890:
        jmp dispatch_backward_short
    native_00bf78a4:
        mov al, byte ptr [esi + 3]
        // Native 00bf78a7
        and edx, ecx
        // Native 00bf78a9
        mov byte ptr [edi + 3], al
        // Native 00bf78ac
        sub esi, 1
        // Native 00bf78af
        shr ecx, 2
        // Native 00bf78b2
        sub edi, 1
        // Native 00bf78b5
        cmp ecx, 8
        // Native 00bf78b8
        jb native_00bf786c
        // Native 00bf78ba
        std
        // Native 00bf78bb
        rep movsd
        // Native 00bf78bd
        cld
        // Native 00bf78be
        jmp dispatch_backward_remainder
    native_00bf78c8:
        mov al, byte ptr [esi + 3]
        // Native 00bf78cb
        and edx, ecx
        // Native 00bf78cd
        mov byte ptr [edi + 3], al
        // Native 00bf78d0
        mov al, byte ptr [esi + 2]
        // Native 00bf78d3
        shr ecx, 2
        // Native 00bf78d6
        mov byte ptr [edi + 2], al
        // Native 00bf78d9
        sub esi, 2
        // Native 00bf78dc
        sub edi, 2
        // Native 00bf78df
        cmp ecx, 8
        // Native 00bf78e2
        jb native_00bf786c
        // Native 00bf78e4
        std
        // Native 00bf78e5
        rep movsd
        // Native 00bf78e7
        cld
        // Native 00bf78e8
        jmp dispatch_backward_remainder
    native_00bf78f0:
        mov al, byte ptr [esi + 3]
        // Native 00bf78f3
        and edx, ecx
        // Native 00bf78f5
        mov byte ptr [edi + 3], al
        // Native 00bf78f8
        mov al, byte ptr [esi + 2]
        // Native 00bf78fb
        mov byte ptr [edi + 2], al
        // Native 00bf78fe
        mov al, byte ptr [esi + 1]
        // Native 00bf7901
        shr ecx, 2
        // Native 00bf7904
        mov byte ptr [edi + 1], al
        // Native 00bf7907
        sub esi, 3
        // Native 00bf790a
        sub edi, 3
        // Native 00bf790d
        cmp ecx, 8
        // Native 00bf7910
        jb native_00bf786c
        // Native 00bf7916
        std
        // Native 00bf7917
        rep movsd
        // Native 00bf7919
        cld
        // Native 00bf791a
        jmp dispatch_backward_remainder
    native_00bf7944:
        mov eax, dword ptr [esi + ecx*4 + 0x1c]
        // Native 00bf7948
        mov dword ptr [edi + ecx*4 + 0x1c], eax
    native_00bf794c:
        mov eax, dword ptr [esi + ecx*4 + 0x18]
        // Native 00bf7950
        mov dword ptr [edi + ecx*4 + 0x18], eax
    native_00bf7954:
        mov eax, dword ptr [esi + ecx*4 + 0x14]
        // Native 00bf7958
        mov dword ptr [edi + ecx*4 + 0x14], eax
    native_00bf795c:
        mov eax, dword ptr [esi + ecx*4 + 0x10]
        // Native 00bf7960
        mov dword ptr [edi + ecx*4 + 0x10], eax
    native_00bf7964:
        mov eax, dword ptr [esi + ecx*4 + 0xc]
        // Native 00bf7968
        mov dword ptr [edi + ecx*4 + 0xc], eax
    native_00bf796c:
        mov eax, dword ptr [esi + ecx*4 + 8]
        // Native 00bf7970
        mov dword ptr [edi + ecx*4 + 8], eax
    native_00bf7974:
        mov eax, dword ptr [esi + ecx*4 + 4]
        // Native 00bf7978
        mov dword ptr [edi + ecx*4 + 4], eax
        // Native 00bf797c
        lea eax, [ecx*4]
        // Native 00bf7983
        add esi, eax
        // Native 00bf7985
        add edi, eax
    native_00bf7987:
        jmp dispatch_backward_remainder
    native_00bf79a0:
        mov eax, dword ptr [ebp + 8]
        // Native 00bf79a3
        pop esi
        // Native 00bf79a4
        pop edi
        // Native 00bf79a5
        leave
        // Native 00bf79a6
        ret
    native_00bf79a8:
        mov al, byte ptr [esi + 3]
        // Native 00bf79ab
        mov byte ptr [edi + 3], al
        // Native 00bf79ae
        mov eax, dword ptr [ebp + 8]
        // Native 00bf79b1
        pop esi
        // Native 00bf79b2
        pop edi
        // Native 00bf79b3
        leave
        // Native 00bf79b4
        ret
    native_00bf79b8:
        mov al, byte ptr [esi + 3]
        // Native 00bf79bb
        mov byte ptr [edi + 3], al
        // Native 00bf79be
        mov al, byte ptr [esi + 2]
        // Native 00bf79c1
        mov byte ptr [edi + 2], al
        // Native 00bf79c4
        mov eax, dword ptr [ebp + 8]
        // Native 00bf79c7
        pop esi
        // Native 00bf79c8
        pop edi
        // Native 00bf79c9
        leave
        // Native 00bf79ca
        ret
    native_00bf79cc:
        mov al, byte ptr [esi + 3]
        // Native 00bf79cf
        mov byte ptr [edi + 3], al
        // Native 00bf79d2
        mov al, byte ptr [esi + 2]
        // Native 00bf79d5
        mov byte ptr [edi + 2], al
        // Native 00bf79d8
        mov al, byte ptr [esi + 1]
        // Native 00bf79db
        mov byte ptr [edi + 1], al
        // Native 00bf79de
        mov eax, dword ptr [ebp + 8]
        // Native 00bf79e1
        pop esi
        // Native 00bf79e2
        pop edi
        // Native 00bf79e3
        leave
        // Native 00bf79e4
        ret

        // TEST destination,3 selected nonzero; MOV EAX,destination / AND EAX,3 =>1..3. Targets copy3/2/1 leading bytes.
    dispatch_forward_alignment:
        cmp eax, 1
        je native_00bf7718
        cmp eax, 2
        je native_00bf7744
        // Final member of the proven domain; no invented default path.
        jmp native_00bf7768

        // Logical SHR ECX,2 followed by unsigned CMP ECX,8 / JB =>0..7.
    dispatch_forward_dwords:
        cmp ecx, 0
        je native_00bf77eb
        cmp ecx, 1
        je native_00bf77d8
        cmp ecx, 2
        je native_00bf77d0
        cmp ecx, 3
        je native_00bf77c8
        cmp ecx, 4
        je native_00bf77c0
        cmp ecx, 5
        je native_00bf77b8
        cmp ecx, 6
        je native_00bf77b0
        // Final member of the proven domain; no invented default path.
        jmp native_00bf77a8

        // EDX=count&3, or initialized3 AND adjusted count =>0..3. REP MOVSD and unrolled DWORD payload do not modify EDX.
    dispatch_forward_remainder:
        cmp edx, 0
        je native_00bf7804
        cmp edx, 1
        je native_00bf780c
        cmp edx, 2
        je native_00bf7818
        // Final member of the proven domain; no invented default path.
        jmp native_00bf782c

        // Unsigned borrow after SUB ECX,4 means original n in0..3; ECX wrapped to FFFFFFFC..FFFFFFFF.
    dispatch_forward_short:
        cmp ecx, -4
        je native_00bf7804
        cmp ecx, -3
        je native_00bf780c
        cmp ecx, -2
        je native_00bf7818
        // Final member of the proven domain; no invented default path.
        jmp native_00bf782c

        // TEST end-4 destination,3 selected nonzero; MOV EAX,EDI / AND EAX,3 =>1..3. Targets copy1/2/3 trailing bytes.
    dispatch_backward_alignment:
        cmp eax, 1
        je native_00bf78a4
        cmp eax, 2
        je native_00bf78c8
        // Final member of the proven domain; no invented default path.
        jmp native_00bf78f0

        // Logical DWORD count gated unsigned below8, then NEG ECX =>-7..0. Zero goes directly to remainder dispatch.
    dispatch_backward_dwords:
        cmp ecx, -7
        je native_00bf7944
        cmp ecx, -6
        je native_00bf794c
        cmp ecx, -5
        je native_00bf7954
        cmp ecx, -4
        je native_00bf795c
        cmp ecx, -3
        je native_00bf7964
        cmp ecx, -2
        je native_00bf796c
        cmp ecx, -1
        je native_00bf7974
        // Final member of the proven domain; no invented default path.
        jmp native_00bf7987

        // EDX original count&3, or initialized3 AND adjusted count =>0..3; payload preserves EDX.
    dispatch_backward_remainder:
        cmp edx, 0
        je native_00bf79a0
        cmp edx, 1
        je native_00bf79a8
        cmp edx, 2
        je native_00bf79b8
        // Final member of the proven domain; no invented default path.
        jmp native_00bf79cc

        // Unsigned CMP ECX,4 / JB =>0..3 without subtracting4.
    dispatch_backward_short:
        cmp ecx, 0
        je native_00bf79a0
        cmp ecx, 1
        je native_00bf79a8
        cmp ecx, 2
        je native_00bf79b8
        // Final member of the proven domain; no invented default path.
        jmp native_00bf79cc
    }
}
} // namespace bsp

#include "bsp/native_crt_memmove.hpp"
#include "bsp/native_crt_vector_copy.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT memmove requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);

// BF87E0: all 247 native payload instructions are represented. Six constant
// tables and skipped padding are evidence only; eight known-domain ladders
// replace the 16 indexed transfers without stack touches or selector changes.
__declspec(naked) void* __cdecl move_native_crt_bytes_00bf87e0(
    void*, const void*, std::uint32_t, const volatile std::uint32_t&) {
    __asm {
        // Native 00bf87e0
        push ebp
        // Native 00bf87e1
        mov ebp, esp
        // Native 00bf87e3
        push edi
        // Native 00bf87e4
        push esi
        // Native 00bf87e5
        mov esi, dword ptr [ebp + 0xc]
        // Native 00bf87e8
        mov ecx, dword ptr [ebp + 0x10]
        // Native 00bf87eb
        mov edi, dword ptr [ebp + 8]
        // Native 00bf87ee
        mov eax, ecx
        // Native 00bf87f0
        mov edx, ecx
        // Native 00bf87f2
        add eax, esi
        // Native 00bf87f4
        cmp edi, esi
        // Native 00bf87f6
        jbe native_00bf8800
        // Native 00bf87f8
        cmp edi, eax
        // Native 00bf87fa
        jb native_00bf89a4
    native_00bf8800:
        cmp ecx, 0x100
        // Native 00bf8806
        jb native_00bf8827
        // Native 00bf8808
        // The completed direction decision has made EAX dead.
        mov eax, dword ptr [ebp + 0x14]
        cmp dword ptr [eax], 0
        // Native 00bf880f
        je native_00bf8827
        // Native 00bf8811
        push edi
        // Native 00bf8812
        push esi
        // Native 00bf8813
        and edi, 0xf
        // Native 00bf8816
        and esi, 0xf
        // Native 00bf8819
        cmp edi, esi
        // Native 00bf881b
        pop esi
        // Native 00bf881c
        pop edi
        // Native 00bf881d
        jne native_00bf8827
        // Native 00bf881f
        pop esi
        // Native 00bf8820
        pop edi
        // Native 00bf8821
        pop ebp
        // Native 00bf8822
        jmp copy_native_crt_vector_00c0c82b
    native_00bf8827:
        test edi, 3
        // Native 00bf882d
        jne native_00bf8844
        // Native 00bf882f
        shr ecx, 2
        // Native 00bf8832
        and edx, 3
        // Native 00bf8835
        cmp ecx, 8
        // Native 00bf8838
        jb native_00bf8864
        // Native 00bf883a
        rep movsd
        // Native 00bf883c
        jmp dispatch_forward_remainder
    native_00bf8844:
        mov eax, edi
        // Native 00bf8846
        mov edx, 3
        // Native 00bf884b
        sub ecx, 4
        // Native 00bf884e
        jb native_00bf885c
        // Native 00bf8850
        and eax, 3
        // Native 00bf8853
        add ecx, eax
        // Native 00bf8855
        jmp dispatch_forward_alignment
    native_00bf885c:
        jmp dispatch_forward_short
    native_00bf8864:
        jmp dispatch_forward_dwords
    native_00bf8878:
        and edx, ecx
        // Native 00bf887a
        mov al, byte ptr [esi]
        // Native 00bf887c
        mov byte ptr [edi], al
        // Native 00bf887e
        mov al, byte ptr [esi + 1]
        // Native 00bf8881
        mov byte ptr [edi + 1], al
        // Native 00bf8884
        mov al, byte ptr [esi + 2]
        // Native 00bf8887
        shr ecx, 2
        // Native 00bf888a
        mov byte ptr [edi + 2], al
        // Native 00bf888d
        add esi, 3
        // Native 00bf8890
        add edi, 3
        // Native 00bf8893
        cmp ecx, 8
        // Native 00bf8896
        jb native_00bf8864
        // Native 00bf8898
        rep movsd
        // Native 00bf889a
        jmp dispatch_forward_remainder
    native_00bf88a4:
        and edx, ecx
        // Native 00bf88a6
        mov al, byte ptr [esi]
        // Native 00bf88a8
        mov byte ptr [edi], al
        // Native 00bf88aa
        mov al, byte ptr [esi + 1]
        // Native 00bf88ad
        shr ecx, 2
        // Native 00bf88b0
        mov byte ptr [edi + 1], al
        // Native 00bf88b3
        add esi, 2
        // Native 00bf88b6
        add edi, 2
        // Native 00bf88b9
        cmp ecx, 8
        // Native 00bf88bc
        jb native_00bf8864
        // Native 00bf88be
        rep movsd
        // Native 00bf88c0
        jmp dispatch_forward_remainder
    native_00bf88c8:
        and edx, ecx
        // Native 00bf88ca
        mov al, byte ptr [esi]
        // Native 00bf88cc
        mov byte ptr [edi], al
        // Native 00bf88ce
        add esi, 1
        // Native 00bf88d1
        shr ecx, 2
        // Native 00bf88d4
        add edi, 1
        // Native 00bf88d7
        cmp ecx, 8
        // Native 00bf88da
        jb native_00bf8864
        // Native 00bf88dc
        rep movsd
        // Native 00bf88de
        jmp dispatch_forward_remainder
    native_00bf8908:
        mov eax, dword ptr [esi + ecx*4 - 0x1c]
        // Native 00bf890c
        mov dword ptr [edi + ecx*4 - 0x1c], eax
    native_00bf8910:
        mov eax, dword ptr [esi + ecx*4 - 0x18]
        // Native 00bf8914
        mov dword ptr [edi + ecx*4 - 0x18], eax
    native_00bf8918:
        mov eax, dword ptr [esi + ecx*4 - 0x14]
        // Native 00bf891c
        mov dword ptr [edi + ecx*4 - 0x14], eax
    native_00bf8920:
        mov eax, dword ptr [esi + ecx*4 - 0x10]
        // Native 00bf8924
        mov dword ptr [edi + ecx*4 - 0x10], eax
    native_00bf8928:
        mov eax, dword ptr [esi + ecx*4 - 0xc]
        // Native 00bf892c
        mov dword ptr [edi + ecx*4 - 0xc], eax
    native_00bf8930:
        mov eax, dword ptr [esi + ecx*4 - 8]
        // Native 00bf8934
        mov dword ptr [edi + ecx*4 - 8], eax
    native_00bf8938:
        mov eax, dword ptr [esi + ecx*4 - 4]
        // Native 00bf893c
        mov dword ptr [edi + ecx*4 - 4], eax
        // Native 00bf8940
        lea eax, [ecx*4]
        // Native 00bf8947
        add esi, eax
        // Native 00bf8949
        add edi, eax
    native_00bf894b:
        jmp dispatch_forward_remainder
    native_00bf8964:
        mov eax, dword ptr [ebp + 8]
        // Native 00bf8967
        pop esi
        // Native 00bf8968
        pop edi
        // Native 00bf8969
        leave
        // Native 00bf896a
        ret
    native_00bf896c:
        mov al, byte ptr [esi]
        // Native 00bf896e
        mov byte ptr [edi], al
        // Native 00bf8970
        mov eax, dword ptr [ebp + 8]
        // Native 00bf8973
        pop esi
        // Native 00bf8974
        pop edi
        // Native 00bf8975
        leave
        // Native 00bf8976
        ret
    native_00bf8978:
        mov al, byte ptr [esi]
        // Native 00bf897a
        mov byte ptr [edi], al
        // Native 00bf897c
        mov al, byte ptr [esi + 1]
        // Native 00bf897f
        mov byte ptr [edi + 1], al
        // Native 00bf8982
        mov eax, dword ptr [ebp + 8]
        // Native 00bf8985
        pop esi
        // Native 00bf8986
        pop edi
        // Native 00bf8987
        leave
        // Native 00bf8988
        ret
    native_00bf898c:
        mov al, byte ptr [esi]
        // Native 00bf898e
        mov byte ptr [edi], al
        // Native 00bf8990
        mov al, byte ptr [esi + 1]
        // Native 00bf8993
        mov byte ptr [edi + 1], al
        // Native 00bf8996
        mov al, byte ptr [esi + 2]
        // Native 00bf8999
        mov byte ptr [edi + 2], al
        // Native 00bf899c
        mov eax, dword ptr [ebp + 8]
        // Native 00bf899f
        pop esi
        // Native 00bf89a0
        pop edi
        // Native 00bf89a1
        leave
        // Native 00bf89a2
        ret
    native_00bf89a4:
        lea esi, [ecx + esi - 4]
        // Native 00bf89a8
        lea edi, [ecx + edi - 4]
        // Native 00bf89ac
        test edi, 3
        // Native 00bf89b2
        jne native_00bf89d8
        // Native 00bf89b4
        shr ecx, 2
        // Native 00bf89b7
        and edx, 3
        // Native 00bf89ba
        cmp ecx, 8
        // Native 00bf89bd
        jb native_00bf89cc
        // Native 00bf89bf
        std
        // Native 00bf89c0
        rep movsd
        // Native 00bf89c2
        cld
        // Native 00bf89c3
        jmp dispatch_backward_remainder
    native_00bf89cc:
        neg ecx
        // Native 00bf89ce
        jmp dispatch_backward_dwords
    native_00bf89d8:
        mov eax, edi
        // Native 00bf89da
        mov edx, 3
        // Native 00bf89df
        cmp ecx, 4
        // Native 00bf89e2
        jb native_00bf89f0
        // Native 00bf89e4
        and eax, 3
        // Native 00bf89e7
        sub ecx, eax
        // Native 00bf89e9
        jmp dispatch_backward_alignment
    native_00bf89f0:
        jmp dispatch_backward_short
    native_00bf8a04:
        mov al, byte ptr [esi + 3]
        // Native 00bf8a07
        and edx, ecx
        // Native 00bf8a09
        mov byte ptr [edi + 3], al
        // Native 00bf8a0c
        sub esi, 1
        // Native 00bf8a0f
        shr ecx, 2
        // Native 00bf8a12
        sub edi, 1
        // Native 00bf8a15
        cmp ecx, 8
        // Native 00bf8a18
        jb native_00bf89cc
        // Native 00bf8a1a
        std
        // Native 00bf8a1b
        rep movsd
        // Native 00bf8a1d
        cld
        // Native 00bf8a1e
        jmp dispatch_backward_remainder
    native_00bf8a28:
        mov al, byte ptr [esi + 3]
        // Native 00bf8a2b
        and edx, ecx
        // Native 00bf8a2d
        mov byte ptr [edi + 3], al
        // Native 00bf8a30
        mov al, byte ptr [esi + 2]
        // Native 00bf8a33
        shr ecx, 2
        // Native 00bf8a36
        mov byte ptr [edi + 2], al
        // Native 00bf8a39
        sub esi, 2
        // Native 00bf8a3c
        sub edi, 2
        // Native 00bf8a3f
        cmp ecx, 8
        // Native 00bf8a42
        jb native_00bf89cc
        // Native 00bf8a44
        std
        // Native 00bf8a45
        rep movsd
        // Native 00bf8a47
        cld
        // Native 00bf8a48
        jmp dispatch_backward_remainder
    native_00bf8a50:
        mov al, byte ptr [esi + 3]
        // Native 00bf8a53
        and edx, ecx
        // Native 00bf8a55
        mov byte ptr [edi + 3], al
        // Native 00bf8a58
        mov al, byte ptr [esi + 2]
        // Native 00bf8a5b
        mov byte ptr [edi + 2], al
        // Native 00bf8a5e
        mov al, byte ptr [esi + 1]
        // Native 00bf8a61
        shr ecx, 2
        // Native 00bf8a64
        mov byte ptr [edi + 1], al
        // Native 00bf8a67
        sub esi, 3
        // Native 00bf8a6a
        sub edi, 3
        // Native 00bf8a6d
        cmp ecx, 8
        // Native 00bf8a70
        jb native_00bf89cc
        // Native 00bf8a76
        std
        // Native 00bf8a77
        rep movsd
        // Native 00bf8a79
        cld
        // Native 00bf8a7a
        jmp dispatch_backward_remainder
    native_00bf8aa4:
        mov eax, dword ptr [esi + ecx*4 + 0x1c]
        // Native 00bf8aa8
        mov dword ptr [edi + ecx*4 + 0x1c], eax
    native_00bf8aac:
        mov eax, dword ptr [esi + ecx*4 + 0x18]
        // Native 00bf8ab0
        mov dword ptr [edi + ecx*4 + 0x18], eax
    native_00bf8ab4:
        mov eax, dword ptr [esi + ecx*4 + 0x14]
        // Native 00bf8ab8
        mov dword ptr [edi + ecx*4 + 0x14], eax
    native_00bf8abc:
        mov eax, dword ptr [esi + ecx*4 + 0x10]
        // Native 00bf8ac0
        mov dword ptr [edi + ecx*4 + 0x10], eax
    native_00bf8ac4:
        mov eax, dword ptr [esi + ecx*4 + 0xc]
        // Native 00bf8ac8
        mov dword ptr [edi + ecx*4 + 0xc], eax
    native_00bf8acc:
        mov eax, dword ptr [esi + ecx*4 + 8]
        // Native 00bf8ad0
        mov dword ptr [edi + ecx*4 + 8], eax
    native_00bf8ad4:
        mov eax, dword ptr [esi + ecx*4 + 4]
        // Native 00bf8ad8
        mov dword ptr [edi + ecx*4 + 4], eax
        // Native 00bf8adc
        lea eax, [ecx*4]
        // Native 00bf8ae3
        add esi, eax
        // Native 00bf8ae5
        add edi, eax
    native_00bf8ae7:
        jmp dispatch_backward_remainder
    native_00bf8b00:
        mov eax, dword ptr [ebp + 8]
        // Native 00bf8b03
        pop esi
        // Native 00bf8b04
        pop edi
        // Native 00bf8b05
        leave
        // Native 00bf8b06
        ret
    native_00bf8b08:
        mov al, byte ptr [esi + 3]
        // Native 00bf8b0b
        mov byte ptr [edi + 3], al
        // Native 00bf8b0e
        mov eax, dword ptr [ebp + 8]
        // Native 00bf8b11
        pop esi
        // Native 00bf8b12
        pop edi
        // Native 00bf8b13
        leave
        // Native 00bf8b14
        ret
    native_00bf8b18:
        mov al, byte ptr [esi + 3]
        // Native 00bf8b1b
        mov byte ptr [edi + 3], al
        // Native 00bf8b1e
        mov al, byte ptr [esi + 2]
        // Native 00bf8b21
        mov byte ptr [edi + 2], al
        // Native 00bf8b24
        mov eax, dword ptr [ebp + 8]
        // Native 00bf8b27
        pop esi
        // Native 00bf8b28
        pop edi
        // Native 00bf8b29
        leave
        // Native 00bf8b2a
        ret
    native_00bf8b2c:
        mov al, byte ptr [esi + 3]
        // Native 00bf8b2f
        mov byte ptr [edi + 3], al
        // Native 00bf8b32
        mov al, byte ptr [esi + 2]
        // Native 00bf8b35
        mov byte ptr [edi + 2], al
        // Native 00bf8b38
        mov al, byte ptr [esi + 1]
        // Native 00bf8b3b
        mov byte ptr [edi + 1], al
        // Native 00bf8b3e
        mov eax, dword ptr [ebp + 8]
        // Native 00bf8b41
        pop esi
        // Native 00bf8b42
        pop edi
        // Native 00bf8b43
        leave
        // Native 00bf8b44
        ret

        // TEST destination,3 selected nonzero; MOV EAX,destination / AND EAX,3 =>1..3. Targets copy3/2/1 leading bytes.
    dispatch_forward_alignment:
        cmp eax, 1
        je native_00bf8878
        cmp eax, 2
        je native_00bf88a4
        // Final member of the proven domain; no invented default path.
        jmp native_00bf88c8

        // Logical SHR ECX,2 followed by unsigned CMP ECX,8 / JB =>0..7.
    dispatch_forward_dwords:
        cmp ecx, 0
        je native_00bf894b
        cmp ecx, 1
        je native_00bf8938
        cmp ecx, 2
        je native_00bf8930
        cmp ecx, 3
        je native_00bf8928
        cmp ecx, 4
        je native_00bf8920
        cmp ecx, 5
        je native_00bf8918
        cmp ecx, 6
        je native_00bf8910
        // Final member of the proven domain; no invented default path.
        jmp native_00bf8908

        // EDX=count&3, or initialized3 AND adjusted count =>0..3. REP MOVSD and unrolled DWORD payload do not modify EDX.
    dispatch_forward_remainder:
        cmp edx, 0
        je native_00bf8964
        cmp edx, 1
        je native_00bf896c
        cmp edx, 2
        je native_00bf8978
        // Final member of the proven domain; no invented default path.
        jmp native_00bf898c

        // Unsigned borrow after SUB ECX,4 means original n in0..3; ECX wrapped to FFFFFFFC..FFFFFFFF.
    dispatch_forward_short:
        cmp ecx, -4
        je native_00bf8964
        cmp ecx, -3
        je native_00bf896c
        cmp ecx, -2
        je native_00bf8978
        // Final member of the proven domain; no invented default path.
        jmp native_00bf898c

        // TEST end-4 destination,3 selected nonzero; MOV EAX,EDI / AND EAX,3 =>1..3. Targets copy1/2/3 trailing bytes.
    dispatch_backward_alignment:
        cmp eax, 1
        je native_00bf8a04
        cmp eax, 2
        je native_00bf8a28
        // Final member of the proven domain; no invented default path.
        jmp native_00bf8a50

        // Logical DWORD count gated unsigned below8, then NEG ECX =>-7..0. Zero goes directly to remainder dispatch.
    dispatch_backward_dwords:
        cmp ecx, -7
        je native_00bf8aa4
        cmp ecx, -6
        je native_00bf8aac
        cmp ecx, -5
        je native_00bf8ab4
        cmp ecx, -4
        je native_00bf8abc
        cmp ecx, -3
        je native_00bf8ac4
        cmp ecx, -2
        je native_00bf8acc
        cmp ecx, -1
        je native_00bf8ad4
        // Final member of the proven domain; no invented default path.
        jmp native_00bf8ae7

        // EDX original count&3, or initialized3 AND adjusted count =>0..3; payload preserves EDX.
    dispatch_backward_remainder:
        cmp edx, 0
        je native_00bf8b00
        cmp edx, 1
        je native_00bf8b08
        cmp edx, 2
        je native_00bf8b18
        // Final member of the proven domain; no invented default path.
        jmp native_00bf8b2c

        // Unsigned CMP ECX,4 / JB =>0..3 without subtracting4.
    dispatch_backward_short:
        cmp ecx, 0
        je native_00bf8b00
        cmp ecx, 1
        je native_00bf8b08
        cmp ecx, 2
        je native_00bf8b18
        // Final member of the proven domain; no invented default path.
        jmp native_00bf8b2c
    }
}
} // namespace bsp

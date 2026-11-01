#include "bsp/native_gui_icon_cropped_quad_storage.hpp"

namespace bsp {
void write_native_gui_icon_cropped_quad_storage_00ab1860(void* actual_icon,
    std::uint32_t (&actual_argument_homes)[7], NativeGuiIconCroppedQuadStorageFrame& frame,
    const volatile float& actual_one_00d7a24c,
    const volatile float& actual_vertical_scale_00e12fd0) {
    static_assert(sizeof(void*) == 4, "Raw DWORD identity interface requires Win32");
    auto* const source_homes = &actual_argument_homes[0];
    auto* const source_spills = &frame.native_spill_words[0];
    const volatile float* const source_one = &actual_one_00d7a24c;
    const volatile float* const source_vertical = &actual_vertical_scale_00e12fd0;
    std::uint32_t source_saved_ecx;
    std::uint32_t source_switch_targets[5];
    __asm {
        lea eax, quad_00ab194b
        mov source_switch_targets[0], eax
        lea eax, quad_00ab1a46
        mov source_switch_targets[4], eax
        lea eax, quad_00ab1b6f
        mov source_switch_targets[8], eax
        lea eax, quad_00ab1c96
        mov source_switch_targets[12], eax
        lea eax, quad_00ab1dc5
        mov source_switch_targets[16], eax
        mov ebx, source_spills
        mov edi, source_homes
        mov ecx, actual_icon
    quad_00ab1860:
        movss xmm1,dword ptr [ecx + 0x114]
    quad_00ab1868:
        xorps xmm0,xmm0
    quad_00ab186b:
    quad_00ab186e:
        comiss xmm0,xmm1
    quad_00ab1871:
        jbe quad_00ab1878
    quad_00ab1873:
        movaps xmm1,xmm0
    quad_00ab1876:
        jmp quad_00ab1888
    quad_00ab1878:
        mov source_saved_ecx, ecx
        mov ecx, source_one
        movss xmm2,dword ptr [ecx]
        mov ecx, source_saved_ecx
    quad_00ab1880:
        comiss xmm1,xmm2
    quad_00ab1883:
        jbe quad_00ab1888
    quad_00ab1885:
        movaps xmm1,xmm2
    quad_00ab1888:
        mov eax,dword ptr [edi + 4]
    quad_00ab188c:
        movss dword ptr [ecx + 0x114],xmm1
    quad_00ab1894:
        fld dword ptr [eax]
    quad_00ab1896:
        fstp dword ptr [edi + 4]
    quad_00ab189a:
        movss dword ptr [ebx + 0],xmm1
    quad_00ab189f:
        fld dword ptr [eax + 0x8]
    quad_00ab18a2:
        fld dword ptr [edi + 4]
    quad_00ab18a6:
        fld st(0)
    quad_00ab18a8:
        fsubp st(2),st(0)
    quad_00ab18aa:
        fld dword ptr [edi + 8]
    quad_00ab18ae:
        fmul st(0), st(2)
    quad_00ab18b0:
        fadd st(0),st(1)
    quad_00ab18b2:
        fstp dword ptr [ebx + 8]
    quad_00ab18b6:
        fld dword ptr [edi + 16]
    quad_00ab18ba:
        fmulp st(2), st(0)
    quad_00ab18bc:
        faddp st(1), st(0)
    quad_00ab18be:
        fstp dword ptr [ebx + 16]
    quad_00ab18c2:
        fld dword ptr [eax + 0xc]
    quad_00ab18c5:
        fstp dword ptr [edi + 4]
    quad_00ab18c9:
        fld dword ptr [eax + 0x4]
    quad_00ab18cc:
        fstp dword ptr [ebx + 4]
    quad_00ab18d0:
        fld dword ptr [edi + 4]
    quad_00ab18d4:
        fld st(0)
    quad_00ab18d6:
        fld dword ptr [ebx + 4]
    quad_00ab18da:
        fld st(0)
    quad_00ab18dc:
        fsubp st(2),st(0)
    quad_00ab18de:
        fld dword ptr [edi + 12]
    quad_00ab18e2:
        fmulp st(2), st(0)
    quad_00ab18e4:
        faddp st(1), st(0)
    quad_00ab18e6:
        fstp dword ptr [ebx + 12]
    quad_00ab18ea:
        fld dword ptr [eax + 0x4]
    quad_00ab18ed:
        mov eax,dword ptr [ecx + 0x110]
    quad_00ab18f3:
        cmp eax,0x4
    quad_00ab18f6:
        fstp dword ptr [edi + 4]
    quad_00ab18fa:
        fld dword ptr [edi + 4]
    quad_00ab18fe:
        fld st(0)
    quad_00ab1900:
        fsubp st(2),st(0)
    quad_00ab1902:
        fld dword ptr [edi + 20]
    quad_00ab1906:
        fmulp st(2), st(0)
    quad_00ab1908:
        faddp st(1), st(0)
    quad_00ab190a:
        fstp dword ptr [ebx + 20]
    quad_00ab190e:
        fld dword ptr [ebx + 20]
    quad_00ab1912:
        fld dword ptr [ebx + 12]
    quad_00ab1916:
        fld st(0)
    quad_00ab1918:
        fsubp st(2),st(0)
    quad_00ab191a:
        fxch st(1)
    quad_00ab191c:
        fstp dword ptr [edi + 4]
    quad_00ab1920:
        fld dword ptr [ebx + 16]
    quad_00ab1924:
        fld dword ptr [ebx + 8]
    quad_00ab1928:
        fld st(0)
    quad_00ab192a:
        fsubp st(2),st(0)
    quad_00ab192c:
        fxch st(1)
    quad_00ab192e:
        fstp dword ptr [ebx + 4]
    quad_00ab1932:
        fld dword ptr [ebx + 0]
    quad_00ab1935:
        fld1
    quad_00ab1937:
        fsubrp st(1), st(0)
    quad_00ab1939:
        fstp dword ptr [edi + 8]
    quad_00ab193d:
        ja quad_00ab1ec7
    quad_00ab1943:
    quad_00ab1944:
        jmp dword ptr source_switch_targets[eax*4]
    quad_00ab194b:
        mov eax,dword ptr [edi + 0]
    quad_00ab194f:
        fstp st(1)
    quad_00ab1951:
        mov ecx,dword ptr [eax + 0x8]
    quad_00ab1954:
        fstp st(0)
    quad_00ab1956:
        add ecx,dword ptr [eax + 0x10]
    quad_00ab1959:
        movss xmm2,dword ptr [ebx + 12]
    quad_00ab195f:
        movss dword ptr [ecx],xmm0
    quad_00ab1963:
        movss dword ptr [ecx + 0x4],xmm0
    quad_00ab1968:
        movss dword ptr [ecx + 0x8],xmm0
    quad_00ab196d:
        mov ecx,dword ptr [eax + 0x8]
    quad_00ab1970:
        add ecx,dword ptr [eax + 0x28]
    quad_00ab1973:
        movss xmm1,dword ptr [ebx + 8]
    quad_00ab1979:
        movss dword ptr [ecx + 0x4],xmm2
    quad_00ab197e:
        movss dword ptr [ecx],xmm1
    quad_00ab1982:
        mov ecx,dword ptr [eax + 0xc]
    quad_00ab1985:
        add ecx,dword ptr [eax + 0x8]
    quad_00ab1988:
        mov edx,dword ptr [edi + 24]
    quad_00ab198c:
        add ecx,dword ptr [eax + 0x10]
    quad_00ab198f:
        movss xmm3,dword ptr [edx]
    quad_00ab1993:
        movss dword ptr [ecx],xmm3
    quad_00ab1997:
        movss dword ptr [ecx + 0x4],xmm0
    quad_00ab199c:
        movss dword ptr [ecx + 0x8],xmm0
    quad_00ab19a1:
        mov ecx,dword ptr [eax + 0xc]
    quad_00ab19a4:
        add ecx,dword ptr [eax + 0x8]
    quad_00ab19a7:
        movss xmm3,dword ptr [ebx + 16]
    quad_00ab19ad:
        add ecx,dword ptr [eax + 0x28]
    quad_00ab19b0:
    quad_00ab19b1:
        movss dword ptr [ecx + 0x4],xmm2
    quad_00ab19b6:
        movss dword ptr [ecx],xmm3
    quad_00ab19ba:
        fld dword ptr [edx + 0x4]
    quad_00ab19bd:
        mov ecx,dword ptr [eax + 0xc]
    quad_00ab19c0:
        mov source_saved_ecx, ecx
        mov ecx, source_vertical
        fld dword ptr [ecx]
        mov ecx, source_saved_ecx
    quad_00ab19c6:
        movss xmm2,dword ptr [edx]
    quad_00ab19ca:
        fld st(0)
    quad_00ab19cc:
        lea ecx,[ecx + ecx*0x2]
    quad_00ab19cf:
        fmulp st(2), st(0)
    quad_00ab19d1:
        add ecx,dword ptr [eax + 0x8]
    quad_00ab19d4:
        fxch st(1)
    quad_00ab19d6:
        add ecx,dword ptr [eax + 0x10]
    quad_00ab19d9:
        fstp dword ptr [edi + 12]
    quad_00ab19dd:
        fld dword ptr [edi + 12]
    quad_00ab19e1:
        fstp dword ptr [ecx + 0x4]
    quad_00ab19e4:
        movss dword ptr [ecx],xmm2
    quad_00ab19e8:
        movss xmm2,dword ptr [ebx + 20]
    quad_00ab19ee:
        movss dword ptr [ecx + 0x8],xmm0
    quad_00ab19f3:
        mov ecx,dword ptr [eax + 0xc]
    quad_00ab19f6:
        lea ecx,[ecx + ecx*0x2]
    quad_00ab19f9:
        add ecx,dword ptr [eax + 0x8]
    quad_00ab19fc:
        add ecx,dword ptr [eax + 0x28]
    quad_00ab19ff:
        movss dword ptr [ecx],xmm3
    quad_00ab1a03:
        movss dword ptr [ecx + 0x4],xmm2
    quad_00ab1a08:
        fmul dword ptr [edx + 0x4]
    quad_00ab1a0b:
        mov edx,dword ptr [eax + 0x8]
    quad_00ab1a0e:
        mov ecx,dword ptr [eax + 0xc]
    quad_00ab1a11:
        lea ecx,[edx + ecx*0x2]
    quad_00ab1a14:
        fstp dword ptr [edi + 12]
    quad_00ab1a18:
        add ecx,dword ptr [eax + 0x10]
    quad_00ab1a1b:
        fld dword ptr [edi + 12]
    quad_00ab1a1f:
        fstp dword ptr [ecx + 0x4]
    quad_00ab1a22:
        movss dword ptr [ecx],xmm0
    quad_00ab1a26:
        movss dword ptr [ecx + 0x8],xmm0
    quad_00ab1a2b:
        mov ecx,dword ptr [eax + 0xc]
    quad_00ab1a2e:
        mov edx,dword ptr [eax + 0x8]
    quad_00ab1a31:
        lea ecx,[edx + ecx*0x2]
    quad_00ab1a34:
        add ecx,dword ptr [eax + 0x28]
    quad_00ab1a37:
        movss dword ptr [ecx],xmm1
    quad_00ab1a3b:
        movss dword ptr [ecx + 0x4],xmm2
    quad_00ab1a40:
    quad_00ab1a43:
        jmp quad_source_return
    quad_00ab1a46:
        mov eax,dword ptr [edi + 0]
    quad_00ab1a4a:
        fstp st(0)
    quad_00ab1a4c:
        mov edx,dword ptr [eax + 0x8]
    quad_00ab1a4f:
        add edx,dword ptr [eax + 0x10]
    quad_00ab1a52:
        movss xmm2,dword ptr [ebx + 12]
    quad_00ab1a58:
        movss dword ptr [edx],xmm0
    quad_00ab1a5c:
        movss dword ptr [edx + 0x4],xmm0
    quad_00ab1a61:
        movss dword ptr [edx + 0x8],xmm0
    quad_00ab1a66:
        mov edx,dword ptr [eax + 0x8]
    quad_00ab1a69:
        add edx,dword ptr [eax + 0x28]
    quad_00ab1a6c:
        movss xmm1,dword ptr [ebx + 8]
    quad_00ab1a72:
        movss dword ptr [edx],xmm1
    quad_00ab1a76:
        movss dword ptr [edx + 0x4],xmm2
    quad_00ab1a7b:
        mov edx,dword ptr [eax + 0xc]
    quad_00ab1a7e:
        add edx,dword ptr [eax + 0x8]
    quad_00ab1a81:
        mov esi,dword ptr [edi + 24]
    quad_00ab1a85:
        add edx,dword ptr [eax + 0x10]
    quad_00ab1a88:
        movss xmm3,dword ptr [esi]
    quad_00ab1a8c:
        movss dword ptr [edx],xmm3
    quad_00ab1a90:
        movss dword ptr [edx + 0x4],xmm0
    quad_00ab1a95:
        movss dword ptr [edx + 0x8],xmm0
    quad_00ab1a9a:
        mov edx,dword ptr [eax + 0xc]
    quad_00ab1a9d:
        add edx,dword ptr [eax + 0x8]
    quad_00ab1aa0:
        movss xmm3,dword ptr [ebx + 16]
    quad_00ab1aa6:
        add edx,dword ptr [eax + 0x28]
    quad_00ab1aa9:
        movss dword ptr [edx],xmm3
    quad_00ab1aad:
        movss dword ptr [edx + 0x4],xmm2
    quad_00ab1ab2:
        fld dword ptr [esi + 0x4]
    quad_00ab1ab5:
        mov edx,dword ptr [eax + 0xc]
    quad_00ab1ab8:
        mov source_saved_ecx, ecx
        mov ecx, source_vertical
        fld dword ptr [ecx]
        mov ecx, source_saved_ecx
    quad_00ab1abe:
        movss xmm2,dword ptr [esi]
    quad_00ab1ac2:
        fld st(0)
    quad_00ab1ac4:
        fmulp st(2), st(0)
    quad_00ab1ac6:
        lea edx,[edx + edx*0x2]
    quad_00ab1ac9:
        add edx,dword ptr [eax + 0x8]
    quad_00ab1acc:
        fld dword ptr [ecx + 0x114]
    quad_00ab1ad2:
        add edx,dword ptr [eax + 0x10]
    quad_00ab1ad5:
        fmulp st(2), st(0)
    quad_00ab1ad7:
        fxch st(1)
    quad_00ab1ad9:
        movss dword ptr [edx],xmm2
    quad_00ab1add:
        fstp dword ptr [edi + 12]
    quad_00ab1ae1:
        movss dword ptr [edx + 0x8],xmm0
    quad_00ab1ae6:
        fld dword ptr [edi + 12]
    quad_00ab1aea:
        fstp dword ptr [edx + 0x4]
    quad_00ab1aed:
        mov edx,dword ptr [eax + 0xc]
    quad_00ab1af0:
        fld dword ptr [edi + 4]
    quad_00ab1af4:
        lea edx,[edx + edx*0x2]
    quad_00ab1af7:
        add edx,dword ptr [eax + 0x8]
    quad_00ab1afa:
        fld st(0)
    quad_00ab1afc:
        fmul dword ptr [ecx + 0x114]
    quad_00ab1b02:
        add edx,dword ptr [eax + 0x28]
    quad_00ab1b05:
        fadd st(0),st(3)
    quad_00ab1b07:
        movss dword ptr [edx],xmm3
    quad_00ab1b0b:
        fstp dword ptr [edi + 12]
    quad_00ab1b0f:
        fld dword ptr [edi + 12]
    quad_00ab1b13:
        fstp dword ptr [edx + 0x4]
    quad_00ab1b16:
        mov edx,dword ptr [eax + 0xc]
    quad_00ab1b19:
        fld dword ptr [esi + 0x4]
    quad_00ab1b1c:
        mov esi,dword ptr [eax + 0x8]
    quad_00ab1b1f:
        fmulp st(2), st(0)
    quad_00ab1b21:
        lea edx,[esi + edx*0x2]
    quad_00ab1b24:
        fld dword ptr [ecx + 0x114]
    quad_00ab1b2a:
        add edx,dword ptr [eax + 0x10]
    quad_00ab1b2d:
    quad_00ab1b2e:
        fmulp st(2), st(0)
    quad_00ab1b30:
        movss dword ptr [edx],xmm0
    quad_00ab1b34:
        fxch st(1)
    quad_00ab1b36:
        movss dword ptr [edx + 0x8],xmm0
    quad_00ab1b3b:
        fstp dword ptr [edi + 12]
    quad_00ab1b3f:
        fld dword ptr [edi + 12]
    quad_00ab1b43:
        fstp dword ptr [edx + 0x4]
    quad_00ab1b46:
        mov edx,dword ptr [eax + 0x8]
    quad_00ab1b49:
        fmul dword ptr [ecx + 0x114]
    quad_00ab1b4f:
        mov ecx,dword ptr [eax + 0xc]
    quad_00ab1b52:
        lea ecx,[edx + ecx*0x2]
    quad_00ab1b55:
        add ecx,dword ptr [eax + 0x28]
    quad_00ab1b58:
        faddp st(1), st(0)
    quad_00ab1b5a:
        movss dword ptr [ecx],xmm1
    quad_00ab1b5e:
        fstp dword ptr [edi + 12]
    quad_00ab1b62:
        fld dword ptr [edi + 12]
    quad_00ab1b66:
        fstp dword ptr [ecx + 0x4]
    quad_00ab1b69:
    quad_00ab1b6c:
        jmp quad_source_return
    quad_00ab1b6f:
        mov eax,dword ptr [edi + 0]
    quad_00ab1b73:
        fstp st(0)
    quad_00ab1b75:
        mov edx,dword ptr [edi + 24]
    quad_00ab1b79:
        fld dword ptr [edx + 0x4]
    quad_00ab1b7c:
        mov ecx,dword ptr [eax + 0x8]
    quad_00ab1b7f:
        add ecx,dword ptr [eax + 0x10]
    quad_00ab1b82:
        mov source_saved_ecx, ecx
        mov ecx, source_vertical
        fld dword ptr [ecx]
        mov ecx, source_saved_ecx
    quad_00ab1b88:
        fld st(0)
    quad_00ab1b8a:
        movss xmm2,dword ptr [ebx + 8]
    quad_00ab1b90:
        fmulp st(2), st(0)
    quad_00ab1b92:
        movss dword ptr [ecx],xmm0
    quad_00ab1b96:
        fld dword ptr [edi + 8]
    quad_00ab1b9a:
        movss dword ptr [ecx + 0x8],xmm0
    quad_00ab1b9f:
        fld st(0)
    quad_00ab1ba1:
        fmulp st(3), st(0)
    quad_00ab1ba3:
        fxch st(2)
    quad_00ab1ba5:
        fstp dword ptr [edi + 12]
    quad_00ab1ba9:
        fld dword ptr [edi + 12]
    quad_00ab1bad:
        fstp dword ptr [ecx + 0x4]
    quad_00ab1bb0:
        mov ecx,dword ptr [eax + 0x8]
    quad_00ab1bb3:
        add ecx,dword ptr [eax + 0x28]
    quad_00ab1bb6:
        fld st(1)
    quad_00ab1bb8:
        fmul dword ptr [edi + 4]
    quad_00ab1bbc:
        movss dword ptr [ecx],xmm2
    quad_00ab1bc0:
        faddp st(3),st(0)
    quad_00ab1bc2:
        fxch st(2)
    quad_00ab1bc4:
        fstp dword ptr [edi + 24]
    quad_00ab1bc8:
        movss xmm1,dword ptr [edi + 24]
    quad_00ab1bce:
        movss dword ptr [ecx + 0x4],xmm1
    quad_00ab1bd3:
        fld dword ptr [edx + 0x4]
    quad_00ab1bd6:
        mov ecx,dword ptr [eax + 0xc]
    quad_00ab1bd9:
        fmul st(0), st(2)
    quad_00ab1bdb:
        add ecx,dword ptr [eax + 0x8]
    quad_00ab1bde:
        movss xmm3,dword ptr [edx]
    quad_00ab1be2:
        add ecx,dword ptr [eax + 0x10]
    quad_00ab1be5:
        fmulp st(1), st(0)
    quad_00ab1be7:
        movss dword ptr [ecx],xmm3
    quad_00ab1beb:
        movss dword ptr [ecx + 0x8],xmm0
    quad_00ab1bf0:
        fstp dword ptr [edi + 12]
    quad_00ab1bf4:
        fld dword ptr [edi + 12]
    quad_00ab1bf8:
        movss xmm3,dword ptr [ebx + 16]
    quad_00ab1bfe:
        fstp dword ptr [ecx + 0x4]
    quad_00ab1c01:
        mov ecx,dword ptr [eax + 0xc]
    quad_00ab1c04:
        add ecx,dword ptr [eax + 0x8]
    quad_00ab1c07:
        add ecx,dword ptr [eax + 0x28]
    quad_00ab1c0a:
        movss dword ptr [ecx + 0x4],xmm1
    quad_00ab1c0f:
        movss dword ptr [ecx],xmm3
    quad_00ab1c13:
        fld dword ptr [edx + 0x4]
    quad_00ab1c16:
        mov ecx,dword ptr [eax + 0xc]
    quad_00ab1c19:
        fmul st(0), st(1)
    quad_00ab1c1b:
        movss xmm1,dword ptr [edx]
    quad_00ab1c1f:
        lea ecx,[ecx + ecx*0x2]
    quad_00ab1c22:
        add ecx,dword ptr [eax + 0x8]
    quad_00ab1c25:
        fstp dword ptr [edi + 12]
    quad_00ab1c29:
        fld dword ptr [edi + 12]
    quad_00ab1c2d:
        add ecx,dword ptr [eax + 0x10]
    quad_00ab1c30:
        fstp dword ptr [ecx + 0x4]
    quad_00ab1c33:
        movss dword ptr [ecx],xmm1
    quad_00ab1c37:
        movss xmm1,dword ptr [ebx + 20]
    quad_00ab1c3d:
        movss dword ptr [ecx + 0x8],xmm0
    quad_00ab1c42:
        mov ecx,dword ptr [eax + 0xc]
    quad_00ab1c45:
        lea ecx,[ecx + ecx*0x2]
    quad_00ab1c48:
        add ecx,dword ptr [eax + 0x8]
    quad_00ab1c4b:
        add ecx,dword ptr [eax + 0x28]
    quad_00ab1c4e:
        movss dword ptr [ecx],xmm3
    quad_00ab1c52:
        movss dword ptr [ecx + 0x4],xmm1
    quad_00ab1c57:
        fmul dword ptr [edx + 0x4]
    quad_00ab1c5a:
        mov edx,dword ptr [eax + 0x8]
    quad_00ab1c5d:
        mov ecx,dword ptr [eax + 0xc]
    quad_00ab1c60:
        lea ecx,[edx + ecx*0x2]
    quad_00ab1c63:
        fstp dword ptr [edi + 12]
    quad_00ab1c67:
        add ecx,dword ptr [eax + 0x10]
    quad_00ab1c6a:
        fld dword ptr [edi + 12]
    quad_00ab1c6e:
        fstp dword ptr [ecx + 0x4]
    quad_00ab1c71:
        movss dword ptr [ecx],xmm0
    quad_00ab1c75:
        movss dword ptr [ecx + 0x8],xmm0
    quad_00ab1c7a:
        mov ecx,dword ptr [eax + 0xc]
    quad_00ab1c7d:
        mov edx,dword ptr [eax + 0x8]
    quad_00ab1c80:
        lea ecx,[edx + ecx*0x2]
    quad_00ab1c83:
        add ecx,dword ptr [eax + 0x28]
    quad_00ab1c86:
        movss dword ptr [ecx],xmm2
    quad_00ab1c8a:
        movss dword ptr [ecx + 0x4],xmm1
    quad_00ab1c8f:
    quad_00ab1c90:
    quad_00ab1c93:
        jmp quad_source_return
    quad_00ab1c96:
        mov eax,dword ptr [edi + 0]
    quad_00ab1c9a:
        fstp st(1)
    quad_00ab1c9c:
        mov edx,dword ptr [eax + 0x8]
    quad_00ab1c9f:
        add edx,dword ptr [eax + 0x10]
    quad_00ab1ca2:
        mov esi,dword ptr [edi + 24]
    quad_00ab1ca6:
        movss dword ptr [edx],xmm0
    quad_00ab1caa:
        movss dword ptr [edx + 0x4],xmm0
    quad_00ab1caf:
        movss dword ptr [edx + 0x8],xmm0
    quad_00ab1cb4:
        mov edx,dword ptr [eax + 0x8]
    quad_00ab1cb7:
        add edx,dword ptr [eax + 0x28]
    quad_00ab1cba:
        movss xmm2,dword ptr [ebx + 12]
    quad_00ab1cc0:
        movss dword ptr [edx + 0x4],xmm2
    quad_00ab1cc5:
        movss xmm1,dword ptr [ebx + 8]
    quad_00ab1ccb:
        movss dword ptr [edx],xmm1
    quad_00ab1ccf:
        fld dword ptr [esi]
    quad_00ab1cd1:
        fmul dword ptr [ecx + 0x114]
    quad_00ab1cd7:
        mov edx,dword ptr [eax + 0xc]
    quad_00ab1cda:
        add edx,dword ptr [eax + 0x8]
    quad_00ab1cdd:
        add edx,dword ptr [eax + 0x10]
    quad_00ab1ce0:
        fstp dword ptr [edi + 8]
    quad_00ab1ce4:
        fld dword ptr [edi + 8]
    quad_00ab1ce8:
        fstp dword ptr [edx]
    quad_00ab1cea:
        movss dword ptr [edx + 0x4],xmm0
    quad_00ab1cef:
        fld dword ptr [ebx + 4]
    quad_00ab1cf3:
        movss dword ptr [edx + 0x8],xmm0
    quad_00ab1cf8:
        mov edx,dword ptr [eax + 0xc]
    quad_00ab1cfb:
        fld st(0)
    quad_00ab1cfd:
        fmul dword ptr [ecx + 0x114]
    quad_00ab1d03:
        add edx,dword ptr [eax + 0x8]
    quad_00ab1d06:
        add edx,dword ptr [eax + 0x28]
    quad_00ab1d09:
        fadd st(0),st(2)
    quad_00ab1d0b:
        movss dword ptr [edx + 0x4],xmm2
    quad_00ab1d10:
        movss xmm2,dword ptr [ebx + 20]
    quad_00ab1d16:
        fstp dword ptr [edi + 8]
    quad_00ab1d1a:
        fld dword ptr [edi + 8]
    quad_00ab1d1e:
        fstp dword ptr [edx]
    quad_00ab1d20:
        mov edx,dword ptr [eax + 0xc]
    quad_00ab1d23:
        fld dword ptr [esi]
    quad_00ab1d25:
        lea edx,[edx + edx*0x2]
    quad_00ab1d28:
        fmul dword ptr [ecx + 0x114]
    quad_00ab1d2e:
        add edx,dword ptr [eax + 0x8]
    quad_00ab1d31:
        add edx,dword ptr [eax + 0x10]
    quad_00ab1d34:
        fstp dword ptr [edi + 8]
    quad_00ab1d38:
        fld dword ptr [esi + 0x4]
    quad_00ab1d3b:
        movss dword ptr [edx + 0x8],xmm0
    quad_00ab1d40:
        mov source_saved_ecx, ecx
        mov ecx, source_vertical
        fld dword ptr [ecx]
        mov ecx, source_saved_ecx
    quad_00ab1d46:
        fld st(0)
    quad_00ab1d48:
        fmulp st(2), st(0)
    quad_00ab1d4a:
        fxch st(1)
    quad_00ab1d4c:
        fstp dword ptr [edi + 12]
    quad_00ab1d50:
        fld dword ptr [edi + 8]
    quad_00ab1d54:
        fstp dword ptr [edx]
    quad_00ab1d56:
        fld dword ptr [edi + 12]
    quad_00ab1d5a:
        fstp dword ptr [edx + 0x4]
    quad_00ab1d5d:
        fld dword ptr [ecx + 0x114]
    quad_00ab1d63:
        mov ecx,dword ptr [eax + 0xc]
    quad_00ab1d66:
        fmulp st(2), st(0)
    quad_00ab1d68:
        lea ecx,[ecx + ecx*0x2]
    quad_00ab1d6b:
        add ecx,dword ptr [eax + 0x8]
    quad_00ab1d6e:
        fxch st(1)
    quad_00ab1d70:
        add ecx,dword ptr [eax + 0x28]
    quad_00ab1d73:
        faddp st(2),st(0)
    quad_00ab1d75:
        fxch st(1)
    quad_00ab1d77:
        movss dword ptr [ecx + 0x4],xmm2
    quad_00ab1d7c:
        fstp dword ptr [edi + 8]
    quad_00ab1d80:
        fld dword ptr [edi + 8]
    quad_00ab1d84:
        fstp dword ptr [ecx]
    quad_00ab1d86:
        mov ecx,dword ptr [eax + 0xc]
    quad_00ab1d89:
        mov edx,dword ptr [eax + 0x8]
    quad_00ab1d8c:
        fmul dword ptr [esi + 0x4]
    quad_00ab1d8f:
        lea ecx,[edx + ecx*0x2]
    quad_00ab1d92:
        add ecx,dword ptr [eax + 0x10]
    quad_00ab1d95:
        fstp dword ptr [edi + 12]
    quad_00ab1d99:
        movss dword ptr [ecx],xmm0
    quad_00ab1d9d:
        fld dword ptr [edi + 12]
    quad_00ab1da1:
    quad_00ab1da2:
        fstp dword ptr [ecx + 0x4]
    quad_00ab1da5:
        movss dword ptr [ecx + 0x8],xmm0
    quad_00ab1daa:
        mov ecx,dword ptr [eax + 0xc]
    quad_00ab1dad:
        mov edx,dword ptr [eax + 0x8]
    quad_00ab1db0:
        lea ecx,[edx + ecx*0x2]
    quad_00ab1db3:
        add ecx,dword ptr [eax + 0x28]
    quad_00ab1db6:
        movss dword ptr [ecx],xmm1
    quad_00ab1dba:
        movss dword ptr [ecx + 0x4],xmm2
    quad_00ab1dbf:
    quad_00ab1dc2:
        jmp quad_source_return
    quad_00ab1dc5:
        mov eax,dword ptr [edi + 0]
    quad_00ab1dc9:
        fstp st(1)
    quad_00ab1dcb:
        mov edx,dword ptr [edi + 24]
    quad_00ab1dcf:
        fld dword ptr [edx]
    quad_00ab1dd1:
        mov ecx,dword ptr [eax + 0x8]
    quad_00ab1dd4:
        add ecx,dword ptr [eax + 0x10]
    quad_00ab1dd7:
        fld dword ptr [edi + 8]
    quad_00ab1ddb:
        fld st(0)
    quad_00ab1ddd:
        movss xmm2,dword ptr [ebx + 12]
    quad_00ab1de3:
        fmulp st(2), st(0)
    quad_00ab1de5:
        movss dword ptr [ecx + 0x4],xmm0
    quad_00ab1dea:
        movss dword ptr [ecx + 0x8],xmm0
    quad_00ab1def:
        fxch st(1)
    quad_00ab1df1:
        fstp dword ptr [edi + 8]
    quad_00ab1df5:
        fld dword ptr [edi + 8]
    quad_00ab1df9:
        fstp dword ptr [ecx]
    quad_00ab1dfb:
        mov ecx,dword ptr [eax + 0x8]
    quad_00ab1dfe:
        add ecx,dword ptr [eax + 0x28]
    quad_00ab1e01:
        fld st(0)
    quad_00ab1e03:
        fmul dword ptr [ebx + 4]
    quad_00ab1e07:
        movss dword ptr [ecx + 0x4],xmm2
    quad_00ab1e0c:
        faddp st(2),st(0)
    quad_00ab1e0e:
        fxch st(1)
    quad_00ab1e10:
        fstp dword ptr [edi + 24]
    quad_00ab1e14:
        movss xmm1,dword ptr [edi + 24]
    quad_00ab1e1a:
        movss dword ptr [ecx],xmm1
    quad_00ab1e1e:
        mov ecx,dword ptr [eax + 0xc]
    quad_00ab1e21:
        add ecx,dword ptr [eax + 0x8]
    quad_00ab1e24:
        movss xmm3,dword ptr [edx]
    quad_00ab1e28:
        add ecx,dword ptr [eax + 0x10]
    quad_00ab1e2b:
        movss dword ptr [ecx],xmm3
    quad_00ab1e2f:
        movss dword ptr [ecx + 0x4],xmm0
    quad_00ab1e34:
        movss dword ptr [ecx + 0x8],xmm0
    quad_00ab1e39:
        mov ecx,dword ptr [eax + 0xc]
    quad_00ab1e3c:
        add ecx,dword ptr [eax + 0x8]
    quad_00ab1e3f:
        movss xmm3,dword ptr [ebx + 16]
    quad_00ab1e45:
        add ecx,dword ptr [eax + 0x28]
    quad_00ab1e48:
        movss dword ptr [ecx + 0x4],xmm2
    quad_00ab1e4d:
        movss dword ptr [ecx],xmm3
    quad_00ab1e51:
        fld dword ptr [edx + 0x4]
    quad_00ab1e54:
        mov ecx,dword ptr [eax + 0xc]
    quad_00ab1e57:
        mov source_saved_ecx, ecx
        mov ecx, source_vertical
        fld dword ptr [ecx]
        mov ecx, source_saved_ecx
    quad_00ab1e5d:
        movss xmm2,dword ptr [edx]
    quad_00ab1e61:
        fld st(0)
    quad_00ab1e63:
        fmulp st(2), st(0)
    quad_00ab1e65:
        lea ecx,[ecx + ecx*0x2]
    quad_00ab1e68:
        add ecx,dword ptr [eax + 0x8]
    quad_00ab1e6b:
        fxch st(1)
    quad_00ab1e6d:
        add ecx,dword ptr [eax + 0x10]
    quad_00ab1e70:
        fstp dword ptr [edi + 12]
    quad_00ab1e74:
        fld dword ptr [edi + 12]
    quad_00ab1e78:
        fstp dword ptr [ecx + 0x4]
    quad_00ab1e7b:
        movss dword ptr [ecx],xmm2
    quad_00ab1e7f:
        movss xmm2,dword ptr [ebx + 20]
    quad_00ab1e85:
        movss dword ptr [ecx + 0x8],xmm0
    quad_00ab1e8a:
        mov ecx,dword ptr [eax + 0xc]
    quad_00ab1e8d:
        lea ecx,[ecx + ecx*0x2]
    quad_00ab1e90:
        add ecx,dword ptr [eax + 0x8]
    quad_00ab1e93:
        add ecx,dword ptr [eax + 0x28]
    quad_00ab1e96:
        movss dword ptr [ecx],xmm3
    quad_00ab1e9a:
        movss dword ptr [ecx + 0x4],xmm2
    quad_00ab1e9f:
        fld dword ptr [edx]
    quad_00ab1ea1:
        mov ecx,dword ptr [eax + 0xc]
    quad_00ab1ea4:
        fmulp st(2), st(0)
    quad_00ab1ea6:
        fxch st(1)
    quad_00ab1ea8:
        fstp dword ptr [edi + 8]
    quad_00ab1eac:
        fmul dword ptr [edx + 0x4]
    quad_00ab1eaf:
        mov edx,dword ptr [eax + 0x8]
    quad_00ab1eb2:
        lea ecx,[edx + ecx*0x2]
    quad_00ab1eb5:
        add ecx,dword ptr [eax + 0x10]
    quad_00ab1eb8:
        fstp dword ptr [edi + 12]
    quad_00ab1ebc:
        fld dword ptr [edi + 8]
    quad_00ab1ec0:
        fstp dword ptr [ecx]
    quad_00ab1ec2:
        jmp quad_00ab1d9d
    quad_00ab1ec7:
        fstp st(1)
    quad_00ab1ec9:
        fstp st(0)
    quad_00ab1ecb:
    quad_00ab1ece:
        jmp quad_source_return
    quad_source_return:
    }
}
} // namespace bsp

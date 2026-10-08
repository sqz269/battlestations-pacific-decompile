#include "bsp/native_parent_header_clear.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {

// Whole Original SHA256 d2bcd01aa9aa3c30eae420c969b256957f769149e7e8bc59be5029d2e473c9d3.
// Keep all66 bytes outside the sole CALL operand[56,60) literal, including
// every relink branch. Actual node base is already on the CDECL free stack.
// Bind the unchanged production canonical free directly; no provider shim.
__declspec(naked) void __fastcall clear_native_parent_header_004bf8e0(
    void*, std::uint32_t) noexcept {
    __asm {
        _emit 0x56
        _emit 0x8b
        _emit 0xf1
        _emit 0x83
        _emit 0x3e
        _emit 0x00
        _emit 0x74
        _emit 0x3c
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x8b
        _emit 0x08
        _emit 0x85
        _emit 0xc9
        _emit 0x74
        _emit 0x08
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        _emit 0x89
        _emit 0x51
        _emit 0x04
        _emit 0xeb
        _emit 0x06
        _emit 0x8b
        _emit 0x48
        _emit 0x04
        _emit 0x89
        _emit 0x4e
        _emit 0x04
        _emit 0x83
        _emit 0x78
        _emit 0x04
        _emit 0x00
        _emit 0x74
        _emit 0x09
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        _emit 0x8b
        _emit 0x08
        _emit 0x89
        _emit 0x0a
        _emit 0xeb
        _emit 0x05
        _emit 0x8b
        _emit 0x10
        _emit 0x89
        _emit 0x56
        _emit 0x08
        _emit 0x83
        _emit 0x06
        _emit 0xff
        _emit 0x50
        call singleton_lifetime_free
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x83
        _emit 0x3e
        _emit 0x00
        _emit 0x75
        _emit 0xc4
        _emit 0x5e
        _emit 0xc3
    }
}

} // namespace bsp

#include "bsp/native_unit_list_remove.hpp"
#include "bsp/native_unit_list_erase.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4, "The raw native list contract is Win32.");

// Whole original SHA256:
// 021c3de90775c6eba0451ba26f707b87d2c72cc853246f4c0b784c2a411303db.
// Preserve all 42 bytes outside the sole genuine CALL operand [36,40).
// Bind the actual accepted erase directly, with ECX root and one stacked node.
// Literal NOP/branch/store/RET4 encodings preserve complete physical behavior.
__declspec(naked) void* __fastcall remove_native_unit_list_payload_004845a0(
    void*, void*, void*) noexcept {
    __asm {
        // Root head, initial TEST, saved ESI and original stacked payload.
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x56
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x74
        _emit 0x1c
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
        // First payload match; actual next chain; absent return with RET4.
        _emit 0x39
        _emit 0x70
        _emit 0x08
        _emit 0x74
        _emit 0x0d
        _emit 0x8b
        _emit 0x40
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0xf4
        _emit 0x8b
        _emit 0xc6
        _emit 0x5e
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // Exact found actual node; accepted erase consumes its one DWORD.
        _emit 0x50
        call erase_native_unit_list_004837d0
        // Return borrowed payload after free without reading the freed node.
        _emit 0x8b
        _emit 0xc6
        _emit 0x5e
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}

}  // namespace bsp

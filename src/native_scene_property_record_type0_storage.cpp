#include "bsp/native_scene_property_record_type0_storage.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4 && native_scene_property_record_type0_storage_bytes == 56,
    "The original partial storage and physical ABI are Win32.");

// Original whole [008EF140,008EF16D), SHA256
// 55f7af3bf7ae1662a71c0e72fa4ff5e411b43754989762c7b025a63a3e5f814e.
// Exact encodings retain the complete body and XOR's 33 C9 form. No CALL,
// relocations, synthetic globals, Source vft or class/lifetime provider exists.
__declspec(naked) void* __fastcall initialize_native_scene_property_record_type0_storage_008ef140(
    void*, std::uint32_t, std::uint32_t) noexcept {
    __asm {
        // MOV EDX,[ESP+4]; MOV EAX,ECX; XOR ECX,ECX
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x8b
        _emit 0xc1
        _emit 0x33
        _emit 0xc9
        // MOV [EAX],00CE89D4; MOV [EAX+4],ECX; MOV [EAX+C],EDX
        _emit 0xc7
        _emit 0x00
        _emit 0xd4
        _emit 0x89
        _emit 0xce
        _emit 0x00
        _emit 0x89
        _emit 0x48
        _emit 0x04
        _emit 0x89
        _emit 0x50
        _emit 0x0c
        // Four zero DWORDs at +18,+1C,+20,+24
        _emit 0x89
        _emit 0x48
        _emit 0x18
        _emit 0x89
        _emit 0x48
        _emit 0x1c
        _emit 0x89
        _emit 0x48
        _emit 0x20
        _emit 0x89
        _emit 0x48
        _emit 0x24
        // Zero owner +30 and ordinal +34; byte1 at +2C; RET4
        _emit 0x89
        _emit 0x48
        _emit 0x30
        _emit 0x89
        _emit 0x48
        _emit 0x34
        _emit 0xc6
        _emit 0x40
        _emit 0x2c
        _emit 0x01
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}

} // namespace bsp

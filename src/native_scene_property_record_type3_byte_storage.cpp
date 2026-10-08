#include "bsp/native_scene_property_record_type3_byte_storage.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4 && native_scene_property_record_type3_byte_storage_bytes == 56,
    "The original partial byte storage and physical ABI are Win32.");

// Original whole [008EF1F0,008EF221), SHA256
// 77c57bbbe4e8f241db03c9753b6655903eb4e6028d32b1c6b256d083ce32e5ab.
// Exact whole encodings preserve byte-first store order, 33 C9 XOR, untouched
// EDX, ignored high argument bytes and all 22 untouched storage bytes.
// Zero CALLs/relocations; no synthetic globals, Source vft or lifetime provider.
__declspec(naked) void* __fastcall initialize_native_scene_property_record_type3_byte_storage_008ef1f0(
    void*, std::uint32_t, std::uint8_t) noexcept {
    __asm {
        // MOV EAX,ECX; MOV CL,[ESP+4]; MOV byte[EAX+C],CL; XOR ECX,ECX
        _emit 0x8b
        _emit 0xc1
        _emit 0x8a
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x88
        _emit 0x48
        _emit 0x0c
        _emit 0x33
        _emit 0xc9
        // Literal phase CE89D4; DWORD tag3
        _emit 0xc7
        _emit 0x00
        _emit 0xd4
        _emit 0x89
        _emit 0xce
        _emit 0x00
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // Zero DWORDs +18,+1C,+20,+24,+30,+34
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
        _emit 0x89
        _emit 0x48
        _emit 0x30
        _emit 0x89
        _emit 0x48
        _emit 0x34
        // Byte1 +2C; RET4
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

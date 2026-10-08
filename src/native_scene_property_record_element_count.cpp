#include "bsp/native_scene_property_record_element_count.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4,
    "The original raw receiver and physical ABI are Win32.");

// Complete native42, SHA256
// e84a191988638f684ec2d81e68f96fae767f4035f8d21a2ae736501afbeb8ce7.
// Literal whole encodings retain all tag branches, unsigned MUL/SHR,
// branch-specific EDX/flags, receiver preservation and three plain RETs.
// No synthetic globals, providers or owning class are supplied.
__declspec(naked) std::uint32_t __fastcall native_scene_property_record_element_count_008ef7f0(
    const void*, std::uint32_t) noexcept {
    __asm {
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        _emit 0x83
        _emit 0xe8
        _emit 0x09
        _emit 0x74
        _emit 0x1b
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        _emit 0x74
        _emit 0x16
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        _emit 0x74
        _emit 0x03
        _emit 0x33
        _emit 0xc0
        _emit 0xc3
        _emit 0xb8
        _emit 0xab
        _emit 0xaa
        _emit 0xaa
        _emit 0xaa
        _emit 0xf7
        _emit 0x61
        _emit 0x24
        _emit 0x8b
        _emit 0xc2
        _emit 0xc1
        _emit 0xe8
        _emit 0x03
        _emit 0xc3
        _emit 0x8b
        _emit 0x41
        _emit 0x24
        _emit 0xc1
        _emit 0xe8
        _emit 0x02
        _emit 0xc3
    }
}

} // namespace bsp

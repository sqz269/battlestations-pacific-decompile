#include "bsp/native_scene_property_bag_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>
#include <stdexcept>

namespace bsp {

static_assert(sizeof(void*) == 4 && native_scene_property_bag_storage_bytes == 276,
    "The original storage and physical ABI are Win32.");

// Evidence: original [008F41A0,008F41DD), SHA256
// 4d8da1738a25d34afd2e5cba2773ad15e2642ae2ae55831e304751ac62b43420.
// Literal encodings preserve the whole body, including XOR's 33 C0 encoding.
// No relocations, unresolved calls, synthetic globals, dispatch or lifetime.
__declspec(naked) void* __fastcall initialize_native_scene_property_bag_storage_008f41a0(
    void*, std::uint32_t, std::uint32_t) noexcept {
    __asm {
        // MOV EDX,ECX; PUSH EDI
        _emit 0x8b
        _emit 0xd1
        _emit 0x57
        // [EDX]=00D16504; XOR EAX,EAX
        _emit 0xc7
        _emit 0x02
        _emit 0x04
        _emit 0x65
        _emit 0xd1
        _emit 0x00
        _emit 0x33
        _emit 0xc0
        // [EDX+4]=00D162C4; [EDX+8]=0
        _emit 0xc7
        _emit 0x42
        _emit 0x04
        _emit 0xc4
        _emit 0x62
        _emit 0xd1
        _emit 0x00
        _emit 0xc7
        _emit 0x42
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // LEA EDI,[EDX+C]; MOV ECX,40h; REP STOSD
        _emit 0x8d
        _emit 0x7a
        _emit 0x0c
        _emit 0xb9
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0xab
        // Owner from entry ESP+4, now ESP+8 after PUSH EDI; [EDX+110]=EAX
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x89
        _emit 0x82
        _emit 0x10
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // [EDX+10C]=0; MOV EAX,EDX; POP EDI; RET4
        _emit 0xc7
        _emit 0x82
        _emit 0x0c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xc2
        _emit 0x5f
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}

void* clone_empty_native_scene_property_bag_008f41f0_fragment(
    const void* actual_source_bag) {
    void* const output = singleton_lifetime_allocate({SingletonAllocationKind::object,
        native_scene_property_bag_storage_bytes, native_scene_property_bag_storage_bytes});
    initialize_native_scene_property_bag_storage_008f41a0(output, 0, 0);

    // Original initializes the new root before 480690 reads source map+4.
    // Its genuine count-zero iterator exit reads no head or node. Whole raw61
    // stores owner+110 then ordinal+10C, exactly like the clone's inline init.
    std::uint32_t actual_count;
    std::memcpy(&actual_count, static_cast<const std::byte*>(actual_source_bag) + 8,
        sizeof(actual_count));
    if (actual_count != 0) {
        // Source-only unsupported-input rejection. This fresh empty raw
        // allocation has no payload to destroy; no class callback is invented.
        singleton_lifetime_free(output);
        throw std::invalid_argument("empty property bag clone requires zero source count");
    }
    return output;
}

} // namespace bsp

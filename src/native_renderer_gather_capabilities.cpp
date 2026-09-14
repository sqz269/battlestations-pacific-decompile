#include "bsp/native_renderer_gather_capabilities.hpp"
#include "bsp/native_renderer_capability_array_reserves.hpp"
#include "bsp/native_renderer_capability_nested_arrays.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Renderer capability gathering requires MSVC Win32.
#endif

namespace bsp {
namespace {
using U32 = std::uint32_t;
using I32 = std::int32_t;
static_assert(sizeof(void*) == 4);

U32 address(const void* value) noexcept { return reinterpret_cast<U32>(value); }
void* pointer(U32 value) noexcept { return reinterpret_cast<void*>(value); }
void* at(const void* base, U32 byte_offset) noexcept {
    return pointer(address(base) + byte_offset);
}
U32 load(const void* base, U32 byte_offset) noexcept {
    return *reinterpret_cast<const volatile U32*>(at(base, byte_offset));
}
std::uint16_t load_word(const void* base, U32 byte_offset) noexcept {
    return *reinterpret_cast<const volatile std::uint16_t*>(at(base, byte_offset));
}
std::uint8_t load_byte(const void* base, U32 byte_offset) noexcept {
    return *reinterpret_cast<const volatile std::uint8_t*>(at(base, byte_offset));
}
void store(void* base, U32 byte_offset, U32 value) noexcept {
    *reinterpret_cast<volatile U32*>(at(base, byte_offset)) = value;
}
void store_byte(void* base, U32 byte_offset, std::uint8_t value) noexcept {
    *reinterpret_cast<volatile std::uint8_t*>(at(base, byte_offset)) = value;
}

using OutputQuery = I32 (__stdcall*)(void*, U32, U32, void*);
using FormatQuery = I32 (__stdcall*)(void*, U32, U32, U32, U32, U32, U32);

// Concrete COM signature, no injectable provider or cached object/table.
I32 check_format(void* renderer, U32 format, U32 resource, U32 usage) {
    void* const factory = pointer(load(renderer, 0x1990));
    void* const table = pointer(load(factory, 0));
    const auto method = reinterpret_cast<FormatQuery>(load(table, 0x28));
    return method(factory, 0, 1, 0x16, usage, resource, format);
}

I32 doubled_capacity(U32 value) noexcept {
    const I32 doubled = static_cast<I32>(value + value);
    return doubled > 1 ? doubled : 1;
}

// B2CB55..B2CE72 repeats this current-header append for fixed declarations.
void append_declaration(void* header, U32 declaration) {
    const U32 capacity = load(header, 8);
    if (load(header, 4) == capacity)
        reserve_native_capability_dwords_00b236b0(
            header, 0, doubled_capacity(capacity));
    const U32 count = load(header, 4);
    const U32 data = load(header, 0);
    const U32 destination = data + count * 4u;
    if (destination != 0) store(pointer(destination), 0, declaration);
    store(header, 4, load(header, 4) + 1u);
}

// Exact immutable bytes at D5E810, D5E808, D5E800 and D5E7FC.
const char description_8800[] = "8800";
const char description_8600[] = "8600";
const char description_8200[] = "8200";
const char description_ati[] = "ATI";

// Fixed host service boundary, not an implementation of optimized BF9440.
bool description_contains(const char* description, const char* needle) {
    const char* const found = std::strstr(description, needle);
    return found != nullptr && address(found) - address(description) != 0xffffffffu;
}

// B2CEBA..B2D716: 57 rows, five individual native stores per row. The fourth
// metadata byte is written but is never consumed by this gather's queries.
void initialize_format_table(void* scratch) {
    store(scratch, 0x2c, 0x32495441); // 00B2CEBA
    store_byte(scratch, 0x30, 0x1); // 00B2CEC2
    store_byte(scratch, 0x31, 0x0); // 00B2CEC7
    store_byte(scratch, 0x32, 0x1); // 00B2CECB
    store_byte(scratch, 0x33, 0x1); // 00B2CED0
    store(scratch, 0x34, 0x14); // 00B2CED5
    store_byte(scratch, 0x38, 0x1); // 00B2CEDD
    store_byte(scratch, 0x39, 0x1); // 00B2CEE2
    store_byte(scratch, 0x3a, 0x1); // 00B2CEE7
    store_byte(scratch, 0x3b, 0x1); // 00B2CEEC
    store(scratch, 0x3c, 0x15); // 00B2CEF1
    store_byte(scratch, 0x40, 0x1); // 00B2CEF9
    store_byte(scratch, 0x41, 0x0); // 00B2CEFE
    store_byte(scratch, 0x42, 0x0); // 00B2CF02
    store_byte(scratch, 0x43, 0x0); // 00B2CF06
    store(scratch, 0x44, 0x16); // 00B2CF0A
    store_byte(scratch, 0x48, 0x1); // 00B2CF12
    store_byte(scratch, 0x49, 0x0); // 00B2CF17
    store_byte(scratch, 0x4a, 0x0); // 00B2CF1B
    store_byte(scratch, 0x4b, 0x0); // 00B2CF1F
    store(scratch, 0x4c, 0x17); // 00B2CF23
    store_byte(scratch, 0x50, 0x1); // 00B2CF2B
    store_byte(scratch, 0x51, 0x0); // 00B2CF30
    store_byte(scratch, 0x52, 0x0); // 00B2CF34
    store_byte(scratch, 0x53, 0x0); // 00B2CF38
    store(scratch, 0x54, 0x18); // 00B2CF3C
    store_byte(scratch, 0x58, 0x1); // 00B2CF44
    store_byte(scratch, 0x59, 0x0); // 00B2CF49
    store_byte(scratch, 0x5a, 0x0); // 00B2CF4D
    store_byte(scratch, 0x5b, 0x0); // 00B2CF51
    store(scratch, 0x5c, 0x19); // 00B2CF55
    store_byte(scratch, 0x60, 0x1); // 00B2CF5D
    store_byte(scratch, 0x61, 0x0); // 00B2CF62
    store_byte(scratch, 0x62, 0x0); // 00B2CF66
    store_byte(scratch, 0x63, 0x0); // 00B2CF6A
    store(scratch, 0x64, 0x1a); // 00B2CF6E
    store_byte(scratch, 0x68, 0x1); // 00B2CF76
    store_byte(scratch, 0x69, 0x0); // 00B2CF7B
    store_byte(scratch, 0x6a, 0x0); // 00B2CF7F
    store_byte(scratch, 0x6b, 0x0); // 00B2CF83
    store(scratch, 0x6c, 0x1b); // 00B2CF87
    store_byte(scratch, 0x70, 0x1); // 00B2CF8F
    store_byte(scratch, 0x71, 0x0); // 00B2CF94
    store_byte(scratch, 0x72, 0x0); // 00B2CF98
    store_byte(scratch, 0x73, 0x0); // 00B2CF9C
    store(scratch, 0x74, 0x1c); // 00B2CFA0
    store_byte(scratch, 0x78, 0x1); // 00B2CFA8
    store_byte(scratch, 0x79, 0x0); // 00B2CFAD
    store_byte(scratch, 0x7a, 0x0); // 00B2CFB1
    store_byte(scratch, 0x7b, 0x0); // 00B2CFB5
    store(scratch, 0x7c, 0x1d); // 00B2CFB9
    store_byte(scratch, 0x80, 0x1); // 00B2CFC1
    store_byte(scratch, 0x81, 0x0); // 00B2CFC9
    store_byte(scratch, 0x82, 0x0); // 00B2CFD0
    store_byte(scratch, 0x83, 0x0); // 00B2CFD7
    store(scratch, 0x84, 0x1e); // 00B2CFDE
    store_byte(scratch, 0x88, 0x1); // 00B2CFE9
    store_byte(scratch, 0x89, 0x0); // 00B2CFF1
    store_byte(scratch, 0x8a, 0x0); // 00B2CFF8
    store_byte(scratch, 0x8b, 0x0); // 00B2CFFF
    store(scratch, 0x8c, 0x1f); // 00B2D006
    store_byte(scratch, 0x90, 0x0); // 00B2D011
    store_byte(scratch, 0x91, 0x0); // 00B2D018
    store_byte(scratch, 0x92, 0x0); // 00B2D01F
    store_byte(scratch, 0x93, 0x0); // 00B2D026
    store(scratch, 0x94, 0x20); // 00B2D02D
    store_byte(scratch, 0x98, 0x1); // 00B2D038
    store_byte(scratch, 0x99, 0x0); // 00B2D040
    store_byte(scratch, 0x9a, 0x0); // 00B2D047
    store_byte(scratch, 0x9b, 0x0); // 00B2D04E
    store(scratch, 0x9c, 0x21); // 00B2D055
    store_byte(scratch, 0xa0, 0x1); // 00B2D060
    store_byte(scratch, 0xa1, 0x0); // 00B2D068
    store_byte(scratch, 0xa2, 0x0); // 00B2D06F
    store_byte(scratch, 0xa3, 0x0); // 00B2D076
    store(scratch, 0xa4, 0x22); // 00B2D07D
    store_byte(scratch, 0xa8, 0x1); // 00B2D088
    store_byte(scratch, 0xa9, 0x0); // 00B2D090
    store_byte(scratch, 0xaa, 0x0); // 00B2D097
    store_byte(scratch, 0xab, 0x0); // 00B2D09E
    store(scratch, 0xac, 0x23); // 00B2D0A5
    store_byte(scratch, 0xb0, 0x1); // 00B2D0B0
    store_byte(scratch, 0xb1, 0x0); // 00B2D0B8
    store_byte(scratch, 0xb2, 0x0); // 00B2D0BF
    store_byte(scratch, 0xb3, 0x0); // 00B2D0C6
    store(scratch, 0xb4, 0x24); // 00B2D0CD
    store_byte(scratch, 0xb8, 0x1); // 00B2D0D8
    store_byte(scratch, 0xb9, 0x0); // 00B2D0E0
    store_byte(scratch, 0xba, 0x0); // 00B2D0E7
    store_byte(scratch, 0xbb, 0x0); // 00B2D0EE
    store(scratch, 0xbc, 0x28); // 00B2D0F5
    store_byte(scratch, 0xc0, 0x1); // 00B2D100
    store_byte(scratch, 0xc1, 0x0); // 00B2D108
    store_byte(scratch, 0xc2, 0x0); // 00B2D10F
    store_byte(scratch, 0xc3, 0x0); // 00B2D116
    store(scratch, 0xc4, 0x29); // 00B2D11D
    store_byte(scratch, 0xc8, 0x1); // 00B2D128
    store_byte(scratch, 0xc9, 0x0); // 00B2D130
    store_byte(scratch, 0xca, 0x0); // 00B2D137
    store_byte(scratch, 0xcb, 0x0); // 00B2D13E
    store(scratch, 0xcc, 0x32); // 00B2D145
    store_byte(scratch, 0xd0, 0x1); // 00B2D150
    store_byte(scratch, 0xd1, 0x0); // 00B2D158
    store_byte(scratch, 0xd2, 0x0); // 00B2D15F
    store_byte(scratch, 0xd3, 0x0); // 00B2D166
    store(scratch, 0xd4, 0x33); // 00B2D16D
    store_byte(scratch, 0xd8, 0x1); // 00B2D178
    store_byte(scratch, 0xd9, 0x0); // 00B2D180
    store_byte(scratch, 0xda, 0x0); // 00B2D187
    store_byte(scratch, 0xdb, 0x0); // 00B2D18E
    store(scratch, 0xdc, 0x34); // 00B2D195
    store_byte(scratch, 0xe0, 0x1); // 00B2D1A0
    store_byte(scratch, 0xe1, 0x0); // 00B2D1A8
    store_byte(scratch, 0xe2, 0x0); // 00B2D1AF
    store_byte(scratch, 0xe3, 0x0); // 00B2D1B6
    store(scratch, 0xe4, 0x3c); // 00B2D1BD
    store_byte(scratch, 0xe8, 0x1); // 00B2D1C8
    store_byte(scratch, 0xe9, 0x0); // 00B2D1D0
    store_byte(scratch, 0xea, 0x0); // 00B2D1D7
    store_byte(scratch, 0xeb, 0x0); // 00B2D1DE
    store(scratch, 0xec, 0x3d); // 00B2D1E5
    store_byte(scratch, 0xf0, 0x1); // 00B2D1F0
    store_byte(scratch, 0xf1, 0x0); // 00B2D1F8
    store_byte(scratch, 0xf2, 0x0); // 00B2D1FF
    store_byte(scratch, 0xf3, 0x0); // 00B2D206
    store(scratch, 0xf4, 0x3e); // 00B2D20D
    store_byte(scratch, 0xf8, 0x1); // 00B2D218
    store_byte(scratch, 0xf9, 0x0); // 00B2D220
    store_byte(scratch, 0xfa, 0x0); // 00B2D227
    store_byte(scratch, 0xfb, 0x0); // 00B2D22E
    store(scratch, 0xfc, 0x3f); // 00B2D235
    store_byte(scratch, 0x100, 0x1); // 00B2D240
    store_byte(scratch, 0x101, 0x0); // 00B2D248
    store_byte(scratch, 0x102, 0x0); // 00B2D24F
    store_byte(scratch, 0x103, 0x0); // 00B2D256
    store(scratch, 0x104, 0x40); // 00B2D25D
    store_byte(scratch, 0x108, 0x1); // 00B2D268
    store_byte(scratch, 0x109, 0x0); // 00B2D270
    store_byte(scratch, 0x10a, 0x0); // 00B2D277
    store_byte(scratch, 0x10b, 0x0); // 00B2D27E
    store(scratch, 0x10c, 0x43); // 00B2D285
    store_byte(scratch, 0x110, 0x1); // 00B2D290
    store_byte(scratch, 0x111, 0x0); // 00B2D298
    store_byte(scratch, 0x112, 0x0); // 00B2D29F
    store_byte(scratch, 0x113, 0x0); // 00B2D2A6
    store(scratch, 0x114, 0x59565955); // 00B2D2AD
    store_byte(scratch, 0x118, 0x1); // 00B2D2B8
    store_byte(scratch, 0x119, 0x0); // 00B2D2C0
    store_byte(scratch, 0x11a, 0x0); // 00B2D2C7
    store_byte(scratch, 0x11b, 0x0); // 00B2D2CE
    store(scratch, 0x11c, 0x47424752); // 00B2D2D5
    store_byte(scratch, 0x120, 0x1); // 00B2D2E0
    store_byte(scratch, 0x121, 0x0); // 00B2D2E8
    store_byte(scratch, 0x122, 0x0); // 00B2D2EF
    store_byte(scratch, 0x123, 0x0); // 00B2D2F6
    store(scratch, 0x124, 0x32595559); // 00B2D2FD
    store_byte(scratch, 0x128, 0x1); // 00B2D308
    store_byte(scratch, 0x129, 0x0); // 00B2D310
    store_byte(scratch, 0x12a, 0x0); // 00B2D317
    store_byte(scratch, 0x12b, 0x0); // 00B2D31E
    store(scratch, 0x12c, 0x42475247); // 00B2D325
    store_byte(scratch, 0x130, 0x1); // 00B2D330
    store_byte(scratch, 0x131, 0x0); // 00B2D338
    store_byte(scratch, 0x132, 0x0); // 00B2D33F
    store_byte(scratch, 0x133, 0x0); // 00B2D346
    store(scratch, 0x134, 0x31545844); // 00B2D34D
    store_byte(scratch, 0x138, 0x1); // 00B2D358
    store_byte(scratch, 0x139, 0x0); // 00B2D360
    store_byte(scratch, 0x13a, 0x0); // 00B2D367
    store_byte(scratch, 0x13b, 0x0); // 00B2D36E
    store(scratch, 0x13c, 0x32545844); // 00B2D375
    store_byte(scratch, 0x140, 0x1); // 00B2D380
    store_byte(scratch, 0x141, 0x0); // 00B2D388
    store_byte(scratch, 0x142, 0x0); // 00B2D38F
    store_byte(scratch, 0x143, 0x0); // 00B2D396
    store(scratch, 0x144, 0x33545844); // 00B2D39D
    store_byte(scratch, 0x148, 0x1); // 00B2D3A8
    store_byte(scratch, 0x149, 0x0); // 00B2D3B0
    store_byte(scratch, 0x14a, 0x0); // 00B2D3B7
    store_byte(scratch, 0x14b, 0x0); // 00B2D3BE
    store(scratch, 0x14c, 0x34545844); // 00B2D3C5
    store_byte(scratch, 0x150, 0x1); // 00B2D3D0
    store_byte(scratch, 0x151, 0x0); // 00B2D3D8
    store_byte(scratch, 0x152, 0x0); // 00B2D3DF
    store_byte(scratch, 0x153, 0x0); // 00B2D3E6
    store(scratch, 0x154, 0x35545844); // 00B2D3ED
    store_byte(scratch, 0x158, 0x1); // 00B2D3F8
    store_byte(scratch, 0x159, 0x0); // 00B2D400
    store_byte(scratch, 0x15a, 0x0); // 00B2D407
    store_byte(scratch, 0x15b, 0x0); // 00B2D40E
    store(scratch, 0x15c, 0x46); // 00B2D415
    store_byte(scratch, 0x160, 0x1); // 00B2D420
    store_byte(scratch, 0x161, 0x1); // 00B2D428
    store_byte(scratch, 0x162, 0x0); // 00B2D430
    store_byte(scratch, 0x163, 0x0); // 00B2D437
    store(scratch, 0x164, 0x47); // 00B2D43E
    store_byte(scratch, 0x168, 0x1); // 00B2D449
    store_byte(scratch, 0x169, 0x1); // 00B2D451
    store_byte(scratch, 0x16a, 0x0); // 00B2D459
    store_byte(scratch, 0x16b, 0x0); // 00B2D460
    store(scratch, 0x16c, 0x49); // 00B2D467
    store_byte(scratch, 0x170, 0x1); // 00B2D472
    store_byte(scratch, 0x171, 0x1); // 00B2D47A
    store_byte(scratch, 0x172, 0x0); // 00B2D482
    store_byte(scratch, 0x173, 0x0); // 00B2D489
    store(scratch, 0x174, 0x4b); // 00B2D490
    store_byte(scratch, 0x178, 0x1); // 00B2D49B
    store_byte(scratch, 0x179, 0x1); // 00B2D4A3
    store_byte(scratch, 0x17a, 0x0); // 00B2D4AB
    store_byte(scratch, 0x17b, 0x0); // 00B2D4B2
    store(scratch, 0x17c, 0x4d); // 00B2D4B9
    store_byte(scratch, 0x180, 0x1); // 00B2D4C4
    store_byte(scratch, 0x181, 0x1); // 00B2D4CC
    store_byte(scratch, 0x182, 0x0); // 00B2D4D4
    store_byte(scratch, 0x183, 0x0); // 00B2D4DB
    store(scratch, 0x184, 0x4f); // 00B2D4E2
    store_byte(scratch, 0x188, 0x1); // 00B2D4ED
    store_byte(scratch, 0x189, 0x1); // 00B2D4F5
    store_byte(scratch, 0x18a, 0x0); // 00B2D4FD
    store_byte(scratch, 0x18b, 0x0); // 00B2D504
    store(scratch, 0x18c, 0x50); // 00B2D50B
    store_byte(scratch, 0x190, 0x1); // 00B2D516
    store_byte(scratch, 0x191, 0x1); // 00B2D51E
    store_byte(scratch, 0x192, 0x0); // 00B2D526
    store_byte(scratch, 0x193, 0x0); // 00B2D52D
    store(scratch, 0x194, 0x52); // 00B2D534
    store_byte(scratch, 0x198, 0x1); // 00B2D53F
    store_byte(scratch, 0x199, 0x1); // 00B2D547
    store_byte(scratch, 0x19a, 0x0); // 00B2D54F
    store_byte(scratch, 0x19b, 0x0); // 00B2D556
    store(scratch, 0x19c, 0x53); // 00B2D55D
    store_byte(scratch, 0x1a0, 0x1); // 00B2D568
    store_byte(scratch, 0x1a1, 0x1); // 00B2D570
    store_byte(scratch, 0x1a2, 0x0); // 00B2D578
    store_byte(scratch, 0x1a3, 0x0); // 00B2D57F
    store(scratch, 0x1a4, 0x51); // 00B2D586
    store_byte(scratch, 0x1a8, 0x1); // 00B2D591
    store_byte(scratch, 0x1a9, 0x0); // 00B2D599
    store_byte(scratch, 0x1aa, 0x0); // 00B2D5A0
    store_byte(scratch, 0x1ab, 0x0); // 00B2D5A7
    store(scratch, 0x1ac, 0x6e); // 00B2D5AE
    store_byte(scratch, 0x1b0, 0x1); // 00B2D5B9
    store_byte(scratch, 0x1b1, 0x0); // 00B2D5C1
    store_byte(scratch, 0x1b2, 0x0); // 00B2D5C8
    store_byte(scratch, 0x1b3, 0x0); // 00B2D5CF
    store(scratch, 0x1b4, 0x3154454d); // 00B2D5D6
    store_byte(scratch, 0x1b8, 0x1); // 00B2D5E1
    store_byte(scratch, 0x1b9, 0x0); // 00B2D5E9
    store_byte(scratch, 0x1ba, 0x0); // 00B2D5F0
    store_byte(scratch, 0x1bb, 0x0); // 00B2D5F7
    store(scratch, 0x1bc, 0x6f); // 00B2D5FE
    store_byte(scratch, 0x1c0, 0x1); // 00B2D609
    store_byte(scratch, 0x1c1, 0x0); // 00B2D611
    store_byte(scratch, 0x1c2, 0x0); // 00B2D618
    store_byte(scratch, 0x1c3, 0x0); // 00B2D61F
    store(scratch, 0x1c4, 0x70); // 00B2D626
    store_byte(scratch, 0x1c8, 0x1); // 00B2D631
    store_byte(scratch, 0x1c9, 0x0); // 00B2D639
    store_byte(scratch, 0x1ca, 0x0); // 00B2D640
    store_byte(scratch, 0x1cb, 0x0); // 00B2D647
    store(scratch, 0x1cc, 0x71); // 00B2D64E
    store_byte(scratch, 0x1d0, 0x1); // 00B2D659
    store_byte(scratch, 0x1d1, 0x0); // 00B2D661
    store_byte(scratch, 0x1d2, 0x0); // 00B2D668
    store_byte(scratch, 0x1d3, 0x0); // 00B2D66F
    store(scratch, 0x1d4, 0x72); // 00B2D676
    store_byte(scratch, 0x1d8, 0x1); // 00B2D681
    store_byte(scratch, 0x1d9, 0x0); // 00B2D689
    store_byte(scratch, 0x1da, 0x0); // 00B2D690
    store_byte(scratch, 0x1db, 0x0); // 00B2D697
    store(scratch, 0x1dc, 0x73); // 00B2D69E
    store_byte(scratch, 0x1e0, 0x1); // 00B2D6A9
    store_byte(scratch, 0x1e1, 0x0); // 00B2D6B1
    store_byte(scratch, 0x1e2, 0x0); // 00B2D6B8
    store_byte(scratch, 0x1e3, 0x0); // 00B2D6BF
    store(scratch, 0x1e4, 0x74); // 00B2D6C6
    store_byte(scratch, 0x1e8, 0x1); // 00B2D6D1
    store_byte(scratch, 0x1e9, 0x0); // 00B2D6D9
    store_byte(scratch, 0x1ea, 0x0); // 00B2D6E0
    store_byte(scratch, 0x1eb, 0x0); // 00B2D6E7
    store(scratch, 0x1ec, 0x75); // 00B2D6EE
    store_byte(scratch, 0x1f0, 0x1); // 00B2D6F9
    store_byte(scratch, 0x1f1, 0x0); // 00B2D701
    store_byte(scratch, 0x1f2, 0x0); // 00B2D708
    store_byte(scratch, 0x1f3, 0x0); // 00B2D70F
}

void gather_format_records(void* renderer, void* scratch) {
    // Native ESI starts at3; its signed <=3 backedge executes one resource type.
    const U32 resource = 3;
    store(scratch, 0x10, 0);
    bool more;
    do {
        const U32 index = load(scratch, 0x10);
        const U32 format = load(scratch, 0x2c + index * 8u);
        if (check_format(renderer, format, resource, 0) == 0) {
            const U32 metadata_index = load(scratch, 0x10);
            const bool test_usage1 = load_byte(scratch, 0x30 + metadata_index * 8u) != 0;
            store_byte(scratch, 0x20, 0);
            store_byte(scratch, 0x21, 0);
            store_byte(scratch, 0x22, 0);
            store_byte(scratch, 0x23, 0);
            if (test_usage1)
                store_byte(scratch, 0x20,
                    static_cast<std::uint8_t>(check_format(renderer, format, resource, 1) == 0));

            const U32 depth_index = load(scratch, 0x10);
            if (load_byte(scratch, 0x31 + depth_index * 8u) != 0)
                store_byte(scratch, 0x21,
                    static_cast<std::uint8_t>(check_format(renderer, format, resource, 2) == 0));

            const U32 dynamic_index = load(scratch, 0x10);
            if (load_byte(scratch, 0x32 + dynamic_index * 8u) != 0)
                store_byte(scratch, 0x22,
                    static_cast<std::uint8_t>(check_format(renderer, format, resource, 0x200) == 0));

            const bool last_supported =
                check_format(renderer, format, resource, 0x80001) == 0;
            const bool grow_outer = static_cast<I32>(resource) >=
                static_cast<I32>(load(renderer, 0x1b6c));
            store_byte(scratch, 0x24, static_cast<std::uint8_t>(last_supported));
            if (grow_outer)
                resize_native_capability_headers_00b2ae20(at(renderer, 0x1b68), 0, 4);

            const U32 outer_data = load(renderer, 0x1b68);
            void* inner = pointer(outer_data + resource * 12u);
            const U32 capacity = load(inner, 8);
            const bool grow_inner = load(inner, 4) == capacity;
            store(scratch, 0x28, address(inner));
            if (grow_inner) {
                const I32 request = doubled_capacity(capacity);
                void* const current_inner = pointer(load(scratch, 0x28));
                reserve_native_capability_records_00b22b30(current_inner, 0, request);
                inner = pointer(load(scratch, 0x28));
            }
            const U32 count = load(inner, 4);
            const U32 data = load(inner, 0);
            const U32 destination = data + count * 12u;
            if (destination != 0) {
                const U32 flags = load(scratch, 0x20);
                store(pointer(destination), 0, format);
                store(pointer(destination), 4, flags);
                const U32 payload = load(scratch, 0x24);
                store(pointer(destination), 8, payload);
            }
            store(inner, 4, load(inner, 4) + 1u);
        }
        const U32 next = load(scratch, 0x10) + 1u;
        more = next < 0x39u;
        store(scratch, 0x10, next);
    } while (more);
}
} // namespace

namespace {
SingletonLifetimeDomain& pool_lifetime(const NativeRendererGatherCapabilitiesContext* context) noexcept {
    return context->actual_lifetime;
}
void* volatile& pool_lifetime(const NativeRendererGatherCapabilitiesActualContext* context) noexcept {
    return context->actual_manager_publication_01090aa0;
}
template<class Context>
void gather_capabilities_body(void* renderer, const Context* context) {
    void* const scratch = context->actual_callee_scratch;
    volatile I32 cleanup_state = -1;

    // B2C8FF..B2C91A: actual output bytes survive an ignored failed HRESULT.
    void* const caps_output = at(scratch, 0x1f4);
    void* factory = pointer(load(renderer, 0x1990));
    void* table = pointer(load(factory, 0));
    const auto caps_method = reinterpret_cast<OutputQuery>(load(table, 0x38));
    (void)caps_method(factory, 0, 1, caps_output);

    // Interleaved original scratch reads and renderer publications, including
    // the factory capture BEFORE the final pixel-shader/volume stores.
    const U32 caps2 = load(scratch, 0x200);
    const U32 width = load(scratch, 0x24c);
    store_byte(renderer, 0x1b54, static_cast<std::uint8_t>((caps2 >> 20u) & 1u));
    const U32 height = load(scratch, 0x250);
    store(renderer, 0x1b1c, height);
    const U32 vertex_version = load_word(scratch, 0x2b8);
    store(renderer, 0x1b18, width);
    const U32 pixel_version = load_word(scratch, 0x2c0);
    store_byte(renderer, 0x1b53, static_cast<std::uint8_t>((caps2 >> 17u) & 1u));
    const U32 volume = load(scratch, 0x254);
    store(renderer, 0x1b44, vertex_version);
    factory = pointer(load(renderer, 0x1990));
    store(renderer, 0x1b40, pixel_version);
    store(renderer, 0x1b20, volume);
    table = pointer(load(factory, 0));
    const auto atoc_method = reinterpret_cast<FormatQuery>(load(table, 0x28));
    if (atoc_method(factory, 0, 1, 0x16, 0, 1, 0x434f5441) == 0)
        store_byte(renderer, 0x1b55, 1);

    factory = pointer(load(renderer, 0x1990));
    table = pointer(load(factory, 0));
    void* const identifier_output = at(scratch, 0x324);
    const auto identifier_method = reinterpret_cast<OutputQuery>(load(table, 0x14));
    (void)identifier_method(factory, 0, 0, identifier_output);

    // B2C9BD..B2CA0A: the scan, resize and copy precede cleanup-state0.
    const U32 source_description = address(scratch) + 0x524u;
    store(scratch, 0x14, 0);
    store(scratch, 0x18, 0);
    U32 cursor = source_description;
    const U32 past_first = cursor + 1u;
    std::uint8_t character;
    do {
        character = load_byte(pointer(cursor), 0);
        ++cursor;
    } while (character != 0);
    const U32 description_length = cursor - past_first;
    ActualNativeStringPoolStorage storage(context->actual_pool_publication_01090aa8,
        context->actual_small_returns_disabled_01090aa4, pool_lifetime(context));
    resize_native_string_header_0041dd40(at(scratch, 0x14), storage,
        description_length, true);
    const U32 description_address = load(scratch, 0x18);
    if (description_address != 0) {
        const U32 bytes = load(scratch, 0x14) + 1u;
        void* const copy_source = at(scratch, 0x524);
        void* const copy_destination = pointer(description_address);
        if (bytes != 0) std::memmove(copy_destination, copy_source, bytes);
    }
    const bool nonnull_description = description_address != 0;
    cleanup_state = 0;
    bool release_on_return;
    try {
        const auto* const description = reinterpret_cast<const char*>(description_address);
        const bool special_description = nonnull_description &&
            (description_contains(description, description_8800) ||
             description_contains(description, description_8600) ||
             description_contains(description, description_8200) ||
             description_contains(description, description_ati));
        store_byte(renderer, 0x1d89, static_cast<std::uint8_t>(special_description));

        const U32 current_pixel_version = load(renderer, 0x1b40);
        store(renderer, 0x1b48, current_pixel_version < 0x200u ? 1u : 2u);
        if (current_pixel_version <= 0x104u) {
            store(renderer, 0x1b2c, 4);
            store_byte(renderer, 0x1b30, 0);
        } else if (current_pixel_version == 0x200u || current_pixel_version == 0x300u) {
            store_byte(renderer, 0x1b30, 1);
            store(renderer, 0x1b2c, 8);
        }

        const U32 max_vertex_constants = load(scratch, 0x2bc);
        const bool clip_planes = load(scratch, 0x298) != 0;
        const U32 textures = load(scratch, 0x28c);
        const U32 anisotropy = load(scratch, 0x260);
        store(renderer, 0x1b4c, max_vertex_constants);
        const bool hardware_transform = (load(scratch, 0x210) & 0x10000u) != 0;
        store(renderer, 0x1b28, textures);
        store(renderer, 0x1b24, anisotropy);
        store_byte(renderer, 0x1b51, static_cast<std::uint8_t>(clip_planes));
        if (hardware_transform && load_word(scratch, 0x2b8) >= 0x101u) {
            store_byte(renderer, 0x1b74, 0);
        } else {
            store_byte(renderer, 0x1b74, 1);
            store(renderer, 0x1b44, 0x101);
        }

        void* const declarations = at(renderer, 0x1b5c);
        append_declaration(declarations, 0);
        append_declaration(declarations, 1);
        append_declaration(declarations, 2);
        append_declaration(declarations, 3);
        append_declaration(declarations, 4);
        if ((load_byte(scratch, 0x2e0) & 1u) != 0) append_declaration(declarations, 5);
        if ((load_byte(scratch, 0x2e0) & 2u) != 0) append_declaration(declarations, 8);
        if ((load_byte(scratch, 0x2e0) & 4u) != 0) append_declaration(declarations, 9);
        if ((load_byte(scratch, 0x2e0) & 8u) != 0) append_declaration(declarations, 10);
        if ((load_byte(scratch, 0x2e0) & 0x10u) != 0) append_declaration(declarations, 11);
        if ((load_byte(scratch, 0x2e0) & 0x20u) != 0) append_declaration(declarations, 12);
        if ((load_byte(scratch, 0x2e0) & 0x40u) != 0) append_declaration(declarations, 13);
        if ((load_byte(scratch, 0x2e0) & 0x80u) != 0) append_declaration(declarations, 14);
        if ((load(scratch, 0x2e0) & 0x100u) != 0) append_declaration(declarations, 15);

        if (load(renderer, 0x1b40) >= 0x300u ||
            check_format(renderer, 0x54534e49, 1, 0) == 0)
            store_byte(renderer, 0x1b50, 1);
        const std::uint8_t dev_caps2 = static_cast<std::uint8_t>(load_byte(scratch, 0x2c8) & 1u);
        store_byte(renderer, 0x1b50, 0);
        store_byte(renderer, 0x1b52, dev_caps2);
        initialize_format_table(scratch);
        gather_format_records(renderer, scratch);

        // The predicate is captured before disarm; cleanup operands are later.
        release_on_return = load(scratch, 0x18) != 0;
        cleanup_state = -1;
    } catch (...) {
        if (cleanup_state == 0)
            destroy_native_string_header_0041dd20(at(scratch, 0x14), storage);
        throw;
    }
    if (release_on_return) {
        const U32 length = load(scratch, 0x14);
        void* const data = pointer(load(scratch, 0x18));
        const U32 bytes = length + 1u;
        auto* const current_pool = native_string_pool_get_or_create_00419cc0(
            context->actual_pool_publication_01090aa8, pool_lifetime(context));
        return_native_string_pool_00bd1510(current_pool, data, bytes,
            context->actual_small_returns_disabled_01090aa4);
    }
}
} // namespace

void __fastcall gather_native_renderer_capabilities_00b2c8e0(
    void* renderer, const NativeRendererGatherCapabilitiesContext* context) {
    gather_capabilities_body(renderer, context);
}
void __fastcall gather_native_renderer_capabilities_00b2c8e0(
    void* renderer, const NativeRendererGatherCapabilitiesActualContext* context) {
    gather_capabilities_body(renderer, context);
}
} // namespace bsp

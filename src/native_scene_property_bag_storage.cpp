#include "bsp/native_scene_property_bag_storage.hpp"
#include "bsp/native_scene_property_record_type6_storage.hpp"
#include "bsp/native_scene_property_recursive_lifetime.hpp"
#include "bsp/game_native_physical_pool.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_enum_node_pool.hpp"
#include "bsp/native_scene_property_array_block_release.hpp"
#include "bsp/native_string.hpp"
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

// Keep the accepted producer as an actual call from the type-6 fragment below.
// This does not change its own emitted body or any earlier caller.
__declspec(noinline) void* clone_empty_native_scene_property_bag_008f41f0_fragment(
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

static_assert(native_scene_property_record_type6_storage_bytes == 0x38,
    "The original type-6 record allocation is 38h bytes.");

// Whole [008EF780,008EF7BA), SHA256
// 6ecd8cb72e2e8d06bf1f3f5c53f442d88c3b3606ffc3dacdc6a98f9c5a2f0450.
// Retain all 16 instructions, including the actual child backlink store before
// the final record fields. The phase word is data, never a Source dispatch table.
__declspec(naked) void* __fastcall construct_native_scene_property_record_type6_storage_008ef780(
    void*, std::uint32_t, void*) noexcept {
    __asm {
        mov edx, dword ptr [esp + 4]
        mov eax, ecx
        xor ecx, ecx
        mov dword ptr [eax], 00ce89d4h
        mov dword ptr [eax + 4], 6
        mov dword ptr [eax + 0ch], edx
        mov dword ptr [eax + 18h], ecx
        mov dword ptr [eax + 1ch], ecx
        mov dword ptr [eax + 20h], ecx
        mov dword ptr [eax + 24h], ecx
        mov dword ptr [eax + 30h], ecx
        mov dword ptr [edx + 110h], eax
        mov dword ptr [eax + 34h], ecx
        mov byte ptr [eax + 2ch], 1
        mov dword ptr [eax + 8], ecx
        ret 4
    }
}

void* clone_empty_child_native_scene_property_record_type6_008f50ed_fragment(
    const void* actual_source_record) {
    void* const output = singleton_lifetime_allocate({SingletonAllocationKind::object,
        native_scene_property_record_type6_storage_bytes,
        native_scene_property_record_type6_storage_bytes});

    void* child;
    try {
        void* source_child;
        std::memcpy(&source_child,
            static_cast<const std::byte*>(actual_source_record) + 0x0C,
            sizeof(source_child));
        child = clone_empty_native_scene_property_bag_008f41f0_fragment(source_child);
    } catch (...) {
        // Source-only cleanup of our fresh, still-unattached raw allocation.
        // This does not invent Native CA4B6D cleanup or a record destructor.
        singleton_lifetime_free(output);
        throw;
    }

    void* const record = construct_native_scene_property_record_type6_storage_008ef780(
        output, 0, child);
    std::uint32_t ordinal;
    std::memcpy(&ordinal,
        static_cast<const std::byte*>(actual_source_record) + 0x34, sizeof(ordinal));
    std::memcpy(static_cast<std::byte*>(record) + 0x34, &ordinal, sizeof(ordinal));
    return record;
}

namespace {
// Physical DWORD operations retain every Native read/store, including repeated
// zero stores. No fabricated C++ record/map object or Original numeric access.
__forceinline std::uint32_t property_lifetime_word(const void* actual,
    std::uint32_t byte_offset) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, actual
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}

__forceinline void property_lifetime_store(void* actual, std::uint32_t byte_offset,
    std::uint32_t value) noexcept {
    __asm {
        mov eax, actual
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}

__forceinline void* property_lifetime_pointer(std::uint32_t bits) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}

__forceinline void* property_lifetime_at(void* actual, std::uint32_t offset) noexcept {
    return property_lifetime_pointer(static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(actual)) + offset);
}
} // namespace

NativeScenePropertyLifetimeContext::NativeScenePropertyLifetimeContext(
    game::GameNativeStringProcess& actual_strings,
    game::GameNativePhysicalPoolProcess& actual_physical)
    : keys_(actual_strings.raw_context()),
      property_pool_(actual_physical.property_node_pool_storage_00e175b0()) {}

// Whole008F3F30: actual 108h map, same genuine property-node pool. These fixed
// named calls bind the two proven Native phases to complete Source lifetimes.
__declspec(noinline) void clear_native_scene_property_record_map_008f3f30(
    void* actual_map, NativeScenePropertyLifetimeContext& context) {
    void* head = property_lifetime_at(actual_map, 8);
    for (std::uint32_t bucket = 0; bucket != 64; ++bucket) {
        void* node = property_lifetime_pointer(property_lifetime_word(head, 0));
        while (node) {
            void* const record = property_lifetime_pointer(property_lifetime_word(node, 8));
            void* const next = property_lifetime_pointer(property_lifetime_word(node, 0x0c));
            if (record) {
                scalar_delete_native_scene_property_record_004e6730(record, 1, context);
                property_lifetime_store(node, 8, 0);
            }
            // The node itself already IS the real length/pointer key header.
            destroy_native_string_header_0041dd20(node, context.keys());
            return_native_enum_node_0043b0a0(context.property_pool(), node);
            node = next; // No further read through the returned slot.
        }
        property_lifetime_store(head, 0, 0);
        head = property_lifetime_at(head, 4);
    }
    property_lifetime_store(actual_map, 4, 0);
}

// Whole004E6730, including the real returning-free cleanup at004E6745.
// New ordinary ABI: result carries EAX's original root bits, not RET4 transport.
__declspec(noinline) std::uint32_t scalar_delete_native_scene_property_record_004e6730(
    void* actual_record, std::uint32_t flags, NativeScenePropertyLifetimeContext& context) {
    const auto root_bits = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(actual_record));
    destroy_native_scene_property_record_008f0de0(actual_record, context);
    if (flags & 1u) singleton_lifetime_free(actual_record);
    return root_bits;
}

// Whole008F0DE0: actual phase publication, unchanged receiver, full release.
__declspec(noinline) void destroy_native_scene_property_record_008f0de0(
    void* actual_record, NativeScenePropertyLifetimeContext& context) {
    property_lifetime_store(actual_record, 0, 0x00ce89d4u);
    release_native_scene_property_record_value_008f0640(actual_record, context);
}

// Whole008F0640. Actual8F06F4 selectors for tags2..11 are
// 00 04 04 01 02 04 03 03 03 03: tag7 has no owned release.
__declspec(noinline) void release_native_scene_property_record_value_008f0640(
    void* actual_record, NativeScenePropertyLifetimeContext& context) {
    switch (property_lifetime_word(actual_record, 4)) {
    case 2: {
        void* const data = property_lifetime_pointer(property_lifetime_word(actual_record, 0x0c));
        if (data) {
            singleton_lifetime_free(data);
            property_lifetime_store(actual_record, 0x0c, 0);
        }
        break;
    }
    case 5: {
        void* const data = property_lifetime_pointer(property_lifetime_word(actual_record, 0x1c));
        if (data) {
            singleton_lifetime_free(data);
            property_lifetime_store(actual_record, 0x1c, 0);
        }
        property_lifetime_store(actual_record, 0x18, 0);
        break;
    }
    case 6: {
        void* const child = property_lifetime_pointer(property_lifetime_word(actual_record, 0x0c));
        if (child) {
            scalar_delete_native_scene_property_bag_008f59e0(child, 1, context);
            property_lifetime_store(actual_record, 0x0c, 0);
        }
        break;
    }
    case 8:
    case 9:
    case 10:
    case 11:
        release_native_scene_property_array_block_008f03f0(property_lifetime_at(actual_record, 0x20));
        break;
    default:
        break;
    }
    property_lifetime_store(actual_record, 4, 0);
    property_lifetime_store(actual_record, 0x0c, 0);
    property_lifetime_store(actual_record, 0x28, 0);
    property_lifetime_store(actual_record, 8, 0);
}

// Whole008F59E0. Destruction is unconditional, independent of low free bit.
__declspec(noinline) std::uint32_t scalar_delete_native_scene_property_bag_008f59e0(
    void* actual_bag, std::uint32_t flags, NativeScenePropertyLifetimeContext& context) {
    const auto root_bits = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(actual_bag));
    destroy_native_scene_property_bag_008f5410(actual_bag, context);
    if (flags & 1u) singleton_lifetime_free(actual_bag);
    return root_bits;
}

// Whole008F5410 ordinary successful schedule. Original FS/SEH transport and
// exceptional cleanup are outside this context-bearing Source interface.
__declspec(noinline) void destroy_native_scene_property_bag_008f5410(
    void* actual_bag, NativeScenePropertyLifetimeContext& context) {
    property_lifetime_store(actual_bag, 0, 0x00d16504u);
    void* const actual_map = property_lifetime_at(actual_bag, 4);
    clear_native_scene_property_record_map_008f3f30(actual_map, context);
    property_lifetime_store(actual_map, 0, 0x00d162c4u);
    clear_native_scene_property_record_map_008f3f30(actual_map, context);
}

} // namespace bsp

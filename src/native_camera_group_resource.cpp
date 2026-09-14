#include "bsp/native_camera_group_resource.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_resource_node_traversal.hpp"
#include "bsp/native_resource_reader_references.hpp"
#include "bsp/native_resource_value_reads.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native camera resource parsing requires MSVC Win32 x87 assembly.
#endif
extern "C" double __cdecl _CIatan();
namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
void* ptr(U value) noexcept { return reinterpret_cast<void*>(value); }
U bits(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
void* at(const void* p, U offset = 0) noexcept { return ptr(bits(p) + offset); }
U word(const void* p, U offset = 0) noexcept { return *static_cast<const volatile U*>(at(p, offset)); }
void put(void* p, U offset, U value) noexcept { *static_cast<volatile U*>(at(p, offset)) = value; }
void give_back(void* data, U bytes, NativeStringRawPoolContext& strings) {
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, data, bytes, strings.actual_small_returns_disabled_01090aa4);
}
// No intermediate float return through C++ between BE99D0 and the item store.
__declspec(naked) void __cdecl read_group_float(void*, void*, NativeResourceStreamReadContext&) {
    __asm {
        push ebp
        mov ebp, esp
        push dword ptr [ebp+16]
        push dword ptr [ebp+12]
        call read_native_resource_node_float_00be99d0
        add esp, 8
        mov eax, dword ptr [ebp+8]
        fstp dword ptr [eax+8]
        pop ebp
        ret
    }
}
// Preserve every original FSTP32/FLD32, including the repeated atan result
// round trip. BF8490 uses the existing host x87 _CIatan library boundary.
__declspec(naked) void __cdecl read_camera_floats(void*, void*, NativeResourceStreamReadContext&) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 12
        push dword ptr [ebp+16]
        push dword ptr [ebp+12]
        call read_native_resource_node_float_00be99d0
        add esp, 8
        fstp dword ptr [ebp-4]
        push dword ptr [ebp+16]
        push dword ptr [ebp+12]
        call read_native_resource_node_float_00be99d0
        add esp, 8
        fstp dword ptr [ebp-8]
        fld dword ptr [ebp-4]
        fld1
        fdivrp st(1), st(0)
        fstp dword ptr [ebp-12]
        fld dword ptr [ebp-12]
        call _CIatan
        fstp dword ptr [ebp-12]
        fld dword ptr [ebp-12]
        fstp dword ptr [ebp-12]
        fld dword ptr [ebp-12]
        fadd st(0), st(0)
        mov eax, dword ptr [ebp+8]
        fstp dword ptr [eax+8]
        fld dword ptr [ebp-8]
        fstp dword ptr [eax+0ch]
        mov esp, ebp
        pop ebp
        ret
    }
}
}

void* construct_native_resource_item_base_00b868b0(void* item) noexcept {
    put(item, 0, 0x00ceb130); put(item, 4, 1); put(item, 0, 0x00d631a0); return item;
}
void destroy_native_resource_item_base_00b86890(void* item) noexcept {
    put(item, 0, 0x00d5c104); destroy_native_ref_counted_base_00bd30f0(item);
}
void* construct_native_group_params_item_00b8e5a0(void* item) noexcept {
    construct_native_resource_item_base_00b868b0(item); put(item, 0, 0x00d634b0); return item;
}
void read_native_group_params_item_00b8e580(void* item, void* handle, NativeResourceStreamReadContext& reads) {
    read_group_float(item, handle, reads); skip_native_resource_node_00be9c40(handle, reads);
}
void* parse_native_group_params_item_00b8eb50(void* handle, NativeResourceStreamReadContext& reads) {
    auto* const allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x0c, 0x0c});
    auto* const item = allocation ? construct_native_group_params_item_00b8e5a0(allocation) : nullptr;
    // Native allocation cleanup covers only nonthrowing construction stores.
    // It is disarmed before this virtual read, including the null-allocation path.
    const auto table = word(item); const auto target = word(ptr(table), 0x20);
    if (target != 0x00b8e580) throw std::runtime_error("Reached GroupParams reader is not reconstructed");
    read_native_group_params_item_00b8e580(item, handle, reads); return item;
}
void* delete_native_group_params_item_00b8ebc0(void* item, U flags) {
    destroy_native_resource_item_base_00b86890(item);
    if ((flags & 1u) != 0) singleton_lifetime_free(item);
    return item;
}
std::uint8_t matches_native_group_params_type_00b8e5c0(U token, const volatile U* cells) {
    return matches_native_fallback_type_00b86950(token, cells);
}
std::uint8_t matches_native_camera_type_00b8aa30(U token, const volatile U* cells) {
    return matches_native_fallback_type_00b86950(token, cells);
}
void read_native_camera_item_00b8b0b0(void* item, void* handle, NativeResourceStreamReadContext& reads) {
    read_camera_floats(item, handle, reads);
    if (!native_resource_node_has_remaining_00715bf0(handle)) return;
    void* child; U temporary[2]; int state = -1;
    try {
        do {
            create_native_resource_child_00bea680(handle, &child, reads); state = 0;
            auto* const tag = ptr(word(child, 0x14));
            if (tag && _stricmp(static_cast<const char*>(tag), reinterpret_cast<const char*>(0x00d633c8)) == 0) {
                auto* const result = read_native_resource_handle_string_00bea010(&child, temporary, reads);
                auto* const destination = at(item, 0x10); state = 1;
                if (destination != result) {
                    const auto length = word(result);
                    resize_native_string_header_0041dd40(destination, reads.strings, length, true);
                    if (word(result) != 0) {
                        const auto count = word(destination);
                        auto* const data = ptr(word(destination, 4));
                        auto* const source = ptr(word(result, 4));
                        if (count != 0) std::memmove(data, source, count);
                    }
                }
                auto* const data = ptr(word(temporary, 4)); state = 0;
                if (data) { const auto bytes = word(temporary) + 1u; give_back(data, bytes, reads.strings); }
            } else skip_native_resource_node_00be9c40(&child, reads);
            state = -1;
            release_native_structured_node_handle_00be9ed0(&child, reads.streams);
        } while (native_resource_node_has_remaining_00715bf0(handle));
    } catch (...) {
        try {
            if (state == 1) destroy_native_string_header_0041dd20(temporary, reads.strings);
            if (state >= 0) release_native_structured_node_handle_00be9ed0(&child, reads.streams);
        } catch (...) { std::terminate(); }
        throw;
    }
}
void* parse_native_camera_item_00b8b240(void* handle, NativeResourceStreamReadContext& reads) {
    auto* const allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x18, 0x18});
    void* item = nullptr;
    if (allocation) {
        item = construct_native_resource_item_base_00b868b0(allocation);
        put(item, 0, 0x00d632dc); put(item, 0x10, 0); put(item, 0x14, 0);
    }
    // The native state becomes -1 before this direct reader call.
    read_native_camera_item_00b8b0b0(item, handle, reads); return item;
}
void destroy_native_camera_item_00b8aa60(void* item, NativeStringRawPoolContext& strings) {
    auto* const data = ptr(word(item, 0x14));
    try {
        if (data) { const auto bytes = word(item, 0x10) + 1u; give_back(data, bytes, strings); }
    } catch (...) { destroy_native_resource_item_base_00b86890(item); throw; }
    destroy_native_resource_item_base_00b86890(item);
}
void* delete_native_camera_item_00b8b2c0(void* item, U flags, NativeStringRawPoolContext& strings) {
    destroy_native_camera_item_00b8aa60(item, strings);
    if ((flags & 1u) != 0) singleton_lifetime_free(item);
    return item;
}
NativeCameraGroupResourceCalls::NativeCameraGroupResourceCalls(NativeResourceDispatchCalls& other,
    NativeResourceItemTypeCalls& types, NativeResourceStreamReadContext& reads,
    const volatile U* group, const volatile U* camera)
    : other_(other), types_(types), reads_(reads), group_tokens_(group), camera_tokens_(camera) {}
void NativeCameraGroupResourceCalls::renderer_hook(std::uintptr_t e, void* renderer) { other_.renderer_hook(e, renderer); }
void* NativeCameraGroupResourceCalls::parse_item(std::uintptr_t e, void* parser, void* handle) {
    if (e == 0x00b8eb50) return parse_native_group_params_item_00b8eb50(handle, reads_);
    if (e == 0x00b8b240) return parse_native_camera_item_00b8b240(handle, reads_);
    return other_.parse_item(e, parser, handle);
}
void NativeCameraGroupResourceCalls::append_item(std::uintptr_t e, void* resource, void* item) {
    other_.append_item(e, resource, item);
}
std::uint8_t NativeCameraGroupResourceCalls::matches_type(std::uintptr_t e, void* item, U token) {
    if (e == 0x00b8e5c0) return matches_native_group_params_type_00b8e5c0(token, group_tokens_);
    if (e == 0x00b8aa30) return matches_native_camera_type_00b8aa30(token, camera_tokens_);
    return types_.matches_type(e, item, token);
}
NativeCameraGroupResourceReferences::NativeCameraGroupResourceReferences(NativeAdoptedSubstreamDispatch& other,
    NativeStringRawPoolContext& strings) : other_(other), strings_(strings) {}
std::uint8_t NativeCameraGroupResourceReferences::source_is_open(std::uintptr_t e, void* owner) { return other_.source_is_open(e, owner); }
U NativeCameraGroupResourceReferences::source_seek(std::uintptr_t e, void* owner, U lo, U hi, U origin) { return other_.source_seek(e, owner, lo, hi, origin); }
void NativeCameraGroupResourceReferences::source_read(std::uintptr_t e, void* owner, void* data, U count, U* actual) { other_.source_read(e, owner, data, count, actual); }
void NativeCameraGroupResourceReferences::source_write(std::uintptr_t e, void* owner, const void* data, U count, U* actual) { other_.source_write(e, owner, data, count, actual); }
void NativeCameraGroupResourceReferences::source_zero_reference(std::uintptr_t e, void* owner, std::uintptr_t table) {
    if (e == 0x00bd30e0 && (table == 0x00d634b0 || table == 0x00d632dc)) {
        if (!owner) return;
        const auto target = word(ptr(word(owner)), 4);
        if (target == 0x00b8ebc0) delete_native_group_params_item_00b8ebc0(owner, 1);
        else if (target == 0x00b8b2c0) delete_native_camera_item_00b8b2c0(owner, 1, strings_);
        else throw std::runtime_error("Reached camera/GroupParams deleting target is not reconstructed");
        return;
    }
    other_.source_zero_reference(e, owner, table);
}
} // namespace bsp

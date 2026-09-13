#include "bsp/native_node_construction.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native node construction requires MSVC Win32 field and pointer widths.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(NativeString) == 8);
static_assert(sizeof(NativeNodePointLightArray) == 12);
static_assert(sizeof(NativeNodeStorage) == 0x174);
static_assert(offsetof(NativeNodeStorage, references_04) == 4);
static_assert(offsetof(NativeNodeStorage, untouched_08) == 8);
static_assert(offsetof(NativeNodeStorage, parent_30) == 0x30);
static_assert(offsetof(NativeNodeStorage, released_44) == 0x44);
static_assert(offsetof(NativeNodeStorage, name_54) == 0x54);
static_assert(offsetof(NativeNodeStorage, valid_flags_5c) == 0x5c);
static_assert(offsetof(NativeNodeStorage, view_60) == 0x60);
static_assert(offsetof(NativeNodeStorage, notification_context_a0) == 0xa0);
static_assert(offsetof(NativeNodeStorage, root_list_a4) == 0xa4);
static_assert(offsetof(NativeNodeStorage, local_b0) == 0xb0);
static_assert(offsetof(NativeNodeStorage, world_f0) == 0xf0);
static_assert(offsetof(NativeNodeStorage, retained_130) == 0x130);
static_assert(offsetof(NativeNodeStorage, enabled_134) == 0x134);
static_assert(offsetof(NativeNodeStorage, auxiliary_flags_138) == 0x138);
static_assert(offsetof(NativeNodeStorage, world_sphere_13c) == 0x13c);
static_assert(offsetof(NativeNodeStorage, bounds_min_14c) == 0x14c);
static_assert(offsetof(NativeNodeStorage, bounds_max_158) == 0x158);
static_assert(offsetof(NativeNodeStorage, point_lights_164) == 0x164);
static_assert(offsetof(NativeNodeStorage, scene_170) == 0x170);

namespace {
constexpr std::uint32_t one_word = 0x3f800000u; // native 00D7A24C
constexpr std::uint32_t positive_bound_word = 0x501502f9u; // native 00CE4970
constexpr std::uint32_t negative_bound_word = 0xd01502f9u; // native 00CE4ADC

void store_word(float& destination, std::uint32_t word) noexcept {
    std::memcpy(&destination, &word, sizeof(word));
}
void initialize_identity(CameraMatrix& destination) {
    // The native constructs a fresh stack matrix for each destination, then
    // invokes the recovered sequential x87 copy. Destination order is B0,F0,60.
    const CameraMatrix identity{1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    copy_camera_matrix_004134f0(destination, identity);
}
CameraTransformBacking transform_backing(NativeNodeStorage& node) noexcept {
    return {node.parent_30, node.first_child_34, node.child_count_38,
        node.next_sibling_3c, node.previous_sibling_40, node.root_list_a4,
        node.valid_flags_5c, node.auxiliary_flags_138, node.notification_context_a0,
        node.view_60, node.local_b0, node.world_f0,
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&node))};
}

// Raw bytes only: the NativeNodeStorage lifetime has not started yet. Keep the
// first MOVSS before the name header's default initialization, including alias
// cases where the current constant cell resides in the actual slot.
__declspec(noinline) void raw_node_prelude(void* slot,
    const volatile std::uint32_t* positive) noexcept {
    __asm {
        mov ecx, slot
        mov edx, positive
        mov dword ptr [ecx], 00ceb130h
        mov dword ptr [ecx+4], 1
        movss xmm0, dword ptr [edx]
        mov dword ptr [ecx], 00d62c88h
        xor eax, eax
        mov [ecx+30h], eax
        mov [ecx+34h], eax
        mov [ecx+38h], eax
        mov [ecx+3ch], eax
        mov [ecx+40h], eax
        mov [ecx+48h], eax
        movss dword ptr [ecx+50h], xmm0
    }
}

// Deliberately isolated for emitted-code verification. Do not value-initialize
// this aggregate: all native-unwritten bytes and matrices retain preimages.
__declspec(noinline) NativeNodeStorage* materialize_raw_node(void* slot) noexcept {
    return ::new (slot) NativeNodeStorage;
}

__declspec(noinline) std::uint32_t raw_node_word(const void* address) noexcept {
    __asm {
        mov eax, address
        mov eax, [eax]
    }
}

// A fresh stack matrix is filled row by row from one MOVSS capture. The first
// capture also supplies +4C/+AC before +5C is cleared, as at B6F676..B6F696.
__declspec(noinline) void raw_node_identity(CameraMatrix* matrix,
    const volatile std::uint32_t* one, NativeNodeStorage* first_node) noexcept {
    __asm {
        mov ecx, matrix
        mov edx, one
        xorps xmm0, xmm0
        movss xmm1, dword ptr [edx]
        mov eax, first_node
        test eax, eax
        jz fill_matrix
        movss dword ptr [eax+4ch], xmm1
        movss dword ptr [eax+0ach], xmm1
        mov dword ptr [eax+5ch], 0
    fill_matrix:
        movss dword ptr [ecx], xmm1
        movss dword ptr [ecx+4], xmm0
        movss dword ptr [ecx+8], xmm0
        movss dword ptr [ecx+0ch], xmm0
        movss dword ptr [ecx+10h], xmm0
        movss dword ptr [ecx+14h], xmm1
        movss dword ptr [ecx+18h], xmm0
        movss dword ptr [ecx+1ch], xmm0
        movss dword ptr [ecx+20h], xmm0
        movss dword ptr [ecx+24h], xmm0
        movss dword ptr [ecx+28h], xmm1
        movss dword ptr [ecx+2ch], xmm0
        movss dword ptr [ecx+30h], xmm0
        movss dword ptr [ecx+34h], xmm0
        movss dword ptr [ecx+38h], xmm0
        movss dword ptr [ecx+3ch], xmm1
    }
}

__declspec(noinline) void raw_node_finish(NativeNodeStorage* node,
    const volatile std::uint32_t* negative,
    const volatile std::uint32_t* positive) noexcept {
    __asm {
        mov ecx, node
        mov edx, negative
        xorps xmm0, xmm0
        movss xmm1, dword ptr [edx]
        mov dword ptr [ecx+138h], 40h
        movss dword ptr [ecx+13ch], xmm0
        movss dword ptr [ecx+140h], xmm0
        movss dword ptr [ecx+144h], xmm0
        mov edx, positive
        movss xmm0, dword ptr [edx]
        movss dword ptr [ecx+148h], xmm0
        movss dword ptr [ecx+14ch], xmm1
        movss dword ptr [ecx+150h], xmm1
        movss dword ptr [ecx+154h], xmm1
        movss dword ptr [ecx+158h], xmm0
        movss dword ptr [ecx+15ch], xmm0
        movss dword ptr [ecx+160h], xmm0
        mov byte ptr [ecx+44h], 0
        mov byte ptr [ecx+134h], 1
        mov dword ptr [ecx+48h], 0fffffh
    }
}

void finish_raw_node_base(NativeNodeStorage& node) noexcept {
    // Preserve both base profile stores; an optimizing compiler may otherwise
    // remove the intermediate AA6E10 profile in this new interface.
    __asm {
        mov ecx, node
        mov dword ptr [ecx], 00d5c104h
        mov dword ptr [ecx], 00ceb130h
    }
    node.~NativeNodeStorage();
}
}

NativeNodeStorage& construct_native_node_00b6f5a0(void* actual_slot,
    std::size_t slot_bytes, const void* name, NativeStringRawPoolContext& strings,
    const NativeNodeRawConstants& constants) {
    if (!actual_slot || slot_bytes < sizeof(NativeNodeStorage) ||
        (reinterpret_cast<std::uintptr_t>(actual_slot) % alignof(NativeNodeStorage)) != 0)
        throw std::invalid_argument("native node construction requires an aligned actual 0x174-byte prefix");

    raw_node_prelude(actual_slot, &constants.positive_bound_00ce4970);
    auto& node = *materialize_raw_node(actual_slot);
    // MSVC Win32 default construction writes zero at +4,+54,+58 only. The name
    // stores occur at the intended native header-init boundary; restore +4.
    node.references_04.store(1, std::memory_order_relaxed);
    node.notification_context_a0 = nullptr;
    node.root_list_a4 = nullptr;
    node.field_a8 = 0;
    node.retained_130 = nullptr;
    node.point_lights_164 = {nullptr, 0, 0};
    node.scene_170 = nullptr;
    try {
        if (static_cast<const void*>(&node.name_54) != name) {
            resize_native_string_header_0041dd40(&node.name_54, strings,
                raw_node_word(name), true);
            // Deliberately separate current reads after the potentially
            // throwing pool operation: source length, destination length,
            // source data, destination data. Copy excludes the terminator.
            if (raw_node_word(name) != 0) {
                const auto count = raw_node_word(&node.name_54);
                const auto source = raw_node_word(static_cast<const std::byte*>(name) + 4);
                const auto destination = raw_node_word(
                    reinterpret_cast<const std::byte*>(&node.name_54) + 4);
                // Retain all four reads even for a zero destination count;
                // omit only the native zero-byte CRT call, as in raw resize.
                if (count != 0)
                    std::memmove(reinterpret_cast<void*>(destination),
                        reinterpret_cast<const void*>(source), count);
            }
        }
        CameraMatrix local_identity;
        raw_node_identity(&local_identity, &constants.one_00d7a24c, &node);
        copy_camera_matrix_004134f0(node.local_b0, local_identity);
        CameraMatrix world_identity;
        raw_node_identity(&world_identity, &constants.one_00d7a24c, nullptr);
        copy_camera_matrix_004134f0(node.world_f0, world_identity);
        CameraMatrix view_identity;
        raw_node_identity(&view_identity, &constants.one_00d7a24c, nullptr);
        copy_camera_matrix_004134f0(node.view_60, view_identity);
        raw_node_finish(&node, &constants.negative_bound_00ce4adc,
            &constants.positive_bound_00ce4970);
    } catch (...) {
        // State 2 is consumed before entering state 1. Fresh construction only
        // owns an empty descriptor; a callback cannot install foreign backing.
        if (node.point_lights_164.begin || node.point_lights_164.count != 0 ||
            node.point_lights_164.capacity != 0) std::terminate();
        singleton_lifetime_free(nullptr);
        try {
            destroy_native_string_header_0041dd20(&node.name_54, strings);
        } catch (...) {
            // State 1 has already been consumed; do not retry the raw return.
            finish_raw_node_base(node);
            throw;
        }
        finish_raw_node_base(node);
        throw;
    }
    return node;
}

NativeNodeStorage& construct_native_node_00b6f5a0(void* actual_slot,
    std::size_t slot_bytes, const NativeString& name, NativeStringStorage& strings) {
    return construct_native_node_00b6f5a0(actual_slot, slot_bytes, &name, strings);
}
NativeNodeStorage& construct_native_node_00b6f5a0(void* actual_slot,
    std::size_t slot_bytes, const void* name, NativeStringStorage& strings) {
    if (!actual_slot || slot_bytes < sizeof(NativeNodeStorage) ||
        (reinterpret_cast<std::uintptr_t>(actual_slot) % alignof(NativeNodeStorage)) != 0)
        throw std::invalid_argument("native node construction requires an aligned actual 0x174-byte prefix");

    // No parentheses: value-initialization would overwrite the slot preimage.
    auto& node = *::new (actual_slot) NativeNodeStorage;
    node.vtable_00 = 0x00ceb130u;
    node.references_04.store(1, std::memory_order_relaxed);
    node.vtable_00 = 0x00d62c88u;
    node.parent_30 = 0;
    node.first_child_34 = 0;
    node.child_count_38 = 0;
    node.next_sibling_3c = 0;
    node.previous_sibling_40 = 0;
    node.mask_48 = 0;
    node.bounds_scalar_50 = positive_bound_word;
    node.notification_context_a0 = nullptr;
    node.root_list_a4 = nullptr;
    node.field_a8 = 0;
    node.retained_130 = nullptr;
    node.point_lights_164 = {nullptr, 0, 0};
    node.scene_170 = nullptr;
    try {
        copy_native_string_header_00be0a30_fragment(&node.name_54, strings, name);
        node.scalar_4c = one_word;
        node.scalar_ac = one_word;
        node.valid_flags_5c = 0;
        initialize_identity(node.local_b0);
        initialize_identity(node.world_f0);
        initialize_identity(node.view_60);
        node.auxiliary_flags_138 = 0x40;
        store_word(node.world_sphere_13c[0], 0);
        store_word(node.world_sphere_13c[1], 0);
        store_word(node.world_sphere_13c[2], 0);
        store_word(node.world_sphere_13c[3], positive_bound_word);
        for (auto& value : node.bounds_min_14c) store_word(value, negative_bound_word);
        for (auto& value : node.bounds_max_158) store_word(value, positive_bound_word);
        node.released_44 = 0;
        node.enabled_134 = 1;
        node.mask_48 = 0x000fffffu;
    } catch (...) {
        // Native CC1A31 unwind states 2,1,0: array resize0/free, name pool
        // release, then AA6E10 ->BD30F0. No physical slot return belongs here.
        while (node.point_lights_164.count > 0) --node.point_lights_164.count;
        node.point_lights_164.count = 0;
        // This constructor only published an empty array. A supplied string
        // service must not replace its backing while the node is constructing;
        // no unproven foreign pointer may be freed through the host CRT.
        if (node.point_lights_164.begin) std::terminate();
        singleton_lifetime_free(nullptr);
        destroy_native_string_header_0041dd20(&node.name_54, strings);
        node.vtable_00 = 0x00d5c104u;
        node.vtable_00 = 0x00ceb130u;
        node.~NativeNodeStorage();
        throw;
    }
    return node;
}

NativeNodeStorage& construct_native_node_00b6f5a0(void* actual_slot,
    std::size_t slot_bytes, const NativeString& name, SizedStoragePool& strings) {
    PooledStringStorage storage(strings);
    return construct_native_node_00b6f5a0(actual_slot, slot_bytes, name, storage);
}

NativeNodeBinding::NativeNodeBinding(NativeNodeStorage& node,
    SceneTypePredicate actual_virtual_0c, SceneAttachOverride actual_virtual_50,
    void (*actual_notify_changed)(void*), void* dispatch_context)
    : storage(node), transform(transform_backing(node), actual_notify_changed),
      scene_attachment(transform, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&node)),
          node.scene_170, actual_virtual_0c, actual_virtual_50, dispatch_context) {
    if ((node.notification_context_a0 != nullptr) != (actual_notify_changed != nullptr))
        throw std::invalid_argument("native node attachment and notification callback must be bound together");
    // Fresh construction has A0=null, so its caller explicitly passes null.
}
NativeNodeBinding::NativeNodeBinding(NativeNodeStorage& node, NativeNodePreconstructionBinding,
    SceneTypePredicate actual_virtual_0c, SceneAttachOverride actual_virtual_50,
    void* dispatch_context)
    : storage(node), transform(transform_backing(node), nullptr),
      scene_attachment(transform, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&node)),
          node.scene_170, actual_virtual_0c, actual_virtual_50, dispatch_context) {}

} // namespace bsp

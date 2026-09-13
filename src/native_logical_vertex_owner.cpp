#include "bsp/native_logical_vertex_owner.hpp"
#include "bsp/native_logical_vertex_pool_return.hpp"
#include "bsp/native_render_buffer_unregistration.hpp"
#include "bsp/native_shader_device_reset.hpp"

#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native logical vertex owners require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(LONG) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);
void* at(const void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uintptr_t>(value);
}
volatile std::uint32_t& word(const void* base, std::uint32_t offset = 0) noexcept {
    return *static_cast<volatile std::uint32_t*>(at(base, offset));
}
volatile std::uint16_t& half(const void* base, std::uint32_t offset) noexcept {
    return *static_cast<volatile std::uint16_t*>(at(base, offset));
}
void* pointer(const void* base, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(base, offset));
}
std::int32_t signed_bits(std::uint32_t bits) noexcept {
    std::int32_t result;
    std::memcpy(&result, &bits, 4);
    return result;
}
std::atomic<std::int32_t>& references(void* owner) noexcept {
    return *std::launder(static_cast<std::atomic<std::int32_t>*>(at(owner, 4)));
}
void retain(void* owner) noexcept {
    (void)InterlockedIncrement(static_cast<volatile LONG*>(at(owner, 4)));
}
std::uint32_t saved_guard_word(const NativeRendererOptionalGuardStorage& guard) noexcept {
    std::uint32_t result;
    __asm {
        mov eax, guard
        mov eax, dword ptr [eax]
        mov result, eax
    }
    return result;
}
bool enter_guard(NativeRendererOptionalGuardStorage& guard,
    NativeLogicalVertexOwnerContext& context) {
    const bool enabled = context.actual_synchronization_0108d6dc.mode_00 != 0;
    if (enabled) {
        guard.renderer_04 = context.actual_renderer_00f8d394;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(
            guard.renderer_04, context.actual_synchronization_0108d6dc);
    }
    return enabled;
}
void leave_guard(const NativeRendererOptionalGuardStorage& guard,
    NativeLogicalVertexOwnerContext& context, bool entered) {
    if (context.actual_synchronization_0108d6dc.mode_00 != 0) {
        __assume(entered);
        const auto ignored = saved_guard_word(guard);
        leave_native_renderer_optional_guard_00b33b00(guard.renderer_04, ignored,
            context.actual_synchronization_0108d6dc);
    }
}
void unwind_guard(const NativeRendererOptionalGuardStorage& guard,
    NativeLogicalVertexOwnerContext& context) noexcept {
    destroy_native_renderer_optional_guard_00b21110(guard,
        context.actual_synchronization_0108d6dc);
}
void unwind_base(void* owner, NativeLogicalVertexOwnerContext& context) noexcept {
    destroy_native_logical_vertex_base_00b62010(owner, context.actual_owners);
}
void unwind_allocation(void* allocation, NativeLogicalVertexOwnerContext& context) noexcept {
    return_native_logical_vertex_pool_slot_00b49570(
        context.actual_logical_vertex_pool_0108fe18, allocation);
}
void enter_tracked_renderer(void* renderer) {
    auto* const section = static_cast<CRITICAL_SECTION*>(at(renderer, 0x19f4));
    EnterCriticalSection(section);
    word(section, 0x18) = word(section, 0x18) + 1u;
}
void leave_current_tracked_renderer(NativeLogicalVertexOwnerContext& context) {
    void* const renderer = const_cast<void*>(context.actual_renderer_00f8d394);
    word(renderer, 0x1a0c) = word(renderer, 0x1a0c) - 1u;
    LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(at(renderer, 0x19f4)));
}
using Profile = const volatile std::uint32_t*;
Profile physical_profile(void* physical, const NativeLogicalBufferDeviceRestoreProfiles& profiles) noexcept {
    const auto token = word(physical);
    if (token == 0x00d61e34u) return profiles.private_vertex_00d61e34;
    if (token == 0x00d61e7cu) return profiles.pooled_vertex_00d61e7c;
    __assume(0);
}
void release_physical(void* physical, NativeLogicalVertexOwnerContext& context) {
    if (InterlockedDecrement(static_cast<volatile LONG*>(at(physical, 4))) != 0) return;
    const auto invoker = physical_profile(physical, context.actual_physical_profiles)[0];
    __assume(invoker == 0x00bd30e0u);
    const auto terminal = physical_profile(physical, context.actual_physical_profiles)[1];
    if (terminal == 0x00b4bb40u) {
        (void)delete_native_private_vertex_buffer_00b4bb40(physical, 1, context.actual_physical);
        return;
    }
    if (terminal == 0x00b4c230u) {
        (void)delete_native_pooled_vertex_buffer_00b4c230(physical, 1, context.actual_physical);
        return;
    }
    __assume(0);
}
void append_renderer_pointer(void* header, void* value) {
    const auto capacity = word(header, 8);
    if (word(header, 4) == capacity) {
        const auto doubled = capacity * 2u;
        reserve_native_renderer_pointer_array_00b22d10(header,
            signed_bits(doubled) > 1 ? doubled : 1u);
    }
    void* const destination = at(pointer(header), word(header, 4) * 4u);
    if (destination) word(destination) = address(value);
    word(header, 4) = word(header, 4) + 1u;
}
using CreateBuffer = HRESULT (STDMETHODCALLTYPE*)(void*, std::uint32_t,
    std::uint32_t, std::uint32_t, std::uint32_t, void**, void*);
using ComReference = ULONG (STDMETHODCALLTYPE*)(void*);
HRESULT create_private_buffer(void* renderer, void* declaration,
    std::uint32_t count, std::uint32_t usage, std::uint32_t pool, void** output) {
    void* const device = get_native_renderer_device_00b1fef0(renderer);
    const auto stride = word(declaration, 0xcc);
    void* const table = pointer(device);
    const auto create = reinterpret_cast<CreateBuffer>(word(table, 0x68));
    return create(device, stride * count, usage, 0, pool, output, nullptr);
}
void initialize_private_physical(void* physical) noexcept {
    word(physical) = 0x00ceb130u;
    ::new (at(physical, 4)) std::atomic<std::int32_t>(1);
    word(physical, 8) = 0;
    word(physical, 0x0c) = 0;
    word(physical, 0x10) = 0;
    word(physical) = 0x00d61e34u;
    word(physical, 0x14) = 0;
    word(physical, 0x18) = 0;
    word(physical, 0x1c) = 0;
    word(physical, 0x20) = 0;
    word(physical, 0x28) = 0;
    word(physical, 0x24) = 0;
}
} // namespace

bool native_vertex_declaration_has_semantic_00b47c90(const void* declaration,
    std::uint32_t usage, std::uint32_t occurrence) noexcept {
    std::uint32_t matched = 0;
    std::uint32_t index = 0;
    if (signed_bits(word(declaration, 0x10)) > 0) {
        void* cursor = at(pointer(declaration, 0x0c), 0x0c);
        do {
            if (word(cursor) == usage) {
                if (matched == occurrence) return true;
                ++matched;
            }
            ++index;
            cursor = at(cursor, 0x14);
        } while (signed_bits(index) < signed_bits(word(declaration, 0x10)));
    }
    return false;
}
std::uint32_t native_vertex_declaration_find_semantic_00b47ce0(const void* declaration,
    std::uint32_t usage, std::uint32_t occurrence) noexcept {
    std::uint32_t matched = 0;
    std::uint32_t index = 0;
    if (signed_bits(word(declaration, 0x10)) > 0) {
        void* cursor = at(pointer(declaration, 0x0c), 0x0c);
        do {
            if (word(cursor) == usage) {
                if (matched == occurrence) return index;
                ++matched;
            }
            ++index;
            cursor = at(cursor, 0x14);
        } while (signed_bits(index) < signed_bits(word(declaration, 0x10)));
    }
    return 0xffffffffu;
}
std::uint32_t native_vertex_declaration_semantic_offset_00b47c40(const void* declaration,
    std::uint32_t usage, std::uint32_t occurrence) noexcept {
    return word(pointer(declaration, 0x18u + usage * 12u), occurrence * 0x14u);
}
std::uint32_t native_vertex_declaration_semantic_type_00b47c20(const void* declaration,
    std::uint32_t usage, std::uint32_t occurrence) noexcept {
    return word(pointer(declaration, 0x18u + usage * 12u), 4u + occurrence * 0x14u);
}
std::uint32_t native_vertex_declaration_semantic_size_00b47c60(const void* declaration,
    std::uint32_t usage, std::uint32_t occurrence, const volatile std::uint32_t* sizes) noexcept {
    const auto type = native_vertex_declaration_semantic_type_00b47c20(declaration, usage, occurrence);
    return word(const_cast<const std::uint32_t*>(sizes), type * 4u);
}

void* construct_native_logical_vertex_base_00b61e20(void* owner, const void* declaration,
    NativeLogicalVertexOwnerContext& context) noexcept {
    word(owner) = 0x00ceb130u;
    ::new (at(owner, 4)) std::atomic<std::int32_t>(1);
    const auto id = context.actual_next_id_0108fee0;
    word(owner, 0x48) = id;
    context.actual_next_id_0108fee0 = id + 1u;
    word(owner) = 0x00d62b68u;
    word(owner, 0x50) = 0;
    word(owner, 0x10) = 0xffffffffu;
    word(owner, 0x1c) = 0xffffffffu;
    word(owner, 0x28) = 0xffffffffu;
    word(owner, 0x18) = 0;
    word(owner, 0x24) = 0;
    word(owner, 0x30) = 0;
    word(owner, 0x34) = 0xffffffffu;
    word(owner, 0x38) = 0xffffffffu;
    word(owner, 0x3c) = 0xffffffffu;
    word(owner, 0x40) = 0xffffffffu;
    word(owner, 0x44) = 0xffffffffu;
    word(owner, 8) = 0;
    word(owner, 0x4c) = 0;
    word(owner, 0x0c) = word(declaration, 0xcc);
    word(owner, 0x54) = 0x40000000u;
    constexpr std::uint32_t usages[] = {0, 3, 5};
    constexpr std::uint32_t offsets[] = {0x10, 0x1c, 0x28};
    for (std::uint32_t i = 0; i < 3; ++i) {
        if (native_vertex_declaration_has_semantic_00b47c90(declaration, usages[i], 0)) {
            word(owner, offsets[i]) = native_vertex_declaration_semantic_offset_00b47c40(declaration, usages[i], 0);
            word(owner, offsets[i] + 8u) = native_vertex_declaration_find_semantic_00b47ce0(declaration, usages[i], 0);
            word(owner, offsets[i] + 4u) = native_vertex_declaration_semantic_type_00b47c20(declaration, usages[i], 0);
        }
    }
    if (native_vertex_declaration_has_semantic_00b47c90(declaration, 10, 0) &&
        native_vertex_declaration_semantic_size_00b47c60(declaration, 10, 0,
            context.actual_type_sizes_00d61cc0) == 4)
        word(owner, 0x34) = native_vertex_declaration_semantic_offset_00b47c40(declaration, 10, 0);
    return owner;
}
void destroy_native_logical_vertex_base_00b62010(void* owner, NativeRenderActualOwners& owners) {
    word(owner) = 0x00d62b68u;
    void* const retained = pointer(owner, 0x4c);
    try {
        if (retained) {
            release_native_render_actual_owner(owners, retained);
            word(owner, 0x4c) = 0;
        }
        void* const allocation = pointer(owner, 0x50);
        if (allocation) {
            singleton_lifetime_free(allocation);
            word(owner, 0x50) = 0; // Returning-free gap B6206D..B62076.
        }
    } catch (...) {
        word(owner) = 0x00ceb130u;
        throw;
    }
    word(owner) = 0x00ceb130u;
}

void* construct_native_logical_vertex_stream_00b4bc00(void* owner, std::uint32_t count,
    void* declaration, std::uint32_t flags, NativeLogicalVertexOwnerContext& context) {
    construct_native_logical_vertex_base_00b61e20(owner, declaration, context);
    word(owner) = 0x00d61d6cu;
    word(owner, 0x58) = 0;
    word(owner, 0x68) = 0;
    word(owner, 0x6c) = 0;
    word(owner, 0x70) = 0;
    NativeRendererOptionalGuardStorage guard;
    try { // State0: only the base dtor is armed.
        const bool entered = enter_guard(guard, context);
        try { // State1: optional guard then base; no derived-member rollback.
            auto adjusted_flags = flags;
            std::uint32_t pool;
            switch (flags & 0x0fu) {
            case 0: pool = 0; adjusted_flags |= 0x10000u; break;
            case 1: pool = 1; break;
            case 2: pool = 2; break;
            case 3: pool = 3; break;
            default: pool = context.native_pool_stack_bits; break;
            }
            std::uint32_t usage = (adjusted_flags & 0x10u) != 0 ? 1u : 0u;
            switch (adjusted_flags & 0x0f00u) {
            case 0x100: usage |= 2; break;
            case 0x200: usage |= 0x4000; break;
            case 0x300: usage |= 0x40; break;
            case 0x400: usage |= 0x100; break;
            case 0x500: usage |= 0x80; break;
            }
            if ((adjusted_flags & 0xf000u) == 0x1000u) usage |= 0x200;
            if ((adjusted_flags & 0xf0000u) == 0x10000u) usage |= 8;
            void* const renderer = const_cast<void*>(context.actual_renderer_00f8d394);
            if ((flags & 0xf000u) == 0x1000u) {
                // B1FEB0 is an unretained actual+1974 getter.
                void* const physical = pointer(renderer, 0x1974);
                word(owner, 0x58) = address(physical);
                retain(physical);
                enter_tracked_renderer(const_cast<void*>(context.actual_renderer_00f8d394));
                register_native_physical_vertex_stream_00b4b1e0(pointer(owner, 0x58), owner, context);
                leave_current_tracked_renderer(context);
                word(owner, 0x5c) = 0xffffffffu;
            } else {
                void* temporary = nullptr;
                const auto result = create_private_buffer(renderer, declaration, count, usage, pool, &temporary);
                if (!pointer(&temporary) && result != 0 &&
                    static_cast<std::uint32_t>(result) != 0x8876017cu &&
                    static_cast<std::uint32_t>(result) != 0x8007000eu) {
                    context.actual_device_recreation.call_00b29670(
                        const_cast<void*>(context.actual_renderer_00f8d394));
                    (void)create_private_buffer(const_cast<void*>(context.actual_renderer_00f8d394),
                        declaration, count, usage, pool, &temporary);
                }
                void* const physical = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x2c, 0x2c});
                if (physical) initialize_private_physical(physical);
                word(owner, 0x58) = address(physical);
                const auto bytes = word(declaration, 0xcc) * count;
                const auto target = physical_profile(physical, context.actual_physical_profiles)[5];
                __assume(target == 0x00b4c370u);
                attach_native_physical_vertex_buffer_00b4c370(physical,
                    static_cast<IDirect3DVertexBuffer9*>(pointer(&temporary)), flags, bytes, context.actual_physical);
                void* const current_temporary = pointer(&temporary);
                const auto release = reinterpret_cast<ComReference>(word(pointer(current_temporary), 8));
                (void)release(current_temporary);
                word(owner, 0x5c) = 0;
            }
            word(owner, 0x64) = count;
            word(owner, 0x60) = flags;
            void* const previous = pointer(owner, 0x68);
            if (previous != declaration) {
                word(owner, 0x68) = address(declaration);
                if (declaration) retain(declaration);
                if (previous) release_native_render_actual_owner(context.actual_owners, previous);
            }
        } catch (...) {
            unwind_guard(guard, context);
            throw;
        }
        leave_guard(guard, context, entered); // Guard disarmed before leave.
    } catch (...) {
        unwind_base(owner, context);
        throw;
    }
    return owner;
}

void destroy_native_logical_vertex_stream_00b4b5d0(void* owner,
    NativeLogicalVertexOwnerContext& context) {
    word(owner) = 0x00d61d6cu;
    NativeRendererOptionalGuardStorage guard;
    try {
        const bool entered = enter_guard(guard, context);
        const bool dynamic = (word(owner, 0x60) & 0xf000u) == 0x1000u;
        try {
            if (dynamic) {
                enter_tracked_renderer(const_cast<void*>(context.actual_renderer_00f8d394));
                unregister_native_physical_vertex_stream_00b4b3f0(pointer(owner, 0x58),
                    address(owner), context.actual_renderer_00f8d394, context.actual_synchronization_0108d6dc);
                leave_current_tracked_renderer(context);
            } else {
                (void)word(pointer(owner, 0x58), 4);
            }
            (void)unregister_native_renderer_vertex_stream_00b268e0(
                const_cast<void*>(context.actual_renderer_00f8d394), address(owner));
            release_native_render_actual_owner(context.actual_owners, pointer(owner, 0x68));
            release_physical(pointer(owner, 0x58), context);
        } catch (...) {
            unwind_guard(guard, context);
            throw;
        }
        leave_guard(guard, context, entered);
    } catch (...) {
        unwind_base(owner, context);
        throw;
    }
    destroy_native_logical_vertex_base_00b62010(owner, context.actual_owners);
}
void* delete_native_logical_vertex_stream_00b4bf10(void* owner, std::uint32_t flags,
    NativeLogicalVertexOwnerContext& context) {
    destroy_native_logical_vertex_stream_00b4b5d0(owner, context);
    if ((flags & 1u) != 0)
        return_native_logical_vertex_pool_slot_00b49570(context.actual_logical_vertex_pool_0108fe18, owner);
    return owner;
}

void* initialize_native_logical_vertex_slab_00b48eb0(void* slab, std::uint32_t index) noexcept {
    half(slab, 0xf40) = 32;
    for (std::uint32_t slot = 0; slot < 32; ++slot) {
        half(slab, 0xf00u + slot * 2u) = static_cast<std::uint16_t>(31u - slot);
        word(slab, 0x74u + slot * 0x78u) = index;
    }
    return slab;
}
void* allocate_native_logical_vertex_slot_00b4ae80(void* pool) {
    auto* const section = static_cast<CRITICAL_SECTION*>(at(pool, 0x0c));
    EnterCriticalSection(section);
    word(section, 0x18) = word(section, 0x18) + 1u;
    if (word(pool, 0x34) == 0xffffffffu) {
        word(pool, 0x34) = word(pool, 0x2c);
        void* const raw = singleton_lifetime_allocate({SingletonAllocationKind::object, 0xf44, 0xf44});
        void* const slab = raw ? initialize_native_logical_vertex_slab_00b48eb0(raw, word(pool, 0x34)) : nullptr;
        const auto capacity = word(pool, 0x30);
        if (word(pool, 0x2c) == capacity) {
            const auto grown = capacity * 2u + 2u;
            word(pool, 0x30) = grown;
            const auto bytes = grown * 4u;
            void* const replacement = singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, bytes, bytes});
            void* destination = replacement;
            for (std::uint32_t index = 0; index < word(pool, 0x2c); ++index) {
                if (destination) word(destination) = word(pointer(pool, 0x28), index * 4u);
                destination = at(destination, 4);
            }
            void* const old = pointer(pool, 0x28);
            if (old) singleton_lifetime_free(old);
            word(pool, 0x28) = address(replacement);
        }
        void* const destination = at(pointer(pool, 0x28), word(pool, 0x2c) * 4u);
        if (destination) word(destination) = address(slab);
        word(pool, 0x2c) = word(pool, 0x2c) + 1u;
    }
    void* const slab = pointer(pointer(pool, 0x28), word(pool, 0x34) * 4u);
    half(slab, 0xf40) = static_cast<std::uint16_t>(half(slab, 0xf40) - 1u);
    const auto remaining = half(slab, 0xf40);
    const auto index = half(slab, 0xf00u + static_cast<std::uint32_t>(remaining) * 2u);
    void* const result = at(slab, static_cast<std::uint32_t>(index) * 0x78u);
    if (remaining == 0) {
        auto next = word(pool, 0x34) + 1u;
        const bool more = next < word(pool, 0x2c);
        word(pool, 0x34) = 0xffffffffu;
        if (more) {
            void* cursor = at(pointer(pool, 0x28), next * 4u);
            for (;;) {
                if (half(pointer(cursor), 0xf40) != 0) {
                    word(pool, 0x34) = next;
                    break;
                }
                ++next;
                cursor = at(cursor, 4);
                if (next >= word(pool, 0x2c)) break;
            }
        }
    }
    word(section, 0x18) = word(section, 0x18) - 1u;
    LeaveCriticalSection(section);
    return result;
}
void* allocate_native_logical_vertex_stream_00b4b370(void* pool) {
    return allocate_native_logical_vertex_slot_00b4ae80(pool);
}
void reserve_native_renderer_pointer_array_00b22d10(void* header, std::uint32_t requested) {
    if (signed_bits(requested) < 1) requested = 1;
    if (signed_bits(word(header, 8)) >= signed_bits(requested)) return;
    const auto bytes = requested * 4u;
    void* const replacement = singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, bytes, bytes});
    void* destination = replacement;
    for (std::uint32_t index = 0; signed_bits(index) < signed_bits(word(header, 4)); ++index) {
        if (destination) word(destination) = word(pointer(header), index * 4u);
        destination = at(destination, 4);
    }
    singleton_lifetime_free(pointer(header));
    word(header) = address(replacement);
    word(header, 8) = requested;
}
void register_native_physical_vertex_stream_00b4b1e0(void* physical, void* stream,
    NativeLogicalVertexOwnerContext& context) {
    NativeRendererOptionalGuardStorage guard;
    const bool entered = enter_guard(guard, context);
    void* const data = pointer(physical, 8);
    void* const end = at(data, word(physical, 0x0c) * 4u);
    try {
        void* cursor = data;
        bool found = false;
        while (address(cursor) < address(end)) {
            if (word(cursor) == address(stream)) {
                found = (signed_bits(address(cursor) - address(data)) >> 2) != -1;
                break;
            }
            cursor = at(cursor, 4);
        }
        if (!found) {
            const auto capacity = word(physical, 0x10);
            if (word(physical, 0x0c) == capacity) {
                const auto doubled = capacity * 2u;
                reserve_native_physical_buffer_pointer_array_00b496a0(at(physical, 8),
                    signed_bits(doubled) > 1 ? doubled : 1u);
            }
            void* const destination = at(pointer(physical, 8), word(physical, 0x0c) * 4u);
            if (destination) word(destination) = address(stream);
            word(physical, 0x0c) = word(physical, 0x0c) + 1u;
        }
    } catch (...) {
        unwind_guard(guard, context);
        throw;
    }
    leave_guard(guard, context, entered);
}
std::uint8_t native_renderer_secondary_stream_registration_00b1fe50(const void* renderer) noexcept {
    return *static_cast<const volatile std::uint8_t*>(at(renderer, 0x19ac));
}
void* create_native_registered_vertex_stream_00b287c0(void* renderer,
    std::uint32_t count, std::uint32_t flags, void* declaration,
    NativeLogicalVertexOwnerContext& context, void** acquired_before_registration) {
    void* const allocation = allocate_native_logical_vertex_stream_00b4b370(
        context.actual_logical_vertex_pool_0108fe18);
    void* owner = nullptr;
    try {
        if (allocation) owner = construct_native_logical_vertex_stream_00b4bc00(
            allocation, count, declaration, flags, context);
    } catch (...) {
        // Original factory funclet CBD230 tail-jumps full B49960, which pushes
        // captured allocation then calls B49570 on the actual108FE18 pool.
        unwind_allocation(allocation, context);
        throw;
    }
    if (acquired_before_registration) *acquired_before_registration = owner;
    append_renderer_pointer(at(renderer, 0x1aac), owner);
    const auto renderer_token = word(renderer);
    __assume(renderer_token == 0x00d5f0a8u);
    const auto target = context.actual_renderer_profile_00d5f0a8[0x58 / 4];
    __assume(target == 0x00b1fe50u);
    if (native_renderer_secondary_stream_registration_00b1fe50(renderer) == 1)
        append_renderer_pointer(at(renderer, 0x19b0), owner);
    return owner;
}

NativeLogicalVertexReference::NativeLogicalVertexReference(void* stream,
    NativeLogicalVertexOwnerContext& context, NativeLogicalVertexCompanionDisposal disposal)
    : RenderCommandReference(references(stream)), actual_stream_(stream), context_(context), disposal_(disposal) {
    if (!disposal.retire || reference_count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("logical vertex reference requires live actual storage and retirement");
    require_current_profile();
}
NativeLogicalVertexReference::~NativeLogicalVertexReference() {
    if (phase_ != Phase::retired) std::terminate();
}
void NativeLogicalVertexReference::require_current_profile() const noexcept {
    if (word(actual_stream_) != 0x00d61d6cu || !context_.actual_logical_profile_00d61d6c ||
        context_.actual_logical_profile_00d61d6c[0] != 0x00bd30e0u ||
        context_.actual_logical_profile_00d61d6c[1] != 0x00b4bf10u) std::terminate();
}
void NativeLogicalVertexReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound) std::terminate();
    require_current_profile();
    phase_ = Phase::destroying;
    const auto disposal = disposal_;
    (void)delete_native_logical_vertex_stream_00b4bf10(actual_stream_, 1, context_);
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this);
}
} // namespace bsp

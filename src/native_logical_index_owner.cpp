#include "bsp/native_logical_index_owner.hpp"
#include "bsp/native_render_buffer_unregistration.hpp"
#include "bsp/native_logical_vertex_owner.hpp"
#include "bsp/native_shader_device_reset.hpp"

#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native logical-index owners require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(LONG) == 4);
static_assert(sizeof(NativeRendererOptionalGuardStorage) == 8);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);

void* at(const void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
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
std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uintptr_t>(value);
}
std::atomic<std::int32_t>& references(void* owner) noexcept {
    return *std::launder(static_cast<std::atomic<std::int32_t>*>(at(owner, 4)));
}
std::int32_t signed_bits(std::uint32_t bits) noexcept {
    std::int32_t result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
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
void destroy_base(void* logical) noexcept { word(logical) = 0x00ceb130u; }

const volatile std::uint32_t* profile_view(std::uint32_t profile,
    const NativeLogicalIndexPhysicalProfiles& profiles) noexcept {
    if (profile == 0x00d61e10u) return profiles.private_index_00d61e10;
    if (profile == 0x00d61e58u) return profiles.pooled_index_00d61e58;
    __assume(0); // No arbitrary profile or callback fallback.
}
void invoke_physical_index_deleting_slot(void* physical,
    NativeLogicalIndexOwnerContext& context) {
    // Both observed immutable profiles have BD30E0 at slot0. That invoker
    // rereads the current physical profile before loading its deleting slot.
    const auto* const invoker_table = profile_view(word(physical), context.actual_physical_profiles);
    const auto invoker = invoker_table[0];
    __assume(invoker == 0x00bd30e0u);
    const auto* const terminal_table = profile_view(word(physical), context.actual_physical_profiles);
    const auto terminal = terminal_table[1];
    if (terminal == 0x00b4bb20u) {
        (void)delete_native_private_index_buffer_00b4bb20(physical, 1, context.actual_physical);
        return;
    }
    if (terminal == 0x00b4c210u) {
        (void)delete_native_pooled_index_buffer_00b4c210(physical, 1, context.actual_physical);
        return;
    }
    __assume(0); // Other physical profiles have no recovered terminal contract.
}
void unwind_guard(const NativeRendererOptionalGuardStorage& guard,
    NativeRendererSynchronizationGlobals& globals) noexcept {
    // Original FH3 invokes B21110 as an unwind action: another exception here
    // terminates instead of replacing the exception already being unwound.
    destroy_native_renderer_optional_guard_00b21110(guard, globals);
}
void leave_guard(const NativeRendererOptionalGuardStorage& guard,
    NativeRendererSynchronizationGlobals& globals, bool entry_enabled) {
    if (globals.mode_00 != 0) {
        __assume(entry_enabled);
        // Preserve the native whole-DWORD load, including padding, through an
        // isolated MOV. B33B00 ignores it; no C++ indeterminate scalar is read.
        const std::uint32_t ignored = saved_guard_word(guard);
        const void* const retained_renderer = guard.renderer_04;
        leave_native_renderer_optional_guard_00b33b00(retained_renderer,
            ignored, globals);
    }
}
std::uint32_t index_width(std::uint32_t format) noexcept {
    if (format == 0x65u) return 2;
    if (format == 0x66u) return 4;
    return 0; // Native format branch, not an unsupported-profile success default.
}
using CreateIndexBuffer = HRESULT (STDMETHODCALLTYPE*)(void*, std::uint32_t,
    std::uint32_t, std::uint32_t, std::uint32_t, void**, void*);
using ComRelease = ULONG (STDMETHODCALLTYPE*)(void*);
HRESULT create_private_index_buffer(void* renderer, std::uint32_t count,
    std::uint32_t format, std::uint32_t usage, std::uint32_t pool, void** output) {
    void* const device = get_native_renderer_device_00b1fef0(renderer);
    const auto bytes = index_width(format) * count;
    const auto create = reinterpret_cast<CreateIndexBuffer>(word(pointer(device), 0x6c));
    return create(device, bytes, usage, format, pool, output, nullptr);
}
void initialize_private_index_physical(void* physical) noexcept {
    word(physical) = 0x00ceb130u;
    ::new (at(physical, 4)) std::atomic<std::int32_t>(1);
    word(physical, 8) = 0;
    word(physical, 0x0c) = 0;
    word(physical, 0x10) = 0;
    word(physical) = 0x00d61e10u;
    word(physical, 0x14) = 0;
    word(physical, 0x18) = 0;
    word(physical, 0x1c) = 0;
    word(physical, 0x20) = 0;
    word(physical, 0x28) = 0;
    word(physical, 0x24) = 0;
}
void append_renderer_index_pointer(void* header, void* value) {
    const auto capacity = word(header, 8);
    if (word(header, 4) == capacity) {
        const auto doubled = capacity * 2u;
        reserve_native_renderer_index_pointer_array_00b22d70(header,
            signed_bits(doubled) > 1 ? doubled : 1u);
    }
    void* const destination = at(pointer(header), word(header, 4) * 4u);
    if (destination) word(destination) = address(value);
    word(header, 4) = word(header, 4) + 1u;
}
void unwind_index_allocation(void* allocation, NativeLogicalIndexOwnerContext& context) noexcept {
    // CBD250 -> B49970 -> B495E0. A second exception during FH3 cleanup terminates.
    return_native_logical_index_slot_00b495e0(context.actual_logical_index_pool_0108fe50, allocation);
}
} // namespace

void* construct_native_logical_index_stream_00b4bf30(void* logical, std::uint32_t count,
    std::uint32_t format, std::uint32_t flags, NativeLogicalIndexCreationContext& creation) {
    auto& context = creation.lifetime;
    auto& globals = context.actual_synchronization_0108d6dc;
    word(logical) = 0x00ceb130u;
    ::new (at(logical, 4)) std::atomic<std::int32_t>(1);
    word(logical) = 0x00d61de0u;
    word(logical, 8) = 0;
    word(logical, 0x1c) = 0;
    word(logical, 0x20) = 0;
    NativeRendererOptionalGuardStorage guard;
    try { // State0: CBF9C0 -> B49420 -> BD30F0 base cleanup.
        const bool entry_enabled = globals.mode_00 != 0;
        if (entry_enabled) {
            guard.renderer_04 = context.actual_renderer_00f8d394;
            guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(guard.renderer_04, globals);
        }
        try { // State1: CBF9C8 -> B21110 optional guard, then state0 base.
            auto adjusted_flags = flags;
            std::uint32_t pool;
            switch (flags & 0x0fu) {
            case 0: pool = 0; adjusted_flags |= 0x10000u; break;
            case 1: pool = 1; break;
            case 2: pool = 2; break;
            case 3: pool = 3; break;
            default: pool = creation.native_pool_stack_bits; break;
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
            void* temporary = nullptr;
            const auto result = create_private_index_buffer(
                const_cast<void*>(context.actual_renderer_00f8d394), count, format, usage, pool, &temporary);
            if (!pointer(&temporary) && result != 0 &&
                static_cast<std::uint32_t>(result) != 0x8876017cu &&
                static_cast<std::uint32_t>(result) != 0x8007000eu) {
                creation.actual_device_recreation.call_00b29670(
                    const_cast<void*>(context.actual_renderer_00f8d394));
                (void)create_private_index_buffer(const_cast<void*>(context.actual_renderer_00f8d394),
                    count, format, usage, pool, &temporary);
            }
            void* const physical = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x2c, 0x2c});
            if (physical) initialize_private_index_physical(physical);
            word(logical, 8) = address(physical);
            const auto bytes = index_width(format) * count;
            const auto physical_token = word(physical);
            const auto* const physical_profile = context.actual_physical_profiles.private_index_00d61e10;
            if (physical_token != 0x00d61e10u || !physical_profile || physical_profile[5] != 0x00b4c250u)
                throw std::logic_error("index construction requires the actual private current14 Attach");
            attach_native_physical_index_buffer_00b4c250(physical,
                static_cast<IDirect3DIndexBuffer9*>(pointer(&temporary)), flags, bytes, context.actual_physical);
            void* const current_temporary = pointer(&temporary);
            const auto release = reinterpret_cast<ComRelease>(word(pointer(current_temporary), 8));
            (void)release(current_temporary); // Native null result still faults; no repair.
            word(logical, 0x0c) = 0;
            word(logical, 0x10) = flags;
            word(logical, 0x14) = count;
            word(logical, 0x18) = format;
        } catch (...) {
            unwind_guard(guard, globals);
            throw;
        }
        leave_guard(guard, globals, entry_enabled); // State0 before normal leave.
    } catch (...) {
        destroy_base(logical); // No invented physical/COM rollback.
        throw;
    }
    return logical;
}

void destroy_native_logical_index_stream_00b4b6f0(void* logical,
    NativeLogicalIndexOwnerContext& context) {
    word(logical) = 0x00d61de0u;
    auto& globals = context.actual_synchronization_0108d6dc;
    const bool entry_enabled = globals.mode_00 != 0;
    NativeRendererOptionalGuardStorage guard;
    try { // Native state0: base cleanup is active before optional entry.
        if (entry_enabled) {
            guard.renderer_04 = context.actual_renderer_00f8d394;
            guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(
                guard.renderer_04, globals);
        }
        const auto flags = word(logical, 0x10);
        void* const captured_physical = pointer(logical, 8);
        try { // Native state1 begins only after these two storage reads.
            if ((flags & 0xf000u) == 0x1000u) {
                unregister_native_physical_index_stream_00b4b390(captured_physical,
                    static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(logical)),
                    context.actual_renderer_00f8d394, globals);
            } else {
                (void)word(captured_physical, 4); // Native otherwise-unused MOV.
            }
            (void)unregister_native_renderer_index_stream_00b26900(
                const_cast<void*>(context.actual_renderer_00f8d394),
                static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(logical)));
            void* const current_physical = pointer(logical, 8);
            auto* const references = static_cast<volatile LONG*>(at(current_physical, 4));
            if (InterlockedDecrement(references) == 0)
                invoke_physical_index_deleting_slot(current_physical, context);
        } catch (...) {
            unwind_guard(guard, globals);
            throw;
        }
        leave_guard(guard, globals, entry_enabled); // State0: guard disarmed.
    } catch (...) {
        destroy_base(logical);
        throw;
    }
    destroy_base(logical); // State-1 before normal base destruction.
}

void* delete_native_pooled_logical_index_stream_00b4c1f0(void* logical,
    std::uint32_t flags, NativeLogicalIndexOwnerContext& context) {
    destroy_native_logical_index_stream_00b4b6f0(logical, context);
    if ((flags & 1u) != 0)
        return_native_logical_index_slot_00b495e0(
            context.actual_logical_index_pool_0108fe50, logical);
    return logical;
}

void return_native_logical_index_slot_00b495e0(void* pool, void* slot) {
    auto* const section = static_cast<CRITICAL_SECTION*>(at(pool, 0x0c));
    EnterCriticalSection(section);
    word(section, 0x18) = word(section, 0x18) + 1u;
    const auto slab_index = word(slot, 0x24);
    void* const slabs = pointer(pool, 0x28);
    void* const slab = pointer(slabs, slab_index * 4u);
    const auto delta = reinterpret_cast<std::uintptr_t>(slot) -
        reinterpret_cast<std::uintptr_t>(slab);
    const auto index = signed_bits(delta) / 0x28;
    const auto old_free_count = half(slab, 0x540);
    half(slab, 0x500u + static_cast<std::uint32_t>(old_free_count) * 2u) =
        static_cast<std::uint16_t>(index);
    half(slab, 0x540) = static_cast<std::uint16_t>(half(slab, 0x540) + 1u);
    if (slab_index < word(pool, 0x34)) word(pool, 0x34) = slab_index;
    word(section, 0x18) = word(section, 0x18) - 1u;
    LeaveCriticalSection(section);
}

void* initialize_native_logical_index_slab_00b48f70(void* slab, std::uint32_t index) noexcept {
    half(slab, 0x540) = 32;
    for (std::uint32_t slot = 0; slot < 32; ++slot) {
        half(slab, 0x500u + slot * 2u) = static_cast<std::uint16_t>(31u - slot);
        word(slab, 0x24u + slot * 0x28u) = index;
    }
    return slab;
}
void* allocate_native_logical_index_slot_00b4b0a0(void* pool) {
    auto* const section = static_cast<CRITICAL_SECTION*>(at(pool, 0x0c));
    EnterCriticalSection(section);
    word(section, 0x18) = word(section, 0x18) + 1u;
    if (word(pool, 0x34) == 0xffffffffu) {
        word(pool, 0x34) = word(pool, 0x2c);
        void* const raw = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x544, 0x544});
        void* const slab = raw ? initialize_native_logical_index_slab_00b48f70(raw, word(pool, 0x34)) : nullptr;
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
            // Native B4B136 is ADD ESP4, then B4B139 publishes this pointer.
            word(pool, 0x28) = address(replacement);
        }
        void* const destination = at(pointer(pool, 0x28), word(pool, 0x2c) * 4u);
        if (destination) word(destination) = address(slab);
        word(pool, 0x2c) = word(pool, 0x2c) + 1u;
    }
    void* const slab = pointer(pointer(pool, 0x28), word(pool, 0x34) * 4u);
    half(slab, 0x540) = static_cast<std::uint16_t>(half(slab, 0x540) - 1u);
    const auto remaining = half(slab, 0x540);
    const auto index = half(slab, 0x500u + static_cast<std::uint32_t>(remaining) * 2u);
    void* const result = at(slab, static_cast<std::uint32_t>(index) * 0x28u);
    if (remaining == 0) {
        auto next = word(pool, 0x34) + 1u;
        const bool more = next < word(pool, 0x2c);
        word(pool, 0x34) = 0xffffffffu;
        if (more) {
            void* cursor = at(pointer(pool, 0x28), next * 4u);
            for (;;) {
                if (half(pointer(cursor), 0x540) != 0) {
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
void* allocate_native_logical_index_stream_00b4b380(void* pool) {
    return allocate_native_logical_index_slot_00b4b0a0(pool);
}
void reserve_native_renderer_index_pointer_array_00b22d70(void* header, std::uint32_t requested) {
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
    // B22DC1..B22DC9: returning-free stack cleanup, pointer and capacity stores.
    word(header) = address(replacement);
    word(header, 8) = requested;
}
void* create_native_registered_index_stream_00b288b0(void* renderer,
    std::uint32_t count, std::uint32_t flags, std::uint32_t format,
    NativeLogicalIndexCreationContext& context, void** publication) {
    if (publication && *publication)
        throw std::invalid_argument("index factory creator publication must begin empty");
    void* const allocation = allocate_native_logical_index_stream_00b4b380(
        context.lifetime.actual_logical_index_pool_0108fe50);
    void* owner = nullptr;
    try {
        if (allocation) owner = construct_native_logical_index_stream_00b4bf30(
            allocation, count, format, flags, context);
    } catch (...) {
        unwind_index_allocation(allocation, context.lifetime);
        throw;
    }
    if (publication) *publication = owner; // Host creator bookkeeping, no AddRef.
    append_renderer_index_pointer(at(renderer, 0x1ab8), owner);
    const auto renderer_token = word(renderer);
    if (renderer_token != 0x00d5f0a8u || !context.actual_renderer_profile_00d5f0a8 ||
        context.actual_renderer_profile_00d5f0a8[0x58 / 4] != 0x00b1fe50u)
        throw std::logic_error("index registration requires the actual renderer current58 profile");
    if (native_renderer_secondary_stream_registration_00b1fe50(renderer) == 1)
        append_renderer_index_pointer(at(renderer, 0x19c4), owner);
    return owner;
}

NativeLogicalIndexReference::NativeLogicalIndexReference(void* stream,
    NativeLogicalIndexCreationContext& context, NativeLogicalIndexCompanionDisposal disposal)
    : RenderCommandReference(references(stream)), actual_stream_(stream), context_(context), disposal_(disposal) {
    if (!disposal.retire || reference_count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("logical index reference requires live actual storage and retirement");
    require_current_profile();
}
NativeLogicalIndexReference::~NativeLogicalIndexReference() {
    if (phase_ != Phase::retired) std::terminate();
}
void NativeLogicalIndexReference::require_current_profile() const noexcept {
    if (word(actual_stream_) != 0x00d61de0u || !context_.actual_logical_profile_00d61de0 ||
        context_.actual_logical_profile_00d61de0[0] != 0x00bd30e0u ||
        context_.actual_logical_profile_00d61de0[1] != 0x00b4c1f0u) std::terminate();
}
void NativeLogicalIndexReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound) std::terminate();
    require_current_profile();
    phase_ = Phase::destroying;
    const auto disposal = disposal_;
    (void)delete_native_pooled_logical_index_stream_00b4c1f0(actual_stream_, 1, context_.lifetime);
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this);
}
} // namespace bsp

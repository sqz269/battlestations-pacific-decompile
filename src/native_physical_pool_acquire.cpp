#include "bsp/native_physical_pool_acquire.hpp"
#include "bsp/native_physical_buffer_owner.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native physical pool acquisition requires MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
volatile std::uint32_t& word(void* base, std::uint32_t offset = 0) noexcept {
    return *static_cast<volatile std::uint32_t*>(at(base, offset));
}
volatile std::uint16_t& half(void* base, std::uint32_t offset) noexcept {
    return *static_cast<volatile std::uint16_t*>(at(base, offset));
}
void* pointer(void* base, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(base, offset));
}
} // namespace

__declspec(naked) void* __fastcall initialize_native_physical_buffer_slab_00b48df0(
    void*, void*, std::uint32_t) noexcept {
    __asm {
        push ebx
        mov ebx, dword ptr [esp + 8]
        push esi
        mov eax, ecx
        push edi
        mov word ptr [eax + 640h], 20h
        xor edx, edx
        lea edi, [eax + 2ch]
        lea esi, [eax + 600h]
        _emit 0x8d // Original three-byte LEA ECX,[ECX+0] alignment instruction.
        _emit 0x49
        _emit 0x00
    initialize_slot:
        mov ecx, 1fh
        sub ecx, edx
        mov word ptr [esi], cx
        mov dword ptr [edi], ebx
        add edx, 1
        add esi, 2
        add edi, 30h
        cmp edx, 20h
        jl initialize_slot
        pop edi
        pop esi
        pop ebx
        ret 4
    }
}

void* __fastcall acquire_native_physical_buffer_slot_00b4ac60(void* pool) {
    auto* const section = static_cast<CRITICAL_SECTION*>(at(pool, 0x0c));
    EnterCriticalSection(section);
    word(section, 0x18) = word(section, 0x18) + 1u;
    if (word(pool, 0x34) == 0xffffffffu) {
        word(pool, 0x34) = word(pool, 0x2c); // Published before slab allocation.
        void* slab = singleton_lifetime_allocate(
            {SingletonAllocationKind::object, 0x644, 0x644});
        if (slab != nullptr) {
            const auto current_slab_index = word(pool, 0x34);
            slab = initialize_native_physical_buffer_slab_00b48df0(
                slab, nullptr, current_slab_index);
        }
        const auto capacity = word(pool, 0x30);
        if (word(pool, 0x2c) == capacity) {
            const auto new_capacity = capacity + capacity + 2u;
            const auto bytes = new_capacity * 4u;
            word(pool, 0x30) = new_capacity; // Publication precedes allocation.
            void* const replacement = singleton_lifetime_allocate(
                {SingletonAllocationKind::object, bytes, bytes});
            std::uint32_t index = 0;
            void* destination = replacement;
            while (index < word(pool, 0x2c)) {
                if (destination != nullptr) {
                    void* const current_base = pointer(pool, 0x28);
                    word(destination) = word(current_base, index * 4u);
                }
                ++index;
                destination = at(destination, 4);
            }
            void* const old_base = pointer(pool, 0x28);
            if (old_base != nullptr) singleton_lifetime_free(old_base);
            // Native B4ACF6 ADD ESP4 and this post-free publication are omitted
            // by the old saved no-return call-site override; full bytes prove it.
            word(pool, 0x28) = reinterpret_cast<std::uintptr_t>(replacement);
        }
        const auto count = word(pool, 0x2c);
        void* const current_base = pointer(pool, 0x28);
        void* const destination = at(current_base, count * 4u);
        if (destination != nullptr)
            word(destination) = reinterpret_cast<std::uintptr_t>(slab);
        word(pool, 0x2c) = word(pool, 0x2c) + 1u;
    }

    const auto current_slab_index = word(pool, 0x34);
    void* const current_base = pointer(pool, 0x28);
    void* const slab = pointer(current_base, current_slab_index * 4u);
    half(slab, 0x640) = static_cast<std::uint16_t>(half(slab, 0x640) - 1u);
    const auto free_count = half(slab, 0x640);
    const auto slot_index = half(slab, 0x600u + static_cast<std::uint32_t>(free_count) * 2u);
    void* const slot = at(slab, static_cast<std::uint32_t>(slot_index) * 0x30u);
    if (free_count == 0) {
        auto index = word(pool, 0x34) + 1u;
        const bool has_remaining = index < word(pool, 0x2c);
        word(pool, 0x34) = 0xffffffffu;
        if (has_remaining) {
            void* entry = at(pointer(pool, 0x28), index * 4u);
            for (;;) {
                if (half(pointer(entry), 0x640) != 0) {
                    word(pool, 0x34) = index;
                    break;
                }
                ++index;
                entry = at(entry, 4);
                if (index >= word(pool, 0x2c)) break;
            }
        }
    }
    word(section, 0x18) = word(section, 0x18) - 1u;
    LeaveCriticalSection(section);
    return slot;
}

void* __fastcall acquire_native_index_buffer_slot_00b4b350(
    const NativePhysicalBufferOwnerContext& context) {
    return acquire_native_physical_buffer_slot_00b4ac60(context.actual_index_pool_0108fda8);
}
void* __fastcall acquire_native_vertex_buffer_slot_00b4b360(
    const NativePhysicalBufferOwnerContext& context) {
    return acquire_native_physical_buffer_slot_00b4ac60(context.actual_vertex_pool_0108fde0);
}
} // namespace bsp

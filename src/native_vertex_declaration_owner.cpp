#include "bsp/native_vertex_declaration_owner.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native vertex declaration owners require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 24);
constexpr std::uint32_t declaration_profile = 0x00d61d1c;
constexpr std::uint32_t base_profile = 0x00ceb130;

// Byte-addressed loads/stores preserve real header aliases and native store
// order without imposing a C++ owner/header overlay on supplied storage.
std::uint32_t word(const volatile void* base, std::uint32_t byte_offset = 0) noexcept {
    std::uint32_t result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
void put(void* base, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
std::uint16_t half(const void* base, std::uint32_t byte_offset) noexcept {
    std::uint16_t result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ax, word ptr [eax + edx]
        mov result, ax
    }
    return result;
}
void put_half(void* base, std::uint32_t byte_offset, std::uint16_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov cx, value
        mov word ptr [eax + edx], cx
    }
}
std::int32_t signed_word(std::uint32_t bits) noexcept {
    std::int32_t result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}
void* at(const void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
void* pointer(std::uint32_t bits) noexcept { return reinterpret_cast<void*>(bits); }

// BF7C10 __ArrayUnwind: when C++ cleanup throws during an active unwind the
// native filter terminates. noexcept supplies that existing C++ boundary.
void unwind_usage_prefix(void* owner, unsigned count) noexcept {
    while (count != 0) {
        --count;
        destroy_native_vertex_elements_00b48ad0(at(owner, 0x18u + count * 12u));
    }
}
void initialize_usage_arrays(void* owner) {
    unsigned initialized = 0;
    try {
        for (; initialized != 15; ++initialized)
            initialize_native_vertex_elements_00b47910(at(owner, 0x18u + initialized * 12u));
    } catch (...) {
        unwind_usage_prefix(owner, initialized);
        throw;
    }
}
void destroy_usage_arrays(void* owner) {
    unsigned remaining = 15;
    try {
        while (remaining != 0) {
            --remaining; // BF7C6E decrements before entering the destructor.
            destroy_native_vertex_elements_00b48ad0(at(owner, 0x18u + remaining * 12u));
        }
    } catch (...) {
        // Do not retry the failed element; only the remaining lower prefix.
        unwind_usage_prefix(owner, remaining);
        throw;
    }
}
void unwind_owner(void* owner, unsigned state) noexcept {
    if (state >= 2) destroy_usage_arrays(owner);
    if (state >= 1) destroy_native_vertex_elements_00b48ad0(at(owner, 0x0c));
    put(owner, 0, base_profile);
}
} // namespace

void* initialize_native_vertex_elements_00b47910(void* header) noexcept {
    put(header, 0, 0);
    put(header, 4, 0);
    put(header, 8, 0);
    return header;
}

void reserve_native_vertex_elements_00b47a30(void* header, std::int32_t capacity) {
    if (capacity < 1) capacity = 1;
    if (signed_word(word(header, 8)) >= capacity) return;
    const auto bytes = static_cast<std::uint32_t>(capacity) * 20u;
    auto* const replacement = singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, bytes, bytes});
    auto* destination = replacement;
    std::uint32_t offset = 0;
    std::uint32_t index = 0;
    while (signed_word(index) < signed_word(word(header, 4))) {
        if (destination) {
            auto* const source = pointer(word(header) + offset);
            put(destination, 0, word(source));
            put(destination, 4, word(source, 4));
            put(destination, 8, word(source, 8));
            put(destination, 12, word(source, 12));
            put(destination, 16, word(source, 16));
        }
        ++index;
        offset += 20u;
        destination = at(destination, 20u);
    }
    // B47AAF..B47AB8 is the returning-free continuation missing from the old
    // listing. Read current old data, free unconditionally, then publish only
    // the captured replacement and requested capacity. Leave count untouched.
    singleton_lifetime_free(pointer(word(header)));
    put(header, 0, reinterpret_cast<std::uintptr_t>(replacement));
    put(header, 8, static_cast<std::uint32_t>(capacity));
}

void resize_native_vertex_elements_00b480f0(void* header, std::int32_t count) {
    if (count > signed_word(word(header, 8))) reserve_native_vertex_elements_00b47a30(header, count);
    const auto old_count = word(header, 4);
    if (signed_word(old_count) < count) {
        auto offset = old_count * 20u;
        auto remaining = static_cast<std::uint32_t>(count) - old_count;
        do {
            if (auto* const destination = pointer(word(header) + offset)) {
                put(destination, 4, 0x11);
                put(destination, 8, 0);
                put(destination, 12, 0x0e);
                put(destination, 0, 0xffffffffu);
                put(destination, 16, 0xffffffffu);
            }
            offset += 20u;
        } while (--remaining != 0);
    }
    while (count < signed_word(word(header, 4))) put(header, 4, word(header, 4) - 1u);
    put(header, 4, static_cast<std::uint32_t>(count));
}

void destroy_native_vertex_elements_00b48ad0(void* header) {
    resize_native_vertex_elements_00b480f0(header, 0);
    singleton_lifetime_free(pointer(word(header)));
}

void* construct_native_vertex_declaration_00b48af0(void* owner) {
    put(owner, 0, base_profile);
    put(owner, 4, 1);
    put(owner, 0, declaration_profile);
    put(owner, 8, 0);
    unsigned state = 0;
    try {
        initialize_native_vertex_elements_00b47910(at(owner, 0x0c));
        state = 1;
        initialize_usage_arrays(owner);
        put(owner, 0xcc, 0);
    } catch (...) {
        // Constructor states1->0 destroy main then restore base. The iterator
        // itself already destroys the initialized usage prefix in reverse.
        unwind_owner(owner, state);
        throw;
    }
    return owner;
}

void clear_native_vertex_declaration_00b488e0(
    void* owner, const volatile std::uint32_t* type_sizes) {
    resize_native_vertex_elements_00b480f0(at(owner, 0x0c), 0);
    for (unsigned index = 0; index != 15; ++index)
        resize_native_vertex_elements_00b480f0(at(owner, 0x18u + index * 12u), 0);
    const auto initial_count = signed_word(word(owner, 0x10));
    put(owner, 0xcc, 0);
    if (initial_count > 0) {
        auto* cursor = pointer(word(owner, 0x0c) + 4u);
        std::uint32_t index = 0;
        do {
            const auto size = word(type_sizes, word(cursor) * 4u);
            put(owner, 0xcc, word(owner, 0xcc) + size);
            ++index;
            cursor = at(cursor, 20u);
        } while (signed_word(index) < signed_word(word(owner, 0x10)));
    }
}

void destroy_native_vertex_declaration_00b48b70(
    void* owner, const volatile std::uint32_t* type_sizes) {
    put(owner, 0, declaration_profile);
    unsigned state = 2;
    try {
        clear_native_vertex_declaration_00b488e0(owner, type_sizes);
        state = 1;
        destroy_usage_arrays(owner);
        state = 0;
        destroy_native_vertex_elements_00b48ad0(at(owner, 0x0c));
    } catch (...) {
        unwind_owner(owner, state);
        throw;
    }
    put(owner, 0, base_profile);
}

void* delete_native_vertex_declaration_00b48ca0(
    void* owner, std::uint32_t flags, void* pool, const volatile std::uint32_t* type_sizes) {
    destroy_native_vertex_declaration_00b48b70(owner, type_sizes);
    if ((flags & 1u) != 0) return_native_vertex_declaration_slot_00b47950(pool, owner);
    return owner;
}

void return_native_vertex_declaration_slot_00b47950(void* pool, void* slot) {
    auto* const critical_section = reinterpret_cast<CRITICAL_SECTION*>(at(pool, 0x0c));
    EnterCriticalSection(critical_section);
    put(pool, 0x24, word(pool, 0x24) + 1u);
    const auto slab_index = word(slot, 0xd0);
    auto* const slab = pointer(word(pointer(word(pool, 0x28)), slab_index * 4u));
    const auto offset = signed_word(static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(slot) - reinterpret_cast<std::uintptr_t>(slab)));
    const auto slot_index = static_cast<std::uint16_t>(offset / 0xd4);
    const auto free_count = half(slab, 0x1ac0);
    put_half(slab, 0x1a80u + static_cast<std::uint32_t>(free_count) * 2u, slot_index);
    // ADD WORD reloads the count after the free-index write, even if aliased.
    put_half(slab, 0x1ac0, static_cast<std::uint16_t>(half(slab, 0x1ac0) + 1u));
    if (slab_index < word(pool, 0x34)) put(pool, 0x34, slab_index);
    put(pool, 0x24, word(pool, 0x24) - 1u);
    LeaveCriticalSection(critical_section);
}
} // namespace bsp

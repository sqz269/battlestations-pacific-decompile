#include "bsp/native_vfs_sequence_lifetime.hpp"

#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4);

void* at(const void* pointer, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(pointer) + offset);
}
std::uint32_t word(const void* pointer, std::uint32_t offset = 0) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at(pointer, offset));
}
void put(void* pointer, std::uint32_t offset, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(at(pointer, offset)) = value;
}
std::int32_t signed_bits(std::uint32_t bits) noexcept {
    std::int32_t result;
    std::memcpy(&result, &bits, sizeof result);
    return result;
}
std::int32_t signed_word(const void* pointer, std::uint32_t offset) noexcept {
    return signed_bits(word(pointer, offset));
}
void* pointer(const void* owner, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(owner, offset));
}
std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uintptr_t>(value);
}

void destroy_plain_list(void* owner) noexcept {
    void* head = pointer(owner, 4);
    void* current = pointer(head);
    put(head, 0, address(head));
    head = pointer(owner, 4);
    put(head, 4, address(head));
    const bool empty = current == pointer(owner, 4);
    put(owner, 8, 0);
    if (!empty) {
        do {
            void* const next = pointer(current);
            singleton_lifetime_free(current);
            const bool finished = next == pointer(owner, 4);
            current = next;
            if (finished) break;
        } while (true);
    }
    singleton_lifetime_free(pointer(owner, 4));
    put(owner, 4, 0);
}

void copy_pair_string(void* destination, const void* source,
    NativeStringStorage& storage, bool second_string) {
    put(destination, 0, 0);
    put(destination, 4, 0);
    if (destination == source) return;
    resize_native_string_header_0041dd40(destination, storage, word(source), true);
    if (word(source) != 0) {
        const auto length = word(destination);
        // BF7694/98 selects the backward path BF7844 when source<destination
        // <source+length. A standard memcpy call would introduce overlap UB.
        // Zero count performs no memory access; the raw pointers are still read.
        // BDCCA5 reads first destination data before source; BDCCE8/EB
        // reads second source data before destination. Retain both orders.
        void* data;
        const void* input;
        if (second_string) {
            input = pointer(source, 4);
            data = pointer(destination, 4);
        } else {
            data = pointer(destination, 4);
            input = pointer(source, 4);
        }
        if (length != 0) std::memmove(data, input, length);
    }
}
} // namespace

void __fastcall destroy_native_vfs_plain_list_00bdaed0(void* owner, void*) noexcept {
    destroy_plain_list(owner);
}
void __fastcall destroy_native_vfs_plain_list_007f8310(void* owner, void*) noexcept {
    destroy_plain_list(owner);
}
__declspec(naked) void __fastcall destroy_native_vfs_plain_list_thunk_00bdb3c0(
    void*, void*) noexcept {
    __asm { jmp destroy_native_vfs_plain_list_00bdaed0 }
}
__declspec(naked) void __fastcall destroy_native_vfs_plain_list_thunk_007f8770(
    void*, void*) noexcept {
    __asm { jmp destroy_native_vfs_plain_list_007f8310 }
}

void destroy_native_vfs_string_pair_00bdb850(void* pair,
    NativeStringStorage& storage) noexcept {
    // Native state0 arms cleanup of +0 while +8 is released. The existing
    // storage release is noexcept; native throwing-getter/FH3 is not projected.
    destroy_native_string_header_0041dd20(at(pair, 8), storage);
    destroy_native_string_header_0041dd20(pair, storage);
}

void* copy_construct_native_vfs_string_pair_00bdcc60(void* destination,
    const void* source, NativeStringStorage& storage) {
    copy_pair_string(destination, source, storage, false);
    try {
        copy_pair_string(at(destination, 8), at(source, 8), storage, true);
    } catch (...) {
        // FuncInfo E006A8 state0 -> CC6250 ->41DD20 on captured destination+0.
        // Do not destroy the partially constructed second string.
        destroy_native_string_header_0041dd20(destination, storage);
        throw;
    }
    return destination;
}

void reserve_native_vfs_pair_vector_00bdcd10(void* owner, std::int32_t capacity,
    NativeStringStorage& storage) {
    if (capacity < 1) capacity = 1;
    if (signed_word(owner, 8) >= capacity) return;
    const std::uint32_t bytes = static_cast<std::uint32_t>(capacity) << 4;
    void* const replacement = singleton_lifetime_allocate({
        SingletonAllocationKind::object, bytes, bytes});
    for (std::uint32_t index = 0;
        signed_bits(index) < signed_word(owner, 4); ++index) {
        const auto offset = index << 4;
        void* const destination = at(replacement, offset);
        if (destination != nullptr) {
            copy_construct_native_vfs_string_pair_00bdcc60(
                destination, at(pointer(owner), offset), storage);
        }
        // E006D4 state0 -> CC6270 ->401130 (RET): intentionally no rollback
        // of replacement storage or already completed pairs if copying throws.
    }
    for (std::uint32_t index = 0;
        signed_bits(index) < signed_word(owner, 4); ++index) {
        destroy_native_vfs_string_pair_00bdb850(
            at(pointer(owner), index << 4), storage);
    }
    singleton_lifetime_free(pointer(owner));
    // Raw BDCDD0..D9 follows returning free. Preserve publication order and
    // leave any count changes performed by destruction/free callbacks intact.
    put(owner, 0, address(replacement));
    put(owner, 8, static_cast<std::uint32_t>(capacity));
}

void resize_native_vfs_pair_vector_00bdec70(void* owner, std::int32_t count,
    NativeStringStorage& storage) {
    if (count > signed_word(owner, 8))
        reserve_native_vfs_pair_vector_00bdcd10(owner, count, storage);
    const auto initial_count = word(owner, 4);
    if (signed_bits(initial_count) < count) {
        std::uint32_t offset = initial_count << 4;
        std::uint32_t remaining = static_cast<std::uint32_t>(count) - initial_count;
        do {
            void* const slot = at(pointer(owner), offset);
            if (slot != nullptr) {
                put(slot, 0, 0);
                put(slot, 4, 0);
                put(slot, 8, 0);
                put(slot, 12, 0);
            }
            offset += 0x10;
            --remaining;
        } while (remaining != 0);
    }
    while (count < signed_word(owner, 4)) {
        put(owner, 4, word(owner, 4) - 1u);
        const auto offset = word(owner, 4) << 4;
        destroy_native_vfs_string_pair_00bdb850(at(pointer(owner), offset), storage);
    }
    put(owner, 4, static_cast<std::uint32_t>(count));
}

void destroy_native_vfs_pair_vector_00be0350(void* owner,
    NativeStringStorage& storage) {
    resize_native_vfs_pair_vector_00bdec70(owner, 0, storage);
    singleton_lifetime_free(pointer(owner));
}
} // namespace bsp

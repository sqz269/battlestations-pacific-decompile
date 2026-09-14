#include "bsp/native_renderer_resolution_enumeration.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
using Word = std::uint32_t;
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word bits) noexcept { return reinterpret_cast<void*>(bits); }
Word load(const void* base, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(address(base) + offset);
}
void store(void* base, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(address(base) + offset) = value;
}
std::int32_t signed_bits(Word bits) noexcept {
    std::int32_t result; std::memcpy(&result, &bits, sizeof result); return result;
}
using ModeCount = Word (__stdcall*)(void*, Word, Word);
using EnumMode = std::int32_t (__stdcall*)(void*, Word, Word, Word, void*);
} // namespace

std::int32_t __fastcall find_native_resolution_pair_008d46c0(
    const void* header, Word, const void* pair) {
    const Word base = load(header);
    const Word count = load(header, 4);
    const Word end = base + count * 8u;
    Word cursor = base;
    if (cursor >= end) return -1;
    const Word first = load(pair);
    do {
        if (load(pointer(cursor)) == first) {
            const Word second = load(pointer(cursor), 4);
            if (second == load(pair, 4)) return signed_bits(cursor - base) >> 3;
        }
        cursor += 8u;
    } while (cursor < end);
    return -1;
}

void __fastcall reserve_native_resolution_pairs_008d4750(
    void* header, Word, std::int32_t requested) {
    if (requested < 1) requested = 1;
    if (signed_bits(load(header, 8)) >= requested) return;
    const Word bytes = static_cast<Word>(requested) * 8u;
    void* const replacement = singleton_lifetime_allocate({
        SingletonAllocationKind::object, bytes, bytes});
    Word index = 0;
    Word destination = address(replacement);
    while (signed_bits(index) < signed_bits(load(header, 4))) {
        if (destination != 0) {
            const Word source = load(header) + index * 8u;
            const Word first = load(pointer(source));
            store(pointer(destination), 0, first);
            const Word second = load(pointer(source), 4);
            store(pointer(destination), 4, second);
        }
        index += 1u; destination += 8u;
    }
    singleton_lifetime_free(pointer(load(header)));
    store(header, 0, address(replacement));
    store(header, 8, static_cast<Word>(requested));
}

int __cdecl compare_native_resolution_pairs_00b1ffd0(const void* left, const void* right) {
    const Word left_width = load(left);
    const Word right_width = load(right);
    if (left_width != right_width) return signed_bits(left_width - right_width);
    const Word left_height = load(left, 4);
    return signed_bits(left_height - load(right, 4));
}

void __fastcall enumerate_native_renderer_resolutions_00b27d80(void* renderer, void* mode) {
    void* const initial_api = pointer(load(renderer, 0x1990));
    store(renderer, 0x19dc, 0);
    void* const initial_table = pointer(load(initial_api));
    const auto count_modes = reinterpret_cast<ModeCount>(load(initial_table, 0x18));
    const Word mode_count = count_modes(initial_api, 0, 0x16);
    for (Word index = 0; index < mode_count; ++index) {
        void* const api = pointer(load(renderer, 0x1990));
        void* const table = pointer(load(api));
        const auto enum_mode = reinterpret_cast<EnumMode>(load(table, 0x1c));
        if (enum_mode(api, 0, 0x16, index, mode) != 0) continue;
        const Word width = load(mode);
        if (width < 0x280) continue;
        const Word height = load(mode, 4);
        if (height < 0x1e0) continue;
        const Word pair[2]{width, height};
        void* const header = pointer(address(renderer) + 0x1c);
        if (find_native_resolution_pair_008d46c0(header, 0, pair) != -1) continue;
        const Word capacity = load(header, 8);
        if (load(header, 4) == capacity) {
            const Word doubled = capacity + capacity;
            reserve_native_resolution_pairs_008d4750(header, 0,
                signed_bits(doubled) > 1 ? signed_bits(doubled) : 1);
        }
        const Word count = load(header, 4);
        const Word data = load(header);
        const Word destination = data + count * 8u;
        if (destination != 0) {
            store(pointer(destination), 0, width);
            store(pointer(destination), 4, height);
        }
        store(header, 4, load(header, 4) + 1u);
    }
    const Word count = load(renderer, 0x20);
    void* const data = pointer(load(renderer, 0x1c));
    std::qsort(data, count, 8, &compare_native_resolution_pairs_00b1ffd0);
}
} // namespace bsp

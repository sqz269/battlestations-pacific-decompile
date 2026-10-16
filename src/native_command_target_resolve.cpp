#include "bsp/native_command_target_resolve.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw storage reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
Word bits(const void* p) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(p));
}
void* pointer(Word p) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(p));
}
void* add(const void* p, Word n) noexcept { return pointer(bits(p) + n); }
Word read(const void* p) noexcept { return *static_cast<const volatile Word*>(p); }
void write(void* p, Word n) noexcept { *static_cast<volatile Word*>(p) = n; }
} // namespace

Word resolve_native_command_target_00521ea0(void* actual_descriptor,
    const NativeCommandTargetHandleTables& tables) noexcept {
    void* const descriptor = actual_descriptor;
    if (*static_cast<const volatile unsigned char*>(descriptor) == 0) return 0;
    void* const cache = add(descriptor, 4);
    if (read(cache) != 0) return read(cache);
    const Word id = *static_cast<const volatile std::uint16_t*>(add(descriptor, 2));
    const Word split = read(tables.actual_split_00f89a10);
    // Biasing both sign bits exactly implements native signed JGE's inverse.
    if ((id ^ 0x80000000u) < (split ^ 0x80000000u)) {
        const Word index = id - read(tables.actual_low_base_00f89a0c);
        const Word table = read(tables.actual_low_table_00f89a54);
        const Word value = read(pointer(table + (index << 4) + 0x0cu));
        write(cache, value);
        return value;
    }
    const Word index = id - read(tables.actual_high_base_00f89a60);
    const Word table = read(tables.actual_high_table_00f89aa8);
    const Word value = read(pointer(table + (index << 4) + 0x0cu));
    write(cache, value);
    return read(cache);
}
} // namespace bsp

#include "bsp/native_damageable_section_fill.hpp"
#include "bsp/native_damageable_section.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Damageable section fill requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using DeleteRecord = void (__thiscall*)(void*, Word);
static_assert(sizeof(void*) == sizeof(Word));
void* pointer(Word address) noexcept { return reinterpret_cast<void*>(address); }
template<class T> T read(Word address) noexcept {
    return *reinterpret_cast<const volatile T*>(address);
}
} // namespace

void fill_native_damageable_sections_008798f0(void* first,
    Word count, const void* source, Word actual_vtable_00d0df04) {
    const Word begin = reinterpret_cast<Word>(first);
    Word current = begin;
    try {
        while (count != 0) {
            // State1's placement-delete funclet only calls bare RET00401130.
            // It neither destroys nor frees a partially constructed record.
            if (current != 0) {
                copy_construct_native_damageable_section_00878b40(
                    pointer(current), source, actual_vtable_00d0df04);
            }
            --count;
            current += 0x30; // Publish completed end only after copy returns.
        }
    } catch (...) {
        // 879951 captures begin and completed end once. Reload every row's
        // actual vptr and slot before its call; retain forward cleanup order.
        Word destroyed = begin;
        const Word end = current;
        while (destroyed != end) {
            const Word table = read<Word>(destroyed);
            read<DeleteRecord>(table)(pointer(destroyed), 0);
            destroyed += 0x30;
        }
        throw; // BF6885(0,0), using the current C++ exception runtime.
    }
}
} // namespace bsp

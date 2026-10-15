#include "bsp/native_dword_vector_fill_objects.hpp"

#include "bsp/native_dword_vector_assign.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);

Word address(const void* value) noexcept {
    return reinterpret_cast<Word>(value);
}
void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(value);
}
} // namespace

void __cdecl fill_native_checked_dword_vector_objects_00698010(
    void* first, const void* last, const void* source) {
    Word current = address(first);
    const Word captured_last = address(last);
    if (current == captured_last)
        return;
    const Word captured_source = address(source);
    do {
        assign_native_checked_dword_vector_00697bd0(
            pointer(current), 0u, pointer(captured_source));
        current += 0x10u;
    } while (current != captured_last);
}

} // namespace bsp

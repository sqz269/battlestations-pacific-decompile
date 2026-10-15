#include "bsp/native_raw_dword_list_node.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw three-DWORD node allocation requires MSVC Win32.
#endif

namespace bsp {
namespace {

using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));

Word read_parameter(const void* actual_parameter4) noexcept {
    return *static_cast<const volatile Word*>(actual_parameter4);
}
void write_word(Word address, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(address) = value;
}

} // namespace

void* allocate_native_raw_dword_list_node_00467ba0(const void* actual_parameter4) {
    void* const allocation = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x0c, 0x0c}); // 00467BA2
    const Word node = reinterpret_cast<Word>(allocation);
    if (node != 0) {
        write_word(node, 0); // 00467BB0
        write_word(node + 4u, 0); // 00467BB2
        const Word payload = read_parameter(actual_parameter4); // 00467BB5
        write_word(node + 8u, payload); // 00467BB9
        return allocation;
    }
    const Word payload = read_parameter(actual_parameter4); // 00467BBF
    write_word(8u, payload); // 00467BC5: retain the literal native null tail.
    return nullptr;
}

} // namespace bsp

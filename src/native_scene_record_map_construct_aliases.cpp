#include "bsp/native_scene_record_map_construct_aliases.hpp"
#include "bsp/native_scene_record_map_head.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw scene-map construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
U read(U address) noexcept { return *reinterpret_cast<const volatile U*>(address); }
void write(U address, U value) noexcept { *reinterpret_cast<volatile U*>(address) = value; }
} // namespace

void* construct_native_scene_record_map_head_00468900(void* actual_tree) {
    const U tree = reinterpret_cast<std::uintptr_t>(actual_tree);
    void* const allocation = allocate_native_scene_record_map_head_00468260();
    const U allocated_head = reinterpret_cast<std::uintptr_t>(allocation);
    write(tree + 4u, allocated_head);
    *reinterpret_cast<volatile unsigned char*>(allocated_head + 0x19u) = 1;
    U head = read(tree + 4u);
    write(head + 4u, head);
    head = read(tree + 4u);
    write(head, head);
    head = read(tree + 4u);
    write(head + 8u, head);
    write(tree + 8u, 0);
    return reinterpret_cast<void*>(head);
}

void* construct_native_scene_record_map_head_0046f130(void* actual_tree) {
    return construct_native_scene_record_map_head_00468e70(actual_tree);
}
} // namespace bsp

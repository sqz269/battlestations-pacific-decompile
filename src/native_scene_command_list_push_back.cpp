#include "bsp/native_scene_command_list_push_back.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4,
    "Native addresses and DWORD arithmetic require Win32.");

std::uint32_t read_word(const void* base, std::uint32_t offset) {
    const auto address = reinterpret_cast<std::uintptr_t>(base) + offset;
    return *reinterpret_cast<const volatile std::uint32_t*>(address);
}

void write_word(void* base, std::uint32_t offset, std::uint32_t value) {
    const auto address = reinterpret_cast<std::uintptr_t>(base) + offset;
    *reinterpret_cast<volatile std::uint32_t*>(address) = value;
}

} // namespace

void push_back_native_scene_command_list_00468350(
    void* actual_list12, const void* actual_parameter4) {
    void* node = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x0CU, 0x0CU});
    const auto node_bits = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(node));
    if (node_bits != 0) {
        write_word(node, 0, 0);
        write_word(node, 4, 0);
        write_word(node, 8, 0);
    }

    const auto payload = read_word(actual_parameter4, 0);
    write_word(node, 8, payload);
    const auto previous = read_word(actual_list12, 8);
    write_word(node, 0, previous);
    const auto count_for_branch = read_word(actual_list12, 0);
    if (count_for_branch != 0) {
        const auto current_tail = read_word(actual_list12, 8);
        write_word(reinterpret_cast<void*>(current_tail), 4, node_bits);
        write_word(actual_list12, 8, node_bits);
        write_word(node, 4, 0);
        const auto current_count = read_word(actual_list12, 0);
        write_word(actual_list12, 0, current_count + 1U);
    } else {
        write_word(actual_list12, 4, node_bits);
        write_word(actual_list12, 8, node_bits);
        write_word(node, 4, 0);
        const auto current_count = read_word(actual_list12, 0);
        write_word(actual_list12, 0, current_count + 1U);
    }
}

} // namespace bsp

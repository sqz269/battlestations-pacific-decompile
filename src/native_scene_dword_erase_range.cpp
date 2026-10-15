#include "bsp/native_scene_dword_erase_range.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_scene_dword_move.hpp"

#include <cstdint>

namespace bsp {
namespace {

using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));

Word address(const void* pointer) noexcept {
    return reinterpret_cast<Word>(pointer);
}
void* pointer(Word address) noexcept {
    return reinterpret_cast<void*>(address);
}
Word read_word(Word address) noexcept {
    return *reinterpret_cast<const volatile Word*>(address);
}
void write_word(Word address, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(address) = value;
}

} // namespace

void* erase_native_scene_dword_range_004685a0(
    void* actual_control, const void* actual_parameters20) {
    const Word parameters = address(actual_parameters20);
    const Word captured_owner = read_word(parameters + 4u); // 004685A1
    const Word control = address(actual_control);
    if (captured_owner == 0u || captured_owner != read_word(parameters + 0x0cu))
        invoke_native_invalid_parameter_00bf6713(); // 004685B3: can return.

    const Word first_node = read_word(parameters + 8u); // 004685B8
    const Word last_node = read_word(parameters + 0x10u); // 004685BC
    if (first_node != last_node) {
        const Word current_end = read_word(control + 8u); // 004685C4
        void* const result = move_native_scene_dword_range_00467d90(
            pointer(last_node), pointer(current_end), pointer(first_node));
        write_word(control + 8u, address(result)); // 004685E7, even zero/negative.
    }

    const Word output = read_word(parameters); // 004685EB: current, after SDK.
    write_word(output + 4u, first_node); // 004685F0
    write_word(output, captured_owner); // 004685F4
    return pointer(output);
}

} // namespace bsp

#include "bsp/native_scene_dword_vector_range_erase.hpp"

#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_scene_dword_erase_range.hpp"

#include <cstdint>

namespace bsp {
namespace {

using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));

Word address(const void* storage) noexcept {
    return reinterpret_cast<Word>(storage);
}

void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(value);
}

Word read_word(Word location) noexcept {
    return *reinterpret_cast<const volatile Word*>(location);
}

void write_word(Word location, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(location) = value;
}

} // namespace

void* erase_native_scene_dword_vector_range_004687e0(
    void* actual_control, const void* actual_parameters20) {
    return erase_native_scene_dword_range_004685a0(actual_control,
        actual_parameters20);
}

void* clear_native_scene_dword_vector_00468d70(void* actual_control,
    void* actual_parameters20, void* actual_private_output8) {
    const Word control = address(actual_control);
    const Word parameters = address(actual_parameters20);
    const Word private_output = address(actual_private_output8);

    const Word captured_end = read_word(control + 8u);
    const Word first_begin = read_word(control + 4u);
    if (first_begin > captured_end)
        invoke_native_invalid_parameter_00bf6713();

    const Word captured_begin = read_word(control + 4u);
    const Word current_end = read_word(control + 8u);
    if (captured_begin > current_end)
        invoke_native_invalid_parameter_00bf6713();

    write_word(parameters + 0x10u, captured_end);
    write_word(parameters + 0x0cu, control);
    write_word(parameters + 8u, captured_begin);
    write_word(parameters + 4u, control);
    write_word(parameters, private_output);
    return erase_native_scene_dword_vector_range_004687e0(pointer(control),
        pointer(parameters));
}

} // namespace bsp

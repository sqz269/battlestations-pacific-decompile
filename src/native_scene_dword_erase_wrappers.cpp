#include "bsp/native_scene_dword_erase_wrappers.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_scene_dword_erase_range.hpp"

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

void* clear_control(Word control, const NativeSceneDwordEraseWrappersFrame& frame) {
    const Word captured_end = read_word(control + 8u);
    if (read_word(control + 4u) > captured_end)
        invoke_native_invalid_parameter_00bf6713();
    const Word captured_begin = read_word(control + 4u);
    if (captured_begin > read_word(control + 8u))
        invoke_native_invalid_parameter_00bf6713();

    const Word arguments = address(frame.actual_child_parameters20);
    write_word(arguments + 0x10u, captured_end);
    write_word(arguments + 0x0cu, control);
    write_word(arguments + 8u, captured_begin);
    write_word(arguments + 4u, control);
    write_word(arguments, address(frame.actual_output8));
    return erase_native_scene_dword_range_004685a0(
        pointer(control), pointer(arguments));
}

} // namespace

void* clear_native_scene_dword_vector_00468af0(void* actual_control,
    const NativeSceneDwordEraseWrappersFrame& frame) {
    return clear_control(address(actual_control), frame);
}

void* clear_native_scene_dword_vector_embedded8_00468c90(void* actual_receiver,
    const NativeSceneDwordEraseWrappersFrame& frame) {
    return clear_control(address(actual_receiver) + 8u, frame);
}

} // namespace bsp

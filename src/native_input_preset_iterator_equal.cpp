#include "bsp/native_input_preset_iterator_equal.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"

namespace bsp {
namespace {

// Unsigned address addition preserves Win32 DWORD wrapping; the caller supplies
// valid reached aligned storage. No node overlay or alternate iterator model.
__forceinline std::uint32_t read_iterator_word(
    const void* actual_iterator, std::uint32_t byte_offset) {
    const auto address = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(actual_iterator)) + byte_offset;
    return *reinterpret_cast<const volatile std::uint32_t*>(
        static_cast<std::uintptr_t>(address));
}

} // namespace

bool __fastcall equal_native_input_preset_tree_iterators_00699ef0(
    const NativeInputPresetTreeIterator* actual_left, std::uint32_t,
    const NativeInputPresetTreeIterator* actual_right) {
    const auto left_owner = read_iterator_word(actual_left, 0);
    if (left_owner == 0 || left_owner != read_iterator_word(actual_right, 0)) {
        invoke_native_invalid_parameter_00bf6713();
    }
    const auto current_left_node = read_iterator_word(actual_left, 4);
    const auto current_right_node = read_iterator_word(actual_right, 4);
    return current_left_node == current_right_node;
}

} // namespace bsp

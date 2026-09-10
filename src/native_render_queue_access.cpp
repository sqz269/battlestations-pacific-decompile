#include "bsp/native_render_queue_access.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native queue field access requires MSVC Win32 pointer widths.
#endif

namespace bsp {

std::uint32_t read_native_render_batch_sort_configuration_00b1cb30(
    const NativeRenderCommandQueueStorage& queue, std::uint32_t index,
    volatile std::uint8_t& byte_output, volatile std::uint32_t& word_output) noexcept {
    const auto entry = reinterpret_cast<std::uintptr_t>(&queue) + index * 8u;
    const auto byte = *reinterpret_cast<const volatile std::uint8_t*>(entry + 4u);
    byte_output = byte;
    const auto word = *reinterpret_cast<const volatile std::uint32_t*>(entry + 8u);
    word_output = word;
    return (word & 0xffffff00u) | 1u;
}

void set_native_last_render_command_metadata_00b1cb50(
    const NativeRenderCommandQueueStorage& queue, const void* three_source_words) noexcept {
    const volatile auto& actual = queue;
    const auto count = static_cast<std::uint32_t>(actual.commands_14.count_04);
    const auto base = reinterpret_cast<std::uintptr_t>(actual.commands_14.data_00);
    void* command;
    std::memcpy(&command, reinterpret_cast<const void*>(base + count * 4u - 4u), 4);
    auto* const destination = static_cast<unsigned char*>(command) + 0x1c;
    const auto* const source = static_cast<const unsigned char*>(three_source_words);
    for (std::uint32_t offset = 0; offset != 12; offset += 4) {
        std::uint32_t word;
        std::memcpy(&word, source + offset, 4);
        std::memcpy(destination + offset, &word, 4);
    }
}

} // namespace bsp

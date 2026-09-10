#pragma once

#include "bsp/native_render_pointer_arrays.hpp"

#include <type_traits>

namespace bsp {

struct NativeRenderBatchSortConfigurationStorage {
    std::uint8_t enabled_00;
    std::uint8_t preserved_01_03[3];
    std::uint32_t value_04;
};

// Actual 34h queue storage; this declaration performs no initialization,
// registration, allocation, destruction, scheduling or command execution.
struct NativeRenderCommandQueueStorage {
    std::uint32_t native_vtable_00;
    NativeRenderBatchSortConfigurationStorage configurations_04[2];
    NativeRenderPointerArrayStorage commands_14;
    std::uint32_t control_20;
    void* rows_data_24;
    std::int32_t rows_count_28;
    std::int32_t rows_capacity_2c;
    void* context_30;
};

static_assert(sizeof(NativeRenderBatchSortConfigurationStorage) == 8);
static_assert(offsetof(NativeRenderBatchSortConfigurationStorage, value_04) == 4);
static_assert(sizeof(NativeRenderCommandQueueStorage) == 0x34);
static_assert(offsetof(NativeRenderCommandQueueStorage, configurations_04) == 4);
static_assert(offsetof(NativeRenderCommandQueueStorage, commands_14) == 0x14);
static_assert(offsetof(NativeRenderCommandQueueStorage, control_20) == 0x20);
static_assert(offsetof(NativeRenderCommandQueueStorage, rows_data_24) == 0x24);
static_assert(offsetof(NativeRenderCommandQueueStorage, context_30) == 0x30);
static_assert(std::is_trivially_default_constructible_v<NativeRenderCommandQueueStorage>);

// Full B1CB30; ECX queue, stack index/byte-output/DWORD-output, RET0Ch.
// Copy the byte first, then read/copy the DWORD, so outputs may alias the
// source fields. Return the copied DWORD with its low byte replaced by 1,
// preserving the original EAX high 24 bits. No native bounds/empty checks.
std::uint32_t read_native_render_batch_sort_configuration_00b1cb30(
    const NativeRenderCommandQueueStorage&, std::uint32_t index,
    volatile std::uint8_t& byte_output, volatile std::uint32_t& word_output) noexcept;

// Full B1CB50; ECX queue, stack pointer to three readable DWORDs, RET4.
// Capture the current last command, then copy source words in order to that
// command+1C/+20/+24. Source/destination overlap preserves the sequential
// reads and writes. A nonempty queue and valid command/storage are required.
void set_native_last_render_command_metadata_00b1cb50(
    const NativeRenderCommandQueueStorage&, const void* three_source_words) noexcept;

// Full B1C3C0: unconditionally clear actual publication F8D440, then write the
// base table word CE3818 at the supplied base. No identity guard, command
// execution, member destruction, unregister or free. This is only base cleanup,
// not the complete queue destructor. Original ECX=base, RET, no semantic result.
void destroy_native_render_queue_base_00b1c3c0(void* actual_base,
    NativeRenderCommandQueueStorage* volatile& global_00f8d440) noexcept;

} // namespace bsp

#pragma once

#include "bsp/native_crt_small_block_heap_init.hpp"

#include <cstdint>
#include <type_traits>

namespace bsp {

// Borrow the canonical native CRT cells. This view owns no heap, allocation,
// global storage, initialization or lock. The caller must hold the actual
// required lock and bind the same cells used by C11CF5 and native SBH free.
struct NativeCrtSbhState {
    void* volatile& actual_heap_0109e1bc;
    volatile std::uint32_t& actual_descriptor_count_0109ed64;
    volatile std::uint32_t& actual_descriptor_capacity_0109ed74;
    void* volatile& actual_descriptors_0109ed68;
    void* volatile& actual_allocation_rover_0109ed70;
    volatile std::uint32_t& actual_retained_empty_descriptor_0109e310;
    volatile std::uint32_t& actual_retained_empty_group_0109ed78;
};

static_assert(std::is_same_v<decltype(NativeCrtSbhState::actual_heap_0109e1bc),
    decltype(NativeCrtSmallBlockHeapInitContext::actual_heap_0109e1bc)>);
static_assert(std::is_same_v<decltype(NativeCrtSbhState::actual_descriptors_0109ed68),
    decltype(NativeCrtSmallBlockHeapInitContext::actual_result_0109ed68)>);
static_assert(std::is_same_v<decltype(NativeCrtSbhState::actual_allocation_rover_0109ed70),
    decltype(NativeCrtSmallBlockHeapInitContext::actual_second_result_0109ed70)>);
static_assert(std::is_same_v<decltype(NativeCrtSbhState::actual_retained_empty_descriptor_0109e310),
    decltype(NativeCrtSmallBlockHeapInitContext::actual_word_0109e310)>);
static_assert(std::is_same_v<decltype(NativeCrtSbhState::actual_descriptor_count_0109ed64),
    decltype(NativeCrtSmallBlockHeapInitContext::actual_word_0109ed64)>);
static_assert(std::is_same_v<decltype(NativeCrtSbhState::actual_descriptor_capacity_0109ed74),
    decltype(NativeCrtSmallBlockHeapInitContext::actual_word_0109ed74)>);

// Mapping only: the initialized context must already bind actual canonical
// cells. ED78 belongs to the same native SBH owner but is not a C11CF5 input.
inline NativeCrtSbhState bind_native_crt_sbh_state(
    NativeCrtSmallBlockHeapInitContext& initialized_cells,
    volatile std::uint32_t& actual_retained_empty_group_0109ed78) noexcept {
    return {initialized_cells.actual_heap_0109e1bc,
        initialized_cells.actual_word_0109ed64,
        initialized_cells.actual_word_0109ed74,
        initialized_cells.actual_result_0109ed68,
        initialized_cells.actual_second_result_0109ed70,
        initialized_cells.actual_word_0109e310,
        actual_retained_empty_group_0109ed78};
}
} // namespace bsp

#pragma once

#include <cstdint>

namespace bsp {
// Caller binds these references to the actual owning CRT words. No state,
// allocation provider, or lifetime policy is created by this interface.
struct NativeCrtSmallBlockHeapInitContext {
    void* volatile& actual_heap_0109e1bc;
    void* volatile& actual_result_0109ed68;
    volatile std::uint32_t& actual_word_0109e310;
    volatile std::uint32_t& actual_word_0109ed64;
    void* volatile& actual_second_result_0109ed70;
    volatile std::uint32_t& actual_threshold_0109ed6c;
    volatile std::uint32_t& actual_word_0109ed74;
};

// Full C11CF5 small-block initializer schedule for a current valid native
// heap handle. Original ABI: cdecl one stacked DWORD threshold, EAX 0/1,
// plain RET with caller cleanup. This is a context-bearing C++ interface,
// not a native ABI thunk or a reconstruction of SBH allocation/free.
std::uint32_t initialize_native_crt_small_block_heap_00c11cf5(
    std::uint32_t threshold, NativeCrtSmallBlockHeapInitContext& context);
} // namespace bsp

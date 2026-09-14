#include "bsp/native_crt_small_block_heap_init.hpp"

#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT small-block initializer requires MSVC Win32.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);

std::uint32_t initialize_native_crt_small_block_heap_00c11cf5(
    std::uint32_t threshold, NativeCrtSmallBlockHeapInitContext& context) {
    // C11CF5 reads the current owning heap, not the process heap. HeapAlloc
    // receives flags zero: the 140h bytes are not cleared by this body.
    void* const allocation = ::HeapAlloc(context.actual_heap_0109e1bc, 0, 0x140u);
    context.actual_result_0109ed68 = allocation; // Also publish a null result.
    if (!allocation) return 0;

    // Native AND memory,0 performs a read and a write of each current word.
    context.actual_word_0109e310 &= 0u;
    context.actual_word_0109ed64 &= 0u;
    context.actual_second_result_0109ed70 = allocation;
    context.actual_threshold_0109ed6c = threshold;
    context.actual_word_0109ed74 = 0x10u;
    return 1;
}
} // namespace bsp

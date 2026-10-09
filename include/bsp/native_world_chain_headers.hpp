#pragma once

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native World chain headers require MSVC Win32.
#endif

namespace bsp {

// Hypothetical type name. Actual World+4/+8 headers are first/last/count;
// the embedded category roots use a different count/head/tail layout.
struct NativeWorldChainHeader {
    void* first;
    void* last;
    std::uint32_t count;
};

inline constexpr std::size_t native_world_chain_header_bytes = 0x0c;
inline constexpr std::size_t native_world_chain_headers_owner_bytes = 0x4bc;

// New C++ interface for the complete 009037F0 storage/publication schedule,
// not an original-ABI replacement. The caller supplies actual fresh post-base
// World storage, with no previous ownership at +4, +8 or +4A8. Only active_word's
// low byte is used; unused_word corresponds to the ignored native stack word.
// Allocations use singleton_lifetime_allocate/free's domain. This publishes
// World+4 before allocating World+8 and returns the second header. If the second
// allocation throws, the first header remains published and +8 is unchanged.
// No local rollback or full-World destructor is invoked. Full World ownership,
// normal teardown, category roots and matrix-sentinel lifetime are external.
NativeWorldChainHeader* allocate_native_world_chain_headers_009037f0(
    void* actual_world_storage, std::uint32_t active_word,
    std::uint32_t unused_word);

} // namespace bsp

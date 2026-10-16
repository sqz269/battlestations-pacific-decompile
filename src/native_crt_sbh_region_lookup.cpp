#include "bsp/native_crt_sbh_region_lookup.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native SBH region lookup requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);

// One actual native-width raw read, including unaligned x86 storage. This is
// a Source operation, not a replacement provider or descriptor owner overlay.
__forceinline Word load_word(Word native_address) {
    Word result;
    __asm {
        mov eax, native_address
        mov eax, dword ptr [eax]
        mov result, eax
    }
    return result;
}
} // namespace

void* find_native_crt_sbh_region_00c11d3d(
    const NativeCrtSbhState& state,
    const void* actual_parameter4) {
    const Word count = state.actual_descriptor_count_0109ed64;
    Word descriptor = static_cast<Word>(reinterpret_cast<std::uintptr_t>(
        state.actual_descriptors_0109ed68));
    const Word end = descriptor + count * 0x14u;

    while (descriptor < end) {
        Word distance = load_word(static_cast<Word>(
            reinterpret_cast<std::uintptr_t>(actual_parameter4)));
        distance -= load_word(descriptor + 0x0cu);
        if (distance < 0x100000u) {
            return reinterpret_cast<void*>(static_cast<std::uintptr_t>(descriptor));
        }
        descriptor += 0x14u;
    }

    return nullptr;
}

} // namespace bsp

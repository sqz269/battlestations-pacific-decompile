#include "bsp/cc16_dyn_radix_owner.hpp"

namespace bsp::cc16_dyn_radix_owner {
namespace {
using U32 = std::uint32_t;
static_assert(sizeof(void*) == 4, "The recovered layout requires Win32 pointers.");

void* at(const void* base, U32 offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<U32>(base) + offset);
}

volatile U32& word(const void* base, U32 offset = 0) noexcept {
    return *static_cast<volatile U32*>(at(base, offset));
}
} // namespace

void* create_object_00c55d30(void* manager, U32 object_word_00,
    const void* source_six_words, std::uint8_t flag_1c, const Heap& heap) {
    void* object = heap.operator_new_00bf681b(0x24);
    if (object != nullptr) {
        word(object) = object_word_00;
        *static_cast<volatile std::uint8_t*>(at(object, 0x1c)) = flag_1c;
        for (U32 offset = 0; offset != 0x18; offset += 4) {
            word(object, offset + 4) = word(source_six_words, offset);
        }
    }
    // Both native branches call the same helper. Its original null-object
    // behavior is retained, including the eventual invalid object+20h store.
    cc16_dyn_radix_entries::append_object_00c37180(manager, object, heap.entries);
    return object;
}

} // namespace bsp::cc16_dyn_radix_owner

#include "bsp/native_resource_hierarchy_append.hpp"

#include "bsp/native_resource_pointer_array.hpp"
#include <cstring>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::int32_t) == 4);
using Word = std::uint32_t;

Word address(const void* pointer) noexcept {
    return reinterpret_cast<Word>(pointer);
}
volatile Word& word(Word address_value) noexcept {
    return *reinterpret_cast<volatile Word*>(address_value);
}
std::int32_t signed_bits(Word bits) noexcept {
    std::int32_t value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
} // namespace

void append_native_hierarchy_item_00b87ae0(void* resource,
    Word hierarchy_item_word) {
    const Word header = address(resource) + 0x1cu;
    const Word capacity = word(header + 8u);
    if (word(header + 4u) == capacity) {
        Word requested = capacity + 16u;
        if (signed_bits(requested) <= 16) requested = 16u;
        reserve_native_hierarchy_item_pointers_00b87350(
            reinterpret_cast<void*>(header), signed_bits(requested));
    }
    const Word count = word(header + 4u);
    const Word data = word(header);
    const Word slot = data + count * 4u;
    if (slot != 0) word(slot) = hierarchy_item_word;
    word(header + 4u) = word(header + 4u) + 1u;
}
} // namespace bsp

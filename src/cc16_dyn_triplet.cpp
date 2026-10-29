#include "bsp/cc16_dyn_triplet.hpp"

namespace bsp::cc16_dyn_triplet {
namespace {
using U32 = std::uint32_t;
static_assert(sizeof(void*) == 4, "The recovered address arithmetic requires Win32.");

// Actual input reads remain ordered and conditional, rather than eagerly
// snapshotting both records before the native first-mismatch gates.
U32 word(const U32* data, U32 index) noexcept {
    const U32 address = reinterpret_cast<U32>(data) + index * 4u;
    return *reinterpret_cast<const volatile U32*>(address);
}
}  // namespace

U32 cyclic_equal_00c39040(const U32* left, const U32* right) noexcept {
    U32 index = 0;
    for (; index < 3u; ++index) {
        const U32 right_word = word(right, index);
        if (right_word != word(left, index)) {
            break;
        }
    }
    if (index == 3u) {
        return 1u;
    }

    // 00C3906E..00C3907E reads right[1], right[2], right[0].
    U32 rotated[3];
    rotated[0] = word(right, 1u);
    rotated[1] = word(right, 2u);
    rotated[2] = word(right, 0u);
    for (index = 0; index < 3u; ++index) {
        if (rotated[index] != word(left, index)) {
            break;
        }
    }
    if (index == 3u) {
        return 1u;
    }

    // 00C390B1..00C390C7 captures left[0], left[1], left[2], in order,
    // before reusing the three local slots as [left1,left2,left0].
    const U32 left0 = word(left, 0u);
    const U32 left1 = word(left, 1u);
    const U32 left2 = word(left, 2u);
    rotated[2] = left0;
    rotated[1] = left2;
    rotated[0] = left1;
    for (index = 0; index < 3u; ++index) {
        if (word(right, index) != rotated[index]) {
            return 0u;
        }
    }
    return 1u;
}

U32 reversed_cyclic_equal_00c48140(const U32* left, const U32* right) noexcept {
    const U32 left1 = word(left, 1u);
    const U32 left2 = word(left, 2u);
    const U32 left0 = word(left, 0u);
    const U32 reversed[3]{left2, left1, left0};
    return cyclic_equal_00c39040(reversed, right);
}

}  // namespace bsp::cc16_dyn_triplet

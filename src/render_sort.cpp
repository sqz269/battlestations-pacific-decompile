#include "bsp/render_sort.hpp"

namespace bsp {
bool render_entry_key_less_00b51b00(const RenderEntrySortFields& left,
                                 const RenderEntrySortFields& right) noexcept {
    // 00b51b06 JA / 00b51b08 JC compare the high DWORD unsigned, then
    // 00b51b10 JNC rejects a low DWORD greater than or equal to the right.
    return left.key < right.key;
}

bool render_entry_material_value_less_00b51ab0(const RenderEntrySortFields& left,
                                            const RenderEntrySortFields& right) noexcept {
    // 00b51aee SETL establishes signed ordering, despite the related key path
    // masking this material field down to six bits.
    if (left.material_order != right.material_order)
        return left.material_order < right.material_order;
    // FCOMIP left,right followed by JBE rejects <= and unordered comparisons.
    // The boolean result matches; x87 exception/status side effects are not ported.
    return left.secondary_value > right.secondary_value;
}
}

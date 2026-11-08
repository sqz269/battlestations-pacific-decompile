#pragma once
#include <cstdint>

namespace bsp {
// Actual three-DWORD header at canonical 0108FE9C+28: data/count/capacity.
// New C++ interfaces, not drop-in ABI replacements. Native ECX is this
// header; set-capacity/reserve/append RET4, grow RET. Descriptive names are
// hypotheses. No payload ownership, bounds validation, or exception rollback.
void set_native_hardware_layout_table_capacity_00b601c0(
    void* actual_table, std::uint32_t capacity);
void grow_native_hardware_layout_table_00b60210(void* actual_table);
void append_native_hardware_layout_table_00b603f0(
    void* actual_table, const void* actual_pointer_word);
void reserve_native_hardware_layout_table_00b60470(
    void* actual_table, std::uint32_t capacity);
} // namespace bsp

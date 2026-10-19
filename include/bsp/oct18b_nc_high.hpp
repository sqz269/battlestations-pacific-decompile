#pragma once
#include <cstdint>

// New build-only C++ interfaces. See reports/oct18b_nc_high.json for original
// ABIs and uncertainty. Native layouts require Win32; no game host is wired.
namespace bsp::oct18b_nc_high {
// 00BBD7F0: ECX=raw {dword* data, uint32 count,...}; stack pointer-to-key,
// AL=found, RET 4. Remove the first match by replacing it with the last word.
bool pointer_array_remove_unordered_00bbd7f0(void* array, const std::uint32_t* key);
// Vtable 00D68208 slots 1/2/3, class identity unresolved. Each is exactly RET;
// no stack arguments, no defined return value. Naked bodies preserve registers.
void owner_default_slot1_00bd1800();
void owner_default_slot2_00bd1810();
void owner_default_slot3_00bd1820();
} // namespace bsp::oct18b_nc_high

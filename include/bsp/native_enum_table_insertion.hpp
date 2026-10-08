#pragma once
#include "bsp/native_string.hpp"
#include <cstdint>

namespace bsp {
constexpr std::uint32_t native_scene_enum_registry_storage_bytes = 0x10c;

// Only [004D3069,004D308B), inside004D3040; no global startup binding.
void* initialize_native_scene_enum_registry_storage_004d3069(void* actual_10c_owner);

// Normal008F17E0 binding ONLY for null or disjoint current-CRT19Ch CEnums
// constructed by008F4DD0 (D16508), whose slot0 is genuine008F59C0.
// This producer/heap/lifetime precondition applies to old and new payloads;
// Source directly binds the target, without original profile/slot loads or
// general virtual dispatch. Fresh owning key-copy storage must be disjoint.
// Receiver is actual10Ch registry+4. Distinct real14h table/symbol pools
// and genuine string context are supplied; generic mapped classes are external.
std::uint32_t insert_native_cenum_table_008f17e0(void* actual_nested_map,
    const NativeString& actual_key, void* new_actual_cenum_or_null,
    void* actual_table_pool, void* actual_symbol_pool,
    NativeStringRawPoolContext& strings, const char* node_empty,
    const char* key_empty);
} // namespace bsp

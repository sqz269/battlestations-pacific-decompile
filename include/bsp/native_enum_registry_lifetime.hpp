#pragma once
#include "bsp/native_string.hpp"

namespace bsp {
// Complete ordinary normal004D0760, only actual10Ch registry+4 containing
// null or uniquely owned current-CRT19Ch CEnums from genuine008F4DD0.
// D16508->008F59C0 is a producer/heap/lifetime precondition: Source directly
// binds that complete target, without original profile/slot loads or generic
// virtual dispatch. Distinct actual tableE175E8/symbol pools are required.
void clear_native_cenum_table_map_004d0760(void* actual_map_receiver,
    void* actual_table_pool, void* actual_symbol_pool,
    NativeStringRawPoolContext& actual_strings);

// Complete ordinary normal008F4F00: CE78BC, clear(owner+4), CE7514,
// SECOND empty clear(owner+4). Borrow the genuine initialized10Ch root;
// no root free. Native FS/EH/class ABI and scalar wrappers remain external.
void destroy_native_scene_enum_registry_008f4f00(void* actual_10c_owner,
    void* actual_table_pool, void* actual_symbol_pool,
    NativeStringRawPoolContext& actual_strings);

// Successful coherent/disjoint/nonoverflow ordinary storage only. No shared
// payload ownership, aliases, reentry/concurrency, fault/rollback/default policy,
// foreign classes/heaps, namespace/global startup, world or game binding.
} // namespace bsp

#pragma once
#include "bsp/native_string.hpp"

namespace bsp {
// Whole native 004BA130: ECX borrowed map, EAX same address, EDI preserved,
// plain RET. Requires DF=0 and actual cleared/unowned 108h storage at root+4.
void* __fastcall initialize_native_cenum_table_map_storage_004ba130(void*) noexcept;

// Whole native 004D25F0: ECX actual fresh/unowned 10Ch root, EAX same address,
// EDI preserved, plain RET, DF=0. Neither initializer allocates or frees.
void* __fastcall initialize_native_scene_enum_registry_storage_004d25f0(void*) noexcept;

// Whole normal 004D0EA0 phase and cleanup through the genuine current clear.
// This explicit-context C++ ABI is distinct from the native ECX/tail-JMP ABI.
// Borrow root+4 and real distinct table/symbol pools and owning raw strings.
void destroy_native_cenum_table_map_004d0ea0(void* actual_map_receiver,
    void* actual_table_pool, void* actual_symbol_pool,
    NativeStringRawPoolContext& actual_strings);

// The nested scalar-deleting wrapper 004D0EB0 remains outside this contract:
// root+4 is interior storage and must never be freed separately. No foreign
// class/heap, shared ownership, faults, reentry, native EH or game binding.
} // namespace bsp

#pragma once
#include "bsp/native_enum_symbol_insertion.hpp"
#include <cstdint>

namespace bsp {
// Borrow actual, already-initialized bindings from the admitted current-CRT
// CEnum domain. The symbol pool E17578 is distinct from table pool E175E8 and
// property pool E175B0. The two nonnull empty-string roles remain distinct.
// This context creates no owner, process cells, fallback bytes or callbacks.
struct NativeEnumSymbolDeclarationContext {
    NativeStringRawPoolContext& actual_strings;
    void* actual_initialized_symbol_pool_00e17578;
    const char* actual_enum_empty_00e186ed;
    const char* actual_query_empty_00e17654;
};

// Complete ordinary 008F2E40 through a new context-bearing C++ interface.
// Original: ECX outer19Ch CEnum, stack C string/int32, RET8, no stable semantic
// EAX result. Duplicate retains the first matching key/value and all metadata.
// New symbol: signed min(old+10C,value)->+10C; derive +110 from THAT candidate
// and value, ignoring old+110. Only then reconstruct a query and call008F2850.
// Both actual temporary string allocations are explicitly returned in order.
void declare_native_enum_symbol_008f2e40(void* actual_raw_19c_enum,
    const char* actual_symbol_text, std::int32_t value,
    NativeEnumSymbolDeclarationContext&);

// Admission: genuine live current-CRT19Ch owner and canonical raw strings;
// initialized real symbol pool; stable/disjoint/coherent successful ordinary
// storage, finite valid chains and nonoverflowing lengths/counts; nonnull,
// closed NUL-free ASCII text and stable C-locale CRT. Mapped value is signed
// only for metadata comparisons; insertion retains all32 bits. Existing key
// ownership/lifetime is unchanged. Original ABI/FS/EH/private CRT, faults,
// reentry/concurrency, canonical process startup, class and game are external.
} // namespace bsp

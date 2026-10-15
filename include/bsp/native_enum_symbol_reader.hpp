#pragma once
#include "bsp/native_enum_symbol_declaration.hpp"
#include "bsp/native_scene_token_text.hpp"

namespace bsp {
// Genuine already-live owners/bindings only. Scratch is the writable process
// F89758 role, not a replacement local buffer; its capacity must fit each name.
// Immutable punctuators are the actual }NUL, =NUL and ;NUL roles. Context owns
// nothing and supplies no construction, default cells, callbacks or model.
struct NativeEnumSymbolReaderContext {
    NativeSceneTokenValueContext& actual_token_values;
    NativeEnumSymbolDeclarationContext& actual_enum_declaration;
    char* actual_symbol_scratch_00f89758;
    const char* actual_close_00ce4cd4;
    const char* actual_equals_00ce55bc;
    const char* actual_semicolon_00ce5698;
};

// Complete ordinary008F31A0, with a new explicit-context C++ interface.
// Original ECX genuine19Ch CEnum; stack+4 live838h tokenizer; RET4; AL result
// of final expect-close. Original flag output aliases its incoming stack slot;
// Source uses an actual local byte and is not that register/stack transport.
bool read_native_enum_symbols_008f31a0(void* actual_raw_19c_enum,
    void* actual_live_838h_tokenizer,NativeEnumSymbolReaderContext&);

// Preserve full name-copy/consume/expect/int/optional-semicolon/declaration loop
// until exact at-most-two-byte }NUL match. Failures still declare returned int;
// no EOF, nesting, error exit or iteration guard is invented. Input without a
// closing token may not terminate. Admit inherited successful stable/disjoint
// current-CRT CEnum/raw-string/real-E17578 pool and live-tokenizer domains,
// NUL-terminated ASCII names fitting both genuine buffers, distinct E186ED and
// E17654 fallback roles, shared canonical raw-string authority and C-locale CRT.
// Bind the accepted concrete token-value default scan_decimal/std::sscanf body;
// arbitrary replacement callbacks are not this admission. Scratch is shared:
// caller synchronization excludes reentry/concurrent mutation. File constructor
// 008D9CF0/startup/wiring and Original ABI/FS/EH/privateCRT/class/game are external.
} // namespace bsp

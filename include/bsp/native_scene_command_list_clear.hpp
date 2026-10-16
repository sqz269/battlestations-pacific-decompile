#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// Whole110B/43op0046A9F0 over the actual12-byte {count,head,tail} header and
// actual12-byte nodes {prev,next,record}. Native ECX=header, unusedEDX,
// plainRET/semanticvoid; this new ordinary C++ API is not Original binary ABI.
//
// Borrow actual storage and the existing concrete raw pool/current CRT
// contracts. Binding metadata/context stays stable and disjoint from reached
// raw accesses; actual header/node/record cells retain all live aliases and
// ordered reloads. Supply the explicit saved-this4 backing only where a
// nonnull record reaches its real destructor. Empty count skips head/tail and
// every child backing access; null records skip child/free/record-slot writes.
// No implicit compiler/private-FH3/outgoing-argument homes are supplied.
//
// Child C++ exceptions propagate before later record/node frees or list edits;
// no parent cleanup or rollback is invented. Child cleanup/current CRT
// transport is qualified separately; Original heap, FH3/SEH/private-frame,
// binary ABI, application adoption and game equivalence remain unproved.
void clear_native_scene_command_list_0046a9f0(void* actual_header12,
    void* actual_record_destructor_saved_this4, NativeStringRawPoolContext& strings);
} // namespace bsp

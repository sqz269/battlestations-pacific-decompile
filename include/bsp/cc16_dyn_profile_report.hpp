#pragma once

#include "bsp/dyn_profile_scopes.hpp"

namespace bsp::cc16_dyn_profile_report {

// Native sink boundary at C32157: ECX/EDX are two raw position words;
// the only stack argument is the borrowed text pointer and the sink pops4.
// Names/position meanings are hypotheses. No sink or context is synthesized.
using DynProfileReportSink = void (__fastcall*)(
    std::uint32_t first_position_bits, std::uint32_t second_position_bits,
    const char* text);

// Complete normal 00C320D0..00C321AB, ECX=node, stack(sink,first*,second*),
// RET0C. This explicit cdecl C++ API borrows the existing actual48h node,
// position words, scope context's actual0109E9F8 publication cell and the
// genuine double100 cell00D7A220. No substitute publication cell, native class
// or default provider is created.
// The denominator node is the actual profile owner's +10h word, not a
// substituted root node. All original non-null/storage/tree invariants apply.
//
// Reuses real cc15 format_string_1024 and the real MSVC x86 __alldiv helper.
// Retains ambient x87 operations, their single double argument spill, captured
// tick words/SBO decision, callback epochs and uint32-wrapping live traversal.
// first is read again for each child's local first+20; shared second +=20 is
// visible before each sink. The sink borrows text for this invocation's life.
//
// Caller bindings must stay valid/stable; published profile/fields may change
// through callbacks as observed by the native reloads. Original frame/fault/
// exception/stack-exhaustion identity, concurrent equivalence and register-ABI
// adaptation are not supplied. Existing formatter/vendor-context qualification
// remains a dependency; this is not a runtime activation claim.
void __cdecl report_profile_node_00c320d0(
    DynProfileNodeStorage& node, DynProfileReportSink sink,
    volatile std::uint32_t& first_position,
    volatile std::uint32_t& second_position,
    const DynProfileScopeContext& profile_context,
    const volatile double& actual_percent_scale_00d7a220);

}  // namespace bsp::cc16_dyn_profile_report

#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual string-tree lower bounds require MSVC Win32.
#endif

namespace bsp {
class NativeCrtStricmpCallees;

// Whole0069CE30 and0069CDB0: each83B/36instructions. Native ECX tree,
// EDX unconsumed, ONE stacked key-header pointer, EAX selected NODE/RET4.
// No iterator/output argument. Source adds a borrowed actual0109DE1C cell
// and the EXISTING required CRT service object: RET0Ch, a new Source ABI.
// The stored Ghidra undefined(void) signatures are incomplete, not repaired.
//
// Existing CRT parent PROVIDED-CALLEE DOMAIN: actual BF7FBF Source preserves
// zero/nonzero mode, errno/invalid-handler and locale-service requirements.
// No global/default adapter/new provider interface, current-stricmp/443D00 or
// ASCII-only substitution is introduced. Keep actual cell/services/storage
// live; concrete Original locale/TLS/CRT ownership remains outside this scope.
//
// Separate fixed-nil21/29 bodies. Capture head/root once; nil root returns
// captured head BEFORE key/header reads. CURRENT counts0/C distinguish empty
// headers without data reads. Both nonempty: capture key.data4 BEFORE node.data10,
// call parent(node-data,key-data,samecell/services), signednegative -> CURRENT
// right8, otherwise candidate=CURRENTnode BEFORE CURRENTleft0. Test new nil
// after descent. No head reload, length bound, owner/null/iterator guard,
// topology/count write, payload cleanup, local EH or rollback is added.
void* __fastcall lower_bound_native_input_controller_tree_0069ce30(
    void* actual_tree, std::uint32_t unused_edx, const void* actual_key_header,
    const volatile std::int32_t& actual_locale_changed_0109de1c,
    NativeCrtStricmpCallees& callees);
void* __fastcall lower_bound_native_input_preset_tree_0069cdb0(
    void* actual_tree, std::uint32_t unused_edx, const void* actual_key_header,
    const volatile std::int32_t& actual_locale_changed_0109de1c,
    NativeCrtStricmpCallees& callees);

// Names are hypotheses; established STL/CRT library identities are retained.
// Provided-callee/domain effects and Source private arguments differ from
// Original CRT ownership/frames/registers/flags/heap/FH3/SEH/RTTI/fault identity.
// No drop-in ABI or game equivalence is established.
} // namespace bsp

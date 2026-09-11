#pragma once
#include "bsp/gui_text_lifetime.hpp"

namespace bsp {
// Required CURRENT native CRT __stricmp/00BF7FBF contract and locale. Called
// only for equal nonzero lengths. No ASCII fallback or copied name index.
using GuiNativeNameCompare = int (*)(const char*, const char*);

// Complete00B6D800: ECX actual node, EAX SAME native8h header at+54, RET.
// No retained reference, name copy or mutation. Caller retains the node.
const NativeString& native_node_name_00b6d800(const NativeNodeStorage&) noexcept;

// Actual-name overload of00AA7E00: ECX parent, name and UNUSED DWORD stack,
// RET8. Walk the SAME borrowed GUI+64/+68 list (transform.children), including
// duplicate entries, skip null node slots, compare current node name length
// then actual CRT, return first matching widget. No recursion or key snapshot.
// Canonical owners/valid native headers must outlive this call. The native
// list-node/iterator ABI and corrupt-list CRT termination are not reproduced.
// The required CRT comparer must preserve its ordinary comparison contract,
// including not mutating the GUI list while this traversal is active.
GuiLayoutWidget* find_child_by_name_00aa7e00(GuiWidgetOwnerRuntime&,
    GuiLayoutWidget& parent, const NativeString& name, std::uint32_t unused,
    GuiNativeNameCompare);

struct GuiTextTypeDispatchServices {
    GuiTextBufferServices& buffers; // SAME actual widget/geometry/string domains.
    GuiNativeNameCompare compare_names_00bf7fbf;
};

// Complete supported00AB7700, Text current74. ECX Text/no stack/RET. Capture
// main model BEFORE allocation, unconditionally create/associate actual mesh,
// release creator, then existing AA7DC0 with zero XYZ. No base bounds flag gate.
// Actual allocation failure/SEH is outside the supported successful domain.
void construct_gui_text74_00ab7700(GuiTextLifetime&, GuiTextTypeDispatchServices&);

// Full raw00AB6AA0..6AA4: JMP00AA7170. Base implementation dispatches the
// CURRENT type60(false), then AA70E0. The eventual Text implementation must
// bind its current60 to the concrete function below; no factory is enabled here.
void load_gui_text78_00ab6aa0(GuiTextLifetime&, GuiTextTypeDispatchServices&);

// Complete00AB87D0 for native Boolean inputs0/1; ECX Text, DWORD stack, RET4.
// Native names use the actual NativeString pool. Capture left, clean its name;
// capture right, clean its name; only when BOTH exist call left then right
// CURRENT virtual34 with the same flag. Right dispatch reload follows left's
// callback. Captured widgets must survive those callbacks. Does NOT write the
// Text/base active byte, and never substitutes a missing arrow or child owner.
void set_gui_text_active60_00ab87d0(GuiTextLifetime&, bool,
    GuiTextTypeDispatchServices&);

// Text D5C6C8+38/+3C are inherited A9E0D0/A9E100; use existing owner methods
// base_is_visible38_00a9e0d0/base_visibility_changed3c_00a9e100. Their real
// bodies read current node+AC >0 (null/unordered false), and RET4 respectively.
// This module intentionally provides no incomplete GuiWidgetTypeImplementation
// or factory. Properties18, full content/size/current70, deleting ownership and
// actual scene-release composition must be ready before enabling Text creation.
} // namespace bsp

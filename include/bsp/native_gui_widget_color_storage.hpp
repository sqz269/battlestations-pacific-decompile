#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw GUI color storage requires MSVC Win32.
#endif

namespace bsp {
// Whole AA6870 [AA6870,AA68EA),122B/44 starts. Native ECX actual widget,
// stack original RGBA pointer, RET4. This ordinary C++ pointer/pointer API is
// new; it does not replace that register/private ABI or provide SEH handling.
//
// Require genuinely constructed, live DWORD-aligned actual widget storage
// through+5F, a nonnull genuinely constructed actual Model at+4C with live
// native field storage through+183 (not a Group's capacity at+180). Geometry
// reads its actual+180 DWORD directly, without creating a typed tail view.
// Require valid reached mesh/element/material storage and an original readable RGBA
// quartet. Keep the complete independent graph/backing alive through return.
// Actual object/provider storage is disjoint; RGBA may overlap their live
// fields, including widget color or material diffuse. Its words are read/stored in order, then read
// AGAIN from that SAME original pointer for the diffuse stores. No snapshot.
// Inputs cannot reside in mutable callee scratch; access is externally
// serialized. No ownership is added or inferred from raw field identities.
//
// Capture widget+4C between the initial Z read/store. Test that captured model's
// actual+180, then reload widget+4C for EACH reached geometry getter. Existing
// concrete model/mesh/element/material getters are noexcept and have no
// callback service. This API adds no throwing provider, semantic GUI owner,
// validation/count gate, retain/release, dirty flag, recursion, FP conversion,
// cleanup or rollback. Malformed pointers, hardware faults and Native private
// exception behavior are outside this valid-storage Source contract.
void set_native_gui_widget_color_storage_00aa6870(void* actual_widget,
    const void* original_rgba) noexcept;
} // namespace bsp

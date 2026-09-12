#pragma once
// Complete 004F1360..004F1386, BSP_Landscape_IsKindOf.
// Native: ECX entity; stack requested class; EAX 0/1; RET4. No callees.
// The constructor 004F11C0 installs vtable00CEA090 and stamps entity+C4h=44h;
// vtable+5Ch at00CEA0EC contains004F1360. See docs/SCENE_LANDSCAPE_CLASS.md.
#include <cstdint>

namespace bsp {

// Explicit C++ interface over the native entity+C4h value. Invoke only after
// resolving dispatch to the Landscape vtable. This is not a generic predicate
// for an arbitrary scene record, nor a native object/creator reconstruction.
// Preserves the dynamic equality branch, including values outside the class
// table. Only the returned EAX value is reproduced, not native pointer reads
// or the __thiscall ABI.
std::uint32_t scene_landscape_is_kind_of_004f1360(
    std::int32_t dynamic_class_id, std::int32_t requested_class) noexcept;

} // namespace bsp

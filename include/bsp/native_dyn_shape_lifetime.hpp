#pragma once
#include "bsp/dyn_body_creation.hpp"

namespace bsp {
// Complete29B 004062C0..004062DC. Native ECX shape,stack flags,RET4,
// EAX captured shape. Stamp D7A04C base Shape profile; low flags bit0
// returns the slot through existing00408040 using its ORIGINAL convex pool.
// Shape+210 mesh is borrowed; links and other fields remain stale. The pool
// and pages outlive this operation. No general shape or runtime table is made.
// Explicit source interface,not original register ABI or native SEH behavior.
DynConvexShapeStorage* delete_native_dyn_convex_shape_004062c0(
    DynConvexShapeStorage&,std::uint32_t flags,DynConvexShapePoolStorage&);
} // namespace bsp

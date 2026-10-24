#pragma once

// cc13 pool06 (segment 33): three command-class name leaves and the ConvexObject box builder.
// Build only: no host path calls these. Descriptive names are hypotheses.

namespace bsp {

// Slot +4h of the moveonpath / Leave / disband command-class vtables (00CFB63C / 00CFB660 /
// 00CFB67C), __thiscall(this) with `this` unused, RET: MOV EAX,imm32 / RET. The C++ returns
// the same text, not the .rdata address.
const char* command_class_moveonpath_name_006f8ef0() noexcept;  // 00CFB654 "moveonpath"
const char* command_class_leave_name_006f9000() noexcept;       // 00CFB674 "Leave"
const char* command_class_disband_name_006f90a0() noexcept;     // 00CFB690 "disband"

// ConvexObject +14h..+2Bh as 006F9EE0 writes them: the box centre and half extents.
struct ConvexObjectBox {
    float centre[3]{};       // +14h, +18h, +1Ch
    float half_extent[3]{};  // +20h, +24h, +28h
};

// 006F9EE0, __thiscall(object /*ECX*/, const float* points, int count), RET 8. Called once, by
// the ConvexObject payload reader 006FAD70 (006FAE89: ECX = EDI, push count, push points).
// `points` are packed xyz triples (stride 0Ch). Writes only `object.centre`/`half_extent`.
void convex_object_compute_box_006f9ee0(ConvexObjectBox& object, const float* points,
                                        int count) noexcept;

}  // namespace bsp

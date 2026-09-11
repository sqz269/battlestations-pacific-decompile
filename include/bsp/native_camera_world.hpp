#pragma once

namespace bsp {
// B6DB70[80], ECX actual node, RET. Required ORIGINAL storage: +30 contains
// another actual node address (or zero), +5C flags, +B0 local64, +F0 world64.
// Capture parent once; refresh it recursively when its low flag byte lacks
// bit2; compose local * captured parent.world, or copy local for no parent.
// OR current own DWORD flags with2 only after the matrix provider returns.
// There is no own-cache fast return, cycle/null validation or exception rollback.
// Incidental native EAX is captured parent.world, or own.world for a root;
// the void API does not present either as a semantic getter result.
void __fastcall refresh_native_camera_world_00b6db70(void* actual_node);

// NativeNodeStorage/NativeCameraOwner currently store CameraTransform companion
// pointers in their hierarchy words. They are NOT valid raw parent chains for
// this interface, even though matrix/flag offsets match. Do not reinterpret
// those companions, temporarily swizzle the shared words, or claim null-parent
// cases establish general compatibility. The packet documents the migration
// boundary. No new context, callback, node overlay or hierarchy adapter exists.
} // namespace bsp

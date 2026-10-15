#pragma once

namespace bsp {

// Complete 004682B0..004682E6: no native arguments, EAX actual14h head, RET.
// Existing real current-CRT allocator is nonnull-or-throw. Preserve all three
// conditional link zero stores, color10=1 and nil11=0. PayloadC..F and
// padding12/13 are untouched; no placement call, callback or cleanup is added.
void* allocate_native_scene_integer_set_head_004682b0();

// Complete 00468EA0..00468ECC: ECX actual tree, two unread native stack words,
// RET8/EAX original tree, ESI preserved. Publish head, set captured nil11=1,
// then read CURRENT tree+4 separately before parent/left/right self-links and
// zero count+8 last. Tree+0 is untouched. The new C++ interface omits the two
// unread words; no previous-head cleanup, rollback or original ABI is claimed.
void* construct_native_scene_integer_set_head_00468ea0(void* actual_tree);

} // namespace bsp

#pragma once

namespace bsp {

// Complete 00468260..00468296: no native arguments; EAX actual1Ch raw head;
// RET. Use the existing real current-CRT malloc/new-handler retry service.
// On return, conditionally zero native link addresses+0/+4/+8, set color18=1
// and nil19=0. PayloadC..17 and padding1A/1B are untouched. The allocator is
// nonnull-or-throw; no null fallback, placement call or local cleanup is added.
void* allocate_native_scene_record_map_head_00468260();

// Complete 00468E70..00468E9C: ECX actual tree; two unread stacked words;
// RET8/EAX original tree; ESI preserved. Publish the allocation at tree+4,
// set captured head.nil19=1, then reread CURRENT tree+4 independently before
// parent/left/right self-links, and finally count+8=0. Tree+0 is untouched.
// No tree lifetime wrapper, prior-head cleanup or exception rollback. This
// ordinary C++ interface omits the unread words and is not original ABI parity.
void* construct_native_scene_record_map_head_00468e70(void* actual_tree);

} // namespace bsp

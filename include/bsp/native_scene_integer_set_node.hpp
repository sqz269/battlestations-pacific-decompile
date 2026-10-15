#pragma once

namespace bsp {

// Complete 004681B0..004681E8: no register input; five native stack words
// left,parent,right,key-pointer,color; RET14/EAX allocation. The explicit
// actual_parameters20 binds those five current DWORD cells at0/4/8/C/10.
// Allocate real14h storage BEFORE reading any cell. Preserve interleaved
// captures/stores, including key dereference after link writes and color LOW
// byte before keyC write; nil11=0 and padding12/13 untouched. Full null branch
// remains although the genuine current-CRT allocator is nonnull-or-throw.
// Backing remains valid through the allocator/new handler and reached stores;
// valid payload aliases are admitted. No implicit physical-stack coincidence,
// new callback, nominal enum, local EH or cleanup is introduced by this API.
void* allocate_native_scene_integer_set_node_004681b0(const void* actual_parameters20);

} // namespace bsp

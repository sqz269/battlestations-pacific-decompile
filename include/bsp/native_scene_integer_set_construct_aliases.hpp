#pragma once

namespace bsp {

// Complete 00468940..00468968: ECX actual tree, RET, no stacked words, ESI
// preserved. Expose the raw final EAX: the current head captured for the right
// self-link. No live caller establishes a semantic return type. Allocate via
// actual 004682B0 before tree writes, publish head, set captured nil11=1, reload
// current head for each parent/left/right self-link, then zero count+8 last.
// Tree+0 survives; no previous-head cleanup, rollback or extra EH is supplied.
void* construct_native_scene_integer_set_head_00468940(void* actual_tree);

// Complete 0046CF10..0046CF3A: ECX actual tree, RET/EAX original tree, no stacked
// words, ESI preserved. Same memory/result schedule as accepted 00468EA0. That
// native variant has two unread stack words and RET8; its ordinary Source API
// omits them explicitly. Reusing that Source here does not claim a native call
// to 00468EA0: both native aliases call 004682B0 directly.
void* construct_native_scene_integer_set_head_0046cf10(void* actual_tree);

// Actual 14h head/current-CRT allocation and reached storage must be valid.
// No implicit compiler/private-frame aliases or concurrency are supplied. These
// ordinary Source interfaces do not establish original register/stack/CRT ABI,
// fault-address identity or game behavior.

} // namespace bsp

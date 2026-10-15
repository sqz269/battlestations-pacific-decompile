#pragma once

namespace bsp {
// Complete 00468900, ECX tree, RET. Expose raw final EAX (the head captured
// for the right self-link); no live caller establishes a semantic return type.
// Allocate through actual 00468260 before all tree writes, publish tree.head,
// set allocated nil19, then reload current head before each self-link and
// zero count. Tree+0 survives; no old-head cleanup or rollback is introduced.
void* construct_native_scene_record_map_head_00468900(void* actual_tree);

// Complete 0046F130, ECX tree, RET/EAX original tree. Identical allocation and
// memory schedule to accepted 00468E70, whose two unread stack words/RET8 are
// omitted by both ordinary Source interfaces. Reuse is a Source implementation
// fact, not a claim that native F130 calls E70.
void* construct_native_scene_record_map_head_0046f130(void* actual_tree);

// Reached storage uses actual raw layout/current-CRT allocation. These Source
// interfaces do not establish original register/stack/CRT ABI or game behavior.
} // namespace bsp

#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native subtree invalidation requires MSVC Win32 assembly.
#endif

namespace bsp {

// Full 0042ED50..0042ED75: ECX actual node, no stack arguments or result,
// ESI preserved, plain RET. The descriptive name is not a recovered symbol.
// Capture child +48 before clearing bytes +C8 and +10C; recurse, then reload
// that same child's sibling +44. No complete node type or size is implied.
// All reached storage must remain valid, including each child's post-call
// sibling read. Normal termination requires a finite terminating link walk.
// Null/invalid storage can fault; earlier writes remain. No ownership, cycle
// guard, recovery or new exception policy is supplied by this helper.
void __fastcall invalidate_native_subtree_pose_0042ed50(void* actual_node);

} // namespace bsp

#pragma once

#if defined(_MSC_VER) && defined(_M_IX86)

namespace bsp {

// Qualified actual-pointer reconstruction of 009035E0 (42 bytes, 16 instructions).
// ECX is the actual node. A nonzero full DWORD at +50h causes recursive calls on
// the freshly read +48h pointer; +50h is reloaded after each complete child call.
// The node's then-current virtual slot 0 receives DWORD 1 by an ordinary CALL.
// That target must consume its four-byte argument and preserve the native callee-
// saved registers. Parent storage must survive child returns, and real hierarchy
// mutation must provide progress. After its own virtual call, this body only
// restores ESI and returns; it does not read its receiver again.
// No typed result, ownership, unlink/free policy, exception recovery, or runtime
// compatibility is established. The caller supplies all real objects and targets.
void __fastcall retire_native_world_child_subtree_009035e0(void* actual_node);

} // namespace bsp

#endif

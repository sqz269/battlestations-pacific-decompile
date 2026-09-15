#pragma once
#include "bsp/gui_page_root.hpp"

namespace bsp {
// B866C0..B8671E and B86780..B867DE: incoming ECX is unused, one actual8h
// name header on the stack, EAX actual owner or null, RET4. These are the
// D63210 plain-node and D63220 group factory slot04 bodies selected by B891A0.
// New C++ interfaces; callers still bind the returned owner to their canonical
// scene/lifetime companions. No reference is added and no root is published.

// Uses the actual initialized38h plain-node pool0108FF58 and178h slots.
// FH3 CC2518/DFB944 returns the captured failed slot through B6E670.
NativeNodeStorage* create_native_plain_node_00b866c0(void* actual_pool_0108ff58,
    const void* actual_name_header, NativeStringRawPoolContext&,
    const NativeNodeRawConstants&);

// Uses the already bound actual010902F4 group pool through B8F450 and18Ch
// slots. FH3 CC2558/DFB99C returns the captured failed slot through B8EEB0.
// The same current string-pool and constant cells feed the full group/base
// constructors. This does not create a replacement group pool or type registry.
NativeNodeStorage* create_native_resource_group_00b86780(const void* actual_name_header,
    NativeStringRawPoolContext&, const NativeNodeRawConstants&);
} // namespace bsp

#pragma once
#include "bsp/native_game_profile_lifetime.hpp"
#include <cstdint>
namespace bsp {
// Borrowed observations. A failure leaves the original partial graph in place;
// no rollback, ownership transfer or repeatability of failed destruction implied.
struct NativeGameTreeLifetimeProgress {
    void* owner{};
    void* cursor{};
    std::uint32_t native_site{};
};
// Complete53B bodies,ECX tree,stack node,RET4.18h nodes,nil15;recurse right,
// capture left after recursion,before nodefree,then follow captured left.
// Both preserve payload bytes;their distinct native instantiations stay named.
void erase_native_game_tree_subtree_004c18d0(void*,void*,NativeGameProfileLifetimeContext&,NativeGameTreeLifetimeProgress&);
void erase_native_game_tree_subtree_004c1910(void*,void*,NativeGameProfileLifetimeContext&,NativeGameTreeLifetimeProgress&);
// Complete82B,ECX tree,stack node,RET4.28h nodes,nil25. After right recursion,
// capture string data1C BEFORE left,then length18+1 before getter/return.
// Free node and follow captured left;other payload fields remain untouched.
void erase_native_game_string_payload_subtree_004d16c0(void*,void*,NativeGameProfileLifetimeContext&,NativeGameTreeLifetimeProgress&);
// Consumed full-range branches of three201B library entries. Require both
// owners==tree,first==currenthead.left,last==currenthead;partial erase not exposed.
// Native ECX tree;five stack arguments;EAX output;RET14. Source keeps current
// head reloads and output-owner-before-node stores,not native private-stack ABI.
void* clear_native_game_tree_full_range_004d2000(void*,void*,void*,void*,void*,void*,NativeGameProfileLifetimeContext&,NativeGameTreeLifetimeProgress&);
void* clear_native_game_tree_full_range_004d22f0(void*,void*,void*,void*,void*,void*,NativeGameProfileLifetimeContext&,NativeGameTreeLifetimeProgress&);
void* clear_native_game_tree_full_range_004d41a0(void*,void*,void*,void*,void*,void*,NativeGameProfileLifetimeContext&,NativeGameTreeLifetimeProgress&);
} // namespace bsp

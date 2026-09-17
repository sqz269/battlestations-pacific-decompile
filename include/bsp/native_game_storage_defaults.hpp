#pragma once
#include "bsp/native_game_array_elements.hpp"
namespace bsp {
// Allocation sizes and flag offsets are native storage contracts. Payload and
// padding remain allocator-owned. Trees become sentinels in the calling parent.
void* allocate_native_game_tree_004c2700(NativeGameArrayCalls&);
void* allocate_native_game_tree_004c2750(NativeGameArrayCalls&);
void* allocate_native_game_tree_004c27a0(NativeGameArrayCalls&);
void* allocate_native_game_tree_004c2830(NativeGameArrayCalls&);
void* allocate_native_game_tree_004c26b0(NativeGameArrayCalls&);
void* allocate_native_game_list_004c1950(NativeGameArrayCalls&);
void* allocate_native_game_list_004c1a40(NativeGameArrayCalls&);
void* allocate_native_mission_lua_list_00884830(NativeGameArrayCalls&);
// Raw adapter of the existing typed race initializer. Writes only 00/08/0C;
// does not create a nontrivial RaceRecord over unconstructed game storage.
void* initialize_native_race_storage_007ff9d0(void*) noexcept;
// 36B normal native body 8882D0..8882F3, ECX receiver/EAX result, RET.
// Receiver is 14h bytes. Write vptr, allocate 2Ch self-linked head, publish
// head+0C, clear count+10 and Lua pointer+04. Preserve allocator word+08.
// On allocation failure only the vptr is changed. Native unwinding/ABI open.
void* construct_native_mission_lua_owner_008882d0(void*,NativeGameArrayCalls&);
} // namespace bsp

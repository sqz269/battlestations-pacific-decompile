#include "bsp/native_game_storage_defaults.hpp"
#include "bsp/detail/native_blank_container_storage.hpp"
namespace bsp {
namespace {
using Word=std::uint32_t;
void word(void* p,Word offset,Word value) noexcept {
    *reinterpret_cast<volatile Word*>(reinterpret_cast<Word>(p)+offset)=value;
}
void* tree(Word bytes,Word color,NativeGameArrayCalls& c){
    return detail::allocate_blank_tree_storage(bytes,color,[&](Word n){return c.allocate_00bf681b(n);});
}
void* list(Word bytes,NativeGameArrayCalls& c){
    return detail::allocate_self_linked_list_storage(bytes,[&](Word n){return c.allocate_00bf681b(n);});
}
}
void* allocate_native_game_tree_004c2700(NativeGameArrayCalls& c){return tree(0x14,0x10,c);}
void* allocate_native_game_tree_004c2750(NativeGameArrayCalls& c){return tree(0x28,0x24,c);}
void* allocate_native_game_tree_004c27a0(NativeGameArrayCalls& c){return tree(0x18,0x14,c);}
void* allocate_native_game_tree_004c2830(NativeGameArrayCalls& c){return tree(0x18,0x14,c);}
void* allocate_native_game_tree_004c26b0(NativeGameArrayCalls& c){return tree(0x18,0x14,c);}
void* allocate_native_game_list_004c1950(NativeGameArrayCalls& c){return list(0x24,c);}
void* allocate_native_game_list_004c1a40(NativeGameArrayCalls& c){return list(0xc,c);}
void* allocate_native_mission_lua_list_00884830(NativeGameArrayCalls& c){return list(0x2c,c);}
void* initialize_native_race_storage_007ff9d0(void* p) noexcept {
    word(p,0,0x00d08d20);word(p,8,0);word(p,0xc,0);return p;
}
void* construct_native_mission_lua_owner_008882d0(void* p,NativeGameArrayCalls& calls){
    word(p,0,0x00d0e7a8);
    void* const head=allocate_native_mission_lua_list_00884830(calls);
    word(p,0xc,reinterpret_cast<Word>(head));word(p,0x10,0);word(p,4,0);return p;
}
} // namespace bsp

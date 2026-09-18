#include "bsp/native_game_tree_lifetime.hpp"
#include <cstring>
#include <stdexcept>
namespace bsp {
namespace {
using U=std::uint32_t;using Context=NativeGameProfileLifetimeContext;using Progress=NativeGameTreeLifetimeProgress;
static_assert(sizeof(void*)==4);
void* at(void* p,U n=0) noexcept{return reinterpret_cast<void*>(reinterpret_cast<U>(p)+n);}
U word(void* p,U n=0) noexcept{U v;std::memcpy(&v,at(p,n),4);return v;}
void word(void* p,U n,U v) noexcept{std::memcpy(at(p,n),&v,4);}
void* pointer(void* p,U n=0) noexcept{return reinterpret_cast<void*>(word(p,n));}
void plain_subtree(void* tree,void* node,U recurse,U release,Context& c,Progress& o){
    while(*static_cast<std::uint8_t*>(at(node,0x15))==0){
        o.owner=tree;o.cursor=node;o.native_site=recurse;plain_subtree(tree,pointer(node,8),recurse,release,c,o);
        void* left=pointer(node);o.cursor=node;o.native_site=release;c.calls.free_00bf65ac(node);node=left;
    }
}
using Erase=void(*)(void*,void*,Context&,Progress&);
void* full_range(void* tree,void* output,void* fo,void* first,void* lo,void* last,Context& c,Progress& o,U site,Erase erase){
    if(!tree||fo!=tree||lo!=tree||first!=pointer(pointer(tree,4))||last!=pointer(tree,4))throw std::invalid_argument("native game tree adapter requires its current full range");
    o.owner=tree;o.native_site=site;erase(tree,pointer(pointer(tree,4),4),c,o);
    void* head=pointer(tree,4);word(head,4,reinterpret_cast<U>(head));head=pointer(tree,4);word(tree,8,0);word(head,0,reinterpret_cast<U>(head));
    head=pointer(tree,4);word(head,8,reinterpret_cast<U>(head));head=pointer(tree,4);void* current_first=pointer(head);
    word(output,0,reinterpret_cast<U>(tree));word(output,4,reinterpret_cast<U>(current_first));return output;
}
}
void erase_native_game_tree_subtree_004c18d0(void* t,void* n,Context& c,Progress& o){plain_subtree(t,n,0x004c18e7,0x004c18ef,c,o);}
void erase_native_game_tree_subtree_004c1910(void* t,void* n,Context& c,Progress& o){plain_subtree(t,n,0x004c1927,0x004c192f,c,o);}
void erase_native_game_string_payload_subtree_004d16c0(void* tree,void* node,Context& c,Progress& o){
    while(*static_cast<std::uint8_t*>(at(node,0x25))==0){
        o.owner=tree;o.cursor=node;o.native_site=0x004d16d7;erase_native_game_string_payload_subtree_004d16c0(tree,pointer(node,8),c,o);
        void* data=pointer(node,0x1c);void* left=pointer(node);o.cursor=node;
        if(data){U size=word(node,0x18)+1;o.native_site=0x004d16ef;auto* pool=c.calls.profile_pool_00419cc0(c.strings);
            o.native_site=0x004d16f6;c.calls.profile_return_00bd1510(pool,data,size,c.strings);}
        o.native_site=0x004d16fc;c.calls.free_00bf65ac(node);node=left;
    }
}
void* clear_native_game_tree_full_range_004d2000(void* t,void* out,void* fo,void* f,void* lo,void* l,Context& c,Progress& o){return full_range(t,out,fo,f,lo,l,c,o,0x004d204a,erase_native_game_tree_subtree_004c18d0);}
void* clear_native_game_tree_full_range_004d22f0(void* t,void* out,void* fo,void* f,void* lo,void* l,Context& c,Progress& o){return full_range(t,out,fo,f,lo,l,c,o,0x004d233a,erase_native_game_tree_subtree_004c1910);}
void* clear_native_game_tree_full_range_004d41a0(void* t,void* out,void* fo,void* f,void* lo,void* l,Context& c,Progress& o){return full_range(t,out,fo,f,lo,l,c,o,0x004d41ea,erase_native_game_string_payload_subtree_004d16c0);}
} // namespace bsp

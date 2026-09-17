#include "bsp/native_game_embedded_state.hpp"
#include "bsp/native_renderer_worker_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native game embedded state requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word=std::uint32_t;
using Op=NativeGameEmbeddedStateOperation;
static_assert(sizeof(void*)==4);
void* at(void* p,Word n) noexcept {return reinterpret_cast<void*>(reinterpret_cast<Word>(p)+n);}
template<class T>T read(void* p,Word n=0) noexcept {return *static_cast<volatile T*>(at(p,n));}
template<class T>void put(void* p,Word n,T v) noexcept {*static_cast<volatile T*>(at(p,n))=v;}
void word(void* p,Word n,Word v=0) noexcept {put(p,n,v);}
void byte(void* p,Word n,std::uint8_t v=0) noexcept {put(p,n,v);}
void row(void* p,Word offset,Word constant) noexcept {
    word(p,offset);byte(p,offset+0x10);word(p,offset+0x18);word(p,offset+0x14);
    word(p,offset+4);word(p,offset+8);word(p,offset+0xc,constant);
}
Word subtract(Word left,const volatile Word& right) noexcept {
    const volatile Word* address=&right;Word result;
    __asm {
        movss xmm0,left
        mov eax,address
        subss xmm0,dword ptr [eax]
        movss result,xmm0
    }
    return result;
}
} // namespace
void* NativeGameEmbeddedStateCalls::call_00bd1860(){return create_native_tracked_critical_section_00bd1860();}
void* NativeGameEmbeddedStateCalls::allocate_00bf55be(Word n){return singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots,n,n});}
void NativeGameEmbeddedStateCalls::free_00bf6989(void* p){singleton_lifetime_free(p);}
NativeGameEmbeddedStateOperation::~NativeGameEmbeddedStateOperation(){
    if(phase==Phase::running||phase==Phase::failed)std::terminate();
}
void NativeGameEmbeddedStateOperation::acknowledge_diagnostic_cleanup() noexcept {
    if(phase==Phase::failed)phase=Phase::diagnostic_retired;
}
void reset_native_game_peer_rows_007868c0(void* p,std::uint8_t mode,
    const NativeGameEmbeddedStateConstants& c) noexcept {
    byte(p,0xe0,mode);word(p,0xe4,c.bits_00d042f8);word(p,0xe8);
    const Word constant=c.bits_00d7a24c;byte(p,0xe1);
    for(Word offset=0;offset<0xe0;offset+=0x1c)row(p,offset,constant);
}
void* construct_native_game_peer_rows_00786a80(void* p,
    const NativeGameEmbeddedStateConstants& c) noexcept {
    const Word constant=c.bits_00d7a24c;
    // First record has a distinct store order before the common reset call.
    word(p,0x18);word(p,0x14);word(p,0);byte(p,0x10);
    word(p,4);word(p,8);word(p,0xc,constant);
    for(Word offset=0x1c;offset<0xe0;offset+=0x1c)row(p,offset,constant);
    reset_native_game_peer_rows_007868c0(p,0,c);return p;
}
void reset_native_game_peer_pairs_00778590(void* p) noexcept {
    for(Word index=0;index<8;++index){
        byte(p,0x80+index);word(p,index*8+0x40);word(p,index*8+0x44);
        word(p,index*8);word(p,index*8+4);
    }
}
void* construct_native_game_peer_pairs_00778610(void* p) noexcept {
    reset_native_game_peer_pairs_00778590(p);return p;
}
void* construct_native_game_embedded_state_0076ede0(void* p,
    const NativeGameEmbeddedStateConstants& c,NativeGameEmbeddedStateCalls& calls,Op& a){
    if(a.phase!=Op::Phase::fresh)throw std::logic_error("embedded game construction is one-shot");
    a.owner=p;a.phase=Op::Phase::running;
    try{
        word(p,0,0x00d039cc);a.native_site=0x0076ee07;
        construct_native_game_peer_rows_00786a80(at(p,8),c);
        a.native_site=0x0076ee12;construct_native_game_peer_pairs_00778610(at(p,0x190));
        word(p,0x238,15);word(p,0x234);byte(p,0x224);word(p,0x24c);word(p,0x250);
        a.unwind_state=0;word(p,0x254);word(p,0x25c);word(p,0x260);word(p,0x264);
        const Word timer=c.bits_00d7a260;word(p,0x278,timer);word(p,0x294,timer);
        const Word left=c.bits_00d7a208;
        word(p,0xf4);word(p,0xf8);word(p,0x188);word(p,0x18c);word(p,0x23c,0xffffffff);
        byte(p,0x290);byte(p,0x274);put<std::uint16_t>(p,0x244,0);word(p,0x248,2);
        byte(p,0x100);byte(p,0x125,0xff);byte(p,0x168);byte(p,0x16a,1);
        byte(p,0x127,8);byte(p,0x129);byte(p,0x12d,1);byte(p,0x12c,0x14);
        byte(p,0x128);byte(p,0x12a);byte(p,0x12b);word(p,0x170);word(p,0x174);
        const Word difference=subtract(left,c.bits_00d0de84);a.unwind_state=2;word(p,0x240,difference);
        byte(p,0x27c);word(p,0x284);word(p,0x280);word(p,0x28c);word(p,0x288);word(p,0x258);
        a.native_site=0x0076ef40;put(p,0x298,calls.call_00bd1860());
        if(read<Word>(p,0x254)<0x200){
            word(p,0x254,0x200);a.native_site=0x0076ef79;
            a.current_allocation=calls.allocate_00bf55be(0x800);
            if(read<void*>(p,0x24c)){
                a.copy_index=0;
                if(read<Word>(p,0x250)!=0){
                    do{
                        const Word value=read<Word>(read<void*>(p,0x24c),a.copy_index*4);
                        word(a.current_allocation,a.copy_index*4,value);
                        const Word count=read<Word>(p,0x250);++a.copy_index;
                        if(a.copy_index>=count)break;
                    }while(true);
                }
                void* const old=read<void*>(p,0x24c);a.native_site=0x0076efc0;calls.free_00bf6989(old);
            }
            put(p,0x24c,a.current_allocation);
        }
        byte(p,0x27d);byte(p,0x29c);byte(p,4);
        a.phase=Op::Phase::complete;return p;
    }catch(...){a.phase=Op::Phase::failed;throw;}
}
} // namespace bsp

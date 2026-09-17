#include "bsp/native_game_construction.hpp"
#include "bsp/native_input_configuration_owner.hpp"
#include "bsp/native_lua_objects.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native game construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word=std::uint32_t;
using Op=NativeGameConstructionOperation;
static_assert(sizeof(void*)==4&&sizeof(DynWorldDescriptor)==0x40);
void* at(void* p,Word n) noexcept {return static_cast<std::byte*>(p)+n;}
const void* at(const void* p,Word n) noexcept {return static_cast<const std::byte*>(p)+n;}
template<class T> T read(const void* p,Word n=0) noexcept {
    return *static_cast<const volatile T*>(at(p,n));
}
template<class T> void put(void* p,Word n,T value) noexcept {
    *static_cast<volatile T*>(at(p,n))=value;
}
void word(void* p,Word n,Word value=0) noexcept {put(p,n,value);}
void byte(void* p,Word n,std::uint8_t value=0) noexcept {put(p,n,value);}
// Parent performs these reloads after the allocator. Do not initialize the
// opaque header+0 or payload bytes, or collapse the head to a typed container.
void sentinel(void* header,void* allocation,Word nil_offset) noexcept {
    put(header,4,allocation);byte(allocation,nil_offset,1);
    void* head=read<void*>(header,4);put(head,4,head);
    head=read<void*>(header,4);put(head,0,head);
    head=read<void*>(header,4);put(head,8,head);word(header,8);
}
float spill(const volatile float& value) noexcept {
    const volatile float* p=&value;float result;
    __asm {
        mov eax,p
        fld dword ptr [eax]
        fstp result
    }
    return result;
}
float positive_zero() noexcept {
    float result;
    __asm {
        fldz
        fstp result
    }
    return result;
}
void descriptor_word(DynWorldDescriptor& d,Word offset,Word value) noexcept {
    std::memcpy(static_cast<std::byte*>(static_cast<void*>(&d))+offset,&value,4);
}
}
void NativeGameConstructionCalls::call_0076ede0(void* owner,
    const NativeGameEmbeddedStateConstants& constants,NativeGameEmbeddedStateOperation& operation){
    construct_native_game_embedded_state_0076ede0(owner,constants,*this,operation);
}
void NativeGameConstructionCalls::array_construct_00bf7cd1(void* base,Word stride,Word count,
    Word constructor,Word destructor,const NativeGameArrayConstants& constants,NativeGameArrayOperation& operation){
    construct_native_game_array_00bf7cd1(base,stride,count,constructor,destructor,constants,*this,operation);
}
NativeGameConstructionOperation::~NativeGameConstructionOperation(){
    if(phase==Phase::running||phase==Phase::failed)std::terminate();
}
void NativeGameConstructionOperation::acknowledge_diagnostic_cleanup() noexcept {
    if(phase==Phase::failed){
        profile.acknowledge_diagnostic_cleanup();embedded.acknowledge_diagnostic_cleanup();
        for(auto& array:arrays)array.acknowledge_diagnostic_cleanup();phase=Phase::diagnostic_retired;
    }
}

NativeGameStorage* construct_native_game_004ddb90(NativeGameStorage& storage,
    const void* name,NativeGameConstructionContext& c,Op& a) {
    if(a.phase!=Op::Phase::fresh)throw std::logic_error("game constructor operation is one-shot");
    a.owner=&storage;a.context=&c;a.phase=Op::Phase::running;
    auto& calls=c.calls;void* const game=&storage;
    try {
        word(game,0,0x00ce7cb8);byte(game,8,1);word(game,0xc,0xffffffff);byte(game,0x10);
        word(game,0x1c,0x00ce78c0);word(game,0x24);word(game,0x28);word(game,0x2c);
        a.unwind_state=0;a.native_site=0x004ddbe2;
        construct_native_input_configuration_00698680(at(game,0x3c));
        a.unwind_state=1;a.native_site=0x004ddc01;
        calls.array_construct_00bf7cd1(at(game,0x560),0x10,5,0x004d3730,0x004cafd0,c.array_constants,a.arrays[0]);
        a.unwind_state=2;a.native_site=0x004ddc13;
        sentinel(at(game,0x5b0),calls.call_004c2700(),0x11);
        a.unwind_state=3;a.native_site=0x004ddc40;
        sentinel(at(game,0x5bc),calls.call_004c2750(),0x25);
        a.unwind_state=4;a.native_site=0x004ddc6d;
        sentinel(at(game,0x5c8),calls.call_004c27a0(),0x15);
        word(game,0x5dc);word(game,0x5e0);word(game,0x5e4);word(game,0x5e8);
        byte(game,0x5ec);word(game,0x5f0);word(game,0x5f4);word(game,0x5f8);
        word(game,0x600);word(game,0x604);a.unwind_state=8;
        byte(game,0x610);byte(game,0x61d);byte(game,0x61e,1);byte(game,0x61f);
        byte(game,0x620);word(game,0x624);a.native_site=0x004ddcfb;
        sentinel(at(game,0x628),calls.call_004c2830(),0x15);
        a.unwind_state=9;byte(game,0x634);byte(game,0x635);a.native_site=0x004ddd34;
        put(game,0x63c,calls.call_004c1950());word(game,0x640);
        a.unwind_state=10;a.native_site=0x004ddd4a;
        construct_native_player_profile_007fee20(at(game,0x650),c.profile,a.profile);
        a.unwind_state=11;a.native_site=0x004ddd6c;
        calls.array_construct_00bf7cd1(at(game,0x748),0x118,8,0x004d6ba0,0x004cb2f0,c.array_constants,a.arrays[1]);
        a.unwind_state=12;a.native_site=0x004ddd8e;
        calls.array_construct_00bf7cd1(at(game,0x1008),0x118,8,0x004d6ba0,0x004cb2f0,c.array_constants,a.arrays[2]);
        a.unwind_state=13;a.native_site=0x004ddda0;
        sentinel(at(game,0x1930),calls.call_004c26b0(),0x15);
        a.unwind_state=14;word(game,0x1940);a.native_site=0x004dddd1;
        calls.call_007ff9d0(at(game,0x1944));
        for(Word offset=0x1964;offset<=0x19c0;offset+=4)word(game,offset);
        byte(game,0x19c4);
        for(Word offset=0x19d4;offset<=0x19e4;offset+=4)word(game,offset);
        a.unwind_state=28;word(game,0x19e8);a.native_site=0x004dde9b;
        construct_native_lua_state_00b66bd0(at(game,0x1a0c));
        a.unwind_state=29;a.native_site=0x004ddeab;
        calls.call_0076ede0(at(game,0x1ef0),c.embedded_constants,a.embedded);
        byte(game,0x2194,1);word(game,0x2198);word(game,0x219c);a.unwind_state=31;
        word(game,0x21f4);word(game,0x21f8);word(game,0x21fc);word(game,0x2200);
        byte(game,0x22dc,1);a.native_site=0x004ddefc;
        calls.array_construct_00bf7cd1(at(game,0x7134),0xc,4,0x004c8140,0x004c4600,c.array_constants,a.arrays[3]);
        void* const destination=at(game,0x7164);word(destination,0);word(destination,4);
        a.unwind_state=33;a.native_site=0x004ddf17;
        put(game,0x7170,calls.call_004c1a40());word(game,0x7174);word(game,0x7178);
        word(game,0x717c);word(game,0x7180);byte(game,0x7184);
        word(game,0x7190);word(game,0x7194);word(game,0x7198);a.unwind_state=36;
        byte(game,0x719c);byte(game,0x719d);byte(game,0x719e);word(game,0x38);byte(game,0x1ee0);
        if(destination!=name) {
            a.native_site=0x004ddf81;
            resize_native_string_header_0041dd40(destination,c.strings,read<Word>(name),true);
            if(read<Word>(name)!=0) {
                const auto count=read<Word>(destination);
                void* const target=read<void*>(destination,4);
                const void* const source=read<const void*>(name,4);
                a.native_site=0x004ddf99;std::memcpy(target,source,count);
            }
        }
        a.native_site=0x004ddfa1;calls.call_008d9150();
        word(game,0x618,4);word(game,0x614,4);byte(game,0x61c);
        a.native_site=0x004ddfbd;void* const configuration=calls.call_00432650();
        a.native_site=0x004ddfc4;calls.call_0087d7b0(configuration);
        a.native_site=0x004ddfc9;calls.call_00717e80();
        a.native_site=0x004ddfd3;a.current_allocation=calls.allocate_00bf681b(0x84);
        a.unwind_state=37;void* value=nullptr;
        if(a.current_allocation){const float argument=positive_zero();a.native_site=0x004ddff0;value=calls.call_0070bd70(a.current_allocation,argument);}
        a.unwind_state=36;c.actual_00e19b0c=value;
        a.native_site=0x004de008;a.current_allocation=calls.allocate_00bf681b(0x84);
        a.unwind_state=38;value=nullptr;
        if(a.current_allocation){const float argument=spill(c.constants.argument_00ce7d20);a.native_site=0x004de029;value=calls.call_0070bd70(a.current_allocation,argument);}
        a.unwind_state=36;c.actual_00e19b08=value;
        a.native_site=0x004de041;a.current_allocation=calls.allocate_00bf681b(0x84);
        a.unwind_state=39;value=nullptr;
        if(a.current_allocation){const float argument=spill(c.constants.argument_00ce7d1c);a.native_site=0x004de062;value=calls.call_0070bd70(a.current_allocation,argument);}
        a.unwind_state=36;c.actual_00e19b04=value;
        a.native_site=0x004de075;calls.call_00727bd0();
        a.native_site=0x004de07c;a.current_allocation=calls.allocate_00bf681b(0x14);
        a.unwind_state=40;value=nullptr;
        if(a.current_allocation){a.native_site=0x004de093;value=calls.call_008882d0(a.current_allocation);}
        put(game,0x1a08,value);word(game,0x5d4);byte(game,0x2193);word(game,0x5fc);
        c.actual_movie_00e18d48=nullptr;word(game,0x19c8);word(game,0x21e0);
        byte(game,0x2191);byte(game,0x2192);word(game,0x18c8,0xffffffff);word(game,0x18ec,0xffffffff);
        word(game,0x7058);byte(game,0x22dd);byte(game,0x1ee5);byte(game,0x1ee3);
        byte(game,0x1ee4);byte(game,0x1ee1);c.actual_game_00e188a8=game;
        a.unwind_state=36;word(game,0x34);a.worker_count=1;
        a.native_site=0x004de11c;a.worker_count=calls.call_00be4800();
        a.native_site=0x004de129;void* const engine=calls.call_00c55f50(&a.worker_count);
        const Word solver=c.constants.bits_00ce7480;
        const Word half=c.constants.bits_00ce3800;
        descriptor_word(a.descriptor,0x30,solver);
        descriptor_word(a.descriptor,0x34,c.constants.bits_00d7a24c);
        descriptor_word(a.descriptor,0x38,c.constants.bits_00ce746c);
        descriptor_word(a.descriptor,0x00,c.constants.bits_00ce7638);
        descriptor_word(a.descriptor,0x2c,c.constants.bits_00d7a2f0);
        descriptor_word(a.descriptor,0x3c,half);
        const Word gravity=c.constants.bits_00ce6848;put(game,0x14,engine);
        descriptor_word(a.descriptor,0x1c,0x14);descriptor_word(a.descriptor,0x20,0);
        descriptor_word(a.descriptor,0x24,10);descriptor_word(a.descriptor,0x10,1);
        descriptor_word(a.descriptor,0x28,0);descriptor_word(a.descriptor,0x04,0);
        descriptor_word(a.descriptor,0x08,gravity);descriptor_word(a.descriptor,0x0c,0);
        descriptor_word(a.descriptor,0x14,0);descriptor_word(a.descriptor,0x18,0);
        a.native_site=0x004de1d3;void* const world=calls.call_00c420e0(engine,a.descriptor);
        put(game,0x18,world);a.native_site=0x004de1de;calls.call_00c31a40(world,at(game,0x1c));
        a.native_site=0x004de1e5;value=calls.allocate_00bf681b(0x20);
        if(value){word(value,4);word(value,8);word(value,0xc);word(value,0x14);word(value,0x18);word(value,0x1c);}
        put(game,0x30,value);
        for(Word offset=0x21a4;offset<=0x21c0;offset+=4)word(game,offset);
        a.unwind_state=36;byte(game,0x1ee7);a.native_site=0x004de245;
        put(game,0x1ee8,calls.call_00bd1860());
        a.phase=Op::Phase::complete;return &storage;
    }catch(...){a.phase=Op::Phase::failed;throw;}
}
} // namespace bsp

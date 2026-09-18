#include "bsp/native_game_lua_globals_lifetime.hpp"
#include "bsp/native_game_profile_lifetime.hpp"
#include "bsp/native_checked_string_storage.hpp"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using U=std::uint32_t;
static_assert(sizeof(void*)==4);
U address(const void* p) noexcept {return reinterpret_cast<U>(p);}
void* pointer(U p) noexcept {return reinterpret_cast<void*>(p);}
U word(const void* p,U n=0) noexcept {U v;std::memcpy(&v,pointer(address(p)+n),4);return v;}
void word(void* p,U n,U v) noexcept {std::memcpy(pointer(address(p)+n),&v,4);}
std::int32_t signed_word(U v) noexcept {std::int32_t result;std::memcpy(&result,&v,4);return result;}
U record_count(U first,U last) noexcept {return static_cast<U>(signed_word(last-first)/12);}
void normal(NativeGameLuaGlobalsLifetimeContext& x,NativeGameProfileLifetimeContext& profile,
    NativeGameLuaGlobalsLifetimeOperation& o) {
    auto& c=x.calls;void* strings=x.actual_strings_0108ff30;void* records=x.actual_records_0108ff40;
    U first=word(strings,4);if(!first)return;
    U end=word(strings,8);if(((end-first)>>3)==0)return;
    const U captured_string_end=end;
    if(first>end) {o.native_site=0xb6cfba;c.lua_globals_invalid_parameter_00bf6713();end=word(strings,8);first=word(strings,4);}
    const U captured_string_first=first;
    if(first>end) {o.native_site=0xb6cfd7;c.lua_globals_invalid_parameter_00bf6713();}
    o.native_site=0xb6cff0;c.lua_globals_erase_strings_004954f0(strings,o.iterator_output,
        pointer(captured_string_first),pointer(captured_string_end),profile);
    U index=0,offset=0;
    for(;;) {
        first=word(records,4);end=word(records,8);o.record_index=index;
        if(!first||index>=record_count(first,end))break;
        void* payload=pointer(word(pointer(first+offset)));o.native_site=0xb6d02c;
        c.lua_globals_free_00bf6989(payload);++index;offset+=12;
    }
    const U captured_record_end=end;
    if(first>end) {
        o.native_site=0xb6d043;c.lua_globals_invalid_parameter_00bf6713();end=word(records,8);first=word(records,4);
        if(first>end) {o.native_site=0xb6d058;c.lua_globals_invalid_parameter_00bf6713();end=word(records,8);}
    }
    if(first!=captured_record_end) {
        o.native_site=0xb6d07e;
        void* new_end=copy_native_game_lua_records_00b6c360(pointer(captured_record_end),pointer(end),pointer(first));
        word(records,8,address(new_end));
    }
}
}
void NativeGameLuaGlobalsLifetimeCalls::lua_globals_invalid_parameter_00bf6713(){_invalid_parameter_noinfo();}
void NativeGameLuaGlobalsLifetimeCalls::lua_globals_free_00bf6989(void* p){std::free(p);}
void NativeGameLuaGlobalsLifetimeCalls::lua_globals_erase_strings_004954f0(void* h,void* output,
    void* first,void* last,NativeGameProfileLifetimeContext& profile){
    erase_checked_native_string_storage(h,output,h,first,h,last,profile.actual_strings);
}
void* copy_native_game_lua_records_00b6c360(const void* first,const void* last,void* destination) noexcept {
    U cursor=address(first),limit=address(last),delta=address(destination)-cursor;
    const U result=address(destination)+record_count(cursor,limit)*12;
    while(cursor!=limit) {
        // Keep each load adjacent to its store, including overlapping records.
        word(pointer(cursor+delta),0,word(pointer(cursor)));
        word(pointer(cursor+delta),4,word(pointer(cursor),4));
        word(pointer(cursor+delta),8,word(pointer(cursor),8));
        cursor+=12;
    }
    return pointer(result);
}
NativeGameLuaGlobalsLifetimeOperation::~NativeGameLuaGlobalsLifetimeOperation(){if(phase==Phase::running||phase==Phase::failed)std::terminate();}
void NativeGameLuaGlobalsLifetimeOperation::acknowledge_diagnostic_cleanup() noexcept {if(phase==Phase::failed)phase=Phase::diagnostic_retired;}
void clear_native_game_lua_globals_00b6cf90(NativeGameLuaGlobalsLifetimeContext& x,
    NativeGameProfileLifetimeContext& profile,NativeGameLuaGlobalsLifetimeOperation& o){
    if(o.phase!=NativeGameLuaGlobalsLifetimeOperation::Phase::fresh)throw std::logic_error("native Lua globals cleanup cannot be replayed");
    o.context=&x;o.profile=&profile;o.native_site=0xb6cf90;o.phase=NativeGameLuaGlobalsLifetimeOperation::Phase::running;
    try {normal(x,profile,o);o.phase=NativeGameLuaGlobalsLifetimeOperation::Phase::complete;}
    catch(...) {o.phase=NativeGameLuaGlobalsLifetimeOperation::Phase::failed;throw;}
}
} // namespace bsp

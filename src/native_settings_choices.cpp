#include "bsp/native_settings_choices.hpp"
#include "bsp/native_input_binding_storage.hpp"
#include "bsp/native_renderer_resolution_enumeration.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native settings choices require MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;
U load(const void* p,U byte_offset=0) noexcept {
    U result;
    __asm {mov eax,p}
    __asm {mov edx,byte_offset}
    __asm {mov eax,dword ptr[eax+edx]}
    __asm {mov result,eax}
    return result;
}
void store(void* p,U byte_offset,U value) noexcept {
    __asm {mov eax,p}
    __asm {mov edx,byte_offset}
    __asm {mov ecx,value}
    __asm {mov dword ptr[eax+edx],ecx}
}
void* pointer(U bits) noexcept {return reinterpret_cast<void*>(bits);}
std::int32_t signed_bits(U bits) noexcept {std::int32_t value;std::memcpy(&value,&bits,4);return value;}
U growth(U capacity) noexcept {const U next=capacity*2;return signed_bits(next)>1?next:1;}
}
void NativeSettingsChoiceCalls::resize_dwords_0086a430(void* p,std::int32_t n){resize_native_input_dwords_0086a430(p,n);}
void NativeSettingsChoiceCalls::reserve_dwords_0086a220(void* p,std::int32_t n){reserve_native_input_dwords_0086a220(p,n);}
void NativeSettingsChoiceCalls::reserve_pairs_008d4750(void* p,std::int32_t n){reserve_native_resolution_pairs_008d4750(p,0,n);}
int NativeSettingsChoiceCalls::compare_language_00bf7fbf(const char* a,const char* b){return ::_stricmp(a,b);}
void* assign_native_settings_dwords_008d4df0(void* target,const void* source,NativeSettingsChoiceCalls& calls) {
    calls.resize_dwords_0086a430(target,0);
    calls.reserve_dwords_0086a220(target,signed_bits(load(source,4)));
    U index=0;
    while(signed_bits(index)<signed_bits(load(source,4))) {
        const U capacity=load(target,8);const U count=load(target,4);
        const void* const row=pointer(load(source)+index*4);
        if(count==capacity)calls.reserve_dwords_0086a220(target,signed_bits(growth(capacity)));
        const U current_count=load(target,4);const U current_data=load(target);
        void* const destination=pointer(current_data+current_count*4);
        if(destination)store(destination,0,load(row));
        store(target,4,load(target,4)+1);++index;
    }
    return target;
}
void* assign_native_settings_pairs_008d4ea0(void* target,const void* source,NativeSettingsChoiceCalls& calls) {
    if(signed_bits(load(target,8))<0)calls.reserve_pairs_008d4750(target,0);
    while(signed_bits(load(target,4))>0)store(target,4,load(target,4)-1);
    store(target,4,0);calls.reserve_pairs_008d4750(target,signed_bits(load(source,4)));
    U index=0;
    while(signed_bits(index)<signed_bits(load(source,4))) {
        const U capacity=load(target,8);const U count=load(target,4);
        const void* const row=pointer(load(source)+index*8);
        if(count==capacity)calls.reserve_pairs_008d4750(target,signed_bits(growth(capacity)));
        const U current_count=load(target,4);const U current_data=load(target);
        void* const destination=pointer(current_data+current_count*8);
        if(destination) {
            const U first=load(row);store(destination,0,first);
            const U second=load(row,4);store(destination,4,second);
        }
        store(target,4,load(target,4)+1);++index;
    }
    return target;
}
NativeSettingsLanguageOperation::~NativeSettingsLanguageOperation(){if(phase==Phase::running||phase==Phase::failed)std::terminate();}
void NativeSettingsLanguageOperation::acknowledge_diagnostic_cleanup() noexcept {phase=Phase::diagnostic_retired;}
void select_native_settings_language_008d56c0(void* settings,const char* name,NativeSettingsLanguageContext& c,NativeSettingsLanguageOperation& op) {
    using Phase=NativeSettingsLanguageOperation::Phase;
    if(op.phase!=Phase::fresh)throw std::logic_error("native language selection operation cannot replay");
    op.phase=Phase::running;
    try {
        op.native_site=0x008d56e9;
        construct_native_string_header_0041e870(op.temporary_string,c.strings,name);
        op.captured_data=static_cast<char*>(pointer(load(op.temporary_string,4)));
        op.captured_length=load(op.temporary_string);
        char* const captured_data=op.captured_data;
        const U captured_length=op.captured_length;
        U index=0;bool found=false;
        if(signed_bits(c.actual_count_00f88978)>0) {
            U table=reinterpret_cast<U>(c.actual_catalog_00f88974);U offset=0;
            do {
                const U length=load(pointer(table+offset));
                if(captured_length==length) {
                    int compared;
                    if(captured_length==0)compared=length? -1:0;
                    else if(length==0)compared=1;
                    else {
                        const auto* data=static_cast<const char*>(pointer(load(pointer(table+offset),4)));
                        op.native_site=0x008d572a;
                        compared=c.calls.compare_language_00bf7fbf(captured_data,data);
                        table=reinterpret_cast<U>(c.actual_catalog_00f88974);
                    }
                    if(compared==0){store(settings,4,index);found=true;break;}
                }
                ++index;offset+=0x20;
            }while(signed_bits(index)<signed_bits(c.actual_count_00f88978));
        }
        if(!found)store(settings,4,0);
        // Original EBX/EBP captures drive return even if the temporary header
        // changes through a service; no automatic NativeString destructor.
        alignas(4) U captured[2]={captured_length,reinterpret_cast<U>(captured_data)};
        op.native_site=0x008d576d;destroy_native_string_header_0041dd20(captured,c.strings);
        op.phase=Phase::complete;
    }catch(...){op.phase=Phase::failed;throw;}
}
} // namespace bsp

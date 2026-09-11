#include "bsp/native_lua_script_overrides.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_physical_file_date.hpp"
#include <cstring>
#include <Windows.h>
namespace bsp {
std::uint32_t reverse_find_native_string_bytes_004bcb80(const NativeString& text,const char* set,std::int32_t start){
    const char* const data=text.data();if(!data)return 0xffffffffu;
    const auto count=text.length();if(!count || !set || start<0)return 0xffffffffu;
    auto index=static_cast<std::uint32_t>(start);if(index>=count)index=count-1u;
    const auto set_size=static_cast<std::int32_t>(std::strlen(set));
    const auto begin=reinterpret_cast<std::uintptr_t>(data);auto cursor=begin+index;
    for(;;){
        for(std::int32_t i=0;i<set_size;++i)if(*reinterpret_cast<const char*>(cursor)==set[i])return static_cast<std::uint32_t>(cursor-begin);
        if(cursor<=begin)return 0xffffffffu;--cursor;
    }
}
const char* native_string_data_or_00419ca0(const NativeString& text,const char* empty) noexcept {return text.data()?text.data():empty;}
void register_native_lua_script_suffix_00bdef80(void* manager,const NativeString& suffix,NativeStringStorage& strings){
    append_native_string_vector_004cdc20(*reinterpret_cast<NativeStringVectorStorage*>(static_cast<char*>(manager)+0x48),suffix,strings);
}
namespace {
void append_candidate(void* manager,const NativeString& stem,const NativeString& extension,const NativeString& suffix,
    NativeStringVectorStorage& output,NativeStringStorage& strings){
    NativeString underscore;underscore.assign_0041e870(strings,"_");
    char* const underscore_data=underscore.data();const auto underscore_length=underscore.length();
    NativeString prefix,with_suffix,candidate;
    bool underscore_live=true,prefix_live=false,suffix_live=false,candidate_live=false;
    __try {
        concatenate_native_string_headers_004261a0(&stem,&prefix,&underscore,strings);prefix_live=true;
        concatenate_native_string_headers_004261a0(&prefix,&with_suffix,&suffix,strings);suffix_live=true;
        concatenate_native_string_headers_004261a0(&with_suffix,&candidate,&extension,strings);candidate_live=true;
        suffix_live=false;destroy_native_string_header_0041dd20(&with_suffix,strings);
        prefix_live=false;destroy_native_string_header_0041dd20(&prefix,strings);
        underscore_live=false;if(underscore_data)strings.release(underscore_data,underscore_length+1u);
        using Exists=std::uint8_t(__fastcall*)(void*,void*,NativeString*);
        const auto method=(*static_cast<std::uintptr_t**>(manager))[2];
        if(reinterpret_cast<Exists>(method)(manager,nullptr,&candidate))append_native_string_vector_004cdc20(output,candidate,strings);
    } __finally {
        if(candidate_live)destroy_native_string_header_0041dd20(&candidate,strings);
        if(suffix_live)destroy_native_string_header_0041dd20(&with_suffix,strings);
        if(prefix_live)destroy_native_string_header_0041dd20(&prefix,strings);
        if(underscore_live)destroy_native_string_header_0041dd20(&underscore,strings);
    }
}
}
void append_native_lua_script_overrides_00bdef90(void* manager,const NativeString& path,NativeStringVectorStorage& output,NativeStringStorage& strings){
    const auto slash=reverse_find_native_string_bytes_004bcb80(path,"/",0x7fffffff);
    auto start=slash==0xffffffffu?0u:slash;std::uint32_t dot=0xffffffffu;
    const char* const data=path.data();
    if(data && path.length()){
        if(static_cast<std::int32_t>(start)<0)start=0;
        if(start<=path.length()){
            const auto found=start+static_cast<std::uint32_t>(std::strcspn(data+start,"."));
            if(found!=path.length())dot=found;
        }
    }
    NativeString stem;
    __try {
        NativeString extension;
        __try {
            if(dot!=0xffffffffu){
                NativeString temporary;construct_native_string_substring_00469840(&path,&temporary,0,dot,strings);
                __try {copy_native_string_header_00be0a30_fragment(&stem,strings,&temporary);}
                __finally {destroy_native_string_header_0041dd20(&temporary,strings);}
                const auto address=reinterpret_cast<std::uintptr_t>(path.data()?path.data():"")+dot;
                const char* const source=reinterpret_cast<const char*>(address);
                const auto length=source?static_cast<std::uint32_t>(std::strlen(source)):0u;
                resize_native_string_header_0041dd40(&extension,strings,length,false);
                if(extension.data())std::memcpy(extension.data(),source,extension.length());
            }else copy_native_string_header_00be0a30_fragment(&stem,strings,&path);
            auto& suffixes=*reinterpret_cast<NativeStringVectorStorage*>(static_cast<char*>(manager)+0x48);
            auto cursor=reinterpret_cast<std::uintptr_t>(suffixes.data_00);
            const auto end=reinterpret_cast<std::uintptr_t>(suffixes.data_00)+static_cast<std::uint32_t>(suffixes.count_04)*8u;
            while(cursor!=end){append_candidate(manager,stem,extension,*reinterpret_cast<NativeString*>(cursor),output,strings);cursor+=8u;}
        } __finally {destroy_native_string_header_0041dd20(&extension,strings);}
    } __finally {destroy_native_string_header_0041dd20(&stem,strings);}
}
} // namespace bsp

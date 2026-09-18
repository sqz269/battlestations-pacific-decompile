#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "bsp/native_scene_tokenizer.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene tokenizer requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;
using Op=NativeSceneTokenizerOperation;
void* at(void* p,U n) noexcept {return reinterpret_cast<void*>(reinterpret_cast<U>(p)+n);}
template<class T=U> T get(void* p,U n) noexcept {T v;std::memcpy(&v,at(p,n),sizeof v);return v;}
void word(void* p,U n,U v) noexcept {std::memcpy(at(p,n),&v,4);}
std::uint8_t& byte(void* p,U n) noexcept {return *static_cast<std::uint8_t*>(at(p,n));}
void begin(void* p,Op& op) {
    if(op.phase!=Op::Phase::fresh)throw std::logic_error("native scene tokenizer operation cannot replay");
    op.owner=p;op.phase=Op::Phase::running;
}
void destroy_body(void* p,NativeSceneTokenizerContext& c,Op& op) {
    word(p,0,0x00d15fe4);op.unwind_state=0;
    void* const delimiters=get<void*>(p,0x818);
    if(delimiters!=c.default_delimiters_00e0c944) {
        op.native_site=0x008d9c6a;c.calls.free_00bf6989(delimiters);
    }
    void* const stream=get<void*>(p,0x828);
    if(stream) {
        op.native_site=0x008d9c81;
        if(InterlockedDecrement(static_cast<volatile LONG*>(at(stream,4)))==0) {
            op.native_site=0x008d9c91;c.calls.stream_zero_reference_slot0(stream,c.memory);
        }
        word(p,0x828,0);
    }
    op.native_site=0x008d9ca4;c.calls.free_00bf6989(get<void*>(p,0x82c));
    op.unwind_state=-1;op.native_site=0x008d9ccc;
    destroy_native_string_header_0041dd20(at(p,0x820),c.strings);
}
void append(void* p,U offset) {std::strncat(static_cast<char*>(at(p,5)),static_cast<const char*>(at(p,offset)),1);}
}
void* NativeSceneTokenizerCalls::allocate_00bf55be(U n) {return singleton_lifetime_allocate({SingletonAllocationKind::object,n,n});}
void NativeSceneTokenizerCalls::free_00bf6989(void* p) {singleton_lifetime_free(p);}
void NativeSceneTokenizerCalls::free_00bf65ac(void* p) {singleton_lifetime_free(p);}
std::int64_t NativeSceneTokenizerCalls::stream_size_slot30(void* p,NativeRetainedMemoryOwnerContext& c) {return dispatch_native_memory_stream_length(p,c);}
void NativeSceneTokenizerCalls::stream_read_slot24(void* p,void* d,U n,U* actual,NativeRetainedMemoryOwnerContext& c) {dispatch_native_memory_stream_read(p,d,n,actual,c);}
void NativeSceneTokenizerCalls::stream_zero_reference_slot0(void* p,NativeRetainedMemoryOwnerContext& c) {dispatch_native_memory_owner_zero_reference(p,c);}
Op::~NativeSceneTokenizerOperation() {if(phase==Phase::running||phase==Phase::failed)std::terminate();}
void Op::acknowledge_diagnostic_cleanup() noexcept {phase=Phase::diagnostic_retired;}

void* construct_native_scene_tokenizer_008d9f20(void* p,void* stream,const char* extra,NativeSceneTokenizerContext& c,Op& op) {
    begin(p,op);
    try {
        word(p,0,0x00d15fe4);word(p,0x820,0);word(p,0x824,0);
        op.unwind_state=0;word(p,0x81c,0);
        std::memset(at(p,5),0,0x400);std::memset(at(p,0x405),0,0x400);
        byte(p,0x805)=0;byte(p,0x80a)=0;byte(p,0x809)=0;byte(p,0x808)=0;
        word(p,0x814,0);word(p,0x810,0);word(p,0x80c,0);word(p,0x818,0);
        byte(p,0x807)=0;byte(p,0x806)=0;
        op.native_site=0x008d9fc2;resize_native_string_header_0041dd40(at(p,0x820),c.strings,12,false);
        if(void* data=get<void*>(p,0x824))std::memcpy(data,c.unknown_file_00d15fe8,get(p,0x820));
        word(p,0x828,reinterpret_cast<U>(stream));op.native_site=0x008d9fed;
        InterlockedIncrement(static_cast<volatile LONG*>(at(stream,4)));
        if(void* current=get<void*>(p,0x828)) {
            op.native_site=0x008da007;
            const U size=static_cast<U>(c.calls.stream_size_slot30(current,c.memory));
            word(p,0x830,size);op.native_site=0x008da010;
            void* const buffer=c.calls.allocate_00bf55be(size);
            const U current_size=get(p,0x830);current=get<void*>(p,0x828);
            word(p,0x82c,reinterpret_cast<U>(buffer));word(p,0x834,0);
            op.native_site=0x008da038;c.calls.stream_read_slot24(current,buffer,current_size,nullptr,c.memory);
            if(extra) {
                const U n=static_cast<U>(std::strlen(c.default_delimiters_00e0c944));
                const U e=static_cast<U>(std::strlen(extra));op.native_site=0x008da072;
                auto* const allocated=static_cast<char*>(c.calls.allocate_00bf55be(n+e+2));
                word(p,0x818,reinterpret_cast<U>(allocated));
                std::strcpy(allocated,c.default_delimiters_00e0c944);
                const U extra_bytes=static_cast<U>(std::strlen(extra))+1;
                auto* const destination=get<char*>(p,0x818);
                std::memcpy(destination+std::strlen(destination),extra,extra_bytes);
            } else word(p,0x818,reinterpret_cast<U>(c.default_delimiters_00e0c944));
        }
        op.phase=Op::Phase::complete;return p;
    }catch(...){op.phase=Op::Phase::failed;throw;}
}
void destroy_native_scene_tokenizer_008d9c30(void* p,NativeSceneTokenizerContext& c,Op& op) {
    begin(p,op);try{destroy_body(p,c,op);op.phase=Op::Phase::complete;}
    catch(...){op.phase=Op::Phase::failed;throw;}
}
void* delete_native_scene_tokenizer_008d9f00(void* p,U flags,NativeSceneTokenizerContext& c,Op& op) {
    begin(p,op);try{destroy_body(p,c,op);if(flags&1){op.native_site=0x008d9f10;c.calls.free_00bf65ac(p);}op.phase=Op::Phase::complete;return p;}
    catch(...){op.phase=Op::Phase::failed;throw;}
}
char peek_native_scene_byte_008d8900(void* p) noexcept {
    if(!byte(p,0x808)) {
        const U cursor=get(p,0x834);
        if(static_cast<std::int32_t>(cursor)<get<std::int32_t>(p,0x830)) {
            const auto ch=*static_cast<const std::uint8_t*>(at(get<void*>(p,0x82c),cursor));
            word(p,0x834,cursor+1);byte(p,0x807)=ch;byte(p,0x808)=1;
            if(ch=='\n')word(p,0x81c,get(p,0x81c)+1);
            return static_cast<char>(ch);
        }
        byte(p,0x809)=1;byte(p,0x807)=0;
    }
    return static_cast<char>(byte(p,0x807));
}
char advance_native_scene_byte_008d8a50(void* p) noexcept {
    byte(p,0x806)=byte(p,0x807);byte(p,0x808)=0;return peek_native_scene_byte_008d8900(p);
}
void consume_native_scene_token_008d8960(void* p) noexcept {
    auto* source=static_cast<const char*>(at(p,5));auto* destination=static_cast<char*>(at(p,0x405));
    char ch;do{ch=*source++;*destination++=ch;}while(ch);
    byte(p,0x805)=0;
}
char* peek_native_scene_token_008d8a70(void* p,NativeSceneTokenizerContext& c) {
    auto* const token=static_cast<char*>(at(p,5));
    if(byte(p,0x805))return token;
    byte(p,4)=0;byte(p,0x80a)=byte(p,0x809);byte(p,5)=0;
    peek_native_scene_byte_008d8900(p);
    while(!byte(p,0x809)) {
        while(std::strchr(c.whitespace_00e0c940,peek_native_scene_byte_008d8900(p))) {
            advance_native_scene_byte_008d8a50(p);
            if(byte(p,0x809))break;
        }
        advance_native_scene_byte_008d8a50(p);
        if(byte(p,0x806)=='/') {
            if(byte(p,0x807)=='/') {
                while(!byte(p,0x809)&&byte(p,0x807)!='\n')advance_native_scene_byte_008d8a50(p);
                advance_native_scene_byte_008d8a50(p);continue;
            }
            if(byte(p,0x807)=='*') {
                while(!byte(p,0x809)&&!(byte(p,0x806)=='*'&&byte(p,0x807)=='/'))advance_native_scene_byte_008d8a50(p);
                advance_native_scene_byte_008d8a50(p);continue;
            }
        }
        if(std::strchr(get<const char*>(p,0x818),static_cast<signed char>(byte(p,0x806)))) {append(p,0x806);break;}
        if(byte(p,0x806)=='"') {
            byte(p,4)=1;
            while(!byte(p,0x809)) {
                if(peek_native_scene_byte_008d8900(p)=='"'){advance_native_scene_byte_008d8a50(p);break;}
                append(p,0x807);advance_native_scene_byte_008d8a50(p);
            }
            break;
        }
        append(p,0x806);
        while(!byte(p,0x809)) {
            if(std::strchr(c.whitespace_00e0c940,peek_native_scene_byte_008d8900(p)))break;
            if(std::strchr(get<const char*>(p,0x818),peek_native_scene_byte_008d8900(p)))break;
            append(p,0x807);advance_native_scene_byte_008d8a50(p);
        }
        break;
    }
    byte(p,0x805)=1;return token;
}
} // namespace bsp

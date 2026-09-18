#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "bsp/native_stream_text_scanner.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native stream text scanner requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;using Op=NativeStreamTextScannerOperation;
void* at(void* p,U n) noexcept {return reinterpret_cast<void*>(reinterpret_cast<U>(p)+n);}
template<class T=U>T get(void* p,U n=0) noexcept {T v;std::memcpy(&v,at(p,n),sizeof v);return v;}
void word(void* p,U n,U v) noexcept {std::memcpy(at(p,n),&v,4);}
std::uint8_t& byte(void* p,U n) noexcept {return *static_cast<std::uint8_t*>(at(p,n));}
U slot(U table,U offset) noexcept {return get(reinterpret_cast<void*>(table),offset);}
void begin(void* p,Op& op) {
    if(op.phase!=Op::Phase::fresh)throw std::logic_error("native stream scanner operation cannot replay");
    op.owner=p;op.phase=Op::Phase::running;
}
void zero_reference(void* stream,NativeStreamTextScannerContext& c) {
    const U table=get(stream);const U target=slot(table,0);
    c.calls.zero_reference(target,stream,table);
}
void refill(void* p,NativeStreamTextScannerContext& c,U& actual) {
    void* const stream=get<void*>(p,0x824);const U table=get(stream);const U target=slot(table,0x24);
    c.calls.read(target,stream,at(p,0x803),1,&actual);
    if(actual==0){byte(p,0x805)=1;byte(p,0x803)=0;}
    else{byte(p,0x804)=1;if(byte(p,0x803)=='\n')word(p,0x818,get(p,0x818)+1);}
}
char peek_inline(void* p,NativeStreamTextScannerContext& c,U& actual) {
    if(!byte(p,0x804))refill(p,c,actual);return static_cast<char>(byte(p,0x803));
}
void advance_inline(void* p,NativeStreamTextScannerContext& c,U& actual) {
    byte(p,0x802)=byte(p,0x803);byte(p,0x804)=0;refill(p,c,actual);
}
void append(void* p,U offset){std::strncat(static_cast<char*>(at(p,1)),static_cast<const char*>(at(p,offset)),1);}
}
void* NativeStreamTextScannerCalls::allocate_00bf55be(U n){return singleton_lifetime_allocate({SingletonAllocationKind::object,n,n});}
void NativeStreamTextScannerCalls::free_00bf65ac(void* p){singleton_lifetime_free(p);}
NativeStreamTextScannerVfsCalls::NativeStreamTextScannerVfsCalls(NativeVfsRuntimeBindings& b) noexcept:bindings_(b){}
void* NativeStreamTextScannerVfsCalls::open(U e,void* p,const void* name,U flags){return bindings_.open_manager_entry(e,p,name,flags);}
void NativeStreamTextScannerVfsCalls::read(U e,void* p,void* d,U n,U* actual){bindings_.source_read(e,p,d,n,actual);}
void NativeStreamTextScannerVfsCalls::zero_reference(U e,void* p,U table){bindings_.source_zero_reference(e,p,table);}
Op::~NativeStreamTextScannerOperation(){if(phase==Phase::running||phase==Phase::failed)std::terminate();}
void Op::acknowledge_diagnostic_cleanup() noexcept {phase=Phase::diagnostic_retired;}
void* construct_native_stream_text_scanner_00bef2e0(void* p,void* filename,const char* extra,NativeStreamTextScannerContext& c,Op& op) {
    begin(p,op);op.consumed_filename_header=filename;
    try {
        word(p,0x81c,0);word(p,0x820,0);word(p,0x824,0);op.unwind_state=2;
        std::memset(p,0,0x828);
        void* const name=at(p,0x81c);
        if(name!=filename) {
            const U length=get(filename);op.native_site=0x00bef33c;
            resize_native_string_header_0041dd40(name,c.strings,length,true);
            if(length)std::memcpy(get<void*>(name,4),get<void*>(filename,4),get(name));
        }
        void* const manager=c.actual_vfs_0109ceec;const U target=slot(get(manager),4);
        op.native_site=0x00bef367;void* const opened=c.calls.open(target,manager,name,0x32);
        if(void* const old=get<void*>(p,0x824)) {
            op.native_site=0x00bef379;
            if(InterlockedDecrement(static_cast<volatile LONG*>(at(old,4)))==0){op.native_site=0x00bef389;zero_reference(old,c);}
            word(p,0x824,0);
        }
        word(p,0x824,reinterpret_cast<U>(opened));
        if(extra) {
            const U n=static_cast<U>(std::strlen(c.default_delimiters_00e15338));
            const U e=static_cast<U>(std::strlen(extra));op.native_site=0x00bef3d2;
            auto* const data=static_cast<char*>(c.calls.allocate_00bf55be(n+e+2));
            word(p,0x814,reinterpret_cast<U>(data));std::strcpy(data,c.default_delimiters_00e15338);
            const U bytes=static_cast<U>(std::strlen(extra))+1;
            auto* const current=get<char*>(p,0x814);std::memcpy(current+std::strlen(current),extra,bytes);
        }else word(p,0x814,reinterpret_cast<U>(c.default_delimiters_00e15338));
        op.unwind_state=-1;op.native_site=0x00bef457;
        destroy_native_string_header_0041dd20(filename,c.strings);
        op.phase=Op::Phase::complete;return p;
    }catch(...){op.phase=Op::Phase::failed;throw;}
}
void destroy_native_stream_text_scanner_00bef220(void* p,NativeStreamTextScannerContext& c,Op& op) {
    begin(p,op);
    try {
        op.unwind_state=0;
        void* const delimiter=get<void*>(p,0x814);
        if(delimiter!=c.default_delimiters_00e15338&&delimiter) {
            op.native_site=0x00bef258;c.calls.free_00bf65ac(delimiter);word(p,0x814,0);
        }
        if(void* const stream=get<void*>(p,0x824)) {
            op.native_site=0x00bef279;
            if(InterlockedDecrement(static_cast<volatile LONG*>(at(stream,4)))==0){op.native_site=0x00bef289;zero_reference(stream,c);}
            word(p,0x824,0);
        }
        op.unwind_state=-1;op.native_site=0x00bef2b5;
        destroy_native_string_header_0041dd20(at(p,0x81c),c.strings);
        op.phase=Op::Phase::complete;
    }catch(...){op.phase=Op::Phase::failed;throw;}
}
char peek_native_stream_text_byte_00bee840(void* p,NativeStreamTextScannerContext& c) {
    U actual=reinterpret_cast<U>(p);return peek_inline(p,c,actual);
}
char advance_native_stream_text_byte_00bee8c0(void* p,NativeStreamTextScannerContext& c) {
    byte(p,0x802)=byte(p,0x803);byte(p,0x804)=0;return peek_native_stream_text_byte_00bee840(p,c);
}
void accept_native_stream_text_token_00bee800(void* p) noexcept {
    auto* source=static_cast<const char*>(at(p,1));auto* destination=static_cast<char*>(at(p,0x401));
    char ch;do{ch=*source++;*destination++=ch;}while(ch);byte(p,0x801)=0;
}
char* peek_native_stream_text_token_00bee8e0(void* p,NativeStreamTextScannerContext& c,const NativeStreamTextStackPreimages& preimage) {
    auto actual=preimage.token_counts;auto* const token=static_cast<char*>(at(p,1));
    if(byte(p,0x801))return token;
    byte(p,0)=0;byte(p,0x806)=byte(p,0x805);byte(p,1)=0;
    peek_native_stream_text_byte_00bee840(p,c);
    while(!byte(p,0x805)) {
        while(std::strchr(c.whitespace_00e15334,peek_inline(p,c,actual[0]))) {
            advance_inline(p,c,actual[1]);if(byte(p,0x805))break;
        }
        advance_native_stream_text_byte_00bee8c0(p,c);
        if(byte(p,0x802)=='/') {
            if(byte(p,0x803)=='/') {
                while(!byte(p,0x805)&&byte(p,0x803)!='\n')advance_inline(p,c,actual[2]);
                advance_native_stream_text_byte_00bee8c0(p,c);continue;
            }
            if(byte(p,0x803)=='*') {
                while(!byte(p,0x805)&&!(byte(p,0x802)=='*'&&byte(p,0x803)=='/'))advance_inline(p,c,actual[3]);
                advance_native_stream_text_byte_00bee8c0(p,c);continue;
            }
        }
        if(std::strchr(get<const char*>(p,0x814),static_cast<signed char>(byte(p,0x802)))){append(p,0x802);break;}
        if(byte(p,0x802)=='"') {
            byte(p,0)=1;
            while(!byte(p,0x805)) {
                if(peek_native_stream_text_byte_00bee840(p,c)=='"'){advance_native_stream_text_byte_00bee8c0(p,c);break;}
                append(p,0x803);advance_native_stream_text_byte_00bee8c0(p,c);
            }
            break;
        }
        append(p,0x802);
        while(!byte(p,0x805)) {
            if(std::strchr(c.whitespace_00e15334,peek_native_stream_text_byte_00bee840(p,c)))break;
            if(std::strchr(get<const char*>(p,0x814),peek_native_stream_text_byte_00bee840(p,c)))break;
            append(p,0x803);advance_native_stream_text_byte_00bee8c0(p,c);
        }
        break;
    }
    byte(p,0x801)=1;return token;
}
void skip_native_stream_text_whitespace_00beedb0(void* p,NativeStreamTextScannerContext& c,const NativeStreamTextStackPreimages& preimage) {
    auto actual=preimage.recovery_counts;
    while(!byte(p,0x805)) {
        if(!std::strchr(c.whitespace_00e15334,peek_inline(p,c,actual[0])))break;
        if(std::strchr(c.recovery_stop_00d15f34,peek_inline(p,c,actual[1])))break;
        advance_inline(p,c,actual[2]);
    }
    byte(p,0x806)=byte(p,0x805);
}
char* read_native_stream_text_string_00bef020(void* p,std::uint8_t* ok,NativeStreamTextScannerContext& c,const NativeStreamTextStackPreimages& preimage) {
    peek_native_stream_text_token_00bee8e0(p,c,preimage);
    auto* const token=static_cast<char*>(at(p,1));
    if(!byte(p,0x806)&&(std::strlen(token)!=0||byte(p,0)!=0)) {
        if(std::strlen(peek_native_stream_text_token_00bee8e0(p,c,preimage))!=0) {
            peek_native_stream_text_token_00bee8e0(p,c,preimage);accept_native_stream_text_token_00bee800(p);*ok=1;return token;
        }
    }
    skip_native_stream_text_whitespace_00beedb0(p,c,preimage);*ok=0;
    return peek_native_stream_text_token_00bee8e0(p,c,preimage);
}
} // namespace bsp

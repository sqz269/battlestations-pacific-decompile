#include "bsp/native_language_catalog_lifetime.hpp"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native language catalog lifetime requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;using I=std::int32_t;
using Op=NativeLanguageCatalogLifetimeOperation;using Phase=Op::Phase;
void* at(const void* p,U n=0) noexcept{return reinterpret_cast<void*>(reinterpret_cast<U>(p)+n);}
template<class T=U>T get(const void* p,U n=0) noexcept{T v;std::memcpy(&v,at(p,n),sizeof v);return v;}
void put(void* p,U n,U v) noexcept{std::memcpy(at(p,n),&v,4);}
void begin(void* p,Op& op) {
    if(op.phase!=Phase::fresh)throw std::logic_error("native language catalog lifetime operation cannot replay");
    op.owner=p;op.phase=Phase::running;
}
void resize(void* p,I requested,NativeStringRawPoolContext& strings,
    NativeLanguageCatalogAllocationCalls& calls,Op& op) {
    if(requested>get<I>(p,8)) {
        op.native_site=0x8d5aaf;
        reserve_native_language_catalog_008d59c0(p,requested,strings,calls,op.child.emplace());
    }
    const I count=get<I>(p,4);
    if(count<requested) {
        U offset=static_cast<U>(count)<<5;
        U remaining=static_cast<U>(requested)-static_cast<U>(count);
        do {
            void* const row=at(get<void*>(p),offset);op.current_row=row;
            if(row){for(U n=0;n<0x20;n+=4)put(row,n,0);}
            ++op.initialized_rows;offset+=0x20;
        }while(--remaining!=0);
    }
    while(requested<get<I>(p,4)) {
        put(p,4,get(p,4)-1);
        void* const row=at(get<void*>(p),get(p,4)<<5);op.current_row=row;
        op.native_site=0x8d5afe;
        destroy_native_language_row_008d4f50(row,strings,op.child.emplace());
        ++op.destroyed_rows;
    }
    put(p,4,static_cast<U>(requested));
}
}
Op::~NativeLanguageCatalogLifetimeOperation(){if(phase==Phase::running||phase==Phase::failed)std::terminate();}
void Op::acknowledge_diagnostic_cleanup() noexcept {
    if(child)child->acknowledge_diagnostic_cleanup();
    phase=Phase::diagnostic_retired;
}
void resize_native_language_catalog_008d5aa0(void* p,I n,NativeStringRawPoolContext& strings,
    NativeLanguageCatalogAllocationCalls& calls,Op& op) {
    begin(p,op);
    try{resize(p,n,strings,calls,op);op.phase=Phase::complete;}
    catch(...){op.phase=Phase::failed;throw;}
}
void shutdown_native_language_catalog_00cdeea0(void* p,NativeStringRawPoolContext& strings,
    NativeLanguageCatalogAllocationCalls& calls,Op& op) {
    begin(p,op);
    try {
        op.native_site=0xcdeea7;resize(p,0,strings,calls,op);
        void* const backing=get<void*>(p);op.released_backing=backing;
        op.native_site=0xcdeeb2;calls.free_00bf6989(backing);
        op.phase=Phase::complete;
    }catch(...){op.phase=Phase::failed;throw;}
}
int NativeLanguageCatalogRegistrationCalls::register_shutdown_00bf6ff5(void (*f)()){return std::atexit(f);}
int register_native_language_catalog_shutdown_00cd2da0(NativeLanguageCatalogRegistrationCalls& calls,void (*f)()) {
    return calls.register_shutdown_00bf6ff5(f);
}
} // namespace bsp

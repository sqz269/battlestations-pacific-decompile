#include "bsp/native_lua_vfs_dispatch.hpp"
namespace bsp {
std::uintptr_t capture_native_lua_vfs_table(const void* owner) noexcept {
    return *static_cast<const volatile std::uintptr_t*>(owner);
}
namespace {
std::uintptr_t slot(std::uintptr_t table,unsigned offset) {
    return reinterpret_cast<const volatile std::uintptr_t*>(table)[offset/4];
}
class CallableDispatch final : public NativeLuaVfsDispatch {
public:
    void* open(std::uintptr_t table,void* manager,const NativeString& path,std::uint32_t flags) override {
        using Call=void*(__fastcall*)(void*,void*,const NativeString*,std::uint32_t);
        return reinterpret_cast<Call>(slot(table,4))(manager,nullptr,&path,flags);
    }
    std::uint8_t exists(std::uintptr_t table,void* manager,NativeString& path) override {
        using Call=std::uint8_t(__fastcall*)(void*,void*,NativeString*);
        return reinterpret_cast<Call>(slot(table,8))(manager,nullptr,&path);
    }
    std::uint8_t is_open(std::uintptr_t table,void* stream) override {
        using Call=std::uint8_t(__fastcall*)(void*,void*);
        return reinterpret_cast<Call>(slot(table,0x18))(stream,nullptr);
    }
    std::uint64_t length(std::uintptr_t table,void* stream) override {
        using Call=std::uint64_t(__fastcall*)(void*,void*);
        return reinterpret_cast<Call>(slot(table,0x30))(stream,nullptr);
    }
    void read(std::uintptr_t table,void* stream,void* destination,std::uint32_t count,std::uint32_t* actual) override {
        using Call=void(__fastcall*)(void*,void*,void*,std::uint32_t,std::uint32_t*);
        reinterpret_cast<Call>(slot(table,0x24))(stream,nullptr,destination,count,actual);
    }
    void zero_reference(std::uintptr_t table,void* stream) override {
        using Call=void(__fastcall*)(void*,void*);
        reinterpret_cast<Call>(slot(table,0))(stream,nullptr);
    }
};
}
NativeLuaVfsDispatch& callable_native_lua_vfs_dispatch() noexcept {
    static CallableDispatch dispatch;return dispatch;
}
std::uint32_t native_stream_size_low_00be41a0(void* stream,std::uint32_t* high,NativeLuaVfsDispatch& dispatch) {
    const auto size=dispatch.length(capture_native_lua_vfs_table(stream),stream);
    if(high)*high=static_cast<std::uint32_t>(size>>32);
    return static_cast<std::uint32_t>(size);
}
} // namespace bsp

#pragma once
#include <cstdint>
namespace bsp {
class NativeString;
// Original owner vtable word, captured at the corresponding native load.
// It is an identity/data address; native dispatch never executes that address.
std::uintptr_t capture_native_lua_vfs_table(const void* owner) noexcept;

// Source composition interface, separate from the actual native owner layout.
// The table argument retains the caller's captured table across nested calls.
class NativeLuaVfsDispatch {
public:
    virtual ~NativeLuaVfsDispatch() = default;
    virtual void* open(std::uintptr_t table,void* manager,const NativeString&,std::uint32_t flags)=0;
    virtual std::uint8_t exists(std::uintptr_t table,void* manager,NativeString&)=0;
    virtual std::uint8_t is_open(std::uintptr_t table,void* stream)=0;
    virtual std::uint64_t length(std::uintptr_t table,void* stream)=0;
    virtual void read(std::uintptr_t table,void* stream,void* destination,std::uint32_t requested,std::uint32_t* actual)=0;
    virtual void zero_reference(std::uintptr_t table,void* stream)=0;
};
// Existing explicit CALLABLE original-ABI table domain. No owner or default
// provider is created. Numeric native tables require a concrete dispatcher.
NativeLuaVfsDispatch& callable_native_lua_vfs_dispatch() noexcept;

// BE41A0 complete eight-instruction wrapper: current virtual30 -> EDX:EAX,
// optionally store high DWORD, return low. Original ECX stream, RET4.
std::uint32_t native_stream_size_low_00be41a0(void* stream,
    std::uint32_t* optional_high,NativeLuaVfsDispatch&);
} // namespace bsp

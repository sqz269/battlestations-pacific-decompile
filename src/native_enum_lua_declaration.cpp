#include "bsp/native_enum_lua_declaration.hpp"
#include "bsp/game_native_lua_display_string.hpp"
#include "bsp/lua_numeric.hpp"
#include "bsp/native_lua_objects.hpp"
#include <cstddef>
#include <cstring>
#include <type_traits>
extern "C" {
#include <lua.h>
}

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native enum Lua declaration requires MSVC Win32.
#endif
namespace bsp {
namespace {
static_assert(sizeof(void*)==4 && sizeof(NativeString)==8 && alignof(NativeString)==4);
static_assert(std::is_nothrow_default_constructible_v<NativeString>);
static_assert(std::is_trivially_destructible_v<NativeString>);
static_assert(sizeof(NativeLuaObjectStorage)==0x14 && sizeof(NativeLuaStateStorage)==0x4c8);

// Physical reads and byte stores retain the target's current-field schedule.
__declspec(noinline) std::uint32_t read_word(const void* owner,std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        reinterpret_cast<std::uintptr_t>(owner)+offset);
}
__declspec(noinline) void copy_name_through_nul(void* owner,const char* source) noexcept {
    auto* destination=reinterpret_cast<volatile char*>(
        reinterpret_cast<std::uintptr_t>(owner)+0x11cu);
    const volatile char* input=source;
    char byte;
    do { byte=*input++; *destination++=byte; } while(byte!='\0');
}

class CompletedLuaObject final {
public:
    explicit CompletedLuaObject(NativeLuaObjectStorage& object) noexcept : object_(object) {}
    CompletedLuaObject(const CompletedLuaObject&)=delete;
    CompletedLuaObject& operator=(const CompletedLuaObject&)=delete;
    ~CompletedLuaObject() noexcept(false) { if(active_) destroy_native_lua_object_00b67700(object_); }
    void arm() noexcept { active_=true; }
    void destroy() { active_=false; destroy_native_lua_object_00b67700(object_); }
private:
    NativeLuaObjectStorage& object_;
    bool active_=false;
};
class CompletedRawString final {
public:
    CompletedRawString(NativeString& object,NativeStringRawPoolContext& strings) noexcept
        : object_(object),strings_(strings) {}
    CompletedRawString(const CompletedRawString&)=delete;
    CompletedRawString& operator=(const CompletedRawString&)=delete;
    ~CompletedRawString() noexcept(false) {
        if(active_) destroy_native_string_header_0041dd20(&object_,strings_);
    }
    void arm() noexcept { active_=true; }
    void destroy() { active_=false; destroy_native_string_header_0041dd20(&object_,strings_); }
private:
    NativeString& object_;
    NativeStringRawPoolContext& strings_;
    bool active_=false;
};
} // namespace

void declare_native_enum_symbols_from_lua_008f2f70(void* owner,
    NativeString& consumed_key,NativeEnumLuaDeclarationContext& context) {
    auto& strings=context.actual_enum_declaration.actual_strings;
    CompletedRawString incoming_cleanup(consumed_key,strings);
    incoming_cleanup.arm(); // Original state0, before name/type work

    const char* authored_name=consumed_key.data();
    if(authored_name==nullptr) authored_name=context.actual_clone_empty_00f89450;
    copy_name_through_nul(owner,authored_name);
    resize_native_string_header_0041dd40(static_cast<std::byte*>(owner)+0x114,
        strings,5,false);
    void* const type_data=reinterpret_cast<void*>(read_word(owner,0x118));
    if(type_data!=nullptr) {
        const auto current_length=read_word(owner,0x114);
        std::memcpy(type_data,context.actual_type_bytes_00d162d8,current_length);
    }

    // Do not cache this publication in the binding or before the above stores.
    void* const world=context.actual_world_publication_00e188a8;
    auto& lua_owner=*reinterpret_cast<NativeLuaStateStorage*>(
        static_cast<std::byte*>(world)+0x1a0c);
    NativeLuaObjectStorage globals;
    CompletedLuaObject globals_cleanup(globals);
    NativeLuaObjectStorage selected;
    CompletedLuaObject selected_cleanup(selected);
    auto* const global_result=native_lua_globals_00b67980(lua_owner,&globals);
    globals_cleanup.arm();
    native_lua_get_by_string_00b68100(*global_result,&selected,consumed_key);
    selected_cleanup.arm();
    globals_cleanup.destroy(); // state3: completed selected object only

    if(native_lua_is_table_00b661b0(selected)) {
        NativeLuaObjectStorage key;
        CompletedLuaObject key_cleanup(key);
        NativeLuaObjectStorage value;
        CompletedLuaObject value_cleanup(value);
        construct_native_lua_object_00b65f50(&key);
        key_cleanup.arm();
        construct_native_lua_object_00b65f50(&value);
        value_cleanup.arm();
        native_lua_iterate_first_00b67080(selected,key,value);
        while(!native_lua_is_unbound_00b66420(key)) {
            NativeLuaObjectStorage name;
            CompletedLuaObject name_cleanup(name);
            auto* const name_result=native_lua_get_by_name_00b67800(
                value,&name,context.actual_name_key_00ce8ed0);
            name_cleanup.arm();
            NativeString display;
            CompletedRawString display_cleanup(display,strings);
            auto* const display_result=context.actual_fixed_display.to_display_string(
                *name_result,&display);
            const char* symbol=display_result->data();
            display_cleanup.arm(); // Only completed output, never failed output
            if(symbol==nullptr) symbol=context.actual_clone_empty_00f89450;

            const double number=lua_tonumber(key.owner_00->state_04,key.index_08);
            const bool sse2=context.actual_crt_feature_word_0109eea4!=0;
            const auto integer=lua_number_integer_00b66290(number,sse2);
            declare_native_enum_symbol_008f2e40(owner,symbol,integer,
                context.actual_enum_declaration);

            // Each destroy disarms BEFORE its call: no retry if cleanup throws.
            // Raw string destruction reads CURRENTdata/length and repeats getter.
            display_cleanup.destroy();
            name_cleanup.destroy();
            native_lua_iterate_next_00b67190(selected,key,value);
        }
        value_cleanup.destroy();
        key_cleanup.destroy();
    }
    selected_cleanup.destroy();
    incoming_cleanup.destroy();
}
} // namespace bsp

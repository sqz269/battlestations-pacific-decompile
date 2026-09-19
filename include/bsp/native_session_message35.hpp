#pragma once
#include "bsp/native_session_message_base.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_word_array_storage.hpp"
#include <type_traits>
namespace bsp {
struct NativeSessionMessage35 {
    NativeSessionMessageStorage base;
    std::uint32_t value_18,value_1c,subtype_20;
    std::uint8_t flag_24,flag_25,reserved_26[2];
    std::uint32_t length_28;char* data_2c;
    std::uint32_t length_30;char* data_34;
    std::uint32_t values_38[2];
    NativeWordArrayStorage words_40;
    std::uint32_t length_4c;char* data_50;
};
static_assert(sizeof(NativeSessionMessage35)==0x54);
static_assert(offsetof(NativeSessionMessage35,length_28)==0x28);
static_assert(offsetof(NativeSessionMessage35,words_40)==0x40);
static_assert(offsetof(NativeSessionMessage35,length_4c)==0x4c);
// CED098 has five native slots. Borrowed bindings after them are SOURCE-ONLY
// metadata. Profile, actual pool context and E17669 backing must stay alive.
struct NativeSessionMessage35Profile {
    const std::uint32_t slots[5];NativeStringRawPoolContext* const strings;const char* const fallback_00e17669;
    NativeSessionMessage35Profile(NativeStringRawPoolContext&,const char* actual_fallback_00e17669);
};
static_assert(std::is_standard_layout_v<NativeSessionMessage35Profile>);
static_assert(offsetof(NativeSessionMessage35Profile,slots)==0);
NativeSessionMessage35* construct_native_session_message35_0052aef0(NativeSessionMessage35*,const NativeSessionMessageContext&,const NativeSessionMessage35Profile&);
bool native_session_message_is35_0052af40(std::uint32_t);
void write_native_session_message35_005294f0(const NativeSessionMessage35*,NativeBitCursor*,const char* actual_fallback_00e17669);
void read_native_session_message35_0052abe0(NativeSessionMessage35*,NativeSessionReadStream*,NativeStringRawPoolContext&);
void destroy_native_session_message35_0052af50(NativeSessionMessage35*,NativeStringRawPoolContext&,const NativeSessionMessage35Profile&);
NativeSessionMessage35* delete_native_session_message35_0052b010(NativeSessionMessage35*,std::uint32_t,NativeStringRawPoolContext&,const NativeSessionMessage35Profile&);
} // namespace bsp

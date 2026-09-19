#pragma once
#include "bsp/native_session_message_base.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <type_traits>

namespace bsp {
struct NativeMessage69String {std::uint32_t length_00;char* data_04;};
template<class T> struct NativeMessage69Vector {
    std::uint32_t retained_00;T* begin_04;T* end_08;T* capacity_0c;
};
struct NativeSessionMessage69 {
    NativeSessionMessageStorage base;
    NativeMessage69Vector<std::uint16_t> words_18;
    NativeMessage69Vector<NativeMessage69String> strings_28;
};
struct NativeMessage75ExtendedHeader {
    NativeSessionMessageStorage base;
    std::uint16_t sender_18;std::uint8_t relay_1a,retained_1b;
    std::uint8_t flag_1c,retained_1d[3];
};
struct NativeSessionMessage75 {
    NativeMessage75ExtendedHeader header;
    std::uint8_t flag_20,retained_21[3];
    std::uint32_t value_24,value_28,value_2c;
    std::uint8_t flag_30,retained_31[3];
};
struct NativeSessionMessage76 {
    NativeMessage75ExtendedHeader header;std::uint32_t value_20,value_24;
};
static_assert(sizeof(NativeMessage69String)==8 && sizeof(NativeSessionMessage69)==0x38);
static_assert(offsetof(NativeSessionMessage69,strings_28)==0x28);
static_assert(sizeof(NativeMessage75ExtendedHeader)==0x20 && sizeof(NativeSessionMessage75)==0x34);
static_assert(sizeof(NativeSessionMessage76)==0x28);
struct NativeSessionMessage69To76Context {
    const NativeSessionMessageContext* session;
    NativeStringRawPoolContext* strings;
    const char* fallback_00e17669;
    const SingletonLifetimeCallbacks* invalid_parameters;
    mutable ActualNativeStringPoolStorage checked_strings;
    NativeSessionMessage69To76Context(const NativeSessionMessageContext&,NativeStringRawPoolContext&,
        const char*,const SingletonLifetimeCallbacks*);
};
// Five native slots followed by a source-only borrowed context pointer.
// Context and its live publication bindings must outlive all users.
#define BSP_MESSAGE_PROFILE(N,C,P,W,R,D,S) \
struct NativeSessionMessage##N##Profile {const std::uint32_t slots[5];const NativeSessionMessage69To76Context* const context;explicit NativeSessionMessage##N##Profile(const NativeSessionMessage69To76Context&);}; \
static_assert(std::is_standard_layout_v<NativeSessionMessage##N##Profile>); \
NativeSessionMessage##N* construct_native_session_message##N##_##C(NativeSessionMessage##N*,const NativeSessionMessage69To76Context&,const NativeSessionMessage##N##Profile&); \
bool native_session_message_is##N##_##P(std::uint32_t); \
void write_native_session_message##N##_##W(const NativeSessionMessage##N*,NativeBitCursor*,const NativeSessionMessage69To76Context&); \
void read_native_session_message##N##_##R(NativeSessionMessage##N*,NativeSessionReadStream*,const NativeSessionMessage69To76Context&); \
void destroy_native_session_message##N##_##D(NativeSessionMessage##N*,const NativeSessionMessage69To76Context&,const NativeSessionMessage##N##Profile&); \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N*,std::uint32_t,const NativeSessionMessage69To76Context&,const NativeSessionMessage##N##Profile&);
BSP_MESSAGE_PROFILE(69,00767570,00767600,008e4a60,008eb7e0,00767610,007676a0)
BSP_MESSAGE_PROFILE(75,00759e40,00759e80,0075b950,0075b9e0,00759e70,00759ea0)
BSP_MESSAGE_PROFILE(76,00759ec0,00759f00,0075ba70,0075bad0,00759ef0,00759f20)
#undef BSP_MESSAGE_PROFILE
} // namespace bsp

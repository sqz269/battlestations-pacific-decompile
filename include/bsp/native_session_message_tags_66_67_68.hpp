#pragma once
#include "bsp/native_session_message_base.hpp"
#include "bsp/native_bit_cursor_numeric.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <type_traits>

namespace bsp {
struct NativeMessage66Dwords {
    std::uint32_t retained_00;
    std::int32_t* begin_04;std::int32_t* end_08;std::int32_t* capacity_0c;
};
struct NativeSessionMessage66 {
    NativeSessionMessageStorage base;
    std::int32_t value_18,value_1c;
    std::uint16_t word_20,retained_22;
    NativeMessage66Dwords first_24,second_34,third_44;
    std::uint16_t handle_54;
    std::uint8_t has_handle_56,retained_57;
    std::uint32_t value_58;
};
struct NativeSessionMessage67 {
    NativeSessionMessageStorage base;
    std::int32_t value_18;
    std::uint32_t length_1c;char* data_20;
    std::int32_t value_24;
    std::uint32_t value_28;
    std::uint16_t word_2c,word_2e;
};
struct NativeSessionMessage68 {
    NativeSessionMessageStorage base;
    std::uint32_t value_18;
    std::uint16_t word_1c,retained_1e;
    std::uint32_t float_20;
    std::uint16_t word_24,word_26;
    std::int32_t value_28,value_2c;
    std::uint32_t value_30,float_34,value_38,length_3c;char* data_40;
};
static_assert(sizeof(NativeMessage66Dwords)==0x10 && sizeof(NativeSessionMessage66)==0x5c);
static_assert(offsetof(NativeSessionMessage66,first_24)==0x24 && offsetof(NativeSessionMessage66,handle_54)==0x54);
static_assert(sizeof(NativeSessionMessage67)==0x30 && offsetof(NativeSessionMessage67,length_1c)==0x1c);
static_assert(sizeof(NativeSessionMessage68)==0x44 && offsetof(NativeSessionMessage68,length_3c)==0x3c);
struct NativeSessionMessage66To68Context {
    const NativeSessionMessageContext* session;
    NativeStringRawPoolContext* strings;
    const char* fallback_00e17669;
    const NativeBitNumericContext* numeric;
    const volatile float* maximum_00d7a248;
    const SingletonLifetimeCallbacks* invalid_parameters;
};
// Only the five slots are native. This source-only context pointer and every
// borrowed binding must remain alive while a message uses the profile.
#define BSP_MESSAGE_PROFILE(N,C,P,W,R,D,S) \
struct NativeSessionMessage##N##Profile {const std::uint32_t slots[5];const NativeSessionMessage66To68Context* const context;explicit NativeSessionMessage##N##Profile(const NativeSessionMessage66To68Context&);}; \
static_assert(std::is_standard_layout_v<NativeSessionMessage##N##Profile>); \
NativeSessionMessage##N* construct_native_session_message##N##_##C(NativeSessionMessage##N*,const NativeSessionMessage66To68Context&,const NativeSessionMessage##N##Profile&); \
bool native_session_message_is##N##_##P(std::uint32_t); \
void write_native_session_message##N##_##W(const NativeSessionMessage##N*,NativeBitCursor*,const NativeSessionMessage66To68Context&); \
void read_native_session_message##N##_##R(NativeSessionMessage##N*,NativeSessionReadStream*,const NativeSessionMessage66To68Context&); \
void destroy_native_session_message##N##_##D(NativeSessionMessage##N*,const NativeSessionMessage66To68Context&,const NativeSessionMessage##N##Profile&); \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N*,std::uint32_t,const NativeSessionMessage66To68Context&,const NativeSessionMessage##N##Profile&);
BSP_MESSAGE_PROFILE(66,00766d00,00766d90,00943810,00947e40,00766da0,00766e00)
BSP_MESSAGE_PROFILE(67,00765760,007657c0,008e47e0,008e4850,007657d0,00765830)
BSP_MESSAGE_PROFILE(68,00765850,007658b0,008e48c0,008e4990,007658c0,00765920)
#undef BSP_MESSAGE_PROFILE
} // namespace bsp

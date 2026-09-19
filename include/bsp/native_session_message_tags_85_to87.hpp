#pragma once
#include "bsp/native_session_message_tags_69_75_76.hpp"
#include "bsp/native_bit_cursor_numeric.hpp"

namespace bsp {
struct NativeSessionMessage85 {
    NativeSessionMessageStorage base;
    std::uint32_t float_18,float_1c,float_20,float_24,float_28,float_2c,float_30;
    std::uint8_t flag_34,retained_35[3];
};
struct NativeSessionMessage86 {NativeMessage75ExtendedHeader header;std::uint32_t value_20;};
struct NativeSessionMessage87 {NativeMessage75ExtendedHeader header;};
static_assert(sizeof(NativeSessionMessage85)==0x38 && offsetof(NativeSessionMessage85,flag_34)==0x34);
static_assert(sizeof(NativeSessionMessage86)==0x24 && sizeof(NativeSessionMessage87)==0x20);
struct NativeSessionMessage85To87Context {
    const NativeSessionMessageContext* session;const NativeBitNumericContext* numeric;
    const volatile float *range_00d02f60,*maximum_00d02f64,*minimum_00d02f68;
    const volatile double *offset_00cf0dd8,*upper_00cf0aa0;
    const volatile float *angle_scale_00d7a264,*cap_00ce380c,*scale_00ce3c64;
};
// Five native profile slots plus a source-only borrowed context pointer.
#define BSP_MESSAGE_PROFILE(N,C,P,W,R,D,S) \
struct NativeSessionMessage##N##Profile {const std::uint32_t slots[5];const NativeSessionMessage85To87Context* const context;explicit NativeSessionMessage##N##Profile(const NativeSessionMessage85To87Context&);}; \
static_assert(std::is_standard_layout_v<NativeSessionMessage##N##Profile>); \
NativeSessionMessage##N* construct_native_session_message##N##_##C(NativeSessionMessage##N*,const NativeSessionMessage85To87Context&,const NativeSessionMessage##N##Profile&); \
bool native_session_message_is##N##_##P(std::uint32_t); \
void write_native_session_message##N##_##W(const NativeSessionMessage##N*,NativeBitCursor*,const NativeSessionMessage85To87Context&); \
void read_native_session_message##N##_##R(NativeSessionMessage##N*,NativeSessionReadStream*,const NativeSessionMessage85To87Context&); \
void destroy_native_session_message##N##_##D(NativeSessionMessage##N*); \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N*,std::uint32_t);
// No separate native85 ordinary destructor is asserted by this packet.
BSP_MESSAGE_PROFILE(85,0075c020,0075c080,0075c190,0075c090,source,0075c360)
BSP_MESSAGE_PROFILE(86,0075a470,0075a4a0,0075fdb0,0075fe00,0075a4c0,0075fe70)
BSP_MESSAGE_PROFILE(87,00521f30,00521f60,00521f80,00521fa0,00521fc0,00522ec0)
#undef BSP_MESSAGE_PROFILE
} // namespace bsp

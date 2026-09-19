#pragma once
#include "bsp/native_session_message_tags_69_75_76.hpp"
#include "bsp/native_bit_cursor_numeric.hpp"
namespace bsp {
struct NativeSessionMessage94 {NativeMessage75ExtendedHeader header;std::uint16_t handle_20;std::uint8_t mode_22,flag_23;};
struct NativeSessionMessage95 {NativeMessage75ExtendedHeader header;std::uint32_t vector_20[3];std::uint8_t present_2c,retained_2d[3];};
struct NativeSessionMessage96 {NativeMessage75ExtendedHeader header;std::uint32_t value_20;};
struct NativeSessionMessage97 {NativeMessage75ExtendedHeader header;std::uint32_t value_20,value_24;};
static_assert(sizeof(NativeSessionMessage94)==0x24 && sizeof(NativeSessionMessage95)==0x30 && sizeof(NativeSessionMessage96)==0x24 && sizeof(NativeSessionMessage97)==0x28);
static_assert(offsetof(NativeSessionMessage95,present_2c)==0x2c);
struct NativeSessionMessage94To97Context {const NativeSessionMessageContext* session;const NativeBitNumericContext* numeric;const volatile float* maximum_00d7a248;};
#define BSP_MESSAGE_PROFILE(N,C,P,W,R,D,S) \
struct NativeSessionMessage##N##Profile {const std::uint32_t slots[5];const NativeSessionMessage94To97Context* const context;explicit NativeSessionMessage##N##Profile(const NativeSessionMessage94To97Context&);}; \
static_assert(std::is_standard_layout_v<NativeSessionMessage##N##Profile>); \
NativeSessionMessage##N* construct_native_session_message##N##_##C(NativeSessionMessage##N*,const NativeSessionMessage94To97Context&,const NativeSessionMessage##N##Profile&); \
bool native_session_message_is##N##_##P(std::uint32_t); \
void write_native_session_message##N##_##W(const NativeSessionMessage##N*,NativeBitCursor*,const NativeSessionMessage94To97Context&); \
void read_native_session_message##N##_##R(NativeSessionMessage##N*,NativeSessionReadStream*,const NativeSessionMessage94To97Context&); \
void destroy_native_session_message##N##_##D(NativeSessionMessage##N*); \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N*,std::uint32_t);
BSP_MESSAGE_PROFILE(94,0075b060,0075b090,007640d0,00764160,0075b0c0,00764200)
BSP_MESSAGE_PROFILE(95,0075b100,0071cc70,0071cca0,0071cd30,0071cdc0,0071e3f0)
BSP_MESSAGE_PROFILE(96,0075b0d0,0071cb80,0071cbb0,0071cbe0,0071cc10,0071e3d0)
// No separate native97 ordinary destructor is claimed; this helper is its scalar root stamp.
BSP_MESSAGE_PROFILE(97,0075a290,0075a2c0,0075c700,0075c760,source,0075ca00)
#undef BSP_MESSAGE_PROFILE
NativeSessionMessage95* construct_native_session_message95_value_0071cc20(NativeSessionMessage95*,const float* vector,std::uint8_t present,const NativeSessionMessage94To97Context&,const NativeSessionMessage95Profile&);
NativeSessionMessage96* construct_native_session_message96_value_0071cb50(NativeSessionMessage96*,std::uint32_t value,const NativeSessionMessage94To97Context&,const NativeSessionMessage96Profile&);
} // namespace bsp

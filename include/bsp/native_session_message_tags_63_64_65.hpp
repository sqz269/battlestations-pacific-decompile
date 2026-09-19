#pragma once
#include "bsp/native_session_message_base.hpp"
#include "bsp/native_bit_cursor_numeric.hpp"
#include <type_traits>

namespace bsp {
struct NativeSessionMessage63 {
    NativeSessionMessageStorage base;
    std::int32_t value_18;
    std::uint8_t has_handle_1c,retained_1d[3];
    std::uint32_t position_20[3];
    std::uint16_t handle_2c,retained_2e;
};
using NativeSessionMessage64=NativeSessionMessage63;
struct NativeSessionMessage65 {
    NativeSessionMessageStorage base;
    std::uint32_t value_18;
    std::uint8_t flag_1c,retained_1d[3];
};
static_assert(sizeof(NativeSessionMessage63)==0x30 && offsetof(NativeSessionMessage63,handle_2c)==0x2c);
static_assert(sizeof(NativeSessionMessage65)==0x20);
// Native tables contain five slots. Borrowed metadata after those slots is
// source-only; both the profile and numeric bindings must outlive messages.
#define BSP_MESSAGE_POSITION_PROFILE(N) \
struct NativeSessionMessage##N##Profile { \
    const std::uint32_t slots[5]; \
    const NativeBitNumericContext* const numeric; \
    const volatile float* const maximum_00d7a248; \
    NativeSessionMessage##N##Profile(const NativeBitNumericContext&,const volatile float&); \
}; \
static_assert(std::is_standard_layout_v<NativeSessionMessage##N##Profile>);
BSP_MESSAGE_POSITION_PROFILE(63)
BSP_MESSAGE_POSITION_PROFILE(64)
#undef BSP_MESSAGE_POSITION_PROFILE
const std::uint32_t* native_session_message65_profile_00d02eac();
NativeSessionMessage63* construct_native_session_message63_00763720(NativeSessionMessage63*,const NativeSessionMessageContext&,const NativeSessionMessage63Profile&);
NativeSessionMessage64* construct_native_session_message64_00763930(NativeSessionMessage64*,const NativeSessionMessageContext&,const NativeSessionMessage64Profile&);
// Native constructor deliberately leaves type=0 and selected owner=null.
NativeSessionMessage65* construct_native_session_message65_0075b130(NativeSessionMessage65*);
bool native_session_message_is63_007637a0(const NativeSessionMessage63*,std::uint32_t);
bool native_session_message_is64_007639b0(const NativeSessionMessage64*,std::uint32_t);
bool native_session_message_is65_0075b160(const NativeSessionMessage65*,std::uint32_t);
void write_native_session_message63_007637c0(const NativeSessionMessage63*,NativeBitCursor*,const NativeBitNumericContext&,const volatile float&);
void read_native_session_message63_00763870(NativeSessionMessage63*,NativeSessionReadStream*,const NativeBitNumericContext&,const volatile float&);
void write_native_session_message64_007639d0(const NativeSessionMessage64*,NativeBitCursor*,const NativeBitNumericContext&,const volatile float&);
void read_native_session_message64_00763a80(NativeSessionMessage64*,NativeSessionReadStream*,const NativeBitNumericContext&,const volatile float&);
void write_native_session_message65_0075b180(const NativeSessionMessage65*,NativeBitCursor*);
void read_native_session_message65_0075b1c0(NativeSessionMessage65*,NativeSessionReadStream*);
void destroy_native_session_message63_00763790(NativeSessionMessage63*);
void destroy_native_session_message64_007639a0(NativeSessionMessage64*);
void destroy_native_session_message65_0075b150(NativeSessionMessage65*);
NativeSessionMessage63* delete_native_session_message63_00763910(NativeSessionMessage63*,std::uint32_t);
NativeSessionMessage64* delete_native_session_message64_00763b20(NativeSessionMessage64*,std::uint32_t);
NativeSessionMessage65* delete_native_session_message65_0075b200(NativeSessionMessage65*,std::uint32_t);
} // namespace bsp

#pragma once
#include "bsp/native_session_message_base.hpp"
#include "bsp/native_bit_cursor_numeric.hpp"
#include <type_traits>
namespace bsp {
// All payload and base padding bytes are retained by these constructors.
struct NativeSessionMessage36 {NativeSessionMessageStorage base;std::uint32_t value_18,value_1c,float_bits_20,float_bits_24;};
struct NativeSessionMessage37 {NativeSessionMessageStorage base;std::int32_t value_18;std::uint32_t value_1c,value_20;};
struct NativeSessionMessage38 {NativeSessionMessageStorage base;std::uint32_t words_18[2];};
struct NativeSessionMessage39 {NativeSessionMessageStorage base;std::uint32_t words_18[2];};
struct NativeSessionMessage40 {NativeSessionMessageStorage base;std::uint32_t words_18[2],words_20[2];std::uint8_t flag_28,retained_29[7];};
// The native D03608 table has five slots. Trailing borrowed context is SOURCE-ONLY.
// The actual numeric constants and maximum-float cell must outlive this profile.
struct NativeSessionMessage36Profile {
    const std::uint32_t slots[5];
    const NativeBitNumericContext* const numeric;
    const volatile float* const maximum_00d7a248;
    NativeSessionMessage36Profile(const NativeBitNumericContext&,const volatile float&);
};
static_assert(std::is_standard_layout_v<NativeSessionMessage36Profile>);
static_assert(offsetof(NativeSessionMessage36Profile,slots)==0);
static_assert(sizeof(NativeSessionMessage36)==0x28);
NativeSessionMessage36* construct_native_session_message36_00764670(NativeSessionMessage36*,const NativeSessionMessageContext&,const NativeSessionMessage36Profile&);
bool native_session_message_is36_007646e0(std::uint32_t);
NativeSessionMessage36* delete_native_session_message36_00764890(NativeSessionMessage36*,std::uint32_t);
void write_native_session_message36_007647a0(const NativeSessionMessage36*,NativeBitCursor*,const NativeBitNumericContext&,const volatile float&);
void read_native_session_message36_00764820(NativeSessionMessage36*,NativeSessionReadStream*,const NativeBitNumericContext&,const volatile float&);
static_assert(sizeof(NativeSessionMessage37)==0x24);
const std::uint32_t* native_session_message37_profile_00d030c0();
NativeSessionMessage37* construct_native_session_message37_0075dd80(NativeSessionMessage37*,const NativeSessionMessageContext&);
bool native_session_message_is37_0075de00(std::uint32_t);
NativeSessionMessage37* delete_native_session_message37_0075deb0(NativeSessionMessage37*,std::uint32_t);
void write_native_session_message37_0075de10(const NativeSessionMessage37*,NativeBitCursor*);
void read_native_session_message37_0075de60(NativeSessionMessage37*,NativeSessionReadStream*);
static_assert(sizeof(NativeSessionMessage38)==0x20);
const std::uint32_t* native_session_message38_profile_00d0323c();
NativeSessionMessage38* construct_native_session_message38_0075f670(NativeSessionMessage38*,const NativeSessionMessageContext&);
bool native_session_message_is38_0075f6f0(std::uint32_t);
NativeSessionMessage38* delete_native_session_message38_0075f760(NativeSessionMessage38*,std::uint32_t);
void write_native_session_message38_0075f700(const NativeSessionMessage38*,NativeBitCursor*);
void read_native_session_message38_0075f730(NativeSessionMessage38*,NativeSessionReadStream*);
static_assert(sizeof(NativeSessionMessage39)==0x20);
const std::uint32_t* native_session_message39_profile_00d03250();
NativeSessionMessage39* construct_native_session_message39_0075f780(NativeSessionMessage39*,const NativeSessionMessageContext&);
bool native_session_message_is39_0075f800(std::uint32_t);
NativeSessionMessage39* delete_native_session_message39_0075f870(NativeSessionMessage39*,std::uint32_t);
void write_native_session_message39_0075f810(const NativeSessionMessage39*,NativeBitCursor*);
void read_native_session_message39_0075f840(NativeSessionMessage39*,NativeSessionReadStream*);
static_assert(sizeof(NativeSessionMessage40)==0x30);
const std::uint32_t* native_session_message40_profile_00d03264();
NativeSessionMessage40* construct_native_session_message40_0075f890(NativeSessionMessage40*,const NativeSessionMessageContext&);
bool native_session_message_is40_0075f910(std::uint32_t);
NativeSessionMessage40* delete_native_session_message40_0075f9b0(NativeSessionMessage40*,std::uint32_t);
void write_native_session_message40_0075f920(const NativeSessionMessage40*,NativeBitCursor*);
void read_native_session_message40_0075f970(NativeSessionMessage40*,NativeSessionReadStream*);
} // namespace bsp

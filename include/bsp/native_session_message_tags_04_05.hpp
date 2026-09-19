#pragma once
#include "bsp/native_session_message_base.hpp"
#include "bsp/native_bit_cursor_numeric.hpp"
#include <type_traits>
namespace bsp {
// Field names describe offsets and storage only; semantic meanings remain open.
// Constructors retain every payload and padding byte.
struct NativeSessionMessage04 {
    NativeSessionMessageStorage base;
    std::int32_t signed_18;
    std::uint32_t retained_1c;
    std::uint32_t words_20[2];
    char text_28[32];
    char text_48[8];
    std::uint32_t value_50,value_54;
    std::uint8_t flag_58,retained_59[3];
    std::uint32_t value_5c;
    std::uint8_t value_60,value_61,bytes_62[36],bytes_86[16],bytes_96[8];
    std::uint8_t flag_9e,retained_9f;
    std::int32_t signed_a0[3];
    std::uint32_t retained_ac,words_b0[2],value_b8,value_bc;
};
struct NativeSessionMessage05 {
    NativeSessionMessageStorage base;
    std::int8_t signed_18,signed_19;
    char text_1a[32];
    std::uint8_t retained_3a,flag_3b;
    std::int8_t signed_3c,signed_3d;
    std::uint8_t retained_3e[2];
    std::uint32_t float_bits_40;
    std::int8_t signed_44,signed_45;
    std::uint8_t retained_46[2];
    std::uint32_t words_48[2],values_50[13],retained_84;
};
static_assert(sizeof(NativeSessionMessage04)==0xc0);
static_assert(offsetof(NativeSessionMessage04,text_28)==0x28);
static_assert(offsetof(NativeSessionMessage04,text_48)==0x48);
static_assert(offsetof(NativeSessionMessage04,bytes_62)==0x62);
static_assert(offsetof(NativeSessionMessage04,flag_9e)==0x9e);
static_assert(offsetof(NativeSessionMessage04,words_b0)==0xb0);
static_assert(sizeof(NativeSessionMessage05)==0x88);
static_assert(offsetof(NativeSessionMessage05,flag_3b)==0x3b);
static_assert(offsetof(NativeSessionMessage05,float_bits_40)==0x40);
static_assert(offsetof(NativeSessionMessage05,words_48)==0x48);
static_assert(offsetof(NativeSessionMessage05,values_50)==0x50);
// Five virtual slots followed by SOURCE-ONLY borrowed numeric metadata.
// Native D02FF8 has five slots. Caller owns the profile/context lifetimes.
struct NativeSessionMessage05Profile {
    const std::uint32_t slots[5];
    const NativeBitNumericContext* const numeric;
    explicit NativeSessionMessage05Profile(const NativeBitNumericContext&);
};
static_assert(std::is_standard_layout_v<NativeSessionMessage05Profile>);
static_assert(offsetof(NativeSessionMessage05Profile,slots)==0);
const std::uint32_t* native_session_message04_profile_00d03048();
NativeSessionMessage04* construct_native_session_message04_0075d5d0(NativeSessionMessage04*,const NativeSessionMessageContext&);
NativeSessionMessage05* construct_native_session_message05_0075d010(NativeSessionMessage05*,const NativeSessionMessageContext&,const NativeSessionMessage05Profile&);
void write_native_session_message04_0075d660(const NativeSessionMessage04*,NativeBitCursor*);
void read_native_session_message04_0075d7e0(NativeSessionMessage04*,NativeSessionReadStream*);
void write_native_session_message05_0075d0e0(const NativeSessionMessage05*,NativeBitCursor*,const NativeBitNumericContext&);
void read_native_session_message05_0075d1d0(NativeSessionMessage05*,NativeSessionReadStream*,const NativeBitNumericContext&);
bool native_session_message_is04_0075d650(std::uint32_t);
bool native_session_message_is05_0075d0d0(std::uint32_t);
NativeSessionMessage04* delete_native_session_message04_0075d970(NativeSessionMessage04*,std::uint32_t flags);
NativeSessionMessage05* delete_native_session_message05_0075d2c0(NativeSessionMessage05*,std::uint32_t flags);
} // namespace bsp

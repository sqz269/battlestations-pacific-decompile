#pragma once
#include "bsp/native_session_message_tags_69_75_76.hpp"
#include "bsp/native_bit_cursor_numeric.hpp"

namespace bsp {
// Complete normal bodies. Offset names are descriptive hypotheses. Constructors
// retain payload/padding; the two profiles have different type predicates.
struct NativeSessionMessage111 {
    NativeMessage75ExtendedHeader header;
    std::uint32_t values_20[6];
};
struct NativeSessionMessage112 {
    NativeSessionMessageStorage base;
    std::uint16_t sender_18;
    std::uint8_t relay_1a,retained_1b;
    std::uint16_t value_1c;
    std::uint8_t retained_1e[2];
    std::uint32_t float_bits_20[3];
};
static_assert(sizeof(NativeSessionMessage111)==0x38 && offsetof(NativeSessionMessage111,values_20)==0x20);
static_assert(sizeof(NativeSessionMessage112)==0x2c && offsetof(NativeSessionMessage112,value_1c)==0x1c && offsetof(NativeSessionMessage112,float_bits_20)==0x20);
struct NativeSessionMessage111To112Context {
    const NativeSessionMessageContext* session;
    const NativeBitNumericContext* numeric;
    const volatile float* maximum_00d7a248;
};
// Five native virtual slots followed by a SOURCE-ONLY borrowed context pointer.
// Both profile and context must outlive their users. Full binary ABI is unproved.
#define BSP_MESSAGE_PROFILE(N,C,P,D,S,W,R) \
struct NativeSessionMessage##N##Profile {const std::uint32_t slots[5];const NativeSessionMessage111To112Context* const context;explicit NativeSessionMessage##N##Profile(const NativeSessionMessage111To112Context&);}; \
static_assert(std::is_standard_layout_v<NativeSessionMessage##N##Profile>); \
NativeSessionMessage##N* construct_native_session_message##N##_##C(NativeSessionMessage##N*,const NativeSessionMessage111To112Context&,const NativeSessionMessage##N##Profile&); \
bool native_session_message_is##N##_##P(std::uint32_t); \
void destroy_native_session_message##N##_##D(NativeSessionMessage##N*); \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N*,std::uint32_t); \
void write_native_session_message##N##_##W(const NativeSessionMessage##N*,NativeBitCursor*,const NativeSessionMessage111To112Context&); \
void read_native_session_message##N##_##R(NativeSessionMessage##N*,NativeSessionReadStream*,const NativeSessionMessage111To112Context&);
BSP_MESSAGE_PROFILE(111,00761c20,00761c90,00761c80,00761cb0,00761cd0,00761d30)
BSP_MESSAGE_PROFILE(112,00761d90,00761df0,00761e10,00762640,00761e20,00761ec0)
#undef BSP_MESSAGE_PROFILE
} // namespace bsp

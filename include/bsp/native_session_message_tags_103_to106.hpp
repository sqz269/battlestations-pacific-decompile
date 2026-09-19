#pragma once
#include "bsp/native_session_message_tags_99_to102.hpp"
namespace bsp {
struct NativeSessionMessage103 {
    NativeSessionMessage98ScalarStorage scalar;
    std::uint8_t flag_24;
    std::uint8_t padding_25[3];
};
using NativeSessionMessage104=NativeSessionMessage98ScalarStorage;
using NativeSessionMessage105=NativeSessionMessage98ScalarStorage;
struct NativeSessionMessage106 {
    NativeSessionMessage98ScalarStorage scalar;
    std::int32_t value_24,value_28;
    std::uint32_t value_2c,value_30;
    std::int32_t value_34;
};
static_assert(sizeof(NativeSessionMessage103)==0x28 && offsetof(NativeSessionMessage103,flag_24)==0x24);
static_assert(sizeof(NativeSessionMessage106)==0x38 && offsetof(NativeSessionMessage106,value_34)==0x34);
struct NativeSessionMessage103To106Context {
    // First-member identity lets shared scalar adapters read the common context.
    NativeSessionMessage99To102Context scalar;
    const volatile float* maximum_00d7a248;
};
static_assert(std::is_standard_layout_v<NativeSessionMessage103To106Context> && offsetof(NativeSessionMessage103To106Context,scalar)==0);
void write_native_session_message103_0075c640(const NativeSessionMessage103*,NativeBitCursor*,const NativeSessionMessage103To106Context&);
void read_native_session_message103_0075c660(NativeSessionMessage103*,NativeSessionReadStream*,const NativeSessionMessage103To106Context&);
void write_native_session_message106_007617e0(const NativeSessionMessage106*,NativeBitCursor*,const NativeSessionMessage103To106Context&);
void read_native_session_message106_00761860(NativeSessionMessage106*,NativeSessionReadStream*,const NativeSessionMessage103To106Context&);
#define BSP_MESSAGE_PROFILE(N,C,P,D,S) \
struct NativeSessionMessage##N##Profile : NativeSessionMessage99To102ProfileStorage {explicit NativeSessionMessage##N##Profile(const NativeSessionMessage103To106Context&);}; \
static_assert(std::is_standard_layout_v<NativeSessionMessage##N##Profile>); \
NativeSessionMessage##N* construct_native_session_message##N##_##C(NativeSessionMessage##N*,const NativeSessionMessage103To106Context&,const NativeSessionMessage##N##Profile&); \
bool native_session_message_is##N##_##P(std::uint32_t); \
void destroy_native_session_message##N##_##D(NativeSessionMessage##N*); \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N*,std::uint32_t);
BSP_MESSAGE_PROFILE(103,0075a1f0,0075a240,0075a230,0075a270)
BSP_MESSAGE_PROFILE(104,00728f20,00728f70,00728f60,00729050)
BSP_MESSAGE_PROFILE(105,00728fa0,00728ff0,00728fe0,00729070)
BSP_MESSAGE_PROFILE(106,00761710,00761790,00761780,007617c0)
#undef BSP_MESSAGE_PROFILE
} // namespace bsp

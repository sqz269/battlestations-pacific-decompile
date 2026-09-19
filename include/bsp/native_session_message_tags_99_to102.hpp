#pragma once
#include "bsp/native_session_message_tags_69_75_76.hpp"
#include "bsp/native_bit_cursor_numeric.hpp"
namespace bsp {
// Shared category98 storage; three concrete profiles use the same codec slots.
struct NativeSessionMessage98ScalarStorage {NativeMessage75ExtendedHeader header;std::uint32_t value_20;};
using NativeSessionMessage99=NativeSessionMessage98ScalarStorage;
using NativeSessionMessage100=NativeSessionMessage98ScalarStorage;
struct NativeSessionMessage101 {NativeSessionMessage98ScalarStorage scalar;std::uint32_t value_24;};
using NativeSessionMessage102=NativeSessionMessage98ScalarStorage;
static_assert(sizeof(NativeSessionMessage98ScalarStorage)==0x24 && sizeof(NativeSessionMessage101)==0x28 && offsetof(NativeSessionMessage101,value_24)==0x24);
struct NativeSessionMessage99To102Context {
    const NativeSessionMessageContext* session;
    const NativeBitNumericContext* numeric;
    const volatile float* default_00cf8b3c;
    const volatile double* threshold_00d7a3a0;
    const volatile double* write_scale_00d7a328;
    const volatile double* read_scale_00d7a348;
};
struct NativeSessionMessage99To102ProfileStorage {
    const std::uint32_t slots[5];
    const NativeSessionMessage99To102Context* const context;
    NativeSessionMessage99To102ProfileStorage(const NativeSessionMessage99To102Context&,std::uint32_t scalar,std::uint32_t writer,std::uint32_t reader,std::uint32_t predicate);
};
static_assert(std::is_standard_layout_v<NativeSessionMessage99To102ProfileStorage>);
void write_native_session_scalar98_006d2f20(const NativeSessionMessage98ScalarStorage*,NativeBitCursor*,const NativeSessionMessage99To102Context&);
void read_native_session_scalar98_006d1550(NativeSessionMessage98ScalarStorage*,NativeSessionReadStream*,const NativeSessionMessage99To102Context&);
void write_native_session_message101_0075c520(const NativeSessionMessage101*,NativeBitCursor*,const NativeSessionMessage99To102Context&);
void read_native_session_message101_0075c540(NativeSessionMessage101*,NativeSessionReadStream*,const NativeSessionMessage99To102Context&);
#define BSP_MESSAGE_PROFILE(N,C,P,D,S) \
struct NativeSessionMessage##N##Profile : NativeSessionMessage99To102ProfileStorage {explicit NativeSessionMessage##N##Profile(const NativeSessionMessage99To102Context&);}; \
static_assert(std::is_standard_layout_v<NativeSessionMessage##N##Profile>); \
NativeSessionMessage##N* construct_native_session_message##N##_##C(NativeSessionMessage##N*,const NativeSessionMessage99To102Context&,const NativeSessionMessage##N##Profile&); \
bool native_session_message_is##N##_##P(std::uint32_t); \
void destroy_native_session_message##N##_##D(NativeSessionMessage##N*); \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N*,std::uint32_t);
BSP_MESSAGE_PROFILE(99,0075c380,0075c400,0075c3f0,0075c430)
BSP_MESSAGE_PROFILE(100,0075c450,0075c4d0,0075c4c0,0075c500)
BSP_MESSAGE_PROFILE(101,0075a150,0075a1a0,0075a190,0075a1d0)
BSP_MESSAGE_PROFILE(102,0075c570,0075c5f0,0075c5e0,0075c620)
#undef BSP_MESSAGE_PROFILE
} // namespace bsp

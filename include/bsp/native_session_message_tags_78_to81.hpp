#pragma once
#include "bsp/native_session_message_tags_69_75_76.hpp"

namespace bsp {
// Reuse the established extended-header layout through byte1F.
struct NativeSessionMessage78 {
    NativeMessage75ExtendedHeader header;
    std::uint8_t flag_20,retained_21[3];std::uint32_t value_24;
};
struct NativeSessionMessage79 {NativeMessage75ExtendedHeader header;std::uint32_t value_20;};
struct NativeSessionMessage80 {NativeMessage75ExtendedHeader header;std::int32_t value_20;};
struct NativeSessionMessage81 {NativeMessage75ExtendedHeader header;};
static_assert(sizeof(NativeSessionMessage78)==0x28 && offsetof(NativeSessionMessage78,value_24)==0x24);
static_assert(sizeof(NativeSessionMessage79)==0x24 && sizeof(NativeSessionMessage80)==0x24);
static_assert(sizeof(NativeSessionMessage81)==0x20);
// Native five-slot order; the trailing borrowed context is source-only and
// must outlive the profiles and all messages that use them.
#define BSP_MESSAGE_PROFILE(N,C,P,W,R,D,S) \
struct NativeSessionMessage##N##Profile {const std::uint32_t slots[5];const NativeSessionMessageContext* const context;explicit NativeSessionMessage##N##Profile(const NativeSessionMessageContext&);}; \
static_assert(std::is_standard_layout_v<NativeSessionMessage##N##Profile>); \
NativeSessionMessage##N* construct_native_session_message##N##_##C(NativeSessionMessage##N*,const NativeSessionMessageContext&,const NativeSessionMessage##N##Profile&); \
bool native_session_message_is##N##_##P(std::uint32_t); \
void write_native_session_message##N##_##W(const NativeSessionMessage##N*,NativeBitCursor*); \
void read_native_session_message##N##_##R(NativeSessionMessage##N*,NativeSessionReadStream*); \
void destroy_native_session_message##N##_##D(NativeSessionMessage##N*); \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N*,std::uint32_t);
BSP_MESSAGE_PROFILE(78,0075bd50,0075bdc0,0075be00,0075be60,0075bdb0,0075bde0)
BSP_MESSAGE_PROFILE(79,0075a030,0075a070,0075bec0,0075bf10,0075a060,0075a090)
BSP_MESSAGE_PROFILE(80,0075a0b0,0075a0e0,0075bf60,0075bfb0,0075a100,0075c000)
BSP_MESSAGE_PROFILE(81,005f9660,005f9690,005f96b0,005f96d0,005f96f0,005f9a00)
#undef BSP_MESSAGE_PROFILE
} // namespace bsp

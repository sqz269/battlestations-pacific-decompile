#pragma once
#include "bsp/native_session_message_tags_69_75_76.hpp"

namespace bsp {
struct NativeSessionMessage82 {NativeMessage75ExtendedHeader header;std::uint8_t flag_20,retained_21[3];};
struct NativeSessionMessage83 {NativeMessage75ExtendedHeader header;std::uint32_t value_20;};
struct NativeSessionMessage84 {
    NativeMessage75ExtendedHeader header;std::uint32_t length_20;char* data_24;
    std::uint8_t flag_28,retained_29[3];
};
static_assert(sizeof(NativeSessionMessage82)==0x24 && sizeof(NativeSessionMessage83)==0x24);
static_assert(sizeof(NativeSessionMessage84)==0x2c && offsetof(NativeSessionMessage84,length_20)==0x20);
struct NativeSessionMessage82To84Context {
    const NativeSessionMessageContext* session;NativeStringRawPoolContext* strings;
    const char* fallback_00e17669;
};
// Five native slots followed by a source-only borrowed context pointer.
#define BSP_MESSAGE_PROFILE(N,C,P,W,R,D,S) \
struct NativeSessionMessage##N##Profile {const std::uint32_t slots[5];const NativeSessionMessage82To84Context* const context;explicit NativeSessionMessage##N##Profile(const NativeSessionMessage82To84Context&);}; \
static_assert(std::is_standard_layout_v<NativeSessionMessage##N##Profile>); \
NativeSessionMessage##N* construct_native_session_message##N##_##C(NativeSessionMessage##N*,const NativeSessionMessage82To84Context&,const NativeSessionMessage##N##Profile&); \
bool native_session_message_is##N##_##P(std::uint32_t); \
void write_native_session_message##N##_##W(const NativeSessionMessage##N*,NativeBitCursor*,const NativeSessionMessage82To84Context&); \
void read_native_session_message##N##_##R(NativeSessionMessage##N*,NativeSessionReadStream*,const NativeSessionMessage82To84Context&); \
void destroy_native_session_message##N##_##D(NativeSessionMessage##N*,const NativeSessionMessage82To84Context&); \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N*,std::uint32_t,const NativeSessionMessage82To84Context&);
BSP_MESSAGE_PROFILE(82,0075a3b0,0075a3e0,0075c900,0075c950,0075a400,0075ca60)
// No independent native83 destructor is asserted: this is the root-stamp
// operation embedded in its scalar wrapper.
BSP_MESSAGE_PROFILE(83,0075c9a0,004b5c50,004b5c70,004b5ca0,source,004bd550)
BSP_MESSAGE_PROFILE(84,00765a20,00765a90,00765ab0,00765b10,00765b70,00766830)
#undef BSP_MESSAGE_PROFILE
NativeSessionMessage83* construct_native_session_message83_value_004b5c00(NativeSessionMessage83*,std::uint32_t,const NativeSessionMessage82To84Context&,const NativeSessionMessage83Profile&);
} // namespace bsp

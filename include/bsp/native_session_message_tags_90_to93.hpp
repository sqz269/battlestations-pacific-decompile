#pragma once
#include "bsp/native_session_message88.hpp"
namespace bsp {
struct NativeSessionMessage90 {NativeMessage75ExtendedHeader header;std::uint32_t selector_20,value_24;};
struct NativeSessionMessage91 {NativeMessage75ExtendedHeader header;std::uint16_t value_20,retained_22;std::uint32_t value_24,value_28;};
struct NativeSessionMessage92 {NativeMessage75ExtendedHeader header;std::uint8_t flags_20,command_21,retained_22[2];SceneCommandTarget target_24;};
struct NativeSessionMessage93 {NativeMessage75ExtendedHeader header;std::uint8_t flag_20,retained_21[3];std::int32_t value_24;};
static_assert(sizeof(NativeSessionMessage90)==0x28 && sizeof(NativeSessionMessage91)==0x2c && sizeof(NativeSessionMessage92)==0x3c && sizeof(NativeSessionMessage93)==0x28);
static_assert(offsetof(NativeSessionMessage92,target_24)==0x24);
struct NativeSessionMessage90To93Context {
    const NativeSessionMessageContext* session;
    NativeSessionMessage88Context target;
    const volatile float* position_scale_00cf9360;
};
#define BSP_MESSAGE_PROFILE(N,C,W,R,D,S) \
struct NativeSessionMessage##N##Profile {const std::uint32_t slots[5];const NativeSessionMessage90To93Context* const context;explicit NativeSessionMessage##N##Profile(const NativeSessionMessage90To93Context&);}; \
static_assert(std::is_standard_layout_v<NativeSessionMessage##N##Profile>); \
NativeSessionMessage##N* construct_native_session_message##N##_##C(NativeSessionMessage##N*,const NativeSessionMessage90To93Context&,const NativeSessionMessage##N##Profile&); \
void write_native_session_message##N##_##W(const NativeSessionMessage##N*,NativeBitCursor*,const NativeSessionMessage90To93Context&); \
void read_native_session_message##N##_##R(NativeSessionMessage##N*,NativeSessionReadStream*,const NativeSessionMessage90To93Context&); \
void destroy_native_session_message##N##_##D(NativeSessionMessage##N*); \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N*,std::uint32_t);
BSP_MESSAGE_PROFILE(90,0075af60,0071c670,0071c6c0,0071c720,0071e370)
BSP_MESSAGE_PROFILE(91,0075af90,00763fc0,00764030,0075b020,007640b0)
BSP_MESSAGE_PROFILE(92,00764f90,0071c930,0071ca30,0071cb40,0071e3b0)
BSP_MESSAGE_PROFILE(93,0075b030,0071c7a0,0071c7e0,0071c820,0071e390)
#undef BSP_MESSAGE_PROFILE
bool native_session_message_is90_0071c640(std::uint32_t);
bool native_session_message_is91_0075aff0(std::uint32_t);
bool native_session_message_is93_0071c770(std::uint32_t);
// Reuses gameunit_set_command_message_is_category_0071c900 from cruise_command.hpp.
// Native value constructor: ECX=this; command, target, flag on stack; RET0Ch.
NativeSessionMessage92* construct_native_session_message92_value_0071c830(NativeSessionMessage92*,const void* command,const SceneCommandTarget*,std::uint8_t flag,const NativeSessionMessage90To93Context&,const NativeSessionMessage92Profile&);
} // namespace bsp

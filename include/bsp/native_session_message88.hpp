#pragma once
#include "bsp/native_session_message_tags_69_75_76.hpp"
#include "bsp/native_bit_cursor_numeric_array.hpp"
#include "bsp/scene_deferred_refs.hpp"

namespace bsp {
// Native allocation is 3Ch. Reuse the command target consumed by cruise commands.
struct NativeSessionMessage88 {
    NativeMessage75ExtendedHeader header;
    std::uint8_t command_20,flags_21,retained_22[2];
    SceneCommandTarget target_24;
};
static_assert(sizeof(NativeSessionMessage88)==0x3c && offsetof(NativeSessionMessage88,target_24)==0x24);
static_assert(sizeof(SceneCommandTarget)==0x18 && offsetof(SceneCommandTarget,position)==8 && offsetof(SceneCommandTarget,trailing)==0x14);
struct NativeSessionMessage88Context {
    const NativeBitNumericContext* numeric;
    const volatile float* default_vector_00f87574;
    const volatile float* maximum_00d7a248;
    const volatile float* threshold_00d7a24c;
};
// Five native slots and one borrowed, source-only context pointer.
struct NativeSessionMessage88Profile {
    const std::uint32_t slots[5];
    const NativeSessionMessage88Context* const context;
    explicit NativeSessionMessage88Profile(const NativeSessionMessage88Context&);
};
static_assert(std::is_standard_layout_v<NativeSessionMessage88Profile>);
NativeSessionMessage88* construct_native_session_message88_00764c80(NativeSessionMessage88*,const NativeSessionMessage88Context&,const NativeSessionMessage88Profile&);
void write_native_session_message88_00764d40(const NativeSessionMessage88*,NativeBitCursor*,const NativeSessionMessage88Context&);
void read_native_session_message88_00764e60(NativeSessionMessage88*,NativeSessionReadStream*,const NativeSessionMessage88Context&);
void destroy_native_session_message88_00764cf0(NativeSessionMessage88*);
NativeSessionMessage88* delete_native_session_message88_00764d20(NativeSessionMessage88*,std::uint32_t);
// Predicate 00764D00 is entity_command_message_is_category_00764d00 in cruise_command.hpp.
} // namespace bsp

#pragma once
#include "bsp/native_session_message_base.hpp"
#include "bsp/native_bit_cursor_numeric.hpp"
#include <type_traits>
namespace bsp {
struct NativeSessionMessage21 {
    NativeSessionMessageStorage base;
    std::int32_t subtype_18,subject_1c;
    std::uint32_t value_bits_20,value_bits_24,value_bits_28;
    std::int32_t value_2c,value_30,value_34,values_38[7],values_54[4],value_64;
    std::int32_t value_68,value_6c,value_70,value_74,value_78;
    char text_7c[0x100];
    std::int32_t value_17c,value_180,value_184,value_188;
    std::uint32_t value_bits_18c;
};
struct NativeSessionMessage22 {
    NativeSessionMessageStorage base;
    std::int32_t subtype_18;
    char bytes_1c[0x100];
    std::int32_t counts_11c[6];
    std::uint8_t value_134,retained_135[3];
};
static_assert(sizeof(NativeSessionMessage21)==0x190);
static_assert(offsetof(NativeSessionMessage21,values_38)==0x38);
static_assert(offsetof(NativeSessionMessage21,values_54)==0x54);
static_assert(offsetof(NativeSessionMessage21,text_7c)==0x7c);
static_assert(offsetof(NativeSessionMessage21,value_17c)==0x17c);
static_assert(offsetof(NativeSessionMessage21,value_bits_18c)==0x18c);
static_assert(sizeof(NativeSessionMessage22)==0x138);
static_assert(offsetof(NativeSessionMessage22,counts_11c)==0x11c);
static_assert(offsetof(NativeSessionMessage22,value_134)==0x134);
// Native D035B8 has five slots. Trailing bindings are SOURCE-ONLY metadata;
// actual numeric constants and profile backing must outlive the message.
struct NativeSessionMessage21Profile {
    const std::uint32_t slots[5];
    const NativeBitNumericContext* const numeric;
    const volatile float* const maximum_00d7a248;
    NativeSessionMessage21Profile(const NativeBitNumericContext&,const volatile float&);
};
static_assert(std::is_standard_layout_v<NativeSessionMessage21Profile>);
static_assert(offsetof(NativeSessionMessage21Profile,slots)==0);
const std::uint32_t* native_session_message22_profile_00d035cc();
NativeSessionMessage21* construct_native_session_message21_00764340(NativeSessionMessage21*,const NativeSessionMessageContext&,const NativeSessionMessage21Profile&);
NativeSessionMessage22* construct_native_session_message22_007643e0(NativeSessionMessage22*,const NativeSessionMessageContext&);
void write_native_session_message21_009064f0(const NativeSessionMessage21*,NativeBitCursor*,const NativeBitNumericContext&,const volatile float&);
void read_native_session_message21_00906800(NativeSessionMessage21*,NativeSessionReadStream*,const NativeBitNumericContext&,const volatile float&);
// Mode6 captures the wrapping sum of six signed DWORD counts, then copies that
// many bytes if the sum is positive. The native code has no 256-byte bound.
// Callers must provide physical backing for the actual accesses, including any
// bytes past the nominal record. Counts can be overwritten during the read loop.
void write_native_session_message22_00906af0(const NativeSessionMessage22*,NativeBitCursor*);
void read_native_session_message22_00906c20(NativeSessionMessage22*,NativeSessionReadStream*);
bool native_session_message_is21_007643a0(std::uint32_t);
bool native_session_message_is22_00764450(std::uint32_t);
NativeSessionMessage21* delete_native_session_message21_007643c0(NativeSessionMessage21*,std::uint32_t);
NativeSessionMessage22* delete_native_session_message22_00764470(NativeSessionMessage22*,std::uint32_t);
} // namespace bsp

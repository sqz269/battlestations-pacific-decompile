#pragma once
#include "bsp/native_session_message_base.hpp"
#include "bsp/native_bit_cursor_numeric.hpp"
#include <type_traits>
namespace bsp {
struct NativeSessionMessage03 {NativeSessionMessageStorage base;std::uint32_t value_bits_18[3];};
struct NativeSessionMessage06 {NativeSessionMessageStorage base;std::int32_t value_18;};
using NativeSessionMessage07=NativeSessionMessageStorage;
static_assert(sizeof(NativeSessionMessage03)==0x24);
static_assert(sizeof(NativeSessionMessage06)==0x1c);
// Five ABI-visible slots followed by SOURCE-ONLY borrowed binding metadata.
// Native D02FE4 has only five slots. Keep this object and both bindings alive
// for every translated message using it; no fabricated global numeric state.
struct NativeSessionMessage03Profile {
    const std::uint32_t slots[5];
    const NativeBitNumericContext* const numeric;
    const volatile float* const maximum_float_00d7a248;
    NativeSessionMessage03Profile(const NativeBitNumericContext&,const volatile float&);
};
static_assert(std::is_standard_layout_v<NativeSessionMessage03Profile>);
static_assert(offsetof(NativeSessionMessage03Profile,slots)==0);
const std::uint32_t* native_session_message06_profile_00d0305c();
const std::uint32_t* native_session_message07_profile_00d03188();
NativeSessionMessage03* construct_native_session_message03_0075ce80(NativeSessionMessage03*,const NativeSessionMessageContext&,const NativeSessionMessage03Profile&);
NativeSessionMessage06* construct_native_session_message06_0075d990(NativeSessionMessage06*,const NativeSessionMessageContext&);
NativeSessionMessage07* construct_native_session_message07_0075e940(NativeSessionMessage07*,const NativeSessionMessageContext&);
void write_native_session_message03_0075cef0(const NativeSessionMessage03*,NativeBitCursor*,const NativeBitNumericContext&,const volatile float& maximum);
void read_native_session_message03_0075cf70(NativeSessionMessage03*,NativeSessionReadStream*,const NativeBitNumericContext&,const volatile float& maximum);
void write_native_session_message06_0075da20(const NativeSessionMessage06*,NativeBitCursor*);
void read_native_session_message06_0075da50(NativeSessionMessage06*,NativeSessionReadStream*);
void write_native_session_message07_0075e9d0(const NativeSessionMessage07*,NativeBitCursor*);
void read_native_session_message07_0075e9f0(NativeSessionMessage07*,NativeSessionReadStream*);
bool native_session_message_is03_0075cfe0(std::uint32_t);
bool native_session_message_is06_0075da10(std::uint32_t);
bool native_session_message_is07_0075e9c0(std::uint32_t);
NativeSessionMessage03* delete_native_session_message03_0075cff0(NativeSessionMessage03*,std::uint32_t flags);
NativeSessionMessage06* delete_native_session_message06_0075da80(NativeSessionMessage06*,std::uint32_t flags);
NativeSessionMessage07* delete_native_session_message07_0075ea10(NativeSessionMessage07*,std::uint32_t flags);
} // namespace bsp

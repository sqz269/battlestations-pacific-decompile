#pragma once
#include "bsp/native_session_message_base.hpp"
#include "bsp/native_string.hpp"
#include <type_traits>
namespace bsp {
struct NativeSessionMessage23 {NativeSessionMessageStorage base;std::uint32_t values_18[14];};
struct NativeSessionMessage24 {NativeSessionMessageStorage base;std::uint32_t length_18;char* data_1c;std::int32_t value_20;};
struct NativeSessionMessage25 {NativeSessionMessageStorage base;std::int32_t values_18[2];};
static_assert(sizeof(NativeSessionMessage23)==0x50);
static_assert(sizeof(NativeSessionMessage24)==0x24);
static_assert(offsetof(NativeSessionMessage24,length_18)==0x18);
static_assert(offsetof(NativeSessionMessage24,data_1c)==0x1c);
static_assert(sizeof(NativeSessionMessage25)==0x20);
// D036E4 has five native slots. Trailing bindings are SOURCE-ONLY metadata.
// Profile, actual pool context and E17669 backing must outlive the object.
struct NativeSessionMessage24Profile {
    const std::uint32_t slots[5];
    NativeStringRawPoolContext* const strings;
    const char* const fallback_00e17669;
    NativeSessionMessage24Profile(NativeStringRawPoolContext&,const char* actual_fallback_00e17669);
};
static_assert(std::is_standard_layout_v<NativeSessionMessage24Profile>);
static_assert(offsetof(NativeSessionMessage24Profile,slots)==0);
const std::uint32_t* native_session_message23_profile_00d030e8();
const std::uint32_t* native_session_message25_profile_00d03070();
NativeSessionMessage23* construct_native_session_message23_0075e000(NativeSessionMessage23*,const NativeSessionMessageContext&);
NativeSessionMessage24* construct_native_session_message24_00766060(NativeSessionMessage24*,const NativeSessionMessageContext&,const NativeSessionMessage24Profile&);
// Native constructor accepts a full DWORD type; factory 768A46 supplies25.
// It stores the low byte, while the predicate still tests fixed full DWORD25.
NativeSessionMessage25* construct_native_session_message25_0075daa0(NativeSessionMessage25*,std::uint32_t type,const NativeSessionMessageContext&);
void write_native_session_message23_0075e0a0(const NativeSessionMessage23*,NativeBitCursor*);
void read_native_session_message23_0075e0e0(NativeSessionMessage23*,NativeSessionReadStream*);
void write_native_session_message24_007660d0(const NativeSessionMessage24*,NativeBitCursor*,const char* actual_fallback_00e17669);
void read_native_session_message24_00766110(NativeSessionMessage24*,NativeSessionReadStream*,NativeStringRawPoolContext&);
void write_native_session_message25_0075dad0(const NativeSessionMessage25*,NativeBitCursor*);
void read_native_session_message25_0075db10(NativeSessionMessage25*,NativeSessionReadStream*);
bool native_session_message_is23_0075e090(std::uint32_t);
bool native_session_message_is24_007660c0(std::uint32_t);
bool native_session_message_is25_0075dac0(std::uint32_t);
void destroy_native_session_message24_00766150(NativeSessionMessage24*,NativeStringRawPoolContext&,const NativeSessionMessage24Profile&);
NativeSessionMessage23* delete_native_session_message23_0075e120(NativeSessionMessage23*,std::uint32_t);
NativeSessionMessage24* delete_native_session_message24_007661b0(NativeSessionMessage24*,std::uint32_t,NativeStringRawPoolContext&,const NativeSessionMessage24Profile&);
NativeSessionMessage25* delete_native_session_message25_0075db50(NativeSessionMessage25*,std::uint32_t);
} // namespace bsp

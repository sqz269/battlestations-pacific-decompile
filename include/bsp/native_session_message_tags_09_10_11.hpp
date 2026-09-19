#pragma once
#include "bsp/native_session_message_base.hpp"
#include "bsp/native_string.hpp"
#include <type_traits>
namespace bsp {
struct NativeSessionMessage09 {
    NativeSessionMessageStorage base;
    std::uint32_t length_18;
    char* data_1c;
    std::uint32_t value_20,retained_24,words_28[2];
};
using NativeSessionMessage10=NativeSessionMessageStorage;
struct NativeSessionMessage11 {NativeSessionMessageStorage base;std::uint32_t value_18;};
static_assert(sizeof(NativeSessionMessage09)==0x30);
static_assert(offsetof(NativeSessionMessage09,length_18)==0x18);
static_assert(offsetof(NativeSessionMessage09,words_28)==0x28);
static_assert(sizeof(NativeSessionMessage11)==0x1c);
// Native D036F8 has five slots. Trailing bindings are SOURCE-ONLY metadata;
// caller keeps the profile, actual raw pool context and empty F1AF25 backing alive.
struct NativeSessionMessage09Profile {
    const std::uint32_t slots[5];
    NativeStringRawPoolContext* const strings;
    const char* const empty_00f1af25;
    NativeSessionMessage09Profile(NativeStringRawPoolContext&,const char* actual_empty_00f1af25);
};
static_assert(std::is_standard_layout_v<NativeSessionMessage09Profile>);
static_assert(offsetof(NativeSessionMessage09Profile,slots)==0);
const std::uint32_t* native_session_message10_profile_00d03138();
const std::uint32_t* native_session_message11_profile_00d0314c();
NativeSessionMessage09* construct_native_session_message09_007661d0(NativeSessionMessage09*,const NativeSessionMessageContext&,const NativeSessionMessage09Profile&);
NativeSessionMessage10* construct_native_session_message10_0075e540(NativeSessionMessage10*,const NativeSessionMessageContext&);
NativeSessionMessage11* construct_native_session_message11_0075e630(NativeSessionMessage11*,const NativeSessionMessageContext&);
void write_native_session_message09_00766240(const NativeSessionMessage09*,NativeBitCursor*,const char* actual_empty_00f1af25);
void read_native_session_message09_00766300(NativeSessionMessage09*,NativeSessionReadStream*,NativeStringRawPoolContext&);
void write_native_session_message10_0075e5d0(const NativeSessionMessage10*,NativeBitCursor*);
void read_native_session_message10_0075e5f0(NativeSessionMessage10*,NativeSessionReadStream*);
void write_native_session_message11_0075e6c0(const NativeSessionMessage11*,NativeBitCursor*);
void read_native_session_message11_0075e6f0(NativeSessionMessage11*,NativeSessionReadStream*);
bool native_session_message_is09_00766230(std::uint32_t);
bool native_session_message_is10_0075e5c0(std::uint32_t);
bool native_session_message_is11_0075e6b0(std::uint32_t);
void destroy_native_session_message09_007662a0(NativeSessionMessage09*,NativeStringRawPoolContext&,const NativeSessionMessage09Profile&);
NativeSessionMessage09* delete_native_session_message09_007663d0(NativeSessionMessage09*,std::uint32_t,NativeStringRawPoolContext&,const NativeSessionMessage09Profile&);
NativeSessionMessage10* delete_native_session_message10_0075e610(NativeSessionMessage10*,std::uint32_t);
NativeSessionMessage11* delete_native_session_message11_0075e720(NativeSessionMessage11*,std::uint32_t);
} // namespace bsp

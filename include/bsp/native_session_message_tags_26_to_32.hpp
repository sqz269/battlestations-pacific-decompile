#pragma once
#include "bsp/native_session_message_tags_23_24_25.hpp"
namespace bsp {
// Original20h records. Raw flag bytes may be noncanonical before writing.
using NativeSessionMessage26=NativeSessionMessage25;
using NativeSessionMessage27=NativeSessionMessage25;
using NativeSessionMessage28=NativeSessionMessage25;
struct NativeSessionMessage29 {NativeSessionMessageStorage base;std::uint32_t value_18;std::uint8_t first_1c,second_1d,reserved_1e[2];};
struct NativeSessionMessage30 {NativeSessionMessageStorage base;std::uint32_t value_18;std::uint8_t flag_1c,reserved_1d[3];};
// Writer reads low byte1C; reader captures a local byte then stores full DWORD1C.
struct NativeSessionMessage31 {NativeSessionMessageStorage base;std::uint32_t value_18,value_1c;};
struct NativeSessionMessage32 {NativeSessionMessageStorage base;std::uint32_t value_18;std::uint8_t flag_1c,reserved_1d[3];};
static_assert(sizeof(NativeSessionMessage26)==0x20);
const std::uint32_t* native_session_message26_profile_00d03084();
NativeSessionMessage26* construct_native_session_message26_0075db70(NativeSessionMessage26*,const NativeSessionMessageContext&);
bool native_session_message_is26_0075dbf0(std::uint32_t);
NativeSessionMessage26* delete_native_session_message26_0075dc00(NativeSessionMessage26*,std::uint32_t);
static_assert(sizeof(NativeSessionMessage27)==0x20);
const std::uint32_t* native_session_message27_profile_00d03098();
NativeSessionMessage27* construct_native_session_message27_0075dc20(NativeSessionMessage27*,const NativeSessionMessageContext&);
bool native_session_message_is27_0075dca0(std::uint32_t);
NativeSessionMessage27* delete_native_session_message27_0075dcb0(NativeSessionMessage27*,std::uint32_t);
static_assert(sizeof(NativeSessionMessage28)==0x20);
const std::uint32_t* native_session_message28_profile_00d030ac();
NativeSessionMessage28* construct_native_session_message28_0075dcd0(NativeSessionMessage28*,const NativeSessionMessageContext&);
bool native_session_message_is28_0075dd50(std::uint32_t);
NativeSessionMessage28* delete_native_session_message28_0075dd60(NativeSessionMessage28*,std::uint32_t);
static_assert(sizeof(NativeSessionMessage29)==0x20);
const std::uint32_t* native_session_message29_profile_00d030d4();
NativeSessionMessage29* construct_native_session_message29_0075ded0(NativeSessionMessage29*,const NativeSessionMessageContext&);
bool native_session_message_is29_0075df50(std::uint32_t);
NativeSessionMessage29* delete_native_session_message29_0075dfe0(NativeSessionMessage29*,std::uint32_t);
void write_native_session_message29_0075df60(const NativeSessionMessage29*,NativeBitCursor*);
void read_native_session_message29_0075dfa0(NativeSessionMessage29*,NativeSessionReadStream*);
static_assert(sizeof(NativeSessionMessage30)==0x20);
const std::uint32_t* native_session_message30_profile_00d030fc();
NativeSessionMessage30* construct_native_session_message30_0075e140(NativeSessionMessage30*,const NativeSessionMessageContext&);
bool native_session_message_is30_0075e1c0(std::uint32_t);
NativeSessionMessage30* delete_native_session_message30_0075e250(NativeSessionMessage30*,std::uint32_t);
void write_native_session_message30_0075e1d0(const NativeSessionMessage30*,NativeBitCursor*);
void read_native_session_message30_0075e210(NativeSessionMessage30*,NativeSessionReadStream*);
static_assert(sizeof(NativeSessionMessage31)==0x20);
const std::uint32_t* native_session_message31_profile_00d03110();
NativeSessionMessage31* construct_native_session_message31_0075e270(NativeSessionMessage31*,const NativeSessionMessageContext&);
bool native_session_message_is31_0075e2f0(std::uint32_t);
NativeSessionMessage31* delete_native_session_message31_0075e380(NativeSessionMessage31*,std::uint32_t);
void write_native_session_message31_0075e300(const NativeSessionMessage31*,NativeBitCursor*);
void read_native_session_message31_0075e340(NativeSessionMessage31*,NativeSessionReadStream*);
static_assert(sizeof(NativeSessionMessage32)==0x20);
const std::uint32_t* native_session_message32_profile_00d03124();
NativeSessionMessage32* construct_native_session_message32_0075e3a0(NativeSessionMessage32*,const NativeSessionMessageContext&);
bool native_session_message_is32_0075e420(std::uint32_t);
NativeSessionMessage32* delete_native_session_message32_0075e4b0(NativeSessionMessage32*,std::uint32_t);
void write_native_session_message32_0075e430(const NativeSessionMessage32*,NativeBitCursor*);
void read_native_session_message32_0075e470(NativeSessionMessage32*,NativeSessionReadStream*);
// Native ECX=this. Constructors RET with EAX identity; predicates/scalars
// take one stack DWORD and RET4. Writers use raw cursors, readers18h wrappers.
// These explicit C++ interfaces are not a whole-binary ABI claim.
} // namespace bsp

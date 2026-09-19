#pragma once
#include "bsp/native_session_message_base.hpp"
namespace bsp {
using NativeSessionMessage12=NativeSessionMessageStorage;
struct NativeSessionMessage13 {NativeSessionMessageStorage base;std::uint8_t flag_18,retained_19[3];};
struct NativeSessionMessage14 {NativeSessionMessageStorage base;std::uint8_t flag_18,retained_19[3];};
struct NativeSessionMessage15 {NativeSessionMessageStorage base;std::uint16_t count_18;std::uint8_t retained_1a[2];};
static_assert(sizeof(NativeSessionMessage12)==0x18);
const std::uint32_t* native_session_message12_profile_00ce74c8();
NativeSessionMessage12* construct_native_session_message12_004b5fb0(NativeSessionMessage12*,const NativeSessionMessageContext&);
void write_native_session_message12_004b5ff0(const NativeSessionMessage12*,NativeBitCursor*);
void read_native_session_message12_004b6010(NativeSessionMessage12*,NativeSessionReadStream*);
bool native_session_message_is12_004b5fe0(std::uint32_t);
NativeSessionMessage12* delete_native_session_message12_004b6030(NativeSessionMessage12*,std::uint32_t);
static_assert(sizeof(NativeSessionMessage13)==0x1c);
static_assert(offsetof(NativeSessionMessage13,flag_18)==0x18);
const std::uint32_t* native_session_message13_profile_00d03034();
NativeSessionMessage13* construct_native_session_message13_0075d4b0(NativeSessionMessage13*,const NativeSessionMessageContext&);
void write_native_session_message13_0075d550(const NativeSessionMessage13*,NativeBitCursor*);
void read_native_session_message13_0075d580(NativeSessionMessage13*,NativeSessionReadStream*);
bool native_session_message_is13_0075d530(const NativeSessionMessage13*,std::uint32_t);
NativeSessionMessage13* delete_native_session_message13_0075d5b0(NativeSessionMessage13*,std::uint32_t);
static_assert(sizeof(NativeSessionMessage14)==0x1c);
static_assert(offsetof(NativeSessionMessage14,flag_18)==0x18);
const std::uint32_t* native_session_message14_profile_00d0300c();
NativeSessionMessage14* construct_native_session_message14_0075d2e0(NativeSessionMessage14*,std::uint32_t raw_flag,const NativeSessionMessageContext&);
void write_native_session_message14_0075d370(const NativeSessionMessage14*,NativeBitCursor*);
void read_native_session_message14_0075d3a0(NativeSessionMessage14*,NativeSessionReadStream*);
bool native_session_message_is14_0075d350(const NativeSessionMessage14*,std::uint32_t);
NativeSessionMessage14* delete_native_session_message14_0075d3d0(NativeSessionMessage14*,std::uint32_t);
static_assert(sizeof(NativeSessionMessage15)==0x1c);
static_assert(offsetof(NativeSessionMessage15,count_18)==0x18);
const std::uint32_t* native_session_message15_profile_00d02fbc();
NativeSessionMessage15* construct_native_session_message15_0075caa0(NativeSessionMessage15*,const NativeSessionMessageContext&);
void write_native_session_message15_0075cb10(const NativeSessionMessage15*,NativeBitCursor*);
void read_native_session_message15_0075cb60(NativeSessionMessage15*,NativeSessionReadStream*);
bool native_session_message_is15_0075cb00(std::uint32_t);
NativeSessionMessage15* delete_native_session_message15_0075ce40(NativeSessionMessage15*,std::uint32_t);
// Native ECX object; constructors RET except type14 RET4, all other methods RET4.
// Type13/14 retain raw byte flags; predicates accept the fixed OR current byte tag.
// Type15 writes count copies of 0x21 and consumes/discards count bytes on read.
// Loops reload the unsigned count after every byte; callers supply backing.
} // namespace bsp

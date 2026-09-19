#pragma once
#include "bsp/native_session_message_base.hpp"
#include "bsp/native_bit_cursor_numeric.hpp"
#include "bsp/native_smoothed_remainder_triplet.hpp"
#include <type_traits>
namespace bsp {
struct NativeSessionMessage16 {NativeSessionMessageStorage base;NativeSmoothedRemainderTriplet records_18[8];};
using NativeSessionMessage17=NativeSessionMessageStorage;
using NativeSessionMessage18=NativeSessionMessageStorage;
struct NativeSessionMessage19 {NativeSessionMessageStorage base;std::uint8_t flag_18,retained_19[3];};
struct NativeSessionMessage20 {NativeSessionMessageStorage base;std::uint8_t flag_18,retained_19[3];};
static_assert(sizeof(NativeSessionMessage16)==0x78);
static_assert(offsetof(NativeSessionMessage16,records_18)==0x18);
static_assert(sizeof(NativeSessionMessage19)==0x1c);
static_assert(sizeof(NativeSessionMessage20)==0x1c);
// Native D02FD0 has five slots; these trailing borrowed bindings are SOURCE-ONLY.
// Actual numeric/smoothing contexts and their backing must outlive this profile.
struct NativeSessionMessage16Profile {
    const std::uint32_t slots[5];
    const NativeBitNumericContext* const numeric;
    const NativeTripletSmoothingContext* const smoothing;
    NativeSessionMessage16Profile(const NativeBitNumericContext&,const NativeTripletSmoothingContext&);
};
static_assert(std::is_standard_layout_v<NativeSessionMessage16Profile>);
static_assert(offsetof(NativeSessionMessage16Profile,slots)==0);
NativeSessionMessage16* construct_native_session_message16_0075cbc0(NativeSessionMessage16*,const NativeSessionMessageContext&,const NativeSessionMessage16Profile&);
bool native_session_message_is16_0075cd30(std::uint32_t);
NativeSessionMessage16* delete_native_session_message16_0075ce60(NativeSessionMessage16*,std::uint32_t);
void write_native_session_message16_0075cd40(const NativeSessionMessage16*,NativeBitCursor*,const NativeBitNumericContext&);
void read_native_session_message16_0075cdb0(NativeSessionMessage16*,NativeSessionReadStream*,const NativeBitNumericContext&,const NativeTripletSmoothingContext&);
const std::uint32_t* native_session_message17_profile_00d03020();
NativeSessionMessage17* construct_native_session_message17_0075d3f0(NativeSessionMessage17*,const NativeSessionMessageContext&);
bool native_session_message_is17_0075d470(const NativeSessionMessage17*,std::uint32_t);
NativeSessionMessage17* delete_native_session_message17_0075d490(NativeSessionMessage17*,std::uint32_t);
const std::uint32_t* native_session_message18_profile_00d03160();
NativeSessionMessage18* construct_native_session_message18_0075e740(NativeSessionMessage18*,const NativeSessionMessageContext&);
bool native_session_message_is18_0075e7c0(std::uint32_t);
NativeSessionMessage18* delete_native_session_message18_0075e810(NativeSessionMessage18*,std::uint32_t);
void write_native_session_message18_0075e7d0(const NativeSessionMessage18*,NativeBitCursor*);
void read_native_session_message18_0075e7f0(NativeSessionMessage18*,NativeSessionReadStream*);
const std::uint32_t* native_session_message19_profile_00ce74dc();
NativeSessionMessage19* construct_native_session_message19_004b6050(NativeSessionMessage19*,const NativeSessionMessageContext&);
bool native_session_message_is19_004b6080(std::uint32_t);
NativeSessionMessage19* delete_native_session_message19_004b60f0(NativeSessionMessage19*,std::uint32_t);
void write_native_session_message19_004b6090(const NativeSessionMessage19*,NativeBitCursor*);
void read_native_session_message19_004b60c0(NativeSessionMessage19*,NativeSessionReadStream*);
const std::uint32_t* native_session_message20_profile_00d03174();
NativeSessionMessage20* construct_native_session_message20_0075e830(NativeSessionMessage20*,const NativeSessionMessageContext&);
bool native_session_message_is20_0075e8b0(std::uint32_t);
NativeSessionMessage20* delete_native_session_message20_0075e920(NativeSessionMessage20*,std::uint32_t);
void write_native_session_message20_0075e8c0(const NativeSessionMessage20*,NativeBitCursor*);
void read_native_session_message20_0075e8f0(NativeSessionMessage20*,NativeSessionReadStream*);
} // namespace bsp

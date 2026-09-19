#pragma once
#include "bsp/native_bit_cursor_write.hpp"

namespace bsp {
// The factory's base allocation is18h. Bytes11..13 are retained by constructors.
struct NativeSessionMessageStorage {
    const std::uint32_t* profile_00;
    std::uint32_t delivery_04;
    std::uint32_t field_08;
    std::uint32_t field_0c;
    std::uint8_t type_10;
    std::uint8_t reserved_11[3];
    void* selected_owner_14;
};
static_assert(sizeof(NativeSessionMessageStorage)==0x18);
static_assert(offsetof(NativeSessionMessageStorage,type_10)==0x10);
static_assert(offsetof(NativeSessionMessageStorage,selected_owner_14)==0x14);
// Read calls receive this18h wrapper. Write calls receive its10h cursor shape
// directly, with no virtual-profile prefix. No fabricated stream constructor.
struct NativeSessionReadStream {
    const void* profile_00;
    NativeBitCursor cursor_04;
    std::uint8_t owns_14;
    std::uint8_t reserved_15[3];
};
static_assert(sizeof(NativeSessionReadStream)==0x18);
struct NativeSessionMessageContext {void* volatile& current_game_00e188a8;};

// Actual translated tables: root has three slots; each concrete profile has
// five (scalar delete,write,read,type equality,always true). The virtual-entry
// adapters preserve ECX/stack placement using ignored-EDX fastcall bridges.
const std::uint32_t* native_session_message_root_profile_00ce4974();
const std::uint32_t* native_session_message_base_profile_00d02c68();
const std::uint32_t* native_session_message_one_profile_00ce4980();
const std::uint32_t* native_session_message_zero_profile_00d02ee8();
const std::uint32_t* native_session_message_two_profile_00d02efc();
NativeSessionMessageStorage* construct_native_session_message_0075b430(NativeSessionMessageStorage*,std::uint32_t type,const NativeSessionMessageContext&);
NativeSessionMessageStorage* construct_native_session_message_one_00449980(NativeSessionMessageStorage*,std::uint32_t type,const NativeSessionMessageContext&);
NativeSessionMessageStorage* construct_native_session_message_zero_0075b860(NativeSessionMessageStorage*,std::uint32_t type,const NativeSessionMessageContext&);
NativeSessionMessageStorage* construct_native_session_message_two_0075b8a0(NativeSessionMessageStorage*,std::uint32_t type,const NativeSessionMessageContext&);
// Scalars stamp the actual root profile, free only flags bit0, return identity.
NativeSessionMessageStorage* delete_native_session_message_root_00449910(NativeSessionMessageStorage*,std::uint32_t flags);
NativeSessionMessageStorage* delete_native_session_message_one_004499e0(NativeSessionMessageStorage*,std::uint32_t flags);
NativeSessionMessageStorage* delete_native_session_message_base_00759cd0(NativeSessionMessageStorage*,std::uint32_t flags);
NativeSessionMessageStorage* delete_native_session_message_zero_0075b880(NativeSessionMessageStorage*,std::uint32_t flags);
NativeSessionMessageStorage* delete_native_session_message_two_0075b8c0(NativeSessionMessageStorage*,std::uint32_t flags);
void write_native_session_message_type_00449940(const NativeSessionMessageStorage*,NativeBitCursor*);
void read_native_session_message_type_00449960(NativeSessionMessageStorage*,NativeSessionReadStream*);
bool native_session_message_type_equals_004499a0(const NativeSessionMessageStorage*,std::uint32_t type);
bool native_session_message_always_true_004499c0();
// Complete common extended-header bodies: type8, sender WORD low12, relay1.
// Caller supplies the real derived object with accessible bytes through+1Ah;
// these functions do not invent a constructor or bind its remaining methods.
void write_native_session_message_header_0075b480(const void*,NativeBitCursor*);
void read_native_session_message_header_0075b4c0(void*,NativeSessionReadStream*);
} // namespace bsp

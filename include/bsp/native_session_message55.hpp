#pragma once
#include "bsp/native_session_message_tags_56_to_62.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
struct NativeMessage55Node {NativeMessage55Node* next;NativeMessage55Node* previous;void* payload;};
struct NativeMessage55List {std::uint32_t retained_00;NativeMessage55Node* sentinel_04;std::uint32_t count_08;};
struct NativeMessage55Position {void* object_00;std::uint32_t position_04[3];};
struct NativeMessage55Receive {
    std::uint32_t length_00;char* data_04;
    std::uint8_t has_handle_08,retained_09;std::uint16_t handle_0a;
    void* resolved_0c;std::uint32_t position_10[3];
};
struct NativeMessage55Vector {std::uint32_t retained_00;NativeMessage55Receive* begin_04;NativeMessage55Receive* end_08;NativeMessage55Receive* capacity_0c;};
struct NativeMessage55Entry {
    const std::uint32_t* profile_00;std::uint32_t length_04;char* data_08;
    std::uint32_t length_0c;char* data_10;std::uint32_t retained_14;
    std::uint32_t value_18,value_1c;NativeMessage55List positions_20;
};
struct NativeSessionMessage55 {NativeSessionMessageStorage base;NativeMessage55List entries_18;NativeMessage55Vector received_24;};
static_assert(sizeof(NativeMessage55Node)==0x0c && sizeof(NativeMessage55List)==0x0c);
static_assert(sizeof(NativeMessage55Receive)==0x1c && sizeof(NativeMessage55Vector)==0x10);
static_assert(sizeof(NativeMessage55Entry)==0x2c && offsetof(NativeMessage55Entry,positions_20)==0x20);
static_assert(sizeof(NativeSessionMessage55)==0x34 && offsetof(NativeSessionMessage55,received_24)==0x24);

// Required library contracts, not reconstructed STL implementations. Receivers
// are the original raw storage. No default provider or fabricated success path.
struct NativeMessage55LibraryCalls {
    virtual ~NativeMessage55LibraryCalls()=default;
    virtual NativeMessage55Node* sentinel_008db560(NativeMessage55List*)=0;
    virtual NativeMessage55Node* sentinel_008db4f0(NativeMessage55List*)=0;
    virtual void resize_entries_008e0ec0(NativeMessage55List*,std::uint32_t count,void* value)=0;
    // Native by-value element has only its string header initialized by the
    // caller. The provider must not infer zero initialization of other fields.
    virtual void resize_received_008e17c0(NativeMessage55Vector*,std::uint32_t count,const NativeMessage55Receive& value)=0;
    // Native ECX=begin, EDX=end, two otherwise unused stack owner arguments;
    // RET8. Preserve that ownership boundary rather than substituting a layout.
    virtual void destroy_received_008de660(NativeMessage55Receive* begin,NativeMessage55Receive* end,NativeMessage55Vector*,NativeSessionMessage55*)=0;
    virtual void destroy_entries_008db580(NativeMessage55List*)=0;
    virtual void unwind_entries_008dbab0(NativeMessage55List*)=0;
};
struct NativeMessage55Context {
    const NativeSessionMessage56To62Context* const common;
    NativeMessage55LibraryCalls* const library;
    const SingletonLifetimeCallbacks* const invalid_parameters;
};
// One native payload slot at D16038; five message slots at D16124. Context is
// borrowed source-only storage beyond those slots; not a complete binary ABI.
struct NativeMessage55EntryProfile {const std::uint32_t slots[1];const NativeMessage55Context* const context;explicit NativeMessage55EntryProfile(const NativeMessage55Context&);};
struct NativeSessionMessage55Profile {const std::uint32_t slots[5];const NativeMessage55Context* const context;const NativeMessage55EntryProfile* const entry_profile;NativeSessionMessage55Profile(const NativeMessage55Context&,const NativeMessage55EntryProfile&);};
NativeMessage55Entry* construct_native_message55_entry_008e08f0(NativeMessage55Entry*,const NativeMessage55Context&,const NativeMessage55EntryProfile&);
void destroy_native_message55_entry_008dc100(NativeMessage55Entry*,const NativeMessage55Context&,const NativeMessage55EntryProfile&);
NativeMessage55Entry* delete_native_message55_entry_008dd440(NativeMessage55Entry*,std::uint32_t,const NativeMessage55Context&,const NativeMessage55EntryProfile&);
NativeSessionMessage55* construct_native_session_message55_008e0170(NativeSessionMessage55*,const NativeMessage55Context&,const NativeSessionMessage55Profile&);
bool native_session_message_is55_008e0320(const NativeSessionMessage55*,std::uint32_t);
bool resolve_native_message55_handles_008e0350(NativeSessionMessage55*,const NativeMessage55Context&);
void write_native_session_message55_008e0600(const NativeSessionMessage55*,NativeBitCursor*,const NativeMessage55Context&);
void read_native_session_message55_008e1930(NativeSessionMessage55*,NativeSessionReadStream*,const NativeMessage55Context&,const NativeMessage55EntryProfile&);
void destroy_native_session_message55_008e1480(NativeSessionMessage55*,const NativeMessage55Context&,const NativeSessionMessage55Profile&);
NativeSessionMessage55* delete_native_session_message55_008e1510(NativeSessionMessage55*,std::uint32_t,const NativeMessage55Context&,const NativeSessionMessage55Profile&);
struct NativeMessages55To62Profiles {
    const NativeSessionMessage55Profile* type55;const NativeSessionMessage56Profile* type56;
    const NativeSessionMessage57Profile* type57;const NativeSessionMessage58Profile* type58;
    const NativeSessionMessage59Profile* type59;const NativeSessionMessage60Profile* type60;
    const NativeSessionMessage61Profile* type61;const NativeSessionMessage62Profile* type62;
};
// Complete delegated creator control flow; requires the supplied container
// provider for type55. The full 00768530 stream factory remains unbound.
NativeSessionMessageStorage* create_native_session_messages55_to62_008e1530(std::uint32_t selector,const NativeMessages55To62Profiles&);
} // namespace bsp

#pragma once
#include "bsp/native_session_message_base.hpp"
#include "bsp/native_string.hpp"
#include <type_traits>
namespace bsp {
// Actual2Ch allocation: two native eight-byte pooled-string headers. No implicit
// string destruction or default initialization; +28 remains untouched by ctor.
struct NativeSessionMessage08 {
    NativeSessionMessageStorage base;
    std::uint32_t first_length_18;
    char* first_data_1c;
    std::uint32_t second_length_20;
    char* second_data_24;
    std::int32_t value_28;
};
static_assert(sizeof(NativeSessionMessage08)==0x2c);
static_assert(offsetof(NativeSessionMessage08,first_length_18)==0x18);
static_assert(offsetof(NativeSessionMessage08,second_length_20)==0x20);
static_assert(offsetof(NativeSessionMessage08,value_28)==0x28);
// Five ABI-visible slots followed by SOURCE-ONLY borrowed process bindings.
// Native D036D0 contains five slots; the supplied empty string is nativeF1AF25.
// Keep this profile, string context and empty-string backing alive for messages.
struct NativeSessionMessage08Profile {
    const std::uint32_t slots[5];
    NativeStringRawPoolContext* const strings;
    const char* const empty_00f1af25;
    NativeSessionMessage08Profile(NativeStringRawPoolContext&,const char* actual_empty_00f1af25);
};
static_assert(std::is_standard_layout_v<NativeSessionMessage08Profile>);
static_assert(offsetof(NativeSessionMessage08Profile,slots)==0);
NativeSessionMessage08* construct_native_session_message08_00765e10(NativeSessionMessage08*,const NativeSessionMessageContext&,const NativeSessionMessage08Profile&);
bool native_session_message_is08_00765e70(std::uint32_t);
void write_native_session_message08_00765e80(const NativeSessionMessage08*,NativeBitCursor*,const char* actual_empty_00f1af25);
void read_native_session_message08_00765f50(NativeSessionMessage08*,NativeSessionReadStream*,NativeStringRawPoolContext&);
// Repeats the real pool getter for every nonnull release. Headers remain
// untouched after release. Source cleanup order follows the recovered FH3 map;
// original private FH3/SEH binary compatibility remains a separate boundary.
void destroy_native_session_message08_00765ed0(NativeSessionMessage08*,NativeStringRawPoolContext&,const NativeSessionMessage08Profile&);
NativeSessionMessage08* delete_native_session_message08_00766040(NativeSessionMessage08*,std::uint32_t flags,NativeStringRawPoolContext&,const NativeSessionMessage08Profile&);
} // namespace bsp

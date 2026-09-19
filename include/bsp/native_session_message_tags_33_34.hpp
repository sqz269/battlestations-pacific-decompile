#pragma once
#include "bsp/native_session_message_base.hpp"
#include "bsp/native_string.hpp"
#include <type_traits>
namespace bsp {
struct NativeSessionMessage33 {
    NativeSessionMessageStorage base;
    std::uint32_t length_18;char* data_1c;
    std::uint32_t values_20[2];
    std::uint32_t length_28;char* data_2c;
    std::uint32_t length_30;char* data_34;
    std::uint32_t words_38[2];
};
struct NativeSessionMessage34 {NativeSessionMessageStorage base;std::uint32_t length_18;char* data_1c;};
static_assert(sizeof(NativeSessionMessage33)==0x40);
static_assert(offsetof(NativeSessionMessage33,length_28)==0x28);
static_assert(offsetof(NativeSessionMessage33,length_30)==0x30);
static_assert(offsetof(NativeSessionMessage33,words_38)==0x38);
static_assert(sizeof(NativeSessionMessage34)==0x20);
// Five native-visible slots. Trailing actual pool/fallback bindings are
// SOURCE-ONLY metadata, borrowed for the lifetime of the profile and object.
struct NativeSessionMessage33Profile {
    const std::uint32_t slots[5];NativeStringRawPoolContext* const strings;const char* const fallback_00e17669;
    NativeSessionMessage33Profile(NativeStringRawPoolContext&,const char* actual_fallback_00e17669);
};
struct NativeSessionMessage34Profile {
    const std::uint32_t slots[5];NativeStringRawPoolContext* const strings;const char* const fallback_00e17669;
    NativeSessionMessage34Profile(NativeStringRawPoolContext&,const char* actual_fallback_00e17669);
};
static_assert(std::is_standard_layout_v<NativeSessionMessage33Profile>);
static_assert(std::is_standard_layout_v<NativeSessionMessage34Profile>);
static_assert(offsetof(NativeSessionMessage33Profile,slots)==0);
static_assert(offsetof(NativeSessionMessage34Profile,slots)==0);
NativeSessionMessage33* construct_native_session_message33_00765bd0(NativeSessionMessage33*,const NativeSessionMessageContext&,const NativeSessionMessage33Profile&);
NativeSessionMessage34* construct_native_session_message34_00765d20(NativeSessionMessage34*,const NativeSessionMessageContext&,const NativeSessionMessage34Profile&);
void write_native_session_message33_0076cd60(const NativeSessionMessage33*,NativeBitCursor*,const char* actual_fallback_00e17669);
void read_native_session_message33_0076cdd0(NativeSessionMessage33*,NativeSessionReadStream*,NativeStringRawPoolContext&);
void write_native_session_message34_0076ced0(const NativeSessionMessage34*,NativeBitCursor*,const char* actual_fallback_00e17669);
void read_native_session_message34_0076cf00(NativeSessionMessage34*,NativeSessionReadStream*,NativeStringRawPoolContext&);
bool native_session_message_is33_00765c40(std::uint32_t);
bool native_session_message_is34_00765d80(std::uint32_t);
void destroy_native_session_message33_00765c50(NativeSessionMessage33*,NativeStringRawPoolContext&,const NativeSessionMessage33Profile&);
void destroy_native_session_message34_00765d90(NativeSessionMessage34*,NativeStringRawPoolContext&,const NativeSessionMessage34Profile&);
NativeSessionMessage33* delete_native_session_message33_00765d00(NativeSessionMessage33*,std::uint32_t,NativeStringRawPoolContext&,const NativeSessionMessage33Profile&);
NativeSessionMessage34* delete_native_session_message34_00765df0(NativeSessionMessage34*,std::uint32_t,NativeStringRawPoolContext&,const NativeSessionMessage34Profile&);
} // namespace bsp

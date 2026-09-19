#pragma once
#include "bsp/native_session_message_base.hpp"
#include "bsp/native_string.hpp"
#include <type_traits>
namespace bsp {
// Factory case 77 starts with type byte zero; Read replaces it from the wire.
struct NativeSessionMessage77 {NativeSessionMessageStorage base;std::uint32_t value_18;std::uint16_t value_1c;std::uint8_t retained_1e[2];};
struct NativeSessionMessage53 {NativeSessionMessageStorage base;std::uint32_t length_18;char* data_1c;};
static_assert(sizeof(NativeSessionMessage77)==0x20);
static_assert(offsetof(NativeSessionMessage77,value_1c)==0x1c);
static_assert(sizeof(NativeSessionMessage53)==0x20);
// Native CFE96C has five slots. Trailing borrowed bindings are source-only.
struct NativeSessionMessage53Profile {
    const std::uint32_t slots[5];NativeStringRawPoolContext* const strings;const char* const fallback_00e17669;
    NativeSessionMessage53Profile(NativeStringRawPoolContext&,const char* actual_fallback_00e17669);
};
static_assert(std::is_standard_layout_v<NativeSessionMessage53Profile>);
static_assert(offsetof(NativeSessionMessage53Profile,slots)==0);
const std::uint32_t* native_session_message77_profile_00d02cb8();
NativeSessionMessage77* construct_native_session_message77_00759f40(NativeSessionMessage77*);
bool native_session_message_is77_00759f70(const NativeSessionMessage77*,std::uint32_t);
void write_native_session_message77_00759f90(const NativeSessionMessage77*,NativeBitCursor*);
void read_native_session_message77_00759fd0(NativeSessionMessage77*,NativeSessionReadStream*);
NativeSessionMessage77* delete_native_session_message77_0075a010(NativeSessionMessage77*,std::uint32_t);
// Original no-argument creator returns a newly allocated 20h record in EAX.
// Explicit source context/profile bindings replace implicit process globals.
NativeSessionMessage53* create_native_session_message53_00733720(const NativeSessionMessageContext&,const NativeSessionMessage53Profile&);
bool native_session_message_is53_00733620(const NativeSessionMessage53*,std::uint32_t);
void write_native_session_message53_00733640(const NativeSessionMessage53*,NativeBitCursor*,const char* actual_fallback_00e17669);
void read_native_session_message53_00733670(NativeSessionMessage53*,NativeSessionReadStream*,NativeStringRawPoolContext&);
void destroy_native_session_message53_007336a0(NativeSessionMessage53*,NativeStringRawPoolContext&,const NativeSessionMessage53Profile&);
NativeSessionMessage53* delete_native_session_message53_00733700(NativeSessionMessage53*,std::uint32_t,NativeStringRawPoolContext&,const NativeSessionMessage53Profile&);
} // namespace bsp

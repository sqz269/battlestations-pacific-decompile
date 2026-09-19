#pragma once
#include "bsp/native_session_message_base.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_bit_cursor_numeric_array.hpp"
#include "bsp/object_handle_resolvers.hpp"
#include <type_traits>
namespace bsp {
// Borrow actual process publications, numeric constants and raw handle tables.
// No default state. Each binding must outlive profiles and records using it.
struct NativeSessionMessage56To62Context {
    const NativeSessionMessageContext* const session;
    NativeStringRawPoolContext* const strings;
    const char* const fallback_00e17669;
    const NativeBitNumericContext* const numeric;
    const volatile float* const maximum_00d7a248;
    const ObjectHandleTables* const handles;
};
// Native gate ignores ECX and reads current game +21A4 at each invocation.
bool native_session_message_process_flag_008dae20(const NativeSessionMessageContext&);
struct NativeSessionMessage56 {NativeSessionMessageStorage base;std::uint32_t length_18;char* data_1c;std::uint32_t length_20;char* data_24;std::uint32_t value_28;std::uint8_t flag_2c,retained_2d[3];};
static_assert(sizeof(NativeSessionMessage56)==0x30);
// Five visible native slots; trailing context is source-only metadata.
struct NativeSessionMessage56Profile {const std::uint32_t slots[5];const NativeSessionMessage56To62Context* const context;explicit NativeSessionMessage56Profile(const NativeSessionMessage56To62Context&);};
static_assert(std::is_standard_layout_v<NativeSessionMessage56Profile>);
static_assert(offsetof(NativeSessionMessage56Profile,slots)==0);
NativeSessionMessage56* construct_native_session_message56_008dc6a0(NativeSessionMessage56*,const NativeSessionMessageContext&,const NativeSessionMessage56Profile&);
bool native_session_message_is56_008dc6d0(const NativeSessionMessage56*,std::uint32_t);
void write_native_session_message56_008dc700(const NativeSessionMessage56*,NativeBitCursor*,const NativeSessionMessage56To62Context&);
void read_native_session_message56_008dc750(NativeSessionMessage56*,NativeSessionReadStream*,const NativeSessionMessage56To62Context&);
void destroy_native_session_message56_008dc7a0(NativeSessionMessage56*,NativeStringRawPoolContext&,const NativeSessionMessage56Profile&);
NativeSessionMessage56* delete_native_session_message56_008dc820(NativeSessionMessage56*,std::uint32_t,NativeStringRawPoolContext&,const NativeSessionMessage56Profile&);
struct NativeSessionMessage57 {NativeSessionMessageStorage base;std::uint32_t length_18;char* data_1c;std::uint16_t handle_20;std::uint8_t retained_22[2];};
static_assert(sizeof(NativeSessionMessage57)==0x24);
// Five visible native slots; trailing context is source-only metadata.
struct NativeSessionMessage57Profile {const std::uint32_t slots[5];const NativeSessionMessage56To62Context* const context;explicit NativeSessionMessage57Profile(const NativeSessionMessage56To62Context&);};
static_assert(std::is_standard_layout_v<NativeSessionMessage57Profile>);
static_assert(offsetof(NativeSessionMessage57Profile,slots)==0);
NativeSessionMessage57* construct_native_session_message57_008dc840(NativeSessionMessage57*,const NativeSessionMessageContext&,const NativeSessionMessage57Profile&);
bool native_session_message_is57_008dc870(const NativeSessionMessage57*,std::uint32_t);
void write_native_session_message57_008dc8a0(const NativeSessionMessage57*,NativeBitCursor*,const NativeSessionMessage56To62Context&);
void read_native_session_message57_008dc8e0(NativeSessionMessage57*,NativeSessionReadStream*,const NativeSessionMessage56To62Context&);
void destroy_native_session_message57_008dc980(NativeSessionMessage57*,NativeStringRawPoolContext&,const NativeSessionMessage57Profile&);
NativeSessionMessage57* delete_native_session_message57_008dc9e0(NativeSessionMessage57*,std::uint32_t,NativeStringRawPoolContext&,const NativeSessionMessage57Profile&);
bool native_session_message57_available_008dc920(const NativeSessionMessage57*,const NativeSessionMessage56To62Context&);
struct NativeSessionMessage58 {NativeSessionMessageStorage base;std::uint32_t length_18;char* data_1c;std::uint16_t handle_20;std::uint8_t retained_22[2];};
static_assert(sizeof(NativeSessionMessage58)==0x24);
// Five visible native slots; trailing context is source-only metadata.
struct NativeSessionMessage58Profile {const std::uint32_t slots[5];const NativeSessionMessage56To62Context* const context;explicit NativeSessionMessage58Profile(const NativeSessionMessage56To62Context&);};
static_assert(std::is_standard_layout_v<NativeSessionMessage58Profile>);
static_assert(offsetof(NativeSessionMessage58Profile,slots)==0);
NativeSessionMessage58* construct_native_session_message58_008dca00(NativeSessionMessage58*,const NativeSessionMessageContext&,const NativeSessionMessage58Profile&);
bool native_session_message_is58_008dca30(const NativeSessionMessage58*,std::uint32_t);
void write_native_session_message58_008dca60(const NativeSessionMessage58*,NativeBitCursor*,const NativeSessionMessage56To62Context&);
void read_native_session_message58_008dcaa0(NativeSessionMessage58*,NativeSessionReadStream*,const NativeSessionMessage56To62Context&);
void destroy_native_session_message58_008dcb40(NativeSessionMessage58*,NativeStringRawPoolContext&,const NativeSessionMessage58Profile&);
NativeSessionMessage58* delete_native_session_message58_008dcba0(NativeSessionMessage58*,std::uint32_t,NativeStringRawPoolContext&,const NativeSessionMessage58Profile&);
bool native_session_message58_available_008dcae0(const NativeSessionMessage58*,const NativeSessionMessage56To62Context&);
struct NativeSessionMessage59 {NativeSessionMessageStorage base;std::uint32_t length_18;char* data_1c;std::uint32_t float_bits_20[3];};
static_assert(sizeof(NativeSessionMessage59)==0x2c);
// Five visible native slots; trailing context is source-only metadata.
struct NativeSessionMessage59Profile {const std::uint32_t slots[5];const NativeSessionMessage56To62Context* const context;explicit NativeSessionMessage59Profile(const NativeSessionMessage56To62Context&);};
static_assert(std::is_standard_layout_v<NativeSessionMessage59Profile>);
static_assert(offsetof(NativeSessionMessage59Profile,slots)==0);
NativeSessionMessage59* construct_native_session_message59_008dcbc0(NativeSessionMessage59*,const NativeSessionMessageContext&,const NativeSessionMessage59Profile&);
bool native_session_message_is59_008dcbf0(const NativeSessionMessage59*,std::uint32_t);
void write_native_session_message59_008dcc20(const NativeSessionMessage59*,NativeBitCursor*,const NativeSessionMessage56To62Context&);
void read_native_session_message59_008dcc70(NativeSessionMessage59*,NativeSessionReadStream*,const NativeSessionMessage56To62Context&);
void destroy_native_session_message59_008dccc0(NativeSessionMessage59*,NativeStringRawPoolContext&,const NativeSessionMessage59Profile&);
NativeSessionMessage59* delete_native_session_message59_008dcd20(NativeSessionMessage59*,std::uint32_t,NativeStringRawPoolContext&,const NativeSessionMessage59Profile&);
struct NativeSessionMessage60 {NativeSessionMessageStorage base;std::uint32_t length_18;char* data_1c;std::uint32_t float_bits_20[3];};
static_assert(sizeof(NativeSessionMessage60)==0x2c);
// Five visible native slots; trailing context is source-only metadata.
struct NativeSessionMessage60Profile {const std::uint32_t slots[5];const NativeSessionMessage56To62Context* const context;explicit NativeSessionMessage60Profile(const NativeSessionMessage56To62Context&);};
static_assert(std::is_standard_layout_v<NativeSessionMessage60Profile>);
static_assert(offsetof(NativeSessionMessage60Profile,slots)==0);
NativeSessionMessage60* construct_native_session_message60_008dcd40(NativeSessionMessage60*,const NativeSessionMessageContext&,const NativeSessionMessage60Profile&);
bool native_session_message_is60_008dcd70(const NativeSessionMessage60*,std::uint32_t);
void write_native_session_message60_008dcda0(const NativeSessionMessage60*,NativeBitCursor*,const NativeSessionMessage56To62Context&);
void read_native_session_message60_008dcdf0(NativeSessionMessage60*,NativeSessionReadStream*,const NativeSessionMessage56To62Context&);
void destroy_native_session_message60_008dce40(NativeSessionMessage60*,NativeStringRawPoolContext&,const NativeSessionMessage60Profile&);
NativeSessionMessage60* delete_native_session_message60_008dcea0(NativeSessionMessage60*,std::uint32_t,NativeStringRawPoolContext&,const NativeSessionMessage60Profile&);
struct NativeSessionMessage61 {NativeSessionMessageStorage base;std::uint32_t length_18;char* data_1c;std::uint32_t length_20;char* data_24;std::uint8_t flag_28,retained_29[3];};
static_assert(sizeof(NativeSessionMessage61)==0x2c);
// Five visible native slots; trailing context is source-only metadata.
struct NativeSessionMessage61Profile {const std::uint32_t slots[5];const NativeSessionMessage56To62Context* const context;explicit NativeSessionMessage61Profile(const NativeSessionMessage56To62Context&);};
static_assert(std::is_standard_layout_v<NativeSessionMessage61Profile>);
static_assert(offsetof(NativeSessionMessage61Profile,slots)==0);
NativeSessionMessage61* construct_native_session_message61_008dcec0(NativeSessionMessage61*,const NativeSessionMessageContext&,const NativeSessionMessage61Profile&);
bool native_session_message_is61_008dcef0(const NativeSessionMessage61*,std::uint32_t);
void write_native_session_message61_008dcf20(const NativeSessionMessage61*,NativeBitCursor*,const NativeSessionMessage56To62Context&);
void read_native_session_message61_008dcf60(NativeSessionMessage61*,NativeSessionReadStream*,const NativeSessionMessage56To62Context&);
void destroy_native_session_message61_008dcfa0(NativeSessionMessage61*,NativeStringRawPoolContext&,const NativeSessionMessage61Profile&);
NativeSessionMessage61* delete_native_session_message61_008dd020(NativeSessionMessage61*,std::uint32_t,NativeStringRawPoolContext&,const NativeSessionMessage61Profile&);
struct NativeSessionMessage62 {NativeSessionMessageStorage base;std::uint32_t length_18;char* data_1c;std::uint32_t length_20;char* data_24;std::uint8_t flag_28,retained_29[3];};
static_assert(sizeof(NativeSessionMessage62)==0x2c);
// Five visible native slots; trailing context is source-only metadata.
struct NativeSessionMessage62Profile {const std::uint32_t slots[5];const NativeSessionMessage56To62Context* const context;explicit NativeSessionMessage62Profile(const NativeSessionMessage56To62Context&);};
static_assert(std::is_standard_layout_v<NativeSessionMessage62Profile>);
static_assert(offsetof(NativeSessionMessage62Profile,slots)==0);
NativeSessionMessage62* construct_native_session_message62_008dd040(NativeSessionMessage62*,const NativeSessionMessageContext&,const NativeSessionMessage62Profile&);
bool native_session_message_is62_008dd070(const NativeSessionMessage62*,std::uint32_t);
void write_native_session_message62_008dd0a0(const NativeSessionMessage62*,NativeBitCursor*,const NativeSessionMessage56To62Context&);
void read_native_session_message62_008dd0e0(NativeSessionMessage62*,NativeSessionReadStream*,const NativeSessionMessage56To62Context&);
void destroy_native_session_message62_008dd120(NativeSessionMessage62*,NativeStringRawPoolContext&,const NativeSessionMessage62Profile&);
NativeSessionMessage62* delete_native_session_message62_008dd1a0(NativeSessionMessage62*,std::uint32_t,NativeStringRawPoolContext&,const NativeSessionMessage62Profile&);
} // namespace bsp

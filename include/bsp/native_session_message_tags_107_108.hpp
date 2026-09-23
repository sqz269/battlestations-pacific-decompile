#pragma once

#include "bsp/native_session_message_tags_99_to102.hpp"

namespace bsp {

// Complete normal bodies for 107/108. Descriptive source names are hypotheses.
// These profiles borrow the existing scalar context; five native-shaped slots
// precede a source-only context pointer. No drop-in binary ABI is claimed.
using NativeSessionMessage107 = NativeSessionMessage98ScalarStorage;
using NativeSessionMessage108 = NativeSessionMessage98ScalarStorage;
static_assert(sizeof(NativeSessionMessage107) == 0x24);
static_assert(sizeof(NativeSessionMessage108) == 0x24);

#define BSP_MESSAGE_PROFILE_107_108(N,C,P,D,S) \
struct NativeSessionMessage##N##Profile : NativeSessionMessage99To102ProfileStorage { \
    explicit NativeSessionMessage##N##Profile(const NativeSessionMessage99To102Context&); \
}; \
static_assert(std::is_standard_layout_v<NativeSessionMessage##N##Profile>); \
NativeSessionMessage##N* construct_native_session_message##N##_##C(NativeSessionMessage##N*, const NativeSessionMessage99To102Context&, const NativeSessionMessage##N##Profile&); \
bool native_session_message_is##N##_##P(std::uint32_t); \
void destroy_native_session_message##N##_##D(NativeSessionMessage##N*); \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N*, std::uint32_t);

BSP_MESSAGE_PROFILE_107_108(107,007618e0,00761960,00761950,00761990)
BSP_MESSAGE_PROFILE_107_108(108,007619b0,00761a30,00761a20,00761a60)
#undef BSP_MESSAGE_PROFILE_107_108

} // namespace bsp

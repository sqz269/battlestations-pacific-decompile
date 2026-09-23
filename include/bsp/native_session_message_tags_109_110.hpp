#pragma once
#include "bsp/native_session_message_tags_99_to102.hpp"

namespace bsp {
// Complete normal bodies; source profiles borrow the existing scalar context.
// These names are descriptive hypotheses, not recovered native symbols.
using NativeSessionMessage109 = NativeSessionMessage98ScalarStorage;
using NativeSessionMessage110 = NativeSessionMessage98ScalarStorage;

#define BSP_MESSAGE_PROFILE(N,C,P,D,S) \
struct NativeSessionMessage##N##Profile : NativeSessionMessage99To102ProfileStorage { \
    explicit NativeSessionMessage##N##Profile(const NativeSessionMessage99To102Context&); \
}; \
static_assert(std::is_standard_layout_v<NativeSessionMessage##N##Profile>); \
NativeSessionMessage##N* construct_native_session_message##N##_##C(NativeSessionMessage##N*, const NativeSessionMessage99To102Context&, const NativeSessionMessage##N##Profile&); \
bool native_session_message_is##N##_##P(std::uint32_t); \
void destroy_native_session_message##N##_##D(NativeSessionMessage##N*); \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N*, std::uint32_t);
BSP_MESSAGE_PROFILE(109,00761a80,00761b00,00761af0,00761b30)
BSP_MESSAGE_PROFILE(110,00761b50,00761bd0,00761bc0,00761c00)
#undef BSP_MESSAGE_PROFILE
} // namespace bsp

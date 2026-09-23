#include "bsp/native_session_message_tags_109_110.hpp"

namespace bsp {
namespace {
using U = std::uint32_t;
using Context = NativeSessionMessage99To102Context;
using Storage = NativeSessionMessage98ScalarStorage;

U default_bits(const volatile float* source) {
    U result;
    __asm {
        mov eax, source
        movss xmm0, dword ptr [eax]
        movss dword ptr result, xmm0
    }
    return result;
}

void initialize(Storage* message, U type, const Context& context, const U* profile) {
    // The native constructors inline this exact base sequence; no native CALL.
    construct_native_session_message_0075b430(&message->header.base, type, *context.session);
    const U value = default_bits(context.default_00cf8b3c);
    volatile auto& m = *message;
    m.header.sender_18 = 0;
    m.header.relay_1a = 0;
    m.header.flag_1c = 0;
    m.header.base.delivery_04 = 1;
    m.value_20 = value;
    m.header.base.profile_00 = profile;
}

// Keep the actual shared adapter addresses, including their context convention.
U scalar_slot(const Context& context, U index) {
    return NativeSessionMessage100Profile(context).slots[index];
}

#define BSP_ADAPTERS(N,P,S) \
NativeSessionMessage##N* __fastcall delete##N(NativeSessionMessage##N* message, void*, U flags) { \
    return delete_native_session_message##N##_##S(message, flags); \
} \
bool __fastcall is##N(const void*, void*, U type) { \
    return native_session_message_is##N##_##P(type); \
}
BSP_ADAPTERS(109,00761b00,00761b30)
BSP_ADAPTERS(110,00761bd0,00761c00)
#undef BSP_ADAPTERS
} // namespace

#define BSP_IMPLEMENT(N,C,P,D,S) \
NativeSessionMessage##N##Profile::NativeSessionMessage##N##Profile(const Context& context) \
    : NativeSessionMessage99To102ProfileStorage(context, reinterpret_cast<U>(&delete##N), \
        scalar_slot(context, 1), scalar_slot(context, 2), reinterpret_cast<U>(&is##N)) {} \
NativeSessionMessage##N* construct_native_session_message##N##_##C(NativeSessionMessage##N* message, const Context& context, const NativeSessionMessage##N##Profile& profile) { \
    initialize(message, N, context, profile.slots); \
    return message; \
} \
bool native_session_message_is##N##_##P(U type) { \
    return type == N || type == 98 || type == 73 || type == 70; \
} \
void destroy_native_session_message##N##_##D(NativeSessionMessage##N* message) { \
    static_cast<volatile Storage&>(*message).header.base.profile_00 = native_session_message_root_profile_00ce4974(); \
} \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N* message, U flags) { \
    destroy_native_session_message##N##_##D(message); \
    if (flags & 1) singleton_lifetime_free(message); \
    return message; \
}
BSP_IMPLEMENT(109,00761a80,00761b00,00761af0,00761b30)
BSP_IMPLEMENT(110,00761b50,00761bd0,00761bc0,00761c00)
#undef BSP_IMPLEMENT
} // namespace bsp

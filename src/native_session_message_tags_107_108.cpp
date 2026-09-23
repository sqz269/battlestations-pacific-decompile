#include "bsp/native_session_message_tags_107_108.hpp"

namespace bsp {
namespace {
using U = std::uint32_t;
using Context = NativeSessionMessage99To102Context;

U load_default_bits(const volatile float* source) {
    U result;
    __asm {
        mov eax, source
        movss xmm0, dword ptr [eax]
        movss dword ptr result, xmm0
    }
    return result;
}

// Use the existing adapter addresses as well as their codec implementation.
U scalar_slot(const Context& context, U index) {
    return NativeSessionMessage100Profile(context).slots[index];
}

#define BSP_MESSAGE_ADAPTERS(N,P,S) \
NativeSessionMessage##N* __fastcall delete##N(NativeSessionMessage##N* message, void*, U flags) { \
    return delete_native_session_message##N##_##S(message, flags); \
} \
bool __fastcall is##N(const void*, void*, U type) { \
    return native_session_message_is##N##_##P(type); \
}
BSP_MESSAGE_ADAPTERS(107,00761960,00761990)
BSP_MESSAGE_ADAPTERS(108,00761a30,00761a60)
#undef BSP_MESSAGE_ADAPTERS
} // namespace

#define BSP_MESSAGE_IMPLEMENTATION(N,C,P,D,S) \
NativeSessionMessage##N##Profile::NativeSessionMessage##N##Profile(const Context& context) \
    : NativeSessionMessage99To102ProfileStorage(context, reinterpret_cast<U>(&delete##N), \
          scalar_slot(context, 1), scalar_slot(context, 2), reinterpret_cast<U>(&is##N)) {} \
NativeSessionMessage##N* construct_native_session_message##N##_##C( \
    NativeSessionMessage##N* message, const Context& context, const NativeSessionMessage##N##Profile& profile) { \
    /* Native constructors inline this base sequence; no native CALL is implied. */ \
    construct_native_session_message_0075b430(&message->header.base, N, *context.session); \
    const U value = load_default_bits(context.default_00cf8b3c); \
    volatile auto& storage = *message; \
    storage.header.sender_18 = 0; \
    storage.header.relay_1a = 0; \
    storage.header.flag_1c = 0; \
    storage.header.base.delivery_04 = 1; \
    storage.value_20 = value; \
    storage.header.base.profile_00 = profile.slots; \
    return message; \
} \
bool native_session_message_is##N##_##P(U type) { \
    return type == N || type == 98 || type == 73 || type == 70; \
} \
void destroy_native_session_message##N##_##D(NativeSessionMessage##N* message) { \
    static_cast<volatile NativeSessionMessage##N&>(*message).header.base.profile_00 = \
        native_session_message_root_profile_00ce4974(); \
} \
NativeSessionMessage##N* delete_native_session_message##N##_##S(NativeSessionMessage##N* message, U flags) { \
    destroy_native_session_message##N##_##D(message); \
    if ((flags & 1u) != 0) singleton_lifetime_free(message); \
    return message; \
}
BSP_MESSAGE_IMPLEMENTATION(107,007618e0,00761960,00761950,00761990)
BSP_MESSAGE_IMPLEMENTATION(108,007619b0,00761a30,00761a20,00761a60)
#undef BSP_MESSAGE_IMPLEMENTATION

} // namespace bsp

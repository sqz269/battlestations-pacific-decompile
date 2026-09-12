#include "bsp/native_input_action_listener_owner.hpp"
#include "bsp/singleton_lifetime.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <initializer_list>
#include <stdexcept>

namespace bsp {
namespace {
template<class T> volatile T& field(void* object, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile T*>(static_cast<std::byte*>(object) + offset);
}
constexpr std::uint32_t listener_profile = 0x00d5b610;
void require_listener_profile(std::uint32_t profile) {
    if (profile != listener_profile)
        throw std::invalid_argument("unbound raw input action listener profile");
}
} // namespace

void replace_native_input_action_listener_00a92b70(void* action,
    NativeInputActionRecordCalls& calls) {
    void* const replacement = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x24, 0x24});
    if (replacement) {
        field<std::uint32_t>(replacement, 0) = 0x00ceb130;
        field<LONG>(replacement, 4) = 1;
        field<std::uint32_t>(replacement, 0) = listener_profile;
        // Preserve the native byte-store order; all24h bytes become defined.
        for (const std::size_t offset : {8u, 10u, 11u, 13u, 14u, 15u, 16u,
                                         9u, 12u, 17u, 18u, 19u})
            field<std::uint8_t>(replacement, offset) = 0;
        field<float>(replacement, 0x14) = 0.0f;
        field<float>(replacement, 0x18) = 0.0f;
        field<float>(replacement, 0x1c) = 0.0f;
        field<float>(replacement, 0x20) = 0.0f;
    }
    void* const old_listener = field<void*>(action, 0x2c);
    if (old_listener && InterlockedDecrement(&field<LONG>(old_listener, 4)) == 0)
        calls.call_listener_slot0(old_listener, field<std::uint32_t>(old_listener, 0));
    field<void*>(action, 0x2c) = replacement;
}

void* scalar_delete_native_input_action_listener_00a92150(void* listener,
    std::uint32_t flags) noexcept {
    field<std::uint32_t>(listener, 0) = listener_profile;
    destroy_native_ref_counted_base_00bd30f0(listener);
    if ((flags & 1u) != 0) singleton_lifetime_free(listener);
    return listener;
}

void NativeInputActionListenerCalls::call_listener_slot0(void* listener,
    std::uint32_t profile) {
    require_listener_profile(profile);
    invoke_native_ref_counted_delete_00bd30e0(listener, *this);
}
void NativeInputActionListenerCalls::delete_vslot04(void* listener,
    std::uint32_t profile, std::uint32_t flags) {
    require_listener_profile(profile);
    scalar_delete_native_input_action_listener_00a92150(listener, flags);
}
} // namespace bsp

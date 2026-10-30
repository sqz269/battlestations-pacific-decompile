#include "bsp/native_menu_prompt_lifetime.hpp"

#include "bsp/frontend_seg15_22_cc13.hpp"

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
} // namespace

__declspec(noinline) void* delete_native_menu_prompt_screen_00532a00(
    void* actual_receiver, volatile std::uint32_t& actual_flags_dword_home,
    frontend_seg15_22_cc13::Seg15Host& actual_host) {
    const std::uintptr_t captured_allocation_identity =
        reinterpret_cast<std::uintptr_t>(actual_receiver);
    frontend_seg15_22_cc13::menu_prompt_destroy_005321f0(
        frontend_seg15_22_cc13::Raw{
            reinterpret_cast<std::uint8_t*>(captured_allocation_identity)}, actual_host);

    // Native532A08 tests BYTE[ESP+8] after the whole destructor returns.
    // A byte access also preserves aliases/mutations of the actual DWORD home.
    const unsigned char current_low_byte =
        *reinterpret_cast<const volatile unsigned char*>(&actual_flags_dword_home);
    if ((current_low_byte & 1u) != 0)
        actual_host.free_00bf65ac(reinterpret_cast<void*>(captured_allocation_identity));
    return reinterpret_cast<void*>(captured_allocation_identity);
}

__declspec(noinline) void* delete_native_menu_prompt_secondary_00532340(
    void* actual_secondary_receiver, volatile std::uint32_t& actual_flags_dword_home,
    frontend_seg15_22_cc13::Seg15Host& actual_host) {
    const auto captured_secondary_identity = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(actual_secondary_receiver));
    const std::uint32_t allocation_identity = captured_secondary_identity - 0x0cu;
    return delete_native_menu_prompt_screen_00532a00(
        reinterpret_cast<void*>(static_cast<std::uintptr_t>(allocation_identity)),
        actual_flags_dword_home, actual_host);
}
} // namespace bsp

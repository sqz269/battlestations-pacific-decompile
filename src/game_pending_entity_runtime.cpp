#include "bsp/game_pending_entity_runtime.hpp"

#include <cstdlib>
#include <exception>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Pending entity process ownership requires MSVC Win32.
#endif

namespace bsp::game {
namespace {
// Native F899A8[18h] is zero-filled static storage. This trivial object has
// process storage duration and no implicit C++ construction/destruction work.
static_assert(std::is_trivial_v<NativePendingEntityOwners>);
NativePendingEntityOwners process_owners;

void pending_destroy_shutdown_00cdf4a0() noexcept {
    destroy_native_pending_destroy_owner_00cdf4a0(process_owners);
}
void pending_kill_shutdown_00cdf4b0() noexcept {
    destroy_native_pending_kill_owner_00cdf4b0(process_owners);
}

int register_pending_shutdown(void*, std::uint32_t native_shutdown) noexcept {
    switch (native_shutdown) {
    case 0x00cdf4a0u: return std::atexit(&pending_destroy_shutdown_00cdf4a0);
    case 0x00cdf4b0u: return std::atexit(&pending_kill_shutdown_00cdf4b0);
    default: std::terminate(); // impossible for the two fixed R wrappers
    }
}
constexpr NativePendingEntityCrtRegistration actual_crt{
    nullptr, &register_pending_shutdown};
} // namespace

NativePendingEntityOwners& game_pending_entity_owners() noexcept {
    return process_owners;
}

int initialize_game_pending_destroy_owner_00cd3910() {
    return initialize_native_pending_destroy_owner_00cd3910(process_owners, actual_crt);
}
int initialize_game_pending_kill_owner_00cd3940() {
    return initialize_native_pending_kill_owner_00cd3940(process_owners, actual_crt);
}
} // namespace bsp::game

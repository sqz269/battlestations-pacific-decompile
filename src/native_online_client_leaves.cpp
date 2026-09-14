#include "bsp/native_online_client_leaves.hpp"
#include <Windows.h>
#include <stdexcept>

namespace bsp {
static_assert(sizeof(void*) == 4);
namespace {
using U32 = std::uint32_t;
U32 address(const void* p) { return reinterpret_cast<U32>(p); }
U32 load32(U32 p) { return *reinterpret_cast<const volatile U32*>(p); }
std::uint8_t load8(U32 p) {
    return *reinterpret_cast<const volatile std::uint8_t*>(p);
}
}
NativeOnlineClientSdk resolve_native_online_client_sdk(void* module) {
    if (!module) throw std::invalid_argument("Native client SDK requires a loaded module");
    const auto entry = GetProcAddress(static_cast<HMODULE>(module), MAKEINTRESOURCEA(5314));
    if (!entry) throw std::runtime_error("Missing XLive ordinal5314");
    return {reinterpret_cast<NativeOnlineClientSdk::Query5314>(entry)};
}
std::uint8_t native_online_selected_byte_00a3e510(const NativeOnlineManagerStorage& manager) {
    return load8(address(&manager) + 0x119u);
}
std::uint32_t native_online_active_user_00a3eac0(const NativeOnlineManagerStorage& manager) {
    return load32(address(&manager) + 0x11cu);
}
std::uint8_t query_native_online_client_00a430e0(void* client, U32 low, U32 high,
    const NativeOnlineClientPublications& publications, const NativeOnlineClientSdk& sdk) {
    U32 result = address(client); // PUSH ECX is the original local preimage.
    static_cast<void>(native_online_active_user_00a3eac0(*publications.manager_00f8abe8));
    const auto user = native_online_active_user_00a3eac0(*publications.manager_00f8abe8);
    if (user > 1u) return 1;
    const auto current_user = native_online_active_user_00a3eac0(*publications.manager_00f8abe8);
    static_cast<void>(sdk.query(current_user, low, high, &result));
    return static_cast<std::uint8_t>(result != 0);
}
void refresh_native_online_client_records_00a43180(void* client,
    const NativeOnlineClientPublications& publications, NativeOnlineClientApply771630 apply) {
    if (!load32(address(publications.game_00e188a8) + 0x1fe4u)) return;
    if (!native_online_selected_byte_00a3e510(*publications.manager_00f8abe8)) return;
    U32 result_word = address(client); // Only AL overwrites this retained word.
    for (U32 offset = 0; offset < 0x8c0u; offset += 0x118u) {
        const U32 record = address(publications.game_00e188a8) + offset + 0x748u;
        if (!record || load8(record + 9u) || !load8(record + 8u)) continue;
        const auto high = load32(record + 0x84u);
        const auto low = load32(record + 0x80u);
        using Virtual24 = std::uint8_t (__thiscall*)(void*, U32, U32);
        const auto target = reinterpret_cast<Virtual24>(load32(load32(address(client)) + 0x24u));
        const auto result = target(client, low, high);
        result_word = (result_word & 0xffffff00u) | result;
        auto* const session = reinterpret_cast<void*>(address(publications.game_00e188a8) + 0x1ef0u);
        apply(session, low, high, result_word);
    }
}
} // namespace bsp

#pragma once
#include "bsp/native_online_notifications.hpp"

namespace bsp {
// Actual mutable publication slots, not projected game/client owners. The raw
// game must contain the read fields through +1FE7 and eight118h player records.
// A client is borrowed only for its current vtable; its2218h construction is not
// supplied here. All referenced storage must remain alive across each call.
struct NativeOnlineClientPublications final {
    void* volatile& game_00e188a8;
    NativeOnlineManagerStorage* volatile& manager_00f8abe8;
};

struct NativeOnlineClientSdk final {
    // A4D5E4 -> CE2714 -> xlive ordinal5314. Four DWORD stack words, RET16.
    // Identifier low/high words are retained separately; status is ignored by
    // A430E0. The output starts with the incoming CLIENT POINTER, not zero.
    using Query5314 = std::uint32_t (__stdcall*)(std::uint32_t user,
        std::uint32_t identifier_low, std::uint32_t identifier_high,
        std::uint32_t* result);
    Query5314 query;
};
// Borrows an already loaded module, resolves ordinal5314, and performs no SDK
// operation. Missing import throws. No symbolic SDK name is claimed.
NativeOnlineClientSdk resolve_native_online_client_sdk(void* loaded_module);

// Exact small getters, native ECX=raw3F0h manager, RET; AL and EAX respectively.
// The selected byte is not normalized. Loads remain observable even when the
// caller discards a result (A430E0 calls the DWORD getter three times).
std::uint8_t native_online_selected_byte_00a3e510(const NativeOnlineManagerStorage&);
std::uint32_t native_online_active_user_00a3eac0(const NativeOnlineManagerStorage&);

// Explicit unresolved callee: 771630..771796, ECX=current game+1EF0, RET12.
// It includes privilege/friendship checks, session messages and voice mutations.
// Supply its genuine body; there is no successful default or substitute here.
// The full third DWORD matters: high24 bits retain the incoming client pointer,
// while its low byte is the unnormalized AL from the current client virtual+24.
using NativeOnlineClientApply771630 = void (__thiscall*)(void* session,
    std::uint32_t identifier_low, std::uint32_t identifier_high,
    std::uint32_t result_word);

// Complete A43180..A43213; ECX=captured client, RET. Reads the CURRENT game each
// iteration and after virtual+24; uses the captured client's CURRENT vtable.
// Calls the required raw771630 boundary for each eligible player record.
void refresh_native_online_client_records_00a43180(void* client,
    const NativeOnlineClientPublications&, NativeOnlineClientApply771630);

// Complete A430E0..A4312B; ECX=client, two stack DWORDs, RET8; defined result AL.
// Current manager loads and all three concrete getter reads follow the native
// order. An unsigned user greater than1 returns1 without entering the SDK.
std::uint8_t query_native_online_client_00a430e0(void* client,
    std::uint32_t identifier_low, std::uint32_t identifier_high,
    const NativeOnlineClientPublications&, const NativeOnlineClientSdk&);
} // namespace bsp

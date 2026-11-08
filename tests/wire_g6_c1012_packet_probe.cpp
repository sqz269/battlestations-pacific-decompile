// Focused Win32 source fixture for native packet ownership and target admission.
// Link the game_hosts_session_messages object and bsp_core.lib with
// /MANIFEST:EMBED; this does not execute the game.
#include "bsp/game_native_string_process.hpp"
#include "bsp/game_hosts_powerups.hpp"
#include "bsp/game_hosts_session_messages.hpp"
#include "bsp/multiplayer_session_cc13.hpp"
#include "bsp/session_participant_pools.hpp"

#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace {
void require(bool condition) {
    if (!condition) throw std::runtime_error("c1012 native packet ownership/admission");
}
class SelectedOwner final : public bsp::NativeSessionMessageOwnerView {
public:
    void* selected_owner_14() const noexcept override {
        return const_cast<SelectedOwner*>(this); // fixture identity, never dereferenced
    }
};
}

int main() {
    auto& process = bsp::game::game_native_string_process();
    auto& raw = process.raw_context();
    SelectedOwner owner;
    void* volatile unused_game = nullptr; // the typed owner bypasses raw game reads
    const bsp::NativeSessionMessageContext session{unused_game, &owner};
    const char empty = '\0';
    const bsp::NativeSessionMessage09Profile profile(raw, &empty);
    const bsp::mp_cc13::SceneIndexBuildContext context{session, process.strings(), profile};
    // Embedded NUL distinguishes an exact native-length copy from a C-string copy.
    const char name[]{'M', '\0', 'X', 'Y'};
    const bsp::mp_cc13::SceneIndexNameView header{sizeof(name), name};
    bsp::NativeSessionMessage09 message;
    std::memset(&message, 0xa5, sizeof(message));
    bsp::mp_cc13::build_scene_index_00771190(message, 173, &header,
        0x89abcdefu, 0x76543210u, context);
    require(message.base.profile_00 == profile.slots && message.base.type_10 == 9);
    require(message.base.delivery_04 == 1 && message.base.field_08 == 0
        && message.base.field_0c == 0 && message.base.selected_owner_14 == &owner);
    require(message.length_18 == sizeof(name) && std::memcmp(message.data_1c, name, sizeof(name)) == 0
        && message.data_1c[sizeof(name)] == '\0');
    require(message.value_20 == 173 && message.words_28[0] == 0x89abcdefu
        && message.words_28[1] == 0x76543210u);
    require(message.retained_24 == 0xa5a5a5a5u && message.base.reserved_11[0] == 0xa5
        && message.base.reserved_11[1] == 0xa5 && message.base.reserved_11[2] == 0xa5);
    char* const owned = message.data_1c;
    bsp::destroy_native_session_message09_007662a0(&message, raw, profile);
    require(message.base.profile_00 == bsp::native_session_message_root_profile_00ce4974());
    require(message.length_18 == sizeof(name) && message.data_1c == owned); // native header retained
    // An immediate equal-size allocation must reuse the returned native block.
    bsp::NativeString reused;
    reused.resize_0041dd40(process.strings(), sizeof(name), true);
    require(reused.data() == owned);
    reused.release_to(process.strings());

    // Both send operations must return false for either unsupported target
    // field. PowerupManagerBinding then keeps its existing observation fallback.
    bsp::HostSessionMessages messages;
    bsp::SessionParticipantPools participants{bsp::ParticipantPoolInitialization::ZeroedGameAllocation};
    const std::size_t local_slot = 0;
    bsp::game_powerups::PowerupModifier modifier;
    const std::int32_t operations[]{0, 2};
    for (const auto operation : operations) {
        modifier.unit_id = 1;
        modifier.centre_id = 0;
        require(!messages.broadcast_powerup_modifier(modifier, operation, participants, local_slot));
        modifier.unit_id = 0;
        modifier.centre_id = 1;
        require(!messages.broadcast_powerup_modifier(modifier, operation, participants, local_slot));
    }
    modifier.centre_id = 0;
    require(messages.broadcast_powerup_modifier(modifier, 2, participants, local_slot));
    std::puts("c1012: native payload/pool return and insertion/expiration target rejection passed");
}

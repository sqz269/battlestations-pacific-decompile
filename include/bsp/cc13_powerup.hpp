#pragma once

// Packet cc13-powerup (cc9 lead's order 2): the power-up grant natives AddPowerup
// 008EE410, PreparePowerup 008EA610 and GetAvailablePowerups 008EB350 bound to a
// host power-up manager. Evidence, reach and the ON-only lookups are in
// docs/CC13_POWERUP.md. Names are hypotheses, not recovered symbols.

#include "bsp/game_hosts_powerups.hpp"
#include "bsp/powerup_config.hpp"
#include "bsp/world_entities.hpp"

#include <array>
#include <cstdint>
#include <list>
#include <map>
#include <random>
#include <string>
#include <string_view>
#include <vector>

struct lua_State;

namespace bsp::cc13_powerup {

// True: the global_subsystems load step fills the class registry from
// Scripts/datatables/PowerupClasses.lua (the walk of 008ECEC0, in a Lua state of
// its own), and the three natives run their image bodies against it: 008EE410
// grants through 008EDDC0 (the slot's inventory list, the pup_gain warning
// 009789A0, the PUM1STGET hint), 008EA610 loads and discards through 008E9FD0,
// and 008EB350 returns 008EAB00's table of ready class names, whose 008EA0C0
// walk inserts the zeroed +14Ch records that 008EB110 then reports as pup_ready.
// Nothing reads the inventory for a use in a single-player mission-tree run: the
// party brain 00A179E0 has no coordinator (kAiCoordinatorLoadGateBound) and the
// player's HUD use 008EB6E0 needs input. So no category list fills and every
// 008E6430 / 00470440 product stays 1.0f either way. False: the three natives
// stay unimplemented records returning nothing, and no registry is loaded.
// ON (cc9-lua70, 2026-10-19): JM06 s45_j6_k pair (ref env, local\l70r_j6k_{main,pwg}_ref.log),
// gameplay identical, both complete 790.93 s; AddPowerup runs twice (JM06.lua:864-865) and
// queues pup_ready; the class registry loads 32 records.
inline constexpr bool kPowerupGrantBound = true;

inline constexpr std::size_t kPowerupPlayerSlots = 8;  // manager+20h and +14Ch, stride 0Ch

// One 1Ch inventory item, the value at list node+8h. 008E6A80 zeroes +0h, +4h
// and +8h and sets +0Ch to -1; +10h..+18h is the modifier pointer list, which a
// passive use (008EADA0 -> 008EA250) fills (packet cc13-powerup-bind phase 2a).
struct PowerupItem {
    std::uint32_t word_00{};          // +0h, an active item's live record count
    const PowerupClassConfig* cls{};  // +4h, 008E7820 from classID through 008E6CB0
    // +8h, the granting command building's binding (008ED554), as its units-host
    // index; kNoBinding for an AddPowerup grant (008E6A80's zero).
    std::size_t binding{kNoBinding};
    std::int32_t use_limit{-1};       // +0Ch, useLimit (default -1)
    std::list<game_powerups::PowerupModifier*> modifiers{};  // +10h
    static constexpr std::size_t kNoBinding = static_cast<std::size_t>(-1);
};

// What the manager reaches outside itself. The mission frame implements it and
// publishes it with the registry (the image's 00F88C30 publication at 004DC6A0).
class PowerupManagerHost {
public:
    virtual ~PowerupManagerHost() = default;
    virtual float clock_00f876a4() = 0;
    virtual std::int32_t local_player_slot_18ec() = 0;  // [game+18ECh]
    virtual std::int32_t session_word_1fe4() = 0;       // [game+1FE4h]
    virtual bool powerups_enabled_00e0c978() = 0;       // EnablePowerups
    // 009789A0 on the warning manager [00F8A0C4]: (slot, item, name).
    virtual void report_powerup_009789a0(std::int32_t slot, const PowerupItem& item,
        const std::string& name) = 0;
    // [00E198C4] non-null and its +C0h screen's byte +5 set; then 0061B0F0 on it.
    virtual bool powerup_screen_shown_00e198c4() = 0;
    virtual void powerup_screen_rebuild_lists_0061b0f0() = 0;
    // 004E1CA0's tracker: 0068E600 (has been shown) and 00690FD0(hint, 0).
    virtual bool hint_has_been_shown_0068e600(const std::string& hint) = 0;
    virtual void trigger_hint_00690fd0(const std::string& hint, std::int32_t arg) = 0;
    // 008E34F0 on a found class (acts only on group 1, air support).
    virtual void resolve_air_support_classes_008e34f0(PowerupClassConfig& cls) = 0;
    // 008E6E00 + 0076A9F0 + 007657D0: message 67 to the session (host only).
    virtual void send_powerup_message67(std::int32_t slot, const PowerupItem& item,
        std::int32_t op) = 0;
    // The active slot's +14Ch map values as 008EB110 walks them, and its gate
    // (the inventory size at +28h + slot*0Ch).
    virtual std::vector<PowerUpSlot>& active_slot_records() = 0;
    virtual void set_active_slot_notify_gate(bool inventory_nonempty) = 0;
};

// _stricmp's order in the C locale: bytes compared after folding A..Z to a..z.
struct AsciiCaseLess {
    bool operator()(const std::string& a, const std::string& b) const noexcept {
        const std::size_t n = a.size() < b.size() ? a.size() : b.size();
        for (std::size_t i = 0; i < n; ++i) {
            unsigned char x = static_cast<unsigned char>(a[i]);
            unsigned char y = static_cast<unsigned char>(b[i]);
            if (x >= 'A' && x <= 'Z') x = static_cast<unsigned char>(x + ('a' - 'A'));
            if (y >= 'A' && y <= 'Z') y = static_cast<unsigned char>(y + ('a' - 'A'));
            if (x != y) return x < y;
        }
        return a.size() < b.size();
    }
};

struct PowerupCounts {
    unsigned long long class_records{}, texture_requests{};
    unsigned long long add_calls{}, add_two_argument{}, grants{}, grants_gated{},
        grant_class_misses{}, pup_gain_reports{}, screen_rebuilds{}, hint_triggers{},
        session_messages{}, air_support_resolves{}, stream0_draws{};
    unsigned long long prepare_calls{}, prepare_loads{};
    unsigned long long available_calls{}, available_entries{}, records_inserted{};
};

struct PowerupRuntime {
    PowerupConfigOwner* manager{};  // 00F88C30
    PowerupManagerHost* host{};
    std::array<std::list<PowerupItem>, kPowerupPlayerSlots> inventories{};  // +20h + slot*0Ch
    // +14Ch + slot*0Ch: each slot's runtime map keys (the class pointer, compared
    // as an unsigned dword as 00617030 does), in key order. The active slot's
    // values are host->active_slot_records(), index-aligned with these keys;
    // other slots keep theirs in other_records. Keys, values and the notify gate
    // share one lifetime: the published host's. Its frame builds the runtime at
    // load (empty, as the image's new manager is) and destroys it before it goes
    // (destroy_powerup_manager_004d2d84), so a later frame never sees these keys.
    std::array<std::vector<std::uintptr_t>, kPowerupPlayerSlots> record_keys{};
    std::array<std::vector<PowerUpSlot>, kPowerupPlayerSlots> other_records{};
    // +80h + category*0Ch: the sixteen modifier lists (packet cc13-powerup-bind,
    // include/bsp/game_hosts_powerups.hpp), with the manager's lifetime.
    game_powerups::PowerupCategoryLists category_lists{};
    // The command buildings whose binding the manager observes (00694A60 after a
    // 008ED720 grant), by units-host index, with the class 008ED720 stored at
    // binding+8h. 008ED620 regrants these on code 5.
    std::map<std::size_t, const PowerupClassConfig*> observed_bindings{};
    // +1ACh: 008EC650's pick cache, keyed by the whole chunk under 008E5BB0's
    // _stricmp order: an 'A' chunk's one fixed pick, any other chunk's expanded
    // list (a 'T' pick erases its name).
    std::map<std::string, std::vector<std::string>, AsciiCaseLess> bracket_picks{};
    // LABELLED stand-in for BSP_RandomThreads stream 0, 008EC650's draws
    // (game_hosts_powerups.cpp). Default seed, reset with the manager, so each
    // manager lifetime starts the same sequence; the image's stream 0 is
    // process-wide and its seed and MT sequence are not modelled.
    std::mt19937 stream0_stand_in{};
    PowerupCounts counts{};
    std::string load_error{};
};

// The [00F88C30] slot. The image's manager lives from one mission load to the
// next world teardown; this object holds whichever manager is current.
PowerupRuntime& powerup_runtime();

// 004DC6A0's 008EDC60 + 008ECEC0 for the registry only: constructs the owner and
// walks PowerupClasses after `script_text` ran in a fresh Lua state. The faction
// is [[game+18CCh+4*[game+18ECh]]+28h]. Publishes `host` with the owner. Returns
// false (error set, manager and host cleared) when the text or the walk fails.
// The runtime must be empty (destroyed by the previous frame).
bool load_powerup_classes_004dc6a0(PowerupRuntime& runtime, PowerupManagerHost& host,
    std::string_view script_text, std::int32_t selected_player_faction);

// 004D2BB0's manager release (004D2D84..004D2D96): deletes the manager and
// empties the inventories, keys and other slots' records; clears the host.
void destroy_powerup_manager_004d2d84(PowerupRuntime& runtime);

// [game+18ECh]: whether `slot` is the local player's, whose +14Ch values the
// host keeps.
bool is_active_slot(PowerupRuntime& runtime, std::int32_t slot);
// 00617030 on manager+14Ch + slot*0Ch with the class key; a miss inserts the
// zeroed value (008E3710). Throws for a slot outside 0..7.
PowerUpSlot& slot_record_00617030(PowerupRuntime& runtime, std::int32_t slot,
    const PowerupClassConfig* cls);

// 008E6CB0: the class named `name`, or null.
const PowerupClassConfig* find_powerup_class_008e6cb0(PowerupRuntime& runtime,
    std::string_view name);
// 008EA0C0: `slot`'s ready items (one per class, a later item replacing an
// earlier one of its class). Inserts a zeroed record per inventory class first.
std::vector<const PowerupItem*> list_ready_items_008ea0c0(PowerupRuntime& runtime,
    std::int32_t slot);

// The natives, __fastcall(ECX = Lua state), RET; each returns its result count.
int lua_add_powerup_008ee410(PowerupRuntime& runtime, lua_State* state);
int lua_prepare_powerup_008ea610(PowerupRuntime& runtime, lua_State* state);
int lua_get_available_powerups_008eb350(PowerupRuntime& runtime, lua_State* state);

} // namespace bsp::cc13_powerup

#pragma once

#include "bsp/gui_lua_reader.hpp"
#include "bsp/profile_unlock.hpp"

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>

namespace bsp {

using ProfileArchiveReader = GuiLuaReader;
struct ProfileArchiveWriter;

// Native enum-key trees compare signed ints. The archive spells party indices
// 0..3 OWN/ENEMY/NEUTRAL/UNKNOWN and unit classes 0..19 mothership..other.
using MissionEnumCounters = std::map<int, int>;
using MissionEnumCounterPairs = std::map<int, MissionEnumCounters>;
using MissionEnumCounterTriples = std::map<int, MissionEnumCounterPairs>;

struct MissionObjectiveValue {
    std::string name; // native node+14h native string (text pointer at +18h)
    int value{0};     // native node+1Ch
};
using MissionObjectiveMap =
    std::map<std::string, MissionObjectiveValue, NativeStringCaseInsensitiveLess>;

enum class MissionScoreMap : std::size_t {
    Mission, Action, Ship, Plane, Command, MissionMedals, ActionMedals
};

// Persisted portion of the 284h native score record plus its +284h count.
// Offsets are evidence labels, not this C++ struct's layout. Runtime-only
// native fields are outside this persistence model. All represented defaults
// are established by 0091CE90/0091D620, including empty owned containers.
struct MissionScoreRecord {
    int mission_completed_00{0};
    int ranking_04{0};
    int difficulty_08{0};
    float play_time_0c{0.0f};
    float completion_time_10{0.0f};
    float map_usage_14{0.0f};
    // In MissionScoreMap order: +18h,+24h,+30h,+3Ch,+48h,+54h,+60h.
    std::array<UnlockCounterMap, 7> score_maps;
    // Damages: unit class -> party -> unit class -> int.
    MissionEnumCounterTriples player_damages_9c;
    MissionEnumCounterTriples party_damages_a8;
    // Kills: party -> unit class -> unit class -> int.
    MissionEnumCounterTriples player_kills_b4;
    MissionEnumCounterTriples party_kills_c0;
    MissionEnumCounterPairs unit_remaining_cc; // party -> unit class -> int
    MissionEnumCounters unit_suicide_d8;        // unit class -> int
    std::map<int, float> unit_usage_114;        // unit class -> float
    // repairuses, formationuses, shipvsShip, islandCapture at +168h..174h.
    std::array<int, 4> checkpoint_counters_168{};
    int winner_mode_194{0};
    // mission, action, ship, plane, command, badge, total at +1C8h..1E0h.
    std::array<int, 7> totals_1c8{};
    int allied_party_1e4{0};
    int japanese_party_1e8{0};
    std::array<UnlockCounterMap, 2> losses_1ec; // allied +1ECh; Japanese +1F8h
    // allied primary/secondary/hidden, then Japanese primary/secondary/hidden;
    // trees +204h,+210h,+21Ch,+228h,+234h,+240h. Each outer key owns ONE
    // (name,int) pair. Multiple inner archive keys overwrite that pair in order.
    std::array<MissionObjectiveMap, 6> objectives_204;
    bool checkpoint_pending_24c{false}; // writer consumes and clears this flag
    int used_slot_250{0};
    std::array<int, 3> completed_by_difficulty_26c{};
    std::array<int, 3> scores_by_difficulty_278{};
    int count_284{0};
};

using MissionScoreMapStorage =
    std::map<std::string, MissionScoreRecord, NativeStringCaseInsensitiveLess>;

// 00920E10 constructs three empty trees; normal C++ ownership releases all
// nodes and nested records. Native sentinel allocation/SEH/ABI are not emulated.
struct MissionProgress {
    MissionScoreMapStorage mission_scores_00;
    UnlockCounterMap single_best_scores_0c;
    UnlockCounterMap multi_scores_18;
};

MissionScoreRecord& mission_record_00594a70(MissionProgress&, std::string_view key);

// Native __thiscall(ECX=record, archive*), RET 4. The reader's original global
// game+738h version is explicit here. Signed native versions below 2 read no
// record fields. No reader routine clears existing containers.
void read_mission_score_record_00916a00(
    MissionScoreRecord&, ProfileArchiveReader&, std::uint32_t profile_version);
void write_mission_score_record_00908a30(MissionScoreRecord&, ProfileArchiveWriter&);

// Native __thiscall(ECX=24h object, archive*), RET 4. Reads current wrappers or
// legacy direct mission entries; writes current wrappers. See the documented
// native checkpoint spelling/navigation asymmetry before expecting a roundtrip.
void read_mission_progress_00920000(
    MissionProgress&, ProfileArchiveReader&, std::uint32_t profile_version);
void write_mission_progress_0090cb40(MissionProgress&, ProfileArchiveWriter&);
int sum_mission_progress_0090bf50(const MissionProgress&) noexcept;
void sync_mission_completion(const MissionProgress&, ProfileUnlockState&);

} // namespace bsp

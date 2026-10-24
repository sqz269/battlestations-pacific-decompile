#pragma once
// The squadron free-attack auto-target tick 009F8470, hosted. Packet
// p15_free_attack_tick, docs/OCEAN_WAVE_FIELD.md section 57.
//
// cc13's reconstruction (src/cc13_seg68.cpp, docs/CC13_SEG68.md 1) reads the
// native object graph through a raw-memory host: u8/u32/f32 at offsets of the
// director (the squadron's +348h block), the squadron, its leader, the recon
// slot's contact list, the AttackLists block and the tuning record. This file
// runs that reconstruction unchanged over a small arena the host fills, per
// squadron and per tick, with the fields the tick reads, and records the calls
// the tick makes (the override send, the clear-and-moveto, the stage raise) as a
// decision the host then applies. Every name is a hypothesis, not a recovered
// symbol; RNG-free.

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "bsp/band_009f_cc13.hpp"

namespace bsp {

inline constexpr std::size_t kFreeAttackNoUnit = static_cast<std::size_t>(-1);

// One AttackLists list as 009F89F0 leaves it in the 004D6CE0 block: the vector at
// `offset` (begin at +4h, end at +8h, read by 009F7B00) holding kind indices of the
// 97-entry name table 00E0CD80, in key order "1".."n" (key "0" is absent, so its
// nil is skipped at 009F8C1D).
struct SquadronAttackList {
    const char* name;
    std::uint32_t offset;
    std::vector<int> kinds;
};

// This installation's scripts/datatables/squadronfreeattacktargets.lua (mtime
// 2024-07-13, the untouched bulk). LABELLED: transcribed, not run through the
// loader 009F89F0; the names map through the table at 00E0CD80 read from the
// executable on disk.
const std::array<SquadronAttackList, 8>& squadron_free_attack_lists() noexcept;

// One entry of the side's enemy contact chain [008053C0(party)+DE8h] in chain
// order. `squadron` marks the kind-18h record 00805490 groups a squadron under;
// the host's unit for it is the squadron's flight leader (the fused wing 0), and
// the arena gives it its own object so IsKindOf(18h) and +3D0h answer as the
// squadron entity's do.
struct FreeAttackContact {
    std::size_t unit{kFreeAttackNoUnit};
    bool squadron{false};
};

// The per-unit questions the tick's callees ask, by host unit index.
struct SquadronFreeAttackWorld {
    virtual ~SquadronFreeAttackWorld() = default;
    virtual bool is_kind_of(std::size_t unit, int kind) = 0;          // vtable[5Ch]
    virtual std::array<float, 3> position(std::size_t unit) = 0;      // +FCh..+104h
    virtual bool untouchable_1d4(std::size_t unit) = 0;               // [vtable[140h]()]+1D4h
    virtual bool outside_map_0071c4f0(std::size_t unit) = 0;
    virtual bool command_blocks_auto_action_00811f50(std::size_t unit) = 0;
    virtual bool squadron_outside_map_detached_007ee8b0(std::size_t leader) = 0;
    virtual bool sensors_detect_00804350(std::size_t observer, std::size_t target,
                                         std::uint32_t mask) = 0;
    // 007EEC50(ECX = the squadron, target, a, b): the squadron's self queries are
    // its leader's in this host.
    virtual std::uint32_t choose_attack_command_007eec50(std::size_t leader,
        std::size_t target, std::uint8_t a, int b) = 0;
};

// What the tick reads off the squadron and its +348h block.
struct SquadronFreeAttackInput {
    // Director (the +348h block).
    std::uint32_t mode_30{0};
    bool allow_fire_3c{false};
    bool allow_move_3d{false};
    std::uint32_t slot0_command_54{0};
    std::size_t slot0_target{kFreeAttackNoUnit};   // 00521EA0 on +58h
    bool slot0_target_squadron{false};
    bool accepts_new_target_0071df70{false};
    bool path_point_valid{false};                  // 0071E4C0
    std::array<float, 3> path_point{};
    // Squadron.
    std::size_t leader{kFreeAttackNoUnit};         // +3D0h
    int party_54{0};
    int attack_mode_370{0};
    int leader_slot_1b0{8};
    bool leader_slot_ai_held_00927f10{false};
    bool leader_player_184{false};
    bool leader_pilot_fires_c24{false};
    std::array<float, 3> squadron_position{};      // 00427EB0
    band009f::SquadronFilterInputs filter_inputs{};  // 009F6C80's predicates
    // Tuning.
    float guard_dist_594{0.0f};
    // The enemy contact chain.
    std::vector<FreeAttackContact> contacts;
    // Packet cc13-fapick, log-only: collect 009F7810's per-candidate notes.
    bool trace_candidates{false};
};

// Packet cc13-fapick: one recon-list entity as 009F7810's loop left it. `note`
// is cc13_seg68::SquadronCandidateNote; free_attack_candidate_note_name names it.
struct FreeAttackCandidateTrace {
    std::size_t unit{kFreeAttackNoUnit};
    bool squadron{false};
    int note{0};
    int rank{0};
    float score{0.0f};
};
const char* free_attack_candidate_note_name(int note) noexcept;

// How far a tick got, for the census.
enum class FreeAttackStop {
    kModeNotQueueHead,     // 009F849F: +30h != 1
    kGateClosed,           // 009F84D9..009F84F8
    kNoTarget,             // 009F8520: the selector found none (or filter B)
    kRefused0071df70,      // 009F852C
    kSameTarget,           // 009F8560: the head's own target
    kNotAirTarget,         // 009F85A1: a bomber in vt50 == 1 needs a plane/squadron
    kOrdnanceToOrdnance,   // 009F85E1
    kOverrideSent,         // 009F8631
};

struct SquadronFreeAttackDecision {
    FreeAttackStop stop{FreeAttackStop::kModeNotQueueHead};
    band009f::SquadronTargetFilter filter{band009f::SquadronTargetFilter::At14};
    bool raise_override_stage_2{false};            // 009F8640's 0071D9E0(2)
    std::size_t target{kFreeAttackNoUnit};         // the selector's pick
    bool target_squadron{false};
    std::size_t candidates{0};                     // contacts the selector scored
    std::uint32_t command{0};                      // 009F6D60's class
    bool clear_then_moveto_self{false};            // 009F85EB..009F8615
    std::vector<FreeAttackCandidateTrace> traces;  // cc13-fapick, with in.trace_candidates
};

// 009F8470 after its 009F8160 / 009F7C90 calls (the host runs those itself).
SquadronFreeAttackDecision squadron_free_attack_tick_009f8470(
    const SquadronFreeAttackInput& in, SquadronFreeAttackWorld& world);

// Packet cc13-leash. The descriptor 009F7FE0 measures (009F83DE: director+18Ch,
// the override's): byte +0 an entity, byte +1 a point at +8h, else 00F87574.
struct SquadronGuardDescriptor {
    bool entity{false};
    std::size_t unit{kFreeAttackNoUnit};   // the entity's host unit (a squadron: its leader)
    bool squadron{false};
    bool position_valid{false};
    std::array<float, 3> position{};
};

// 009F7FE0 (cc13_seg68::bot_within_guard_distance_009f7fe0, run unchanged over
// the arena): reads in.path_point_valid / in.path_point (0071E4C0) and
// in.guard_dist_594 (GameTuning+594h) only.
bool squadron_within_guard_distance_009f7fe0(const SquadronFreeAttackInput& in,
    const SquadronGuardDescriptor& descriptor, SquadronFreeAttackWorld& world, float scale);

}  // namespace bsp

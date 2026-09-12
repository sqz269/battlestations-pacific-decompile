// bsp_game.exe milestone 2n: the ship AI controller per unit, and the weapon
// director's own automatic target selector.
//
// Nothing here is a reconstruction of native code. Every method is one call site
// of bsp::ShipAiControllerHost, bsp::ShipAiSyncHost, bsp::ShipAiSetterHost,
// bsp::ShipAiDirectControlHost, bsp::ShipAiPublishHost or bsp::BotFireTargetHost,
// satisfied either by a reconstruction already on main or by the explicit
// unimplemented policy in GameHostLog. Addresses, evidence and the decisions
// this file makes rather than recovers: include/bsp/game_hosts_ship_ai.hpp,
// docs/SHIP_AI_STATES.md, docs/UNIT_AUTOPILOT_PAIR.md, docs/BOT_FIRE_TARGET.md
// and the milestone 2n section of docs/GAME_EXECUTABLE.md.

#include "bsp/game_hosts_ship_ai.hpp"

#include <array>
#include <cmath>
#include <deque>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/command_completion.hpp"
#include "bsp/director_update_arms.hpp"
#include "bsp/ship_ai_approach_update.hpp"
#include "bsp/ship_ai_attackmove_substates.hpp"
#include "bsp/ship_ai_bearing_rating.hpp"
#include "bsp/ship_ai_ring_scan.hpp"
#include "bsp/ship_ai_clearance_profile.hpp"
#include "bsp/ship_ai_nav_block_ctor.hpp"
#include "bsp/ship_ai_navigation.hpp"
#include "bsp/ship_ai_navigation_arm_tail.hpp"
#include "bsp/ship_ai_path_follower.hpp"
#include "bsp/ship_ai_path_planner.hpp"
#include "bsp/ship_ai_sector_scan.hpp"
#include "bsp/ship_ai_path_point.hpp"
#include "bsp/ship_ai_path_refresh.hpp"
#include "bsp/ship_ai_path_search.hpp"
#include "bsp/ship_ai_goal_vector.hpp"
#include "bsp/ship_ai_obstacle_tables.hpp"
#include "bsp/ship_ai_state_steps.hpp"
#include "bsp/ship_ai_throttle_ring.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/unit_state_message.hpp"
#include "bsp/vector_helpers.hpp"
#include "bsp/weapon_director.hpp"

namespace bsp::game {
namespace {

// The two re-plan interval getters, read from the image for this packet.
// 009DAC30 is `FLD [00CE3958]` and 00CE3958 holds 40 00 00 00 = 2.0f; 009DAA90
// is `FLD [00CE3850]` and 00CE3850 holds 40 A0 00 00 = 5.0f. Cruise is the one
// state that carries its own getter; every other leaf vtable's +28h is 009DAA90.
constexpr std::uint32_t kCruiseIntervalGetter = 0x009dac30u;
constexpr std::uint32_t kSharedIntervalGetter = 0x009daa90u;
constexpr float kCruiseIntervalTicks = 2.0f;
constexpr float kSharedIntervalTicks = 5.0f;

// The three state objects 009F3D00 can reach through its attack arm, by their
// brain offsets. docs/SHIP_AI_STATES.md names the objects; 009F3D86, 009F3D8E
// and 009F3D96 are the three LEA sites, in ai offsets.
constexpr std::uint32_t kAiOffsetKamikaze = 0x2254u;   // brain+21FCh
constexpr std::uint32_t kAiOffsetSubAttack = 0x217Cu;  // brain+2124h
constexpr std::uint32_t kAiOffsetAttackMove = 0x0C70u; // brain+0C18h

struct StateDescriptor {
    const char* name;
    std::uint32_t command_object;
    std::uint32_t step;      // the leaf vtable's +0Ch
    std::uint32_t interval;  // the leaf vtable's +28h
    bool step_concrete;
};

// Read out of the seven leaf vtables for this packet: 00D21598 (cruise),
// 00D215C8 (stop), 00D215F8 (follow), 00D21628 (movetopos), 00D21658 (land),
// 00D21688 (moveonpath) and 00D219D0 (attackmove), plus 00D216B8 (kamikaze).
// Only `cruise`'s step has a reconstruction; the rest are records with their
// own addresses, because docs/SHIP_AI_STATES.md read none of their bodies.
const StateDescriptor* state_for_ai_offset(std::uint32_t ai_offset) noexcept {
    static const StateDescriptor kStates[] = {
        {"cruise",          0x00e08f70u, 0x009e1170u, kCruiseIntervalGetter, true},
        {"stop",            0x00e08f88u, 0x009e14c0u, kSharedIntervalGetter, false},
        {"follow",          0x00e08f60u, 0x009e1610u, kSharedIntervalGetter, false},
        {"land",            0x00e08fa0u, 0x009e1950u, kSharedIntervalGetter, false},
        {"movetopos",       0x00e08f68u, 0x009e5770u, kSharedIntervalGetter, false},
        {"moveonpath",      0x00e08f80u, 0x009e59c0u, kSharedIntervalGetter, false},
        {"attackmove",      0x00e08f78u, 0x009e8820u, kSharedIntervalGetter, false},
        {"kamikaze_attack", 0x00e08f78u, 0x009e2020u, kSharedIntervalGetter, false},
        {"sub_attack",      0x00e08f78u, 0u,          kSharedIntervalGetter, false},
    };
    static const std::uint32_t kOffsets[] = {
        0x0BC8u, 0x0BD8u, 0x0BE4u, 0x0C38u, 0x0C5Cu, 0x0C64u,
        kAiOffsetAttackMove, kAiOffsetKamikaze, kAiOffsetSubAttack,
    };
    for (std::size_t i = 0; i < sizeof(kOffsets) / sizeof(kOffsets[0]); ++i) {
        if (kOffsets[i] == ai_offset) return &kStates[i];
    }
    return nullptr;
}

const char* steering_mode_name(int mode) noexcept {
    switch (mode) {
        case 0: return "rudder";
        case 1: return "heading";
        case 2: return "navigate";
        case 3: return "navigate_astern";
        default: return "?";
    }
}

const char* direction_name(int direction) noexcept {
    switch (direction) {
        case 0: return "stopped";
        case 1: return "ahead";
        case 2: return "astern";
        default: return "?";
    }
}

}  // namespace

// ---------------------------------------------------------------------------

struct GameShipAiHost::Impl {
    Impl(GameHostLog& log_in, GameUnitsHost& units_in) : log(log_in), units(units_in) {}

    GameHostLog& log;
    GameUnitsHost& units;

    // One controller per created unit. `ai` in docs/SHIP_AI_STATES.md is the
    // whole of this record: the timers are ai+0B14h / ai+0B18h, the block is
    // ai+60h, and the order slots live on the unit rather than on the AI.
    struct Controller {
        bsp::ShipAiControllerTimers timers{};
        bsp::ShipAiControlBlock blk{};
        bsp::UnitAiOrderPromotion order{};      // unit+0AECh / unit+0A98h, unit+0B40h
        bsp::AutoTargetState target{};          // director+38h
        std::uint32_t active_state_ai_offset{0};  // ai+2264h
        std::uint32_t active_state_command{0};
        bsp::NativeHandle fire_target{0};       // director+238h as this process holds it
        std::string sample;                     // the last line log_sample emitted
        // Milestone 2o. `drive` is the labelled diagnostic stand-in of
        // --ai-drive; the two live values are the previous step's ring+148h /
        // +14Ch, so a change can be counted rather than assumed.
        bool drive{false};
        float drive_throttle{0.0f};
        float drive_rudder{0.0f};
        float live_throttle{0.0f};
        float live_rudder{0.0f};
        // The state steps that landed with packet ship_ai_state_steps. Each is
        // a different C++ object over the same native block or state object:
        // `goal` is the blk fields 009DE050 owns (+1C4h, +1C8h, +1CCh,
        // +1DCh..+1F0h, +314h, +2FDh, +2FEh), `path` the fields 009DA4E0
        // clears, `avoidance` the trio `stop` writes at blk+3ECh / +3F0h /
        // +3F4h, `stop_state` the `stop` leaf's own state+8h byte latch and
        // `selector` the attackmove leaf's +14FCh..+1508h.
        bsp::ShipAiGoalPlan goal{};
        bsp::ShipAiPathPlan path{};
        bsp::ShipAiAvoidanceRequest avoidance{};
        bsp::ShipAiStopStepState stop_state{};
        bsp::ShipAiAttackMoveSelector selector{};
        // Milestone 2p. `goal_vector` is the brain fields 009F1420's head owns
        // (+0B20h, +0B24h, +0B28h, +0B2Ch..+0B34h, +0B38h, +0B54h, +0B58h) and
        // `latched` the record at brain+0AF8h that 009E2FB0 writes and
        // 009DBCC0 reads. `obstacle` is the blk half the middle of 009F3F80
        // owns: the twelve sectors at blk+808h, the 65-bin profile at blk+4h,
        // the danger level and its two dwell timers, the astern latch and the
        // escape state. `path_point` is the 22h-byte record 009E3C00 fills.
        bsp::ShipAiGoalVectorState goal_vector{};
        bsp::ShipAiGoalTargetRecord latched{};
        bsp::ShipAiObstacleState obstacle{};
        bsp::ShipAiPathPointRecord path_point{};
        // The five attackmove sub-state objects 009E8450 builds, each with its
        // own storage. 007B3DD0 has none: its whole body is one RET 4.
        bsp::ShipAiAttackMoveEngageState engage{};
        bsp::ShipAiAttackMoveLeadPursuitState lead_pursuit{};
        bsp::ShipAiAttackMoveTangentState tangent{};
        float substate_ring_timer_14b4{0.0f};  // sub+14B4h, the approach warn sweep
        // 009EF230's own round robin at blk+0A18h: a 0..3 counter that refreshes
        // three of the twelve sectors per frame.
        int sector_refresh_cursor_0a18{0};
        // Two blk fields include/bsp/ship_ai_states.hpp does not declare and
        // this packet does not add to it, because that header belongs to
        // another packet. blk+3A6h is the second half of the station-keeping
        // gate at 009EDA41 and blk+39Ch the slot 009EE5A5 pins to 1.25f before
        // the path refresh and the station-keeping arm computes instead.
        bool flag_3a6{false};
        float speed_scale_39c{0.0f};
        // The two 68h-byte plan blocks the navigator owns at nav+224h and
        // nav+28Ch, and the front / back pointers at nav+2F4h / +2F8h that
        // 009ED3E0 swaps. Packet ship_ai_path_planner, on main at 878325ba.
        bsp::ShipAiPathPlanBlock plan_a{};
        bsp::ShipAiPathPlanBlock plan_b{};
        int plan_front{0};                 // 0 selects plan_a, 1 plan_b
        // Milestone 2q: nav+2FCh, the "a plan is being computed" byte the
        // 009ED4E4 arm raises at 009ED528 / 009ED63B and clears at 009ED5B6.
        bool plan_computing_2fc{false};
        // The blk half 009EE671's output block owns. Every offset is named in
        // bsp/ship_ai_navigation.hpp; none of them is in ShipAiControlBlock.
        bsp::ShipAiNavState nav{};
        // The blk half 009EEAAB's tail owns: blk+2FCh, +2FDh, +2FEh, +344h and
        // the plan's search state. Packet ship_ai_navigation_arm_tail, on main
        // at 4491d04f. blk+2FCh is the same byte 009ED3E0 writes at 009ED4DE,
        // because 009EE5B5 hands 009ED3E0 the controls step's own `this`.
        bsp::ShipAiArmTailState tail{};
        // Whether the tail ran on this tick, which decides whether blk+344h
        // carries its ceiling or the 009F4DA0 record's 1.0f.
        bool arm_tail_ran{false};
        // A deque because a push_back never moves an existing element, and the
        // plan block links the nodes by address.
        std::deque<bsp::ShipAiPathNode> plan_nodes;
        // The attackmove approach sub-state's nested ring object, sub+8h.
        // Packet ship_ai_approach_update, on main at 89d4bb77.
        bsp::ShipAiApproachState approach{};
        // Milestone 2r: what 009E4330 wrote into this block when the brain
        // record was constructed. Packet ship_ai_nav_block_ctor, on main at
        // 3b4e07a6. It is the producer of blk+3C8h, +3CCh, +3D0h, +3D4h, +3D8h
        // and +604h, the five inputs milestone 2q reported as unwritten, and of
        // blk+340h, +318h, +1B4h, +1B8h and +3E4h beside them.
        bsp::ShipAiNavBlockFields nav_block{};
        bool nav_block_built{false};
        // Milestone 2r: the blk half 009EF910 owns - the refresh timer +374h,
        // the clearance +37Ch, the outcome +370h and the hold +354h. Packet
        // cc_ai_clearance_profile. 009F4D87 calls it from inside the publish,
        // one chain slot before the drive whose danger ramp divides +37Ch by
        // unit+9CCh.
        bsp::ShipAiClearanceBlock clearance{};
        // Milestone 2r: blk+4h, blk+34Ch, blk+350h and blk+354h, the fields
        // 009E04E0 owns. Its contact-track list at blk+400h / +404h is the
        // empty one 009E4653 leaves, so every step clears the avoidance vector
        // and the 65-bin profile stays in the bypass 009E435F set.
        bsp::ShipAiThrottleProfileBlock throttle_profile{};
        bsp::ShipAiContactTrack track_scratch{};
        // blk+608h with the count at blk+604h. Nothing appends to it: 009F0D20's
        // only call site is 009F1A25 inside the brain pre-pass's candidate walk,
        // and the node footprint that walk builds has no producer in this
        // process. The ageing pass 009F0EA0 runs over it anyway.
        std::vector<bsp::ShipAiObstacleNode*> neighbours;
        std::vector<bsp::ShipAiObstacleNode*> neighbours_expired;
        // Milestone 2r: the 60-slot approach ring at nested+30h and the 60
        // score records the four scorers fill. 009E5530's second pass builds
        // the ring once, when the attackmove sub-state object is constructed.
        bsp::ShipAiAttackMoveRingSlot approach_ring[bsp::kAttackMoveRingSlotCount]{};
        bsp::ShipAiApproachSlotScore approach_scores[bsp::kShipAiApproachSlotCount]{};
        bool approach_ring_built{false};
    };
    std::vector<Controller> controllers;
    std::vector<GameShipAiRow> rows;
    GameShipAiSummary summary{};
    unsigned long long steps{0};
    bool logged_position{false};
    bool logged_gates{false};
    bool logged_party_list{false};
    bool logged_accept_gate{false};
    bool logged_state_steps{false};

    // 009E5530's second pass: the 60-slot approach ring, built once when the
    // attackmove sub-state object is constructed. 009E5680..009E5758 is the
    // whole of the per-slot rule and is already reconstructed.
    void ensure_approach_ring(Controller& ctl, std::size_t index) {
        if (ctl.approach_ring_built) return;
        ctl.approach_ring_built = true;
        for (int i = 0; i < bsp::kAttackMoveRingSlotCount; ++i) {
            ctl.approach_ring[i] = bsp::ship_ai_attackmove_ring_slot_009e5530(i,
                static_cast<std::uint32_t>(index) + 1u,
                static_cast<std::uint32_t>(index) + 1u);
        }
        done("ShipAiApproach::build_ring_009e5530", 0x009e5530u);
    }

    void record(const char* method, std::uint32_t address) {
        char text[16];
        std::snprintf(text, sizeof(text), "%08lx", static_cast<unsigned long>(address));
        log.unimplemented(method, text);
    }
    void record_slot(const char* method, const char* text) { log.unimplemented(method, text); }
    // Milestone 2o, chain slot 16: 009F4DA0's tail call at 009F50C6 into
    // 009F3F80, and 009F3F80's own tail, which is the hop into the unit's
    // order ring. Defined below the host bindings it builds.
    void drive_order_ring_009f3f80(std::size_t index, Controller& ctl, GameShipAiRow& row,
        float seconds);
    // 009DE050, the navigation goal setter every state step but `cruise`'s goes
    // through. It owns a different set of blk fields than ShipAiControlBlock
    // covers, so the three both describe (+1C4h, +1C8h, +1CCh) are mirrored
    // across the call. Defined below the host bindings it builds.
    void run_navigation_goal_009de050(Controller& ctl, GameShipAiRow& row, std::size_t index,
        float goal_x, float goal_z, bool keep_mode, bool final_leg);
    void done(const char* method, std::uint32_t address) {
        char text[16];
        std::snprintf(text, sizeof(text), "%08lx", static_cast<unsigned long>(address));
        log.implemented(method, text);
    }
};

namespace {

// ---------------------------------------------------------------------------
// bsp::ShipAiSyncHost, the four call sites inside 009F3DD0 plus the dispatch
// 009F3D00 that its last one enters.
// ---------------------------------------------------------------------------

class SyncBinding final : public bsp::ShipAiSyncHost {
public:
    SyncBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}

    void refresh_director_vtable_0114() override {
        // 009F3DE2, CALL EDX = [ai+0B00h]->vtable[114h]. The unit's own getter
        // for its weapon director; this process holds one director per unit in
        // GameCommandsHost, so the dispatch itself is the record.
        owner_.record_slot("ShipAi::unit_weapon_director", "00cfc3d0+vtable114");
    }
    std::uint32_t director_current_command_0071be40() override {
        const std::uint32_t command = owner_.units.director_current_command_0071be40(index_);
        owner_.done("ShipAi::director_current_command", 0x0071be40u);
        return command;
    }
    bool unit_player_controlled_0184() override {
        // 009F3DF3, [ai+0B00h]+184h. The executable's own byte is the controlled
        // unit 004C0890 bound, which is what unit+184h names.
        owner_.done("ShipAi::unit_player_controlled", 0x009f3df3u);
        return owner_.units.unit_player_controlled_0184(index_);
    }
    std::uint32_t active_state_command_vtable24() override {
        // 009F3E12, CALL EAX = [ai+2264h]->vtable[24h]. Every one of the eight
        // getters is `MOV EAX,<singleton>; RET`, read from the image for this
        // packet, so the slot is the table in game_hosts_ship_ai.hpp.
        owner_.done("ShipAiState::command_object_vtable24", 0x009f3e12u);
        return ctl_.active_state_command;
    }
    void select_state_for_command_009f3d00(std::uint32_t command) override {
        // 009F3D00, __thiscall(ai)(command), RET 4, body 009F3D00-009F3DCF,
        // read whole for this packet. Six equality rows, one shared arm for the
        // two attack command objects and a default that falls to `stop`.
        std::uint32_t ai_offset = 0;
        for (const ShipAiCommandStateRow& row : kShipAiCommandStates) {
            if (row.command_object == command) { ai_offset = row.ai_offset; break; }
        }
        if (ai_offset == 0
            && (command == kShipAiArtilleryCommandObject
                || command == kShipAiAttackMoveCommandObject)) {
            // 009F3D73: with [ai+0B0Ch] non-null the arm asks 00779AA0 and picks
            // `kamikaze_attack` or `sub_attack`; with it null it takes
            // `attackmove`. brain+0AB4h has no producer in this process, so the
            // null path is the one that runs and 00779AA0 is not reached.
            owner_.record("ShipAiState::attack_subject_00779aa0", 0x00779aa0u);
            ai_offset = kAiOffsetAttackMove;
        }
        if (ai_offset == 0) {
            ai_offset = kShipAiDefaultStateAiOffset;  // 009F3DA0, the `stop` state
        }
        if (ai_offset == ctl_.active_state_ai_offset) return;
        if (ctl_.active_state_ai_offset != 0) {
            // 009F3DB4..009F3DB9, the outgoing state's vtable[8].
            owner_.record_slot("ShipAiState::exit_vtable08", "00d21598+vtable08");
        }
        ctl_.active_state_ai_offset = ai_offset;
        const StateDescriptor* state = state_for_ai_offset(ai_offset);
        ctl_.active_state_command = state != nullptr ? state->command_object : 0u;
        row_.state = state != nullptr ? state->name : "?";
        row_.state_step = state != nullptr ? state->step : 0u;
        row_.state_step_concrete = state != nullptr && state->step_concrete;
        row_.command_object = command;
        ++row_.state_changes;
        // 009F3DC1..009F3DC8, the incoming state's vtable[4].
        owner_.record_slot("ShipAiState::enter_vtable04", "00d21598+vtable04");
        owner_.done("ShipAiState::select_for_command", 0x009f3d00u);
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// bsp::ShipAiSetterHost, the two callees of 009DFFB0 and 009E0040
// ---------------------------------------------------------------------------

class SetterBinding final : public bsp::ShipAiSetterHost {
public:
    explicit SetterBinding(GameShipAiHost::Impl& owner) : owner_(owner) {}
    void on_steering_mode_change_009da4e0() override {
        owner_.record("ShipAiControls::steering_mode_changed", 0x009da4e0u);
    }
    void after_heading_stored_00605070(float) override {
        owner_.record("ShipAiControls::after_heading_stored", 0x00605070u);
    }

private:
    GameShipAiHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// The state steps, packet ship_ai_state_steps
// ---------------------------------------------------------------------------
//
// 009DE050 is the one writer of the AI's navigation goal and every state step
// but `cruise`'s exists to produce the two floats it takes
// (docs/SHIP_AI_STATE_STEPS.md). It forces blk+1C4h to Navigate, which is what
// hands the steering to 009ED6B0's navigation arm 009EDA26..009EF228 - a span
// this executable records. So a navigation state's step running is not the same
// thing as a navigation state producing a desired throttle, and the run counts
// both separately.

class PathPlanBinding final : public bsp::ShipAiPathPlanHost {
public:
    explicit PathPlanBinding(GameShipAiHost::Impl& owner) : owner_(owner) {}
    void release_path_object_vtable_0000(std::uint32_t) override {
        // 009DA4E9 and 009DA524, (*object)->vtable[0](1). Callee body unread.
        // Both fields are null here because nothing in this process builds a
        // path object, so the arm is recorded and never taken.
        owner_.record_slot("ShipAiPath::release_object", "path+0000+vtable00");
    }
    std::uint32_t path_limit_default_00cf58ec() override {
        // [00CF58EC], read once at 009DA4FB. No producer in this process.
        owner_.record("ShipAiPath::limit_default", 0x00cf58ecu);
        return 0u;
    }

private:
    GameShipAiHost::Impl& owner_;
};

class GoalBinding final : public bsp::ShipAiGoalHost {
public:
    explicit GoalBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl)
        : owner_(owner), ctl_(ctl) {}
    void clear_path_plan_009da4e0() override {
        PathPlanBinding path(owner_);
        bsp::ship_ai_clear_path_plan_009da4e0(ctl_.path, path);
        owner_.done("ShipAiGoal::clear_path_plan", 0x009da4e0u);
    }
    float planar_length_00414c60(float dx, float dz) override {
        owner_.done("ShipAiGoal::planar_length", 0x00414c60u);
        return bsp::length_2d_00414c60(std::array<float, 2>{dx, dz});
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
};

class HeadingHoldBinding final : public bsp::ShipAiHeadingHoldHost {
public:
    HeadingHoldBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                       std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}
    float unit_heading_vtable_0050() override {
        owner_.done("ShipAiHold::unit_heading", 0x009e00b2u);
        return owner_.units.unit_heading_radians(index_);
    }
    void clear_path_plan_009da4e0() override {
        PathPlanBinding path(owner_);
        bsp::ship_ai_clear_path_plan_009da4e0(ctl_.path, path);
        owner_.done("ShipAiHold::clear_path_plan", 0x009da4e0u);
    }
    void after_heading_stored_00605070(float) override {
        owner_.record("ShipAiControls::after_heading_stored", 0x00605070u);
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

class StopStepBinding final : public bsp::ShipAiStopStepHost {
public:
    StopStepBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                    GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}

    bool unit_pose_valid_00c8() override {
        owner_.done("ShipAiStop::unit_pose_valid", 0x009e14d7u);
        return owner_.units.unit_pose_valid_00c8(index_);
    }
    void refresh_unit_pose_00414db0() override {
        owner_.record("ShipAiStop::refresh_unit_pose", 0x00414db0u);
    }
    void unit_position_00fc(float& x, float& y, float& z) override {
        owner_.done("ShipAiStop::unit_position", 0x009e14ecu);
        owner_.units.unit_position_00fc(index_, x, y, z);
    }
    bool position_outside_world_bounds_0071c4f0(float x, float y, float z) override {
        // 0071C4F0's body is read (docs/SHIP_AI_STATE_STEPS.md): it answers 0
        // when the position is INSIDE the box the world object keeps at
        // [00E188A8] +711Ch / +7124h / +7128h / +7130h, and 1 otherwise. The
        // rule is applied here; what is missing is the box, because
        // construct_world 004DE610 is still a load record and this process has
        // no world object. Running the four comparisons against a zero box
        // would put every ship of this mission outside a world that does not
        // exist, so the box is the record and the neutral answer stands.
        float min_x = 0.0f, max_x = 0.0f, min_z = 0.0f, max_z = 0.0f;
        if (owner_.units.world_bounds_box_00e188a8(min_x, max_x, min_z, max_z)) {
            owner_.done("ShipAiStop::outside_world_bounds", 0x0071c4f0u);
            static_cast<void>(y);
            return !(x >= min_x && x <= max_x && z >= min_z && z <= max_z);
        }
        owner_.record("ShipAiStop::world_bounds_box", 0x004de610u);
        return false;
    }
    void set_navigation_goal_009de050(float goal_x, float goal_z, bool keep_mode,
                                      bool final_leg) override {
        owner_.run_navigation_goal_009de050(ctl_, row_, index_, goal_x, goal_z, keep_mode,
                                            final_leg);
    }
    float unit_heading_vtable_0050() override {
        owner_.done("ShipAiStop::unit_heading", 0x009e1534u);
        return owner_.units.unit_heading_radians(index_);
    }
    void set_desired_heading_009e0040(float heading) override {
        SetterBinding setters(owner_);
        bsp::ship_ai_set_desired_heading_009e0040(ctl_.blk, heading, setters);
        owner_.done("ShipAiStop::set_desired_heading", 0x009e0040u);
    }
    float unit_body_axis_speed_0092d730() override {
        owner_.done("ShipAiStop::body_axis_speed", 0x0092d730u);
        return owner_.units.unit_forward_speed_0092d730(index_);
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

class MoveToPosStepBinding final : public bsp::ShipAiMoveToPosStepHost {
public:
    MoveToPosStepBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                         GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}

    bool unit_pose_valid_00c8() override {
        owner_.done("ShipAiMoveTo::unit_pose_valid", 0x009e579au);
        return owner_.units.unit_pose_valid_00c8(index_);
    }
    void refresh_unit_pose_00414db0() override {
        owner_.record("ShipAiMoveTo::refresh_unit_pose", 0x00414db0u);
    }
    void unit_position_xz_00fc(float& x, float& z) override {
        float y = 0.0f;
        owner_.done("ShipAiMoveTo::unit_position", 0x009e57aau);
        owner_.units.unit_position_00fc(index_, x, y, z);
    }
    void brain_goal_xz_0b2c(float& x, float& z) override {
        // 009E57D0 and 009E57B4, brain+0B2Ch and brain+0B34h. Milestone 2p
        // closed the gap milestone 2o recorded here: the brain pre-pass
        // 009F1420 writes all three components on every AI sub-tick from the
        // unit's own active command, so this is now a recovered value.
        owner_.done("ShipAiMoveTo::brain_goal_0b2c", 0x009e57d0u);
        x = ctl_.goal_vector.goal_x_0b2c;
        z = ctl_.goal_vector.goal_z_0b34;
    }
    std::uint32_t director_command_slot_0071bff0(int index) override {
        // 009E57ED, 0071BFF0(director, 0). Milestone 2n established through
        // 0071BE48 that the first command slot holds the command object itself,
        // so this process's own slot 0 answers it.
        owner_.done("ShipAiMoveTo::director_command_slot", 0x0071bff0u);
        static_cast<void>(index);
        return owner_.units.director_current_command_0071be40(index_);
    }
    bool command_on_final_leg_007adc60(std::uint32_t) override {
        // 007ADC60's body was read by packet ship_ai_state_steps: it answers
        // true when the command's waypoint list is absent or empty. No command
        // in this process carries a waypoint list - `moveto` is issued with a
        // target or a position, never a path - so that is the arm the rule
        // itself selects here.
        owner_.done("ShipAiMoveTo::command_final_leg", 0x007adc60u);
        return true;
    }
    void set_navigation_goal_009de050(float goal_x, float goal_z, bool keep_mode,
                                      bool final_leg) override {
        owner_.run_navigation_goal_009de050(ctl_, row_, index_, goal_x, goal_z, keep_mode,
                                            final_leg);
    }
    bool state_goal_reached_vtable_002c(float goal_x, float goal_z) override {
        // 009E5821, state->vtable[2Ch]. Milestone 2p left this a record;
        // docs/SHIP_AI_PATH_PLANNER.md has since read the slot whole: both
        // navigation vtables hold 009DAB10, three instructions that load
        // [state+4h]+8h into ECX and tail-jump to 009DA590, which is projected.
        const bsp::ShipAiPathPlanBlock& live
            = (ctl_.plan_front == 0) ? ctl_.plan_a : ctl_.plan_b;
        const bsp::ShipAiPathArrivalResult arrival = bsp::ship_ai_path_arrival_009da590(
            ctl_.goal.flag_2fe, goal_x, goal_z, live.latched_goal_x, live.latched_goal_z);
        owner_.done("ShipAiMoveTo::goal_reached_009da590", 0x009da590u);
        if (arrival.clears_latch) ctl_.goal.flag_2fe = false;  // 009DA5F6
        // The latch itself is raised only at 009EF034, inside the navigation
        // arm's tail. Milestone 2r fills its input: 009EEF14's release test is
        // blk+3D8h + setback < blk+330h, and blk+3D8h is now the start radius
        // 009E453F wrote, so the stop is released only while the remaining
        // path is longer than it.
        return arrival.reached;
    }
    std::uint32_t director_current_command_0054() override {
        owner_.done("ShipAiMoveTo::director_current_command", 0x009e5831u);
        return owner_.units.director_current_command_0071be40(index_);
    }
    std::uint32_t resolve_command_target_00521ea0() override {
        // 009E5847 on director+58h. Milestone 2p: 0071EB60 answers with that
        // same descriptor and 00521EA0 resolves it, so the arm no longer
        // short-circuits on the null at 009E584E.
        bsp::SceneCommandTarget descriptor{};
        int mode = 0;
        const bool real = owner_.units.active_command_descriptor_0071eb60(index_, descriptor,
            mode);
        owner_.done("ShipAiMoveTo::resolve_command_target", 0x00521ea0u);
        if (!real) return 0u;
        return owner_.units.resolve_command_target_00521ea0(descriptor);
    }
    bool target_is_kind_vtable_005c(std::uint32_t target, int kind) override {
        // 009E585F with the literal 1Ch, through the recovered chain 006FE530.
        owner_.done("ShipAiMoveTo::target_is_kind", 0x009e585fu);
        if (target == 0u) return false;
        return owner_.units.unit_is_kind_of(static_cast<std::size_t>(target - 1u), kind);
    }
    bool target_pose_valid_00c8(std::uint32_t target) override {
        owner_.done("ShipAiMoveTo::target_pose_valid", 0x009e5869u);
        if (target == 0u) return true;
        return owner_.units.unit_pose_valid_00c8(static_cast<std::size_t>(target - 1u));
    }
    void refresh_target_pose_00414db0(std::uint32_t) override {
        owner_.record("ShipAiMoveTo::refresh_target_pose", 0x00414db0u);
    }
    void target_position_xz_00fc(std::uint32_t target, float& x, float& z) override {
        float y = 0.0f;
        x = 0.0f;
        z = 0.0f;
        if (target == 0u) return;
        owner_.done("ShipAiMoveTo::target_position", 0x009e5879u);
        owner_.units.unit_position_00fc(static_cast<std::size_t>(target - 1u), x, y, z);
    }
    int target_range_07a0(std::uint32_t) override {
        owner_.record("ShipAiMoveTo::target_range_07a0", 0x009e58ceu);
        return 0;
    }
    float unit_radius_09c8() override {
        owner_.record("ShipAiMoveTo::unit_radius_09c8", 0x009e58d4u);
        return 0.0f;
    }
    float planar_length_00414c60(float dx, float dz) override {
        owner_.done("ShipAiMoveTo::planar_length", 0x009e58b9u);
        return bsp::length_2d_00414c60(std::array<float, 2>{dx, dz});
    }
    void message_text_assign_0041e870(const char*) override {
        owner_.record("ShipAiMoveTo::message_text_assign", 0x0041e870u);
    }
    void post_command_message_00984300(std::uint32_t command) override {
        // 009E595C, 00984300 on [00F8A0C4] with the `finished` literal the
        // step assigned at 009E58EF.
        const std::size_t callbacks = owner_.units.report_command_event_00984300(
            index_, command, bsp::kCommandEventStatusFinished);
        owner_.done("ShipAiMoveTo::post_command_message", 0x00984300u);
        ++row_.command_events;
        ++owner_.summary.command_events;
        owner_.summary.command_event_callbacks += callbacks;
    }
    void release_message_text_00419cc0() override {
        owner_.record("ShipAiMoveTo::release_message_text", 0x00419cc0u);
    }
    void end_command_0071e430(std::uint32_t command, int flag) override {
        // 009E5997, 0071E430(director, 00E08F68, 1). The whole round trip is in
        // the commands host: the stage ladder, the 5Dh message, this process's
        // own receipt of it and the queue advance.
        const GameCommandCompletion done
            = owner_.units.end_command_0071e430(index_, command, flag != 0);
        owner_.done("ShipAiMoveTo::end_command", 0x0071e430u);
        ++row_.command_endings;
        ++owner_.summary.command_endings;
        if (done.queue_advanced) {
            ++row_.command_completions;
            ++owner_.summary.command_completions;
        }
    }
    void hold_heading_and_stop_009e00a0() override {
        HeadingHoldBinding hold(owner_, ctl_, index_);
        bsp::ship_ai_hold_heading_and_stop_009e00a0(ctl_.blk, hold);
        owner_.done("ShipAiMoveTo::hold_heading_and_stop", 0x009e00a0u);
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// Milestone 2p: 009E85B0, the approach-to-engage gate, over the real goal.
class EngageGateBinding final : public bsp::ShipAiAttackMoveEngageGateHost {
public:
    EngageGateBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                      std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}
    std::uint32_t brain_unit_0aa8() override {
        owner_.done("ShipAiEngageGate::brain_unit", 0x009e85b9u);
        return static_cast<std::uint32_t>(index_) + 1u;
    }
    void armament_readiness_0510(float& a, float& b) override {
        // 009E85CD, [[unit+538h]+510h] and +514h. The object at unit+538h has
        // no recovered class and neither field has a producer anywhere, so the
        // pair is recorded and left at the zero a fresh object carries, which
        // is the arm that fails the gate.
        owner_.record("ShipAiEngageGate::armament_readiness", 0x009e85cdu);
        a = 0.0f;
        b = 0.0f;
    }
    bool unit_pose_valid_00c8() override {
        owner_.done("ShipAiEngageGate::unit_pose_valid", 0x009e85f5u);
        return owner_.units.unit_pose_valid_00c8(index_);
    }
    void refresh_unit_pose_00414db0() override {
        owner_.record("ShipAiEngageGate::refresh_unit_pose", 0x00414db0u);
    }
    void unit_position_xz_00fc(float& x, float& z) override {
        float y = 0.0f;
        owner_.done("ShipAiEngageGate::unit_position", 0x009e8605u);
        owner_.units.unit_position_00fc(index_, x, y, z);
    }
    void brain_destination_0b2c(float& x, float& z) override {
        // 009E8610 / 009E862C, the goal vector 009F1420 now writes.
        owner_.done("ShipAiEngageGate::brain_destination", 0x009e8610u);
        x = ctl_.goal_vector.goal_x_0b2c;
        z = ctl_.goal_vector.goal_z_0b34;
    }
    std::uint32_t avoid_zone_containing_004178f0(float, float) override {
        // 009E864C 0082ADC0 then 009E8658 004178F0. The avoid-zone singleton
        // 004218E0 hands out is not built in this process, so the list is empty
        // and the walk finds nothing, which is the arm that passes the gate.
        owner_.record("ShipAiEngageGate::avoid_zone_list", 0x0082adc0u);
        return 0u;
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

class AttackMoveSelectorBinding final : public bsp::ShipAiAttackMoveSelectorHost {
public:
    AttackMoveSelectorBinding(GameShipAiHost::Impl& owner,
                              GameShipAiHost::Impl::Controller& ctl, std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}

    std::uint32_t brain_attack_target_0b20() override {
        // 009E8714, [brain+0B20h]. Milestone 2p: 009F1420's head writes this
        // field from the command's own resolved target, so the selector no
        // longer takes the no-target arm for a ship whose `attackmove` names an
        // entity. The automatic target selector 009F5DA0 is a different
        // producer and still does not reach 00835860.
        owner_.done("ShipAiAttack::brain_target_0b20", 0x009e8714u);
        return ctl_.goal_vector.raw_target_0b20;
    }
    bool target_is_kind_vtable_005c(std::uint32_t target, int kind) override {
        // 009E8733 and 009E8799, with the literals 8 and 1Ch, answered through
        // the recovered class chain 006FE530.
        owner_.done("ShipAiAttack::target_is_kind", 0x009e8733u);
        if (target == 0u) return false;
        return owner_.units.unit_is_kind_of(static_cast<std::size_t>(target - 1u), kind);
    }
    bool call_00852860(std::uint32_t) override {
        // 009E873B. The routine's arithmetic is projected, but its two inputs
        // at entity+1200h and +1204h have no producer in the ledger, so the
        // call is a record; it is unreachable here anyway, because no target of
        // this mission answers the kind-8 test above.
        owner_.record("ShipAiAttack::call_00852860", 0x00852860u);
        return false;
    }
    bool brain_flag_0b28() override {
        // 009E8747, brain+0B28h, the goal refresh gate 009F1420 maintains.
        owner_.done("ShipAiAttack::brain_flag_0b28", 0x009e8747u);
        return ctl_.goal_vector.target_visible_0b28;
    }
    void set_current_substate_007b6ee0(std::uint32_t member) override {
        // 007B6EE0's body was read by packet ship_ai_state_steps: it returns at
        // once when the machine already holds the member, otherwise exits the
        // old one and enters the new one. The reconstruction of the selector
        // calls this method only on a real change.
        owner_.done("ShipAiAttack::set_current_substate", 0x007b6ee0u);
        ctl_.selector.current_1508 = member;
    }
    bool call_009e85b0() override {
        EngageGateBinding gate(owner_, ctl_, index_);
        const bool open = bsp::ship_ai_attackmove_engage_gate_009e85b0(gate);
        owner_.done("ShipAiAttack::call_009e85b0", 0x009e85b0u);
        return open;
    }
    void substate_exit_vtable_0008(std::uint32_t) override {
        owner_.record_slot("ShipAiAttack::substate_exit", "00d21994+vtable08");
    }
    void substate_enter_vtable_0004(std::uint32_t) override {
        owner_.record_slot("ShipAiAttack::substate_enter", "00d21994+vtable04");
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// Milestone 2p: 009F3240, the approach sub-state step at state+8h
// ---------------------------------------------------------------------------
//
// The one sub-state the selector reaches on this mission: every `attackmove`
// here names a surface entity, which answers neither the kind-8 test at
// 009E8733 nor the kind-1Ch test at 009E8799, so 009E87B6 puts the machine on
// state+8h and the engage gate above keeps it there. The body is projected
// whole by packet cc_ai_attackmove_substates; its approach point comes from the
// nested update 009F3090, which no packet has read, so that one step is a
// record and the four fields it would fill stay at the zeroes 009E5530 seeds.

// bsp::ShipAiApproachPointHost, the call sites inside 009F1BC0's frame state.
// Packet ship_ai_approach_update landed on main at 89d4bb77 during this
// packet's turn and was merged in before validation.
class ApproachPointBinding final : public bsp::ShipAiApproachPointHost {
public:
    ApproachPointBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                         std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}

    float unit_heading_vtable_0050() override {
        owner_.done("ShipAiApproachPoint::unit_heading", 0x009f1c24u);
        return owner_.units.unit_heading_radians(index_);
    }
    void refresh_unit_pose_00414db0() override {
        owner_.record("ShipAiApproachPoint::refresh_unit_pose", 0x00414db0u);
    }
    bsp::ShipAiApproachPoint unit_world_position() override {
        bsp::ShipAiApproachPoint out{};
        owner_.done("ShipAiApproachPoint::unit_position", 0x009f1c45u);
        owner_.units.unit_position_00fc(index_, out.x, out.y, out.z);
        return out;
    }
    bsp::ShipAiApproachPoint brain_goal_0b2c() override {
        // 009F1C6A, brain+0B2Ch / +0B30h / +0B34h: the goal vector 009F1420
        // writes. This is the whole reason the approach point is real now.
        bsp::ShipAiApproachPoint out{};
        owner_.done("ShipAiApproachPoint::brain_goal", 0x009f1c6au);
        out.x = ctl_.goal_vector.goal_x_0b2c;
        out.y = ctl_.goal_vector.goal_y_0b30;
        out.z = ctl_.goal_vector.goal_z_0b34;
        return out;
    }
    bool unit_is_kind_vtable_005c(int kind) override {
        owner_.done("ShipAiApproachPoint::unit_is_kind", 0x009f1d0fu);
        return owner_.units.unit_is_kind_of(index_, kind);
    }
    float shipclass_radius_0500() override {
        owner_.done("ShipAiApproachPoint::shipclass_radius", 0x009f1d1eu);
        return owner_.units.unit_class_max_speed_0500(index_);
    }
    float unit_turn_radius_00811a30(float) override {
        // 009F1D3C, 00811A30 with ECX = unit and the literal 1.0: the turn
        // radius at full helm. Body unread by every packet.
        owner_.record("ShipAiApproachPoint::unit_turn_radius", 0x00811a30u);
        return 0.0f;
    }
    float random_stream1_00bd2f10(float low, float) override {
        // 009F1DB4, the retarget timer's reseed in [2, 3). 00BD2F10 was not
        // read; the low end is taken and recorded, which makes the timer
        // deterministic rather than staggered and says so.
        owner_.record("ShipAiApproachPoint::random_stream1", 0x00bd2f10u);
        return low;
    }
    int target_zone_group_vtable_002c() override {
        owner_.record_slot("ShipAiApproachPoint::target_zone_group", "00cfc3d0+vtable2c");
        return 0;
    }
    int unit_zone_group_0570() override {
        owner_.record("ShipAiApproachPoint::unit_zone_group", 0x009f1e55u);
        return 0;
    }
    float unit_avoid_radius_0082adc0() override {
        owner_.record("ShipAiApproachPoint::unit_avoid_radius", 0x0082adc0u);
        return 0.0f;
    }
    bsp::ShipAiAttackMoveXZ zone_exit_point_00417b10(const bsp::ShipAiApproachPoint& from,
                                                     float) override {
        owner_.record("ShipAiApproachPoint::zone_exit_point", 0x00417b10u);
        return bsp::ShipAiAttackMoveXZ{from.x, from.z};
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// Milestone 2r: the six arms of 009F3090, and the four slot scorers behind them
// ---------------------------------------------------------------------------
// Every method below is one native call site. The ones that need a weapon
// inventory, a zone object or a traffic list answer "there is none", which is
// this process's own state and not a substitute: no gunnery device is built,
// construct_world 004DE610 is a load record and the avoid-zone manager has no
// producer. Their addresses are recorded so a reader can tell a produced value
// from an absent one.

// 0095EB40, the expected-damage rating 009E5DA0 asks for per slot.
class FirepowerBinding final : public bsp::ShipAiFirepowerHost {
public:
    FirepowerBinding(GameShipAiHost::Impl& owner, std::size_t index)
        : owner_(owner), index_(index) {}
    float unit_max_weapon_range() override {
        owner_.record("ShipAiFirepower::unit_max_weapon_range_0494", 0x0095eb62u);
        return 0.0f;
    }
    int category_device_count(int) override {
        // 0095EBB3, [unit+394h + category*0Ch]. This process builds no gunnery
        // device list, so every category is empty and the walk never starts.
        owner_.record("ShipAiFirepower::category_device_count", 0x0095ebb3u);
        return 0;
    }
    float category_max_range(int) override {
        owner_.record("ShipAiFirepower::category_max_range", 0x0095ebc4u);
        return 0.0f;
    }
    bsp::NativeHandle category_list_head(int) override {
        owner_.record("ShipAiFirepower::category_list_head", 0x0095ec24u);
        return 0;
    }
    bsp::NativeHandle list_next(bsp::NativeHandle) override { return 0; }
    bsp::NativeHandle list_device(bsp::NativeHandle) override { return 0; }
    bool device_is_turning_gun(bsp::NativeHandle) override { return false; }
    bool device_is_operational(bsp::NativeHandle) override { return false; }
    int device_ready_rounds(bsp::NativeHandle, float) override { return 0; }
    bool device_is_destroyed(bsp::NativeHandle) override { return true; }
    int device_barrel_count(bsp::NativeHandle) override { return 0; }
    int device_weapon_function(bsp::NativeHandle) override { return 0; }
    bsp::NativeHandle device_ammo_record(bsp::NativeHandle) override { return 0; }
    void ammo_select_flak_alternate(bsp::NativeHandle) override {}
    bsp::ShipAiFirepowerProjectileClass ammo_projectile_class(bsp::NativeHandle) override {
        return bsp::ShipAiFirepowerProjectileClass{};
    }
    float ammo_cycle_period(bsp::NativeHandle) override { return 0.0f; }
    float weapon_hit_probability(bsp::NativeHandle, float, float) override { return 0.0f; }
    bool device_can_bear(bsp::NativeHandle, bsp::NativeHandle, float, float) override {
        return false;
    }
    bsp::ShipAiFirepowerTickDamage gameplay_tick_damage() override {
        owner_.record("ShipAiFirepower::gameplay_tick_damage_00424c40", 0x0095eeadu);
        return bsp::ShipAiFirepowerTickDamage{};
    }

private:
    GameShipAiHost::Impl& owner_;
    std::size_t index_;
};

// 009E5DA0's own host: the one call it makes, into 0095EB40.
class RingScanClassScoreBinding final : public bsp::ShipAiRingScanClassScoreHost {
public:
    RingScanClassScoreBinding(GameShipAiHost::Impl& owner, GameShipAiRow& row,
                              std::size_t index)
        : owner_(owner), row_(row), index_(index) {}
    float rate_bearing_0095eb40(const bsp::ShipAiRingScanClassQuery& query) override {
        bsp::ShipAiFirepowerQuery q{};
        static_assert(sizeof(q) == sizeof(query.word), "the block is 17 dwords");
        std::memcpy(&q, query.word, sizeof(q));
        FirepowerBinding firepower(owner_, index_);
        const bsp::ShipAiFirepowerResult result
            = bsp::ship_ai_firepower_rating_0095eb40(q, firepower);
        owner_.done("ShipAiApproach::rate_bearing_0095eb40", 0x009e5dc4u);
        ++row_.firepower_ratings;
        ++owner_.summary.firepower_ratings;
        return result.total;
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// 009E6640's own host, the obstacle probe behind the accept/reject test.
class RingScanProbeBinding final : public bsp::ShipAiRingScanHost {
public:
    RingScanProbeBinding(GameShipAiHost::Impl& owner,
                         GameShipAiHost::Impl::Controller& ctl, std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}
    float wrap_phase_00605070(float value) override {
        return bsp::wrapped_angle_add_00438aa0(value, 0.0f);
    }
    std::uint32_t probe_space_vtable_0218() override {
        owner_.record("ShipAiRingScan::probe_space_vtable_0218", 0x009e66ebu);
        return 0u;
    }
    bool unit_pose_fresh_00c8() override {
        return owner_.units.unit_pose_valid_00c8(index_);
    }
    void refresh_unit_pose_00414db0() override {
        owner_.record("ShipAiRingScan::refresh_unit_pose", 0x00414db0u);
    }
    bsp::ShipAiAttackMoveXZ unit_world_xz() override {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        owner_.units.unit_position_00fc(index_, x, y, z);
        owner_.done("ShipAiRingScan::unit_world_xz", 0x009e6710u);
        bsp::ShipAiAttackMoveXZ out{};
        out.x = x;
        out.z = z;
        return out;
    }
    bsp::ShipAiAttackMoveXZ probe_origin_00417b10(std::uint32_t,
                                                  const bsp::ShipAiAttackMoveXZ& point,
                                                  float, int) override {
        // 009E673E, 00417B10 on the probe space. No space exists here, so the
        // start point is the query point itself.
        owner_.record("ShipAiRingScan::probe_origin_00417b10", 0x00417b10u);
        return point;
    }
    bool probe_hit_0041b4e0(std::uint32_t, const bsp::ShipAiAttackMoveXZ&,
                            const bsp::ShipAiAttackMoveXZ&,
                            bsp::ShipAiAttackMoveXZ&) override {
        owner_.record("ShipAiRingScan::probe_hit_0041b4e0", 0x0041b4e0u);
        return false;
    }
    float planar_length_00414c60(const bsp::ShipAiAttackMoveXZ& delta) override {
        return bsp::length_2d_00414c60(std::array<float, 2>{delta.x, delta.z});
    }
    float tune_reject_penalty_04() override {
        owner_.record("ShipAiRingScan::tune_reject_penalty_04", 0x009e784bu);
        return 0.0f;
    }
    void rebuild_unit_world_matrix() override {
        owner_.record("ShipAiRingScan::rebuild_unit_world_matrix", 0x009e7cadu);
    }
    bsp::ShipAiAttackMoveXZ brain_goal_0b2c() override {
        bsp::ShipAiAttackMoveXZ out{};
        out.x = ctl_.goal_vector.goal_x_0b2c;
        out.z = ctl_.goal_vector.goal_z_0b34;
        owner_.done("ShipAiRingScan::brain_goal_0b2c", 0x009e7d7au);
        return out;
    }
    float unit_cruise_speed_0490() override {
        owner_.record("ShipAiRingScan::unit_cruise_speed_0490", 0x009e7de7u);
        return 0.0f;
    }
    void commit_bearing_009e5e90(float, float) override {}

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

// 009E7FC0.
class ScoreResetBinding final : public bsp::ShipAiApproachScoreResetHost {
public:
    ScoreResetBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                      GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}
    void decay_slot_009e6400(int slot, float radius) override {
        // 009E8080, the bearing-decay scorer. Packet cc_ai_ring_scan read it
        // whole; the bearing it decays against is nested+1290h.
        if (slot < 0 || slot >= bsp::kShipAiApproachSlotCount) return;
        bsp::ship_ai_ring_scan_decay_slot_009e6400(ctl_.approach_scores[slot], radius,
            ctl_.approach_ring[slot].angle_08);
        owner_.done("ShipAiApproach::decay_slot_009e6400", 0x009e6400u);
    }
    void target_kind_probe_vtable_005c(int) override {
        owner_.record("ShipAiApproach::target_kind_probe_005c", 0x009e80abu);
    }
    std::uint32_t brain_target_0b20() override { return ctl_.goal_vector.raw_target_0b20; }
    bool brain_flag_0b28() override {
        owner_.record("ShipAiApproach::brain_flag_0b28", 0x009e80b0u);
        return false;
    }
    float nested_reference_127c() override { return ctl_.approach.avoid_radius_1290; }
    float unit_lookahead_0494() override {
        owner_.record("ShipAiApproach::unit_lookahead_0494", 0x009e80cdu);
        return 0.0f;
    }
    bool zone_allows_target_00864fd0(std::uint32_t) override {
        owner_.record("ShipAiApproach::zone_allows_target_00864fd0", 0x00864fd0u);
        return false;
    }
    bsp::ShipAiApproachPoint probe_point_009e6120() override {
        owner_.record("ShipAiApproach::probe_point_009e6120", 0x009e6120u);
        return bsp::ShipAiApproachPoint{};
    }
    bool zone_allows_point_00864ba0(const bsp::ShipAiApproachPoint&) override {
        owner_.record("ShipAiApproach::zone_allows_point_00864ba0", 0x00864ba0u);
        return false;
    }
    float score_slot_009e5da0(int slot) override {
        // 009E81A7, the ship-class rating. Packet cc_ai_ring_scan read the
        // adapter whole and packet cc_ai_bearing_rating the 0095EB40 behind it.
        if (slot < 0 || slot >= bsp::kShipAiApproachSlotCount) return 0.0f;
        bsp::ShipAiRingScanClassQuery query{};
        RingScanClassScoreBinding score(owner_, row_, index_);
        bsp::ship_ai_ring_scan_class_score_009e5da0(ctl_.approach_ring[slot],
            ctl_.approach_scores[slot], query, score);
        owner_.done("ShipAiApproach::score_slot_009e5da0", 0x009e5da0u);
        return ctl_.approach_scores[slot].raw_18;
    }
    float tune_scale_00() override {
        owner_.record("ShipAiApproach::tune_scale_00", 0x009e81fau);
        return 0.0f;
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// 009E74D0.
class EvadeBinding final : public bsp::ShipAiApproachEvadeHost {
public:
    EvadeBinding(GameShipAiHost::Impl& owner, std::size_t index)
        : owner_(owner), index_(index) {}
    float unit_evade_flag_1128() override {
        owner_.record("ShipAiApproach::unit_evade_flag_1128", 0x009e751fu);
        return 0.0f;
    }
    float tune_bearing_10() override {
        owner_.record("ShipAiApproach::tune_bearing_10", 0x009e75f2u);
        return 0.0f;
    }
    float tune_evade_14() override { return 0.0f; }
    float tune_evade_span_18() override { return 0.0f; }

private:
    GameShipAiHost::Impl& owner_;
    std::size_t index_;
};

// 009E6E80.
class StandoffBinding final : public bsp::ShipAiApproachStandoffHost {
public:
    StandoffBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                    std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}
    void construct_scratch_00954940() override {
        owner_.record("ShipAiApproach::scratch_00954940", 0x00954940u);
    }
    float tune_range_override_1c() override {
        owner_.record("ShipAiApproach::tune_range_override_1c", 0x009e6ec5u);
        return -1.0f;
    }
    bool target_is_kind_vtable_005c(int) override {
        owner_.record("ShipAiApproach::target_kind_005c", 0x009e6efcu);
        return false;
    }
    bool shipclass_allows_close_00827f70() override {
        owner_.record("ShipAiApproach::shipclass_allows_close_00827f70", 0x00827f70u);
        return false;
    }
    std::int32_t target_radius_07c4() override {
        owner_.record("ShipAiApproach::target_radius_07c4", 0x009e6f4eu);
        return 0;
    }
    std::int32_t target_gun_range_07a0() override {
        owner_.record("ShipAiApproach::target_gun_range_07a0", 0x009e706fu);
        return 0;
    }
    bool unit_is_group_leader_00778890() override {
        owner_.record("ShipAiApproach::unit_is_group_leader_00778890", 0x00778890u);
        return false;
    }
    float unit_gun_reference_09c8() override {
        return owner_.units.unit_hull_length_09c8(index_);
    }
    float unit_cruise_speed_0490() override {
        owner_.record("ShipAiApproach::unit_cruise_speed_0490", 0x009e7140u);
        return 0.0f;
    }
    float random_stream1_00bd2f10(float low, float) override {
        owner_.record("ShipAiApproach::random_stream1_00bd2f10", 0x00bd2f10u);
        return low;
    }
    float curve_base_00952530() override {
        owner_.record("ShipAiApproach::curve_base_00952530", 0x00952530u);
        return 0.0f;
    }
    float curve_reference_009523c0() override {
        owner_.record("ShipAiApproach::curve_reference_009523c0", 0x009523c0u);
        return 0.0f;
    }
    float curve_primary_00955a40(float) override {
        owner_.record("ShipAiApproach::curve_primary_00955a40", 0x00955a40u);
        return 0.0f;
    }
    float curve_secondary_00955a40(float) override { return 0.0f; }
    float nested_scan_scale_1284() override {
        owner_.record("ShipAiApproach::nested_scan_scale_1284", 0x009e7284u);
        return 0.0f;
    }
    float unit_turn_radius_00811a30(float rudder) override {
        return owner_.units.unit_class_turn_circle_radius_0082e960(index_, rudder);
    }
    int unit_clearance_count_0080df40() override {
        owner_.record("ShipAiApproach::unit_clearance_count_0080df40", 0x0080df40u);
        return 0;
    }
    void score_slot_009e6870(int slot, float side_weight, float span_weight,
                             float slot_scale, float tune_04) override {
        // 009E74B7, the standoff-arc score. Packet cc_ai_ring_scan read it whole.
        if (slot < 0 || slot >= bsp::kShipAiApproachSlotCount) return;
        bsp::ship_ai_ring_scan_arc_slot_009e6870(ctl_.approach_ring[slot],
            ctl_.approach_scores[slot], slot_scale, side_weight, span_weight, tune_04);
        owner_.done("ShipAiApproach::score_slot_009e6870", 0x009e6870u);
    }
    float tune_slot_04() override {
        owner_.record("ShipAiApproach::tune_slot_04", 0x009e7489u);
        return 0.0f;
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

// 009E9190.
class AvoidBinding final : public bsp::ShipAiApproachAvoidHost {
public:
    AvoidBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                 std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}
    float random_stream1_00bd2f10(float low, float) override {
        owner_.record("ShipAiApproach::avoid_random_00bd2f10", 0x00bd2f10u);
        return low;
    }
    int candidate_count_008053c0() override {
        // 009E9220, the entity list at [unit+54h]+0DE8h. construct_world
        // 004DE610 is a load record here, so the list does not exist.
        owner_.record("ShipAiApproach::candidate_count_008053c0", 0x008053c0u);
        return 0;
    }
    std::uint32_t candidate_at(int) override { return 0u; }
    bool candidate_is_kind_vtable_005c(std::uint32_t) override { return false; }
    std::uint32_t brain_target_0b20() override { return ctl_.goal_vector.raw_target_0b20; }
    void refresh_pose_00414db0(std::uint32_t) override {}
    bsp::ShipAiApproachPoint entity_world_position(std::uint32_t) override {
        return bsp::ShipAiApproachPoint{};
    }
    float entity_speed_0494(std::uint32_t) override { return 0.0f; }
    bsp::ShipAiApproachPoint unit_world_position() override {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        owner_.units.unit_position_00fc(index_, x, y, z);
        bsp::ShipAiApproachPoint out{};
        out.x = x;
        out.y = y;
        out.z = z;
        owner_.done("ShipAiApproach::avoid_unit_position", 0x009e93f3u);
        return out;
    }
    void insert_traffic_record(std::uint32_t) override {
        owner_.record("ShipAiApproach::insert_traffic_record_009e8360", 0x009e8360u);
    }
    int traffic_record_count() override { return 0; }
    bool traffic_record_active_009e6170(int, const bsp::ShipAiApproachPoint&,
                                        float) override {
        return false;
    }
    void erase_traffic_record(int) override {}
    void advance_traffic_record_009e6240(int, float,
                                         const bsp::ShipAiApproachPoint&) override {}
    float traffic_record_weight_0120(int) override { return 0.0f; }
    bsp::ShipAiApproachPoint traffic_record_direction_010c(int) override {
        return bsp::ShipAiApproachPoint{};
    }
    float vector_length_0042b2f0(const bsp::ShipAiApproachPoint& v) override {
        return bsp::length_2d_00414c60(std::array<float, 2>{v.x, v.z});
    }
    float tune_avoid_strength_08() override {
        owner_.record("ShipAiApproach::tune_avoid_strength_08", 0x009e964eu);
        return 0.0f;
    }
    float tune_avoid_span_0c() override { return 0.0f; }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

// 009E76D0.
class SelectBinding final : public bsp::ShipAiApproachSelectHost {
public:
    SelectBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                  GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}
    float wrap_angle_00605070(float value) override {
        return bsp::wrapped_angle_add_00438aa0(value, 0.0f);
    }
    float score_slot_009e6640(int slot, float seconds, float slot_scale, bool override_a,
                              bool override_b, float turn_radius) override {
        // 009E7755, the obstacle probe. Packet cc_ai_ring_scan read it whole.
        if (slot < 0 || slot >= bsp::kShipAiApproachSlotCount) return 0.0f;
        RingScanProbeBinding probe(owner_, ctl_, index_);
        const float score = bsp::ship_ai_ring_scan_probe_009e6640(ctl_.approach_ring[slot],
            ctl_.approach_scores[slot], seconds, slot_scale, override_a, override_b,
            turn_radius, probe);
        owner_.done("ShipAiApproach::score_slot_009e6640", 0x009e6640u);
        return score;
    }
    float tune_reject_penalty_04() override {
        owner_.record("ShipAiApproach::select_tune_reject_04", 0x009e784bu);
        return 0.0f;
    }
    void refresh_unit_pose() override {
        owner_.record("ShipAiApproach::select_refresh_pose", 0x009e7cb4u);
    }
    bsp::ShipAiAttackMoveXZ brain_goal_0b2c() override {
        bsp::ShipAiAttackMoveXZ out{};
        out.x = ctl_.goal_vector.goal_x_0b2c;
        out.z = ctl_.goal_vector.goal_z_0b34;
        owner_.done("ShipAiApproach::select_brain_goal_0b2c", 0x009e7d7au);
        return out;
    }
    bsp::ShipAiAttackMoveXZ unit_world_xz() override {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        owner_.units.unit_position_00fc(index_, x, y, z);
        bsp::ShipAiAttackMoveXZ out{};
        out.x = x;
        out.z = z;
        owner_.done("ShipAiApproach::select_unit_xz", 0x009e7d92u);
        return out;
    }
    float unit_cruise_speed_0490() override {
        owner_.record("ShipAiApproach::select_cruise_speed_0490", 0x009e7de7u);
        return 0.0f;
    }
    void commit_bearing_009e5e90(float bearing, float) override {
        // 009E7ECB. Packet cc_ai_approach_update read 009E5E90 whole; it is the
        // producer of the commanded heading at nested+120Ch.
        bool blocked[bsp::kShipAiApproachSlotCount]{};
        for (int i = 0; i < bsp::kShipAiApproachSlotCount; ++i) {
            blocked[i] = ctl_.approach_scores[i].blocked_40;
        }
        bsp::ship_ai_approach_commit_bearing_009e5e90(ctl_.approach, blocked, bearing);
        owner_.done("ShipAiApproach::commit_bearing_009e5e90", 0x009e5e90u);
        ++row_.ring_scan_bearings;
        ++owner_.summary.ring_scan_bearings;
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// 009E6A90.
class ThrottleLimitBinding final : public bsp::ShipAiApproachThrottleHost {
public:
    ThrottleLimitBinding(GameShipAiHost::Impl& owner,
                         GameShipAiHost::Impl::Controller& ctl, std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}
    float unit_heading_vtable_0050() override {
        owner_.done("ShipAiApproach::limit_unit_heading", 0x009e6ab5u);
        return owner_.units.unit_heading_radians(index_);
    }
    float unit_evade_flag_1128() override {
        owner_.record("ShipAiApproach::limit_evade_flag_1128", 0x009e6b46u);
        return 0.0f;
    }
    std::uint32_t engagement_target_009e5e00() override {
        owner_.done("ShipAiApproach::engagement_target_009e5e00", 0x009e5e00u);
        return ctl_.goal_vector.raw_target_0b20;
    }
    std::int32_t target_radius_07c4(std::uint32_t) override {
        owner_.record("ShipAiApproach::limit_target_radius_07c4", 0x009e6b90u);
        return 0;
    }
    bool unit_is_group_leader_00778890() override {
        owner_.record("ShipAiApproach::limit_group_leader_00778890", 0x00778890u);
        return false;
    }
    bool target_accepted_vtable_0234(std::uint32_t) override {
        owner_.record("ShipAiApproach::limit_target_accepted_0234", 0x009e6c86u);
        return false;
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

// bsp::ShipAiApproachUpdateHost, the seven calls of 009F3090 in their fixed
// order. Only the first is run: it is the one that produces the approach point
// and the goal range, and the other six need the 60-slot ring the four unread
// scorers 009E6400, 009E5DA0, 009E6870 and 009E6640 fill.
class ApproachUpdateBinding final : public bsp::ShipAiApproachUpdateHost {
public:
    ApproachUpdateBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                          GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}

    void frame_state_009f1bc0(float seconds) override {
        // 009F309B. The projection covers 009F1BC0-009F1DBF and
        // 009F1E60-009F1F47: the frame timers, the planar range to the
        // attackmove destination, the turn radius, the retarget timer and the
        // approach point itself, which is that destination copied verbatim
        // unless the target carries a zone object.
        ApproachPointBinding point(owner_, ctl_, index_);
        const bool has_target = ctl_.goal_vector.raw_target_0b20 != 0u;
        // 009F1E36, [target+740h]: the target's own zone object. No producer in
        // this process, so the displacement arm at 009F1E94 is never taken.
        owner_.record("ShipAiApproach::target_zone_object_0740", 0x009f1e36u);
        bsp::ship_ai_approach_frame_state_009f1bc0(ctl_.approach, has_target, false,
                                                   seconds, point);
        owner_.done("ShipAiApproach::frame_state", 0x009f1bc0u);
        owner_.record("ShipAiApproach::frame_state_unread_spans", 0x009f1dbfu);
        ++row_.approach_frames;
        ++owner_.summary.approach_frames;
        row_.approach_point_x = ctl_.approach.point_1228.x;
        row_.approach_point_z = ctl_.approach.point_1228.z;
        row_.approach_goal_range = ctl_.approach.goal_range_11e0;
    }
    // Milestone 2r. The six arms milestone 2q recorded now run: packets
    // cc_ai_ring_scan, cc_ai_approach_update and cc_ai_bearing_rating between
    // them cover 009E7FC0, 009E6E80, 009E9190, 009E74D0, 009E76D0 with the four
    // slot scorers 009E6400 / 009E5DA0 / 009E6870 / 009E6640, the commit
    // 009E5E90 and the throttle limiter 009E6A90.
    void reset_scores_009e7fc0() override {
        owner_.ensure_approach_ring(ctl_, index_);
        ScoreResetBinding reset(owner_, ctl_, row_, index_);
        bsp::ship_ai_approach_reset_scores_009e7fc0(ctl_.approach, ctl_.approach_scores,
            ctl_.goal_vector.raw_target_0b20 != 0u, reset);
        owner_.done("ShipAiApproach::reset_scores", 0x009e7fc0u);
    }
    void choose_standoff_range_009e6e80() override {
        StandoffBinding standoff(owner_, ctl_, index_);
        bsp::ship_ai_approach_choose_standoff_009e6e80(ctl_.approach,
            ctl_.goal_vector.raw_target_0b20 != 0u, standoff);
        owner_.done("ShipAiApproach::choose_standoff_range", 0x009e6e80u);
    }
    void refresh_avoidance_009e9190(float seconds) override {
        AvoidBinding avoid(owner_, ctl_, index_);
        bsp::ship_ai_approach_refresh_avoidance_009e9190(ctl_.approach, ctl_.approach_ring,
            ctl_.approach_scores, seconds, avoid);
        owner_.done("ShipAiApproach::refresh_avoidance", 0x009e9190u);
    }
    void score_evade_009e74d0(float seconds) override {
        EvadeBinding evade(owner_, index_);
        bsp::ship_ai_approach_score_evade_009e74d0(ctl_.approach, ctl_.approach_ring,
            ctl_.approach_scores, seconds, evade);
        owner_.done("ShipAiApproach::score_evade", 0x009e74d0u);
    }
    void select_slot_009e76d0(float seconds) override {
        SelectBinding select(owner_, ctl_, row_, index_);
        bsp::ship_ai_approach_select_slot_009e76d0(ctl_.approach, ctl_.approach_ring,
            ctl_.approach_scores, seconds, select);
        owner_.done("ShipAiApproach::select_slot", 0x009e76d0u);
        ++row_.ring_scans;
        ++owner_.summary.ring_scans;
        row_.ring_scan_winner = ctl_.approach.committed_slot_11e8;
        row_.approach_heading_120c = ctl_.approach.commanded_heading_120c;
    }
    void limit_throttle_009e6a90() override {
        // 009E6A90's first act is wrap(heading - nested+120Ch), and nested+120Ch
        // is now what 009E5E90 wrote behind the ring scan above rather than the
        // 0.0f the constructor leaves, so the limiter runs on a produced input.
        ThrottleLimitBinding limit(owner_, ctl_, index_);
        bsp::ship_ai_approach_limit_throttle_009e6a90(ctl_.approach, limit);
        owner_.done("ShipAiApproach::limit_throttle", 0x009e6a90u);
        row_.approach_throttle_1210 = ctl_.approach.commanded_throttle_1210;
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

class ApproachStepBinding final : public bsp::ShipAiAttackMoveApproachHost {
public:
    ApproachStepBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                        GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}

    std::uint32_t brain_target_0b20() override {
        owner_.done("ShipAiApproach::brain_target_0b20", 0x009f3262u);
        return ctl_.goal_vector.raw_target_0b20;
    }
    bool target_retired_005d(std::uint32_t) override {
        // 009F3277, the byte at target+5Dh. bsp/unit_instance.hpp names unit+5Dh
        // `simulate` and milestone 2i holds it clear for a live ship.
        owner_.done("ShipAiApproach::target_retired_005d", 0x009f3277u);
        return false;
    }
    void hold_heading_and_stop_009e00a0() override {
        HeadingHoldBinding hold(owner_, ctl_, index_);
        bsp::ship_ai_hold_heading_and_stop_009e00a0(ctl_.blk, hold);
        owner_.done("ShipAiApproach::hold_heading_and_stop", 0x009e00a0u);
    }
    void nested_update_009f3090(float seconds) override {
        // 009F328F, with ECX = sub+8h (009F3289), so the nested object's
        // offsets are the sub-state's minus 8. Packet ship_ai_approach_update
        // projected the driver and six of its seven callees; the executable
        // runs the driver and the frame state that produces the approach point,
        // and records the rest with their own addresses.
        ApproachUpdateBinding update(owner_, ctl_, row_, index_);
        bsp::ship_ai_approach_update_009f3090(update, seconds);
        owner_.done("ShipAiApproach::nested_update", 0x009f3090u);
    }
    float unit_depth_reference_0494() override {
        // 009F32A0 / 009F32A6, [unit+494h]. No producer in this process.
        owner_.record("ShipAiApproach::unit_depth_reference", 0x009f32a0u);
        return 0.0f;
    }
    float sub_throttle_bias_11e8() override {
        // 009F3294, sub+11E8h = nested+11E0h, the planar range from the unit to
        // the attackmove destination that 009F1BC0 rewrites every frame at
        // 009F1CDE. A recovered value now.
        owner_.done("ShipAiApproach::sub_throttle_bias", 0x009f3294u);
        return ctl_.approach.goal_range_11e0;
    }
    void sub_goal_1230(float& x, float& z) override {
        // 009F3300 / 009F32EB, sub+1230h and sub+1238h = nested+1228h/+1230h,
        // the approach point. 009F1F2D..009F1F3D copies the attackmove
        // destination into it verbatim unless the target carries a zone object.
        owner_.done("ShipAiApproach::sub_goal_1230", 0x009f3300u);
        x = ctl_.approach.point_1228.x;
        z = ctl_.approach.point_1228.z;
    }
    float sub_heading_command_1214() override {
        // 009F3314, sub+1214h = nested+120Ch, written by 009E5E90 behind the
        // recorded ring scan 009E76D0.
        owner_.record("ShipAiApproach::sub_heading_command", 0x009f3314u);
        return ctl_.approach.commanded_heading_120c;
    }
    float sub_throttle_command_1218() override {
        // 009F339A, sub+1218h = nested+1210h. 009F1BF7 seeds this field with
        // the 9999.0f sentinel at 00CE4C04 on every frame and 009E6A90 is the
        // only routine that replaces it. 009E6A90 is recorded here, so the
        // sentinel is still in the field, and 009F3635's clamp to [-1, +1]
        // would turn it into full ahead - a number that looks like an order and
        // is only the marker for "the producer has not run". The read is
        // recorded and the neutral zero is used instead, which is what every
        // other unproduced value in this file answers with.
        owner_.record("ShipAiApproach::sub_throttle_command", 0x009f339au);
        if (static_cast<double>(ctl_.approach.commanded_throttle_1210) > 1000.0) {
            return 0.0f;
        }
        return ctl_.approach.commanded_throttle_1210;
    }
    void set_navigation_goal_009de050(const bsp::ShipAiAttackMoveXZ& goal, int keep_mode,
                                      int final_leg) override {
        owner_.run_navigation_goal_009de050(ctl_, row_, index_, goal.x, goal.z,
                                            keep_mode != 0, final_leg != 0);
    }
    // The five brain displacements this step writes are the control block's
    // own fields, because blk is brain+8h: brain+1CCh is blk+1C4h the steering
    // mode, brain+1D0h is blk+1C8h the throttle hold, brain+1D4h is blk+1CCh
    // the requested direction, brain+1D8h is blk+1D0h the desired throttle and
    // brain+1E0h is blk+1D8h the desired heading - which is why 009F3360 wraps
    // it with the same 00605070 that 009E0040 uses on that field.
    int brain_steering_mode_01cc() override {
        return static_cast<int>(ctl_.blk.mode);
    }
    void clear_brain_turn_accumulators_0368() override {
        // 009F3335..009F3348, brain+370h and brain+368h, which are blk+368h and
        // blk+360h, the two timers 009ED6B0 counts down.
        ctl_.blk.timer_368 = 0.0f;
        ctl_.blk.timer_360 = 0.0f;
        owner_.done("ShipAiApproach::clear_turn_accumulators", 0x009f3348u);
    }
    void set_brain_heading_01e0(float heading) override {
        ctl_.blk.desired_heading = heading;
        owner_.done("ShipAiApproach::set_brain_heading", 0x009f335cu);
    }
    void wrap_brain_heading_00605070() override {
        // 009F3360, 00605070 on brain+1E0h in place. The reconstruction of that
        // wrap is bsp::wrap_angle_00605070 where it exists; the field it wraps
        // has no reader in this process, so the call is recorded.
        owner_.record("ShipAiApproach::wrap_brain_heading", 0x00605070u);
    }
    void set_brain_replan_01d4(int value) override {
        ctl_.blk.requested_direction = static_cast<bsp::ShipAiThrottleDirection>(value);
    }
    void set_brain_steering_mode_01cc(int mode) override {
        ctl_.blk.mode = static_cast<bsp::ShipAiSteeringMode>(mode);
    }
    void set_brain_goal_hold_01d0(int value) override {
        ctl_.blk.throttle_hold_1c8 = value;
    }
    void set_brain_throttle_0258(float throttle) override {
        // 009F337B and 009F3383, brain+258h and brain+2C0h. Neither field has a
        // reader in the recovered chain, so this is where the approach's own
        // throttle stops: it is NOT blk+1D0h, the desired throttle the ring
        // hop carries.
        brain_throttle_0258_ = throttle;
        owner_.record("ShipAiApproach::set_brain_throttle_0258", 0x009f337bu);
    }
    float sub_sweep_timer_14b4() override { return ctl_.substate_ring_timer_14b4; }
    void set_sub_sweep_timer_14b4(float seconds_left) override {
        ctl_.substate_ring_timer_14b4 = seconds_left;
    }
    bool target_is_kind_vtable_005c(std::uint32_t target, int kind) override {
        owner_.done("ShipAiApproach::target_is_kind", 0x009f33bfu);
        if (target == 0u) return false;
        return owner_.units.unit_is_kind_of(static_cast<std::size_t>(target - 1u), kind);
    }
    bool unit_and_target_share_side_0054(std::uint32_t target) override {
        owner_.done("ShipAiApproach::share_side", 0x009f3402u);
        if (target == 0u) return false;
        return owner_.units.unit_side_0054(index_)
            == owner_.units.unit_side_0054(static_cast<std::size_t>(target - 1u));
    }
    bool unit_is_group_leader_00778890() override {
        // 00778890's body was read: MOV EAX,[ECX+284h] then [group+14h] == unit.
        // No AI group object exists in this process (milestone 2m reports
        // ai_groups=0), so the group pointer is null and the answer is false,
        // which takes the non-leader arm at 009F35D8.
        owner_.record("ShipAiApproach::unit_is_group_leader", 0x00778890u);
        return false;
    }
    int group_member_count_04f8() override { return 0; }
    std::uint32_t group_member_at_0070d060(int) override {
        owner_.record("ShipAiApproach::group_member_at", 0x0070d060u);
        return 0u;
    }
    bool member_is_kind_vtable_005c(std::uint32_t, int) override { return false; }
    bool member_armament_ready_vtable_002c(std::uint32_t) override { return false; }
    bool unit_armament_ready_vtable_002c() override {
        // 009F35D8, [unit+538h]->vtable[2Ch](). The object at unit+538h has no
        // recovered class, so the predicate is a record and false, which leaves
        // the candidate list empty and skips the warn sweep.
        owner_.record("ShipAiApproach::unit_armament_ready", 0x009f35d8u);
        return false;
    }
    std::uint32_t brain_unit_0aa8() override {
        return static_cast<std::uint32_t>(index_) + 1u;
    }
    void target_position_xz_00427eb0(std::uint32_t target, float& x, float& z) override {
        float y = 0.0f;
        x = 0.0f;
        z = 0.0f;
        if (target == 0u) return;
        owner_.done("ShipAiApproach::target_position", 0x00427eb0u);
        owner_.units.unit_position_00fc(static_cast<std::size_t>(target - 1u), x, y, z);
    }
    float candidate_body_speed_0092d730(std::uint32_t candidate) override {
        if (candidate == 0u) return 0.0f;
        return owner_.units.unit_forward_speed_0092d730(
            static_cast<std::size_t>(candidate - 1u));
    }
    float candidate_reference_speed_0080fc30(std::uint32_t) override {
        owner_.record("ShipAiApproach::candidate_reference_speed", 0x0080fc30u);
        return 1.0f;
    }
    bool candidate_pose_valid_00c8(std::uint32_t candidate) override {
        if (candidate == 0u) return true;
        return owner_.units.unit_pose_valid_00c8(static_cast<std::size_t>(candidate - 1u));
    }
    void refresh_candidate_pose_00414db0(std::uint32_t) override {
        owner_.record("ShipAiApproach::refresh_candidate_pose", 0x00414db0u);
    }
    void candidate_position_xz_00fc(std::uint32_t candidate, float& x, float& z) override {
        float y = 0.0f;
        x = 0.0f;
        z = 0.0f;
        if (candidate == 0u) return;
        owner_.units.unit_position_00fc(static_cast<std::size_t>(candidate - 1u), x, y, z);
    }
    std::int32_t target_warn_radius_07c4(std::uint32_t) override {
        owner_.record("ShipAiApproach::target_warn_radius", 0x009f3534u);
        return 0;
    }
    bool candidate_accepts_warning_vtable_0234(std::uint32_t, std::uint32_t) override {
        owner_.record_slot("ShipAiApproach::candidate_accepts_warning", "00cfc3d0+vtable234");
        return false;
    }
    void route_warning_message_0077c2a0(std::uint32_t) override {
        owner_.record("ShipAiApproach::route_warning_message", 0x0077c2a0u);
    }
    void set_brain_command_01d8(float command) override {
        // 009F3635, brain+1D8h. That is blk+1D0h - brain+8h is blk, so
        // brain+1D8h is the block's desired throttle - and the value is the
        // clamped sub+1218h the nested update would have produced.
        ctl_.blk.desired_throttle = command;
        owner_.done("ShipAiApproach::set_brain_command_01d8", 0x009f3635u);
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
    float brain_throttle_0258_{0.0f};
};

class AttackMoveStepBinding final : public bsp::ShipAiAttackMoveStepHost {
public:
    AttackMoveStepBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                          GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}

    std::uint32_t brain_unit_0aa8() override {
        // 009E8828, [brain+0AA8h]. Every controller this process builds owns a
        // created instance, so the field is the unit and never zero; the handle
        // is this process's own index, one-based so 0 stays "no unit".
        owner_.done("ShipAiAttack::brain_unit_0aa8", 0x009e8828u);
        return static_cast<std::uint32_t>(index_) + 1u;
    }
    bool entity_is_kind_vtable_005c(std::uint32_t entity, int kind) override {
        // 009E883F and 009E888C, entity->vtable[5Ch](9), answered through the
        // recovered class chain 006FE530 this process already uses for the
        // automatic target scan. Only the owner's own handle can be resolved
        // here; a group member cannot, and no group exists.
        owner_.done("ShipAiAttack::entity_is_kind", 0x009e883fu);
        if (entity == 0u) return false;
        return owner_.units.unit_is_kind_of(static_cast<std::size_t>(entity - 1u), kind);
    }
    std::uint32_t unit_group_0284() override {
        // 009E8852, [unit+284h]. No AI group object exists in this process
        // (milestone 2m: ai_groups=0), so the null arm runs and the member walk
        // at 009E8867..009E889C is not reached.
        owner_.record("ShipAiAttack::unit_group_0284", 0x009e8852u);
        return 0u;
    }
    int group_member_count_04f8(std::uint32_t) override {
        owner_.record("ShipAiAttack::group_member_count", 0x009e8861u);
        return 0;
    }
    std::uint32_t group_member_at_0070d060(std::uint32_t, int) override {
        owner_.record("ShipAiAttack::group_member_at", 0x0070d060u);
        return 0u;
    }
    std::uint32_t unit_director_vtable_0114() override {
        owner_.record_slot("ShipAiAttack::unit_director", "00cfc3d0+vtable114");
        return 0u;
    }
    void end_command_0071e430(std::uint32_t, std::uint32_t command, int flag) override {
        // 009E88C1. The director argument is the unit's own vtable[114h]
        // accessor, which this process answers with the unit index, so the
        // completion runs on this unit's director.
        const GameCommandCompletion done
            = owner_.units.end_command_0071e430(index_, command, flag != 0);
        owner_.done("ShipAiAttack::end_command", 0x0071e430u);
        ++row_.command_endings;
        ++owner_.summary.command_endings;
        if (done.queue_advanced) {
            ++row_.command_completions;
            ++owner_.summary.command_completions;
        }
    }
    void select_substate_009e86f0(float seconds) override {
        AttackMoveSelectorBinding selector(owner_, ctl_, index_);
        bsp::ship_ai_attackmove_select_009e86f0(ctl_.selector, kAttackMoveStateBase, seconds,
                                                selector);
        owner_.done("ShipAiAttack::select_substate", 0x009e86f0u);
        row_.substate = ctl_.selector.current_1508 - kAttackMoveStateBase;
    }
    void substate_step_vtable_000c(float seconds) override {
        // 009E88F0, [state+1508h]->vtable[0Ch]. Milestone 2p dispatches on the
        // member the selector settled on, over the table
        // bsp::kShipAiAttackMoveSubStates that packet ship_ai_state_steps read
        // out of 009E8450. Packet cc_ai_attackmove_substates projected four of
        // the five bodies; the fifth, 007B3DD0, is one RET 4.
        ++row_.substate_steps;
        ++owner_.summary.substate_steps;
        const std::uint32_t offset = ctl_.selector.current_1508 - kAttackMoveStateBase;
        if (offset == 0x0008u) {
            ApproachStepBinding approach(owner_, ctl_, row_, index_);
            bsp::ship_ai_attackmove_approach_step_009f3240(seconds, approach);
            owner_.done("ShipAiAttack::approach_step", 0x009f3240u);
            ++row_.substate_concrete;
            ++owner_.summary.substate_concrete;
            return;
        }
        // The other four members are records with their own addresses. The
        // selector never reaches them on this mission: 009E8733's kind-8 test
        // and 009E8799's kind-1Ch test both answer false for a surface target,
        // which is the arm that pins the machine to state+8h, and the engage
        // gate 009E85B0 fails on the two unrecovered readiness floats.
        if (offset == 0x14C0u) {
            owner_.record("ShipAiAttack::engage_step", 0x009e23b0u);
        } else if (offset == 0x14CCu) {
            owner_.record("ShipAiAttack::lead_pursuit_step", 0x009e26c0u);
        } else if (offset == 0x14E0u) {
            owner_.record("ShipAiAttack::tangent_step", 0x009f3670u);
        } else if (offset == 0x14F4u) {
            // 007B3DD0's whole body is `C2 04 00`, one RET 4, COMDAT-folded
            // across twenty vtables. Running it is running nothing, so the
            // step is concrete and does nothing.
            owner_.done("ShipAiAttack::initial_step", 0x007b3dd0u);
            ++row_.substate_concrete;
            ++owner_.summary.substate_concrete;
        } else {
            owner_.record_slot("ShipAiAttack::substate_step_vtable0c", "00d21994+vtable0c");
        }
    }

private:
    // The attackmove state object's own base. This process holds no native
    // pointers, so the sub-state members are named by their offsets from an
    // arbitrary non-zero base; only their identity matters to the selector.
    static constexpr std::uint32_t kAttackMoveStateBase = 0x10000000u;

    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// bsp::ShipAiRudderLawHost, the one call site inside 009DA250
// ---------------------------------------------------------------------------

class RudderLawBinding final : public bsp::ShipAiRudderLawHost {
public:
    RudderLawBinding(GameShipAiHost::Impl& owner, std::size_t index)
        : owner_(owner), index_(index) {}
    float unit_body_axis_speed_0092d730() override {
        // 009DA2D7 with ECX = [unit+1018h] set at 009DA2D1: the hull's signed
        // forward speed, the same value the trajectory dump's fwd_speed carries.
        const float speed = owner_.units.unit_forward_speed_0092d730(index_);
        owner_.done("ShipAiRudder::body_axis_speed", 0x0092d730u);
        return speed;
    }

private:
    GameShipAiHost::Impl& owner_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// bsp::ShipAiRingHopHost, the three call sites of 009F4B99..009F4D04
// ---------------------------------------------------------------------------

class RingHopBinding final : public bsp::ShipAiRingHopHost {
public:
    RingHopBinding(GameShipAiHost::Impl& owner, GameShipAiRow& row, std::size_t index)
        : owner_(owner), row_(row), index_(index) {}

    float unit_body_axis_speed_0092d730() override {
        // 009F4BD4, reached only when the throttle is already inside the
        // 009F4BB9 deadband, which is what makes the zeroing arm a measurement.
        const float speed = owner_.units.unit_forward_speed_0092d730(index_);
        owner_.done("ShipAiRing::hop_body_axis_speed", 0x0092d730u);
        return speed;
    }
    void set_ring_write_slot_rudder_0080e190(float value) override {
        // 009F4CE8, 0080E190 with ECX = [blk+3FCh]: [unit + ([unit+97Ch]<<5) +
        // 83Ch]. Always the first of the two.
        owner_.units.unit_ring_set_write_slot_rudder_0080e190(index_, value);
        owner_.done("ShipAiRing::set_write_slot_rudder", 0x0080e190u);
        row_.ring_slot_rudder = value;
    }
    void set_ring_write_slot_throttle_0080e170(float value) override {
        // 009F4CFB, 0080E170: [unit + ([unit+97Ch]<<5) + 838h].
        owner_.units.unit_ring_set_write_slot_throttle_0080e170(index_, value);
        owner_.done("ShipAiRing::set_write_slot_throttle", 0x0080e170u);
        row_.ring_slot_throttle = value;
        ++row_.ring_writes;
        ++owner_.summary.ring_writes;
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// bsp::ShipAiDirectControlHost, the seven call sites of 009ED6B0's own arm
// ---------------------------------------------------------------------------

class DirectControlBinding final : public bsp::ShipAiDirectControlHost {
public:
    DirectControlBinding(GameShipAiHost::Impl& owner, std::size_t index)
        : owner_(owner), index_(index) {}

    void prologue_0080e000(float) override {
        owner_.record("ShipAiControls::prologue", 0x0080e000u);
    }
    bool controller_belongs_to_another_007788b0() override {
        owner_.record("ShipAiControls::controller_belongs_to_another", 0x007788b0u);
        return false;
    }
    float unit_body_axis_speed_0092d730() override {
        owner_.done("ShipAiControls::body_axis_speed", 0x0092d730u);
        return owner_.units.unit_forward_speed_0092d730(index_);
    }
    float ship_class_field_0508() override {
        // 009ED8E4/009ED8EC: FDIV [ECX+508h] with ECX = [unit+538h]. That field
        // is `Retardation` (docs/SHIP_CLASS_FIELDS.md, the loader at
        // 00831882..00831998, and bsp::ShipMotionClass::retardation), which this
        // process already reads out of the installed `VehicleClass` row, so the
        // divisor is the ship's own braking deceleration.
        owner_.done("ShipAiControls::class_retardation_0508", 0x009ed8ecu);
        return owner_.units.unit_retardation_0508(index_);
    }
    float unit_field_09c8() override {
        // 009ED902, FADD [EAX+9C8h]. No recovered field and no producer, so the
        // record's neutral zero is added to the stopping distance.
        owner_.record("ShipAiControls::unit_field_09c8", 0x009ed902u);
        return 0.0f;
    }
    float unit_heading_vtable_0050() override {
        owner_.done("ShipAiControls::unit_heading", 0x009ed95du);
        return owner_.units.unit_heading_radians(index_);
    }
    float unit_current_yaw_rate_00811940() override {
        owner_.done("ShipAiControls::current_yaw_rate", 0x00811940u);
        return owner_.units.unit_current_yaw_rate_00811940(index_);
    }

private:
    GameShipAiHost::Impl& owner_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// bsp::UnitHeadingTargetHost and bsp::ShipAiPublishHost, 009F4D10's own calls
// ---------------------------------------------------------------------------

class HeadingTargetBinding final : public bsp::UnitHeadingTargetHost {
public:
    HeadingTargetBinding(GameShipAiHost::Impl& owner, std::size_t index)
        : owner_(owner), index_(index) {}
    float heading_virtual_0050() override {
        owner_.done("ShipAiOrder::unit_heading", 0x00811974u);
        return owner_.units.unit_heading_radians(index_);
    }
    float forward_speed_0092d730() override {
        owner_.done("ShipAiOrder::forward_speed", 0x008119a1u);
        return owner_.units.unit_forward_speed_0092d730(index_);
    }

private:
    GameShipAiHost::Impl& owner_;
    std::size_t index_;
};

class PublishBinding final : public bsp::ShipAiPublishHost {
public:
    PublishBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                   std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}

    int order_slot_index_0b40() override {
        owner_.done("ShipAiOrder::slot_index_0b40", 0x009f4d2fu);
        return ctl_.order.index;
    }
    void set_heading_target_00811960(bsp::UnitHeadingTargetState& state,
                                     float desired_heading) override {
        HeadingTargetBinding binding(owner_, index_);
        bsp::unit_set_heading_target_00811960(state, desired_heading, binding);
        owner_.done("ShipAiOrder::set_heading_target", 0x00811960u);
    }
    void tail_009f0100(float) override {
        owner_.record("ShipAiOrder::tail_009f0100", 0x009f0100u);
    }
    void tail_009ef350() override {
        owner_.record("ShipAiOrder::tail_009ef350", 0x009ef350u);
    }
    void tail_009ef910(float) override {
        owner_.record("ShipAiOrder::tail_009ef910", 0x009ef910u);
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// Milestone 2p: 009F1420's head, the brain pre-pass that writes the goal vector
// ---------------------------------------------------------------------------
//
// docs/SHIP_AI_GOAL_VECTOR.md: `brain+0B2Ch..+0B34h` has one writer and it is
// this routine, which runs on every AI sub-tick from whatever command the
// entity is carrying. Milestone 2o recorded the call site and reported all 98
// of its `movetopos` goal sets as (0,0); this runs the producer instead. The
// executable already holds the director's command slots from milestones 2l,
// 2m and 2n, so 0071EB60 answers with a real descriptor rather than the empty
// singleton at 00E19B98 and the goal becomes the command's own position or the
// resolved target's world position.

class GoalVectorBinding final : public bsp::ShipAiGoalVectorHost {
public:
    GoalVectorBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                      std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}

    bsp::ShipAiGoalCommandDescriptor active_command_0071eb60() override {
        bsp::ShipAiGoalCommandDescriptor out{};
        bsp::SceneCommandTarget descriptor{};
        int mode = 0;
        const bool real = owner_.units.active_command_descriptor_0071eb60(index_, descriptor,
            mode);
        owner_.done("ShipAiGoal::active_command", 0x0071eb60u);
        descriptor_ = descriptor;
        singleton_ = !real;
        if (!real) {
            // The 00E19B98 singleton: +1h clear, +14h zero, the triple copied
            // from the zero vector at 00F87574.
            return out;
        }
        out.has_position = descriptor.position_valid != 0;
        out.x = descriptor.position[0];
        out.y = descriptor.position[1];
        out.z = descriptor.position[2];
        out.target = owner_.units.resolve_command_target_00521ea0(descriptor);
        return out;
    }
    std::uint32_t resolve_command_target_00521ea0() override {
        const std::uint32_t target = singleton_
            ? 0u
            : owner_.units.resolve_command_target_00521ea0(descriptor_);
        owner_.done("ShipAiGoal::resolve_command_target", 0x00521ea0u);
        return target;
    }
    void observer_unregister_006952a0(std::uint32_t) override {
        // 009E2FFD. The observer list the record joins is not built here, so
        // the pair is recorded; nothing downstream reads it.
        owner_.record("ShipAiGoal::observer_unregister", 0x006952a0u);
    }
    void observer_register_00694a60(std::uint32_t) override {
        owner_.record("ShipAiGoal::observer_register", 0x00694a60u);
    }
    bool target_is_kind_vtable_005c(std::uint32_t target, int kind) override {
        // 009F1491 with the literal 2 at 009F148D, answered through the same
        // recovered class chain 006FE530 the automatic target scan uses.
        owner_.done("ShipAiGoal::target_is_kind", 0x009f1491u);
        if (target == 0u) return false;
        return owner_.units.unit_is_kind_of(static_cast<std::size_t>(target - 1u), kind);
    }
    int unit_side_0054() override {
        owner_.done("ShipAiGoal::unit_side", 0x009f14e4u);
        return owner_.units.unit_side_0054(index_);
    }
    int target_side_0054(std::uint32_t target) override {
        owner_.done("ShipAiGoal::target_side", 0x009f14dbu);
        if (target == 0u) return -1;
        return owner_.units.unit_side_0054(static_cast<std::size_t>(target - 1u));
    }
    bool recon_knows_target_009dfbe0(int, std::uint32_t) override {
        // 009F14EC 008053C0 BSP_Recon_EnsureSlot then 009F14FA 009DFBE0. The
        // recon slot's intrusive list is empty in this process, the same
        // substitution milestone 2n makes for the party list, and 009DFBE0's
        // body was not read. The answer is recorded and false, which is the
        // arm that closes the gate; the surface test below then reopens it.
        owner_.record("ShipAiGoal::recon_knows_target", 0x009dfbe0u);
        return false;
    }
    bool target_is_surface_00922dc0(std::uint32_t) override {
        // 009F1519, 00922DC0 BSP_Entity_IsSurfaceTarget on the RAW target. The
        // routine is a thunk in this image and its body was not read, so the
        // answer is a record and the neutral one is false: the gate stays as
        // the recon test left it and nothing in this run is claimed about what
        // a surface target would do.
        owner_.record("ShipAiGoal::target_is_surface", 0x00922dc0u);
        return false;
    }
    bool target_pose_valid_00c8(std::uint32_t target) override {
        owner_.done("ShipAiGoal::target_pose_valid", 0x009dbcceu);
        if (target == 0u) return true;
        return owner_.units.unit_pose_valid_00c8(static_cast<std::size_t>(target - 1u));
    }
    void refresh_target_pose_00414db0(std::uint32_t) override {
        owner_.record("ShipAiGoal::refresh_target_pose", 0x00414db0u);
    }
    void transform_by_target_matrix_004142e0(std::uint32_t target, float in_x, float in_y,
                                             float in_z, float& out_x, float& out_y,
                                             float& out_z) override {
        // 009DBCED, 004142E0 with the matrix at target+0CCh: the latched triple
        // is a point in the target's frame. With a command that named an entity
        // the triple is the zero vector 009E2FC4 substituted, so the answer is
        // the target's own world position.
        owner_.done("ShipAiGoal::transform_by_target_matrix", 0x004142e0u);
        if (target == 0u) {
            out_x = in_x;
            out_y = in_y;
            out_z = in_z;
            return;
        }
        owner_.units.transform_by_unit_matrix_004142e0(
            static_cast<std::size_t>(target - 1u), in_x, in_y, in_z, out_x, out_y, out_z);
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
    bsp::SceneCommandTarget descriptor_{};
    bool singleton_{true};
};

// ---------------------------------------------------------------------------
// Milestone 2p: 009EF230, the obstacle sector refresh at chain slot 009F51FA
// ---------------------------------------------------------------------------
//
// docs/SHIP_AI_OBSTACLE_TABLES.md reads the routine and its schedule but does
// not project it, so this runs the two parts the doc establishes instruction by
// instruction - the round robin on blk+0A18h and the braking distance the
// store at 009EF32F writes into each visited sector's +8h - and records
// 009EB660 BSP_ShipAi_ScanObstacleSector once per visited sector, because the
// swept-arc geometry between 009EB6B7 and 009EBECC was not read by any packet
// and the neighbour list at blk+608h is empty in this process.

// Milestone 2r: bsp::ShipAiSectorScanHost, the call sites of 009EB660. The
// neighbour list blk+608h is empty and blk+0A3Ch / blk+0A24h are the zeroes
// 009E4330 wrote, so both avoid-zone gates are shut and no node can block a
// sector; what the scan does produce is the probe geometry and the range with
// the hysteresis margin, per sector, per frame.
class SectorScanBinding final : public bsp::ShipAiSectorScanHost {
public:
    SectorScanBinding(GameShipAiHost::Impl& owner, std::size_t index)
        : owner_(owner), index_(index) {}

    float settings_blocked_margin_1d8() override {
        owner_.record("ShipAiSectorScan::settings_blocked_margin_1d8", 0x009eb686u);
        return 0.0f;
    }
    float settings_neighbour_memory_194() override {
        owner_.record("ShipAiSectorScan::settings_neighbour_memory_194", 0x009ebee3u);
        return 0.0f;
    }
    float unit_heading_vtable50() override {
        owner_.done("ShipAiSectorScan::unit_heading_vtable50", 0x009eb939u);
        return owner_.units.unit_heading_radians(index_);
    }
    bool avoid_zone_segment_crossing_004158e0(const std::array<float, 2>&,
                                              const std::array<float, 2>&,
                                              std::array<float, 2>&) override {
        owner_.record("ShipAiSectorScan::zone_segment_crossing_004158e0", 0x004158e0u);
        return false;
    }
    bool point_in_avoid_box_009d8160(const bsp::ShipAiObstacleNode&,
                                     const std::array<float, 2>&) override {
        owner_.record("ShipAiSectorScan::point_in_avoid_box_009d8160", 0x009d8160u);
        return false;
    }
    bool point_in_near_box_009d80c0(const bsp::ShipAiObstacleNode&,
                                    const std::array<float, 2>&) override {
        owner_.record("ShipAiSectorScan::point_in_near_box_009d80c0", 0x009d80c0u);
        return false;
    }
    bool clip_ray_against_node_009dd540(const bsp::ShipAiObstacleNode&,
                                        const std::array<float, 2>&,
                                        const std::array<float, 2>&, float&) override {
        owner_.record("ShipAiSectorScan::clip_ray_009dd540", 0x009dd540u);
        return false;
    }
    bool clip_arc_against_avoid_zones_00415970(const std::array<float, 2>&, float, float,
                                               float&) override {
        owner_.record("ShipAiSectorScan::clip_arc_zones_00415970", 0x00415970u);
        return false;
    }
    bool clip_arc_against_node_009dd010(const bsp::ShipAiObstacleNode&,
                                        const std::array<float, 2>&, float, float,
                                        float&) override {
        owner_.record("ShipAiSectorScan::clip_arc_node_009dd010", 0x009dd010u);
        return false;
    }
    void raise_node_lifetime_78(bsp::ShipAiObstacleNode& node, float value) override {
        // 009EBEF7, an inlined compare and store on the blocking node. Reached
        // only when a node blocks, which needs a neighbour list.
        if (value > node.lifetime_78) node.lifetime_78 = value;
        owner_.record("ShipAiSectorScan::raise_node_lifetime_78", 0x009ebef7u);
    }
    bool avoid_zone_free_bearing_009dc2e0(bsp::ShipAiSectorFreeBearingQuery&) override {
        owner_.record("ShipAiSectorScan::free_bearing_009dc2e0", 0x009dc2e0u);
        return false;
    }

private:
    GameShipAiHost::Impl& owner_;
    std::size_t index_;
};

class ObstacleSectorRefresh {
public:
    static void run(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                    std::size_t index) {
        // 009EF247..009EF283: v = max(0092D730(unit), class+500h * 0.1).
        const float speed = owner.units.unit_forward_speed_0092d730(index);
        owner.done("ShipAiSectors::body_axis_speed", 0x0092d730u);
        const float max_speed = owner.units.unit_class_max_speed_0500(index);
        const double floor_speed = static_cast<double>(max_speed) * 0.1;  // 00D7A3A0
        double v = static_cast<double>(speed);
        if (v < floor_speed) v = floor_speed;
        // 009EF289..009EF2C9, with the doubles 00D7A2B0 = 3.0, 00CEC8F0 = 0.55
        // and 00CEFF98 = 0.6. class+508h is `Retardation`, the same divisor
        // 009ED8EC uses.
        const double retardation = static_cast<double>(owner.units.unit_retardation_0508(index));
        const double reach = v + 3.0;
        const double half_width = static_cast<double>(owner.units.unit_half_width_09cc(index));
        double braking = 0.0;
        if (retardation != 0.0) {
            braking = (reach / retardation) * reach * 0.55 + half_width * 0.6;
        } else {
            // 009EF2A1 FDIV with a zero divisor. The class row this mission
            // installs carries a real Retardation for every ship, so this arm
            // is reported rather than reasoned about.
            owner.record("ShipAiSectors::retardation_zero", 0x009ef2a1u);
        }
        const float braking_distance = static_cast<float>(braking);
        // 009EF2BD..009EF323: the schedule. The cursor is a 0..3 counter and
        // the loop starts at (counter & 1) + (counter >= 2 ? 6 : 0), stepping
        // two strides three times, so the twelve sectors refresh over four
        // frames, three a frame.
        const int cursor = ctl.sector_refresh_cursor_0a18;
        int sector = (cursor & 1) + ((cursor >= 2) ? 6 : 0);
        for (int i = 0; i < 3; ++i) {
            if (sector >= 0 && sector < bsp::kShipAiObstacleSectorCount) {
                ctl.obstacle.sector[static_cast<std::size_t>(sector)].braking_distance
                    = braking_distance;
                // 009EF334, 009EB660(sector, blk). Milestone 2r runs packet
                // cc_ai_sector_scan's whole-body projection in place of
                // milestone 2p's record, so the probe geometry, the swept arc
                // and the range with the hysteresis margin are code. The
                // neighbour list at blk+608h is empty and both avoid-zone
                // gates are the zeroes 009E4330 wrote, so no sector is marked.
                bsp::ShipAiSectorScanInputs scan{};
                {
                    float x = 0.0f, y = 0.0f, z = 0.0f;
                    owner.units.unit_position_00fc(index, x, y, z);
                    scan.pose.x = x;      // blk+184h
                    scan.pose.z = z;      // blk+188h
                }
                // blk+19Ch..+1B0h, the two beam axes and the hull forward that
                // 009DE2F0 rebuilds inside the pre-step 009E0270, a record
                // here. The hull's own world forward is available and is what
                // 009DE452 normalises, so the forward pair is filled from the
                // pose and the two beam pairs are its perpendiculars, which is
                // the same construction 009DE4F6 makes.
                {
                    float right[3] = {0.0f, 0.0f, 0.0f};
                    float up[3] = {0.0f, 0.0f, 0.0f};
                    float forward[3] = {0.0f, 0.0f, 0.0f};
                    float translation[3] = {0.0f, 0.0f, 0.0f};
                    if (owner.units.unit_pose(index, right, up, forward, translation)) {
                        scan.pose.forward_x = forward[0];
                        scan.pose.forward_z = forward[2];
                        scan.pose.port_x = -forward[2];
                        scan.pose.port_z = forward[0];
                        scan.pose.starboard_x = forward[2];
                        scan.pose.starboard_z = -forward[0];
                    }
                    owner.record("ShipAiSectors::hull_axes_009de2f0", 0x009de2f0u);
                }
                scan.pose.heading = owner.units.unit_heading_radians(index);
                scan.avoid_zones_present = false;  // blk+0A3Ch, 009E4401
                scan.avoid_zones_enabled = false;  // blk+0A24h
                SectorScanBinding scan_host(owner, index);
                const bsp::ShipAiSectorScanResult result
                    = bsp::ship_ai_scan_obstacle_sector_009eb660(
                        ctl.obstacle.sector[static_cast<std::size_t>(sector)], scan,
                        ctl.neighbours, scan_host);
                owner.done("ShipAiSectors::scan_sector", 0x009eb660u);
                ++owner.summary.sector_scans;
                if (result.blocked) ++owner.summary.sector_marks;
                // 009D84E0, the corner to steer at and the side to pass on. Its
                // only call site 009EBF51 sits inside the blocked arm, so a run
                // with no neighbour never reaches it.
                if (result.blocking_node < 0) {
                    owner.record("ShipAiSectors::passing_corner_009d84e0", 0x009d84e0u);
                }
                ctl.obstacle.sector[static_cast<std::size_t>(sector)].blocked
                    = result.blocked;
                ctl.obstacle.sector[static_cast<std::size_t>(sector)].blocker = nullptr;
            }
            sector += 2;
        }
        ctl.sector_refresh_cursor_0a18 = (cursor + 1) & 3;
        owner.done("ShipAi::refresh_obstacle_sectors", 0x009ef230u);
    }
};

// ---------------------------------------------------------------------------
// Milestone 2p: bsp::ShipAiObstacleHost, the middle of 009F3F80
// ---------------------------------------------------------------------------

class ObstacleBinding final : public bsp::ShipAiObstacleHost {
public:
    ObstacleBinding(GameShipAiHost::Impl& owner, std::size_t index) : owner_(owner),
        index_(index) {}
    void raise_turn_assist_load_102c(float value) override {
        // 009F438C, 009F462A and 009F4A51, the inlined body of 009D4FB0.
        owner_.units.raise_turn_assist_load_102c(index_, value);
        owner_.done("ShipAiObstacle::raise_turn_assist_load", 0x009f438cu);
    }
    void raise_secondary_load_1034(float value) override {
        // 009F45FE, 009F4912 and 009F4AB4, the inlined body of 009D4FE0.
        owner_.units.raise_secondary_load_1034(index_, value);
        owner_.done("ShipAiObstacle::raise_secondary_load", 0x009f45feu);
    }
    float rudder_law_009da250(float heading_error) override {
        // 009F44F7 and 009F4694. The law itself is the same reconstruction the
        // 2o path already ran; the divisor is the derived class+524h.
        RudderLawBinding law(owner_, index_);
        bool derived = false;
        const float authority = owner_.units.unit_yaw_authority_0524(index_, derived);
        if (derived) {
            owner_.done("ShipAiRudder::class_yaw_authority_0524", 0x00828f20u);
        } else {
            owner_.record("ShipAiRudder::class_yaw_authority_ungated", 0x009da268u);
        }
        const float rudder = bsp::ship_ai_rudder_from_heading_error_009da250(direction_,
            heading_error, authority, law);
        owner_.done("ShipAiRudder::from_heading_error", 0x009da250u);
        ++calls_;
        return rudder;
    }
    float neighbour_body_axis_speed_0092d730(const bsp::ShipAiNeighbourRecord&) override {
        // 009F47B8 and 009D8BC1. Unreachable here: the blocked-sector arm needs
        // a sector with a blocker, and 009EB660's scan finds none.
        owner_.record("ShipAiObstacle::neighbour_body_axis_speed", 0x009f47b8u);
        return 0.0f;
    }

    void set_direction(bsp::ShipAiThrottleDirection direction) noexcept {
        direction_ = direction;
    }
    unsigned long long calls() const noexcept { return calls_; }

private:
    GameShipAiHost::Impl& owner_;
    std::size_t index_;
    bsp::ShipAiThrottleDirection direction_{bsp::ShipAiThrottleDirection::Stopped};
    unsigned long long calls_{0};
};

// ---------------------------------------------------------------------------
// Milestone 2p: bsp::ShipAiPathPickHost, 009EE580..009EE670
// ---------------------------------------------------------------------------

// bsp::ShipAiPathPlannerHost, the call sites inside 009E3780. Packet
// ship_ai_path_planner landed on main at 878325ba during this packet's turn and
// was merged in before validation, so the plan request is no longer a record.
class PathPlannerBinding final : public bsp::ShipAiPathPlannerHost {
public:
    PathPlannerBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                       std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}

    std::uint32_t avoid_zone_manager_004218e0() override {
        // The avoid-zone singleton is not built in this process, the same
        // record the attackmove engage gate hits at 009E864C.
        owner_.record("ShipAiPlanner::avoid_zone_manager", 0x004218e0u);
        return 0u;
    }
    std::uint32_t zone_containing_point_00417e40(std::uint32_t,
        const std::array<float, 2>&, std::uint32_t) override {
        owner_.record("ShipAiPlanner::zone_containing_point", 0x00417e40u);
        return 0u;
    }
    std::array<float, 2> push_point_out_of_zone_00417580(std::uint32_t,
        const std::array<float, 2>& point, float) override {
        owner_.record("ShipAiPlanner::push_point_out_of_zone", 0x00417580u);
        return point;
    }
    std::uint32_t zone_group_for_layer_004120d0(std::uint32_t, std::uint32_t) override {
        owner_.record("ShipAiPlanner::zone_group_for_layer", 0x004120d0u);
        return 0u;
    }
    std::array<float, 2> nearest_zone_boundary_0041b840(std::uint32_t,
        const std::array<float, 2>& point, float, float) override {
        owner_.record("ShipAiPlanner::nearest_zone_boundary", 0x0041b840u);
        return point;
    }
    bsp::ShipAiPathNode* allocate_path_node_00bf681b(std::size_t) override {
        // operator new. The nodes belong to the plan block, so this process
        // owns them for the life of the controller.
        ctl_.plan_nodes.emplace_back();
        owner_.done("ShipAiPlanner::allocate_path_node", 0x00bf681bu);
        return &ctl_.plan_nodes.back();
    }
    float owner_seed_vtable50() override {
        owner_.done("ShipAiPlanner::owner_seed_heading", 0x009e3982u);
        return owner_.units.unit_heading_radians(index_);
    }
    float owner_radius_09c8() override {
        // 009E3ADB, [ship+9C8h]. No recovered producer anywhere; the same field
        // the drive's danger ramp records at 009F4174.
        owner_.record("ShipAiPlanner::owner_radius_09c8", 0x009e3adbu);
        return 0.0f;
    }
    float owner_class_max_speed_0500() override {
        owner_.done("ShipAiPlanner::owner_class_max_speed", 0x009e3af0u);
        return owner_.units.unit_class_max_speed_0500(index_);
    }
    void release_node_list_vtable0(bsp::ShipAiPathNode*) override {
        owner_.record_slot("ShipAiPlanner::release_node_list", "00d214f4+vtable00");
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

// Milestone 2q: bsp::ShipAiPathSearchHost, the nine call sites of 009EC680's
// five routines. Packet cc_ai_path_search landed on main at 4d3b1e49 and this
// packet merged it in, so the search itself is a projection; what stays a
// record is the avoid-zone manager, which construct_world 004DE610 would have
// to build. With no manager, 00417E90 cannot be called and the edge probe's own
// step 2 is the whole "is the sea clear between these two points" question
// (docs/SHIP_AI_PATH_SEARCH.md), so the neutral answer is `not blocked` and the
// search never reaches 00422500, 0071C4F0, 00417610 or 00423190.
class PathSearchBinding final : public bsp::ShipAiPathSearchHost {
public:
    PathSearchBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl)
        : owner_(owner), ctl_(ctl) {}

    bsp::ShipAiPathSearchTurnRamp game_settings_turn_ramp_00424c40() override {
        // settings+6F0h / +6F4h / +6F8h. The gameplay settings singleton this
        // process builds carries the zeroes a fresh object has for this block:
        // 0083B5E0's recovered head fills the rudder curve and the auto-thrust
        // band, and no recovered loader writes these three.
        owner_.record("ShipAiSearch::turn_ramp", 0x00424c40u);
        return bsp::ShipAiPathSearchTurnRamp{0.0f, 0.0f, 0.0f};
    }
    std::uint32_t avoid_zone_manager_004218e0() override {
        owner_.record("ShipAiSearch::avoid_zone_manager", 0x004218e0u);
        return 0u;
    }
    bool segment_blocked_00417e90(std::uint32_t, std::uint32_t,
                                  const std::array<float, 2>&, const std::array<float, 2>&,
                                  std::uint32_t& out_zone,
                                  std::int32_t& out_edge_index) override {
        owner_.record("ShipAiSearch::segment_blocked", 0x00417e90u);
        out_zone = 0u;
        out_edge_index = -1;
        return false;
    }
    std::int32_t zone_detour_corners_00422500(std::uint32_t, const std::array<float, 2>&,
                                              std::int32_t, std::int32_t, std::int32_t, float,
                                              std::array<float, 2>&, std::array<float, 2>&,
                                              std::int32_t& out_left_index,
                                              std::int32_t& out_right_index) override {
        owner_.record("ShipAiSearch::zone_detour_corners", 0x00422500u);
        out_left_index = -1;
        out_right_index = -1;
        return 0;
    }
    bool point_outside_world_bounds_0071c4f0(const std::array<float, 3>&) override {
        // Milestone 2p correction 8: the world box at [00E188A8]+711Ch.. does
        // not exist, so the neutral `inside` answer stands.
        owner_.record("ShipAiSearch::point_outside_world_bounds", 0x0071c4f0u);
        return false;
    }
    bsp::ShipAiPathNode* allocate_path_node_00bf681b(std::size_t) override {
        ctl_.plan_nodes.emplace_back();
        owner_.done("ShipAiSearch::allocate_path_node", 0x00bf681bu);
        return &ctl_.plan_nodes.back();
    }
    std::uint32_t zone_corner_record_00417610(std::uint32_t, std::int32_t) override {
        owner_.record("ShipAiSearch::zone_corner_record", 0x00417610u);
        return 0u;
    }
    void ensure_zone_corner_metric_00423190(std::uint32_t, std::uint32_t) override {
        owner_.record("ShipAiSearch::zone_corner_metric", 0x00423190u);
    }
    void release_path_node_vtable0(bsp::ShipAiPathNode*) override {
        // The nodes live in the controller's own deque, which the plan links by
        // address, so nothing is freed here. The same boundary the planner's
        // own release records.
        owner_.record_slot("ShipAiSearch::release_path_node", "00d214f4+vtable00");
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
};

// Milestone 2q: bsp::ShipAiPathPointHost, the four call sites of 009E3C00 the
// walk reaches.
// Milestone 2r: bsp::ShipAiPathFollowerHost. Packet cc_ai_path_follower read
// 009E3C00-009E432A whole, including the corner arm 009E3F1A..009E4222 that
// milestone 2q's projection stopped at, so the executable now runs that
// reconstruction instead and its host has the follower's nine call sites.
class PathFollowerBinding final : public bsp::ShipAiPathFollowerHost {
public:
    PathFollowerBinding(GameShipAiHost::Impl& owner, GameShipAiRow& row, std::size_t index)
        : owner_(owner), row_(row), index_(index) {}

    const bsp::ShipAiPathLateralAnchor* lateral_anchor_node_10(std::uint32_t handle)
        override {
        // 009E3D8C and 009E3DD2, a dereference of node+10h, not a call. No node
        // of an unzoned plan carries a lateral record, so the answer is null and
        // the read is recorded with the field's own site.
        static_cast<void>(handle);
        owner_.record("ShipAiPathFollower::lateral_anchor_node_10", 0x009e3d8cu);
        return nullptr;
    }
    float order_turn_limit_at_00811d80(const std::array<float, 2>& xz) override {
        // 009E3DCD, 00811D80 on the unit's own PUBLISHED order slot. Reached
        // only inside the lateral-anchor arm above, which no node of an
        // unzoned plan enters, so the call is not made in this run at all. The
        // executable holds the four published fields of the slot
        // (UnitAiOrderSlot) and not the two sub-records 00811D80 searches, so
        // the answer would be the routine's own 30.0f whatever the position is.
        static_cast<void>(xz);
        owner_.record("ShipAiPathFollower::order_turn_limit_00811d80", 0x00811d80u);
        return bsp::kUnitAiOrderTurnLimitDefault;
    }
    float owner_class_turn_radius_0082e850() override {
        // 009E3EAE, 0082E850 on [[plan+3Ch]+538h]: class+520h, which 00828F20
        // derives from MaxSpeed and MaxRotAngle. Milestone 2q recorded this
        // because no packet had read the deriver; packet ship_ai_class_field_0524
        // has, and the units host answers with the derived field.
        const float radius = owner_.units.unit_class_turn_radius_0520(index_);
        owner_.done("ShipAiPathFollower::class_turn_radius_0082e850", 0x0082e850u);
        return radius;
    }
    float owner_radius_09c8() override {
        // 009E3EC0, [plan+3Ch]+9C8h. Its producers are 0081106E and 0081FA4D,
        // and with no model box the answer is the descriptor's `Length`.
        const float length = owner_.units.unit_hull_length_09c8(index_);
        owner_.done("ShipAiPathFollower::owner_length_09c8", 0x009e3ec0u);
        return length;
    }
    std::uint32_t avoid_zone_manager_004218e0() override {
        // 009E4200. The manager singleton has no producer in this process;
        // packet cc_ai_avoid_zones owns it.
        owner_.record("ShipAiPathFollower::avoid_zone_manager", 0x004218e0u);
        return 0u;
    }
    bool segment_hits_zone_00417ef0(std::uint32_t manager, std::uint32_t zone_layer,
                                    const std::array<float, 2>& from,
                                    const std::array<float, 2>& to,
                                    std::array<float, 2>& hit) override {
        static_cast<void>(manager);
        static_cast<void>(zone_layer);
        static_cast<void>(from);
        static_cast<void>(to);
        static_cast<void>(hit);
        owner_.record("ShipAiPathFollower::segment_hits_zone", 0x00417ef0u);
        return false;
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// Milestone 2r: bsp::ShipAiThrottleProfileHost, the call sites of 009E04E0.
// The contact-track list is empty in this process, so the routine's list walk
// does nothing, the avoidance vector blk+34Ch/+350h is cleared every step and
// the 65-bin profile keeps the bypass byte 009E435F set. That is the run's own
// state: 009E4653 clears blk+400h and nothing in this process appends a track.
class ThrottleProfileBinding final : public bsp::ShipAiThrottleProfileHost {
public:
    ThrottleProfileBinding(GameShipAiHost::Impl& owner,
                           GameShipAiHost::Impl::Controller& ctl, std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}

    float hull_heading_vtable50() override {
        owner_.done("ShipAiThrottleProfile::hull_heading_vtable50", 0x009e0509u);
        return owner_.units.unit_heading_radians(index_);
    }
    bool track_range_within_target_488(int) override {
        owner_.record("ShipAiThrottleProfile::track_range_488", 0x009e0613u);
        return false;
    }
    bool avoidance_active_009da1d0() override {
        // 009E061B. 009DA1D0 needs the unit's gameplay byte +240h, which has no
        // producer here; blk+3ECh is the byte `stop` writes.
        owner_.record("ShipAiThrottleProfile::avoidance_active_009da1d0", 0x009da1d0u);
        return false;
    }
    bool refresh_track_009dc060(int) override {
        owner_.record("ShipAiThrottleProfile::refresh_track_009dc060", 0x009dc060u);
        return false;
    }
    float class_length_three_quarters_00811a30() override {
        const float radius = owner_.units.unit_class_turn_circle_radius_0082e960(index_,
            bsp::kShipAiContactGapLengthArg);
        owner_.done("ShipAiThrottleProfile::class_length_00811a30", 0x00811a30u);
        return radius;
    }
    int track_count_400() override {
        owner_.done("ShipAiThrottleProfile::track_count_400", 0x009e05c0u);
        return 0;
    }
    bsp::ShipAiContactTrack& track_at(int) override {
        owner_.record("ShipAiThrottleProfile::track_at", 0x009e05c7u);
        return ctl_.track_scratch;
    }
    void destroy_track(int) override {
        owner_.record("ShipAiThrottleProfile::destroy_track", 0x009e0fbdu);
    }
    bool track_has_source(int) override {
        owner_.record("ShipAiThrottleProfile::track_has_source", 0x009e05efu);
        return false;
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

// Milestone 2r: bsp::ShipAiClearanceHost, the fifteen call sites of 009EF910.
// Packet cc_ai_clearance_profile read the routine whole; the executable runs it
// from inside 009F4D10 at 009F4D87. Every neighbour and zone test answers "no
// such thing" here, and that is the run's own state rather than a stand-in: the
// neighbour count blk+604h is the zero 009E4659 wrote and 009F0D20 / 009F0EA0
// never move because no world entity list exists, and blk+0A3Ch is the zero
// 009E4401 wrote because no avoid zone is loaded. With nothing to lower it the
// clearance stays at the 9999.0f sentinel 009EF96F seeds.
class ClearanceBinding final : public bsp::ShipAiClearanceHost {
public:
    ClearanceBinding(GameShipAiHost::Impl& owner, std::size_t index)
        : owner_(owner), index_(index) {}

    float hull_heading_vtable50() override {
        owner_.done("ShipAiClearance::hull_heading_vtable50", 0x009ef97cu);
        return owner_.units.unit_heading_radians(index_);
    }
    bool obstacle_category_enabled_009ec770(int category) override {
        // 009EFA08. The predicate needs the unit's gameplay object byte +241h
        // and the settings byte +4h, neither of which has a producer here.
        static_cast<void>(category);
        owner_.record("ShipAiClearance::category_enabled_009ec770", 0x009ec770u);
        return false;
    }
    bool avoidance_globally_enabled_0080e160_242() override {
        owner_.record("ShipAiClearance::avoidance_enabled_0080e160", 0x0080e160u);
        return false;
    }
    bool static_zone_blocks_009d57e0(float, float, float, float, float) override {
        owner_.record("ShipAiClearance::static_zone_blocks_009d57e0", 0x009d57e0u);
        return false;
    }
    float static_zone_clearance_00415d70(float, float, float, float, float, float,
                                         float) override {
        owner_.record("ShipAiClearance::static_zone_clearance_00415d70", 0x00415d70u);
        return bsp::kShipAiClearanceSentinel;
    }
    int neighbour_count_604() override {
        owner_.done("ShipAiClearance::neighbour_count_604", 0x009efd5bu);
        return 0;
    }
    bool neighbour_owner_present_14(int) override {
        owner_.record("ShipAiClearance::neighbour_owner_14", 0x009efd80u);
        return false;
    }
    bool neighbour_owner_gone_5e(int) override {
        owner_.record("ShipAiClearance::neighbour_owner_5e", 0x009efd8du);
        return true;
    }
    int neighbour_owner_category_54(int) override {
        owner_.record("ShipAiClearance::neighbour_owner_54", 0x009efd9du);
        return -1;
    }
    bool neighbour_blocks_sweep_009dd010(int, float, float, float, float, float) override {
        owner_.record("ShipAiClearance::neighbour_blocks_sweep_009dd010", 0x009dd010u);
        return false;
    }
    void neighbour_support_point_009d8860(int, float, float, float& out_x,
                                          float& out_z) override {
        owner_.record("ShipAiClearance::neighbour_support_009d8860", 0x009d8860u);
        out_x = 0.0f;
        out_z = 0.0f;
    }
    void neighbour_closest_point_009d8a30(int, float, float, float& out_x,
                                          float& out_z) override {
        owner_.record("ShipAiClearance::neighbour_closest_009d8a30", 0x009d8a30u);
        out_x = 0.0f;
        out_z = 0.0f;
    }
    float neighbour_speed_0092d730(int) override {
        owner_.record("ShipAiClearance::neighbour_speed_0092d730", 0x009eff7fu);
        return 0.0f;
    }
    bool path_fade_applies_00778890() override {
        // 009F0000 / 009F0019: the group-leader query and the vtable identity
        // test against 00E08F80, the `moveonpath` command object. Neither has a
        // producer here; the same pair is recorded in the approach binding.
        owner_.record("ShipAiClearance::path_fade_00778890", 0x00778890u);
        return false;
    }
    float class_length_unit_00811a30() override {
        // 009F0038, 00811A30(unit, 1.0f): 0082E960(class, 1.0f) divided by the
        // gameplay modifier product for channel 5, which is 1.0f in this
        // process. The turn-circle half is now a recovered value, so only the
        // divide is a record.
        const float radius = owner_.units.unit_class_turn_circle_radius_0082e960(index_,
            1.0f);
        owner_.done("ShipAiClearance::class_length_unit_00811a30", 0x00811a30u);
        return radius;
    }

private:
    GameShipAiHost::Impl& owner_;
    std::size_t index_;
};

// Milestone 2q: bsp::ShipAiNavHost, the two call sites of 009EE671's block.
class NavArmBinding final : public bsp::ShipAiNavHost {
public:
    NavArmBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                  std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}

    float remaining_path_length_009d9e50() override {
        // 009EE6AC, 009D9E50([nav+2F4h])(&pose). Packet ship_ai_path_planner
        // projected it; with the plan now filled it walks real nodes.
        float x = 0.0f, y = 0.0f, z = 0.0f;
        owner_.units.unit_position_00fc(index_, x, y, z);
        bsp::ShipAiPathPlanBlock& live
            = (ctl_.plan_front == 0) ? ctl_.plan_a : ctl_.plan_b;
        const float length = bsp::ship_ai_path_remaining_length_009d9e50(
            live, std::array<float, 2>{x, z});
        owner_.done("ShipAiNav::remaining_path_length", 0x009d9e50u);
        return length;
    }

    float unit_heading_vtable_0050() override {
        owner_.done("ShipAiNav::unit_heading", 0x009ee8c7u);
        return owner_.units.unit_heading_radians(index_);
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

// Milestone 2q: bsp::ShipAiArmTailHost, the ten call sites of 009EEAAB..
// 009EF226. Packet ship_ai_navigation_arm_tail landed on main at 4491d04f
// during this packet's turn and was merged in before validation, so the tail is
// a projection rather than the record milestone 2p left.
class ArmTailBinding final : public bsp::ShipAiArmTailHost {
public:
    ArmTailBinding(GameShipAiHost::Impl& owner, std::size_t index)
        : owner_(owner), index_(index) {}

    bsp::ShipAiNavPose normalize_004192e0_009eeb63(float x, float z) override {
        // 004192E0(&out, &in): 00419260 answers 1/sqrt(x*x + z*z), and +0.0f
        // for a zero-length vector, and both components are scaled by it.
        owner_.done("ShipAiArmTail::normalize", 0x004192e0u);
        const double square = static_cast<double>(x) * x + static_cast<double>(z) * z;
        bsp::ShipAiNavPose out{};
        if (square <= 0.0) return out;
        const float scale = static_cast<float>(1.0 / std::sqrt(square));
        out.x = x * scale;
        out.z = z * scale;
        return out;
    }
    int neighbour_list_count_009eeb8b() override {
        // [[00E188A8]+19CCh]+60h. construct_world 004DE610 is a load record, so
        // there is no world entity list to walk, the same boundary the obstacle
        // sector scan and the world-bounds box hit.
        owner_.record("ShipAiArmTail::neighbour_list_count", 0x009eeb8bu);
        return 0;
    }
    bsp::ShipAiArmTailEntity list_element_009dbbc0(int) override {
        owner_.record("ShipAiArmTail::list_element", 0x009dbbc0u);
        return nullptr;
    }
    bool entity_is_kind_009eebc8(bsp::ShipAiArmTailEntity, int) override {
        owner_.record_slot("ShipAiArmTail::entity_is_kind", "00cfc3d0+vtable5c");
        return false;
    }
    bsp::ShipAiArmTailEntity own_unit_009eebd2() override {
        owner_.done("ShipAiArmTail::own_unit", 0x009eebd2u);
        return reinterpret_cast<bsp::ShipAiArmTailEntity>(
            static_cast<std::uintptr_t>(index_) + 1u);
    }
    float entity_hull_radius_009eebe0(bsp::ShipAiArmTailEntity) override {
        owner_.record("ShipAiArmTail::entity_hull_radius", 0x009eebe0u);
        return 0.0f;
    }
    bsp::ShipAiNavPose entity_position_00427eb0_009eec41(
        bsp::ShipAiArmTailEntity) override {
        owner_.record("ShipAiArmTail::entity_position", 0x00427eb0u);
        return bsp::ShipAiNavPose{};
    }
    float unit_heading_vtable_0050_009ef0d6() override {
        owner_.done("ShipAiArmTail::unit_heading", 0x009ef0d6u);
        return owner_.units.unit_heading_radians(index_);
    }
    float ship_class_turn_radius_0082e850_009ef112() override {
        // class+520h, whose only writer 00828F20 is not reconstructed. It only
        // widens the astern test's distance threshold, so a zero answer makes a
        // ship decline to reverse rather than invent a reversal.
        owner_.record("ShipAiArmTail::class_turn_radius", 0x0082e850u);
        return 0.0f;
    }
    void after_arm_009de5b0(float) override {
        owner_.record("ShipAiArmTail::after_arm", 0x009de5b0u);
    }

private:
    GameShipAiHost::Impl& owner_;
    std::size_t index_;
};

class PathPickBinding final : public bsp::ShipAiPathPickHost {
public:
    PathPickBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                    GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}
    void refresh_path_plan_009ed3e0(float seconds) override {
        // 009EE5C2, 009ED3E0(nav)(seconds). Milestone 2q projects the arm
        // 009ED4E4..009ED69E out of that body: the two 009E3780 revalidation
        // sites, the two 009EC680 search ticks and the front/back swap. The
        // head 009ED3E0..009ED4E2, which builds the two corridor widths from
        // the unit's group, is not projected, so both widths are the literal
        // 20.0f at 00CE3930 that 009ED3E3 seeds them with.
        owner_.done("ShipAiPath::refresh_plan_009ed4e4", 0x009ed4e4u);
        owner_.record("ShipAiPath::refresh_plan_head", 0x009ed3e0u);
        ++row_.path_plan_refreshes;
        ++owner_.summary.path_plan_refreshes;

        ctl_.plan_a.owner = &ctl_;
        ctl_.plan_b.owner = &ctl_;
        float pose_x = 0.0f, pose_y = 0.0f, pose_z = 0.0f;
        owner_.units.unit_position_00fc(index_, pose_x, pose_y, pose_z);

        bsp::ShipAiPathRefreshState state{};
        state.in_use = (ctl_.plan_front == 0) ? &ctl_.plan_a : &ctl_.plan_b;
        state.computing = (ctl_.plan_front == 0) ? &ctl_.plan_b : &ctl_.plan_a;
        state.computing_flag_2fc = ctl_.plan_computing_2fc;

        PathPlannerBinding planner(owner_, ctl_, index_);
        PathSearchBinding search(owner_, ctl_);
        const bsp::ShipAiPathRefreshResult result = bsp::ship_ai_path_refresh_arm_009ed4e4(
            state, seconds,
            std::array<float, 2>{pose_x, pose_z},
            std::array<float, 2>{ctl_.goal.goal_x_1dc, ctl_.goal.goal_z_1e0},
            state.in_use->zone_layer, 0.0f,
            bsp::kShipAiPathCorridorWidthDefault, bsp::kShipAiPathCorridorWidthDefault,
            planner, search);
        owner_.done("ShipAiPath::plan_009e3780", 0x009e3780u);
        owner_.done("ShipAiPath::search_step_009ec680", 0x009ec680u);
        owner_.done("ShipAiPath::corridor_width_009d9de0", 0x009d9de0u);

        ctl_.plan_computing_2fc = state.computing_flag_2fc;
        ctl_.plan_front = (state.in_use == &ctl_.plan_a) ? 0 : 1;
        if (result.seeded_fresh || result.reseeded_back) {
            ++row_.path_plan_seeds;
            ++owner_.summary.path_plan_seeds;
        }
        if (result.front_accepted) {
            ++row_.path_plan_accepts;
            ++owner_.summary.path_plan_accepts;
        }
        if (result.searched_back || result.searched_front) {
            ++row_.path_search_ticks;
            ++owner_.summary.path_search_ticks;
        }
        if (result.swapped) {
            ++row_.path_plan_swaps;
            ++owner_.summary.path_plan_swaps;
        }
        const bsp::ShipAiPathPlanBlock& live = *state.in_use;
        row_.path_plan_state = live.search_state;
        row_.path_plan_nodes = live.node_count;
    }
    void next_path_point_009e3c00(bsp::ShipAiPathPointRecord& record) override {
        // 009EE5F4, 009E3C00([nav+2F4h])(&record). Milestone 2r runs packet
        // cc_ai_path_follower's whole-body projection in place of milestone
        // 2q's partial one, so the corner arm 009E3F1A..009E4222, the shortcut
        // test and the cursor advance at 009E421F are code rather than records.
        // On a two-node open-sea plan the walk does not run, the target is the
        // goal node and the published point is the goal itself.
        bsp::ShipAiPathPlanBlock& live
            = (ctl_.plan_front == 0) ? ctl_.plan_a : ctl_.plan_b;
        PathFollowerBinding point(owner_, row_, index_);
        const bsp::ShipAiPathFollowerResult result
            = bsp::ship_ai_path_follower_point_009e3c00(live, record, point);
        owner_.done("ShipAiPath::next_point_009e3c00", 0x009e3c00u);
        if (result.exit == bsp::ShipAiPathFollowerExit::StraightAtPoint
            || result.exit == bsp::ShipAiPathFollowerExit::CornerTangent) {
            ++row_.path_points;
            ++owner_.summary.path_points;
            ++owner_.summary.path_follower_points;
            row_.path_point_x = record.point_x_08;
            row_.path_point_z = record.point_z_0c;
        }
        if (result.exit == bsp::ShipAiPathFollowerExit::CornerTangent) {
            ++row_.path_corner_arms;
            ++owner_.summary.path_corner_arms;
            ++owner_.summary.path_follower_corners;
        }
        if (result.advanced_cursor) ++owner_.summary.path_follower_advances;
    }
    float path_width_2f4_08() override {
        owner_.record("ShipAiPath::path_width_2f4", 0x009ee61eu);
        return 0.0f;
    }
    void publish_lateral_offset_00815f30(std::uint32_t, int, float, float) override {
        owner_.record("ShipAiPath::publish_lateral_offset", 0x00815f30u);
        ++row_.path_publishes;
        ++owner_.summary.path_publishes;
    }
    float path_node_width_20(std::uint32_t) override {
        owner_.record("ShipAiPath::node_width", 0x009ee63au);
        return 0.0f;
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// bsp::ShipAiControllerHost, the sixteen steps of 009F50E0
// ---------------------------------------------------------------------------

class ControllerBinding final : public bsp::ShipAiControllerHost {
public:
    ControllerBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                      GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}

    bool unit_present_0b00() override { return true; }           // 009F50E4
    bool unit_flag_005d() override {
        // 009F50F2. bsp/unit_instance.hpp names unit+5Dh `simulate`; milestone
        // 2i holds it clear for a live ship, which is what the eight
        // local-player unit lists of docs/LOCAL_PLAYER_UNIT_LISTS.md require.
        return owner_.units.unit_flag_005d(index_);
    }
    bool unit_flag_0061() override {
        // 009F50FC. The same byte 008266C1 reads to choose the autopilot pair.
        // docs/UNIT_AUTOPILOT_PAIR.md scanned `.text` for a writer and found
        // none outside the constructor, so it is clear here and the AI runs.
        return owner_.units.unit_flag_0061(index_);
    }
    bool sync_state_009f3dd0() override {
        SyncBinding sync(owner_, ctl_, row_, index_);
        const bool changed = bsp::ship_ai_sync_state_to_command_009f3dd0(sync);
        owner_.done("ShipAi::sync_state_to_command", 0x009f3dd0u);
        return changed;
    }
    void pre_step_009e0270(bool) override {
        owner_.record("ShipAi::pre_step", 0x009e0270u);
    }
    void replan_prepare_009f1420(float elapsed) override {
        // 009F516C, the brain pre-pass, once per AI sub-tick with the
        // accumulated delta. Milestone 2o recorded this call site; packet
        // cc_ai_goal_vector read 009F1420-009F158A and this runs it, so the
        // goal vector at brain+0B2Ch..+0B34h is written from the unit's own
        // active command rather than left at the zeroes the block carries.
        // 009F158A..009F1BB8, the two further countdowns at brain+0B44h /
        // +0B4Ch and the proximity scan they drive, is not projected and is
        // recorded with its own address.
        GoalVectorBinding goal(owner_, ctl_, index_);
        const bsp::ShipAiGoalRefreshResult result
            = bsp::ship_ai_refresh_goal_vector_009f1420(ctl_.goal_vector, ctl_.latched,
                                                        elapsed, goal);
        owner_.done("ShipAi::replan_prepare", 0x009f1420u);
        owner_.record("ShipAi::replan_prepare_threat_scan", 0x009f158au);
        ++row_.goal_prepasses;
        ++owner_.summary.goal_prepasses;
        if (result.goal_rewritten) {
            ++row_.goal_refreshes;
            ++owner_.summary.goal_refreshes;
        }
        row_.brain_goal_x = ctl_.goal_vector.goal_x_0b2c;
        row_.brain_goal_y = ctl_.goal_vector.goal_y_0b30;
        row_.brain_goal_z = ctl_.goal_vector.goal_z_0b34;
        row_.brain_target = ctl_.goal_vector.raw_target_0b20;
        row_.brain_target_name.clear();
        if (ctl_.goal_vector.raw_target_0b20 != 0u) {
            const GameUnitRow* target = owner_.units.unit_row(
                static_cast<std::size_t>(ctl_.goal_vector.raw_target_0b20 - 1u));
            if (target != nullptr) row_.brain_target_name = target->name;
        }
        {
            // The descriptor as it stands on this sub-tick, not as it stood on
            // the first one: a unit's slot 0 changes when a later command is
            // pushed, and the goal below is the value that descriptor produced.
            bsp::SceneCommandTarget descriptor{};
            int mode = 0;
            const bool real = owner_.units.active_command_descriptor_0071eb60(index_,
                descriptor, mode);
            char text[96];
            if (!real) {
                std::snprintf(text, sizeof(text), "mode=%d empty singleton 00e19b98", mode);
            } else if (descriptor.position_valid != 0) {
                std::snprintf(text, sizeof(text), "mode=%d position (%.1f, %.1f)", mode,
                    static_cast<double>(descriptor.position[0]),
                    static_cast<double>(descriptor.position[2]));
            } else {
                std::snprintf(text, sizeof(text), "mode=%d object id=%u", mode,
                    static_cast<unsigned>(descriptor.object_id));
            }
            row_.command_descriptor = text;
        }
    }
    void state_step_vtable0c(float elapsed) override {
        ++row_.replans;
        ++owner_.summary.replans;
        const StateDescriptor* state = state_for_ai_offset(ctl_.active_state_ai_offset);
        if (state != nullptr && state->step_concrete) {
            SetterBinding setters(owner_);
            if (owner_.units.run_cruise_state_step_009e1170(index_, ctl_.blk, setters)) {
                ++owner_.summary.state_steps_concrete;
                ++row_.state_step_real;
                ++owner_.summary.state_steps_real;
                apply_ai_drive();
                return;
            }
            // 009E1170 ran and took its 009E11E8 player arm, which is not
            // projected; GameCommandsHost has already recorded that arm with its
            // own address. Recording the step again here would put one method
            // name on both dispositions, and the log keeps the first.
            ++owner_.summary.state_steps_recorded;
            return;
        }
        // Milestone 2o, second pass: three of the eight leaves now have a body.
        // Packet ship_ai_state_steps projected 009E14C0 `stop`, 009E5770
        // `movetopos` and 009E8820 `attackmove` with its selector 009E86F0
        // complete, so those run here instead of being recorded. `follow`,
        // `land`, `moveonpath`, `kamikaze_attack` and `sub_attack` are still
        // records with their own addresses.
        if (state != nullptr && state->step == 0x009e14c0u) {
            StopStepBinding stop(owner_, ctl_, row_, index_);
            bsp::ship_ai_stop_step_009e14c0(ctl_.stop_state, ctl_.blk, ctl_.avoidance, stop);
            owner_.done("ShipAiState::stop_step", 0x009e14c0u);
            row_.avoidance_enabled = ctl_.avoidance.enable_3f4;
            row_.avoidance_side = ctl_.avoidance.side_filter_3f8;
            ++owner_.summary.state_steps_concrete;
            ++row_.state_step_real;
            ++owner_.summary.state_steps_real;
            apply_ai_drive();
            return;
        }
        if (state != nullptr && state->step == 0x009e5770u) {
            MoveToPosStepBinding move(owner_, ctl_, row_, index_);
            bsp::ship_ai_movetopos_step_009e5770(move);
            owner_.done("ShipAiState::movetopos_step", 0x009e5770u);
            ++owner_.summary.state_steps_concrete;
            ++row_.state_step_real;
            ++owner_.summary.state_steps_real;
            apply_ai_drive();
            return;
        }
        if (state != nullptr && state->step == 0x009e8820u) {
            AttackMoveStepBinding attack(owner_, ctl_, row_, index_);
            bsp::ship_ai_attackmove_step_009e8820(elapsed, attack);
            owner_.done("ShipAiState::attackmove_step", 0x009e8820u);
            ++owner_.summary.state_steps_concrete;
            ++row_.state_step_real;
            ++owner_.summary.state_steps_real;
            apply_ai_drive();
            return;
        }
        // Every remaining leaf's step was named by docs/SHIP_AI_STATES.md as a
        // vtable slot and its body was not read, so the step is a record with
        // its own address. `sub_attack`'s vtable was not read at all.
        ++owner_.summary.state_steps_recorded;
        apply_ai_drive();
        if (state == nullptr || state->step == 0u) {
            owner_.record_slot("ShipAiState::step_vtable0c", "00d21598+vtable0c");
            return;
        }
        char method[64];
        std::snprintf(method, sizeof(method), "ShipAiState::%s_step", state->name);
        owner_.record(method, state->step);
        static_cast<void>(elapsed);
    }
    // --ai-drive was milestone 2o's labelled diagnostic stand-in for the state
    // steps that produced no desired throttle. Milestone 2r retires it: all
    // four states of this mission now form their own pair.
    void apply_ai_drive() {
        // Milestone 2r: nothing. The switch is retired; see set_ai_drive.
    }

    float state_interval_vtable28() override {
        const StateDescriptor* state = state_for_ai_offset(ctl_.active_state_ai_offset);
        const bool cruise = state != nullptr && state->interval == kCruiseIntervalGetter;
        owner_.done("ShipAiState::replan_interval_vtable28",
            cruise ? kCruiseIntervalGetter : kSharedIntervalGetter);
        const float ticks = cruise ? kCruiseIntervalTicks : kSharedIntervalTicks;
        row_.replan_interval = ticks * bsp::kShipAiIntervalScale;
        return ticks;
    }
    void replan_finish_009ddbc0() override {
        owner_.record("ShipAi::replan_finish", 0x009ddbc0u);
    }
    void hold_009da0d0() override { owner_.record("ShipAi::hold", 0x009da0d0u); }
    void step_009eca20(float) override { owner_.record("ShipAi::step_009eca20", 0x009eca20u); }
    void step_009da6e0(float) override { owner_.record("ShipAi::step_009da6e0", 0x009da6e0u); }
    void step_009f0ea0(float seconds) override {
        // 009F51E4, chain slot 14, one slot before the sector refresh, so the
        // list the scan walks is aged and compacted first. Packet
        // cc_ai_sector_scan projected the pass; it runs here over the list this
        // process holds, which nothing appends to.
        const bsp::ShipAiNeighbourRefreshResult refresh
            = bsp::ship_ai_neighbour_list_refresh_009f0ea0(ctl_.neighbours, seconds,
                                                           ctl_.neighbours_expired);
        owner_.done("ShipAi::neighbour_list_refresh_009f0ea0", 0x009f0ea0u);
        // 009F1A25, the candidate walk's call into 009F0D20. The candidates come
        // from the world object's linked list at [[00E188A8]+19CCh], which
        // construct_world 004DE610 does not build here, and the node's own
        // footprint at +44h..+60h has no producer in this packet either.
        owner_.record("ShipAi::neighbour_list_add_009f0d20", 0x009f0d20u);
        ctl_.nav_block.neighbour_count_604 = refresh.survivors;
    }
    void step_009e04e0(float seconds) override {
        // 009F51F3, the chain slot that builds the 65-bin throttle profile at
        // blk+4h and the avoidance vector at blk+34Ch/+350h. Packet
        // cc_ai_clearance_profile projected it; the contact-track list at
        // blk+400h is the empty one 009E4653 left, so the walk finds nothing
        // and the profile keeps the bypass byte the constructor set at
        // 009E435F, which is what makes 009D6B40 a clamp and nothing more.
        bsp::ShipAiThrottleProfileInputs in{};
        in.reference_speed_3c4 = ctl_.obstacle.reference_speed_3c4;
        in.own_speed = owner_.units.unit_forward_speed_0092d730(index_);
        in.acceleration = owner_.units.unit_class_max_accel_0504(index_);
        in.hull_half_width = owner_.units.unit_half_width_09cc(index_);
        in.hull_beam = owner_.units.unit_hull_length_09c8(index_);
        {
            float x = 0.0f, y = 0.0f, z = 0.0f;
            owner_.units.unit_position_00fc(index_, x, y, z);
            in.position_x = x;
            in.position_z = z;
        }
        {
            float right[3] = {0.0f, 0.0f, 0.0f};
            float up[3] = {0.0f, 0.0f, 0.0f};
            float forward[3] = {0.0f, 0.0f, 0.0f};
            float translation[3] = {0.0f, 0.0f, 0.0f};
            if (owner_.units.unit_pose(index_, right, up, forward, translation)) {
                in.forward_x = forward[0];
                in.forward_z = forward[2];
                in.normal_x = -forward[2];
                in.normal_z = forward[0];
            }
            owner_.record("ShipAiThrottleProfile::hull_axes_009de2f0", 0x009de2f0u);
        }
        ThrottleProfileBinding profile(owner_, ctl_, index_);
        bsp::ship_ai_build_throttle_profile_009e04e0(ctl_.throttle_profile, in, seconds,
                                                     profile);
        owner_.done("ShipAi::build_throttle_profile_009e04e0", 0x009e04e0u);
        // The profile the drive's middle snaps the throttle with, and the
        // bypass byte that decides whether it does anything at all.
        ctl_.obstacle.profile = ctl_.throttle_profile.profile;
        ++row_.throttle_profiles;
        ++owner_.summary.throttle_profiles;
    }
    void step_009ef230() override {
        // 009F51FA, chain slot 15, one slot before the throttle ceiling at
        // 009F5248 whose tail runs 009F3F80, so the twelve sectors at blk+808h
        // are always one call fresh when the drive's middle reads them.
        ObstacleSectorRefresh::run(owner_, ctl_, index_);
        ++row_.sector_refreshes;
        ++owner_.summary.sector_refreshes;
    }
    bool navigate_009ed6b0(float seconds) override {
        DirectControlBinding binding(owner_, index_);
        const bool early_out
            = bsp::ship_ai_direct_control_arm_009ed6b0(ctl_.blk, seconds, binding);
        owner_.done("ShipAi::direct_control_arm", 0x009ed6b0u);
        // 009ED769..009ED779, three stores inside the per-step reset span that
        // src/ship_ai_states.cpp's projection of the arm does not model (it
        // covers 009ED74D..009ED780 and keeps only blk+330h). blk+2FDh and
        // blk+2FEh are cleared at the top of every tick, which is what makes
        // 009EF034 the only thing that can set the arrival latch for a
        // Navigate ship, and blk+340h is re-seeded from blk+3C8h.
        ctl_.arm_tail_ran = false;
        ctl_.tail.parked_2fd = false;        // 009ED772
        ctl_.tail.goal_reached_2fe = false;  // 009ED779
        ctl_.goal.flag_2fd = false;
        ctl_.goal.flag_2fe = false;
        ctl_.nav.look_ahead_340 = ctl_.nav.look_ahead_max_3c8;  // 009ED769
        owner_.done("ShipAi::per_tick_arrival_reset", 0x009ed779u);
        if (early_out) return true;
        // Milestone 2p. The rest of 009ED6B0 is two alternatives, not a
        // prologue and a tail (docs/SHIP_AI_GOAL_VECTOR.md, correction 4):
        // 009EDA28..009EE57B is the station-keeping arm and it leaves through
        // 009EE57B JMP 009EF206, so 009EE580 is reached only when that arm did
        // not run.
        if (ctl_.blk.flag_3a5 && !ctl_.flag_3a6) {
            // 009EDA34 / 009EDA41. blk+3A5h is the "another entity's controller
            // owns me" byte; its five writers (009D5B90, 009DA0D0, 009DA3B0,
            // 009DDBC0 and 009DE5B0) are all records here, two of them chain
            // steps of this same frame, so the byte is never set and this arm
            // is never entered. The whole span is one record with its address.
            owner_.record("ShipAi::station_keeping_arm", 0x009eda28u);
            ++row_.station_keeping;
            ++owner_.summary.station_keeping;
            return false;
        }
        // 009EE580..009EE670, projected by packet cc_ai_goal_vector: the path
        // plan refresh, the path point pick and the lateral-offset publish.
        bsp::ShipAiPathPickState pick{};
        pick.steering_mode_1c4 = static_cast<int>(ctl_.blk.mode);
        pick.astern = ctl_.blk.mode == bsp::ShipAiSteeringMode::NavigateAstern;
        float pose_x = 0.0f, pose_y = 0.0f, pose_z = 0.0f;
        owner_.units.unit_position_00fc(index_, pose_x, pose_y, pose_z);
        pick.pose_x_184 = pose_x;
        pick.pose_z_188 = pose_z;
        PathPickBinding path(owner_, ctl_, row_, index_);
        const bsp::ShipAiPathPickResult result = bsp::ship_ai_pick_path_point_009ee580(
            pick, seconds, ctl_.path_point, ctl_.speed_scale_39c, path);
        owner_.done("ShipAi::pick_path_point", 0x009ee580u);
        if (result.entered) {
            ++row_.path_picks;
            ++owner_.summary.path_picks;
        }
        // 009EE671..009EEAA2, the output block src/ship_ai_navigation.cpp
        // projects. Milestone 2p said the block was skipped when the record
        // came back without a node; that reads 009EE609 backwards. `JZ 009EE671`
        // JUMPS INTO the block: the only thing record+18h gates is the lateral
        // publish at 009EE66C. The block therefore runs on every pass the
        // 009EE59B / 009EE59F gate lets through, on whatever point 009E3C00
        // left in the record. A closed gate leaves through 009EE59F JZ 009EF206
        // and reaches neither.
        if (!result.entered) {
            owner_.record("ShipAi::navigation_arm_tail", 0x009eeaabu);
            return false;
        }
        bsp::ShipAiNavWaypoint waypoint{};
        waypoint.x = ctl_.path_point.point_x_08;            // 009EE671
        waypoint.z = ctl_.path_point.point_z_0c;            // 009EE694
        waypoint.next_x = ctl_.path_point.next_x_10;        // 009EE78B
        waypoint.next_z = ctl_.path_point.next_z_14;        // 009EE79D
        waypoint.more_path = ctl_.path_point.more_path_20;  // 009EE776
        waypoint.steer_enabled = ctl_.path_point.steer_enabled_21;  // 009EE801
        waypoint.side = static_cast<bsp::ShipAiNavTurnSide>(
            ctl_.path_point.direction_1c);                  // 009EE765
        bsp::ShipAiNavPose nav_pose{};
        nav_pose.x = pose_x;
        nav_pose.z = pose_z;
        NavArmBinding nav_host(owner_, ctl_, index_);
        const bsp::ShipAiNavResult nav = bsp::ship_ai_navigation_arm_009ee671(
            ctl_.blk, ctl_.nav, waypoint, nav_pose, nav_host);
        owner_.done("ShipAi::navigation_output_block", 0x009ee671u);
        ++row_.nav_output_blocks;
        ++owner_.summary.nav_output_blocks;
        if (nav.bearing_taken) {
            ++row_.nav_bearings;
            ++owner_.summary.nav_bearings;
        }
        row_.nav_distance_32c = nav.distance_to_waypoint;
        row_.nav_heading_324 = nav.heading_target;

        // 009EEAAB..009EF226, packet ship_ai_navigation_arm_tail, merged from
        // main at 4491d04f. It is the only code that latches blk+35Ch for a
        // ship in Navigate, the only writer of the approach ceiling blk+344h on
        // this arm, and the only writer of the arrival latch blk+2FEh after the
        // per-tick clear at 009ED779.
        bsp::ShipAiArmTailTuning tuning{};
        // blk+3C4h, the cached reference speed. The executable already fills it
        // for the obstacle middle from 009EC97B's inputs.
        tuning.reference_speed_3c4 = ctl_.obstacle.reference_speed_3c4;
        // [[blk+3FCh]+538h]+508h, `Retardation` out of the installed
        // VehicleClass row, the same divisor 009ED8EC uses.
        tuning.class_deceleration_508 = owner_.units.unit_retardation_0508(index_);
        owner_.done("ShipAiArmTail::class_deceleration_508", 0x009eed1au);
        // Milestone 2r: blk+3CCh, blk+3D4h, blk+3D8h and blk+604h are what
        // 009E4330 wrote when the brain record was built, not zeroes. The
        // constructor runs once per unit in register_units; 009E4568 and
        // 009E4537 / 009E453F are their only other writer's defaults, and
        // 009F0E69 / 009F114E move +604h every frame, which this process's
        // neighbour list does not, so the count stays the constructor's zero
        // and the traffic setback walk at 009EEAD5 stays shut.
        tuning.turn_distance_3cc = ctl_.nav_block.turn_circle_cruise_3cc;
        tuning.stop_radius_3d4 = ctl_.nav_block.stop_radius_3d4;
        tuning.start_radius_3d8 = ctl_.nav_block.start_radius_3d8;
        tuning.neighbour_count_604 = ctl_.nav_block.neighbour_count_604;
        owner_.done("ShipAiArmTail::nav_tuning_009e4330", 0x009e4330u);
        // [blk+3FCh]+9C8h, the full hull length 0081106E / 0081FA4D produce,
        // and the navigatorParams byte +21h.
        tuning.hull_radius_9c8 = owner_.units.unit_hull_length_09c8(index_);
        owner_.done("ShipAiArmTail::hull_radius_09c8", 0x009ef112u);
        tuning.keep_clear_of_traffic_21 = true;
        ctl_.tail.plan_reset_2fc = ctl_.plan_computing_2fc;
        ctl_.tail.leader_snapshot_3a6 = ctl_.flag_3a6;
        {
            const bsp::ShipAiPathPlanBlock& live
                = (ctl_.plan_front == 0) ? ctl_.plan_a : ctl_.plan_b;
            ctl_.tail.plan_search_state_1c = live.search_state;
        }
        // [ESP+43h] at 009EE6E5 / 009EE6EC: record+20h != 0 && blk+1E4h != 0.
        const bool goal_is_destination
            = waypoint.more_path && ctl_.goal.final_leg_1e4;
        ArmTailBinding tail_host(owner_, index_);
        const bsp::ShipAiArmTailResult tail = bsp::ship_ai_navigation_arm_tail_009eeaab(
            ctl_.blk, ctl_.tail, ctl_.nav, tuning, waypoint,
            bsp::ShipAiNavPose{ctl_.goal.goal_x_1dc, ctl_.goal.goal_z_1e0},
            nav_pose, goal_is_destination, seconds, tail_host);
        owner_.done("ShipAi::navigation_arm_tail", 0x009eeaabu);
        ctl_.arm_tail_ran = true;
        ++row_.arm_tails;
        ++owner_.summary.arm_tails;
        row_.throttle_limit_344 = tail.throttle_ceiling;
        row_.latched_direction_35c = static_cast<int>(tail.direction);
        if (tail.direction != bsp::ShipAiThrottleDirection::Stopped) {
            ++row_.arm_tail_latched;
            ++owner_.summary.arm_tail_latched;
        }
        if (tail.request_stop) {
            ++row_.arm_tail_stops;
            ++owner_.summary.arm_tail_stops;
        }
        if (ctl_.tail.goal_reached_2fe) {
            ctl_.goal.flag_2fe = true;   // 009EF034, the byte 009DA590 requires
            ++row_.arrival_latches;
            ++owner_.summary.arrival_latches;
        }
        ctl_.goal.flag_2fd = ctl_.tail.parked_2fd;   // 009EF045
        return false;
    }
    void publish_009f4d10(float seconds) override {
        PublishBinding publish(owner_, ctl_, index_);
        const bsp::ShipAiPublishResult result = bsp::ship_ai_publish_order_009f4d10(
            ctl_.blk.heading_target_324, ctl_.blk.distance_32c, ctl_.blk.distance_330,
            seconds, publish);
        ctl_.order.slots[result.slot_index] = result.slot;
        owner_.done("ShipAi::publish_order", 0x009f4d10u);
        ++row_.publishes;
        ++owner_.summary.publishes;
        row_.slot_index = result.slot_index;
        row_.slot_heading_44 = result.slot.heading_44;
        row_.slot_distance_40 = result.slot.distance_40;
        row_.slot_distance_48 = result.slot.distance_48;
        row_.slot_valid = result.slot.valid_4c;
        run_clearance_refresh(seconds);
    }
    void run_clearance_refresh(float seconds) {
        // 009F4D87, the unconditional call 009F4D10 makes into 009EF910.
        // Milestone 2q recorded it and left blk+37Ch at the zero a fresh block
        // carries, which made the danger ramp's ratio zero and pinned every
        // navigating ship at full danger. Packet cc_ai_clearance_profile read
        // the routine whole, so it runs here.
        bsp::ShipAiClearanceGeometry geometry{};
        // blk+1B4h and blk+3CCh, both written by 009E4330 at construction.
        geometry.sweep_half_angle = ctl_.nav_block.shoulder_angle_1b4;
        geometry.hull_radius = ctl_.nav_block.turn_circle_cruise_3cc;
        // blk+18Ch..+1B0h, the two shoulders and the hull forward, which
        // 009DE2F0 rebuilds inside the pre-step 009E0270. That routine is a
        // record here, so the four are the zeroes the constructor left. They
        // only place the pivot of the sweep; nothing in this run lowers the
        // clearance, so they decide nothing it measures.
        owner_.record("ShipAiClearance::shoulder_geometry_009de2f0", 0x009de2f0u);
        bsp::ShipAiClearanceSettings settings{};
        // settings+1D4h, +214h and +218h. 00424C40's block has no producer for
        // these three in this process; a zero refresh period makes 009EF91D's
        // countdown expire every tick, which is the most active reading.
        owner_.record("ShipAiClearance::settings_00424c40", 0x009ef948u);
        ctl_.clearance.heading_target_324 = ctl_.blk.heading_target_324;
        ctl_.clearance.path_length_330 = ctl_.blk.distance_330;
        ctl_.clearance.steering_mode_35c = static_cast<int>(ctl_.blk.direction);
        ctl_.clearance.category_3f0 = ctl_.nav_block.plan_state_3f0;
        ctl_.clearance.avoidance_enabled_3f4 = ctl_.nav_block.flag_3f4;
        ctl_.clearance.static_zone_present_a3c = false;
        ClearanceBinding clearance(owner_, index_);
        bsp::ship_ai_refresh_turn_clearance_009ef910(ctl_.clearance, geometry, settings,
                                                     seconds, clearance);
        owner_.done("ShipAi::refresh_turn_clearance_009ef910", 0x009ef910u);
        ++row_.clearance_refreshes;
        ++owner_.summary.clearance_refreshes;
        row_.clearance_37c = ctl_.clearance.clearance_37c;
    }
    void tail_009da8d0(float) override { owner_.record("ShipAi::tail_009da8d0", 0x009da8d0u); }
    void tail_009f4da0(float seconds) override {
        // 009F5248, chain slot 16. 009F4DA0's own body is the throttle ceiling
        // on brain+34Ch / +350h, which no packet has reconstructed, so it stays
        // a record with its own address. It is not on the way to anything here:
        // the routine's ONLY exit is the tail call at 009F50C6 with
        // ECX = brain+8h = blk, taken whether or not brain+0B38h skipped the
        // body (009F4DAF), so 009F3F80 runs on every step either way.
        owner_.record("ShipAi::throttle_ceiling", 0x009f4da0u);
        owner_.drive_order_ring_009f3f80(index_, ctl_, row_, seconds);
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// Milestone 2p: 0071F290, the command controller's own per-frame update
// ---------------------------------------------------------------------------
//
// docs/DIRECTOR_UPDATE_ARMS.md (packet cc2_director_update_arms) reads the
// whole of it: seven arms in order, of which arm 3 is the auto-target hold
// countdown 0071F314 that 0071DF70 tests, and arm 7 ticks
// [controller+38h]->vtable[4](dt), which on a director is 009F5DA0, then
// vtable[7Ch], which is 00836920. Milestone 2n supplied where the think runs;
// this milestone takes that placement from the routine instead.

class ControllerUpdateBinding final : public bsp::CommandControllerUpdateHost {
public:
    ControllerUpdateBinding(GameShipAiHost::Impl& owner,
                            GameShipAiHost::Impl::Controller& ctl, GameShipAiRow& row,
                            std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}

    void reset_path_vector() override {
        // 0071F2D1..0071F2F3, the shared zero vector at 00F87574 into path
        // object 0's +30h/+34h/+38h. The +1A4h path array is not built here.
        owner_.record("CommandController::reset_path_vector", 0x0071f2d1u);
    }
    bool begin_command(int) override {
        // vtable[78h], which on a director is 00835C70. Milestone 2l already
        // runs that body at its own site, right after the push that made a
        // command current; the two arms here are gated on the accepted bytes
        // at +44h / +4Ch, which this process does not model, so neither arm
        // runs and the call is recorded rather than made twice.
        owner_.record("CommandController::begin_command", 0x00835c70u);
        return true;
    }
    void raise_override_stage(int) override {
        owner_.record("CommandController::raise_override_stage", 0x0071d9e0u);
    }
    void raise_queue_stage(int) override {
        owner_.record("CommandController::raise_queue_stage", 0x0071d810u);
    }
    void step_auto_target(float frame_delta) override;
    void step_commands() override {
        // vtable[7Ch] = 00836920 BSP_WeaponDirector_Step. Milestone 2m runs
        // that body once per unit per fixed step from GameUnitsHost, which is
        // where the stage ladder, the `stop` arm and the idle tail already run.
        // Running it here as well would step every director twice, so the arm
        // is logged at the site it actually runs and not repeated.
        owner_.done("CommandController::step_commands", 0x00836920u);
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// bsp::BotFireTargetHost, the sixteen call sites of 009F5DA0
// ---------------------------------------------------------------------------

class TargetBinding final : public bsp::BotFireTargetHost {
public:
    TargetBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                  GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}

    bool controller_belongs_to_another(void*) override {
        owner_.record("AutoTarget::controller_belongs_to_another", 0x007788b0u);
        return false;
    }
    void* director_command_slot() override {
        // 009F5DD0, [director+54h]. 0071BE48 establishes that the first command
        // slot holds the command object itself, which is what the follow
        // comparison at 009F5DE0 needs, so this process's own slot 0 answers it.
        owner_.done("AutoTarget::director_command_slot", 0x009f5dd0u);
        const std::uint32_t command = owner_.units.director_current_command_0071be40(index_);
        return reinterpret_cast<void*>(static_cast<std::uintptr_t>(command));
    }
    void release_controller(void*, int) override {
        owner_.record("AutoTarget::release_controller", 0x0077c980u);
    }
    bool unit_suppresses_targeting(void*) override {
        owner_.done("AutoTarget::unit_player_controlled", 0x009f5e06u);
        return owner_.units.unit_player_controlled_0184(index_);
    }
    bool selection_enabled() override {
        // 009F5610: [director+3Dh] must be set and, when the slot is non-null,
        // its vtable[0Ch] must not answer 1 or 2. bsp::WeaponDirectorState's
        // allow_move default is the byte 008363E0 writes for a unit of neither
        // gated class, which is every ship of this mission.
        const bsp::WeaponDirectorState defaults{};
        owner_.done("AutoTarget::director_allow_move", 0x009f5614u);
        const std::uint32_t command = owner_.units.director_current_command_0071be40(index_);
        int kind = 0;
        if (command != 0u) {
            owner_.record_slot("AutoTarget::command_slot_kind", "00e08f70+vtable0c");
        }
        const bool enabled = bsp::auto_target_selection_enabled_009f5610(
            defaults.allow_move, command != 0u, kind);
        owner_.done("AutoTarget::selection_enabled", 0x009f5610u);
        return enabled;
    }
    void send_command_state(void*, int) override {
        owner_.record("AutoTarget::send_command_state", 0x0071d9e0u);
    }
    void* director_current_target() override {
        owner_.done("AutoTarget::director_current_target", 0x009f5e2cu);
        return reinterpret_cast<void*>(static_cast<std::uintptr_t>(ctl_.fire_target));
    }
    bool director_target_locked() override {
        // [director+23Ch], the target-change gate 00835860 writes. Nothing in
        // this process sets it, and its only writer is the setter itself.
        owner_.record("AutoTarget::director_target_locked", 0x009f5e37u);
        return false;
    }
    int director_command_state() override {
        owner_.record("AutoTarget::director_command_state", 0x009f5e69u);
        return 0;
    }
    void* build_command_target(void* entity, float) override {
        // 00465080, read for this packet: it fills a SceneCommandTarget with the
        // zero vector at 00F87574, kind 1 and the entity's +174h object id, or
        // kind 0 for a null entity. The tick only passes the result on, so the
        // host records the routine and carries the entity through it.
        owner_.record("AutoTarget::build_command_target", 0x00465080u);
        return entity;
    }
    bool command_accepts_target(std::uint32_t, void*) override {
        owner_.record("AutoTarget::command_accepts_target", 0x0071d6d0u);
        return false;
    }
    bsp::AutoTargetScanResult scan_party_list() override {
        ++row_.target_scans;
        ++owner_.summary.scans;
        // 009F65E0 builds the priority list from the owner's own kinds, over the
        // recovered class chain this process already answers IsKindOf with.
        bsp::AutoTargetOwnerKinds kinds{};
        kinds.kind_0e = owner_.units.unit_is_kind_of(index_, 0x0e);
        kinds.kind_07 = owner_.units.unit_is_kind_of(index_, 0x07);
        kinds.kind_0a = owner_.units.unit_is_kind_of(index_, 0x0a);
        kinds.kind_0d = owner_.units.unit_is_kind_of(index_, 0x0d);
        kinds.kind_08 = owner_.units.unit_is_kind_of(index_, 0x08);
        kinds.kind_0b = owner_.units.unit_is_kind_of(index_, 0x0b);
        kinds.kind_09 = owner_.units.unit_is_kind_of(index_, 0x09);
        kinds.kind_0c = owner_.units.unit_is_kind_of(index_, 0x0c);
        const bsp::AutoTargetSearchConfig config
            = bsp::auto_target_build_priority_009f65e0(kinds);
        owner_.done("AutoTarget::build_priority_list", 0x009f65e0u);
        // The list itself is the recon slot 008053C0 returns for the owner's
        // party and the intrusive chain at slot+0DE8h. Nothing in this process
        // fills that chain, so the executable hands the recovered scan the
        // created instances of the opposing party and records the producer.
        owner_.record("AutoTarget::party_recon_slot", 0x008053c0u);
        owner_.record_slot("AutoTarget::candidate_owner_vtable140", "00cfc3d0+vtable140");
        const GameUnitRow* self = owner_.units.unit_row(index_);
        std::vector<bsp::AutoTargetCandidate> candidates;
        std::vector<std::size_t> candidate_index;
        if (self != nullptr) {
            for (std::size_t other = 0; other < owner_.units.count(); ++other) {
                if (other == index_) continue;
                const GameUnitRow* row = owner_.units.unit_row(other);
                if (row == nullptr || !row->active || row->party == self->party) continue;
                bsp::AutoTargetCandidate candidate{};
                candidate.entity = reinterpret_cast<void*>(
                    static_cast<std::uintptr_t>(other + 1));
                candidate.passes_candidate_gate
                    = owner_.units.unit_is_kind_of(other, bsp::kEntityKindCandidateGate);
                candidate.skip_all_entries = owner_.units.unit_flag_005d(other);
                candidate.entity_kind_matches = -1;
                for (std::size_t entry = 0; entry < config.priority.size(); ++entry) {
                    if (owner_.units.unit_is_kind_of(other,
                            config.priority[entry].entity_kind)) {
                        candidate.entity_kind_matches = static_cast<int>(entry);
                        break;
                    }
                }
                const double dx = static_cast<double>(row->position[0]) - self->position[0];
                const double dy = static_cast<double>(row->position[1]) - self->position[1];
                const double dz = static_cast<double>(row->position[2]) - self->position[2];
                candidate.distance
                    = static_cast<float>(std::sqrt(dx * dx + dy * dy + dz * dz));
                candidate.owner_can_engage = bsp::auto_target_owner_can_engage_009f59f0(
                    owner_.units.unit_is_kind_of(other, bsp::kEntityKindNeedsSpecialWeapon),
                    false, {});
                candidates.push_back(candidate);
                candidate_index.push_back(other);
            }
        }
        const bsp::AutoTargetScanResult scan
            = bsp::auto_target_scan_009f5d30(config, candidates);
        owner_.done("AutoTarget::scan_party_list", 0x009f5d30u);
        owner_.done("AutoTarget::score_candidate", 0x009f5b70u);
        if (scan.best != nullptr) {
            const std::size_t chosen
                = static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(scan.best)) - 1;
            const GameUnitRow* row = owner_.units.unit_row(chosen);
            row_.fire_target = row != nullptr ? row->name : std::string();
            row_.fire_target_score = scan.best_score;
        }
        return scan;
    }
    void* resolve_command_target_object() override {
        owner_.record("AutoTarget::resolve_command_target", 0x00521ea0u);
        return nullptr;
    }
    bool director_accepts_new_target() override {
        // Milestone 2p. Packet cc2_director_target_gate read 0071DF70 and its
        // three writers whole (docs/DIRECTOR_TARGET_GATE.md) and corrected
        // milestone 2n in two places, so this is no longer a record.
        //
        // Rule 1, 0071DF70..0071DF7E: the compare against the 0.0f at 00D7A218
        // runs the other way round from what 2n said. 0071DF7C is `JBE` to the
        // slot scan and the fall-through at 0071DF7E is `XOR AL,AL / RET`, so a
        // hold ABOVE 0.0f is the rejection and at most 0.0f passes. The field
        // has three producers: 00720225 stores -1.0f in the command controller
        // base constructor 00720180, which 008363E0 reaches at 00836403 with
        // ECX still the director; 0071F314 does `hold -= dt` only while the
        // value is at or above 0.0f, so the sentinel never moves; and 00817031
        // stores 3.0f in the `cleartarget` arm of 00816E30, which this mission
        // never issues. Every director here therefore holds -1.0f and passes.
        //
        // Rule 2 and 3, 0071DF83..0071DFC2: count the leading occupied slots at
        // director+54h with stride 1Ch, stop at the first null, and reject when
        // any of their commands answers category 1 (gunnery) or 2 (weapon run)
        // from vtable[0Ch]. That is exactly the set 008358D0 forces the fire
        // target for, so a queued attack command owns the target by design.
        const float hold = owner_.units.director_target_hold_0040(index_);
        owner_.done("AutoTarget::director_target_hold_0040", 0x0071df75u);
        row_.director_hold_0040 = hold;
        ++row_.target_gate_tests;
        if (hold > 0.0f) {
            row_.target_blocked = "0071df7e hold armed";
            return false;
        }
        int categories[16] = {};
        const int count = owner_.units.director_leading_slot_categories_0071df83(index_,
            categories, 16);
        owner_.done("AutoTarget::director_slot_categories", 0x0071df83u);
        owner_.done("AutoTarget::command_category_vtable0c", 0x0071dfaeu);
        const std::uint32_t slot0 = owner_.units.director_slot_command(index_, 0);
        row_.slot0_command = owner_.units.command_name_of(slot0);
        row_.slot0_category = (count > 0) ? categories[0] : -1;
        for (int i = 0; i < count; ++i) {
            if (categories[i] != 1 && categories[i] != 2) continue;
            char text[64];
            std::snprintf(text, sizeof(text), "0071dfc2 slot %d category %d", i,
                categories[i]);
            row_.target_blocked = text;
            if (!owner_.logged_accept_gate) {
                owner_.logged_accept_gate = true;
                owner_.log.notef("automatic target selection now stops at 0071dfc2, not at the "
                    "hold: director+40h is the constructed -1.0f and passes, and the slot scan "
                    "rejects because a leading command slot carries category %d, the gunnery "
                    "and weapon-run set 008358d0 forces the fire target for "
                    "(docs/DIRECTOR_TARGET_GATE.md)", categories[i]);
            }
            return false;
        }
        ++owner_.summary.units_accepting_new_target;
        owner_.done("AutoTarget::director_accepts_new_target", 0x0071df70u);
        row_.target_blocked.clear();
        return true;
    }
    void issue_command(std::uint32_t, void*) override {
        owner_.record("AutoTarget::issue_attackmove", 0x0071d980u);
        ++row_.attackmove_issues;
        ++owner_.summary.attackmove_issues;
    }
    void set_fire_target(void* entity, bool) override {
        owner_.record("AutoTarget::set_fire_target", 0x00835860u);
        ctl_.fire_target = static_cast<bsp::NativeHandle>(
            reinterpret_cast<std::uintptr_t>(entity));
        ++row_.fire_target_sets;
        ++owner_.summary.fire_target_sets;
    }
    bool director_allow_move() override {
        const bsp::WeaponDirectorState defaults{};
        return defaults.allow_move;
    }
    int command_slot_kind(void*) override {
        owner_.record_slot("AutoTarget::command_slot_kind", "00e08f70+vtable0c");
        return 0;
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

}  // namespace

// ---------------------------------------------------------------------------
// Chain slot 16: 009F4DA0 -> 009F3F80, and 009F3F80's tail into the order ring
// ---------------------------------------------------------------------------
//
// COVERAGE. 009F3F80's body is 009F3F80-009F4D06 and this runs three parts of
// it and records the rest:
//   009F3FEB..009F3FF2  the early-out gate on blk+3F5h                projected
//   009F3FF8..009F402E  the ring slot under the write cursor          projected
//   009F4034..009F40C6  the speed, the heading, the astern flip and
//                       the heading error the rudder law is given     projected
//   009F40CA..009F44E3  the ten writers of blk+1D0h                   RECORD
//   009F44E4..009F44FC  the mode gate and the rudder law's store       projected
//   009F4502..009F4B98  the obstacle and reverse-manoeuvre arms,
//                       including the second rudder store at 009F46A0  RECORD
//   009F4B99..009F4D04  the hop                                       projected
// The recorded spans are the ones docs/SHIP_AI_THROTTLE_TO_RING.md read for the
// fields they write and did not project. They can change blk+1D0h and blk+1D4h
// before the hop reads them, so what this executable puts in the ring is the
// hop applied to the state's own desired pair, not to whatever those arms would
// have made of it. That is stated in the milestone and is a boundary, not a
// result.

// Defined here because it builds a TargetBinding, which the anonymous namespace
// above declares after ControllerUpdateBinding.
void ControllerUpdateBinding::step_auto_target(float frame_delta) {
    // 0071F395, [controller+38h]->vtable[4](dt) = 009F5DA0, the bot's own
    // fire-target think. Its countdown is what turns a per-frame call into a
    // once-a-second think.
    const float before = ctl_.target.think_countdown;
    TargetBinding target(owner_, ctl_, row_, index_);
    bsp::auto_target_tick_009f5da0(target, ctl_.target, nullptr, frame_delta);
    owner_.done("AutoTarget::tick", 0x009f5da0u);
    if (!(frame_delta < before)) {
        ++row_.target_thinks;
        ++owner_.summary.thinks;
    }
}

void GameShipAiHost::Impl::run_navigation_goal_009de050(Controller& ctl, GameShipAiRow& row,
    std::size_t index, float goal_x, float goal_z, bool keep_mode, bool final_leg) {
    // The three fields ShipAiControlBlock and ShipAiGoalPlan both describe, in
    // before 009DE050 runs and out after it: +1C4h the steering mode, +1C8h the
    // throttle hold and +1CCh the requested direction.
    ctl.goal.mode = ctl.blk.mode;
    ctl.goal.throttle_hold_1c8 = ctl.blk.throttle_hold_1c8;
    ctl.goal.requested_direction = ctl.blk.requested_direction;
    // 009DE17C and 009DE189 read the pose the per-frame chain leaves on the
    // block at +184h / +188h. This process has no recovered writer for that
    // pair, so it supplies the unit's own world position, which is what the
    // plan length is meant to measure from, and says so.
    float x = 0.0f, y = 0.0f, z = 0.0f;
    units.unit_position_00fc(index, x, y, z);
    record("ShipAiGoal::block_pose_0184", 0x009de17cu);
    ctl.goal.pose_x_184 = x;
    ctl.goal.pose_z_188 = z;

    const float planned_x = ctl.goal.planned_x_1e8;
    const float planned_z = ctl.goal.planned_z_1ec;
    GoalBinding goal(*this, ctl);
    bsp::ship_ai_set_navigation_goal_009de050(ctl.goal, goal_x, goal_z, keep_mode, final_leg,
                                              goal);
    done("ShipAiState::set_navigation_goal", 0x009de050u);

    ctl.blk.mode = ctl.goal.mode;
    ctl.blk.throttle_hold_1c8 = ctl.goal.throttle_hold_1c8;
    ctl.blk.requested_direction = ctl.goal.requested_direction;

    ++row.goal_sets;
    ++summary.goal_sets;
    if (ctl.goal.planned_x_1e8 != planned_x || ctl.goal.planned_z_1ec != planned_z) {
        ++row.goal_replans;
        ++summary.goal_replans;
    }
    row.goal_x = ctl.goal.goal_x_1dc;
    row.goal_z = ctl.goal.goal_z_1e0;
    row.goal_final_leg = ctl.goal.final_leg_1e4;
}

void GameShipAiHost::Impl::drive_order_ring_009f3f80(std::size_t index, Controller& ctl,
    GameShipAiRow& row, float seconds) {
    done("ShipAi::drive_order_ring", 0x009f3f80u);
    // 009F3FEB CMP byte [ESI+3F5h],0 / 009F3FF2 JNZ 009F4D00: the routine's one
    // early out, and it jumps past the hop to the epilogue. blk+3F5h is the
    // byte 009ED6B0 returns on (bsp::ShipAiControlBlock::early_out_3f5) and
    // nothing in this process writes it, so the gate is open. The count is
    // reported so a later reader does not have to take that on trust.
    if (ctl.blk.early_out_3f5) {
        done("ShipAiRing::early_out_3f5", 0x009f3ff2u);
        ++row.ring_gated_3f5;
        ++summary.ring_gated_3f5;
        return;
    }
    done("ShipAiRing::early_out_3f5", 0x009f3ff2u);

    // 009F3FF8..009F402E: two independent loads of [unit+97Ch], each shifted
    // left by 5. 009F400D reads the slot's +83Ch (the rudder) and 009F4025 its
    // +838h (the throttle). This is the slot the hop slews toward and then
    // writes back, and it is NOT the 84-byte AI order slot 009F4D10 published.
    const float previous_throttle = units.unit_ring_write_slot_throttle(index);
    const float previous_rudder = units.unit_ring_write_slot_rudder(index);
    done("ShipAiRing::write_slot_read", 0x009f400du);

    // 009F4034, 0092D730 with ECX = [unit+1018h]. The routine reads the hull's
    // signed forward speed once at its head and uses it for two byte flags.
    const float speed = units.unit_forward_speed_0092d730(index);
    done("ShipAi::drive_speed", 0x0092d730u);
    // 009F4072..009F407B, CALL [[unit]+50h]: the unit's own heading getter.
    float heading = units.unit_heading_radians(index);
    record_slot("ShipAi::drive_heading_vtable50", "00cfc3d0+vtable50");
    // 009F4081 CMP [ESI+35Ch],2 / 009F409E 00438AA0(heading, [00D7A264]): a
    // ship latched astern is steered about the reciprocal of its own heading.
    // 00D7A264 is the 3.14159265f bsp/unit_state_message.hpp names for the
    // MT_SHIP_SYNC heading range, which is the same constant.
    if (ctl.blk.direction == bsp::ShipAiThrottleDirection::Astern) {
        heading = bsp::wrapped_angle_add_00438aa0(heading,
            bsp::kUnitStateMessageHeadingRange);
        done("ShipAi::drive_astern_heading", 0x00438aa0u);
    }
    // 009F40BB, 00438B10(blk+324h, heading): the heading error, the one value
    // the rudder law is driven with.
    const float error = bsp::wrapped_angle_subtract_00438b10(ctl.blk.heading_target_324,
        heading);
    done("ShipAi::drive_heading_error", 0x00438b10u);
    row.heading_error = error;

    // Milestone 2p. 009F40CA..009F4B98, the whole middle, is no longer one
    // record: packet cc_ai_obstacle_tables projected it operation for operation
    // as bsp::ship_ai_drive_order_ring_middle_009f40ca. It owns the four
    // direction booleans, the danger ramp at blk+0A84h, the throttle through
    // 009EC7C0 and the 65-bin profile at blk+4h, the rudder law at 009F44F7
    // with its 009F44E4 gate, the obstacle sectors and the escape manoeuvre.
    bsp::ShipAiObstacleFrame frame{};
    frame.dt = seconds;
    frame.body_axis_speed = speed;
    frame.heading = heading;
    frame.heading_error = error;
    frame.unit_half_width_9cc = units.unit_half_width_09cc(index);
    record("ShipAiObstacle::unit_half_width_09cc", 0x009f4174u);
    bool settings_loaded = false;
    const bsp::ShipAiAutoThrustSettings& settings = units.auto_thrust_settings(settings_loaded);
    if (settings_loaded) {
        done("ShipAiObstacle::auto_thrust_block_00424c40", 0x00424c40u);
    } else {
        record("ShipAiObstacle::auto_thrust_block_00424c40", 0x00424c40u);
    }
    const bsp::ShipAiThrottleCeilingInputs ceiling = units.throttle_ceiling_inputs(index);
    done("ShipAiObstacle::throttle_ceiling_inputs", 0x009ec97bu);
    // blk+3C4h, the cached reference speed 009EC9AB divides the cruise cap by
    // and 009F478B scales the neighbour closing test with.
    ctl.obstacle.reference_speed_3c4 = ceiling.reference_speed;
    ctl.obstacle.position_x = 0.0f;
    ctl.obstacle.position_z = 0.0f;
    {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        units.unit_position_00fc(index, x, y, z);
        ctl.obstacle.position_x = x;
        ctl.obstacle.position_z = z;
    }
    // blk+37Ch, the clearance the danger ramp divides by unit+9CCh. Its only
    // writer is 009EF910, which 009F4D10 called one chain slot earlier and
    // which milestone 2r runs, so the field carries what that routine left.
    ctl.obstacle.clearance_37c = ctl.clearance.clearance_37c;
    done("ShipAiObstacle::clearance_37c_producer", 0x009ef910u);
    // blk+344h and blk+348h. 009F4DBC stores 1.0f into blk+348h unconditionally
    // before the 009F4DC1 early out; blk+344h's own arms inside 009F4DA0,
    // 009F4DC7..009F50BA, are still the record milestone 2o left. Milestone 2q
    // adds the other writer: 009EEF0A, in the navigation arm's tail, which runs
    // earlier in the same tick for a ship in Navigate, so the ceiling the drive
    // caps against is the one the approach computed.
    ctl.obstacle.rudder_limit_348 = 1.0f;
    done("ShipAiObstacle::rudder_limit_348", 0x009f4dbcu);
    if (ctl.arm_tail_ran) {
        ctl.obstacle.throttle_limit_344 = ctl.tail.throttle_ceiling_344;
        done("ShipAiObstacle::throttle_ceiling_344", 0x009eef0au);
    } else {
        ctl.obstacle.throttle_limit_344 = 1.0f;
        record("ShipAiObstacle::throttle_ceiling_344", 0x009f4dc7u);
    }
    ObstacleBinding obstacle(*this, index);
    obstacle.set_direction(ctl.blk.direction);
    const float rudder_before = ctl.blk.desired_rudder;
    bsp::ship_ai_drive_order_ring_middle_009f40ca(ctl.blk, ctl.obstacle, frame, settings,
                                                  ceiling, obstacle);
    done("ShipAi::drive_order_ring_body", 0x009f40cau);
    ++row.middle_runs;
    ++summary.middle_runs;
    row.danger_a84 = ctl.obstacle.danger_a84;
    row.throttle_limit_344 = ctl.obstacle.throttle_limit_344;
    row.turn_assist_load_102c = units.turn_assist_load_102c(index);
    row.rudder_law_calls += obstacle.calls();
    summary.rudder_law_calls += obstacle.calls();
    static_cast<void>(rudder_before);

    // 009F4B99..009F4D04, the hop, reconstructed operation for operation by
    // packet cc_ai_throttle_ring: the deadband that zeroes blk+1D4h, the slew of
    // both desired values toward the slot by at most dt * 1.5, and the two
    // setters that put them back.
    RingHopBinding hop_host(*this, row, index);
    const bsp::ShipAiRingHop hop = bsp::ship_ai_order_ring_hop_009f4b99(ctl.blk,
        previous_throttle, previous_rudder, seconds, hop_host);
    done("ShipAiRing::order_ring_hop", 0x009f4b99u);
    ++row.ring_hops;
    ++summary.ring_hops;
    if (hop.rudder_zeroed) {
        ++row.rudder_deadbands;
        ++summary.rudder_deadbands;
    }
}

// ---------------------------------------------------------------------------

GameShipAiHost::GameShipAiHost(GameHostLog& log, GameUnitsHost& units)
    : impl_(std::make_unique<Impl>(log, units)) {}
GameShipAiHost::~GameShipAiHost() = default;

namespace {

// Milestone 2r: bsp::ShipAiNavBlockCtorHost, the six call sites of 009E4330.
// 009F1160 builds the brain record once per ship and 009F118D runs this on
// `brain+8h`, so the executable runs it once per created unit at registration,
// before any state object exists.
class NavBlockCtorBinding final : public bsp::ShipAiNavBlockCtorHost {
public:
    NavBlockCtorBinding(GameShipAiHost::Impl& owner, std::size_t index)
        : owner_(owner), index_(index) {}

    bsp::ShipAiNavBlockSteeringDefaults seed_steering_009dfcb0() override {
        // 009E43CC with ECX = blk+1C4h. Only the blk+3C4h..+3E4h window of
        // 009DFCB0 is projected; the constructor overwrites all of it except
        // +3DCh and +3E0h, so the window is what this process needs.
        owner_.done("ShipAiNavBlock::seed_steering_009dfcb0", 0x009dfcb0u);
        owner_.record("ShipAiNavBlock::seed_steering_unprojected", 0x009dfcb0u);
        return bsp::ship_ai_nav_block_steering_defaults_009dfcb0();
    }
    bool unit_answers_class_5c(int class_id) override {
        owner_.done("ShipAiNavBlock::unit_class_5c", 0x009e448eu);
        return owner_.units.unit_is_kind_of(index_, class_id);
    }
    float class_turn_circle_radius_0082e960(float throttle_fraction) override {
        return owner_.units.unit_class_turn_circle_radius_0082e960(index_,
            throttle_fraction);
    }
    float sqrt_00bf7030(float value) override {
        owner_.done("ShipAiNavBlock::sqrt_00bf7030", 0x00bf7030u);
        return value > 0.0f ? std::sqrt(value) : 0.0f;
    }
    float uniform_float_00bd2f10(float low, float high) override {
        // 009E465F. 00BD2F10's random stream has no producer in this process -
        // every other site in this file records it - and 009E4669 stores the
        // negated draw in blk+148h, a field with no traced reader. The low end
        // is taken and the call recorded, which is what the approach point's
        // own draw at 009F1BC0 does.
        owner_.record("ShipAiNavBlock::uniform_00bd2f10", 0x00bd2f10u);
        static_cast<void>(high);
        return low;
    }
    void build_sector_shapes_009e0270(bsp::ShipAiNavBlockFields& fields,
                                      std::uint32_t raw_argument) override {
        // 009E46A9. 009E0270 rewrites blk+168h, blk+3C4h and the twelve sector
        // shapes and calls 009DE2F0 for blk+19Ch / +1A0h. The per-step chain
        // slot that would re-run it (009EF230's own refresh) is wired; the
        // construction-time call is recorded with its own address, as
        // `ShipAi::pre_step` already records the same routine.
        static_cast<void>(fields);
        static_cast<void>(raw_argument);
        owner_.record("ShipAiNavBlock::build_sector_shapes_009e0270", 0x009e0270u);
    }

private:
    GameShipAiHost::Impl& owner_;
    std::size_t index_;
};

}  // namespace

void GameShipAiHost::register_units() {
    Impl& host = *impl_;
    const std::size_t count = host.units.count();
    host.controllers.assign(count, Impl::Controller{});
    host.rows.assign(count, GameShipAiRow{});
    for (std::size_t index = 0; index < count; ++index) {
        const GameUnitRow* row = host.units.unit_row(index);
        host.rows[index].unit_index = index;
        host.rows[index].unit = row != nullptr ? row->name : std::string();
        host.rows[index].state = "none";
        // Milestone 2r: 009F118D, the navigation block constructor, once per
        // brain record. Its five turn fields are the inputs the arm tail and
        // the arrival release test read; milestone 2q had them at zero.
        {
            Impl::Controller& ctl = host.controllers[index];
            bsp::ShipAiNavBlockUnitInputs in{};
            in.present = row != nullptr;
            in.handle = static_cast<std::uint32_t>(index) + 1u;
            in.hull_length_09c8 = host.units.unit_hull_length_09c8(index);
            in.ship_class.max_rot_angle_04f8
                = host.units.unit_class_max_rot_angle_04f8(index);
            in.ship_class.max_speed_0500 = host.units.unit_class_max_speed_0500(index);
            in.ship_class.turn_radius_0520 = host.units.unit_class_turn_radius_0520(index);
            NavBlockCtorBinding ctor(host, index);
            ctl.nav_block = bsp::ship_ai_nav_block_ctor_009e4330(in, ctor);
            ctl.nav_block_built = true;
            host.done("ShipAiNavBlock::construct_009e4330", 0x009e4330u);
            ++host.summary.nav_blocks;
            // The three fields the block carries into the navigation arm's own
            // state: blk+3C8h is the look-ahead ceiling 009ED769 re-seeds
            // blk+340h from every tick, and blk+3D0h the heading window.
            ctl.nav.look_ahead_max_3c8 = ctl.nav_block.turn_circle_full_3c8;
            ctl.nav.look_ahead_340 = ctl.nav_block.look_ahead_340;
            ctl.nav.turn_window_3d0 = ctl.nav_block.yaw_rate_3d0;
            ctl.blk.early_out_3f5 = ctl.nav_block.early_out_3f5;
            host.rows[index].nav_turn_circle_3c8 = ctl.nav_block.turn_circle_full_3c8;
            host.rows[index].nav_turn_circle_3cc = ctl.nav_block.turn_circle_cruise_3cc;
            host.rows[index].nav_yaw_floor_3d0 = ctl.nav_block.yaw_rate_3d0;
            host.rows[index].nav_stop_radius_3d4 = ctl.nav_block.stop_radius_3d4;
            host.rows[index].nav_start_radius_3d8 = ctl.nav_block.start_radius_3d8;
            host.rows[index].nav_hull_length_9c8 = in.hull_length_09c8;
            host.rows[index].hull_mass_00b0 = host.units.unit_hull_mass_00b0(index);
            host.rows[index].hull_material = host.units.unit_hull_material(index);
        }
        // 009F6A20 seeds the think countdown with the negation of a random draw
        // in [0, 1) so the once-a-second thinks of different directors fall on
        // different frames. This process has no 00BD2F10 on this path, so the
        // phase is the unit's own index spread over the interval and is the
        // executable's value rather than a recovered one.
        host.controllers[index].target.think_countdown
            = -static_cast<float>(index % 20) * 0.05f;
        if (row != nullptr && !host.units.unit_player_controlled_0184(index)) {
            ++host.summary.ai_owned;
        }
    }
    host.summary.units = count;
    host.log.notef("ship AI controllers: %zu built, one per created instance. 009f50e0 has no "
        "Ghidra function and no caller, and 009f5da0 is reached only through the derived "
        "vtable slot at 00d21b4c, so this process runs both once per unit per fixed "
        "simulation step, before the motion pass whose head at 00825f2c consumes the order "
        "slot 009f4d10 published", count);
}

void GameShipAiHost::controller_step(float seconds) {
    Impl& host = *impl_;
    if (host.controllers.empty()) return;
    ++host.steps;
    for (std::size_t index = 0; index < host.controllers.size(); ++index) {
        Impl::Controller& ctl = host.controllers[index];
        GameShipAiRow& row = host.rows[index];
        if (!host.units.unit_active(index)) continue;
        ControllerBinding binding(host, ctl, row, index);
        const bool ran = bsp::ship_ai_controller_step_009f50e0(ctl.timers, seconds, binding);
        host.done("ShipAi::controller_step", 0x009f50e0u);
        if (ran) {
            ++row.controller_steps;
            ++host.summary.steps;
        } else {
            ++row.gated;
            ++host.summary.gated;
        }
        row.steering_mode = static_cast<int>(ctl.blk.mode);
        row.desired_throttle = ctl.blk.desired_throttle;
        row.desired_rudder = ctl.blk.desired_rudder;
        row.desired_heading = ctl.blk.desired_heading;
        row.latched_direction = static_cast<int>(ctl.blk.direction);
        row.heading_target = ctl.blk.heading_target_324;
        row.distance_32c = ctl.blk.distance_32c;
        row.distance_330 = ctl.blk.distance_330;
        row.distance_finite = std::isfinite(ctl.blk.distance_32c)
            && std::isfinite(ctl.blk.distance_330);
        // Milestone 2o: ring+148h / +14Ch as 00813020 left them on the previous
        // step. The tick runs inside 00825F20, after this pass, so reading them
        // here samples the value the motion actually integrated.
        const float live_a = host.units.unit_ring_current_throttle(index);
        const float live_b = host.units.unit_ring_current_rudder(index);
        if (live_a != ctl.live_throttle || live_b != ctl.live_rudder) {
            ++row.live_pair_changes;
            ++host.summary.live_pair_changes;
        }
        ctl.live_throttle = live_a;
        ctl.live_rudder = live_b;
        row.ring_live_throttle = live_a;
        row.ring_live_rudder = live_b;
        // 009DE050 forces blk+1C4h to Navigate on every state step but
        // `cruise`'s, which is what hands the steering to the unprojected
        // navigation arm rather than to the three setters.
        if (ctl.blk.mode == bsp::ShipAiSteeringMode::Navigate
            || ctl.blk.mode == bsp::ShipAiSteeringMode::NavigateAstern) {
            ++host.summary.navigate_mode_steps;
        }
        // Milestone 2p: 0071F290, the command controller's own per-frame
        // update, instead of a bare call to the think. Packet
        // cc2_director_update_arms read the routine whole, and its arm 7 is
        // where 009F5DA0 runs: [controller+38h]->vtable[4](dt) with the
        // director's vtable 00D21B48, whose +4h is 009F5DA0. So the think's
        // position in the frame is recovered here rather than supplied, and
        // arm 3 is the hold countdown 0071DF70 tests.
        bsp::CommandControllerUpdateState state{};
        // Arm 1's session predicate. The session object is this process's own
        // (docs/GAME_EXECUTABLE.md milestone 2l: 0077C2A0's routing and the
        // queue are records and the executable delivers synchronously), so the
        // four lifecycle bytes are the live combination and the gate is open.
        state.session_present = true;
        state.session_flags.flag_5c = true;
        state.path_object0_present = false;
        state.auto_target_hold = host.units.director_target_hold_0040(index);
        state.mode = 1;
        state.slot0_occupied = host.units.director_slot_command(index, 0) != 0u;
        state.override_command_present = false;
        state.queue_accepted = false;
        state.override_accepted = false;
        // [*(00E188A8) + 1FE4h]. There is no world object, so the session mode
        // is the single-player 1 the rest of this executable already assumes.
        state.session_mode = 1;
        host.record("CommandController::session_mode_1fe4", 0x00e188a8u);
        state.auto_target_present = true;
        ControllerUpdateBinding update(host, ctl, row, index);
        const bsp::CommandControllerUpdateTrace trace
            = bsp::run_command_controller_update(state, seconds, update);
        host.done("CommandController::update", 0x0071f290u);
        ++row.controller_updates;
        ++host.summary.controller_updates;
        row.controller_update_session_gate = trace.session_gate_passed;
    }
}

bool GameShipAiHost::promote_order_00825f2c(std::size_t unit_index) {
    Impl& host = *impl_;
    if (unit_index >= host.controllers.size()) return false;
    Impl::Controller& ctl = host.controllers[unit_index];
    bsp::unit_promote_ai_order_00825f2c(ctl.order);
    host.done("ShipAiOrder::promote_slot", 0x00825f2cu);
    if (!ctl.order.promoted) return false;
    host.done("ShipAiOrder::copy_slot", 0x00811d10u);
    ++host.rows[unit_index].promotions;
    ++host.summary.promotions;
    // 00825F7C..00826D6B: whatever reads the promoted slot's +40h, +44h and +48h
    // and turns them into the order ring's +148h / +14Ch. docs/UNIT_AUTOPILOT_PAIR.md
    // states the negative result - no reader of those three fields was found -
    // and names the packet that is reading it.
    host.record("ShipAiOrder::slot_to_order_ring", 0x00825f7cu);
    if (!host.logged_position) {
        host.logged_position = true;
        host.log.notef("the promoted AI order slot reaches no order ring: 009f4d10 publishes "
            "the heading target and the two distances into unit+0aech - 84*[unit+0b40h], "
            "00825f2c flips the index and 00811d10 copies the slot across, and no reader of "
            "slot+40h / +44h / +48h was found (docs/UNIT_AUTOPILOT_PAIR.md). The last hop is "
            "packet cc_ai_order_hop, worker agent/cc-ai-order-hop");
    }
    return true;
}

void GameShipAiHost::set_ai_drive(std::size_t unit_index, float throttle, float rudder) {
    Impl& host = *impl_;
    if (unit_index >= host.controllers.size()) return;
    // Milestone 2r RETIRES the switch. Milestone 2q kept it for `attackmove`
    // alone, the one state of four that still produced no desired throttle,
    // because 009E76D0 and its four slot scorers were records. They are wired
    // now, so every state of this mission forms its own pair and there is
    // nothing left for a stand-in to stand in for. The switch is accepted and
    // ignored rather than removed from the command line, so a script that
    // passes it still runs and the log says what happened.
    host.log.notef("--ai-drive \"%s\" = throttle %.3f, rudder %.3f: RETIRED in milestone "
        "2r and IGNORED. The diagnostic stand-in existed for `attackmove`, whose ring scan "
        "009e76d0, four slot scorers 009e6400 / 009e5da0 / 009e6870 / 009e6640, bearing "
        "commit 009e5e90 and throttle limiter 009e6a90 are all wired now; the six "
        "attackmove ships move under their own commanded heading and throttle",
        host.rows[unit_index].unit.c_str(), static_cast<double>(throttle),
        static_cast<double>(rudder));
}

const std::vector<GameShipAiRow>& GameShipAiHost::rows() const noexcept { return impl_->rows; }
const GameShipAiSummary& GameShipAiHost::summary() const noexcept { return impl_->summary; }

void GameShipAiHost::log_sample(unsigned long long step_index, unsigned long long interval) {
    Impl& host = *impl_;
    if (interval == 0 || host.controllers.empty()) return;
    if (step_index % interval != 0) return;
    for (std::size_t index = 0; index < host.rows.size(); ++index) {
        GameShipAiRow& row = host.rows[index];
        char line[320];
        std::snprintf(line, sizeof(line),
            "%-20s state=%-10s mode=%-7s dir=%-7s throttle=%7.3f rudder=%7.3f "
            "heading=%8.4f target=%8.4f d32c=%9.2f d330=%9.2f slot=%d fire=%s",
            row.unit.c_str(), row.state.c_str(), steering_mode_name(row.steering_mode),
            direction_name(row.latched_direction),
            static_cast<double>(row.desired_throttle),
            static_cast<double>(row.desired_rudder),
            static_cast<double>(row.desired_heading),
            static_cast<double>(row.heading_target),
            static_cast<double>(row.distance_32c),
            static_cast<double>(row.distance_330), row.slot_index,
            row.fire_target.empty() ? "-" : row.fire_target.c_str());
        // A sampled line is emitted only when something a reader would act on
        // moved, because 32 ships every ten steps is 1248 lines of the same row.
        if (host.controllers[index].sample == line) continue;
        host.controllers[index].sample = line;
        host.log.notef("  ship ai step %llu  %s", step_index, line);
    }
}

void GameShipAiHost::report() {
    Impl& host = *impl_;
    if (host.rows.empty()) return;
    host.log.notef("  %-20s %-10s %8s %8s %8s %8s %9s %7s %7s %-14s %s", "unit", "state",
        "steps", "replans", "publish", "promote", "heading", "thinks", "scans", "chose",
        "blocked at");
    for (const GameShipAiRow& row : host.rows) {
        host.log.notef("  %-20s %-10s %8llu %8llu %8llu %8llu %9.4f %7llu %7llu %-14s %s",
            row.unit.c_str(), row.state.c_str(), row.controller_steps, row.replans,
            row.publishes, row.promotions, static_cast<double>(row.slot_heading_44),
            row.target_thinks, row.target_scans,
            row.fire_target.empty() ? "-" : row.fire_target.c_str(),
            row.target_blocked.empty() ? "-" : row.target_blocked.c_str());
    }
    for (const GameShipAiRow& row : host.rows) {
        if (row.state == "cruise") ++host.summary.states_cruise;
        else if (row.state == "stop") ++host.summary.states_stop;
        else if (row.state == "attackmove") ++host.summary.states_attackmove;
        else if (row.state == "movetopos") ++host.summary.states_movetopos;
        else ++host.summary.states_other;
        if (!row.fire_target.empty()) ++host.summary.units_with_fire_target;
    }
    host.log.notef("summary mission ship ai units=%zu ai_owned=%zu steps=%llu gated=%llu "
        "replans=%llu state_steps{concrete=%llu records=%llu} publishes=%llu promotions=%llu",
        host.summary.units, host.summary.ai_owned, host.summary.steps, host.summary.gated,
        host.summary.replans, host.summary.state_steps_concrete,
        host.summary.state_steps_recorded, host.summary.publishes, host.summary.promotions);
    host.log.notef("summary mission ship ai states cruise=%zu stop=%zu attackmove=%zu "
        "movetopos=%zu other=%zu", host.summary.states_cruise, host.summary.states_stop,
        host.summary.states_attackmove, host.summary.states_movetopos,
        host.summary.states_other);
    // Milestone 2o: the hop's own table, one row per unit that wrote a ring
    // slot, so a reader can see what reached the ring rather than only what the
    // controller decided.
    host.log.notef("  %-20s %-10s %8s %8s %8s %9s %9s %9s %9s %8s %7s %7s %7s", "unit",
        "state", "hops", "writes", "deadband", "slot_thr", "slot_rud", "live_thr",
        "live_rud", "livechg", "danger", "thr_lim", "load102c");
    for (const GameShipAiRow& row : host.rows) {
        host.log.notef("  %-20s %-10s %8llu %8llu %8llu %9.4f %9.4f %9.4f %9.4f %8llu "
            "%7.3f %7.3f %7.3f",
            row.unit.c_str(), row.state.c_str(), row.ring_hops, row.ring_writes,
            row.rudder_deadbands, static_cast<double>(row.ring_slot_throttle),
            static_cast<double>(row.ring_slot_rudder),
            static_cast<double>(row.ring_live_throttle),
            static_cast<double>(row.ring_live_rudder), row.live_pair_changes,
            static_cast<double>(row.danger_a84),
            static_cast<double>(row.throttle_limit_344),
            static_cast<double>(row.turn_assist_load_102c));
    }
    // Milestone 2p: the goal vector, one row per unit whose brain pre-pass ran,
    // so the (0,0) of milestone 2o can be compared against what the command
    // actually asks for.
    host.log.notef("  %-20s %-10s %8s %8s %11s %11s %-16s %s", "unit", "state", "prepass",
        "refresh", "goal_x", "goal_z", "target", "0071eb60 descriptor");
    for (const GameShipAiRow& row : host.rows) {
        if (row.goal_prepasses == 0) continue;
        host.log.notef("  %-20s %-10s %8llu %8llu %11.1f %11.1f %-16s %s", row.unit.c_str(),
            row.state.c_str(), row.goal_prepasses, row.goal_refreshes,
            static_cast<double>(row.brain_goal_x), static_cast<double>(row.brain_goal_z),
            row.brain_target_name.empty() ? "-" : row.brain_target_name.c_str(),
            row.command_descriptor.c_str());
        if (row.brain_goal_x != 0.0f || row.brain_goal_z != 0.0f) {
            ++host.summary.units_with_nonzero_goal;
        }
        if (row.brain_target != 0u) ++host.summary.units_with_brain_target;
    }
    for (const GameShipAiRow& row : host.rows) {
        if (row.goal_sets > 0) ++host.summary.units_with_goal;
    }
    // Milestone 2p: the path plan 009E3780 builds and the approach point
    // 009F1BC0 copies, for the units whose state reaches either.
    host.log.notef("  %-20s %-10s %8s %8s %6s %6s %8s %11s %11s %11s", "unit", "state",
        "plan_req", "seeds", "state", "nodes", "approach", "point_x", "point_z",
        "goal_range");
    for (const GameShipAiRow& row : host.rows) {
        if (row.path_plan_refreshes == 0 && row.approach_frames == 0) continue;
        host.log.notef("  %-20s %-10s %8llu %8llu %6d %6d %8llu %11.1f %11.1f %11.1f",
            row.unit.c_str(), row.state.c_str(), row.path_plan_refreshes,
            row.path_plan_seeds, row.path_plan_state, row.path_plan_nodes,
            row.approach_frames, static_cast<double>(row.approach_point_x),
            static_cast<double>(row.approach_point_z),
            static_cast<double>(row.approach_goal_range));
    }
    host.log.notef("summary mission ship ai plan requests=%llu seeds=%llu accepts=%llu "
        "approach_frames=%llu controller_updates=%llu",
        host.summary.path_plan_refreshes, host.summary.path_plan_seeds,
        host.summary.path_plan_accepts, host.summary.approach_frames,
        host.summary.controller_updates);
    // Milestone 2q: the search, the swap and the point.
    host.log.notef("  %-20s %-10s %8s %6s %6s %10s %7s %11s %11s %9s %9s", "unit", "state",
        "searches", "state", "nodes", "swaps", "points", "point_x", "point_z", "nav_dist",
        "nav_hdg");
    for (const GameShipAiRow& row : host.rows) {
        if (row.path_search_ticks == 0 && row.nav_output_blocks == 0) continue;
        if (row.path_points > 0) ++host.summary.units_with_path_point;
        host.log.notef("  %-20s %-10s %8llu %6d %6d %10llu %7llu %11.1f %11.1f %9.2f %9.4f",
            row.unit.c_str(), row.state.c_str(), row.path_search_ticks, row.path_plan_state,
            row.path_plan_nodes, row.path_plan_swaps, row.path_points,
            static_cast<double>(row.path_point_x), static_cast<double>(row.path_point_z),
            static_cast<double>(row.nav_distance_32c),
            static_cast<double>(row.nav_heading_324));
    }
    host.log.notef("summary mission ship ai path search ticks=%llu swaps=%llu points=%llu "
        "corner_arms=%llu units_with_point=%zu output_blocks=%llu bearings=%llu",
        host.summary.path_search_ticks, host.summary.path_plan_swaps,
        host.summary.path_points, host.summary.path_corner_arms,
        host.summary.units_with_path_point, host.summary.nav_output_blocks,
        host.summary.nav_bearings);
    // Milestone 2r: what 009E4330 gave each ship, and the hull body beside it.
    host.log.notef("  %-20s %-10s %9s %9s %9s %9s %9s %9s %10s %4s", "unit", "state",
        "len_9c8", "turn_3c8", "turn_3cc", "yaw_3d0", "stop_3d4", "start_3d8", "mass_b0",
        "mat");
    for (const GameShipAiRow& row : host.rows) {
        host.log.notef("  %-20s %-10s %9.2f %9.2f %9.2f %9.5f %9.2f %9.2f %10.1f %4d",
            row.unit.c_str(), row.state.c_str(),
            static_cast<double>(row.nav_hull_length_9c8),
            static_cast<double>(row.nav_turn_circle_3c8),
            static_cast<double>(row.nav_turn_circle_3cc),
            static_cast<double>(row.nav_yaw_floor_3d0),
            static_cast<double>(row.nav_stop_radius_3d4),
            static_cast<double>(row.nav_start_radius_3d8),
            static_cast<double>(row.hull_mass_00b0), row.hull_material);
    }
    // Milestone 2r: the attackmove ring scan, one row per ship that reached it.
    host.log.notef("  %-20s %-10s %9s %9s %7s %10s %10s %9s %9s %8s", "unit", "state",
        "ring_scan", "bearings", "winner", "hdg_120c", "thr_1210", "clear_37c", "profiles",
        "substate");
    for (const GameShipAiRow& row : host.rows) {
        if (row.ring_scans == 0 && row.clearance_refreshes == 0) continue;
        host.log.notef("  %-20s %-10s %9llu %9llu %7d %10.4f %10.4f %9.1f %9llu %8lx",
            row.unit.c_str(), row.state.c_str(), row.ring_scans, row.ring_scan_bearings,
            row.ring_scan_winner, static_cast<double>(row.approach_heading_120c),
            static_cast<double>(row.approach_throttle_1210),
            static_cast<double>(row.clearance_37c), row.throttle_profiles,
            static_cast<unsigned long>(row.substate));
    }
    host.log.notef("summary mission ship ai nav blocks=%zu (009e4330 once per brain "
        "record, 009f118d) clearance=%llu throttle_profiles=%llu sector_scans=%llu "
        "sector_marks=%llu ring_scans=%llu ring_bearings=%llu firepower=%llu "
        "follower_points=%llu follower_corners=%llu follower_advances=%llu",
        host.summary.nav_blocks, host.summary.clearance_refreshes,
        host.summary.throttle_profiles, host.summary.sector_scans,
        host.summary.sector_marks, host.summary.ring_scans,
        host.summary.ring_scan_bearings, host.summary.firepower_ratings,
        host.summary.path_follower_points, host.summary.path_follower_corners,
        host.summary.path_follower_advances);
    host.log.notef("summary mission ship ai command completion events=%llu callbacks=%llu "
        "end_commands=%llu queue_advances=%llu",
        host.summary.command_events, host.summary.command_event_callbacks,
        host.summary.command_endings, host.summary.command_completions);
    host.log.notef("summary mission ship ai arm tail bodies=%llu latched=%llu stops=%llu "
        "arrival_latches=%llu",
        host.summary.arm_tails, host.summary.arm_tail_latched, host.summary.arm_tail_stops,
        host.summary.arrival_latches);
    host.log.notef("summary mission ship ai goal vector prepasses=%llu refreshes=%llu "
        "nonzero_goals=%zu brain_targets=%zu path_plan_refreshes=%llu path_picks=%llu "
        "path_publishes=%llu station_keeping=%llu sector_refreshes=%llu middle_runs=%llu "
        "substate_concrete=%llu",
        host.summary.goal_prepasses, host.summary.goal_refreshes,
        host.summary.units_with_nonzero_goal, host.summary.units_with_brain_target,
        host.summary.path_plan_refreshes, host.summary.path_picks,
        host.summary.path_publishes, host.summary.station_keeping,
        host.summary.sector_refreshes, host.summary.middle_runs,
        host.summary.substate_concrete);
    host.log.notef("summary mission ship ai state steps real=%llu goal_sets=%llu "
        "goal_replans=%llu units_with_goal=%zu substate_steps=%llu navigate_mode_steps=%llu",
        host.summary.state_steps_real, host.summary.goal_sets, host.summary.goal_replans,
        host.summary.units_with_goal, host.summary.substate_steps,
        host.summary.navigate_mode_steps);
    host.log.notef("summary mission ship ai ring hops=%llu gated_3f5=%llu writes=%llu "
        "rudder_law=%llu deadbands=%llu live_pair_changes=%llu driven=%zu",
        host.summary.ring_hops, host.summary.ring_gated_3f5, host.summary.ring_writes,
        host.summary.rudder_law_calls, host.summary.rudder_deadbands,
        host.summary.live_pair_changes, host.summary.units_driven);
    host.log.notef("summary mission auto target thinks=%llu scans=%llu chose=%zu "
        "fire_target_sets=%llu attackmove_issues=%llu accepts=%zu",
        host.summary.thinks, host.summary.scans, host.summary.units_with_fire_target,
        host.summary.fire_target_sets, host.summary.attackmove_issues,
        host.summary.units_accepting_new_target);
    // Milestone 2p: what 0071DF70's two rules saw, per unit.
    host.log.notef("  %-20s %-10s %8s %10s %-12s %8s %s", "unit", "state", "gate",
        "hold+40h", "slot0", "category", "0071df70");
    for (const GameShipAiRow& row : host.rows) {
        if (row.target_thinks == 0) continue;
        // A unit whose scan found no candidate never reaches 0071DF70, so its
        // hold and its slot were never read and the row says so rather than
        // printing the field's default.
        if (row.target_gate_tests == 0) {
            host.log.notef("  %-20s %-10s %8llu %10s %-12s %8s %s", row.unit.c_str(),
                row.state.c_str(), row.target_gate_tests, "-", "-", "-",
                "not reached: no candidate");
            continue;
        }
        host.log.notef("  %-20s %-10s %8llu %10.3f %-12s %8d %s", row.unit.c_str(),
            row.state.c_str(), row.target_gate_tests,
            static_cast<double>(row.director_hold_0040),
            row.slot0_command.empty() ? "-" : row.slot0_command.c_str(),
            row.slot0_category, row.target_blocked.empty() ? "accepted"
                : row.target_blocked.c_str());
    }
}

}  // namespace bsp::game

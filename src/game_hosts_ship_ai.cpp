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

#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/ship_ai_throttle_ring.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/unit_state_message.hpp"
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
    void replan_prepare_009f1420(float) override {
        owner_.record("ShipAi::replan_prepare", 0x009f1420u);
    }
    void state_step_vtable0c(float elapsed) override {
        ++row_.replans;
        ++owner_.summary.replans;
        const StateDescriptor* state = state_for_ai_offset(ctl_.active_state_ai_offset);
        if (state != nullptr && state->step_concrete) {
            SetterBinding setters(owner_);
            if (owner_.units.run_cruise_state_step_009e1170(index_, ctl_.blk, setters)) {
                ++owner_.summary.state_steps_concrete;
                return;
            }
            // 009E1170 ran and took its 009E11E8 player arm, which is not
            // projected; GameCommandsHost has already recorded that arm with its
            // own address. Recording the step again here would put one method
            // name on both dispositions, and the log keeps the first.
            ++owner_.summary.state_steps_recorded;
            return;
        }
        // Every other leaf's step was named by docs/SHIP_AI_STATES.md as a
        // vtable slot and its body was not read, so the step is a record with
        // its own address. `sub_attack`'s vtable was not read at all.
        ++owner_.summary.state_steps_recorded;
        // Milestone 2o, --ai-drive: a LABELLED DIAGNOSTIC STAND-IN for exactly
        // this gap. The state step has no body, so on this re-plan tick the two
        // recovered setters are called on the unit's own control block with the
        // pair the switch names. Nothing else on the chain is substituted: the
        // clamps, the mode switch, 009ED6B0, 009F4D10, the hop and the ring are
        // the game's own routines.
        if (ctl_.drive) {
            SetterBinding setters(owner_);
            bsp::ship_ai_set_desired_throttle_009dbf90(ctl_.blk, ctl_.drive_throttle);
            owner_.done("ShipAiDrive::set_desired_throttle", 0x009dbf90u);
            bsp::ship_ai_set_desired_steering_009dffb0(ctl_.blk, ctl_.drive_rudder,
                setters);
            owner_.done("ShipAiDrive::set_desired_steering", 0x009dffb0u);
        }
        if (state == nullptr || state->step == 0u) {
            owner_.record_slot("ShipAiState::step_vtable0c", "00d21598+vtable0c");
            return;
        }
        char method[64];
        std::snprintf(method, sizeof(method), "ShipAiState::%s_step", state->name);
        owner_.record(method, state->step);
        static_cast<void>(elapsed);
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
    void step_009f0ea0(float) override { owner_.record("ShipAi::step_009f0ea0", 0x009f0ea0u); }
    void step_009e04e0(float) override { owner_.record("ShipAi::step_009e04e0", 0x009e04e0u); }
    void step_009ef230() override { owner_.record("ShipAi::step_009ef230", 0x009ef230u); }
    bool navigate_009ed6b0(float seconds) override {
        DirectControlBinding binding(owner_, index_);
        const bool early_out
            = bsp::ship_ai_direct_control_arm_009ed6b0(ctl_.blk, seconds, binding);
        owner_.done("ShipAi::direct_control_arm", 0x009ed6b0u);
        // 009EDA26..009EF228, the navigation arm, is not projected. It runs on a
        // false return and can overwrite +324h, +32Ch and +330h, so the run
        // records it rather than presenting the direct arm's values as final.
        if (!early_out && ctl_.blk.mode == bsp::ShipAiSteeringMode::Navigate) {
            owner_.record("ShipAi::navigation_arm", 0x009eda26u);
        }
        return early_out;
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
        // 0071DF70, read whole for this packet, body 0071DF70-0071DFCF: the
        // float at [director+40h] must be greater than the 0.0f at 00D7A218,
        // and no filled command slot's vtable[0Ch] may answer 1 or 2. Nothing in
        // this process writes director+40h and no producer of it was found, so
        // the predicate is a record and its neutral answer stops the tick here.
        owner_.record("AutoTarget::director_accepts_new_target", 0x0071df70u);
        if (!owner_.logged_accept_gate) {
            owner_.logged_accept_gate = true;
            owner_.log.notef("automatic target selection stops at 0071df70: its first test is "
                "the float at director+40h against the 0.0f at 00d7a218, and that field has no "
                "writer in this process and no recovered producer anywhere, so the chosen "
                "candidate never reaches 00835860 BSP_WeaponDirector_SetFireTarget and the "
                "attackmove issue at 0071d980 is not reached either");
        }
        row_.target_blocked = "0071df70 director+40h";
        return false;
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

    // 009F40CA..009F44E3 and 009F4502..009F4B98, the spans this packet does not
    // project: the ten writers of blk+1D0h that docs/SHIP_AI_THROTTLE_TO_RING.md
    // tabulates, and the two 2Ch-stride obstacle tables at blk+81Ch / blk+848h
    // with the second rudder store at 009F46A0. One record for the whole of it,
    // so the host-method count carries the boundary.
    record("ShipAi::drive_order_ring_body", 0x009f40cau);

    // 009F44E4 CMP [ESI+1C4h],0 / JZ 009F4502: the rudder law runs only when
    // the steering mode is NOT Rudder. A state that commanded a rudder through
    // 009DFFB0 keeps the rudder it asked for; a Heading or Navigate state has
    // its rudder built here from the heading error.
    if (ctl.blk.mode != bsp::ShipAiSteeringMode::Rudder) {
        RudderLawBinding law(*this, index);
        // The one stand-in on this path: 009DA268 loads the divisor from
        // [[blk+3FCh]+538h]+524h and 00831840 does not write class+524h, so the
        // field has no recovered Lua key. MaxRotAngle (class+4F8h) stands in,
        // exactly as src/ship_motion_probe.cpp does, and the record keeps that
        // visible in the host-method table.
        const float authority = units.unit_yaw_authority_stand_in_04f8(index);
        record("ShipAiRudder::class_yaw_authority_0524", 0x009da268u);
        // 009F44F7 009DA250, 009F44FC FSTP [ESI+1D4h].
        ctl.blk.desired_rudder = bsp::ship_ai_rudder_from_heading_error_009da250(
            ctl.blk.direction, error, authority, law);
        done("ShipAiRudder::from_heading_error", 0x009da250u);
        ++row.rudder_law_calls;
        ++summary.rudder_law_calls;
    }

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
    // The head's speed is read by the native routine into two byte flags that
    // only the unprojected arms consume ([ESP+6] at 009F4043 and BL at
    // 009F4077), so the call site runs and the value goes nowhere here.
    static_cast<void>(speed);
}

// ---------------------------------------------------------------------------

GameShipAiHost::GameShipAiHost(GameHostLog& log, GameUnitsHost& units)
    : impl_(std::make_unique<Impl>(log, units)) {}
GameShipAiHost::~GameShipAiHost() = default;

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
        // 009F5DA0 beside it, with the same step delta: the selector's own
        // countdown is what turns a per-step call into a once-a-second think.
        const float before = ctl.target.think_countdown;
        TargetBinding target(host, ctl, row, index);
        bsp::auto_target_tick_009f5da0(target, ctl.target, nullptr, seconds);
        host.done("AutoTarget::tick", 0x009f5da0u);
        if (!(seconds < before)) {
            ++row.target_thinks;
            ++host.summary.thinks;
        }
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
    Impl::Controller& ctl = host.controllers[unit_index];
    if (!ctl.drive) ++host.summary.units_driven;
    ctl.drive = true;
    ctl.drive_throttle = throttle;
    ctl.drive_rudder = rudder;
    host.rows[unit_index].ai_driven = true;
    host.log.notef("--ai-drive \"%s\" = throttle %.3f, rudder %.3f: a LABELLED DIAGNOSTIC "
        "STAND-IN for the state step. Eight of the nine leaves of the family at 00d21598 "
        "have no reconstructed body, so on every re-plan tick of this unit the executable "
        "calls 009dbf90 and 009dffb0 on its own control block with this pair in place of "
        "the state's decision. Everything after that point is the game's own recovered "
        "path: 009ed6b0, 009f4d10, the 009f50c6 tail into 009f3f80, that routine's hop "
        "through 0080e170 / 0080e190, 00813020's tick and 00825f20's motion",
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
    host.log.notef("  %-20s %-10s %8s %8s %8s %9s %9s %9s %9s %8s", "unit", "state",
        "hops", "writes", "deadband", "slot_thr", "slot_rud", "live_thr", "live_rud",
        "livechg");
    for (const GameShipAiRow& row : host.rows) {
        host.log.notef("  %-20s %-10s %8llu %8llu %8llu %9.4f %9.4f %9.4f %9.4f %8llu",
            row.unit.c_str(), row.state.c_str(), row.ring_hops, row.ring_writes,
            row.rudder_deadbands, static_cast<double>(row.ring_slot_throttle),
            static_cast<double>(row.ring_slot_rudder),
            static_cast<double>(row.ring_live_throttle),
            static_cast<double>(row.ring_live_rudder), row.live_pair_changes);
    }
    host.log.notef("summary mission ship ai ring hops=%llu gated_3f5=%llu writes=%llu "
        "rudder_law=%llu deadbands=%llu live_pair_changes=%llu driven=%zu",
        host.summary.ring_hops, host.summary.ring_gated_3f5, host.summary.ring_writes,
        host.summary.rudder_law_calls, host.summary.rudder_deadbands,
        host.summary.live_pair_changes, host.summary.units_driven);
    host.log.notef("summary mission auto target thinks=%llu scans=%llu chose=%zu "
        "fire_target_sets=%llu attackmove_issues=%llu blocked_at=0071df70",
        host.summary.thinks, host.summary.scans, host.summary.units_with_fire_target,
        host.summary.fire_target_sets, host.summary.attackmove_issues);
}

}  // namespace bsp::game

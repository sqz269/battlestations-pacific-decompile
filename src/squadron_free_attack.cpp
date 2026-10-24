// The squadron free-attack auto-target tick 009F8470, hosted over an arena.
// Packet p15_free_attack_tick, docs/OCEAN_WAVE_FIELD.md section 57; the tick
// itself is cc13's (src/cc13_seg68.cpp), run unchanged.

#include "bsp/squadron_free_attack.hpp"

#include "bsp/cc13_seg68.hpp"

#include <cstring>
#include <map>

namespace bsp {

const std::array<SquadronAttackList, 8>& squadron_free_attack_lists() noexcept {
    // Kind indices in 00E0CD80: 06 SHIP, 07 DESTROYER, 08 SUBMARINE, 09 MOTHERSHIP,
    // 0A CRUISER, 0B CARGO, 0C LANDINGSHIP, 0D BATTLESHIP, 0E TORPEDOBOAT, 0F PLANE,
    // 10 LEVELBOMBER, 11 TORPEDOBOMBER, 12 DIVEBOMBER, 13 FIGHTER, 14 RECONPLANE,
    // 17 KAMIKAZEPLANE, 1B LANDFORT, 35 DUMMYTARGET_VEHICLE, 45 AIRFIELD, 46 SHIPYARD.
    static const std::array<SquadronAttackList, 8> lists = {{
        {"DepthCharge", 0x04, {0x08}},
        {"Strike", 0x74, {0x0D, 0x09, 0x0A, 0x08, 0x07, 0x0B, 0x0C, 0x45, 0x46}},
        {"DiveBomb", 0x14, {0x0D, 0x09, 0x0A, 0x08, 0x07, 0x0B, 0x0C, 0x45, 0x46}},
        {"LevelBomb", 0x24, {0x0D, 0x09, 0x0A, 0x08, 0x07, 0x0B, 0x0C, 0x45, 0x46}},
        {"TorpedoBomb", 0x34, {0x0D, 0x09, 0x0A, 0x07, 0x08, 0x0B, 0x0C}},
        {"NonFighterGun", 0x54,
         {0x13, 0x17, 0x12, 0x11, 0x0E, 0x14, 0x10, 0x0F, 0x0C, 0x35, 0x0B, 0x1B, 0x06}},
        {"FighterGun", 0x44, {0x13, 0x17, 0x10, 0x12, 0x11, 0x14, 0x0E, 0x0C, 0x0B, 0x35}},
        {"Kamikaze", 0x64, {0x0D, 0x09, 0x0A, 0x07, 0x08, 0x0B, 0x45, 0x46}},
    }};
    return lists;
}

namespace {

using cc13_seg68::Handle;

// A flat byte arena standing for the native objects the tick reads. A handle
// is kBase plus an offset, so the tick's own pointer arithmetic (director+58h,
// entity+FCh, block+offset) lands on the field it names.
class Arena {
public:
    static constexpr Handle kBase = 0x10000000u;
    Handle alloc(std::uint32_t size) {
        const std::uint32_t at = static_cast<std::uint32_t>((bytes_.size() + 15u) & ~15u);
        bytes_.resize(at + size, 0u);
        return kBase + at;
    }
    void put32(Handle object, std::uint32_t offset, std::uint32_t value) {
        std::memcpy(slot(object, offset, 4), &value, 4);
    }
    void put8(Handle object, std::uint32_t offset, std::uint8_t value) {
        *slot(object, offset, 1) = value;
    }
    void putf(Handle object, std::uint32_t offset, float value) {
        std::memcpy(slot(object, offset, 4), &value, 4);
    }
    std::uint32_t get32(Handle object, std::uint32_t offset) const {
        std::uint32_t value = 0;
        if (const std::uint8_t* p = cslot(object, offset, 4)) std::memcpy(&value, p, 4);
        return value;
    }
    std::uint8_t get8(Handle object, std::uint32_t offset) const {
        const std::uint8_t* p = cslot(object, offset, 1);
        return p != nullptr ? *p : 0u;
    }
    float getf(Handle object, std::uint32_t offset) const {
        float value = 0.0f;
        if (const std::uint8_t* p = cslot(object, offset, 4)) std::memcpy(&value, p, 4);
        return value;
    }

private:
    std::uint8_t* slot(Handle object, std::uint32_t offset, std::size_t n) {
        const std::size_t at = static_cast<std::size_t>(object - kBase) + offset;
        if (at + n > bytes_.size()) bytes_.resize(at + n, 0u);
        return bytes_.data() + at;
    }
    const std::uint8_t* cslot(Handle object, std::uint32_t offset, std::size_t n) const {
        if (object < kBase) return nullptr;
        const std::size_t at = static_cast<std::size_t>(object - kBase) + offset;
        return at + n <= bytes_.size() ? bytes_.data() + at : nullptr;
    }
    std::vector<std::uint8_t> bytes_;
};

constexpr std::uint32_t kEntitySize = 0xC30u;   // covers +C24h
constexpr std::uint32_t kVt140Size = 0x1D8u;

class ArenaHost final : public cc13_seg68::SquadronAutoTargetHost {
public:
    ArenaHost(const SquadronFreeAttackInput& in, SquadronFreeAttackWorld& world,
              SquadronFreeAttackDecision& out)
        : in_(in), world_(world), out_(out) {
        director_ = arena_.alloc(0x80);
        arena_.put32(director_, 0x30, in.mode_30);
        arena_.put8(director_, 0x3c, in.allow_fire_3c ? 1u : 0u);
        arena_.put8(director_, 0x3d, in.allow_move_3d ? 1u : 0u);
        arena_.put32(director_, 0x54, in.slot0_command_54);
        squadron_ = arena_.alloc(0x400);
        leader_ = in.leader != kFreeAttackNoUnit ? unit_object(in.leader) : 0u;
        arena_.put32(squadron_, 0x3d0, leader_);
        arena_.put32(squadron_, 0x54, static_cast<std::uint32_t>(in.party_54));
        arena_.put32(squadron_, 0x370, static_cast<std::uint32_t>(in.attack_mode_370));
        arena_.put8(squadron_, 0xc8, 1u);
        for (int k = 0; k < 3; ++k) arena_.putf(squadron_, 0xfc + 4u * k, in.squadron_position[k]);
        if (leader_ != 0u) {
            arena_.put32(leader_, 0x1b0, static_cast<std::uint32_t>(in.leader_slot_1b0));
            arena_.put8(leader_, 0x184, in.leader_player_184 ? 1u : 0u);
            arena_.put8(leader_, 0xc24, in.leader_pilot_fires_c24 ? 1u : 0u);
        }
        // The recon slot and its chain: node +4h next, +8h item, item +4h entity.
        recon_ = arena_.alloc(0xDF0);
        Handle previous = 0u;
        for (const FreeAttackContact& c : in.contacts) {
            if (c.unit == kFreeAttackNoUnit) continue;
            const Handle node = arena_.alloc(0x10);
            const Handle item = arena_.alloc(0x10);
            arena_.put32(node, 0x8, item);
            arena_.put32(item, 0x4, c.squadron ? squadron_object(c.unit) : unit_object(c.unit));
            if (previous == 0u) arena_.put32(recon_, 0xde8, node);
            else arena_.put32(previous, 0x4, node);
            previous = node;
        }
        // The AttackLists block: each vector's begin at +4h and end at +8h.
        block_ = arena_.alloc(0x90);
        for (const SquadronAttackList& list : squadron_free_attack_lists()) {
            const Handle elements = arena_.alloc(static_cast<std::uint32_t>(4u * list.kinds.size() + 4u));
            for (std::size_t i = 0; i < list.kinds.size(); ++i) {
                arena_.put32(elements, static_cast<std::uint32_t>(4u * i),
                             static_cast<std::uint32_t>(list.kinds[i]));
            }
            arena_.put32(block_, list.offset + 4u, elements);
            arena_.put32(block_, list.offset + 8u,
                         elements + static_cast<std::uint32_t>(4u * list.kinds.size()));
        }
        tuning_ = arena_.alloc(0x600);
        arena_.putf(tuning_, 0x594, in.guard_dist_594);
        zero_ = arena_.alloc(0x10);
        ggame_ = arena_.alloc(0x10);
    }

    Handle director() const noexcept { return director_; }
    Handle squadron() const noexcept { return squadron_; }

    // -- native reads ---------------------------------------------------------
    std::uint8_t u8(Handle object, std::uint32_t offset) override { return arena_.get8(object, offset); }
    std::uint32_t u32(Handle object, std::uint32_t offset) override { return arena_.get32(object, offset); }
    float f32(Handle object, std::uint32_t offset) override { return arena_.getf(object, offset); }

    // -- virtual calls ---------------------------------------------------------
    bool is_kind_of_5c(Handle entity, int kind) override {
        const auto sq = squadron_unit_.find(entity);
        if (sq != squadron_unit_.end()) return kind == 0x18;   // the PlaneSquadron entity
        const std::size_t unit = unit_of(entity);
        return unit != kFreeAttackNoUnit && world_.is_kind_of(unit, kind);
    }
    bool class_query_18(Handle, int) override { return false; }   // 009F7210 not reached
    Handle entity_vt140(Handle entity) override {
        // [vtable[140h]()]+1D4h is AddUntouchableUnit's byte (008AC263). LABELLED:
        // a squadron entity answers its leader's.
        const auto sq = squadron_unit_.find(entity);
        const std::size_t unit = sq != squadron_unit_.end() ? sq->second : unit_of(entity);
        if (unit == kFreeAttackNoUnit) return 0u;
        auto found = vt140_.find(unit);
        if (found == vt140_.end()) {
            const Handle proxy = arena_.alloc(kVt140Size);
            arena_.put8(proxy, 0x1d4, world_.untouchable_1d4(unit) ? 1u : 0u);
            found = vt140_.emplace(unit, proxy).first;
        }
        return found->second;
    }
    int director_vt50(Handle) override {
        // 0084D960, the squadron block's slot +50h: 0 when +3Dh and +3Ch are both
        // set, else 1.
        return in_.allow_move_3d && in_.allow_fire_3c ? 0 : 1;
    }

    // -- direct calls -----------------------------------------------------------
    void refresh_pose_00414db0(Handle) override {}   // every object is posed (+C8h = 1)
    Handle ggame_00e188a8() override { return ggame_; }
    bool point_outside_map_0071c4f0(Handle, Handle point) override {
        const std::size_t unit = unit_of(point - 0xfcu);
        return unit != kFreeAttackNoUnit && world_.outside_map_0071c4f0(unit);
    }
    bool command_blocks_auto_action_00811f50(Handle unit, int) override {
        const std::size_t u = unit_of(unit);
        return u != kFreeAttackNoUnit && world_.command_blocks_auto_action_00811f50(u);
    }
    bool squadron_outside_map_detached_007ee8b0(Handle squadron) override {
        const auto sq = squadron_unit_.find(squadron);
        return sq != squadron_unit_.end() && world_.squadron_outside_map_detached_007ee8b0(sq->second);
    }
    Handle recon_slot_008053c0(Handle) override {
        ++recon_reads_;
        return recon_;
    }
    bool sensors_detect_unit_00804350(Handle unit, Handle target, std::uint32_t mask) override {
        const std::size_t observer = unit_of(unit);
        const std::size_t t = unit_of(target);
        if (observer == kFreeAttackNoUnit || t == kFreeAttackNoUnit) return false;
        return world_.sensors_detect_00804350(observer, t, mask);
    }
    Handle choose_attack_command_007eec50(Handle, Handle target, std::uint8_t flag,
                                          int one) override {
        const std::size_t t = entity_unit(target);
        if (t == kFreeAttackNoUnit || in_.leader == kFreeAttackNoUnit) return 0u;
        const std::uint32_t chosen = world_.choose_attack_command_007eec50(in_.leader, t, flag, one);
        // The last ask is 009F6D60's, made only past the 009F85A1 air-target test.
        if (accept_asks_ != 0u && head_resolves_ != 0u) ++command_chosen_;
        return chosen;
    }
    Handle ai_weights_block_004d6ce0() override { return block_; }
    cc13_seg68::PathPoint current_path_point_0071e4c0(Handle) override {
        cc13_seg68::PathPoint p;
        p.valid = in_.path_point_valid;
        p.p = in_.path_point;
        return p;
    }
    Handle game_tuning_0042e740() override { return tuning_; }
    Handle command_target_resolve_00521ea0(Handle descriptor) override {
        ++head_resolves_;
        if (guard_descriptor_ != 0u && descriptor == guard_descriptor_) {
            // Packet cc13-leash: 009F8021 on the override descriptor.
            if (guard_.unit == kFreeAttackNoUnit) return 0u;
            return guard_.squadron ? squadron_object(guard_.unit) : unit_object(guard_.unit);
        }
        if (descriptor != director_ + 0x58u || in_.slot0_target == kFreeAttackNoUnit) return 0u;
        return in_.slot0_target_squadron ? squadron_object(in_.slot0_target)
                                         : unit_object(in_.slot0_target);
    }
    Handle static_zero_vector_00f87574() override { return zero_; }
    cc13_seg68::SquadronFilter pick_filter_009f6c80(Handle) override {
        out_.filter = band009f::squadron_auto_target_pick_filter_009f6c80(in_.filter_inputs);
        return out_.filter;
    }
    void revalidate_current_command_009f8160(Handle) override {}   // the host ran it
    void ordnance_latch_009f7c90(Handle) override {}               // the host ran it
    bool party_slot_is_ai_held_00927f10(Handle, std::uint32_t) override {
        return in_.leader_slot_ai_held_00927f10;
    }
    bool director_accepts_new_target_0071df70(Handle) override {
        ++accept_asks_;
        return in_.accepts_new_target_0071df70;
    }
    void send_clear_commands_0071d880(Handle) override { out_.clear_then_moveto_self = true; }
    Handle world_position_refreshed_00427eb0(Handle entity) override { return entity + 0xfcu; }
    Handle command_target_init_position_00468560(Handle position, float) override { return position; }
    Handle command_target_from_entity_00465080(Handle entity, float) override { return entity; }
    void issue_command_0071ecf0(Handle, Handle, Handle) override {}   // the clear's moveto, flagged above
    void send_override_command_0071d980(Handle, Handle command, Handle target) override {
        out_.command = command;
        out_.target = entity_unit(target);
        out_.target_squadron = squadron_unit_.count(target) != 0;
        overrides_ += 1;
    }
    void raise_override_stage_0071d9e0(Handle, int stage) override {
        if (stage == 2) out_.raise_override_stage_2 = true;
    }
    // Packet cc13-fapick, log-only: a copy of what 009F7810's loop computed.
    void trace_candidate_009f7810(Handle entity, const cc13_seg68::SquadronCandidateTrace& t) override {
        if (!in_.trace_candidates) return;
        FreeAttackCandidateTrace row;
        row.unit = entity_unit(entity);
        row.squadron = squadron_unit_.count(entity) != 0;
        row.note = static_cast<int>(t.note);
        row.rank = t.rank;
        row.score = t.score;
        out_.traces.push_back(row);
    }

    // Packet cc13-leash: the override descriptor (director+18Ch in the image; its
    // own 18h block here, since the arena's director is 80h).
    Handle guard_descriptor(const SquadronGuardDescriptor& d) {
        guard_ = d;
        guard_descriptor_ = arena_.alloc(0x18);
        arena_.put8(guard_descriptor_, 0x0, d.entity ? 1u : 0u);
        arena_.put8(guard_descriptor_, 0x1, d.position_valid ? 1u : 0u);
        for (int k = 0; k < 3; ++k) arena_.putf(guard_descriptor_, 0x8 + 4u * k, d.position[k]);
        return guard_descriptor_;
    }

    std::size_t unit_for(Handle object) const { return entity_unit(object); }
    bool is_squadron_object(Handle object) const { return squadron_unit_.count(object) != 0; }

    unsigned recon_reads_{0};
    unsigned accept_asks_{0};
    unsigned head_resolves_{0};
    unsigned command_chosen_{0};
    unsigned overrides_{0};

private:
    Handle unit_object(std::size_t unit) {
        const auto found = unit_object_.find(unit);
        if (found != unit_object_.end()) return found->second;
        const Handle object = arena_.alloc(kEntitySize);
        arena_.put8(object, 0xc8, 1u);
        const std::array<float, 3> p = world_.position(unit);
        for (int k = 0; k < 3; ++k) arena_.putf(object, 0xfc + 4u * k, p[k]);
        unit_object_.emplace(unit, object);
        object_unit_.emplace(object, unit);
        return object;
    }
    Handle squadron_object(std::size_t leader) {
        const auto found = squadron_object_.find(leader);
        if (found != squadron_object_.end()) return found->second;
        const Handle object = arena_.alloc(0x400);
        const Handle lead = unit_object(leader);
        arena_.put32(object, 0x3d0, lead);
        arena_.put8(object, 0xc8, 1u);
        for (int k = 0; k < 3; ++k) arena_.putf(object, 0xfc + 4u * k, arena_.getf(lead, 0xfc + 4u * k));
        squadron_object_.emplace(leader, object);
        squadron_unit_.emplace(object, leader);
        return object;
    }
    std::size_t unit_of(Handle object) const {
        const auto found = object_unit_.find(object);
        return found != object_unit_.end() ? found->second : kFreeAttackNoUnit;
    }
    // A unit object or a squadron object, as the host unit that stands for it.
    std::size_t entity_unit(Handle object) const {
        const auto sq = squadron_unit_.find(object);
        return sq != squadron_unit_.end() ? sq->second : unit_of(object);
    }

    const SquadronFreeAttackInput& in_;
    SquadronFreeAttackWorld& world_;
    SquadronFreeAttackDecision& out_;
    Arena arena_;
    Handle director_{0}, squadron_{0}, leader_{0}, recon_{0}, block_{0}, tuning_{0}, zero_{0}, ggame_{0};
    std::map<std::size_t, Handle> unit_object_, squadron_object_, vt140_;
    std::map<Handle, std::size_t> object_unit_, squadron_unit_;
    SquadronGuardDescriptor guard_{};
    Handle guard_descriptor_{0};
};

}  // namespace

const char* free_attack_candidate_note_name(int note) noexcept {
    using N = cc13_seg68::SquadronCandidateNote;
    switch (static_cast<N>(note)) {
    case N::Scored: return "scored";
    case N::SinglePlane: return "single_plane";
    case N::OutsideMapBlocked: return "outside_map";
    case N::SquadronDetached: return "detached";
    case N::Slot140Blocks: return "untouchable";
    case N::Kind1bCarrier: return "kind1b_carrier";
    case N::GuardOut: return "guard_out";
    case N::NotUnitOrSquadron: return "not_unit_or_squadron";
    case N::NoDetect: return "no_detect";
    case N::NoRank: return "not_in_list";
    case N::LeaderGuardOut: return "leader_guard_out";
    }
    return "?";
}

SquadronFreeAttackDecision squadron_free_attack_tick_009f8470(
    const SquadronFreeAttackInput& in, SquadronFreeAttackWorld& world) {
    SquadronFreeAttackDecision out;
    ArenaHost host(in, world, out);
    cc13_seg68::SquadronAutoTarget self{};
    self.director_04 = host.director();
    self.squadron_08 = host.squadron();
    self.selector_2c.owner_00 = &self;
    cc13_seg68::squadron_auto_target_tick_009f8470(host, self, 0u, 0.0f);
    out.candidates = in.contacts.size();
    const Handle pick = self.selector_2c.target_08;
    if (out.target == kFreeAttackNoUnit && pick != 0u) {
        out.target = host.unit_for(pick);
        out.target_squadron = host.is_squadron_object(pick);
    }
    // Where the tick stopped, for the census only (009F8470's order). The gate is
    // restated here because a closed gate and filter B both leave no pick.
    const bool gate_open =
        ((in.leader_slot_1b0 == 8 || in.leader_slot_ai_held_00927f10) && !in.leader_player_184) ||
        (in.allow_fire_3c && in.allow_move_3d && in.attack_mode_370 == 2);
    if (in.mode_30 != 1u) {
        out.stop = FreeAttackStop::kModeNotQueueHead;
    } else if (host.overrides_ != 0u) {
        out.stop = FreeAttackStop::kOverrideSent;
    } else if (!gate_open) {
        out.stop = FreeAttackStop::kGateClosed;
    } else if (pick == 0u) {
        out.stop = FreeAttackStop::kNoTarget;
    } else if (!in.accepts_new_target_0071df70) {
        out.stop = FreeAttackStop::kRefused0071df70;
    } else if (in.slot0_target == out.target && in.slot0_target_squadron == out.target_squadron) {
        out.stop = FreeAttackStop::kSameTarget;
    } else if (host.command_chosen_ == 0u) {
        out.stop = FreeAttackStop::kNotAirTarget;
    } else {
        out.stop = FreeAttackStop::kOrdnanceToOrdnance;
    }
    return out;
}

bool squadron_within_guard_distance_009f7fe0(const SquadronFreeAttackInput& in,
    const SquadronGuardDescriptor& descriptor, SquadronFreeAttackWorld& world, float scale) {
    SquadronFreeAttackDecision unused;
    ArenaHost host(in, world, unused);
    cc13_seg68::SquadronAutoTarget self{};
    self.director_04 = host.director();
    self.squadron_08 = host.squadron();
    self.selector_2c.owner_00 = &self;
    const Handle d = host.guard_descriptor(descriptor);
    return cc13_seg68::bot_within_guard_distance_009f7fe0(host, self, d, scale);
}

}  // namespace bsp

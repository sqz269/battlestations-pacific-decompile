#include "bsp/bot_tasks.hpp"

// Reconstruction of the thirteen bot task bodies of docs/BOT_TASKS.md.
// Addresses in comments are the native sites each line comes from.

namespace bsp {

const BotTaskClassRecord kBotTaskClasses[kBotTaskClassCount] = {
    // kind, factory, ctor, approach ctor, size, vt1, vt2, vt3, vt3 offset, moveto, follow, name
    {BotTaskKind::kCloseToShip, 0x009A2F40u, 0x009A2730u, 0x009A2500u, 0x554u, 0x00D1F4E8u,
     0x00D1F4DCu, 0x00D1F4D8u, 0x448, 0x45C, 0x498, "closetoship"},
    {BotTaskKind::kDepthCharge, 0x009A6970u, 0x009A5240u, 0x009A4DC0u, 0x7BCu, 0x00D1F738u,
     0x00D1F734u, 0x00D1F730u, 0x4F4, 0x508, 0x544, "depthcharge"},
    {BotTaskKind::kDogfight, 0x009AB570u, 0x009A9810u, 0x009A94E0u, 0x758u, 0x00D1F9B0u,
     0x00D1F9ACu, 0x00D1F9A8u, 0x4FC, 0x510, 0x54C, "dogfight"},
    {BotTaskKind::kLand, 0x009B41C0u, 0x009B3240u, 0x009B2E50u, 0x670u, 0x00D1FFA0u, 0x00D1FF94u,
     0x00D1FF90u, 0x4B0, 0x4C4, 0x500, "land"},
    {BotTaskKind::kLevelBomb, 0x009B9030u, 0x009B7990u, 0x009B75E0u, 0x6F8u, 0x00D20210u,
     0x00D2020Cu, 0x00D20208u, 0x4D4, 0x4E8, 0x534, "levelbomb"},
    {BotTaskKind::kDropKamikaze, 0x009AEBE0u, 0x009ADBF0u, 0x009AD720u, 0x74Cu, 0x00D1FC70u,
     0x00D1FC68u, 0x00D1FC64u, 0x4BC, 0x4D0, 0x50C, "dropkamikaze"},
    {BotTaskKind::kDiveBomb, 0x009C8C70u, 0x009C7710u, 0x009C73A0u, 0x7E0u, 0x00D20E18u,
     0x00D20E10u, 0x00D20E0Cu, 0x4DC, 0x4F0, 0x52C, "divebomb"},
    {BotTaskKind::kRetreat, 0x009CA2B0u, 0x009C9D00u, 0x009C9BB0u, 0x564u, 0x00D20F60u,
     0x00D20F58u, 0x00D20F54u, 0x464, 0x478, 0x4C4, "retreat"},
    {BotTaskKind::kStrafe, 0x009CD300u, 0x009CC230u, 0x009CC020u, 0x6FCu, 0x00D210E0u, 0x00D210D8u,
     0x00D210D4u, 0x4CC, 0x4E0, 0x51C, "strafe"},
    {BotTaskKind::kRocket, 0x007B7FD0u, 0x007B6A40u, 0x007B6830u, 0x708u, 0x00D05918u, 0x00D05910u,
     0x00D0590Cu, 0x4D0, 0x4E4, 0x520, "rocket"},
    {BotTaskKind::kKamikaze, 0x009AF720u, 0x009AEEF0u, 0x009AED20u, 0x6A0u, 0x00D1FD40u,
     0x00D1FD3Cu, 0x00D1FD38u, 0x4D4, 0x4E8, 0x524, "kamikaze"},
    {BotTaskKind::kTorpedo, 0x009D4E30u, 0x009D3050u, 0x009D2DA0u, 0x7DCu, 0x00D213C8u,
     0x00D213C0u, 0x00D213BCu, 0x530, 0x544, 0x580, "torpedo"},
    // 009BADB0 allocates the base size and calls 0099C6F0 directly at 009BADEC. There is no
    // derived constructor, no approach controller and no state object; +310h stays null.
    {BotTaskKind::kStop, 0x009BADB0u, 0u, 0u, 0x3F8u, 0x00D20500u, 0u, 0u, 0, 0, 0, "stop"},
};

const BotTaskClassRecord* find_bot_task_class(BotTaskKind kind) noexcept {
    for (const BotTaskClassRecord& record : kBotTaskClasses) {
        if (record.kind == kind) {
            return &record;
        }
    }
    return nullptr;
}

// 009A2D60. Two altitude channels, no attack-distance clamp and no 0099B740 tail.
const BotTaskCruiseProfile kCloseToShipCruiseProfile = {
    pilot_tuning_off::kCloseToShipCruisingAlt, pilot_tuning_off::kCloseToShipDropAlt, -1, -1};

// 009A6500. The second channel takes AimAltRange/2 rather than a drop altitude.
const BotTaskCruiseProfile kDepthChargeCruiseProfile = {
    pilot_tuning_off::kDepthChargeCruisingAlt, pilot_tuning_off::kDepthChargeAimAltRange2, -1,
    pilot_tuning_off::kDepthChargeAttackDist};

// 009C8920. The second channel adds 00BD2F10(0, [00CE5380]) to BeginAltRange/1; this
// projection carries the tuning row and leaves the randomisation to the host, because
// 00BD2F10's distribution was not read. The third channel writes the constant [00CE4C04].
const BotTaskCruiseProfile kDiveBombCruiseProfile = {
    pilot_tuning_off::kDiveBombCruisingAlt, pilot_tuning_off::kDiveBombBeginAltRange1, -1,
    pilot_tuning_off::kDiveBombAttackDist};

// 009D4A70. The second channel does not come from the singleton at all: it is the float
// at *(this->+40Ch), replaced by [00CE3850] when it falls below [00D7A370].
const BotTaskCruiseProfile kTorpedoCruiseProfile = {pilot_tuning_off::kTorpedoCruisingAlt, -1, -1,
                                                    pilot_tuning_off::kTorpedoAttackDist};

float bot_task_speed_ratio(float unit_reference_speed, float tuning_reference_speed) noexcept {
    // 009F9CE0: param_3 = classBlock->+188h / param_3; then if (1.0 < param_3) keep it,
    // otherwise take 1.0f from 00D7A24C. The comparison is strict, so exactly 1.0 takes
    // the constant and not the quotient; both are the same value.
    const float ratio = unit_reference_speed / tuning_reference_speed;
    return (1.0f < ratio) ? ratio : 1.0f;
}

float bot_task_clamp_attack_distance(float current, float tuning_attack_distance,
                                     float speed_ratio) noexcept {
    // 009A6500's tail: fVar5 = tuning->+4ACh * this->+41Ch; if (fVar5 < this->+43Ch)
    // fVar5 = this->+43Ch; this->+43Ch = fVar5. The field only grows.
    const float scaled = tuning_attack_distance * speed_ratio;
    return (scaled < current) ? current : scaled;
}

bool bot_task_altitude_write_applies(const PilotAltitudeChannel& channel) noexcept {
    // 009A6500 step 4, identical in all ten slot +54h overrides:
    //   if (ctl->+38Dh == 0) { if (ctl->+380h < 0 && ctl->+3A9h == 0) { write; dirty = 1; }
    //                          ctl->+3A9h = 0; }
    // The one-shot byte is cleared whenever the lock is clear, written or not; the caller
    // in bot_task_update_cruise_profile does that clearing.
    if (channel.locked) {
        return false;
    }
    return channel.current < 0.0f && !channel.one_shot;
}

bool bot_task_should_break_off(bool base_gate, bool has_target, bool target_marked,
                               bool class_extra, float distance, float tuning_safe_distance,
                               float speed_ratio) noexcept {
    // 009A65F0, 009C8A90 and 009B8D80 share this shape.
    if (!base_gate) {
        return false; // 0099C230 false falls straight through to return 0
    }
    if (!has_target || target_marked) {
        return true; // the latched target is null, or its byte at +5Dh is set
    }
    if (!class_extra) {
        return false;
    }
    // tuning->+4B0h * this->+41Ch <= distance
    return tuning_safe_distance * speed_ratio <= distance;
}

int bot_task_initial_state(const BotTaskClassRecord& record,
                           bool unit_has_follow_target) noexcept {
    // Step 7 of every constructor. 007B8AD0 returns unit->+9D8h == 0, so a unit with
    // nothing at +9D8h takes moveto; the JNZ at 009A52B4 skips the follow LEA.
    return unit_has_follow_target ? record.moveto_state : record.follow_state;
}

BotTaskStepResult bot_task_step_result(bool in_attack_state, bool attack_flag) noexcept {
    // 009A5D80: if (009A5420(this, this->+310h)) return -(this->+46Eh != 0) & 2; return 1;
    if (!in_attack_state) {
        return BotTaskStepResult::kApproaching;
    }
    return attack_flag ? BotTaskStepResult::kAttackingFlagged : BotTaskStepResult::kAttacking;
}

void* bot_task_create(BotTaskHost& host, const BotTaskClassRecord& record, const void* bot,
                      const void* unit, const void* target, BotTaskRecord& out) {
    // 009A6970 and the twelve sibling factories.
    void* task = host.allocate_task(record.object_size); // 009A6991
    if (task == nullptr) {
        return nullptr; // 009A69C3: XOR EAX,EAX, no construction at all
    }

    host.construct_base(task, bot, static_cast<int>(record.kind)); // 009A5266
    out.kind = record.kind;
    out.owner_bot = bot;   // 0099C7CD writes the bot at +3F4h
    out.unit = unit;       // 009F9CE0 writes the unit at +3FCh
    out.target = target;
    out.current_state = 0; // 0099C77C zeroes +310h

    if (record.constructor == 0u) {
        return task; // stop stops here; 009BADF1 only stores the vtable
    }

    host.construct_approach(task, bot, target); // 009A5281, on task+3F8h

    const bool follow = host.unit_has_follow_target(unit);       // 009A52A7
    out.current_state = bot_task_initial_state(record, follow);  // 009A52AE..009A52BC
    host.enter_state(task);                                      // 009A52C7, state->vtable[4]
    host.register_approach(task, task);                          // 009A52CC

    return task;
}

void bot_task_update_cruise_profile(BotTaskHost& host, const BotTaskCruiseProfile& profile,
                                    void* pilot_control, PilotAltitudeChannel& cruise_channel,
                                    PilotAltitudeChannel& second_channel,
                                    PilotAltitudeChannel& third_channel, BotTaskRecord& task) {
    // Slot +54h. Step 1, the empty base 0099B660, has no effect and is not modelled.
    // Step 2 gates everything below on 007B8AD0.
    if (!host.unit_has_follow_target(task.unit)) {
        // A unit that is following keeps whatever altitudes its leader logic set.
        // The attack-distance clamp of step 7 still runs; see below.
    } else {
        if (profile.cruise_alt_offset >= 0) {
            if (bot_task_altitude_write_applies(cruise_channel)) {
                host.write_pilot_altitude(pilot_control, pilot_control_off::kAltCruise,
                                          host.game_tuning_float(profile.cruise_alt_offset));
            }
            if (!cruise_channel.locked) {
                cruise_channel.one_shot = false; // ctl->+3A9h = 0, written or not
            }
        }
        if (profile.second_alt_offset >= 0) {
            if (bot_task_altitude_write_applies(second_channel)) {
                host.write_pilot_altitude(pilot_control, pilot_control_off::kAltSecond,
                                          host.game_tuning_float(profile.second_alt_offset));
            }
            if (!second_channel.locked) {
                second_channel.one_shot = false; // ctl->+3AAh = 0
            }
        }
        if (profile.third_alt_offset >= 0) {
            if (bot_task_altitude_write_applies(third_channel)) {
                host.write_pilot_altitude(pilot_control, pilot_control_off::kAltThird,
                                          host.game_tuning_float(profile.third_alt_offset));
            }
            if (!third_channel.locked) {
                third_channel.one_shot = false; // ctl->+3ABh = 0
            }
        }
    }

    // Step 7. 009A6500 runs it outside the 007B8AD0 gate, after the altitude block.
    if (profile.attack_distance_offset >= 0) {
        task.attack_distance = bot_task_clamp_attack_distance(
            task.attack_distance, host.game_tuning_float(profile.attack_distance_offset),
            task.speed_ratio);
    }

    // Step 8, the 0099B740 tail, is modelled separately: its gate reads +2F4h and +2FCh,
    // which this packet did not establish.
}

} // namespace bsp

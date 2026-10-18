// cc13 segment 52 leftovers (packet cc13-seg52U). See include/bsp/cc13_seg52U.hpp and
// docs/CC13_SEG52U.md for the evidence and the adoption notes. Instruction addresses in the
// comments are the image's; names are hypotheses, not recovered symbols.

#include "bsp/cc13_seg52U.hpp"

namespace bsp::cc13_seg52U {

namespace {

// 008390C0 / 00839160 share one body; only the class's vtable slot differs (+18h / +5Ch).
// The vector is the _SECURE_SCL std::vector<{int kind; float value}> at owner+1DCh
// (+1E0h first, +1E4h last); every checked-iterator test the image makes is kept.
template <class Accepts>
float kind_value_lookup(void* owner, Seg52UHost& host, Accepts accepts) {
    void* const vec = add(owner, 0x1DC);                                   // 008390D0
    auto* cur = at<std::uint8_t*>(owner, 0x1E0);                           // 008390C3
    if (cur > at<std::uint8_t*>(owner, 0x1E4)) host.invalid_parameter_00bf6713();  // 008390D8
    std::uint8_t* const last = at<std::uint8_t*>(vec, 0x8);                // 008390DD
    if (at<std::uint8_t*>(vec, 0x4) > last) host.invalid_parameter_00bf6713();     // 008390E5
    for (;;) {
        // 008390F0: the iterator-compatibility test; the container is this vector, so only a
        // null container reports.
        if (vec == nullptr) host.invalid_parameter_00bf6713();
        if (cur == last) return 0.0f;                                      // 00839148 FLDZ
        if (vec == nullptr) host.invalid_parameter_00bf6713();             // 00839105
        if (cur >= at<std::uint8_t*>(vec, 0x8)) host.invalid_parameter_00bf6713();  // 0083910F
        if (accepts(at<std::int32_t>(cur, 0))) break;                      // 0083911F
        if (cur >= at<std::uint8_t*>(vec, 0x8)) host.invalid_parameter_00bf6713();  // 0083912A
        cur += 8;                                                          // 0083912F
    }
    if (cur >= at<std::uint8_t*>(vec, 0x8)) host.invalid_parameter_00bf6713();      // 00839139
    return at<float>(cur, 0x4);                                            // 0083913E
}

}  // namespace

// 0084DDA0. Image order throughout; block+224h is the block's squadron unit, unit+3D0h its
// leader plane (+900h state, +184h byte), unit+3CCh the plane count, unit+35Ch the class.
bool squadron_block_begin_command_0084dda0(void* block, std::uint32_t flag, Seg52UHost& host) {
    // 0084DDBF: the byte argument picks which order word is examined after the base begin.
    const std::uint32_t order = (flag & 0xFFu) != 0 ? at<std::uint32_t>(block, 0x54)   // 0084DDC6
                                                    : at<std::uint32_t>(block, 0x188); // 0084DDCF
    if (!host.weapon_director_begin_command_base_0071f600(block, flag)) {  // 0084DDDC
        return false;                                                      // 0084DDF5
    }
    if (order == kOrderMoveTo) {                                           // 0084DDFD
        void* const target_record = add(block, 0x58);                      // 0084DE0A
        void* const target = host.command_target_resolve_object_00521ea0(target_record);
        void* const unit = ptr(block, 0x224);                              // 0084DE1C
        if (target != nullptr && target == unit) {                         // 0084DE14 / 0084DE22
            void* const leader = ptr(unit, 0x3D0);                         // 0084DE2A
            bool settled = false;
            if (leader != nullptr && at<std::uint8_t>(leader, 0x184) != 0) {
                settled = true;                                            // 0084DE3A -> 0084DF2D
            } else if (unit != nullptr && at<std::int32_t>(unit, 0x3CC) <= 1 &&  // 0084DE40..4F
                       leader != nullptr &&                                // 0084DE51
                       at<std::int32_t>(leader, 0x900) == kPlaneStateOnWater) {  // 0084DE55
                void* const plane_class = ptr(unit, 0x35C);                // 0084DE60
                // 0084DE69 FSTP double: the speed is spilled as a double.
                const double speed = host.plane_speed_vslot_38(leader);
                const double floor_speed =
                    host.plane_class_level_flight_speed_007c47f0(plane_class) *
                    kLevelFlightFraction;                                  // 0084DE71..0084DE76
                // 0084DE82 FCOMIP / JBE: a stop only when speed < floor (false on NaN).
                if (speed < floor_speed) {
                    at<std::uint32_t>(block, 0x54) = kOrderStop;           // 0084DE88
                    at<float>(target_record, 0x8) = host.global_float(kDefaultCommandPosition);
                    at<float>(target_record, 0xC) =
                        host.global_float(kDefaultCommandPosition + 4);    // 0084DEA4
                    at<float>(target_record, 0x10) =
                        host.global_float(kDefaultCommandPosition + 8);    // 0084DEB1
                    at<std::uint16_t>(target_record, 0x0) = 0;             // 0084DEB9
                    at<std::uint32_t>(target_record, 0x4) = 0;             // 0084DEBC
                    at<std::uint8_t>(target_record, 0x0) = 0;              // 0084DEBF
                    at<std::uint16_t>(target_record, 0x2) = 0;             // 0084DEC1
                    at<float>(target_record, 0x14) = 0.0f;                 // 0084DEC5
                    settled = true;
                }
            }
            if (!settled) {
                // 0084DECC: re-aim the moveto at the leader's clamped position with a random
                // height: y = U(0, 100) + tuning+53Ch. The leader is not null-tested here.
                void* const pos =
                    host.entity_world_position_00427eb0(ptr(ptr(block, 0x224), 0x3D0));
                float point[3];
                host.world_clamp_point_to_map_bounds_007ef8f0(host.global_ptr(0x00E188A8),
                                                               point, pos);  // 0084DEE9
                void* const tuning = host.game_tuning_singleton_0042e740();  // 0084DEEE
                const double r = host.random_uniform_float_range_00bd2f10(
                    kRandomStream, 0.0f, kAltitudeJitterHigh);             // 0084DF0C
                // 0084DF11 FADD float / 0084DF1E FSTP float: one rounding at 24-bit precision.
                point[1] = static_cast<float>(r) + at<float>(tuning, 0x53C);
                host.scene_property_value_construct_vector3_00467c80(target_record, point,
                                                                     0.0f);  // 0084DF28
            }
            at<std::int32_t>(block, 0x48) = 1;                             // 0084DF31
        }
    }
    // 0084DF39: a land or retreat outside session mode 2 hands the leader back to its AI.
    void* const game = host.global_ptr(0x00E188A8);
    if (at<std::int32_t>(game, 0x1FE4) != kSessionModeNoResume &&
        (order == kOrderLand || order == kOrderRetreat)) {                 // 0084DF4B / 0084DF53
        bool send = at<std::uint8_t>(block, 0x228) == 0;                   // 0084DF5F
        if (!send) {
            void* const leader = ptr(ptr(block, 0x224), 0x3D0);            // 0084DF6D
            send = leader == nullptr || at<std::uint8_t>(leader, 0x184) == 0;  // 0084DF75 / 7D
        }
        if (send) {
            void* const leader = ptr(ptr(block, 0x224), 0x3D0);            // 0084DF85
            host.plane_session_prepare_0077c470(leader, 2, 0);             // 0084DF8E
            // The stack message (image ESP+18h): base construct, then +18h word, +1Ah byte,
            // +4h dword, +1Ch byte and the type-87 vtable; the image leaves the rest as the
            // base constructor wrote it.
            alignas(4) std::uint8_t message[0x20] = {};
            host.session_message_construct_base_0075b430(message, kMsgResumeAiControl);
            at<std::uint16_t>(message, 0x18) = 0;                          // 0084DF9E
            at<std::uint8_t>(message, 0x1A) = 0;                           // 0084DFA3
            at<std::uint32_t>(message, 0x4) = 1;                           // 0084DFA7
            at<std::uint8_t>(message, 0x1C) = 0;                           // 0084DFAF
            at<std::uint32_t>(message, 0x0) = kMsgResumeAiControlVtable;   // 0084DFB3
            host.session_route_message_0077c2a0(ptr(ptr(block, 0x224), 0x3D0), message, 0, 0);
            at<std::uint32_t>(message, 0x0) = kMsgRootVtable;              // 0084DFDF, inline dtor
        }
        host.block_vslot_40(block, 0);                                     // 0084DFEF
        at<std::uint8_t>(block, 0x228) = 0;                                // 0084DFF1
    }
    return true;                                                           // 0084DFFD
}

float kind_value_lookup_008390c0(void* owner, void* cls, Seg52UHost& host) {
    return kind_value_lookup(owner, host,
                             [&](std::int32_t kind) { return host.class_accepts_vslot_18(cls, kind); });
}

float kind_value_lookup_00839160(void* owner, void* cls, Seg52UHost& host) {
    return kind_value_lookup(owner, host,
                             [&](std::int32_t kind) { return host.class_accepts_vslot_5c(cls, kind); });
}

// 00846A80. The vtable words first (00846A9D..00846AD9), then the three member vectors in
// reverse declaration order, then the unit base.
void shipyard_destruct_00846a80(void* shipyard, Seg52UHost& host) {
    for (const auto& vt : kShipyardVtables) {
        at<std::uint32_t>(shipyard, vt[0]) = vt[1];
    }
    host.shipyard_slots_tidy_008468f0(add(shipyard, 0x790));               // 00846AF1
    host.shipyard_hangars_tidy_008468a0(add(shipyard, 0x780));             // 00846B01
    host.shipyard_stock_tidy_00846850(add(shipyard, 0x770));               // 00846B11
    host.unit_game_object_destruct_00959940(shipyard);                     // 00846B20
}

}  // namespace bsp::cc13_seg52U

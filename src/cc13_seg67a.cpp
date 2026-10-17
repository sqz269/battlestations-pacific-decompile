// cc13_seg67a: the plane `closetoship` bot task (packet cc13-seg67a). See
// include/bsp/cc13_seg67a.hpp and docs/CC13_SEG67A.md. Hypothesis names,
// build-tested only, nothing wired.

#include "bsp/cc13_seg67a.hpp"

#include <cstring>

namespace bsp {
namespace {

// Task offsets.
constexpr std::size_t kTaskState = 0x310;          // current state
constexpr std::size_t kTaskRegistrySlot = 0x30C;   // handed to 00411170 / 00411450
constexpr std::size_t kTaskApproach = 0x3F8;
constexpr std::size_t kTaskUnit = 0x3FC;           // approach+4
constexpr std::size_t kTaskControl = 0x404;        // approach+0Ch
constexpr std::size_t kTaskShip = 0x438;           // approach+40h
constexpr std::size_t kTaskMoveToAlt = 0x440;      // approach+48h
constexpr std::size_t kTaskBeginDist = 0x444;      // approach+4Ch
constexpr std::size_t kTaskRegistry = 0x448;       // approach+50h
constexpr std::size_t kTaskMoveTo = 0x45C;         // approach+64h
constexpr std::size_t kTaskFollow = 0x498;         // approach+A0h
constexpr std::size_t kTaskStayClose = 0x530;      // approach+138h
constexpr std::size_t kTaskDropStarted = 0x550;

// Approach offsets.
constexpr std::size_t kApUnit = 0x04;
constexpr std::size_t kApDescriptor = 0x08;
constexpr std::size_t kApControl = 0x0C;
constexpr std::size_t kApPlan = 0x18;
constexpr std::size_t kApSpeedRatio = 0x24;
constexpr std::size_t kApElement = 0x2C;
constexpr std::size_t kApShip = 0x40;              // element+14h
constexpr std::size_t kApDropOrdnance = 0x44;
constexpr std::size_t kApAltitude = 0x48;
constexpr std::size_t kApBeginDist = 0x4C;
constexpr std::size_t kApRegistry = 0x50;
constexpr std::size_t kApMoveTo = 0x64;
constexpr std::size_t kApFollow = 0xA0;
constexpr std::size_t kApStayClose = 0x138;

// Entity offsets.
constexpr std::size_t kEntPoseFresh = 0xC8;
constexpr std::size_t kEntX = 0xFC;
constexpr std::size_t kEntY = 0x100;
constexpr std::size_t kEntZ = 0x104;

// Constants read from the image.
constexpr float kMinusZero = -0.0f;                       // [00D7A208]
constexpr float kMinusOne = -1.0f;                        // [00D7A260]
constexpr double kNoConeDistance = 450.0;                 // [00D1F4C0] qword
constexpr float kDropCone = 1.0471975803375244f;          // [00D05AAC] (pi/3)
constexpr float kRedrawHigh = 2.0f;                       // [00CE3958]
constexpr float kPitchX0 = 20.0f;                         // [00CE3930]
constexpr float kPitchY0 = 0.20000000298023224f;          // [00CE54A0]
constexpr float kPitchX1 = 120.0f;                        // [00D05804]
constexpr double kSpeedFactor = 1.2000000476837158;       // [00CEC160] qword
constexpr double kNarrowHeading = 0.8726646304130554;     // [00D057E0] qword (50 deg)
constexpr float kNearWhenNarrow = 350.0f;                 // [00D1F4C8]
constexpr float kNearWhenWide = 500.0f;                   // [00CE397C]
constexpr double kSteerAhead = 50.0;                      // [00CE3938] qword
constexpr double kBeginDistScale = 1500.0;                // [00CF00F8] qword
constexpr float kFollowValue = 100.0f;                    // [00CE3D08]
constexpr float kStayCloseRange = 1500.0f;                // [00CED724]
constexpr std::size_t kTuningReferenceSpeed = 0x42C;      // Pilot/CloseToShip/ReferenceSpeed
constexpr std::size_t kControlAltitude = 0x398;

constexpr const char* kNameMoveTo = "moveto (CloseToShip)";       // 00D1F458
constexpr const char* kNameFollow = "follow (CloseToShip)";       // 00D1F440
constexpr const char* kNameStayClose = "stayclose (CloseToShip)"; // 00D1F428
constexpr const char* kKeyShipName = "shipName";                  // 00D1F494
constexpr const char* kKeyDropOrdnance = "dropOrdnance";          // 00D1F484
constexpr const char* kKeyBeginDist = "beginDist";                // 00D1F478
constexpr const char* kKeyDropStarted = "dropStarted";            // 00D1F550

template <class T>
T& at(void* base, std::size_t offset) {
    return cc13_seg67a_at<T>(base, offset);
}

void* ptr(void* base, std::size_t offset) {
    return static_cast<std::uint8_t*>(base) + offset;
}

void refresh_if_stale(Cc13Seg67aHost& host, void* entity) {
    if (at<std::uint8_t>(entity, kEntPoseFresh) == 0) {
        host.entity_pose_refresh_world_00414db0(entity);
    }
}

Cc13Seg67aArchiveValue name_value(const char* name) {
    return {0, reinterpret_cast<std::uintptr_t>(name)};
}

Cc13Seg67aArchiveValue ref_value(std::int32_t type, void* address) {
    return {type, reinterpret_cast<std::uintptr_t>(address)};
}

std::uintptr_t float_bits(float value) {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

// The image's absolute value: x when x > 0, else -0.0 - x (an unordered x takes
// the second arm and stays NaN).
float abs_by_minus_zero(float x) {
    return x > 0.0f ? x : kMinusZero - x;
}

void set_vtable(void* object, std::size_t offset, std::uint32_t vtable) {
    at<std::uint32_t>(object, offset) = vtable;
}

} // namespace

// 009A1B70, read from the listing (009A1B70-009A1BAF). ECX = approach block;
// EDI = approach+50h is the registry for all three adds.
void close_to_ship_register_states_009a1b70(Cc13Seg67aHost& host, void* approach) {
    void* registry = ptr(approach, kApRegistry);
    host.state_registry_add_00411e70(registry, kNameMoveTo, ptr(approach, kApMoveTo));
    host.state_registry_add_00411e70(registry, kNameFollow, ptr(approach, kApFollow));
    host.state_registry_add_00411e70(registry, kNameStayClose, ptr(approach, kApStayClose));
}

// 009A1BB0 (009A1BB0-009A1BCB). 009A1BB3 loads ECX = task+3FCh (the unit) for
// 007B8AD0.
void* close_to_ship_follow_state_for_role_009a1bb0(Cc13Seg67aHost& host, void* task) {
    const bool leader = host.unit_is_flight_leader_007b8ad0(at<void*>(task, kTaskUnit));
    return leader ? ptr(task, kTaskMoveTo) : ptr(task, kTaskFollow);
}

// 009A1C30 (009A1C30-009A1C6A, RET 4). ESI = approach+40h, or approach+4 when
// that is null; pose refresh while +C8h is zero; out = {+FCh, +100h, +104h}.
float* close_to_ship_target_point_009a1c30(Cc13Seg67aHost& host, void* approach, float* out) {
    void* entity = at<void*>(approach, kApShip);
    if (entity == nullptr) {
        entity = at<void*>(approach, kApUnit);
    }
    refresh_if_stale(host, entity);
    out[0] = at<float>(entity, kEntX);
    out[1] = at<float>(entity, kEntY);
    out[2] = at<float>(entity, kEntZ);
    return out;
}

// 009A1CA0 (009A1CA0-009A1CF9, RET 4). The SEH frame only tracks state 0.
void* point_observer_element_construct_009a1ca0(Cc13Seg67aHost& host, void* element,
                                                void* observed) {
    at<std::uint32_t>(element, 0x4) = 0;
    at<std::uint32_t>(element, 0x8) = 0;
    at<std::uint32_t>(element, 0xC) = 0;
    set_vtable(element, 0x0, kPointObserverElementVtable);   // 009A1CD2
    at<void*>(element, 0x14) = observed;
    at<std::uint8_t>(element, 0x10) = 1;
    if (observed != nullptr) {
        host.observer_register_pair_00694a60(observed, element);   // 009A1CE3
    }
    return element;
}

// 009A1D60 (009A1D60-009A1DBA, RET 8).
void* close_to_ship_approach_construct_009a1d60(Cc13Seg67aHost& host, void* approach,
                                                void* owner, void* ship) {
    void* tuning = host.game_tuning_singleton_0042e740();                 // 009A1D64
    host.bot_approach_construct_009f9ce0(approach, owner,
                                         at<float>(tuning, kTuningReferenceSpeed));  // 009A1D7A
    set_vtable(approach, 0x0, kCloseToShipApproachBaseVtable);           // 009A1D89
    point_observer_element_construct_009a1ca0(host, ptr(approach, kApElement), ship);
    set_vtable(approach, kApElement, kShipObserverElementVtable);         // 009A1D94
    at<std::uint8_t>(approach, kApDropOrdnance) = 0;                      // 009A1DA6
    // FLD +24h, FMUL qword 1500.0, FSTP: a float times 1500 is exact in double,
    // so one rounding in either precision mode.
    at<float>(approach, kApBeginDist) = static_cast<float>(
        static_cast<double>(at<float>(approach, kApSpeedRatio)) * kBeginDistScale);
    at<float>(approach, kApAltitude) =
        at<float>(at<void*>(approach, kApControl), kControlAltitude);    // 009A1DB0
    return approach;
}

// 009A1DC0 (009A1DC0-009A1EF4, RET 8). The observer element is built inline
// (no 009A1CA0 call, no base vtable store) with a null observed object.
void* close_to_ship_approach_construct_from_stream_009a1dc0(Cc13Seg67aHost& host,
                                                            void* approach, void* owner,
                                                            void* archive) {
    void* tuning = host.game_tuning_singleton_0042e740();                 // 009A1DE0
    host.bot_approach_construct_009f9ce0(approach, owner,
                                         at<float>(tuning, kTuningReferenceSpeed));  // 009A1DF6
    set_vtable(approach, 0x0, kCloseToShipApproachBaseVtable);
    void* element = ptr(approach, kApElement);
    at<std::uint32_t>(element, 0x4) = 0;
    at<std::uint32_t>(element, 0x8) = 0;
    at<std::uint32_t>(element, 0xC) = 0;
    at<void*>(element, 0x14) = nullptr;
    at<std::uint8_t>(element, 0x10) = 1;
    set_vtable(element, 0x0, kShipObserverElementVtable);

    void* ship = nullptr;   // the stack slot the type-4 record points at
    host.archive_vcall_10_read(archive, name_value(kKeyShipName), ref_value(4, &ship));  // 009A1E48
    void* current = at<void*>(element, 0x14);
    if (current != ship) {
        if (current != nullptr) {
            host.observer_unregister_pair_006952a0(current, element);    // 009A1E5B
        }
        at<void*>(element, 0x14) = ship;
        if (ship != nullptr) {
            host.observer_register_pair_00694a60(ship, element);         // 009A1E6F
        }
    }
    host.archive_vcall_10_read(archive, name_value(kKeyDropOrdnance),
                               ref_value(3, ptr(approach, kApDropOrdnance)));   // 009A1EA2
    host.archive_vcall_10_read(archive, name_value(kKeyBeginDist),
                               ref_value(2, ptr(approach, kApBeginDist)));      // 009A1ED2
    at<float>(approach, kApAltitude) =
        at<float>(at<void*>(approach, kApControl), kControlAltitude);
    return approach;
}

// 009A1F00 (009A1F00-009A1F6D, RET 4). Draw: 00BD2F10 at 009A1F54, stream 1,
// U(1.0, 2.0).
void* close_to_ship_stay_close_construct_009a1f00(Cc13Seg67aHost& host, void* state,
                                                  void* approach) {
    at<void*>(state, 0x4) = approach;
    at<std::uint32_t>(state, 0xC) = 0;
    at<std::uint32_t>(state, 0x10) = 0;
    at<std::uint32_t>(state, 0x14) = 0;
    at<std::uint32_t>(state, 0x8) = 0;
    set_vtable(state, 0x0, kCloseToShipStayCloseVtable);
    at<std::uint8_t>(state, 0x1C) = 0;
    at<float>(state, 0x18) = host.random_uniform_00bd2f10(1, 1.0f, kRedrawHigh);
    return state;
}

// 009A1FC0 (009A1FC0-009A2167, RET 8). state+18h is the release delay, +1Ch the
// latch. Branches follow the listing, not the decompiler:
//  - latched: AL = 007B9140(unit, 0); true returns 1, false falls to the reset.
//  - not latched, delay > 0 or unordered (COMISS/JC): delay -= dt, return 1.
//  - delay <= 0: below 450 (FCOMIP/JA, 450 > distance) release at once; else
//    release only inside the 60-degree cone (COMISS/JBE: an unordered angle
//    fails), else return 0 without touching the delay.
//  - release: 007BBBA0, latch = 1, delay = U(1, 2) (00BD2F10 at 009A20FA, stream
//    1), return 1.
//  - reset (latched and 007B9140 false): approach+44h = 0, 007C1D80, latch = 0,
//    return 0.
bool close_to_ship_try_drop_ordnance_009a1fc0(Cc13Seg67aHost& host, void* state,
                                              float distance, float dt) {
    void* approach = at<void*>(state, 0x4);
    if (at<std::uint8_t>(approach, kApDropOrdnance) == 0) {
        return false;
    }
    void* unit = at<void*>(approach, kApUnit);
    void* ship = at<void*>(approach, kApShip);
    std::uint8_t& latch = at<std::uint8_t>(state, 0x1C);
    float& delay = at<float>(state, 0x18);

    if (latch == 0) {
        if (delay <= 0.0f) {
            if (!(kNoConeDistance > static_cast<double>(distance))) {
                refresh_if_stale(host, ship);
                const float sx = at<float>(ship, kEntX);
                const float sz = at<float>(ship, kEntZ);
                refresh_if_stale(host, unit);
                const float ux = at<float>(unit, kEntX);
                const float uz = at<float>(unit, kEntZ);
                const float v[2] = {sx - ux, sz - uz};
                const float unit_heading = host.unit_vcall_50_heading(unit);   // 009A2080
                const float bearing = host.heading_angle_00414eb0(v);          // 009A2092
                const float off = abs_by_minus_zero(
                    host.wrapped_angle_subtract_00438b10(bearing, unit_heading));  // 009A209B
                if (!(kDropCone > off)) {
                    return false;
                }
            }
            host.unit_request_ordnance_release_007bbba0(unit);                 // 009A20DB
            latch = 1;
            delay = host.random_uniform_00bd2f10(1, 1.0f, kRedrawHigh);       // 009A20FA
            return true;
        }
    } else if (host.unit_can_drop_ordnance_007b9140(unit, 0)) {               // 009A2116
        return true;
    }

    if (latch == 0) {
        delay = delay - dt;                                                    // 009A2154
        return true;
    }
    at<std::uint8_t>(at<void*>(state, 0x4), kApDropOrdnance) = 0;              // 009A2139
    host.plane_reload_bomb_platforms_007c1d80(at<void*>(at<void*>(state, 0x4), kApUnit));
    latch = 0;
    return false;
}

// 009A2170 (009A2170-009A241F, RET 4). ECX for 009FB800 and 009F9D90 is state+4
// (EDI), not the approach itself.
void close_to_ship_stay_close_tick_009a2170(Cc13Seg67aHost& host, void* state, float dt) {
    void* slot = ptr(state, 0x4);
    void* approach = at<void*>(state, 0x4);
    void* ship = at<void*>(approach, kApShip);
    void* unit = at<void*>(approach, kApUnit);
    if (unit == nullptr || at<void*>(approach, kApControl) == nullptr) {
        return;
    }
    refresh_if_stale(host, unit);
    approach = at<void*>(slot, 0);
    const float dy = at<float>(unit, kEntY) - at<float>(approach, kApAltitude);
    const float gain = host.interpolate_clamped_00419010(kPitchX0, kPitchY0, kPitchX1, 1.0f,
                                                         abs_by_minus_zero(dy));    // 009A220E
    host.pilot_command_pitch_from_altitude_009fb800(slot, at<float>(approach, kApAltitude),
                                                    gain);                         // 009A2222
    approach = at<void*>(slot, 0);
    void* plan = at<void*>(approach, kApPlan);
    const float level = host.plane_class_level_flight_speed_007c47f0(
        at<void*>(approach, kApDescriptor));                                         // 009A222F
    // A float times the float-representable 1.2000000476837158 is exact in
    // double: one rounding in either precision mode.
    at<float>(plan, 0x2B4) = static_cast<float>(static_cast<double>(level) * kSpeedFactor);
    at<std::uint8_t>(plan, 0x2B0) = 0;
    at<std::uint32_t>(plan, 0x2D8) = 1;

    if (ship != nullptr) {
        refresh_if_stale(host, ship);
        const float sx = at<float>(ship, kEntX);
        const float sz = at<float>(ship, kEntZ);
        refresh_if_stale(host, unit);
        float v[2] = {sx - at<float>(unit, kEntX), sz - at<float>(unit, kEntZ)};
        const float distance = host.vector2_length_00414c60(v);                    // 009A22C7
        const float bearing = host.heading_angle_00414eb0(v);                      // 009A22D4
        const float unit_heading = host.unit_vcall_50_heading(unit);               // 009A22E5
        float off = host.wrapped_angle_subtract_00438b10(unit_heading, bearing);   // 009A22EB
        std::uint32_t bits = 0;
        std::memcpy(&bits, &off, sizeof bits);
        bits &= 0x7FFFFFFFu;                                                        // 009A22F8
        std::memcpy(&off, &bits, sizeof off);
        const bool narrow = kNarrowHeading > static_cast<double>(off);             // JBE: NaN wide
        if (!close_to_ship_try_drop_ordnance_009a1fc0(host, state, distance, dt)) {
            const float near_distance = narrow ? kNearWhenNarrow : kNearWhenWide;
            if (!(near_distance > distance)) {
                // 009A2369..009A2395: swap the two components through a sum, each
                // step rounded at its float store.
                const float a = v[0];
                const float b = v[1];
                const float s = static_cast<float>(static_cast<double>(a) + b);
                const float b2 = static_cast<float>(static_cast<double>(s) - b);
                const float a2 = static_cast<float>(static_cast<double>(s) - b2);
                v[0] = a2;
                v[1] = b2;
                float out[2] = {0.0f, 0.0f};
                const float* n = host.vector2_normalize_004192e0(out, v);           // 009A2399
                v[0] = n[0];
                v[1] = n[1];
                // 50 (qword) times a float is exact in double; one rounding.
                const float ax = static_cast<float>(kSteerAhead * v[0]);
                const float az = static_cast<float>(kSteerAhead * v[1]);
                const float point[2] = {ax + sx, az + sz};
                host.bot_approach_steer_to_point_009f9d90(slot, point);            // 009A23E4
                return;
            }
        }
        plan = at<void*>(at<void*>(slot, 0), kApPlan);                             // 009A23F2
        at<std::uint32_t>(plan, 0x2CC) = 1;
    } else {
        plan = at<void*>(at<void*>(slot, 0), kApPlan);                             // 009A2403
        at<std::uint32_t>(plan, 0x2CC) = 1;
    }
    at<float>(plan, 0x2C4) = 0.0f;
}

// 009A2440 (009A2440-009A24DB). The registry destructor gets approach+50h, or
// null for a null approach (the compiler's base-pointer adjustment).
void close_to_ship_approach_destruct_009a2440(Cc13Seg67aHost& host, void* approach) {
    host.bot_state_base_destruct_007b45f0(ptr(approach, kApStayClose));        // 009A246E
    void* follow = ptr(approach, kApFollow);
    host.member_destruct_006cdd70(ptr(follow, 0x18));                         // 009A2485
    host.bot_state_base_destruct_007b45f0(follow);                             // 009A2491
    host.bot_state_moveto_destruct_007b65e0(ptr(approach, kApMoveTo));        // 009A249E
    host.state_registry_destruct_004116d0(approach != nullptr ? ptr(approach, kApRegistry)
                                                              : nullptr);      // 009A24B0
    set_vtable(approach, 0x0, kCloseToShipApproachBaseVtable);
    host.observer_element_destruct_0064a610(ptr(approach, kApElement));       // 009A24C6
}

// 009A2500 (009A2500-009A25EE, RET 8).
void* close_to_ship_approach_block_construct_009a2500(Cc13Seg67aHost& host, void* approach,
                                                      void* owner, void* ship) {
    close_to_ship_approach_construct_009a1d60(host, approach, owner, ship);     // 009A252A
    void* registry = ptr(approach, kApRegistry);
    host.state_registry_construct_00411e20(registry);                         // 009A253C
    const float altitude = at<float>(approach, kApAltitude);
    const float begin = at<float>(approach, kApBeginDist);
    set_vtable(approach, 0x0, kCloseToShipApproachBlockVtable);
    set_vtable(registry, 0x0, kCloseToShipApproachBlockRegistryVtable);
    host.bot_state_moveto_construct_009c2ac0(ptr(approach, kApMoveTo), approach, ship, altitude,
                                             altitude, begin);                 // 009A2575
    host.bot_state_follow_construct_009c2980(ptr(approach, kApFollow), approach,
                                             kFollowValue);                    // 009A2592
    close_to_ship_stay_close_construct_009a1f00(host, ptr(approach, kApStayClose), approach);
    host.state_registry_add_00411e70(registry, kNameMoveTo, ptr(approach, kApMoveTo));
    host.state_registry_add_00411e70(registry, kNameFollow, ptr(approach, kApFollow));
    host.state_registry_add_00411e70(registry, kNameStayClose, ptr(approach, kApStayClose));
    return approach;
}

// 009A2610 (009A2610-009A2703, RET 8). As 009A2500 over 009A1DC0; the moveto
// state gets the ship the stream restored (approach+40h).
void* close_to_ship_approach_block_construct_from_stream_009a2610(Cc13Seg67aHost& host,
                                                                  void* approach, void* owner,
                                                                  void* archive) {
    close_to_ship_approach_construct_from_stream_009a1dc0(host, approach, owner, archive);
    void* registry = ptr(approach, kApRegistry);
    host.state_registry_construct_00411e20(registry);                         // 009A264E
    const float altitude = at<float>(approach, kApAltitude);
    void* ship = at<void*>(approach, kApShip);
    const float begin = at<float>(approach, kApBeginDist);
    set_vtable(approach, 0x0, kCloseToShipApproachBlockVtable);
    set_vtable(registry, 0x0, kCloseToShipApproachBlockRegistryVtable);
    host.bot_state_moveto_construct_009c2ac0(ptr(approach, kApMoveTo), approach, ship, altitude,
                                             altitude, begin);                 // 009A268A
    host.bot_state_follow_construct_009c2980(ptr(approach, kApFollow), approach,
                                             kFollowValue);                    // 009A26A7
    close_to_ship_stay_close_construct_009a1f00(host, ptr(approach, kApStayClose), approach);
    host.state_registry_add_00411e70(registry, kNameMoveTo, ptr(approach, kApMoveTo));
    host.state_registry_add_00411e70(registry, kNameFollow, ptr(approach, kApFollow));
    host.state_registry_add_00411e70(registry, kNameStayClose, ptr(approach, kApStayClose));
    return approach;
}

// 009A2730 (009A2730-009A27E5, RET 8).
void* close_to_ship_task_construct_009a2730(Cc13Seg67aHost& host, void* task, void* bot,
                                            void* ship) {
    host.bot_task_construct_base_0099c6f0(task, bot, 0);                      // 009A2755
    close_to_ship_approach_block_construct_009a2500(host, ptr(task, kTaskApproach),
                                                    at<void*>(bot, 0x50), ship);
    set_vtable(task, 0x0, kCloseToShipTaskVtable);
    set_vtable(task, kTaskApproach, kCloseToShipApproachVtable);
    set_vtable(task, kTaskRegistry, kCloseToShipRegistryVtable);
    at<float>(task, kTaskDropStarted) = kMinusOne;
    const bool leader = host.unit_is_flight_leader_007b8ad0(at<void*>(task, kTaskUnit));
    void* state = leader ? ptr(task, kTaskMoveTo) : ptr(task, kTaskFollow);
    at<void*>(task, kTaskState) = state;
    host.state_vcall_04_enter(state);                                          // 009A27C9
    host.bot_approach_attach_task_009f9980(ptr(task, kTaskApproach), task);    // 009A27CE
    return task;
}

std::uint32_t close_to_ship_robots_row_009a2800() noexcept { return kCloseToShipRobotsRow; }
std::int32_t close_to_ship_slot10_009a2890() noexcept { return 3; }
std::uint32_t close_to_ship_command_class_009a28b0() noexcept { return kCloseToShipCommandClass; }
bool close_to_ship_slot30_009a28c0() noexcept { return true; }

// 009A2920 (009A2920-009A2995, RET). Slot +174h is called twice.
std::int32_t close_to_ship_command_still_valid_009a2920(Cc13Seg67aHost& host, void* task) {
    void* ctl = at<void*>(task, kTaskControl);
    if (ctl == nullptr || host.control_vcall_174_command_class(ctl) == 0) {
        return 2;
    }
    if (host.control_vcall_174_command_class(at<void*>(task, kTaskControl)) !=
        kCloseToShipCommandClass) {
        return 0;
    }
    ctl = at<void*>(task, kTaskControl);
    void* ship = at<void*>(task, kTaskShip);
    std::uint8_t scratch[0x14] = {};
    void* handle = host.control_vcall_178(ctl, scratch);                      // 009A296E
    return host.resolve_object_00521ea0(handle) == ship ? 1 : 0;              // 009A2972
}

// 009A29B0 (009A29B0-009A29DE, RET 4). No null check on the incoming state.
void close_to_ship_set_state_009a29b0(Cc13Seg67aHost& host, void* task, void* state) {
    void* current = at<void*>(task, kTaskState);
    if (state == current) {
        return;
    }
    if (current != nullptr) {
        host.state_vcall_08_exit(current);
    }
    at<void*>(task, kTaskState) = state;
    host.state_vcall_04_enter(state);
}

// 009A29F0 (009A29F0-009A2AEE, RET 4). With no ship, the role state; with a ship
// and the task in moveto or follow, stayclose once inside 1500 (FCOMIP/JBE:
// an unordered distance stays).
void close_to_ship_select_state_009a29f0(Cc13Seg67aHost& host, void* task, float /*dt*/) {
    if (at<void*>(task, kTaskShip) == nullptr) {
        const bool leader = host.unit_is_flight_leader_007b8ad0(at<void*>(task, kTaskUnit));
        void* role = leader ? ptr(task, kTaskMoveTo) : ptr(task, kTaskFollow);
        void* current = at<void*>(task, kTaskState);
        if (role != current) {
            if (current != nullptr) {
                host.state_vcall_08_exit(current);
            }
            at<void*>(task, kTaskState) = role;
            host.state_vcall_04_enter(role);
        }
        return;
    }
    void* current = at<void*>(task, kTaskState);
    if (current != ptr(task, kTaskMoveTo) && current != ptr(task, kTaskFollow)) {
        return;
    }
    void* unit = at<void*>(task, kTaskUnit);
    refresh_if_stale(host, unit);
    void* ship = at<void*>(task, kTaskShip);
    refresh_if_stale(host, ship);
    const float v[2] = {at<float>(ship, kEntX) - at<float>(unit, kEntX),
                        at<float>(ship, kEntZ) - at<float>(unit, kEntZ)};
    const float distance = host.vector2_length_00414c60(v);                    // 009A2A83
    if (kStayCloseRange > distance) {
        close_to_ship_set_state_009a29b0(host, task, ptr(task, kTaskStayClose));
    }
}

// 009A2B00 (009A2B00-009A2B81, RET 4); no Ghidra function starts here.
void close_to_ship_task_tick_009a2b00(Cc13Seg67aHost& host, void* task, float dt) {
    void* ctl = at<void*>(task, kTaskControl);
    if (ctl == nullptr) {
        return;
    }
    at<float>(task, kTaskMoveToAlt) = at<float>(ctl, kControlAltitude);
    const float altitude = at<float>(task, kTaskMoveToAlt);
    host.bot_state_moveto_set_ranges_009bde80(ptr(task, kTaskMoveTo), altitude, altitude,
                                              at<float>(task, kTaskBeginDist));  // 009A2B47
    host.close_to_ship_attack_mode_countdown_009a2810(task, dt);               // 009A2B56
    close_to_ship_select_state_009a29f0(host, task, dt);                       // 009A2B65
    host.state_vcall_0c_tick(at<void*>(task, kTaskState), dt);                 // 009A2B7D
}

// 009A2B90 (009A2B90-009A2BC4, RET 4); no Ghidra function starts here.
void* close_to_ship_target_record_009a2b90(Cc13Seg67aHost& host, void* task, void* out) {
    float scratch[3] = {0.0f, 0.0f, 0.0f};
    const float* p = host.approach_vcall_00_target_point(ptr(task, kTaskApproach), scratch);
    at<float>(out, 0x0) = p[0];
    at<std::uint8_t>(out, 0xC) = 1;
    at<float>(out, 0x4) = p[1];
    at<float>(out, 0x8) = p[2];
    return out;
}

// 009A2C00 (009A2C00-009A2C19); two tail jumps; no Ghidra function starts here.
float close_to_ship_wait_value_009a2c00(Cc13Seg67aHost& host, void* task) {
    if (at<void*>(task, kTaskState) == ptr(task, kTaskFollow)) {
        return host.bot_state_follow_wait_value_009be3e0(ptr(task, kTaskFollow));
    }
    return host.bot_task_base_wait_value_0099b720(task);
}

// 009A2C50 (009A2C50-009A2CE1, RET 4).
void close_to_ship_approach_save_009a2c50(Cc13Seg67aHost& host, void* approach, void* archive) {
    host.archive_vcall_0c_write(archive, name_value(kKeyShipName),
                                ref_value(4, at<void*>(approach, kApShip)));
    host.archive_vcall_0c_write(archive, name_value(kKeyDropOrdnance),
                                {3, at<std::uint8_t>(approach, kApDropOrdnance)});
    host.archive_vcall_0c_write(archive, name_value(kKeyBeginDist),
                                {2, float_bits(at<float>(approach, kApBeginDist))});
}

// 009A2CF0 (009A2CF0-009A2D52, RET 4); no Ghidra function starts here.
void close_to_ship_task_save_009a2cf0(Cc13Seg67aHost& host, void* task, void* archive) {
    host.bot_task_save_base_0099d060(task, archive);
    close_to_ship_approach_save_009a2c50(host, ptr(task, kTaskApproach), archive);
    host.state_registry_save_00411450(ptr(task, kTaskRegistry), ptr(task, kTaskRegistrySlot),
                                      archive);
    host.archive_vcall_0c_write(archive, name_value(kKeyDropStarted),
                                {2, float_bits(at<float>(task, kTaskDropStarted))});
}

// 009A2DF0 (009A2DF0-009A2E40).
void close_to_ship_task_destruct_009a2df0(Cc13Seg67aHost& host, void* task) {
    close_to_ship_approach_destruct_009a2440(host, task != nullptr ? ptr(task, kTaskApproach)
                                                                   : nullptr);
    host.bot_task_destruct_base_007b4030(task);
}

// 009A2E50 (009A2E50-009A2F16, RET 8).
void* close_to_ship_task_construct_from_stream_009a2e50(Cc13Seg67aHost& host, void* task,
                                                       void* bot, void* archive) {
    host.bot_task_construct_base_from_stream_0099c8b0(task, bot, archive, 0);
    close_to_ship_approach_block_construct_from_stream_009a2610(
        host, ptr(task, kTaskApproach), at<void*>(bot, 0x50), archive);
    set_vtable(task, 0x0, kCloseToShipTaskVtable);
    set_vtable(task, kTaskApproach, kCloseToShipApproachVtable);
    set_vtable(task, kTaskRegistry, kCloseToShipRegistryVtable);
    host.state_registry_load_00411170(ptr(task, kTaskRegistry), ptr(task, kTaskRegistrySlot),
                                      archive);                                 // 009A2EBF
    host.archive_vcall_10_read(archive, name_value(kKeyDropStarted),
                               ref_value(2, ptr(task, kTaskDropStarted)));      // 009A2EF9
    host.bot_approach_attach_task_009f9980(ptr(task, kTaskApproach), task);
    return task;
}

// 009A2F40 (009A2F40-009A2FA5).
void* close_to_ship_make_009a2f40(Cc13Seg67aHost& host, void* bot, void* ship) {
    void* task = host.operator_new_00bf681b(kCloseToShipTaskSize);
    if (task == nullptr) {
        return nullptr;
    }
    return close_to_ship_task_construct_009a2730(host, task, bot, ship);
}

// 009A2FB0 (009A2FB0-009A3014, RET 8).
void* close_to_ship_factory_create_009a2fb0(Cc13Seg67aHost& host, void* bot, void* archive) {
    void* task = host.operator_new_00bf681b(kCloseToShipTaskSize);
    if (task == nullptr) {
        return nullptr;
    }
    return close_to_ship_task_construct_from_stream_009a2e50(host, task, bot, archive);
}

} // namespace bsp

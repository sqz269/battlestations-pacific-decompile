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

// ---------------------------------------------------------------------------
// Second batch: manoeuvre tasks 009BB150-009BC5C0.
// ---------------------------------------------------------------------------
namespace {

constexpr std::size_t kManUnit = 0x2F4;
constexpr std::size_t kManDescriptor = 0x2F8;
constexpr std::size_t kManCondition = 0x3F8;

constexpr std::uint32_t kPullUpVtable = 0x00D20568u;
constexpr std::uint32_t kTightTurnVtable = 0x00D205E0u;
constexpr std::uint32_t kLeaveAloneVtable = 0x00D20658u;
constexpr std::uint32_t kBarrelRollVtable = 0x00D206D0u;
constexpr std::uint32_t kFlikFlakVtable = 0x00D20748u;
constexpr std::uint32_t kSplitSVtable = 0x00D207C0u;
constexpr std::uint32_t kTimedConditionVtable = 0x00D1EA00u;
constexpr std::uint32_t kManoeuvreKind = 0xF;            // PUSH 0Fh into 0099C6F0 / 0099C8B0

constexpr float kOne = 1.0f;                             // [00D7A24C]
constexpr float kHalf = 0.5f;                            // [00CE3800]
constexpr float kLeaveAloneGain = 0.10000000149011612f;  // [00D7A2F0]
constexpr std::size_t kTuningLeaveAlonePwr = 0x598;      // Pilot/General/LeaveAlonePwr

constexpr const char* kKeySafePitch = "safePitch";       // 00D2035C
constexpr const char* kKeyRefPitch = "refPitch";         // 00D20350
constexpr const char* kKeyStarted = "started";           // 00D20348
constexpr const char* kKeyLeft = "left";                 // 00D20368
constexpr const char* kKeyBarrelRollSpeed = "barrelRollSpeed";  // 00D20374
constexpr const char* kKeyTimer = "timer";               // 00D06BE4
constexpr const char* kKeyStage = "stage";               // 00D20474
constexpr const char* kKeySign = "sign";                 // 00D20830
constexpr const char* kKeySpd = "spd";                   // 00D09548

// The {0, name}/{type, &field} archive slot +10h read the stream constructors
// repeat for every field.
void read_field(Cc13Seg67aHost& host, void* archive, const char* key, std::int32_t type,
                void* field) {
    host.archive_vcall_10_read(archive, name_value(key), ref_value(type, field));
}

// The leavealone fields 009BB640 and 009BB710 both write after their base call.
void leavealone_fields(void* task) {
    set_vtable(task, 0x0, kLeaveAloneVtable);
    at<std::uint32_t>(task, 0x3FC) = 0;
    at<std::uint32_t>(task, 0x400) = 0;
    at<std::uint32_t>(task, 0x404) = 0;
    at<std::uint32_t>(task, 0x40C) = 0;
    at<std::uint32_t>(task, 0x410) = 0;
    at<float>(task, 0x408) = kOne;
    at<std::uint16_t>(task, 0x414) = 0;
}

} // namespace

const char* pullup_name_009bb150() noexcept { return "pullup"; }
std::int32_t manoeuvre_slot10_009bb160() noexcept { return 10; }
std::uint32_t tightturn_robots_row_009bb2c0() noexcept { return 0x00F8A4B8u; }
const char* tightturn_name_009bb2d0() noexcept { return "tightturn"; }
std::uint32_t leavealone_robots_row_009bb6a0() noexcept { return 0x00F8A4A8u; }
bool leavealone_slot34_009bb6b0() noexcept { return false; }
bool leavealone_slot30_009bb6d0() noexcept { return false; }
std::int32_t leavealone_slot10_009bb6e0() noexcept { return 3; }
const char* leavealone_name_009bb6f0() noexcept { return "leavealone"; }
std::uint32_t barrelroll_robots_row_009bb820() noexcept { return 0x00F8A488u; }
const char* barrelroll_name_009bb830() noexcept { return "barrelroll"; }
std::uint32_t flikflak_robots_row_009bb9d0() noexcept { return 0x00F8A4C8u; }
std::int32_t flikflak_slot10_009bb9e0() noexcept { return 10; }
const char* flikflak_name_009bb9f0() noexcept { return "flikflak"; }
std::uint32_t splits_robots_row_009bbbb0() noexcept { return 0x00F8A498u; }
bool splits_slot34_009bbbc0() noexcept { return true; }
std::int32_t splits_slot10_009bbbd0() noexcept { return 10; }
const char* splits_name_009bbbe0() noexcept { return "split s"; }

// 009BB180 (009BB180-009BB275, RET 8).
void* pullup_construct_from_stream_009bb180(Cc13Seg67aHost& host, void* task, void* bot,
                                            void* archive) {
    host.bot_task_conditioned_construct_from_stream_009ba830(task, bot, archive);  // 009BB1A8
    set_vtable(task, 0x0, kPullUpVtable);
    read_field(host, archive, kKeySafePitch, 2, ptr(task, 0x400));    // 009BB1F0
    read_field(host, archive, kKeyRefPitch, 2, ptr(task, 0x3FC));     // 009BB227
    read_field(host, archive, kKeyStarted, 3, ptr(task, 0x404));      // 009BB25E
    return task;
}

// 009BB280 (009BB280-009BB2B1, RET 0Ch). Same stores as the inlined tail of
// 009BC030 (hosted, see the doc).
void* tightturn_construct_009bb280(Cc13Seg67aHost& host, void* task, void* bot, void* condition,
                                   std::uint8_t left) {
    host.bot_task_pullup_construct_009bafc0(task, bot, condition);    // 009BB28F
    set_vtable(task, 0x0, kTightTurnVtable);
    at<std::uint8_t>(task, 0x408) = left;
    at<std::uint8_t>(task, 0x404) = 1;
    return task;
}

// 009BB2F0 (009BB2F0-009BB377, RET 8). +404h is not written here; 009BB180
// restores it from "started".
void* tightturn_construct_from_stream_009bb2f0(Cc13Seg67aHost& host, void* task, void* bot,
                                               void* archive) {
    pullup_construct_from_stream_009bb180(host, task, bot, archive);  // 009BB318
    set_vtable(task, 0x0, kTightTurnVtable);
    read_field(host, archive, kKeyLeft, 3, ptr(task, 0x408));         // 009BB360
    return task;
}

// 009BB640 (009BB640-009BB69C, RET 8).
void* leavealone_construct_009bb640(Cc13Seg67aHost& host, void* task, void* bot,
                                    void* condition) {
    host.bot_task_construct_base_0099c6f0(task, bot, kManoeuvreKind);  // 009BB64A
    at<void*>(task, kManCondition) = condition;
    leavealone_fields(task);
    return task;
}

bool leavealone_slot38_009bb6c0(void* task) noexcept {
    return static_cast<std::int16_t>(at<std::uint16_t>(task, 0x414)) > 10;
}

// 009BB710 (009BB710-009BB767, RET 8). The condition at +3F8h is the stream
// base's business (009BA830).
void* leavealone_construct_from_stream_009bb710(Cc13Seg67aHost& host, void* task, void* bot,
                                                void* archive) {
    host.bot_task_conditioned_construct_from_stream_009ba830(task, bot, archive);  // 009BB71F
    leavealone_fields(task);
    return task;
}

// 009BB770 (009BB770-009BB817, RET 8). sign = -1.0 when 0 > unit+C68h (COMISS/
// JBE: NaN gives +1.0); U(0.5, 1.0) from 00BD2F10 stream 1 at 009BB7ED; +404h = 0
// after the draw; +408h = float(U * sign), exact.
void* barrelroll_construct_009bb770(Cc13Seg67aHost& host, void* task, void* bot,
                                    void* condition) {
    host.bot_task_pullup_construct_009bafc0(task, bot, condition);    // 009BB799
    set_vtable(task, 0x0, kBarrelRollVtable);
    const float sign = 0.0f > at<float>(at<void*>(task, kManUnit), 0xC68) ? kMinusOne : kOne;
    const float draw = host.random_uniform_00bd2f10(1, kHalf, kOne);
    at<std::uint8_t>(task, 0x404) = 0;
    at<float>(task, 0x408) = draw * sign;
    return task;
}

// 009BB850 (009BB850-009BB90E, RET 8).
void* barrelroll_construct_from_stream_009bb850(Cc13Seg67aHost& host, void* task, void* bot,
                                                void* archive) {
    pullup_construct_from_stream_009bb180(host, task, bot, archive);  // 009BB878
    set_vtable(task, 0x0, kBarrelRollVtable);
    read_field(host, archive, kKeyBarrelRollSpeed, 2, ptr(task, 0x408));  // 009BB8C0
    read_field(host, archive, kKeyStarted, 3, ptr(task, 0x404));          // 009BB8F7
    return task;
}

// 009BBA10 (009BBA10-009BBACE, RET 8).
void* flikflak_construct_from_stream_009bba10(Cc13Seg67aHost& host, void* task, void* bot,
                                              void* archive) {
    host.bot_task_conditioned_construct_from_stream_009ba830(task, bot, archive);  // 009BBA38
    set_vtable(task, 0x0, kFlikFlakVtable);
    read_field(host, archive, kKeyLeft, 3, ptr(task, 0x400));         // 009BBA80
    read_field(host, archive, kKeyTimer, 2, ptr(task, 0x3FC));        // 009BBAB7
    return task;
}

// 009BBAD0 (009BBAD0-009BBBA3, RET 4). +3F8h = 0 and the vtable before the draw;
// 00BD2F10 stream 1 U(0, 1) at 009BBB1E; sign +3FCh = -1.0 when U > 0.5 (FCOMI/
// JBE: NaN gives +1.0); +400h = the larger of unit vtable[+38h] and 007C47F0
// (FCOMI/JBE: the unit speed unless the level speed is strictly larger).
void* splits_construct_009bbad0(Cc13Seg67aHost& host, void* task, void* bot) {
    host.bot_task_construct_base_0099c6f0(task, bot, kManoeuvreKind);  // 009BBAF6
    set_vtable(task, 0x0, kSplitSVtable);
    at<std::uint32_t>(task, kManCondition) = 0;
    const float draw = host.random_uniform_00bd2f10(1, 0.0f, kOne);
    at<float>(task, 0x3FC) = draw > kHalf ? kMinusOne : kOne;
    const float speed = host.unit_vcall_38_speed(at<void*>(task, kManUnit));        // 009BBB56
    const float level = host.plane_class_level_flight_speed_007c47f0(
        at<void*>(task, kManDescriptor));                                             // 009BBB62
    at<float>(task, 0x400) = level > speed ? level : speed;
    return task;
}

// 009BBC00 (009BBC00-009BBCF0, RET 8). "stage" is read as type 1 into the
// argument slot that held the archive pointer, so +3F8h keeps that pointer's
// bits when the read leaves the slot alone.
void* splits_construct_from_stream_009bbc00(Cc13Seg67aHost& host, void* task, void* bot,
                                            void* archive) {
    host.bot_task_construct_base_from_stream_0099c8b0(task, bot, archive, kManoeuvreKind);
    set_vtable(task, 0x0, kSplitSVtable);
    std::uint32_t stage = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(archive));
    read_field(host, archive, kKeyStage, 1, &stage);                  // 009BBC60
    at<std::uint32_t>(task, kManCondition) = stage;
    read_field(host, archive, kKeySign, 2, ptr(task, 0x3FC));         // 009BBCA1
    read_field(host, archive, kKeySpd, 2, ptr(task, 0x400));          // 009BBCD8
    return task;
}

// 009BBCF0 (009BBCF0-009BBD91, RET 4).
void splits_save_009bbcf0(Cc13Seg67aHost& host, void* task, void* archive) {
    host.bot_task_save_base_0099d060(task, archive);                  // 009BBCFA
    host.archive_vcall_0c_write(archive, name_value(kKeyStage),
                                {1, at<std::uint32_t>(task, kManCondition)});
    host.archive_vcall_0c_write(archive, name_value(kKeySign),
                                {2, float_bits(at<float>(task, 0x3FC))});
    host.archive_vcall_0c_write(archive, name_value(kKeySpd),
                                {2, float_bits(at<float>(task, 0x400))});
}

// 009BBE20 (009BBE20-009BBEE2, RET 8). Allocation order: task (40Ch), the timed
// condition (0Ch), then its distance sub-condition (2Ch). condition+8 =
// float([00F876A4] + seconds), one rounding.
void* barrelroll_make_until_far_009bbe20(Cc13Seg67aHost& host, void* bot, void* target,
                                         float distance, float seconds) {
    void* task = host.operator_new_00bf681b(0x40C);
    if (task == nullptr) {
        return nullptr;
    }
    void* condition = host.operator_new_00bf681b(0xC);
    if (condition != nullptr) {
        void* inner = host.operator_new_00bf681b(0x2C);
        void* built = nullptr;
        if (inner != nullptr) {
            built = host.distance_condition_construct_007b5e20(inner, at<void*>(bot, 0x50), target,
                                                               distance);   // 009BBE9C
        }
        set_vtable(condition, 0x0, kTimedConditionVtable);
        at<void*>(condition, 0x4) = built;
        at<float>(condition, 0x8) = host.global_float_00f876a4() + seconds;
    }
    return barrelroll_construct_009bb770(host, task, bot, condition);  // 009BBEC8
}

void* barrelroll_create_009bbef0(Cc13Seg67aHost& host, void* bot, void* condition) {
    void* task = host.operator_new_00bf681b(0x40C);
    return task != nullptr ? barrelroll_construct_009bb770(host, task, bot, condition) : nullptr;
}

void* splits_create_009bbf60(Cc13Seg67aHost& host, void* bot) {
    void* task = host.operator_new_00bf681b(0x404);
    return task != nullptr ? splits_construct_009bbad0(host, task, bot) : nullptr;
}

void* leavealone_create_009bbfc0(Cc13Seg67aHost& host, void* bot, void* condition) {
    void* task = host.operator_new_00bf681b(0x418);
    return task != nullptr ? leavealone_construct_009bb640(host, task, bot, condition) : nullptr;
}

void* barrelroll_factory_create_009bc110(Cc13Seg67aHost& host, void* bot, void* archive) {
    void* task = host.operator_new_00bf681b(0x40C);
    return task != nullptr ? barrelroll_construct_from_stream_009bb850(host, task, bot, archive)
                           : nullptr;
}

void* splits_factory_create_009bc180(Cc13Seg67aHost& host, void* bot, void* archive) {
    void* task = host.operator_new_00bf681b(0x404);
    return task != nullptr ? splits_construct_from_stream_009bbc00(host, task, bot, archive)
                           : nullptr;
}

void* leavealone_factory_create_009bc1f0(Cc13Seg67aHost& host, void* bot, void* archive) {
    void* task = host.operator_new_00bf681b(0x418);
    return task != nullptr ? leavealone_construct_from_stream_009bb710(host, task, bot, archive)
                           : nullptr;
}

void* tightturn_factory_create_009bc260(Cc13Seg67aHost& host, void* bot, void* archive) {
    void* task = host.operator_new_00bf681b(0x40C);
    return task != nullptr ? tightturn_construct_from_stream_009bb2f0(host, task, bot, archive)
                           : nullptr;
}

void* flikflak_factory_create_009bc2d0(Cc13Seg67aHost& host, void* bot, void* archive) {
    void* task = host.operator_new_00bf681b(0x404);
    return task != nullptr ? flikflak_construct_from_stream_009bba10(host, task, bot, archive)
                           : nullptr;
}

// 009BC3A0 (009BC3A0-009BC55C, RET 4).
//  1. task+3FCh..+410h = unit+9E4h..+9F8h (six dwords, raw copy).
//  2. (unit+72Ch)->vtable[+38h]() true: +344h = 0.1, +354h = 0, +408h = the
//     larger of +408h and tuning+598h (FCOMI/JBE: +408h only when strictly
//     larger, NaN gives the tuning value); then step 4.
//  3. false: only when unit+900h == 6: +344h = 0.1, +354h = 0, +408h = 0 when
//     0.5 > +408h else 1.0 (COMISS/JBE: NaN gives 1.0); then step 4.
//  4. +2C8h = 0, +2D0h = 1, +2C0h = 0, +2D4h = 1, +27Ch = +408h, byte +280h = 1,
//     +2ACh = 0, byte +2B0h = 1, +2DCh = 0.
//  5. condition->vtable[0]() true: 0099B690 (retire).
//  6. unit+9D8h != 0: word +414h += 1; else word +414h = 0 and
//     (unit+DF4h)+80h = 0.0.
void leavealone_tick_009bc3a0(Cc13Seg67aHost& host, void* task, float /*dt*/) {
    void* unit = at<void*>(task, kManUnit);
    for (std::size_t i = 0; i < 6; ++i) {
        at<std::uint32_t>(task, 0x3FC + 4 * i) = at<std::uint32_t>(unit, 0x9E4 + 4 * i);
    }
    bool apply = true;
    if (host.unit_embedded_72c_vcall_38(ptr(unit, 0x72C))) {            // 009BC400
        at<float>(task, 0x344) = kLeaveAloneGain;
        at<float>(task, 0x354) = 0.0f;
        void* tuning = host.game_tuning_singleton_0042e740();          // 009BC426
        const float power = at<float>(task, 0x408);
        const float floor = at<float>(tuning, kTuningLeaveAlonePwr);
        at<float>(task, 0x408) = power > floor ? power : floor;
    } else if (at<std::int32_t>(at<void*>(task, kManUnit), 0x900) == 6) {
        at<float>(task, 0x344) = kLeaveAloneGain;
        at<float>(task, 0x354) = 0.0f;
        at<float>(task, 0x408) = kHalf > at<float>(task, 0x408) ? 0.0f : kOne;
    } else {
        apply = false;
    }
    if (apply) {
        at<float>(task, 0x2C8) = 0.0f;
        at<std::uint32_t>(task, 0x2D0) = 1;
        at<float>(task, 0x2C0) = 0.0f;
        at<std::uint32_t>(task, 0x2D4) = 1;
        at<float>(task, 0x27C) = at<float>(task, 0x408);
        at<std::uint8_t>(task, 0x280) = 1;
        at<float>(task, 0x2AC) = 0.0f;
        at<std::uint8_t>(task, 0x2B0) = 1;
        at<std::uint32_t>(task, 0x2DC) = 0;
    }
    if (host.condition_vcall_00(at<void*>(task, kManCondition))) {      // 009BC50F
        host.bot_task_retire_0099b690(task);
    }
    void* unit_now = at<void*>(task, kManUnit);
    if (at<std::uint32_t>(unit_now, 0x9D8) != 0) {
        at<std::uint16_t>(task, 0x414) = static_cast<std::uint16_t>(at<std::uint16_t>(task, 0x414) + 1);
        return;
    }
    at<std::uint16_t>(task, 0x414) = 0;
    at<float>(at<void*>(unit_now, 0xDF4), 0x80) = 0.0f;
}

} // namespace bsp

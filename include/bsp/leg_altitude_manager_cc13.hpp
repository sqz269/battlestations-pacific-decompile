#pragma once

// The converging-leg altitude manager (packet cc13-legmgr1). One process-wide
// object, built lazily by 009BD400 BSP_BotPathLegManager_Get (operator_new(10h),
// constructor 009FAA40, vtable 00D21C80): +4h a heap array of 10h-byte records,
// +8h the record count, +0Ch the capacity. A record is {unit*, x, y, z}. All three
// members take the lock 009FB010 BSP_BotPathLegLock_Get returns (a second lazy
// singleton at [00F8A724], 8 bytes, constructor 009FA9F0, whose +4h is a
// BSP_CriticalSection_Create section; ++/-- the depth at section+18h around it).
//
// Users (rel32 + absolute-dword scans of the image, docs/CC13_LEG_ALTITUDE_MANAGER.md):
// the plane moveonpath task (009BD4D0 / 009BD120) and the plane moveto and
// attackmove tasks (refresh 009BEBA0, release slot +58h 009C32A0), all through
// 009FD050, which registers inside 3000 m of the goal and releases outside it.
//
// Every name is a hypothesis, not a recovered symbol. These are new C++ interfaces,
// not binary replacements; build-tested only, not wired into any host.
// Precision: the image keeps x87 intermediates in extended precision (at the
// control word's precision) and rounds at each float store; this code computes
// intermediates in double and rounds at the same stores.
// RNG: none. No function here draws from 00BD2F10.

#include <array>
#include <cstdint>
#include <vector>

namespace bsp {

// Unit handle as the record stores it. 0 is the null unit pointer.
using LegUnitId = std::uint32_t;

// The two unit reads the manager makes.
struct LegAltitudeUnitHost {
    virtual ~LegAltitudeUnitHost() = default;
    // byte unit+5Dh, the out-of-action byte (009FC06D, 009FC1FD).
    virtual bool out_of_action_5d(LegUnitId unit) const = 0;
    // float [unit+538h]+268h, the descriptor's TurnCircleRadius (009FC2DF, 009FC384).
    virtual float turn_circle_radius_268(LegUnitId unit) const = 0;
};

// The two game-tuning floats 009FC260 reads through 0042E740.
struct LegAltitudeTuning {
    float circle_alt_diff_378 = 30.0f;   // +378h Pilot/MoveTo/CircleAltDiff
    float ceiling_210 = 1500.0f;         // +210h Dynamics/Ceiling
};

struct LegAltitudeRecord {
    LegUnitId unit = 0;                  // record+0h
    std::array<float, 3> point{};        // record+4h..+0Ch
};

// The manager object. Records keep the image's order: removal copies the last
// record over the removed one and does not advance (swap-remove), which decides
// the order 009FC260 scans the bands in.
class LegAltitudeManager {
public:
    // 009FC000 (009FC000-009FC1B8), __thiscall(this, unit, const float* point),
    // RET 8. Drops null and out-of-action records; returns at once, without
    // updating the point, when `unit` already has a record; otherwise appends
    // {unit, point} unless `unit` is null.
    void register_009fc000(const LegAltitudeUnitHost& host, LegUnitId unit,
                           const std::array<float, 3>& point);

    // 009FC1C0 (009FC1C0-009FC25A), __thiscall(this, unit), RET 4. Drops the
    // records of `unit`, null records and out-of-action records.
    void remove_009fc1c0(const LegAltitudeUnitHost& host, LegUnitId unit);

    // 009FC260 (009FC260-009FC7B2), __thiscall(this, unit, const float* point,
    // float weight), RET 0Ch, float in ST0. Returns 0.0 for a null unit. Otherwise
    // y + weight * (alt - y), y = point[1], alt the separated altitude; when `unit`
    // has a record it becomes {point.x, alt, point.z}. Drops null records only.
    float altitude_009fc260(const LegAltitudeUnitHost& host, const LegAltitudeTuning& tuning,
                            LegUnitId unit, const std::array<float, 3>& point, float weight);

    const std::vector<LegAltitudeRecord>& records() const noexcept { return records_; }
    void clear() noexcept { records_.clear(); }

private:
    std::vector<LegAltitudeRecord> records_;
};

// The separation law of 009FC260 on its own (009FC3F4-009FC73C), for a caller that
// already has the other units' record heights `bands` (in scan order). Returns alt
// before the weight blend.
float leg_separated_altitude_009fc260(const std::vector<float>& bands, float y,
                                      const LegAltitudeTuning& tuning) noexcept;

}  // namespace bsp

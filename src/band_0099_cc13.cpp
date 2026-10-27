#include "bsp/band_0099_cc13.hpp"

#include <cmath>
#include <cstring>

namespace bsp::band0099 {

bool condition_simple_timed_test_009961b0(float deadline, float now) noexcept {
    // 009961B0 FLD [ECX+4]; FLD [00F876A4]; FCOMI ST1; JBE -> 0.
    return now > deadline;
}

bool condition_timed_test_009961d0(float deadline, float now, const std::function<bool()>* inner) {
    // 009961D9 FCOMI (now, deadline) / JA 009961F5 -> 1.
    if (now > deadline) {
        return true;
    }
    // 009961DF CMP [ECX+4],0 / JE -> 0; otherwise the inner condition's slot 0.
    return inner != nullptr && *inner && (*inner)();
}

float condition_time_left_009963e0(float deadline, float now) noexcept { return deadline - now; }

float condition_deadline_from_time_left_00996390(float now, float time_left) noexcept { return now + time_left; }

std::string_view describe_avoid_flags_009a1a90(unsigned avoid_flags) noexcept {
    // 009A1AC4..009A1B14: three assignments into the same string (ESI), so the
    // later ones replace the earlier ones.
    std::string_view text;
    if ((avoid_flags >> 2) & 1u) text = "AV_GUN ";  // 00D1F418
    if ((avoid_flags >> 1) & 1u) text = "AV_VEH ";  // 00D1F410
    if (avoid_flags & 1u) text = "AV_TERR";         // 00D1F408
    return text;
}

void dead_zone_0099ba90(float& value, float width) noexcept {
    const float v = value;
    // 0099BAA5 FCOMI (v, d) / JBE: v > d -> v - d (FSUBRP).
    if (v > width) {
        value = v - width;
        return;
    }
    // 0099BAB1..0099BAB7: -d compared with v; -d > v -> v + d (FADDP).
    if (-width > v) {
        value = v + width;
        return;
    }
    // 0099BAC1 FSTP ST1 / FSUBR ST0,ST0: v - v.
    value = v - v;
}

int close_to_ship_step_result_009a2bd0(bool in_follow_state, bool flag_43c) noexcept {
    return (!in_follow_state && flag_43c) ? 2 : 0;
}

int dive_bomb_step_result_009c8290(bool attacking_state, bool in_state_734) noexcept {
    if (!attacking_state) {
        return 1;
    }
    // ((state != +734h) - 1) & 2.
    return in_state_734 ? 2 : 0;
}

int torpedo_step_result_009d49e0(bool in_listed_state, bool flag_528) noexcept {
    if (!in_listed_state) {
        return 1;
    }
    // -(byte +528h != 0) & 2.
    return flag_528 ? 2 : 0;
}

// Documented deviation (cc13-D3): the image walks the registry list at [00F8A314]
// (next +0Ch) and returns the matching node; this walks a vector projection of the
// same names in the same order and returns the index (-1 for no node). The match
// test (length, then __stricmp) and first-match order are the image's.
int bot_task_factory_find_0099c690(const std::vector<BotTaskFactoryName>& factories,
                                   std::size_t length, const char* chars) {
    for (std::size_t i = 0; i < factories.size(); ++i) {
        const BotTaskFactoryName& f = factories[i];
        // 0099C6A5 CMP ECX,EAX / JNE: the lengths must match.
        if (f.length != length) continue;
        int diff;
        if (length == 0) {
            diff = 0;  // 0099C6AD NEG / SBB on the (equal, zero) length
        } else {
            diff = _stricmp(chars, f.chars);  // 0099C6C6 CALL __stricmp(query, factory name)
        }
        if (diff == 0) return static_cast<int>(i);
    }
    return -1;
}

void ship_ai_heading_set_vector_009da9d0(HeadingRecord& record, float heading) noexcept {
    record.heading = heading;  // 009DA9E3 MOVSS [ECX+8]
    // 009DA9D3..009DA9E8: FLD heading; FSUBR qword 1.5707963705062866; FSTP float.
    float a = static_cast<float>(1.5707963705062866 - static_cast<double>(heading));
    // 009DA9F0..009DA9FC: 0 > a -> FADD qword 6.2831854820251465; FSTP float.
    if (0.0f > a) {
        a = static_cast<float>(static_cast<double>(a) + 6.2831854820251465);
    }
    // 009DAA04..009DAA18: FCOS / FSIN on the float, each stored as a float. The
    // double library functions stand in for the x87 instructions.
    // Documented deviation (cc13-D3): FCOS/FSIN are not subject to precision control
    // and their extended result is rounded once by FSTP float; the double functions
    // round twice (to double, then float), which can differ only in the last float
    // bit. The two qword constants are float values (pi/2 and 2pi rounded to float),
    // so the double subtraction and addition above are exact unless |heading| is
    // below about 2^-28, and their single cast matches the FSUBR / FADD rounding at
    // the main thread's 24-bit control.
    record.direction[0] = static_cast<float>(std::cos(static_cast<double>(a)));
    record.direction[1] = 0.0f;
    record.direction[2] = static_cast<float>(std::sin(static_cast<double>(a)));
}

}  // namespace bsp::band0099

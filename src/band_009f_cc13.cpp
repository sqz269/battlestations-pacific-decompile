#include "bsp/band_009f_cc13.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/pc24_shipai_cc13.hpp"

#include <cstring>

namespace bsp::band009f {

bool ai_key4_less_009fec40(const AiKey4& lhs, const AiKey4& rhs) noexcept {
    // 009FEC4D CMP / JAE / JA (unsigned), 009FEC5F JL / JG (signed),
    // 009FEC6B JB / JA (unsigned), 009FEC74 JL (signed).
    if (lhs.a < rhs.a) return true;
    if (lhs.a > rhs.a) return false;
    if (lhs.b < rhs.b) return true;
    if (lhs.b > rhs.b) return false;
    if (lhs.c < rhs.c) return true;
    if (lhs.c > rhs.c) return false;
    return lhs.d < rhs.d;
}

namespace {

// The image subtracts in x87 and stores each difference as a float, then sums
// the squares in extended precision and stores the sum as a float. Products of
// two floats are exact in double, so the double sum rounded to float stands in
// for the extended sum (a double-rounding difference is possible in principle).
float horizontal_distance_squared(float dx, float dz) noexcept {
    if constexpr (kPc24ShipAiCc13Bound) {
        // 00A2C191 FMUL st0, 00A2C195 FMULP, 00A2C197 FADDP, 00A2C199 FSTP: each op
        // rounds to 24 bits on the main thread.
        using namespace cc13x87;
        return static_cast<float>(x87_add24(x87_mul24(dz, dz), x87_mul24(dx, dx)));
    } else {
        return static_cast<float>(static_cast<double>(dz) * dz + static_cast<double>(dx) * dx);
    }
}

}  // namespace

bool ai_point_outside_sphere_00a1a660(const AiSphereFilter& filter, const float point[3]) noexcept {
    // 00A1A667..00A1A67F: three float differences point - centre.
    const float dx = point[0] - filter.centre[0];
    const float dy = point[1] - filter.centre[1];
    const float dz = point[2] - filter.centre[2];
    // 00A1A683..00A1A69E: (dx*dx + dy*dy) + dz*dz, stored as a float.
    float d;
    if constexpr (kPc24ShipAiCc13Bound) {
        // 00A1A690 / 00A1A694 FMULP, 00A1A698 FADDP, 00A1A69A FMUL st0, 00A1A69C
        // FADDP, 00A1A69E FSTP: each op rounds to 24 bits on the main thread.
        using namespace cc13x87;
        d = static_cast<float>(x87_add24(x87_add24(x87_mul24(dx, dx), x87_mul24(dy, dy)),
                                         x87_mul24(dz, dz)));
    } else {
        d = static_cast<float>(static_cast<double>(dx) * dx + static_cast<double>(dy) * dy +
                               static_cast<double>(dz) * dz);
    }
    // 00A1A6A9 FCOMI st0 = d, st1 = r^2; JBE answers 0, so equal or unordered is inside.
    return d > filter.radius_squared;
}

bool ai_group_command_targets_point(AiCommandType command, AiCommandType family,
                                    const float destination[3], const float point[3]) noexcept {
    // 00A2C161: command->vtable[8](family), the IsType slot.
    if (!ai_command_is_type(command, family)) {
        return false;
    }
    // 00A2C173..00A2C199: dx = dest.x - p.x, dz = dest.z - p.z, both stored as floats.
    const float dx = destination[0] - point[0];
    const float dz = destination[2] - point[2];
    // 00A2C1A1 FLD1 / FCOMI / JBE: true only for 1.0 > d (NaN answers false).
    return 1.0f > horizontal_distance_squared(dx, dz);
}

std::optional<AiCommandType> ai_command_clone_type(AiCommandType type) noexcept {
    switch (type) {
    case AiCommandType::NonControl:      // 00A0FF80
    case AiCommandType::Selling:         // 00A0FFC0
    case AiCommandType::Idle:            // 00A10000
    case AiCommandType::DefendPosition:  // 00A10090
    case AiCommandType::Retreat:         // 00A100C0
    case AiCommandType::MoveTo:          // 00A10500
    case AiCommandType::RegroupingMove:  // 00A10580
    case AiCommandType::PatrolTo:        // 00A105C0
    case AiCommandType::MoveToAttack:    // 00A12C60
    case AiCommandType::CloseAttack:     // 00A12E30
        return type;
    case AiCommandType::CautiousMove:    // 00A10540 writes vtable 00D22ABC (MOVETO)
        return AiCommandType::MoveTo;
    case AiCommandType::CautiousAttack:  // 00A12D40 writes vtable 00D22BBC (MOVETOATTACK)
        return AiCommandType::MoveToAttack;
    case AiCommandType::Move:            // abstract: 00D22A94 slot +20h is __purecall
    case AiCommandType::Attack:          // abstract: 00D22B7C slot +20h is __purecall
    case AiCommandType::Defend:          // abstract: 00D22A10 slot +20h is __purecall
        break;
    }
    return std::nullopt;
}

namespace {

struct PlannerTypeRow {
    int get_type;       // slot +18h body
    int is_type_id;     // the constant slot +1Ch compares with
    const char* name;   // slot +2Ch literal
};

// Brain-slot order, as AiPlannerKind.
constexpr PlannerTypeRow kPlannerTypes[kAiPlannerKindCount] = {
    {0, 0, "Defend Coordinator"},       // 00A1F090 XOR EAX,EAX / 00A1F0A0 / 00A1F0B0
    {1, 1, "Attack Coordinator"},       // 00A1F1C0 / 00A1F1D0 / 00A1F1E0
    {2, 2, "Sell Coordinator"},         // 00A1F2F0 / 00A1F300 / 00A1F350
    {3, 1, "Capture Coordinator"},      // 00A1F480 MOV EAX,3 / 00A1F490 CMP [ESP+4],1 / 00A1F4A0
    {4, 4, "Duel Coordinator"},         // 00A1F5D0 / 00A1F5E0 / 00A1F5F0
    {5, 5, "Escort Coordinator"},       // 00A1F700 / 00A1F710 / 00A1F720
    {6, 6, "Siege Coordinator"},        // 00A1F830 / 00A1F840 / 00A1F850
    {7, 7, "Competitive Coordinator"},  // 00A1F960 / 00A1F970 / 00A1F980
};

const PlannerTypeRow& planner_row(AiPlannerKind kind) noexcept {
    return kPlannerTypes[static_cast<int>(kind)];
}

}  // namespace

int ai_planner_get_type(AiPlannerKind kind) noexcept { return planner_row(kind).get_type; }

bool ai_planner_is_type(AiPlannerKind kind, int id) noexcept { return id == planner_row(kind).is_type_id; }

std::string_view ai_planner_coordinator_name(AiPlannerKind kind) noexcept { return planner_row(kind).name; }

bool input_shift_down_00a2c0a0(const KeyDownQuery& key_down) {
    return key_down(0x2A) || key_down(0x36);  // DIK_LSHIFT, DIK_RSHIFT
}

bool input_ctrl_down_00a2c0d0(const KeyDownQuery& key_down) {
    return key_down(0x1D) || key_down(0x9D);  // DIK_LCONTROL, DIK_RCONTROL
}

bool input_alt_down_00a2c100(const KeyDownQuery& key_down) {
    return key_down(0x38) || key_down(0xB8);  // DIK_LMENU, DIK_RMENU
}

AiNearestGroup ai_nearest_group_00a1c960(const std::vector<AiGroupLeader>& leaders,
                                         const float point[3]) noexcept {
    AiNearestGroup best;
    best.distance_squared = -1.0f;  // 00A1C963 MOVSS [00D7A260]
    for (std::size_t i = 0; i < leaders.size(); ++i) {
        const AiGroupLeader& g = leaders[i];
        if (!g.has_members) continue;  // 00A1C99F +5644h test
        // 00A1C9D1..00A1C9FA: dx = p.x - leader.x, dz = p.z - leader.z (floats),
        // dx*dx + dz*dz stored as a float.
        const float dx = point[0] - g.position[0];
        const float dz = point[2] - g.position[2];
        float d;
        if constexpr (kPc24ShipAiCc13Bound) {
            // 00A1C9F2 FMUL st0, 00A1C9F6 FMULP, 00A1C9F8 FADDP, 00A1C9FA FSTP: each
            // op rounds to 24 bits on the main thread.
            using namespace cc13x87;
            d = static_cast<float>(x87_add24(x87_mul24(dx, dx), x87_mul24(dz, dz)));
        } else {
            d = static_cast<float>(static_cast<double>(dx) * dx + static_cast<double>(dz) * dz);
        }
        // 00A1CA06 FCOMI (best, d) / JA, then 00A1CA0F COMISS 0, best / JBE:
        // take when best > d or best < 0; unordered takes neither arm.
        if (best.distance_squared > d || best.distance_squared < 0.0f) {
            best.distance_squared = d;
            best.index = i;
        }
    }
    return best;
}

std::array<float, 3> ai_team_weighted_centroid_00a1ca60(const std::vector<AiWeightedLeader>& groups) noexcept {
    // 00A1CA83..00A1CA91: the three sums and the total weight start at 0.
    std::array<float, 3> sum{0.0f, 0.0f, 0.0f};
    float total = 0.0f;
    for (const AiWeightedLeader& g : groups) {
        if (!g.has_members) continue;  // the outer +5644h test skips the group entirely
        // 00A1CAF9..00A1CB34: each product is stored as a float, then each sum.
        const float wx = g.position[0] * g.weight;
        const float wy = g.position[1] * g.weight;
        const float wz = g.position[2] * g.weight;
        sum[0] = sum[0] + wx;
        sum[1] = sum[1] + wy;
        sum[2] = sum[2] + wz;
        total = g.weight + total;  // 00A1CB37 FADD [ESP+10h]
    }
    // 00A1CB51..00A1CB6C: three divisions by the total, no zero check.
    return {sum[0] / total, sum[1] / total, sum[2] / total};
}

namespace {

constexpr std::size_t kValueOffset = 0x0;
constexpr std::size_t kLabelOffset = 0x40;
constexpr std::size_t kLabelStride = 6;
constexpr std::size_t kAuxOffset = 0xA0;
constexpr std::size_t kChosenOffset = 0xA4;
constexpr std::size_t kKeyOffset = 0xA8;

template <typename T>
void store(AiGroupScoreList::Record& r, std::size_t offset, T value) noexcept {
    std::memcpy(r.data() + offset, &value, sizeof(T));
}

template <typename T>
T load(const AiGroupScoreList::Record& r, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, r.data() + offset, sizeof(T));
    return value;
}

}  // namespace

void AiGroupScoreList::remove(std::uint32_t key) noexcept {
    // 00A2B8F0: walk every record; once the key is found, each later record is
    // copied one slot down (2Bh dwords); the count drops by one when found.
    bool found = false;
    for (std::size_t i = 0; i < count_; ++i) {
        if (found) {
            records_[i - 1] = records_[i];
        } else if (load<std::uint32_t>(records_[i], kKeyOffset) == key) {
            found = true;
        }
    }
    if (found) {
        --count_;
    }
}

bool AiGroupScoreList::insert_sorted(std::uint32_t key, std::uint32_t aux,
                                     const std::array<Term, 16>& terms) noexcept {
    remove(key);  // 00A2B95F CALL 00A2B8F0
    if (count_ >= kCapacity) {
        return false;  // divergence: the image writes past the 128th record
    }
    const float score = terms[0].value;
    // 00A2B976 FLD [record] / FCOMI / JB: stop at the first record below the score.
    std::size_t at = 0;
    while (at < count_) {
        const float existing = load<float>(records_[at], kValueOffset);
        if (!(existing >= score)) break;
        ++at;
    }
    for (std::size_t i = count_; i > at; --i) {
        records_[i] = records_[i - 1];
    }
    Record& r = records_[at];
    // The store order of 00A2B950: values 0..14, key, chosen, aux, value 15, labels.
    for (int i = 0; i < 15; ++i) {
        store<float>(r, kValueOffset + 4 * static_cast<std::size_t>(i), terms[static_cast<std::size_t>(i)].value);
    }
    store<std::uint32_t>(r, kKeyOffset, key);
    r[kChosenOffset] = 0;
    store<std::uint32_t>(r, kAuxOffset, aux);
    store<float>(r, kValueOffset + 4 * 15, terms[15].value);
    for (std::size_t i = 0; i < terms.size(); ++i) {
        const char* label = terms[i].label != nullptr ? terms[i].label : "";
        std::size_t at_byte = kLabelOffset + kLabelStride * i;
        // strcpy including the terminator; clipped at the record end.
        for (const char* p = label; at_byte < kRecordSize; ++p) {
            r[at_byte++] = static_cast<std::uint8_t>(*p);
            if (*p == '\0') break;
        }
    }
    ++count_;
    return true;
}

void AiGroupScoreList::mark_chosen(std::uint32_t key) noexcept {
    // 00A2BCC0: every record whose +A8h matches gets +A4h = 1.
    for (std::size_t i = 0; i < count_; ++i) {
        if (load<std::uint32_t>(records_[i], kKeyOffset) == key) {
            records_[i][kChosenOffset] = 1;
        }
    }
}

float AiGroupScoreList::value(const Record& record, int index) noexcept {
    return load<float>(record, kValueOffset + 4 * static_cast<std::size_t>(index));
}

std::uint32_t AiGroupScoreList::key(const Record& record) noexcept { return load<std::uint32_t>(record, kKeyOffset); }

std::uint32_t AiGroupScoreList::aux(const Record& record) noexcept { return load<std::uint32_t>(record, kAuxOffset); }

bool AiGroupScoreList::chosen(const Record& record) noexcept { return record[kChosenOffset] != 0; }

SquadronTargetFilter squadron_auto_target_pick_filter_009f6c80(const SquadronFilterInputs& in) noexcept {
    if (in.leader_uncommitted_kamikaze) {
        return SquadronTargetFilter::At0C;  // 009F6C91 JNE 009F6D01
    }
    if (!in.bomber_command_class) {
        return SquadronTargetFilter::At14;  // 009F6CAC JNE 009F6D0D
    }
    // 009F6CB1..009F6CF0: 007EDAC0, then the four ordnance tests; all false -> +14h.
    const bool any = in.has_007edac0 || in.has_bomb_ordnance || in.has_torpedo_ordnance ||
                     in.has_depth_charge_ordnance || in.has_drop_kamikaze_ordnance;
    if (!any) {
        return SquadronTargetFilter::At14;
    }
    // 009F6CF2..009F6CFF: owner+3Ch clear -> +14h; +3Dh clear -> +1Ch; else +0Ch.
    if (!in.latch_3c) {
        return SquadronTargetFilter::At14;
    }
    return in.latch_3d ? SquadronTargetFilter::At0C : SquadronTargetFilter::At1C;
}

void ai_party_schedule_think_within_00a16450(std::array<float, 8>& next_think, int party,
                                             float now, float dt) {
    // 00A16450..00A1645A: now + dt stored as a float.
    const float t = now + dt;
    // 00A16469 FCOMI (next, t) / JBE: store only when next > t.
    float& slot = next_think.at(static_cast<std::size_t>(party));
    if (slot > t) {
        slot = t;
    }
}

int ai_first_active_party_slot_00a15950(const std::array<bool, 8>& slot_active) noexcept {
    for (int i = 0; i < 8; ++i) {
        if (slot_active[static_cast<std::size_t>(i)]) return i;
    }
    return -1;
}

float ai_group_recon_ratio_00a2c3d0(int party, const std::vector<int>& member_values) {
    // 00A2C3D4 CMP [EBP+5638h],2 / JL: parties 2 and up answer FLDZ.
    if (party >= 2) {
        return 0.0f;
    }
    int positive = 0;
    for (int v : member_values) {
        if (v >= 1) ++positive;  // 00A2C422 CMP EAX,1 / JL
    }
    // 00A2C43A FILD / 00A2C441 FIDIV [group+5644h] / FSTP float. Both are exact
    // integers; the double quotient rounded to float stands in for the extended one.
    const int count = static_cast<int>(member_values.size());
    return static_cast<float>(static_cast<double>(positive) / static_cast<double>(count));
}

}  // namespace bsp::band009f

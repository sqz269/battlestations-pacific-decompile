#include "bsp/band_0080_cc13.hpp"

namespace bsp::cc13_band0080 {

int hex_digit_value_008ef6a0(char c) noexcept
{
    const auto u = static_cast<unsigned char>(c);
    // 008EF6A0: three unsigned range tests, SUB 30h / 57h / 37h.
    if (static_cast<unsigned char>(u - 0x30) < 10) {
        return u - 0x30;
    }
    if (static_cast<unsigned char>(u - 0x61) < 6) {
        return u - 0x57;
    }
    if (static_cast<unsigned char>(u - 0x41) < 6) {
        return u - 0x37;
    }
    return 0;
}

std::uint8_t decode_hex_byte_token_008ef6d0(const char* token) noexcept
{
    if (token == nullptr || token[0] == '\0') {
        return 0;
    }
    // 008EF6E0..008EF710 inline the digit test for the first character.
    auto value = static_cast<std::uint8_t>(hex_digit_value_008ef6a0(token[0]));
    if (token[1] != '\0') {
        // 008EF712: MOV CL,[ECX+1]; CALL 008EF6A0; SHL BL,4; OR BL,AL.
        value = static_cast<std::uint8_t>((value << 4) | hex_digit_value_008ef6a0(token[1]));
    }
    return value;
}

std::string escape_scene_text_008ee5f0(const char* text, std::int32_t max_characters)
{
    std::string out;
    if (text == nullptr || max_characters == 0) {
        return out;
    }
    for (; *text != '\0' && max_characters != 0; ++text, --max_characters) {
        switch (*text) {
        case '\n': out += "\\n"; break;
        case '"': out += "\\q"; break;
        case '\\': out += "\\\\"; break;
        default: out += *text; break;
        }
    }
    return out;
}

bool segment_intersect_triangle_0085d020(const Vec3f& p0, const Vec3f& p1, const Vec3f& a,
                                         const Vec3f& b, const Vec3f& c, Vec3f& out) noexcept
{
    const float dx = p1.x - p0.x;
    const float dy = p1.y - p0.y;
    const float dz = p1.z - p0.z;
    const float e1x = b.x - a.x;
    const float e1y = b.y - a.y;
    const float e1z = b.z - a.z;
    const float e2x = c.x - a.x;
    const float e2y = c.y - a.y;
    const float e2z = c.z - a.z;
    // h = d x e2
    const float hx = static_cast<float>(double(dy) * e2z - double(dz) * e2y);
    const float hy = static_cast<float>(double(e2x) * dz - double(dx) * e2z);
    const float hz = static_cast<float>(double(e2y) * dx - double(dy) * e2x);
    // 0085D10B..0085D129: (hx*e1x + hy*e1y) + hz*e1z, stored to a float slot before the two FCOMI
    // tests. The summation order and the float stores of each dot product below were corrected
    // after the native fixture (docs/CC13_FIXTURE_PASS_1.md) showed last-place differences.
    const float det = static_cast<float>(double(hx) * e1x + double(hy) * e1y + double(hz) * e1z);
    if (double(det) > -0.0000999999974737875 && double(det) < 0.0000999999974737875) {
        return false;
    }
    const float inv = static_cast<float>(1.0 / det);
    const float sx = p0.x - a.x;
    const float sy = p0.y - a.y;
    const float sz = p0.z - a.z;
    // 0085D184..0085D1B0: (hy*sy + hx*sx) + hz*sz stored as a float, times inv, stored again.
    const float u_dot = static_cast<float>(double(hy) * sy + double(hx) * sx + double(hz) * sz);
    const float u = static_cast<float>(double(u_dot) * inv);
    if (u < 0.0f || u > 1.0f) {
        return false;
    }
    // q = s x e1
    const float qx = static_cast<float>(double(sy) * e1z - double(sz) * e1y);
    const float qy = static_cast<float>(double(sz) * e1x - double(e1z) * sx);
    const float qz = static_cast<float>(double(e1y) * sx - double(sy) * e1x);
    // 0085D215..0085D23F: (qy*dy + qx*dx) + qz*dz stored as a float, times inv, stored again.
    const float v_dot = static_cast<float>(double(qy) * dy + double(qx) * dx + double(qz) * dz);
    const float v = static_cast<float>(double(v_dot) * inv);
    if (v < 0.0f || 1.0 < double(v) + u) {
        return false;
    }
    // 0085D269..0085D28D: (qy*e2y + qx*e2x) + qz*e2z stored as a float, times inv, stored again.
    const float t_dot = static_cast<float>(double(qy) * e2y + double(qx) * e2x + double(qz) * e2z);
    const float t = static_cast<float>(double(t_dot) * inv);
    if (t < 0.0f || t > 1.0f) {
        return false;
    }
    out.x = p0.x + t * dx;
    out.y = p0.y + t * dy;
    out.z = p0.z + t * dz;
    return true;
}

std::int32_t effect_component_category(std::string_view component) noexcept
{
    for (const auto& row : kEffectComponentCategories) {
        if (row.component == component) {
            return row.category;
        }
    }
    return -1;
}

// ---------------------------------------------------------------------------
// cc13-gp80 gameplay leaves.
// ---------------------------------------------------------------------------

namespace {

// _stricmp in the "C" locale: ASCII folding to lower case.
int ascii_stricmp(const char* a, const char* b) noexcept
{
    for (;; ++a, ++b) {
        int ca = static_cast<unsigned char>(*a);
        int cb = static_cast<unsigned char>(*b);
        if (ca >= 'A' && ca <= 'Z') {
            ca += 'a' - 'A';
        }
        if (cb >= 'A' && cb <= 'Z') {
            cb += 'a' - 'A';
        }
        if (ca != cb || ca == 0) {
            return ca - cb;
        }
    }
}

} // namespace

std::int32_t party_from_name_00805990(const char* buffer) noexcept
{
    // 00E0B080: four pointers into .rdata, none null and none empty.
    static constexpr const char* kPartyNames[4] = {"OWN", "ENEMY", "NEUTRAL", "UNKNOWN"};
    for (std::int32_t i = 0; i < 4; ++i) {
        const char* entry = kPartyNames[i];
        if (buffer == nullptr) {
            // 008059BD..008059CF: a null buffer matches only a null or empty entry.
            if (entry[0] == '\0') {
                return i;
            }
        } else if (ascii_stricmp(buffer, entry) == 0) {
            return i;
        }
    }
    return 4;  // 00805A26: CMP EDI,4; JL loops; past the fourth miss the result is 4.
}

void turn_path_wrap_angle_0080b190(float& angle) noexcept
{
    constexpr double kTwoPi = 6.2831854820251465;  // double 00CE3828
    // 0080B198: COMISS 0.0, angle; JBE takes the second branch for angle >= 0 and NaN.
    if (0.0f > angle) {
        do {
            angle = static_cast<float>(static_cast<double>(angle) + kTwoPi);  // 0080B1AC FADD, FSTP
        } while (0.0 > static_cast<double>(angle));
        return;
    }
    // 0080B1CD: FCOMPI then JBE; NaN and angle <= 2pi leave it alone.
    while (static_cast<double>(angle) > kTwoPi) {
        angle = static_cast<float>(static_cast<double>(angle) - kTwoPi);  // 0080B1D3 FSUB, FSTP
    }
}

FirepowerRatios refresh_firepower_ratios_008127e0(const FirepowerInputs& in) noexcept
{
    FirepowerRatios out;
    for (int i = 0; i < 4; ++i) {
        // 008127EC: UCOMISS total, 0.0; LAHF; TEST AH,44h; JNP takes FLDZ only for an
        // ordered equal, so a NaN total still divides.
        if (in.total[i] == 0.0f) {
            out.ratio[i] = 0.0f;
        } else {
            out.ratio[i] = static_cast<float>(static_cast<double>(in.live[i]) /
                                              static_cast<double>(in.total[i]));  // FDIV, FSTP
        }
    }
    // 0081288C..008128C9: (unit+9E5h == 0 && unit+9D8h != 0.0) as an integer, FILD,
    // FMUL by unit+9DCh, FSTP to +1014h.
    const int engine_available = (!in.engine_jammed && in.engine != 0.0f) ? 1 : 0;
    out.engine_factor = static_cast<float>(static_cast<double>(in.steering) * engine_available);
    return out;
}

StationaryDamageResult stationary_check_destroyed_0084eb80(float protection, float max_damage,
                                                           float& damage) noexcept
{
    // 0084EB89: COMISS protection, 0.0; JBE to the destroy arm for protection <= 0 and NaN.
    if (protection > 0.0f) {
        // 0084EB97..0084EBAA: FLD1; FSUBRP; FSTP float; FMUL max_damage; FSTP float.
        const auto remaining = static_cast<float>(1.0 - static_cast<double>(protection));
        const auto cap = static_cast<float>(static_cast<double>(remaining) * max_damage);
        // 0084EBB6: FCOMPI damage, cap; JB skips for damage < cap and NaN.
        if (damage >= cap) {
            damage = cap;
            return StationaryDamageResult::Clamped;
        }
        return StationaryDamageResult::Unchanged;
    }
    if (damage >= max_damage) {
        // 0084EBEF..0084EBFE: damage = max, +1A4h = 1, vtable +70h(1).
        damage = max_damage;
        return StationaryDamageResult::Destroyed;
    }
    return StationaryDamageResult::Unchanged;
}

bool torpedo_proof_small_craft_00828ec0(bool is_kind_0e, bool is_kind_0c, bool byte_808,
                                        float value_500) noexcept
{
    // 00828ECA / 00828ED9: vtable +18h with 0Eh, then 0Ch only when 0Eh failed.
    if (!is_kind_0e && !(is_kind_0c && !byte_808)) {
        return false;
    }
    // 00828EF5: COMISS +500h, [00D099AC]; JB fails for smaller and for NaN.
    return value_500 >= kTorpedoProofSmallCraftThreshold;
}

void shipyard_consume_stock_008457e0(ShipyardStockRecord* records, std::size_t n,
                                     std::int32_t class_id, std::int32_t count) noexcept
{
    for (std::size_t i = 0; i < n; ++i) {
        if (records[i].class_id != class_id) {  // 00845828: CMP [EAX+70h],EBP
            continue;
        }
        // 00845843: CMP EAX,ECX; JG subtracts, otherwise the count becomes 0.
        if (records[i].count > count) {
            records[i].count -= count;
        } else {
            records[i].count = 0;
        }
        return;
    }
}

std::int32_t shipyard_count_available_00844610(const ShipyardStockRecord* records, std::size_t n_records,
                                               const ShipyardQueueSlot* slots, std::size_t n_slots,
                                               std::int32_t class_id) noexcept
{
    std::int32_t total = 0;
    for (std::size_t i = 0; i < n_records; ++i) {
        if (records[i].class_id == class_id) {
            total += records[i].count;
        }
    }
    for (std::size_t i = 0; i < n_slots; ++i) {
        if (slots[i].has_class && slots[i].class_id == class_id && slots[i].state == 1) {
            ++total;
        }
    }
    return total;
}

std::int32_t shipyard_count_idle_slots_008434d0(const ShipyardQueueSlot* slots, std::size_t n) noexcept
{
    std::int32_t idle = 0;
    for (std::size_t i = 0; i < n; ++i) {
        if (slots[i].state == 0) {
            ++idle;
        }
    }
    return idle;
}

std::int32_t support_site_squad_size_0084cf80(const SupportSiteListEntry* list_328, std::size_t n_328,
                                              const SupportSiteListEntry* list_31c, std::size_t n_31c,
                                              std::int32_t class_id) noexcept
{
    for (std::size_t i = 0; i < n_328; ++i) {
        if (list_328[i].class_id == class_id) {  // 0084CFC2: CMP ECX,[EDX+70h]
            return list_328[i].squad_size;       // 0084CFDF: MOV EAX,[EDI+10h]
        }
    }
    for (std::size_t i = 0; i < n_31c; ++i) {
        if (list_31c[i].class_id == class_id) {
            return list_31c[i].squad_size;
        }
    }
    return 3;
}

} // namespace bsp::cc13_band0080

// Segments 69/70 reference models (packet cc13-gpr69). See the header and
// docs/CC13_GPR_69.md. Every comparison and rounding below was read from the
// disk bytes; the instruction addresses are cited where they decide behaviour.

#include "bsp/gameplay_seg69_70_cc13.hpp"
#include "bsp/band_009f_cc13.hpp"

#include <cmath>

namespace bsp {
namespace gameplay_seg69_70_cc13 {

// ---------------------------------------------------------------------------
// 00A046C0
// ---------------------------------------------------------------------------

float ai_powerup_multiplier_term_00a046c0(const PowerupClassView& cls, int index,
                                          const PowerupUnitInputs& unit) noexcept {
    if (index < 0 || index >= kPowerupMultiplierCount) return 0.0f;
    const float m = cls.multipliers_08c[static_cast<std::size_t>(index)];
    // 00A046F1 UCOMISS [00D7A24C] (1.0f); LAHF/TEST AH,44h/JNP skips equal only.
    if (m == 1.0f) return 0.0f;
    // 00A0470F FSUB / 00A04715 FSUBR against the FLD1: 1 - m for scope 2.
    float term = (cls.scope_084 == 2) ? (1.0f - m) : (m - 1.0f);
    // Jump table 00A04820, indexed by index - 1 (index 0 and 14, 15 fall to
    // 00A047C9, the plain add).
    switch (index) {
    case 1:   // 00A0472D FADD ST0,ST0
        term = term + term;
        break;
    case 2:   // 00A0473C: zero when health > 0.9999 [00D04680] (FCOMI/JBE keeps NaN)
        if (unit.health > 0.9999000000025262) term = 0.0f;
        break;
    case 3:   // 00A04753: FLD1, FSUBRP ST1 (DE E1): (1 - health) * term
        term = (1.0f - unit.health) * term;
        break;
    case 4:
    case 5:   // 00A04768: 00828EC0 class test zeroes the term
        if (unit.torpedo_proof_small) term = 0.0f;
        break;
    case 7:   // 00A04779: -0.0f [00D7A208] - term (SUBSS)
        term = -0.0f - term;
        break;
    case 10:  // 00A04789: ships only, and class+804h > 0.0f (COMISS/JA; NaN zeroes)
        if (!unit.is_kind_06 || !(unit.class_804 > 0.0f)) term = 0.0f;
        break;
    case 11:  // 00A047B1: kind 1Ch only
        if (!unit.is_kind_1c) term = 0.0f;
        break;
    default:
        break;
    }
    return term;
}

float ai_powerup_score_for_unit_00a046c0(const PowerupClassView& cls,
                                         const PowerupUnitInputs& unit) noexcept {
    float sum = 0.0f;
    for (int i = 0; i < kPowerupMultiplierCount; ++i) {
        if (cls.multipliers_08c[static_cast<std::size_t>(i)] == 1.0f) continue;
        sum = sum + ai_powerup_multiplier_term_00a046c0(cls, i, unit);
    }
    // 00A047EE..00A04812: float(00A04240(unit) * double(class weight)) * sum.
    const float factor = static_cast<float>(static_cast<double>(unit.hint_multiplier) *
                                            static_cast<double>(unit.class_weight));
    return factor * sum;
}

// ---------------------------------------------------------------------------
// 00A04860 / 00A04910
// ---------------------------------------------------------------------------

int ai_unit_party_00a04893(int side_180, int team_054, std::uint32_t game_1fe4,
                           int local_player_team) noexcept {
    const std::uint32_t side = static_cast<std::uint32_t>(side_180);
    if (side == 8u) return -1;                      // 00A04899
    if (side <= 7u) return side_180;                // 00A048A1 JBE (unsigned)
    if (game_1fe4 != 0u) return -1;                 // 00A048A3
    return (team_054 != local_player_team) ? 4 : 0; // 00A048B9 NEG/SBB/AND 4
}

float ai_powerup_score_for_side_00a04860(int party, const PowerupClassView& cls,
                                         const std::vector<PowerupWorldUnit>& world,
                                         std::uint32_t game_1fe4, int local_player_team,
                                         PowerupScoreHost& host) {
    float sum = 0.0f;
    for (const PowerupWorldUnit& u : world) {
        if (ai_unit_party_00a04893(u.side_180, u.team_054, game_1fe4, local_player_team) != party) {
            continue;
        }
        if (!host.scope_accepts(u.unit, cls)) continue;
        sum = host.unit_score(u.unit, cls) + sum;
    }
    return sum * 0.25f;   // 00A04900 FMUL [00D7A348]
}

float ai_powerup_score_for_team_00a04910(int team, const PowerupClassView& cls,
                                         const std::vector<PowerupWorldUnit>& world,
                                         PowerupScoreHost& host) {
    float sum = 0.0f;
    for (const PowerupWorldUnit& u : world) {
        if (u.team_054 != team) continue;           // 00A04937
        if (!host.scope_accepts(u.unit, cls)) continue;
        sum = host.unit_score(u.unit, cls) + sum;
    }
    return sum * 0.125f;  // 00A04967 FMUL [00D04380]
}

// ---------------------------------------------------------------------------
// 00A0F680
// ---------------------------------------------------------------------------

AiValueRecord ai_candidate_attack_record_00a0f680(const CandidateAttackBlock& block,
                                                          CandidateAttackHost& host) {
    AiValueRecord r;
    r.weight_014 = host.class_weight(block.class_object_020);   // 00A0F6E2, 00A0F6E6
    r.object_000 = block.class_object_020;
    r.dword_010 = block.dword_01c;
    // 00A0F71C..00A0F737: hi (+54h) is read first, then lo (+50h); ECX = 1.
    const float hi = host.tuning_spread_hi_054();
    const float lo = host.tuning_spread_lo_050();
    r.spread_018 = host.uniform(1, lo, hi);
    return r;
}

float ai_candidate_attack_value_00a0f680(std::uint32_t group_5638,
                                         const CandidateAttackBlock& block_00c,
                                         const CandidateAttackBlock& block_044,
                                         const void* group, CandidateAttackHost& host) {
    const CandidateAttackBlock& block = (group_5638 == 0u) ? block_044 : block_00c;
    const AiValueRecord r = ai_candidate_attack_record_00a0f680(block, host);
    return host.compose_against_group(r, group);   // 00A0F775, stored as float
}

// ---------------------------------------------------------------------------
// 00A179E0
// ---------------------------------------------------------------------------

std::vector<EngagementPair> ai_engagement_pairs_00a179e0(
    int party, const std::vector<EngagementGroup>& own_team,
    const std::vector<EngagementGroup>& enemy_team) {
    std::vector<EngagementPair> pairs;
    for (const EngagementGroup& own : own_team) {
        for (const EngagementGroup& enemy : enemy_team) {
            if (own.population_5644 == 0u || enemy.population_5644 == 0u) continue;
            if (own.party_5634 != party) continue;                     // 00A17B73 SETE
            const float dx = own.leader_x - enemy.leader_x;            // 00A17BE6
            const float dz = own.leader_z - enemy.leader_z;
            const float d2 = dx * dx + dz * dz;                         // 00A17C1B FSTP
            if (d2 > 9000000.0f) continue;                              // [00D22CB8], JA: NaN passes
            const float w_own = own.strength;
            if (15.0 > static_cast<double>(w_own)) continue;            // [00CF3F20], JA
            const float w_enemy = enemy.strength;
            // 00A17C7A..00A17C90 (DC C9, D8 CA): skip when own*0.5 > enemy or
            // enemy*0.5 > own ([00D7A280]); NaN passes both.
            if (w_own * 0.5f > w_enemy) continue;
            if (0.5f * w_enemy > w_own) continue;
            // 00415550(&250000.0f [00D22CB4], &d2): the first unless d2 >= it or NaN.
            const float floor = 250000.0f;
            const float m = (floor > d2) ? floor : d2;
            EngagementPair p;
            p.own = &own;
            p.enemy = &enemy;
            p.dist2_00c = d2;
            p.own_strength_010 = w_own;
            p.enemy_strength_014 = w_enemy;
            p.weight_018 = w_enemy / m;                                 // 00A17CB7 FDIVR
            pairs.push_back(p);
        }
    }
    return pairs;
}

std::optional<PowerupUseDecision> ai_powerup_choose_00a179e0(
    int party, int team, const std::vector<PowerupReadyEntry>& entries,
    const std::vector<EngagementPair>& pairs, const std::vector<PowerupWorldUnit>& world,
    std::uint32_t game_1fe4, int local_player_team, PowerupScoreHost& host) {
    float best = 0.0f;
    std::optional<PowerupUseDecision> chosen;
    for (const PowerupReadyEntry& e : entries) {
        const PowerupClassView& cls = e.cls;
        float score = 0.0f;
        const void* target = nullptr;
        if (cls.kind_008 == 1) {
            // Air support: one flight scored against each pair's enemy group.
            for (const EngagementPair& p : pairs) {
                if (cls.scope_084 == 1) {
                    // 00A17E0A: only the own group's first member is tested and targeted.
                    if (p.own->members.empty()) continue;
                    const void* first = p.own->members.front();
                    if (!host.scope_accepts(first, cls)) continue;
                    const float s = host.candidate_attack_value(p.enemy->id, cls) * p.weight_018;
                    if (s > score) {                                    // 00A17E4D JBE
                        score = s;
                        target = first;
                    }
                } else if (cls.scope_084 == 2) {
                    // 00A17E9F: the enemy group's first accepted member.
                    const void* accepted = nullptr;
                    for (const void* m : p.enemy->members) {
                        if (host.scope_accepts(m, cls)) {
                            accepted = m;
                            break;
                        }
                    }
                    if (accepted == nullptr) continue;
                    const float s = host.candidate_attack_value(p.enemy->id, cls) * p.weight_018;
                    if (s > score) {                                    // 00A17F11 JBE
                        score = s;
                        target = accepted;
                    }
                }
            }
        } else if (cls.kind_008 == 2) {
            if (cls.scope_084 == 1 || cls.scope_084 == 2) {
                // 00A17FC3: every member of the pair's own (1) or enemy (2) group.
                for (const EngagementPair& p : pairs) {
                    const EngagementGroup* g = (cls.scope_084 == 1) ? p.own : p.enemy;
                    for (const void* m : g->members) {
                        if (!host.scope_accepts(m, cls)) continue;
                        const float s = host.unit_score(m, cls) * p.weight_018;
                        if (s > score) {                                // 00A18076 JBE
                            score = s;
                            target = m;
                        }
                    }
                }
            } else if (cls.scope_084 == 4) {
                score = ai_powerup_score_for_side_00a04860(party, cls, world, game_1fe4,
                                                           local_player_team, host);
            } else if (cls.scope_084 == 5) {
                score = ai_powerup_score_for_team_00a04910(team, cls, world, host);
            }
        }
        if (score > best) {                                             // 00A17F79 JBE
            best = score;
            chosen = PowerupUseDecision{e.entry, target, score};
        }
    }
    return chosen;
}

std::optional<PowerupUseDecision> ai_party_brain_engagement_pass_00a179e0(
    int party, int team, const std::vector<PowerupReadyEntry>& entries,
    const std::vector<EngagementGroup>& own_team, const std::vector<EngagementGroup>& enemy_team,
    const std::vector<PowerupWorldUnit>& world, std::uint32_t game_1fe4, int local_player_team,
    PowerupScoreHost& host) {
    if (entries.empty()) return std::nullopt;                           // 00A17A3A, 00A17A49
    const std::vector<EngagementPair> pairs = ai_engagement_pairs_00a179e0(party, own_team,
                                                                           enemy_team);
    if (pairs.empty()) return std::nullopt;                             // 00A17D41
    return ai_powerup_choose_00a179e0(party, team, entries, pairs, world, game_1fe4,
                                      local_player_team, host);
}

// ---------------------------------------------------------------------------
// 00A163D0
// ---------------------------------------------------------------------------

std::array<PartyThinkSlotInit, 8> ai_init_party_think_schedule_00a163d0(float now,
                                                                        ThinkScheduleRng& rng) {
    std::array<PartyThinkSlotInit, 8> slots{};
    for (PartyThinkSlotInit& s : slots) {
        // 00A163F2..00A1640A: U(lo = [00CE3854], hi = [00CE3850]) on ECX = 1, + now.
        const float draw = rng.uniform(1, 3.0f, 5.0f);
        s.ai_enabled_000 = false;
        s.attack_ratio_004 = 0.5f;
        s.aggressive_ratio_008 = 0.5f;
        s.next_think = draw + now;
        s.brain = nullptr;
    }
    return slots;
}

// ---------------------------------------------------------------------------
// 00A02180 / 00A07C10
// ---------------------------------------------------------------------------

float ai_class_speed_00a02180(const SpeedClassView& c) noexcept {
    if (c.is_kind_06) return c.speed_500;   // 00A021CF
    if (c.is_kind_0f) return c.speed_188;   // 00A021E8
    return 0.0f;
}

float ai_unit_list_min_speed_00a02180(const std::vector<SpeedClassView>& list) noexcept {
    float best = 10000000000.0f;   // [00CE4970]
    for (const SpeedClassView& c : list) {
        const float v = ai_class_speed_00a02180(c);
        if (v > best) continue;    // 00A02203 FCOMI/JA keeps the seed; NaN replaces
        best = v;
    }
    return best;
}

// ---------------------------------------------------------------------------
// 00A03900
// ---------------------------------------------------------------------------

float ai_class_pair_value_00a03900(int mode_index, const std::vector<ClassPairRow>& rows_of_mode,
                                   std::uint32_t class_id_a, std::uint32_t class_id_b) noexcept {
    if (mode_index > 2) return 0.0f;                   // 00A0390E JLE (signed)
    for (const ClassPairRow& r : rows_of_mode) {       // 00A039EE, 00A039F6
        if (r.class_id_a == class_id_a && r.class_id_b == class_id_b) return r.value;
    }
    return 0.0f;
}

// ---------------------------------------------------------------------------
// 00A0D1D0
// ---------------------------------------------------------------------------

namespace {

AiValueRecord spawn_value_record(const SpawnClassView& c, const MatchFinalWeightInputs& in,
                                 SpawnCompositionHost& host) {
    AiValueRecord r;
    r.weight_014 = host.class_weight(c.class_id);     // 00A0D5D2/00A0D5D6, 00A0DA51
    r.object_000 = c.id;
    const bool placed = 0.0f > in.arg7;               // 00A0D1FB COMISS/JA
    r.x_004 = placed ? in.pos_x : 0.0f;
    r.z_008 = placed ? in.pos_z : 0.0f;
    r.byte_00c = placed;
    r.dword_010 = c.dword_134;
    r.dword_01c = 0;
    const float hi = host.tuning_054();
    const float lo = host.tuning_050();
    r.spread_018 = host.uniform(1, lo, hi);           // 00A0D604 / 00A0DAB8 / 00A0DB21
    return r;
}

}  // namespace

MatchFinalWeightResult ai_match_final_weight_00a0d1d0(const std::vector<SpawnClassView>& classes,
                                                      const MatchFinalWeightInputs& in,
                                                      SpawnCompositionHost& host) {
    MatchFinalWeightResult out;
    std::vector<const SpawnClassView*> picked;   // the classes behind out.composition
    const float factor = in.byte5 ? in.factor_e0e35c : 1.0f;
    float spent = 0.0f;                                         // [ESP+3Ch]
    bool restrict_to_budget = false;                            // [ESP+33h]
    const SpawnClassView* first = nullptr;                      // [ESP+8Ch]
    float threshold = in.mode_le_3 ? 0.001f : -100000000.0f;    // [00D7A23C] / [00D22800]
    if (!(in.target > 0.0f)) return out;                        // 00A0D408 JBE
    do {
        float round_best = threshold;                           // [ESP+80h]
        const SpawnClassView* best = nullptr;                   // [ESP+A4h]
        float best_total = 0.0f;                                // [ESP+A8h]
        if (classes.empty()) break;                             // 00A0D476
        for (const SpawnClassView& c : classes) {
            const float cost_f = static_cast<float>(c.cost_124);
            if (restrict_to_budget && in.mode_le_3 && cost_f + spent > in.budget) continue;
            if (first != nullptr && !in.allow_mixed) {          // 00A0D4E3..00A0D52B
                if (first->is_plane && !c.is_plane) continue;
                if (!first->is_plane && c.is_plane) continue;
            }
            const float cost = in.mode_le_3 ? cost_f : 100.0f;  // [00CE3D08]
            out.composition.push_back(spawn_value_record(c, in, host));   // 00A0D618
            picked.push_back(&c);
            const float v1 = host.compose_against_threats(out.composition, in.arg7, in.byte5,
                                                          factor);       // 00A0D654
            float pair_sum = 0.0f;                              // [ESP+5Ch]
            const std::size_t n = out.composition.size();
            if (n > 1) {
                for (std::size_t i = 0; i + 1 < n; ++i) {       // 00A0D6E3
                    pair_sum = host.class_pair_value(*picked[i], *picked[n - 1]) + pair_sum;
                }
            }
            const float total = pair_sum + v1;                  // 00A0D706
            if (total > 0.0f) {                                 // 00A0D710 FCOMI/JBE
                const float v2 = host.compose_threats_against(out.composition);   // 00A0D791
                const float k1 = host.tuning_234();
                const float k2 = host.tuning_238();
                // 00A0D7C5..00A0D7E3 (DE CA, DE E1): k1*total - k2*v2.
                const float combined = k1 * total - k2 * v2;
                float final_weight = combined;
                // 00A0D845: [00D7A3A0] = 0.1 (double) < spent, FCOMI/JBE.
                if (in.mode_le_3 && static_cast<double>(spent) > 0.10000000149011612) {
                    float k = host.tuning_230();                // 00A0D86E..00A0D896
                    if (0.0f > k) {
                        k = 0.0f;
                    } else if (k > 1.0f) {
                        k = 1.0f;
                    }
                    const float r = spent / (cost + spent);     // D8 C1, DE F9
                    // 00A0D8B5..00A0D8D4: c*(1-k) + (k*c)*r.
                    final_weight = combined * (1.0f - k) + (k * combined) * r;
                }
                if (final_weight > round_best) {                // 00A0D95A JBE
                    round_best = final_weight;
                    best = &c;
                    best_total = total;
                }
            }
            out.composition.pop_back();                         // 00A0D9B7: end -= 20h
            picked.pop_back();
        }
        if (best == nullptr) break;                             // 00A0D9E9
        const float best_cost_f = static_cast<float>(best->cost_124);
        if (in.mode_le_3 && !restrict_to_budget && best_cost_f + spent > in.budget) {
            // 00A0DA14: give up once 0.75 [00CEC9D8] of the target is bought.
            // FMUL qword rounds to 24 bits at the gameplay precision.
            if (spent > in.target * 0.75f) break;
            restrict_to_budget = true;
        } else {
            spent = best_cost_f + spent;                        // 00A0DA42 (the int cost,
                                                                // also in modes >= 4)
            out.composition.push_back(spawn_value_record(*best, in, host));   // 00A0DB33
            picked.push_back(best);
            threshold = best_total;                             // [ESP+94h]
            out.value = best_total;                             // [ESP+90h]
        }
        if (first == nullptr) first = best;                     // 00A0DB9F, even unaccepted
    } while (in.target > spent);                                // 00A0DBAF JA
    return out;
}

// ---------------------------------------------------------------------------
// 00A236F0 / 00A23980 / 00A25A30
// ---------------------------------------------------------------------------

std::vector<const SpawnSiteRef*> ai_sort_spawn_sites_00a236f0(
    const std::vector<std::vector<SpawnSiteRef>>& lists, float x, float z) {
    struct Keyed {
        const SpawnSiteRef* site;
        float d2;
    };
    std::vector<Keyed> v;
    for (const std::vector<SpawnSiteRef>& l : lists) {
        for (const SpawnSiteRef& s : l) {
            const float dx = s.x_0fc - x;                       // 00A2384E
            const float dz = s.z_104 - z;
            v.push_back(Keyed{&s, dx * dx + dz * dz});          // 00A2387E, 00A22D90
        }
    }
    // 00A1C790's insertion sort: an entry moves left past strictly greater keys
    // (FCOMI/JBE), so NaN keys stay where they are.
    for (std::size_t i = 1; i < v.size(); ++i) {
        const Keyed k = v[i];
        if (v[0].d2 > k.d2) {
            for (std::size_t j = i; j > 0; --j) v[j] = v[j - 1];
            v[0] = k;
            continue;
        }
        std::size_t j = i;
        while (v[j - 1].d2 > k.d2) {
            v[j] = v[j - 1];
            --j;
        }
        v[j] = k;
    }
    std::vector<const SpawnSiteRef*> out;
    out.reserve(v.size());
    for (const Keyed& k : v) out.push_back(k.site);
    return out;
}

std::optional<SpawnRequest> ai_choose_spawn_composition_00a23980(
    const float* pos_xz, const std::vector<SpawnSite>& sorted_sites, float weight,
    bool allow_mixed, SpawnChoiceHost& host) {
    std::optional<SpawnRequest> best;
    float best_weight = -10000000000.0f;                        // [00CE4ADC]
    for (const SpawnSite& site : sorted_sites) {
        const float budget = host.budget();                     // 00A23AF2
        MatchFinalWeightResult m;
        if (host.effective_game_mode() <= 3u) {                 // 00A23B06 JA
            MatchFinalWeightInputs in;
            in.mode_le_3 = true;
            in.budget = budget;
            in.target = weight;
            in.pos_x = site.ref->x_0fc;
            in.pos_z = site.ref->z_104;
            in.byte5 = false;
            in.allow_mixed = allow_mixed;
            in.arg7 = -1.0f;                                    // [00D7A260]
            m = ai_match_final_weight_00a0d1d0(site.classes, in, host);
        } else {
            m = host.mode4_composition(site, budget);
        }
        if (m.composition.empty()) continue;                    // 00A23EA4
        float w = 1.0f;                                         // [00D7A24C]
        if (sorted_sites.size() > 1) {                          // 00A23EAA JLE
            std::vector<SpeedClassView> speeds;
            for (const AiValueRecord& r : m.composition) {
                for (const SpawnClassView& c : site.classes) {
                    if (c.id == r.object_000) {
                        speeds.push_back(c.speed);
                        break;
                    }
                }
            }
            const float speed = ai_unit_list_min_speed_00a02180(speeds);   // 00A23EC7
            float t;
            if (pos_xz == nullptr) {
                t = host.tuning_100();                          // 00A24029
            } else {
                const float dx = pos_xz[0] - site.ref->x_0fc;   // 00A23FB9
                const float dz = pos_xz[1] - site.ref->z_104;
                const float d2 = dx * dx + dz * dz;
                // [00CE3820] = 1e-10 (double), FCOMI/JBE.
                const float dist = (static_cast<double>(d2) > 1e-10)
                                       ? static_cast<float>(std::sqrt(static_cast<double>(d2)))
                                       : 0.0f;
                t = dist / speed;
            }
            w = host.interpolate_clamped(host.tuning_100(), host.tuning_10c(), host.tuning_104(),
                                         host.tuning_108(), t) *
                m.value;                                        // 00A2407C
        }
        if (w > best_weight) {                                  // 00A2408F JBE
            best_weight = w;
            SpawnRequest req;
            req.site = site.ref;
            req.composition = m.composition;                    // 00A21D90
            req.weight = w;
            best = std::move(req);
        }
    }
    if (!best) return std::nullopt;                             // 00A2414D
    for (const AiValueRecord& r : best->composition) {          // 00A241F0..00A242ED
        best->classes.push_back(r.object_000);
        best->values.push_back(r.dword_010);
        best->squads.push_back(host.squad_size(*best->site, r.object_000));
    }
    return best;
}

bool ai_try_spawn_group_gate_00a25a30(float weight, std::size_t threat_records,
                                      std::size_t sources) noexcept {
    if (0.5f > weight) return false;                            // 00A25A50 COMISS/JBE
    if (threat_records == 0) return false;                      // 00A25A7B, 00A25A89
    if (sources == 0) return false;                             // 00A25ABB
    return true;
}

// ---------------------------------------------------------------------------
// RETREAT: 00A01230 / 00A11530 / 00A156E0
// ---------------------------------------------------------------------------

float ai_net_danger_at_point_00a01230(const float point[3], int team, RetreatHost& host) {
    const float own = host.danger_cost(point, team);                    // 00A01239
    const float other = host.danger_cost(point, team == 0 ? 1 : 0);    // 00A01246 SETE
    return own - other;                                                 // 00A01250 FSUBR
}

std::optional<std::array<float, 3>> ai_retreat_pick_direction_00a11530(int team, RetreatHost& host) {
    constexpr double kRadius = 750.0;                    // [00D22C70]
    constexpr double kStep = 0.5235987901687622;         // [00CEC730]
    constexpr double kEnd = 6.2831854820251465;          // [00CE3828]
    const float y = static_cast<float>(0.0 * kRadius);   // 00A11533..00A1154B
    float best = -10000000000.0f;                        // [00CE4ADC]
    std::optional<std::array<float, 3>> out;
    float a = 0.0f;
    do {
        const float c = static_cast<float>(std::cos(static_cast<double>(a)));   // 00A11574 FCOS
        const float s = static_cast<float>(std::sin(static_cast<double>(a)));   // 00A11586 FSIN
        // 00A115A0 DC C9 (FMUL ST1,ST0): x = 750 sin a; z = 750 cos a.
        const float dx = static_cast<float>(kRadius * s);
        const float dz = static_cast<float>(kRadius * c);
        float centre[3];
        host.leader_point(centre);                       // read again every sample
        const float p[3] = {centre[0] + dx, centre[1] + y, centre[2] + dz};
        const float v = ai_net_danger_at_point_00a01230(p, team, host);   // 00A11619
        if (v > best) {                                  // 00A1162A FCOMI/JBE
            out = std::array<float, 3>{dx, y, dz};
            best = v;
        }
        a = static_cast<float>(static_cast<double>(a) + kStep);           // 00A11660
    } while (kEnd > static_cast<double>(a));                              // 00A11674 JA
    return out;
}

bool ai_retreat_tick_00a156e0(std::uint32_t group_5638, RetreatHost& host) {
    host.retreat_prelude();
    const int team = (group_5638 == 0u) ? 1 : 0;        // 00A15759 SETE
    const std::optional<std::array<float, 3>> d = ai_retreat_pick_direction_00a11530(team, host);
    if (!d) return false;   // LABELLED: the image reads its uninitialised out here
    float c[3];
    host.leader_point(c);
    const float ahead[3] = {c[0] + (*d)[0], c[1] + (*d)[1], c[2] + (*d)[2]};    // 00A1579E
    host.leader_point(c);
    const float behind[3] = {c[0] - (*d)[0], c[1] - (*d)[1], c[2] - (*d)[2]};   // 00A1580B
    const float ahead_cost = host.danger_cost(ahead, team) - 8.0f;              // 00A15846 [00CE3DB0]
    const float behind_cost = host.danger_cost(behind, team);                   // 00A15856
    if (!(ahead_cost > behind_cost)) return false;                              // 00A15864 JBE
    host.leader_point(c);
    const float target[3] = {c[0] - (*d)[0], c[1] - (*d)[1], c[2] - (*d)[2]};   // 00A158A7
    host.issue_move_first_member(target);                                       // 00A158F5
    return true;
}

// ---------------------------------------------------------------------------
// 00A07D40 / 00A24870
// ---------------------------------------------------------------------------

std::vector<const TeamEntityView*> ai_collect_team_entities_00a07d40(
    int team, const std::vector<TeamEntityView>& world, const std::array<float, 3>* sphere_centre,
    float sphere_radius_squared, bool exclude_static) {
    std::vector<const TeamEntityView*> out;
    for (const TeamEntityView& e : world) {
        if (e.byte_05d) continue;                                       // 00A07D73
        if (e.team_054 != team) continue;                               // 00A07D81
        if (sphere_centre != nullptr) {                                 // 00A07D8A
            band009f::AiSphereFilter f;
            f.centre = *sphere_centre;
            f.radius_squared = sphere_radius_squared;
            if (band009f::ai_point_outside_sphere_00a1a660(f, e.pos.data())) continue;   // 00A07DAC
        }
        if (!e.kind_06 && !e.kind_18 && !e.kind_1b) continue;           // 00A07DB2..00A07DDD
        if (exclude_static && (e.kind_1b || e.kind_45 || e.kind_46)) continue;   // 00A07DDF
        out.push_back(&e);                                              // 00A07E1B, 00A07E23
    }
    return out;
}

std::vector<const TeamEntityView*> ai_collect_threats_near_00a24870(
    int planner_team_034, const TeamEntityView& entity, float tuning_1a4,
    const std::vector<TeamEntityView>& world) {
    const float r2 = tuning_1a4 * tuning_1a4;                          // 00A248AE, 00A248C0
    std::vector<const TeamEntityView*> out =
        ai_collect_team_entities_00a07d40(planner_team_034, world, &entity.pos, r2, false);
    if (out.empty()) out.push_back(&entity);                            // 00A248FD
    return out;
}

// ---------------------------------------------------------------------------
// Planner spawn arms
// ---------------------------------------------------------------------------

float ai_planner_owned_resource_00a1c900(const std::vector<float>& group_resources) noexcept {
    float sum = 0.0f;
    for (const float r : group_resources) sum = r + sum;   // 00A1C932 CALL 00A2C530, 00A1C937 FADD
    return sum;
}

float ai_capture_spawn_budget_00a2b4a6(const PlannerSpawnBudgetInputs& in) noexcept {
    const float keep = 1.0f - in.defend_pct;              // FLD1, FSUBRP (DE E1)
    const float share = in.budget_942130 * keep;          // 00A2B4C5
    return share - in.owned_resource;                     // 00A2B4DC FSUBR qword
}

float ai_defend_spawn_budget_00a29c33(const PlannerSpawnBudgetInputs& in) noexcept {
    const float share = in.budget_942130 * in.defend_pct; // 00A29C4E
    return share - in.owned_resource;                     // 00A29C65
}

namespace {

float spawn_amount(float cap, float budget) noexcept {
    // 00A2B689 / 00A29D08: FCOMI cap vs budget, JBE keeps the cap.
    return (cap > budget) ? budget : cap;
}

}  // namespace

std::optional<PlannerSpawnCall> ai_capture_spawn_arm_00a2b477(
    const PlannerSpawnBudgetInputs& in, bool ramp_due, const std::vector<CaptureSpawnTarget>& targets) {
    if (in.queued_946970 > 0.0f) return std::nullopt;     // 00A2B491 JA
    if (!ramp_due) return std::nullopt;                   // 00A2B49B
    const float w = ai_capture_spawn_budget_00a2b4a6(in);
    if (1.0f > w) return std::nullopt;                    // 00A2B4EA JBE continues
    float best = -10000000000.0f;                         // [00CE4ADC]
    std::optional<PlannerSpawnCall> out;
    for (std::size_t i = 0; i < targets.size(); ++i) {
        const CaptureSpawnTarget& t = targets[i];
        if (!(t.value_02c > 0.0f)) continue;              // 00A2B613 COMISS/JBE
        if (!(t.score_014 > best)) continue;              // 00A2B62B FCOMI/JBE
        best = t.score_014;
        out = PlannerSpawnCall{t.entity, spawn_amount(t.cap_01c, w), 0, i};
    }
    return out;
}

std::optional<PlannerSpawnCall> ai_defend_spawn_arm_00a29be7(
    const PlannerSpawnBudgetInputs& in, const std::vector<DefendSpawnRecord>& records) {
    if (in.queued_946970 > 0.0f) return std::nullopt;     // 00A29C01 JBE continues
    const float w = ai_defend_spawn_budget_00a29c33(in);
    if (1.0f > w) return std::nullopt;                    // 00A29C73 JBE continues
    for (std::size_t i = 0; i < records.size(); ++i) {
        if (!(records[i].remaining_030 > 0.0f)) continue; // 00A29CE1 COMISS/JA
        return PlannerSpawnCall{records[i].entity, spawn_amount(records[i].remaining_030, w), 1, i};
    }
    return std::nullopt;
}

}  // namespace gameplay_seg69_70_cc13
}  // namespace bsp

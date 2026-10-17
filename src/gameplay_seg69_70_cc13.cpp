// Segments 69/70 reference models (packet cc13-gpr69). See the header and
// docs/CC13_GPR_69.md. Every comparison and rounding below was read from the
// disk bytes; the instruction addresses are cited where they decide behaviour.

#include "bsp/gameplay_seg69_70_cc13.hpp"
#include "bsp/band_009f_cc13.hpp"
#include "bsp/unit_rudder.hpp"

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

void ai_party_brain_engagement_pass_00a179e0(
    int party, int team, const std::vector<PowerupReadyEntry>& entries,
    const std::vector<EngagementGroup>& own_team, const std::vector<EngagementGroup>& enemy_team,
    const std::vector<PowerupWorldUnit>& world, std::uint32_t game_1fe4, int local_player_team,
    PowerupScoreHost& host, PowerupUseHost& use) {
    if (entries.empty()) return;                                        // 00A17A3A, 00A17A49
    const std::vector<EngagementPair> pairs = ai_engagement_pairs_00a179e0(party, own_team,
                                                                           enemy_team);
    if (pairs.empty()) return;                                          // 00A17D41
    const std::optional<PowerupUseDecision> chosen = ai_powerup_choose_00a179e0(
        party, team, entries, pairs, world, game_1fe4, local_player_team, host);
    // 00A180AD..00A180C9: [ESP+68h] (chosen entry) non-zero -> 008EADA0 on
    // [00F88C30] with (entry, brain+20h, [ESP+34h] target). The list and vector
    // frees that follow are storage only.
    if (chosen.has_value()) use.use_item(chosen->entry, party, chosen->target);
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

float ai_net_danger_at_point_00a01230(const float point[3], int team, DangerHost& host) {
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

bool ai_should_route_cautiously_00a2c9f0(std::int32_t group_5638, float tuning_1f0,
                                         const float point[3], DangerHost& host) {
    if (group_5638 >= 2) return false;                          // 00A2C9FD JL
    const float r2 = tuning_1f0 * tuning_1f0;                   // 00A2CA23
    float lead[3];
    host.leader_point(lead);
    const float dx = lead[0] - point[0];                        // 00A2CA62
    const float dz = lead[2] - point[2];
    const float d2 = dx * dx + dz * dz;                         // 00A2CA82
    if (r2 > d2) return false;                                  // 00A2CA8E FCOMI/JA
    host.leader_point(lead);                                    // 00A2CA9D 00A10C20
    const float sx = point[0] + lead[0];
    const float sy = lead[1] + point[1];
    const float sz = lead[2] + point[2];
    const float mid[3] = {sx * 0.5f, sy * 0.5f, 0.5f * sz};
    const int team = (group_5638 == 0) ? 1 : 0;                 // 00A2CAAA SETE
    const float v = ai_net_danger_at_point_00a01230(mid, team, host);   // 00A2CAED
    return !(0.0f >= v);                                        // 00A2CAF5 FCOMI/JB
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

// ---------------------------------------------------------------------------
// Small group and planner helpers
// ---------------------------------------------------------------------------

float ai_group_resource_sum_00a2c530(const std::vector<float>& member_304) noexcept {
    float sum = 0.0f;
    for (const float v : member_304) sum = v + sum;   // 00A2C570 FLD, 00A2C576 FADD
    return sum;
}

void ai_group_count_unit_types_00a2e1e0(const std::vector<CountedMember>& members,
                                        std::vector<std::pair<std::uint32_t, int>>& map_in_out) {
    for (const CountedMember& m : members) {
        std::pair<std::uint32_t, int>* slot = nullptr;
        for (auto& kv : map_in_out) {
            if (kv.first == m.type_key) {
                slot = &kv;
                break;
            }
        }
        if (slot == nullptr) {                     // operator[] inserts 0
            map_in_out.emplace_back(m.type_key, 0);
            slot = &map_in_out.back();
        }
        slot->second += m.kind18_count;            // 00A2E23F ADD [EBX],EAX
    }
}

DefendOrder ai_order_defend_group_00a1a6d0(bool has_groupable_combatant) noexcept {
    return has_groupable_combatant ? DefendOrder::PatrolToEntity : DefendOrder::DefendPosition;
}

float ai_hinted_class_weight_00a04530(float hint_multiplier, float class_weight) noexcept {
    return static_cast<float>(static_cast<double>(hint_multiplier) *
                              static_cast<double>(class_weight));   // FMUL qword, FSTP float
}

// ---------------------------------------------------------------------------
// 00A32DF0
// ---------------------------------------------------------------------------

PartyInitResult ai_controller_init_party_slots_00a32df0(std::uint32_t effective_game_mode,
                                                         std::array<PlayerSlotFlags, 8>& slots,
                                                         bool game_61c) noexcept {
    if (effective_game_mode <= 3u) {                                    // 00A32E3A JA
        for (int half = 0; half < 2; ++half) {                          // 00A32F0C
            const int first = (half != 0) ? 4 : 0;                      // NEG/SBB/AND 4
            bool human = false;
            for (int s = first; s <= first + 3; ++s) {                  // 00A32E82..00A32EA1
                human = human || (slots[s].present_008 && !slots[s].flag_009);
            }
            if (human) {
                for (int s = first; s <= first + 3; ++s) slots[s].elected_00a = false;   // 00A32EC0
            } else {
                slots[first].elected_00a = true;                        // 00A32ECF
                for (int s = first + 1; s <= first + 3; ++s) slots[s].elected_00a = false;
            }
        }
    }
    PartyInitResult out;
    for (std::size_t p = 0; p < 8; ++p) {                               // 00A32F34..00A32F6C
        const bool on = slots[p].present_008 && slots[p].flag_009 && slots[p].elected_00a;
        out.ai_enabled[p] = on;                                         // 00A32F53
        out.ratios_reset[p] = on;                                       // 00A32F58, 00A32F5D
    }
    if (game_61c) out.ai_enabled[0] = true;                             // 00A32F80
    return out;
}

// ---------------------------------------------------------------------------
// 00A0C330 / 00A0C3C0 / 00A0C650 (packet cc13-gpr69b)
// ---------------------------------------------------------------------------

float ai_pair_base_value_00a0c330(const AiValueRecord& attacker, const AiValueRecord& target,
                                  bool rnd_flag, ComposeAttackHost& host) {
    float w = host.target_weight_00a08460(attacker, target);           // 00A0C345
    if (attacker.dword_01c != 0 && host.class_is_kind(attacker, 0x1C)) // 00A0C34E..00A0C361
        w = 0.0f;
    const float rnd = rnd_flag ? attacker.spread_018 * target.spread_018 : 1.0f;  // 00A0C373 / FLD1
    const float rw = rnd * w;                                           // 00A0C38B
    return rw * target.weight_014;                                      // 00A0C38F, FSTP float
}

namespace {

// 00414C60 BSP_Vector2f_LengthWithCutoff: sqrt of f(dx^2 + dz^2) when that is
// above 1e-10 [00CE3820] (FCOMI/JBE), else 0.
float length_00414c60(float dx, float dz) {
    const float d2 = dz * dz + dx * dx;
    if (!(static_cast<double>(d2) > 1.0e-10)) return 0.0f;
    return static_cast<float>(std::sqrt(static_cast<double>(d2)));
}

}  // namespace

float ai_pair_attack_value_00a0c3c0(const AiValueRecord& attacker, const AiValueRecord& target,
                                    float distance, bool rnd_flag, ComposeAttackHost& host) {
    const float base = ai_pair_base_value_00a0c330(attacker, target, rnd_flag, host);  // 00A0C3D4
    bool ship = false;
    if (host.class_is_kind(attacker, 6)) {                              // 00A0C3F5
        ship = true;
    } else if (!host.class_is_kind(attacker, 0x0F)) {                   // 00A0C51D
        return base;                                                    // 00A0C63C
    }
    // 00A0C40A COMISS 0, d / JBE: only a negative distance is replaced.
    if (0.0f > distance) {
        distance = attacker.byte_00c && target.byte_00c
                       ? length_00414c60(attacker.x_004 - target.x_004,
                                         attacker.z_008 - target.z_008)
                       : 0.0f;
    }
    // FLD arrive / FSUBR qword(distance) / FSTP float.
    const float arrive = host.tuning(ship ? 0xD8u : 0xECu);
    const float beyond = static_cast<float>(static_cast<double>(distance) - arrive);
    if (!(beyond > 0.0f)) return base;                                  // FCOMI/JBE -> 00A0C63A
    const float time = beyond / (ship ? host.class_speed_500(attacker)
                                      : host.class_speed_188(attacker));
    const float mul = ship
        ? host.interpolate_00419010(host.tuning(0xDC), host.tuning(0xE8), host.tuning(0xE0),
                                    host.tuning(0xE4), time)
        : host.interpolate_00419010(host.tuning(0xF0), host.tuning(0xFC), host.tuning(0xF4),
                                    host.tuning(0xF8), time);
    return mul * base;                                                  // 00A0C4F0 / 00A0C618
}

ComposeAttackTerms ai_compose_attack_value_00a0c650(const std::vector<AiValueRecord>& attackers,
                                                    const std::vector<AiValueRecord>& targets,
                                                    const ComposeAttackArgs& args,
                                                    ComposeAttackHost& host) {
    ComposeAttackTerms t;
    const int na = static_cast<int>(attackers.size());
    const int nt = static_cast<int>(targets.size());
    std::vector<float> attacker_best(attackers.size(), 0.0f);   // 004A8F10 with FLDZ
    std::vector<float> target_best(targets.size(), 0.0f);
    for (int i = 0; i < na; ++i) {
        for (int j = 0; j < nt; ++j) {
            // 00A0C7AE: rnd flag = !byte5 || i == last attacker.
            const bool rnd = !args.byte5 || i == na - 1;
            float v = ai_pair_attack_value_00a0c3c0(attackers[i], targets[j], args.distance,
                                                    rnd, host);
            if (!rnd) {                                                 // 00A0C840
                const float f = 0.001f > args.factor ? 0.001f : args.factor;
                v = f * v;                                              // 00A0C86A
            }
            t.attack_sum = v + t.attack_sum;                            // 00A0C87C
            if (v > attacker_best[i]) attacker_best[i] = v;             // FCOMPI/JBE
            if (v > target_best[j]) target_best[j] = v;
        }
    }
    for (const float b : attacker_best) {                               // 00A0CC82..00A0CCA1
        t.attack_maxes_sum = b + t.attack_maxes_sum;
        if (!(0.0f < b) && !std::isnan(b)) ++t.attackers_without_target;  // COMISS/JB
    }
    for (const float b : target_best)                                   // 00A0CCEA..00A0CCF3
        if (!(0.0f < b) && !std::isnan(b)) ++t.targets_unattacked;
    for (std::size_t i = 0; i < attackers.size(); ++i)                  // 00A0CD27..00A0CDBB
        for (std::size_t j = i + 1; j < attackers.size(); ++j)
            if (attackers[i].object_000 == attackers[j].object_000) ++t.repeat_pairs;
    const float ref = host.tuning(0x214);
    t.base = (host.tuning(0x218) * t.attack_sum + t.attack_maxes_sum) / ref;  // 00A0CDD5..E7
    float idle = host.tuning(0x21C) * static_cast<float>(t.attackers_without_target);  // FIMUL
    float unattacked = host.tuning(0x220) * static_cast<float>(t.targets_unattacked);
    float repeat = host.tuning(0x224) * static_cast<float>(t.repeat_pairs);
    const float speed = host.tuning(0x22C) * host.records_min_speed_00a07c10(attackers);
    const float cap = host.tuning(0x228) * t.base;
    t.speed_bonus = speed > cap ? cap : speed;                          // 00A0CE5D FCOMI/JBE
    if (!args.penalties) idle = unattacked = repeat = 0.0f;            // 00A0CE71
    const float sb = t.speed_bonus + t.base;                            // 00A0D09D
    t.result = sb - ((unattacked + idle) + repeat);                     // 00A0D0BD
    if (0.0f > t.result) t.result = 0.0f;                               // 00A0D0CB
    return t;
}

// ---------------------------------------------------------------------------
// 00A13B60 (packet cc13-gpr69b)
// ---------------------------------------------------------------------------

namespace {

// FLD dx / FLD dz / FMUL ST0,ST0 / FLD ST1 / FMULP ST2 / FADDP / FSTP float.
float planar_d2(float ax, float az, float bx, float bz) {
    const float dx = ax - bx;
    const float dz = az - bz;
    return dz * dz + dx * dx;
}

}  // namespace

CloseAttackPassResult ai_close_attack_pass_00a13b60(
    int group_team_5638, int population_5644, float radius, const float centre[3],
    bool has_target_group, const std::vector<CloseAttackEntity>& world_lists,
    const std::vector<CloseAttackMember>& members, CloseAttackPassHost& host) {
    CloseAttackPassResult out;
    const float collect = host.tuning(0x1F4) * radius;                  // 00A13B8A..00A13BB1
    const float r2 = collect * collect;                                 // 00A13BBD
    const int own = group_team_5638;
    const int enemy = group_team_5638 == 0 ? 1 : 0;                     // 00A13BAE SETE
    std::vector<const CloseAttackEntity*> list;
    for (const CloseAttackEntity& e : world_lists) {                    // 00A13BDA..00A1437F
        if (!e.byte_05c || e.byte_05d || e.byte_060 || e.byte_05e) continue;
        if (e.list_slot < 2) {
            if (e.team_054 != enemy) continue;                          // ids 6 and 24
        } else if (e.team_054 != enemy && e.team_054 == own) {
            continue;                                                   // 45h, 46h, 1Bh
        }
        // FCOMPI r2 vs d2 / JBE: kept only when d2 < r2.
        if (!(r2 > planar_d2(e.x_0fc, e.z_104, centre[0], centre[2]))) continue;
        list.push_back(&e);
    }
    for (const CloseAttackMember& m : members) {
        if (m.squadron_18) {                                            // 00A143ED
            if (m.leader_kind_17 && !m.leader_byte_c24) continue;       // 00A14413
            if (host.send_empty_squadron_home_009ffeb0(m.id)) continue; // 00A1441B, 00A14444
        } else {
            if (!m.ship_06) continue;                                   // 00A1442F
            if (m.lands_troops_538_2c) continue;                        // 00A14444
        }
        float best = -10000000000.0f;                                   // [00CE4ADC]
        const CloseAttackEntity* chosen = nullptr;
        const CloseAttackEntity* existing =
            m.current_target != nullptr && m.current_target_kind_2 &&
                    m.current_target->team_054 == enemy
                ? m.current_target
                : nullptr;
        if (existing != nullptr) {
            // 00A14604..00A1464C: outside r but inside 2 r^2 joins the list.
            const float d2 = planar_d2(existing->x_0fc, existing->z_104, centre[0], centre[2]);
            if (!(d2 < r2) && r2 + r2 > d2) list.push_back(existing);
        }
        for (const CloseAttackEntity* c : list) {                       // 00A14662..00A149DA
            const bool in_group = has_target_group && host.target_group_contains_00a2c720(c->id);
            const float w = host.candidate_weight_00a0f810(m.id, c->id);
            if (!(0.0f < w) && !in_group) continue;                     // 00A146D3 / 00A146D9
            const float d2 = planar_d2(m.pos[0], m.pos[2], c->x_0fc, c->z_104);  // 00A148CD
            const float d = static_cast<double>(d2) > 1.0e-10
                                ? static_cast<float>(std::sqrt(static_cast<double>(d2)))
                                : 0.0f;
            const float range = host.interpolate_00419010(host.tuning(0x1F8), 1.0f,
                                                          host.tuning(0x1FC), 0.0f, d);
            const float rw = range * w;                                 // 00A14942
            const float g = in_group ? host.tuning(0x204) : 1.0f;       // 00A1494C
            const float s = c == existing ? host.tuning(0x200) : 1.0f;  // 00A14965
            const float score = (s * g) * rw;                           // 00A14990..00A1499C
            if (score > best) {                                         // FCOMPI / JBE
                best = score;
                chosen = c;
            }
        }
        if (chosen != nullptr) {
            out.candidate_found = true;                                 // 00A149F3
            host.issue_order_0077d600(m.id, m.ship_06 ? 0x00E08F78u : 0x00E08EF8u, chosen->id);
            continue;
        }
        // 00A14A78..00A14D48: no candidate.
        float r = static_cast<float>(population_5644) * 160.0f;         // FILD, FMUL [00D22C90]
        if (400.0f > r) r = 400.0f;                                     // [00CE3D90] / [00CFD710]
        const float fr2 = r * r;
        const float d2 = planar_d2(m.pos[0], m.pos[2], centre[0], centre[2]);  // 00A14BD3
        if (!(d2 > fr2)) continue;                                      // 00A14BE3
        float point[3];
        for (int k = 0; k < 3; ++k) {
            const float own_part = m.pos[k] * 0.4f;                     // [00CE65D0], FSTP float
            const float centre_part = centre[k] * 0.6f;                 // [00CEFF98], FSTP float
            point[k] = centre_part + own_part;                          // 00A14D1B..00A14D41
        }
        host.issue_moveto_00a02020(m.id, point);
    }
    for (const CloseAttackEntity* c : list) out.candidates.push_back(c->id);
    return out;
}

// ---------------------------------------------------------------------------
// 00A2DEF0 / 00A28300 (packet cc13-gpr69b)
// ---------------------------------------------------------------------------

bool ai_capture_planner_clock_00a29fd0(CapturePlannerClock& clock, float now,
                                       float tuning_1c0, float tuning_1c4, float tuning_1c8) {
    float dt = 0.0f;
    if (!(clock.last_clock_040 < 0.0f)) {                                // 00A2A020 JB
        const float d = now - clock.last_clock_040;                     // 00A2A02A
        if (!(0.0f > d)) dt = d;                                        // 00A2A038 JA keeps 0
    }
    clock.last_clock_040 = now;                                         // 00A2A04E
    clock.age_038 = clock.age_038 + dt;                                 // 00A2A061
    clock.since_spawn_03c = clock.since_spawn_03c + dt;                 // 00A2A06B
    const float delay = bsp::clamped_interpolate_00419010(0.0f, tuning_1c0, tuning_1c8,
                                                          tuning_1c4, clock.age_038);
    return clock.since_spawn_03c > delay;                               // 00A2A0BB FCOMPI/JA
}

float ai_capture_assignment_score_00a2ac11(float k_1bc, float target_total,
                                           float group_weight) noexcept {
    const float one_minus_k = 1.0f - k_1bc;                             // FLD1 / FSUBRP
    const float wterm = one_minus_k * group_weight;                     // FMUL, FSTP qword
    const float sterm = k_1bc * target_total;                           // 00A2AC33
    return sterm + wterm;                                               // FADD qword, FSTP float
}

// 00A11110..00A112B1: xmm0 = -1.0 [00D7A260], xmm1 = 1.0 [00D7A24C],
// xmm2 = -2.0 [00CE7D7C], xmm3 = 2.0 [00CE3958], xmm4 = 0.
const std::array<std::array<float, 2>, kFormationShapeSlots> kFormationShapeTable00f8a7b0{{
    {-1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {-1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, -1.0f},
    {-1.0f, -1.0f}, {1.0f, -1.0f}, {-2.0f, 0.0f}, {2.0f, 0.0f}, {-2.0f, 1.0f}, {2.0f, 1.0f},
    {-2.0f, -1.0f}, {2.0f, -1.0f}, {0.0f, 2.0f}, {-1.0f, 2.0f}, {1.0f, 2.0f}, {-2.0f, 2.0f},
    {2.0f, 2.0f}, {0.0f, -2.0f}, {-1.0f, -2.0f}, {1.0f, -2.0f}, {-2.0f, -2.0f}, {2.0f, -2.0f},
}};

FormationShapeResult ai_formation_shape_00a11070(bool leader_ship_06, bool leader_has_formation_284,
                                                 float tuning_210,
                                                 const std::vector<FormationShapeFollower>& followers) {
    FormationShapeResult out;
    if (!leader_ship_06 || !leader_has_formation_284) return out;      // 00A110B2 / 00A110DF
    out.ran = true;
    int slot = 0;
    for (const FormationShapeFollower& f : followers) {                 // 00A112DF..00A1136B
        if (!f.ship_06 || !f.has_record) continue;                      // 00A11321 / 00A1133C
        if (slot >= kFormationShapeSlots) break;                        // image: reads past the table
        FormationShapeSlot s;
        s.member = f.member;
        s.slot = slot;
        s.record_010 = -kFormationShapeTable00f8a7b0[slot][0] * tuning_210;  // FCHS, FMULP
        s.record_020 = -kFormationShapeTable00f8a7b0[slot][1] * tuning_210;
        out.slots.push_back(s);
        ++slot;                                                         // ADD EBP,8
    }
    return out;
}

RegroupingMoveResult ai_regrouping_move_tick_00a126c0(
    const std::vector<std::array<float, 3>>& members, const float destination[3],
    float tuning_20c, const float origin_00f87574[3]) {
    RegroupingMoveResult out;
    for (int k = 0; k < 3; ++k) out.point[k] = destination[k];          // 00A127EA LEA EDX,[EBP+8]
    if (members.size() <= 1u) return out;                               // 00A126D3
    const float r2 = tuning_20c * tuning_20c;                           // 00A12740
    const std::array<float, 3>& lead = members.front();
    bool straggler = false;
    for (std::size_t i = 1; i < members.size() && !straggler; ++i) {    // 00A1274A..00A127CC
        straggler = planar_d2(members[i][0], members[i][2], lead[0], lead[2]) > r2;  // JA
    }
    if (!straggler) return out;
    out.regrouping = true;
    float c[3] = {origin_00f87574[0], origin_00f87574[1], origin_00f87574[2]};  // 00A127F2
    int n = 0;
    for (const std::array<float, 3>& m : members) {                     // 00A12840..00A1297C
        ++n;
        for (int k = 0; k < 3; ++k) c[k] = c[k] + m[k];
    }
    const float nf = static_cast<float>(n);                             // 00A12981 FILD
    for (int k = 0; k < 3; ++k) {
        const float mean = c[k] / nf;                                   // 00A1299F..00A129B1
        const float scaled = mean * 3.0f;                               // [00D7A2B0]
        const float sum = destination[k] + scaled;                      // 00A129E0..00A129F6
        out.point[k] = sum * 0.25f;                                     // [00D7A348]
    }
    return out;
}

OrderMoveChoice ai_order_cautious_move_00a2ccf0(std::uint32_t population_5644, const float point[3],
                                                float ratio, OrderMoveHost& host) {
    if (population_5644 == 0u) return OrderMoveChoice::None;           // 00A2CCFA
    if (host.already_moving_to_00a2c150(point)) return OrderMoveChoice::Kept;  // 00A2CD0D
    if (host.leader_is_ship()) return OrderMoveChoice::MoveTo;          // 00A2CD34
    if (!host.should_route_cautiously_00a2c9f0(point)) return OrderMoveChoice::MoveTo;  // 00A2CD40
    const float draw = host.uniform01_stream1();                        // 00A2CD47
    return draw > ratio ? OrderMoveChoice::CautiousMove : OrderMoveChoice::MoveTo;  // 00A2CD52 JBE
}

ModeThinkResult ai_duel_think_00a25f70(const DuelThinkInputs& in) {
    ModeThinkResult out;
    if (in.owned_groups_028 == 0u) {                                    // 00A25F8F
        out.action = ModeThinkAction::SpawnGroup;
        return out;
    }
    if (in.nearest_enemy_group == nullptr) {                            // 00A2607C -> 00A261D1
        out.action = ModeThinkAction::CautiousMove;
        for (int k = 0; k < 3; ++k) out.point[k] = in.own_centroid[k];
        return out;
    }
    if (16000000.0f > in.nearest_enemy_d2 || in.own_team_group_count <= 1u) {  // 00A2608C / 00A26099
        out.action = ModeThinkAction::Attack;
        out.target = in.nearest_enemy_group;
        return out;
    }
    const float v[2] = {in.leader[0] - in.own_centroid[0], in.leader[2] - in.own_centroid[2]};
    const float d2 = v[1] * v[1] + v[0] * v[0];                         // 009FFC10
    const float d = static_cast<double>(d2) > 1.0e-10
                        ? static_cast<float>(std::sqrt(static_cast<double>(d2)))
                        : 0.0f;
    const float t = bsp::clamped_interpolate_00419010(500.0f, 0.0f, 2000.0f, 1.0f, d);
    const float u = 1.0f - t;                                           // 00A2613F
    out.action = ModeThinkAction::CautiousMove;
    for (int k = 0; k < 3; ++k) {
        const float e = in.enemy_centroid[k] * u;                       // 00A26153..00A26165
        const float o = in.own_centroid[k] * t;                         // 00A2616F..00A26181
        out.point[k] = o + e;                                           // 00A2618D..00A261A5
    }
    return out;
}

ModeThinkResult ai_escort_think_00a26210(const EscortThinkInputs& in) {
    ModeThinkResult out;
    if (in.owned_groups_028 == 0u) {                                    // 00A26235
        out.action = ModeThinkAction::SpawnGroup;
        return out;
    }
    if (in.set.empty()) {                                               // 00A262D3 -> 00A264EB
        out.action = ModeThinkAction::ChooseAttackTarget;
        return out;
    }
    const EscortSetEntity* nearest = nullptr;
    float best = 1.0e10f;                                               // [00CE4970]
    for (const EscortSetEntity& e : in.set) {                           // 00A26300..00A263D8
        const float d2 = planar_d2(e.x_0fc, e.z_104, in.leader[0], in.leader[2]);
        if (best > d2) {                                                // FCOMPI / JBE
            best = d2;
            nearest = &e;
        }
    }
    if (nearest == nullptr) return out;   // the image reads [0+54h] here
    if (nearest->team_054 == in.own_team_030) {                         // 00A263EB
        out.action = ModeThinkAction::PatrolTo;
        const float e[3] = {nearest->x_0fc, nearest->y_100, nearest->z_104};
        for (int k = 0; k < 3; ++k) {
            const float s = e[k] + in.enemy_centroid[k];                // 00A2641D..00A26442
            out.point[k] = s * 0.5f;                                    // [00D7A280]
        }
        return out;
    }
    if (nearest->team_054 == in.enemy_team_034) {                       // 00A26482
        if (nearest->group_16c != nullptr) {                            // 00A2648F
            out.action = ModeThinkAction::Attack;
            out.target = nearest->group_16c;
            return out;
        }
        out.action = ModeThinkAction::PatrolTo;                         // 00A264C4..00A264D3
        out.point[0] = nearest->x_0fc;
        out.point[1] = nearest->y_100;
        out.point[2] = nearest->z_104;
        return out;
    }
    return out;
}

float ai_unit_arrival_value_00a03760(float capture_weight_00a03510, float distance_009ffc10,
                                     int target_7a0, float speed, float tuning_198) noexcept {
    const float r = static_cast<float>(target_7a0);                     // 00A037CF FILD, FSTP float
    if (!(distance_009ffc10 > r)) return capture_weight_00a03510;       // 00A037E5 JBE
    if (static_cast<double>(speed) > 0.10000000149011612) return 0.0f; // 00A03840 JA -> 00A03896
    const float time = (distance_009ffc10 - r) / speed;                 // 00A03846 FSUB, DE F1
    const float limit = 1.0f > tuning_198 ? 1.0f : tuning_198;          // 00A0386A FCOMI/JBE
    if (time >= limit) return 0.0f;                                     // 00A03890 JB (NaN computes)
    return (1.0f - time / limit) * capture_weight_00a03510;             // DE F1, DE E9, FMUL
}

bool ai_defend_member_is_candidate_00a2def0(bool member_kind_1c, bool in_team_set) noexcept {
    return member_kind_1c || in_team_set;   // 00A2DF2F JNE / 00A2DF4A JE
}

std::optional<std::vector<std::size_t>> ai_defend_sort_records_00a28300(
    const std::vector<DefendRecordKey>& records) {
    if (records.size() > 32u) return std::nullopt;   // 00A28320
    auto before = [&records](std::size_t x, std::size_t y) {
        return 0.0f < records[x].threat_004 && records[y].weight_008 < records[x].weight_008;
    };
    std::vector<std::size_t> order(records.size());
    for (std::size_t i = 0; i < order.size(); ++i) order[i] = i;
    // 00A25750: each element either goes to the front (copy_backward 00A233F0)
    // or is shifted back while it goes before its predecessor.
    for (std::size_t i = 1; i < order.size(); ++i) {
        const std::size_t v = order[i];
        if (before(v, order[0])) {
            for (std::size_t k = i; k > 0; --k) order[k] = order[k - 1];
            order[0] = v;
            continue;
        }
        std::size_t j = i;
        while (before(v, order[j - 1])) {
            order[j] = order[j - 1];
            --j;
        }
        order[j] = v;
    }
    return order;
}

}  // namespace gameplay_seg69_70_cc13
}  // namespace bsp

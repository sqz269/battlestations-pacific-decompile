// Reconstruction of the unit-side gunnery pass. Evidence in
// docs/UNIT_GUNNERY_PASS.md; each routine is a projection of one native body and
// the coverage of each is in that document's routine table.
#include "bsp/unit_gunnery_pass.hpp"

namespace bsp {

namespace {

// 00861CD0 and 00861D20 edit one bit of the mask word; 00861D70 and 00861DC0
// store 3 or 0 over the whole word. The loops run while the list entry is
// < 0Ch, which is why every list here is already trimmed to its real length.
void set_mask_bit(UnitGunneryCategoryState& state, int category,
                  std::uint32_t bit, bool on) noexcept
{
    if (category < 0 || category >= kUnitGunneryCategoryCount) {
        return;
    }
    if (on) {
        state.mask[static_cast<std::size_t>(category)] |= bit;   // 00861CF2, 00861D42
    } else {
        state.mask[static_cast<std::size_t>(category)] &= ~bit;  // 00861CF7, 00861D47
    }
}

void store_mask_word(UnitGunneryCategoryState& state, int category, bool on) noexcept
{
    if (category < 0 || category >= kUnitGunneryCategoryCount) {
        return;
    }
    // 00861D8B stores 3, 00861D98 stores 0.
    state.mask[static_cast<std::size_t>(category)] = on ? 0x3u : 0x0u;
}

}  // namespace

// ---------------------------------------------------------------------------
// 00727BD0, the target rank table
// ---------------------------------------------------------------------------
void build_target_rank_row_00727bd0(const int* preference_list,
                                    int preference_count,
                                    int* out) noexcept
{
    // 00727BEC: rep stosd clears the whole 61h-entry row first.
    for (int i = 0; i < kUnitGunneryClassIdCount; ++i) {
        out[i] = 0;
    }
    if (preference_list == nullptr) {
        return;
    }

    // 00727BF3: rank starts at 1 and only advances past a non-zero entry, so a
    // zero in the authored list is a hole, not a class id.
    int rank = 1;
    for (int i = 0; i < preference_count && i < kUnitGunneryClassIdCount; ++i) {
        const int id = preference_list[i];
        if (id == 0) {   // 00727BF5
            continue;
        }
        if (id >= 0 && id < kUnitGunneryClassIdCount) {
            out[id] = rank;  // 00727BFB
        }
        ++rank;              // 00727C02
    }
}

// ---------------------------------------------------------------------------
// 00864580 and 008624C0, the category state
// ---------------------------------------------------------------------------
UnitGunneryCategoryState unit_gunnery_initial_category_state_00864580() noexcept
{
    UnitGunneryCategoryState state;
    for (int i = 0; i < kUnitGunneryCategoryCount; ++i) {
        state.enabled[static_cast<std::size_t>(i)] = true;  // 0086462D
        state.mask[static_cast<std::size_t>(i)] = 0x3u;     // 00862632
    }
    // this+7Ch is not written by the constructor; the bridge's step 1 supplies it.
    state.torpedo_enable = false;
    state.torpedo_group_flag = false;
    state.depth_charge_group_flag = false;
    return state;
}

UnitGunneryCategoryState apply_director_stance_008624c0(
    const UnitGunneryCategoryState& previous,
    const DirectorGunneryStance& stance,
    bool owner_is_plane,
    bool force,
    int& countdown,
    bool& allow_fire_cache) noexcept
{
    UnitGunneryCategoryState state = previous;

    // Step 1, 008624CB: this+7Ch tracks director+3Dh unconditionally.
    state.torpedo_enable = stance.torpedo_category_enable;

    // Step 2, 008624D3: the countdown forces a full push every tenth call.
    --countdown;
    bool forced = force;
    if (forced || countdown <= 0) {
        forced = true;
        countdown = kUnitGunneryBridgeForcePeriod;  // 008624EB
    }

    // Step 3, 008624F4. adapter+0Ch caches the last allowFire that was pushed;
    // an unforced call whose allowFire has not moved does nothing here.
    if (forced || stance.allow_fire != allow_fire_cache) {
        allow_fire_cache = stance.allow_fire;   // 00862503
        // 00862508 and 0086252A both write three dwords over the twelve bytes and
        // then restore +77h and +78h, which belong to steps 4 and 7.
        for (int i = 0; i < kUnitGunneryCategoryCount; ++i) {
            state.enabled[static_cast<std::size_t>(i)] = stance.allow_fire;
        }
        state.enabled[7] = previous.enabled[7];
        state.enabled[8] = previous.enabled[8];

        if (!stance.allow_fire && owner_is_plane) {
            // 00862558: a plane keeps its first two categories whatever the
            // director says.
            state.enabled[0] = true;
            state.enabled[1] = true;
        }
    }

    // Steps 4 to 7. Each list is walked in full; the change test the native code
    // makes against adapter+0Dh..+10h only saves work, it does not change the
    // result, so the projection applies every group every time.
    for (const int category : kTorpedoCategories) {          // 00861D70
        store_mask_word(state, category, stance.torpedo);
    }
    state.torpedo_group_flag = stance.torpedo;               // 00861DB7

    for (const int category : kArtilleryCategories) {        // 00861CD0
        set_mask_bit(state, category, kCategoryMaskMayEngageOther, stance.artillery);
    }

    for (const int category : kAaFlakCategories) {           // 00861D20
        set_mask_bit(state, category, kCategoryMaskMayEngagePlane, stance.anti_air);
    }

    for (const int category : kDepthChargeCategories) {      // 00861DC0
        store_mask_word(state, category, stance.depth_charge);
    }
    state.depth_charge_group_flag = stance.depth_charge;     // 00861E07

    return state;
}

// ---------------------------------------------------------------------------
// 008633D0 and 00862820, the target admission tests
// ---------------------------------------------------------------------------
bool category_mask_admits_target_008633d0(std::uint32_t mask,
                                          bool target_is_plane) noexcept
{
    // 008633F0: without bit 0 a plane is refused.
    if ((mask & kCategoryMaskMayEngagePlane) == 0 && target_is_plane) {
        return false;
    }
    // 00863409: without bit 1 everything that is not a plane is refused.
    if ((mask & kCategoryMaskMayEngageOther) == 0 && !target_is_plane) {
        return false;
    }
    return true;
}

bool target_is_engageable_00862820(const GunneryTargetLiveness& liveness) noexcept
{
    // 00862825..0086283B, four bytes in one chain.
    if (!liveness.registered) {
        return false;
    }
    if (liveness.dead) {
        return false;
    }
    if (liveness.flag60) {
        return false;
    }
    if (liveness.flag5e) {
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// 00863990, the candidate score
// ---------------------------------------------------------------------------
GunneryScoreResult score_candidate_00863990(const GunneryScoreInputs& in) noexcept
{
    GunneryScoreResult result;
    result.distance = in.distance;   // 00863A32 stores before the range test

    // 00863A34: FLD the range, FXCH, FCOMIP, JC. The candidate is kept only when
    // the distance is strictly below the category's range.
    if (!(in.distance < in.category_range)) {
        result.accepted = false;
        return result;
    }

    // 00863A4F: a plane with no follow target is pushed back before the tie-break.
    if (in.target_is_plane && in.target_lacks_follow_target) {
        result.distance += kUnitGunneryLoiteringPlanePenalty;
    }
    result.accepted = true;
    return result;
}

// ---------------------------------------------------------------------------
// 00865284, the ordered insert, and 008657A3, the walk order
// ---------------------------------------------------------------------------
int insert_ranked_candidate_00865284(const GunneryCandidate* candidates,
                                     int* order,
                                     int count,
                                     int new_index) noexcept
{
    const int rank = candidates[new_index].rank;
    const float distance = candidates[new_index].distance;

    // 00865291..008653BD. The native scan is unrolled four ways; the predicate is
    // the same at every step. It stops at the first position the new candidate
    // outranks, ties broken on a larger distance.
    int at = 0;
    while (at < count) {
        const GunneryCandidate& other = candidates[order[at]];
        if (rank > other.rank) {
            break;                                  // 008652B2 JG
        }
        if (rank == other.rank && distance > other.distance) {
            break;                                  // 008652C5 JA
        }
        ++at;                                       // 008653BF
    }

    // 008653F9..00865421: shift the tail down and drop the new index in.
    for (int i = count; i > at; --i) {
        order[i] = order[i - 1];
    }
    order[at] = new_index;
    return at;
}

int candidate_walk_order_008657a3(const int* order, int count, int* out) noexcept
{
    // 00865789 sets the cursor to count-1 and 00865822 decrements it, so the last
    // insert position is visited first.
    int written = 0;
    for (int k = count - 1; k >= 0; --k) {
        out[written++] = order[k];
    }
    return written;
}

// ---------------------------------------------------------------------------
// 008657C0, the per-gun decision
// ---------------------------------------------------------------------------
GunneryCandidateVerdict evaluate_candidate_for_gun_008657c0(
    const GunneryGunInputs& gun,
    const GunneryCandidate& candidate,
    bool candidate_is_plane,
    bool candidate_is_fire_target,
    int category,
    bool torpedo_may_take_fire_target) noexcept
{
    // 008657C0..008657F0. A torpedo-class launcher will not take an air target
    // that is nearer than the ammunition's minimum range.
    if (gun.is_torpedo_class_launcher && candidate_is_plane) {
        if (gun.minimum_air_range > candidate.distance) {
            return GunneryCandidateVerdict::kSkip;
        }
    }

    // 008657F7: the gun's own slot test.
    if (!gun.slot_accepts_target) {
        return GunneryCandidateVerdict::kSkip;
    }

    // 00865809: the torpedo category refuses the director's own fire target
    // unless this+7Dh says otherwise.
    if (category == kUnitGunneryTorpedoCategory && candidate_is_fire_target &&
        !torpedo_may_take_fire_target) {
        return GunneryCandidateVerdict::kSkip;
    }

    return GunneryCandidateVerdict::kAssign;
}

bool weapon_sub_type_wants_target_record_00865838(int weapon_sub_type) noexcept
{
    // 00865844..00865856.
    return weapon_sub_type == 2 || weapon_sub_type == 3 || weapon_sub_type == 4 ||
           weapon_sub_type == 6;
}

std::size_t bot_slot_for_projectile_kind_00729bc0(int projectile_kind,
                                                  int weapon_sub_type) noexcept
{
    // 00729C8C..00729D00. The offsets are the gun's six bot slots.
    if (projectile_kind == 4 || projectile_kind == 5 || projectile_kind == 6 ||
        projectile_kind == 7) {
        return 0x398;
    }
    if (projectile_kind == 1 || projectile_kind == 2 || projectile_kind == 3) {
        return 0x390;
    }
    if (projectile_kind == 0x10) {
        return 0x394;
    }
    if (projectile_kind == 0x0A) {
        return 0x39C;
    }
    if (projectile_kind == 0x0B) {
        return weapon_sub_type == 9 ? 0x3A4 : 0x3A0;
    }
    return 0;   // 00729D0A, the gun cannot fire this kind at all
}

// ---------------------------------------------------------------------------
// 00862C30, the visibility cache ageing
// ---------------------------------------------------------------------------
bool visibility_entry_survives_00862c30(float& ttl, float dt) noexcept
{
    // 00862C52: the entry's +8h is reduced by dt and written back before the test.
    ttl -= dt;
    // 00862C67: FCOMI against zero with JBE keeping the entry, so an entry that
    // has reached exactly zero is dropped.
    return ttl > 0.0f;
}

// ---------------------------------------------------------------------------
// 00864FE0, the pass
// ---------------------------------------------------------------------------
void unit_gunnery_pass_tick_00864fe0(UnitGunneryPassHost& host,
                                     float dt,
                                     float& throttle,
                                     bool& enabled) noexcept
{
    // Step 0, 00864FF0..0086500E. One latch, and it never re-arms itself.
    if (!enabled || !host.unit_present() || host.unit_is_dead()) {
        enabled = false;
        return;
    }

    // Step 1, 00865014..00865035.
    throttle += dt;
    const float elapsed = throttle;
    if (elapsed < host.throttle_threshold_00432650()) {
        return;
    }

    // Step 2, 0086503B. The native code clears this+59h here; the host owns the
    // byte, so the projection asks it and lets it clear its own copy.
    const bool sweep_suppressed =
        host.sweep_suppressed() && host.unit_allows_sweep();

    // Steps 3 and 4.
    host.age_visibility_cache_00862c30(elapsed);
    throttle = 0.0f;

    // Step 5, 0086506F.
    if (host.has_director_bridge()) {
        host.apply_director_stance_008624c0();
    }

    // Step 6, 00865078. The list walk prunes and updates; the host holds the list.
    host.update_target_records_00862cd0();

    // Step 7 is the stack array; the host supplies the storage.
    GunneryCandidate candidates[kUnitGunneryCandidateCapacity];
    int order[kUnitGunneryCandidateCapacity + 1];
    int walk[kUnitGunneryCandidateCapacity];

    for (int category = 0; category < kUnitGunneryCategoryCount; ++category) {
        // Step 8.1, 0086516D.
        if (!host.category_record_present(category)) {
            continue;
        }

        // Step 8.2 and 8.3, 00865191 and 008651B4.
        bool category_enabled = host.category_enabled(category);
        if (category_enabled && category == kUnitGunneryTorpedoCategory) {
            category_enabled = host.torpedo_category_enabled();
        }

        // The candidate list is emptied per category at 008651AA.
        int count = 0;

        // Step 8.4, 008651C0. A refused gate skips the gather and the assignment
        // both: the native code jumps past 00865773 as well.
        if (!host.category_gate_slot4(category)) {
            continue;
        }

        // Step 8.5, 008651D7. The torpedo category never sweeps the recon list.
        if (category_enabled && !sweep_suppressed &&
            category != kUnitGunneryTorpedoCategory) {
            const int contacts = host.recon_contact_count_008053c0();
            for (int i = 0; i < contacts && count < kUnitGunneryCandidateCapacity; ++i) {
                void* contact = host.recon_contact(i);
                float distance = 0.0f;
                if (!host.score_candidate_00863990(category, contact, distance)) {
                    continue;                                  // 0086523E
                }
                if (host.unit_ai_suppresses_00862440(contact)) {
                    continue;                                  // 0086524F
                }
                if (!host.visible_00864d90(contact)) {
                    continue;                                  // 00865264
                }
                candidates[count].entity = contact;
                candidates[count].distance = distance;
                candidates[count].rank = host.target_rank(category, contact);
                insert_ranked_candidate_00865284(candidates, order, count, count);
                ++count;
            }
        }

        // Step 8.6, 00865442.
        void* const fire_target = host.director_fire_target_slot4();
        void* const command_target = host.director_command_target_0071ebf0();

        // Step 8.7. Both arms append unsorted, so their entries sit at the end of
        // the order array and are therefore tried first.
        const std::array<float, 3> origin = host.unit_world_position_00427eb0();
        void* const expand[2] = {command_target, fire_target};
        for (int which = 0; which < 2; ++which) {
            void* const target = expand[which];
            if (target == nullptr) {
                continue;                                      // 008654BB, 0086560F
            }
            // 008654AC: the command target is skipped when it is the fire target,
            // because the second arm would add the same sub-entities again.
            if (which == 0 && target == fire_target) {
                continue;
            }
            float distance = 0.0f;
            if (!host.score_candidate_00863990(category, target, distance)) {
                continue;                                      // 008654DB, 0086562D
            }
            const int subs = host.sub_entities_slot0fc(target);
            for (int i = 0; i < subs && count < kUnitGunneryCandidateCapacity; ++i) {
                void* const sub = host.sub_entity(i);
                if (!host.visible_00864d90(sub)) {
                    continue;                                  // 0086558B, 008656EC
                }
                const std::array<float, 3> pos = host.entity_world_position(sub);
                const std::array<float, 3> delta = {origin[0] - pos[0],
                                                    origin[1] - pos[1],
                                                    origin[2] - pos[2]};
                candidates[count].entity = sub;
                candidates[count].distance = host.vector_length_0042b2f0(delta);
                candidates[count].rank = 0;
                order[count] = count;                          // 008655E1, 0086574B
                ++count;
            }
        }

        // Step 8.8, 00865773.
        const int walk_count = candidate_walk_order_008657a3(order, count, walk);
        const int guns = host.category_gun_count(category);
        for (int g = 0; g < guns; ++g) {
            void* const gun = host.category_gun(category, g);
            bool assigned = false;
            for (int k = 0; k < walk_count; ++k) {
                const GunneryCandidate& candidate = candidates[walk[k]];
                const bool is_plane = host.target_is_plane(candidate.entity);
                const bool is_fire_target = candidate.entity == fire_target;
                const GunneryGunInputs inputs = host.gun_inputs(gun, candidate.entity);
                if (evaluate_candidate_for_gun_008657c0(
                        inputs, candidate, is_plane, is_fire_target, category,
                        host.torpedo_may_take_fire_target()) ==
                    GunneryCandidateVerdict::kSkip) {
                    continue;
                }
                host.set_bot_fire_target_00727f10(gun, candidate.entity,
                                                  is_fire_target);   // 00865833
                if (weapon_sub_type_wants_target_record_00865838(inputs.weapon_sub_type)) {
                    host.add_gun_to_target_record_00864ca0(candidate.entity, gun);
                }
                assigned = true;
                break;                                               // 00865871
            }
            if (!assigned) {
                host.clear_bot_fire_target_00728000(gun);             // 0086586C
            }
        }
    }
}

}  // namespace bsp

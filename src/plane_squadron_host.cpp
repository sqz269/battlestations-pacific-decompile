#include "bsp/plane_squadron_host.hpp"

#include <algorithm>

#include "bsp/plane_squadron_entity.hpp"
#include "bsp/native_session_message_be.hpp"
#include "bsp/scene_deferred_refs.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Retained land profiles and descriptor copy require MSVC Win32 assembly.
#endif

// 007F4580 mode 1, bound to a process whose "plane instance" is a scene entity
// record. docs/PLANE_SQUADRON_HOST.md.

namespace bsp {
namespace {

// The binding. Every method is one native call site of the loop; the ones this
// process cannot make are answered with the value the native's own result is
// used for, and nothing else is invented.
class MemberPlanSpawnHost final : public PlaneSquadronSpawnHost {
   public:
    MemberPlanSpawnHost(const PlaneSquadronSpawnRequest& request,
                        PlaneSquadronSpawnPlan& plan)
        : request_(request), plan_(plan) {}

    // 007F45A7 -> 0077E830. The base attach is the Lua self table, which
    // GameMissionLuaHost::attach_created_entity_00928a00 owns for the squadron
    // entity; nothing about it is per-wing, so the loop binding does not repeat
    // it. Recorded, not performed.
    void attach_lua_self_base_0077e830() override { base_attach_calls_ += 1; }

    // 007F4724 / 007F4742 / 007F4759 / 007F479D / 007F47C7 -> 008F2260. The
    // caller has already read the bag, because in this process the bag is a
    // ScenePropertyBlock the scene pass holds and the air-ops seam builds.
    SquadronSpawnProperties read_spawn_properties_008f2260() override {
        SquadronSpawnProperties props;
        props.type_class_id = request_.type_class_id;
        props.wing_count_present = request_.wing_count_present;
        props.wing_count_raw = request_.wing_count_raw;
        props.parent_present = request_.parent_present;
        props.parent_id = request_.parent_id;
        props.behaviour_present = request_.behaviour_present;
        props.behaviour = request_.behaviour;
        return props;
    }

    // 007F477E and 007F47E1 -> 007B8A80. Both calls take the same class id and
    // the second one's result is only ever used as the factory the loop drives,
    // so the class id stands for the class. A zero id resolves to nothing, which
    // is what an unresolvable `Type` does.
    std::uint32_t vehicle_class_for_type_007b8a80(std::int32_t type_class_id) override {
        return type_class_id > 0 ? static_cast<std::uint32_t>(type_class_id) : 0u;
    }

    // 007F47AE -> 00521E30. Resolved by the caller, because the entity table is
    // the units host's.
    std::uint32_t resolve_parent_entity_00521e30(std::int32_t) override {
        return request_.parent_entity;
    }

    // 007F4811 -> vehicleClass->vtable[28h](0). The instance this process can
    // make at this point is a plan; it becomes a unit when create_units runs
    // over the records the caller builds from the plan. The handle is the
    // one-based plan index so that zero keeps meaning "nothing was made".
    std::uint32_t create_plane_instance(std::uint32_t vehicle_class) override {
        if (vehicle_class == 0u) return 0u;
        plan_.members.emplace_back();
        return static_cast<std::uint32_t>(plan_.members.size());
    }

    // 007F48BA / 007F48D2 -> plane->vtable[98h]. The placement itself is the
    // caller's, which has the squadron's world frame; what is kept here is which
    // arm the native took, because that is what decides whether the member
    // inherits the squadron's own local matrix.
    void place_plane_in_world(std::uint32_t plane, std::uint32_t, std::uint32_t,
                              const SquadronPlanePlacement& placement) override {
        PlaneSquadronMemberPlan* member = at(plane);
        if (member == nullptr) return;
        member->under_parent = placement.under_parent;
        member->use_squadron_local = placement.use_squadron_local;
    }

    // 007F48D6 operator new(0Ch) then 007F48FD -> 00922DE0. The per-plane copy of
    // the squadron's spawn descriptor is the property bag, and the caller hands
    // the same bag to every member record it builds, which is the same sharing
    // 00922DE0's clone produces. Counted so the report can say the step ran.
    std::uint32_t clone_spawn_descriptor_00922de0(std::uint32_t squadron_descriptor) override {
        descriptor_clones_ += 1;
        return squadron_descriptor;
    }

    // 007F494B / 007F4A32 / 007F4A47 -> 004261A0 and 00742A70. The suffix comes
    // from `squadron_plane_name_suffix_007f4926`, which the driver has already
    // built; the concatenation with the squadron's own name at +154h is here.
    void set_plane_name(std::uint32_t plane, const char* suffix, std::int32_t,
                        bool) override {
        PlaneSquadronMemberPlan* member = at(plane);
        if (member == nullptr || suffix == nullptr) return;
        member->name = request_.squadron_name + suffix;
    }

    int base_attach_calls() const noexcept { return base_attach_calls_; }
    int descriptor_clones() const noexcept { return descriptor_clones_; }

   private:
    PlaneSquadronMemberPlan* at(std::uint32_t handle) noexcept {
        if (handle == 0u || handle > plan_.members.size()) return nullptr;
        return &plan_.members[static_cast<std::size_t>(handle) - 1u];
    }

    const PlaneSquadronSpawnRequest& request_;
    PlaneSquadronSpawnPlan& plan_;
    int base_attach_calls_{0};
    int descriptor_clones_{0};
};

}  // namespace

PlaneSquadronSpawnPlan plane_squadron_plan_members_007f4580(
    const PlaneSquadronSpawnRequest& request) {
    PlaneSquadronSpawnPlan plan;
    MemberPlanSpawnHost host(request, plan);

    // The array is five slots and 007F4B55 has no bound test; the rule's
    // `steps` buffer is sized to the array for the same reason.
    SquadronSpawnStep steps[kPlaneSquadronMaxWings]{};
    const PlaneSquadronSpawnResult result = plane_squadron_spawn_planes_007f4580(
        request.kind, 0u, 0u, 0u, host, steps,
        static_cast<std::int32_t>(kPlaneSquadronMaxWings));

    plan.wing_count = result.wing_count;
    plan.behaviour = result.behaviour;
    plan.dirty = result.dirty;

    // The tail, run through the reconstructed rule rather than by counting here,
    // so the five-slot refusal is the one place it lives.
    PlaneSquadronEntity entity;
    entity.wing_count = result.wing_count;
    for (std::size_t i = 0; i < plan.members.size(); ++i) {
        PlaneSquadronMemberPlan& member = plan.members[i];
        // A non-null pointer that is distinct per member is all the rule needs;
        // this process has no plane object yet.
        void* handle = reinterpret_cast<void*>(static_cast<std::uintptr_t>(i + 1u));
        int spawn_index = 0;
        if (!plane_squadron_attach_plane_007f4b43(entity, handle, &spawn_index)) {
            plan.refused_overflow = true;
            break;
        }
        member.spawn_index = spawn_index;   // 007F4B43
        member.array_slot = spawn_index;    // 007F4B55 indexes by the same value
    }
    // A refused wing leaves a plan entry with no slot, so drop the tail the
    // array could not hold rather than carrying a member with no place in it.
    if (plan.refused_overflow) {
        plan.members.resize(static_cast<std::size_t>(entity.live_count));
    }
    plan.plane_count = entity.live_count;   // +3CCh

    // 007F4B55's own witness: the rule and the driver must agree on how many
    // wings were placed.
    if (plan.plane_count != result.plane_count && !plan.refused_overflow) {
        plan.refused_overflow = true;
    }
    return plan;
}

std::int32_t PlaneSquadronHostRecord::live_count() const noexcept {
    // +3CCh counts the planes the array actually holds. A wing whose record
    // never became a unit is not one of them, so the slot is kept (to keep the
    // spawn stamps aligned) but not counted.
    std::int32_t live = 0;
    for (std::size_t unit : member_units) {
        if (unit != kPlaneSquadronNoUnit) ++live;
    }
    return live;
}

std::size_t PlaneSquadronHostRecord::flight_leader() const noexcept {
    // 007EDA91 MOV ESI,[ECX+3D0h]: slot 0, whatever the count.
    for (std::size_t unit : member_units) {
        if (unit != kPlaneSquadronNoUnit) return unit;
    }
    return kPlaneSquadronNoUnit;
}

bool PlaneSquadronHostRecord::remove_member_unit_007f3970(std::size_t unit,
                                                          const std::string& member_name) {
    if (unit == kPlaneSquadronNoUnit) return false;
    std::size_t live_seat = 0;
    for (std::size_t slot = 0; slot < member_units.size(); ++slot) {
        const bool held = member_units[slot] == unit;
        const bool cleared = member_units[slot] == kPlaneSquadronNoUnit &&
            slot < member_names.size() && member_names[slot] == member_name;
        if (!held && !cleared) {
            if (member_units[slot] != kPlaneSquadronNoUnit) ++live_seat;
            continue;
        }
        const auto at = static_cast<std::ptrdiff_t>(slot);
        member_units.erase(member_units.begin() + at);
        departed_units.push_back(unit);
        departed_index_9d8.push_back(static_cast<std::int32_t>(live_seat));
        if (slot < member_names.size()) member_names.erase(member_names.begin() + at);
        if (slot < member_spawn_index.size()) {
            member_spawn_index.erase(member_spawn_index.begin() + at);
        }
        if (held) {
            // Not yet compacted elsewhere: the station flags are keyed by live
            // seat, and 007F3A11 -> 007ED260 re-deals the formation indices.
            if (live_seat < member_station_applied.size()) {
                member_station_applied.erase(member_station_applied.begin()
                    + static_cast<std::ptrdiff_t>(live_seat));
            }
            formation_indices_assigned = false;
        }
        return true;
    }
    return false;
}

void PlaneSquadronRegistry::clear() noexcept { records_.clear(); }

PlaneSquadronHostRecord& PlaneSquadronRegistry::add(const std::string& name) {
    if (PlaneSquadronHostRecord* existing = find(name)) return *existing;
    records_.emplace_back();
    records_.back().name = name;
    return records_.back();
}

PlaneSquadronHostRecord* PlaneSquadronRegistry::find(const std::string& name) noexcept {
    for (PlaneSquadronHostRecord& record : records_) {
        if (record.name == name) return &record;
    }
    return nullptr;
}

const PlaneSquadronHostRecord* PlaneSquadronRegistry::find(
    const std::string& name) const noexcept {
    for (const PlaneSquadronHostRecord& record : records_) {
        if (record.name == name) return &record;
    }
    return nullptr;
}

PlaneSquadronHostRecord* PlaneSquadronRegistry::find_by_member_name(
    const std::string& member) noexcept {
    for (PlaneSquadronHostRecord& record : records_) {
        if (std::find(record.member_names.begin(), record.member_names.end(), member)
            != record.member_names.end()) {
            return &record;
        }
    }
    return nullptr;
}

PlaneSquadronHostRecord* PlaneSquadronRegistry::find_by_member_unit(
    std::size_t unit) noexcept {
    if (unit == kPlaneSquadronNoUnit) return nullptr;
    for (PlaneSquadronHostRecord& record : records_) {
        if (std::find(record.member_units.begin(), record.member_units.end(), unit)
            != record.member_units.end()) {
            return &record;
        }
    }
    return nullptr;
}

const PlaneSquadronHostRecord* PlaneSquadronRegistry::find_by_member_unit(
    std::size_t unit) const noexcept {
    if (unit == kPlaneSquadronNoUnit) return nullptr;
    for (const PlaneSquadronHostRecord& record : records_) {
        if (std::find(record.member_units.begin(), record.member_units.end(), unit)
            != record.member_units.end()) {
            return &record;
        }
    }
    return nullptr;
}

const PlaneSquadronHostRecord* PlaneSquadronRegistry::find_by_member_or_departed_unit(
    std::size_t unit) const noexcept {
    if (const PlaneSquadronHostRecord* r = find_by_member_unit(unit)) return r;
    if (unit == kPlaneSquadronNoUnit) return nullptr;
    for (const PlaneSquadronHostRecord& record : records_) {
        if (std::find(record.departed_units.begin(), record.departed_units.end(), unit)
            != record.departed_units.end()) {
            return &record;
        }
    }
    return nullptr;
}

std::int32_t PlaneSquadronRegistry::spawn_index_of_unit(std::size_t unit) const noexcept {
    if (unit == kPlaneSquadronNoUnit) return -1;
    for (const PlaneSquadronHostRecord& record : records_) {
        for (std::size_t i = 0; i < record.member_units.size(); ++i) {
            if (record.member_units[i] != unit) continue;
            // 007F4B43 stamps the pre-append count and 007F3970's compaction
            // never rewrites it, so the stamp is read from its own vector and
            // not from the live array position.
            if (i < record.member_spawn_index.size()) {
                return record.member_spawn_index[i];
            }
            return static_cast<std::int32_t>(i);
        }
    }
    return -1;
}

std::int32_t PlaneSquadronRegistry::total_planned_members() const noexcept {
    std::int32_t total = 0;
    for (const PlaneSquadronHostRecord& record : records_) {
        total += static_cast<std::int32_t>(record.member_names.size());
    }
    return total;
}

std::int32_t PlaneSquadronRegistry::total_members() const noexcept {
    std::int32_t total = 0;
    for (const PlaneSquadronHostRecord& record : records_) {
        total += record.live_count();
    }
    return total;
}

PlaneSquadronRegistry& plane_squadron_registry() {
    static PlaneSquadronRegistry registry;
    return registry;
}

void plane_squadron_request_leader_promotion_007eee50(
    PlaneSquadronEntity& squadron, const void* candidate,
    PlaneSquadronLandingHookHost& host) {
    // 007EEE71/007EEE7E: neither one plane nor the current leader can promote.
    if (squadron.live_count < 2 || squadron.members[0] == candidate) return;
    // 007EEE8A..007EEE9B: only a PRESENT game in mode 2 suppresses the request.
    if (host.current_game_present_00e188a8() && host.session_mode_1fe4() == 2) return;
    // 007EEE9E..007EEED3 searches backwards and never accepts slot zero.
    for (int index = squadron.live_count - 1; index > 0; --index) {
        if (squadron.members[static_cast<std::size_t>(index)] != candidate) continue;
        // 007EEEDE base BEh construction, fields at 007EEEE3..007EEEFF,
        // 007EEF16 route flags 5/status null. Promotion occurs on later receipt,
        // not at this request or at the land task's pre-destructor hook.
        host.construct_and_route_promotion_be(squadron, index);
        return;
    }
}

void plane_squadron_end_landing_007efb60(PlaneSquadronLandingHookView& view,
                                       PlaneSquadronLandingHookHost& host) {
    PlaneSquadronEntity& squadron = view.squadron;
    // CMP count at 007EFB62 precedes both MOV byte clears at 007EFB69/007EFB6F.
    const int count = squadron.live_count;
    view.field_3b0 = 0;
    view.field_3b8 = 0;
    if (count <= 1) return;
    const void* const leader = squadron.members[0];                 // 007EFB77
    if (!host.plane_landed_904(leader)) return;                     // 007EFB7D
    if (host.plane_control_mode_900(leader) != 5) return;            // 007EFB85
    plane_squadron_request_leader_promotion_007eee50(
        squadron, squadron.members[1], host);                      // 007EFB95
}

void land_task_retained_hook_009b33f0(const LandTaskRetainedHookView& task,
                                     PlaneSquadronLandingHookHost& host) {
    // 009B33F3, 009B340B and 009B3422 reload the OLD task's +404h. Preserve
    // both command probes; do not replace them with a current plane lookup.
    const void* const first_receiver = task.squadron_404;   // 009B33F3
    if (first_receiver != nullptr
        && command_queue_current_command(
            host.command_queue_348(host.squadron_entity(first_receiver))) != 0u) {
        const void* const second_receiver = task.squadron_404; // 009B340B
        if (command_queue_current_command(
                host.command_queue_348(host.squadron_entity(second_receiver))) == 0x00e08fa0u) {
            return;                                                // 009B3420
        }
    }
    const void* const final_receiver = task.squadron_404;   // 009B3422
    if (final_receiver != nullptr) {
        auto final_view = host.landing_hook_view(final_receiver);
        plane_squadron_end_landing_007efb60(final_view, host); // 009B342D
    }
}

namespace {
// Preserve COMISS zero,[field]/JBE, including unordered and ambient MXCSR
// denormal/exception behavior. By-value float comparisons would lose this.
__declspec(noinline) bool cruise_gate_ordered_negative(const float* field) {
    unsigned char advances;
    __asm {
        mov eax, field
        xorps xmm0, xmm0
        comiss xmm0, dword ptr [eax]
        seta advances
    }
    return advances != 0;
}

// 009B3C9D/A3/A7 and 009B3CD3/D9/DD. FLD quiets a signaling NaN under
// masked exceptions; the dirty-byte publication occurs before FSTP.
__declspec(noinline) void cruise_load_publish_store(
    const float* tuning, std::uint8_t* dirty, float* destination) {
    __asm {
        mov eax, tuning
        mov ecx, dirty
        mov edx, destination
        fld dword ptr [eax]
        mov byte ptr [ecx], 1
        fstp dword ptr [edx]
    }
}
} // namespace

void pilot_bot_task_update_cruise_profile_base_0099b660() noexcept {
    // Actual empty native RET, not an unresolved provider or substitute stub.
}

void land_task_update_cruise_profile_009b3c60(
    const LandTaskCruiseProfileView& task, LandTaskCruiseProfileHost& host) {
    pilot_bot_task_update_cruise_profile_base_0099b660();       // 009B3C63
    if (!host.plane_is_leader_007b8ad0(task.plane_3fc)) return; // 009B3C68..75

    const LandCruiseTuningView first_tuning = host.game_tuning_0042e740();
    const void* const first_receiver = task.squadron_404; // AFTER C77
    auto first = host.cruise_profile_view(first_receiver);
    if (first.field_38d == 0) {
        if (cruise_gate_ordered_negative(&first.field_380)
            && first.field_3a9 == 0) {
            cruise_load_publish_store(&first_tuning.field_514,
                                      &first.field_3ad, &first.field_394);
        }
        first.field_3a9 = 0;                                 // 009B3CAA
    }

    // Always called for a leader, including a blocked first channel. Both the
    // returned tuning and task's cached squadron may differ from the first.
    const LandCruiseTuningView second_tuning = host.game_tuning_0042e740();
    const void* const second_receiver = task.squadron_404; // AFTER CAE
    auto second = host.cruise_profile_view(second_receiver);
    if (second.field_38c == 0) {
        if (cruise_gate_ordered_negative(&second.field_37c)
            && second.field_3aa == 0) {
            cruise_load_publish_store(&second_tuning.field_514,
                                      &second.field_3ad, &second.field_398);
        }
        second.field_3aa = 0;                                // 009B3CE0
    }
}

namespace {
static_assert(sizeof(SceneCommandTarget) == 0x18);
static_assert(sizeof(void*) == 4);
static_assert(sizeof(float) == 4);
static_assert(offsetof(SceneCommandTarget, kind) == 0);
static_assert(offsetof(SceneCommandTarget, position_valid) == 1);
static_assert(offsetof(SceneCommandTarget, object_id) == 2);
static_assert(offsetof(SceneCommandTarget, object) == 4);
static_assert(offsetof(SceneCommandTarget, position) == 8);
static_assert(offsetof(SceneCommandTarget, position) + sizeof(float) == 0x0c);
static_assert(offsetof(SceneCommandTarget, position) + 2 * sizeof(float) == 0x10);
static_assert(offsetof(SceneCommandTarget, trailing) == 0x14);

// 007EEDCB..007EEDFA, after the actual0071EB60 provider returns. Retain
// WORD0/WORD2/DWORD4 and each separate memory FLD/FSTP pair, in original order.
__declspec(noinline) void copy_current_descriptor_fields(
    const SceneCommandTarget* source, SceneCommandTarget* destination) {
    __asm {
        mov eax, source
        mov ecx, destination
        movzx edx, word ptr [eax]
        mov word ptr [ecx], dx
        movzx edx, word ptr [eax + 2]
        mov word ptr [ecx + 2], dx
        mov edx, dword ptr [eax + 4]
        mov dword ptr [ecx + 4], edx
        fld dword ptr [eax + 8]
        fstp dword ptr [ecx + 8]
        fld dword ptr [eax + 0ch]
        fstp dword ptr [ecx + 0ch]
        fld dword ptr [eax + 10h]
        fstp dword ptr [ecx + 10h]
        fld dword ptr [eax + 14h]
        mov eax, ecx
        fstp dword ptr [ecx + 14h]
    }
}
} // namespace

SceneCommandTarget* plane_squadron_copy_current_command_descriptor_007eedc0(
    const PlaneSquadronEntity& squadron, SceneCommandTarget& output,
    LandTaskCommandValidityHost& host) {
    SceneCommandTarget& source = host.active_command_descriptor_0071eb60(squadron);
    copy_current_descriptor_fields(&source, &output);
    return &output;
}

void land_approach_validate_site_009b34d0(
    const LandTaskCommandValidityView& task, LandTaskCommandValidityHost& host) {
    const void* const first_receiver = task.squadron_404;
    if (first_receiver != nullptr
        && command_queue_current_command(
            host.command_queue_348(host.squadron_entity(first_receiver))) != 0u
        && command_queue_current_command(
            host.command_queue_348(host.squadron_entity(task.squadron_404))) == 0x00e08fa0u) {
        SceneCommandTarget output;
        SceneCommandTarget* const returned =
            plane_squadron_copy_current_command_descriptor_007eedc0(
                host.squadron_entity(task.squadron_404), output, host); // 009B3512
        const void* const resolved = host.resolve_command_target_00521ea0(*returned);
        if (resolved == task.target_428) {                   // AFTER resolver
            const void* const block = task.block_424;        // 009B3520
            if (block != nullptr) {
                const void* const owner = host.block_owner_7c(block);
                if (owner != nullptr && host.owner_field_5d(owner) == 0) {
                    const void* const admission_identity = task.squadron_404; // 3533
                    const PlaneSquadronEntity* const admission_receiver =
                        admission_identity != nullptr
                            ? &host.squadron_entity(admission_identity) : nullptr;
                    if (host.squadron_not_excluded_006c4790(
                            block, admission_receiver)) return;
                }
            }
            task.block_424 = nullptr;                       // 009B3540
        }
    }
    task.block_424 = nullptr;                               // 009B3543
    task.target_428 = nullptr;                              // 009B3546
    task.field_42c = nullptr;                               // 009B3549
}

std::uint32_t land_task_is_command_current_009b3560(
    const LandTaskCommandValidityView& task, LandTaskCommandValidityHost& host) {
    const void* const first_receiver = task.squadron_404;
    if (first_receiver == nullptr
        || command_queue_current_command(
            host.command_queue_348(host.squadron_entity(first_receiver))) == 0u) return 2u;
    // A changed second token (including NULL) means0, not the first-probe2.
    if (command_queue_current_command(
            host.command_queue_348(host.squadron_entity(task.squadron_404))) != 0x00e08fa0u
        || task.block_424 == nullptr) return 0u;
    SceneCommandTarget output;
    SceneCommandTarget* const returned =
        plane_squadron_copy_current_command_descriptor_007eedc0(
            host.squadron_entity(task.squadron_404), output, host); // 009B35B1
    const void* const resolved = host.resolve_command_target_00521ea0(*returned);
    return resolved == task.target_428
        ? 1u : 0u;                                         // fresh428 at35BA
}

void plane_squadron_promote_and_reindex_007ed610(
    PlaneSquadronEntity& squadron, int index, PlaneSquadronPromotionReceiptHost& host) {
    // The existing helper covers007ED614..007ED63F only. Its count/slot guard
    // agrees with the native bounds in the admitted five-member domain.
    if (!plane_squadron_promote_flight_leader_007ed610(squadron, index)) return;
    host.assign_formation_indices_007ed260(squadron); //007ED645, after rotation
}

bool plane_squadron_receive_leader_promotion_be_007f0030(
    PlaneSquadronEntity& squadron, const NativeSessionMessageBE& message,
    PlaneSquadronPromotionReceiptHost& host) {
    //007F0077 loads msg+1Ch,007F007B calls007ED610,007F0083 sets AL=1.
    plane_squadron_promote_and_reindex_007ed610(squadron, message.member_index_1c, host);
    return true;
}

}  // namespace bsp

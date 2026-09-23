#include "bsp/plane_squadron_host.hpp"

#include <algorithm>

#include "bsp/plane_squadron_entity.hpp"

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

}  // namespace bsp

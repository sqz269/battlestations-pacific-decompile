#pragma once
// The process side of the squadron: what turns 007F4580's mode-1 per-wing loop
// into member plane units, and the registry that ties each member back to the
// squadron that spawned it.
//
// Addresses: 007F4580 (the spawn loop), 007F4B43 / 007F4B49 / 007F4B55 /
// 007F4B60 / 007F4B6E (its tail), 004F0AD0 (the scene creator a PlaneSquadronGen
// row reaches), 006C5050 (the air-ops launch bag that reaches the same creator).
//
// The RULES are not restated here. `plane_squadron_spawn_planes_007f4580` in
// include/bsp/plane_squadron.hpp is the reconstruction of the loop and
// `plane_squadron_attach_plane_007f4b43` in include/bsp/plane_squadron_entity.hpp
// is the reconstruction of the tail; this header adds only the binding that
// drives the first one in a process whose "plane instance" is a scene entity
// record, plus the registry the other hosts read.
//
// docs/PLANE_SQUADRON_HOST.md carries the design and the runs;
// docs/PLANE_SQUADRON.md section 4 carries the listing evidence.

#include <cstddef>
#include <functional>
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/plane_squadron.hpp"

namespace bsp {

// A member that has no unit yet, and the value `squadron_unit` carries until
// create_units has run over the records the plan produced.
inline constexpr std::size_t kPlaneSquadronNoUnit = static_cast<std::size_t>(-1);

// The three property-bag keys 007F4580 mode 1 reads besides `Type` (00CE4780,
// which the scene pass already resolves onto its record). Each is verified from
// the bytes at the literal's address, not from a name.
inline constexpr const char* kSceneUnitWingCountKey = "WingCount";         // 00CF8840, 007F4735
inline constexpr const char* kSceneUnitPlaneParentIdKey = "PlaneParentID"; // 00CFACC8, 007F4794
inline constexpr const char* kSceneUnitBehaviourKey = "Behaviour";         // 00CFACBC, 007F47C2

// One wing the loop asks for. `name` is what 007F4926 / 007F49EF build, which is
// the only part of a native plane instance this process can make before
// create_units runs.
struct PlaneSquadronMemberPlan {
    std::string name;
    std::int32_t spawn_index{0};    // plane+9D8h, stamped at 007F4B43
    std::int32_t array_slot{0};     // the slot 007F4B55 writes
    bool under_parent{false};       // 007F4817 TEST EBP,EBP
    bool use_squadron_local{false}; // 007F48C9 LEA ECX,[ESI+74h]
};

// Everything 007F4580 mode 1 reads before the loop. The four property-bag keys
// are the ones 008F2260 is called with at 007F4724, 007F4742 / 007F4759,
// 007F479D and 007F47C7; `parent_entity` is what 00521E30 answered, which is 0
// when the handle is stale and takes the no-parent placement arm.
struct PlaneSquadronSpawnRequest {
    std::string squadron_name;
    SquadronSpawnKind kind{SquadronSpawnKind::kPropertyBag};
    std::int32_t type_class_id{0};
    bool wing_count_present{false};
    std::int32_t wing_count_raw{0};
    bool parent_present{false};
    std::int32_t parent_id{0};
    std::uint32_t parent_entity{0};
    bool behaviour_present{false};
    std::int32_t behaviour{0};
};

// The loop's result, in the fields it writes on the squadron.
struct PlaneSquadronSpawnPlan {
    std::int32_t wing_count{0};    // +3C8h, 007F4778
    std::int32_t plane_count{0};   // +3CCh after the loop, 007F4B60
    std::int32_t behaviour{-1};    // +364h, 007F47D9 or the constructor's seed
    bool dirty{false};             // +3ECh, 007F4B6E
    // 007F4B55 has no bound test and would write +3E4h, +3E8h and +3ECh for a
    // sixth wing. `plane_squadron_attach_plane_007f4b43` refuses instead, and
    // this records that the refusal happened. The authored PlaneWingCount enum
    // admits only 1..5, so a shipped scene never reaches it.
    bool refused_overflow{false};
    std::vector<PlaneSquadronMemberPlan> members;
};

// Drives `plane_squadron_spawn_planes_007f4580` with a binding whose "plane
// instance" is a member plan. Kinds 2 and 3 return a plan with no members, which
// is what the native's two non-bag arms do.
PlaneSquadronSpawnPlan plane_squadron_plan_members_007f4580(
    const PlaneSquadronSpawnRequest& request);

// One squadron as this process holds it: the entity the script names, and the
// member array at +3D0h under the count at +3CCh.
struct PlaneSquadronHostRecord {
    std::string name;
    std::int32_t wing_count{0};        // +3C8h
    std::int32_t behaviour{-1};        // +364h
    std::int32_t type_class_id{0};     // the `Type` the wing was built from
    int party{-1};
    bool from_air_ops_launch{false};   // 006C5050 rather than a scene row
    // +378h, the force flag 007EEF62 tests before it consults 007B8AD0.
    // 007F2D1E seeds it SET, so a squadron that has not been through 007ED3C0
    // raises a release order for every member unconditionally. `release_orders_
    // clear_007ed3c0` is reconstructed and nothing in this host calls it,
    // because its caller is unlocated, so the flag stays set here. Labelled.
    bool force_flag_378{true};
    // The squadron entity itself. This process fuses no unit into it: the
    // squadron is its own slot, and every member below is a separate one.
    std::size_t squadron_unit{kPlaneSquadronNoUnit};
    // +3D0h in array order. `member_names` is filled by the plan and
    // `member_units` by the units host once create_units has run.
    // `member_spawn_index` is plane+9D8h, stamped at 007F4B43 and never
    // rewritten, so it is carried separately from the live array position: a
    // squadron that has lost a plane has members whose stamp is above their slot.
    std::vector<std::string> member_names;
    std::vector<std::size_t> member_units;
    std::vector<std::int32_t> member_spawn_index;

    // +3CCh: the live member count, which is what 007EEF54 and 007EE7F0 test.
    std::int32_t live_count() const noexcept;
    // +3D0h, the flight leader. 007EDA91 reads slot 0 whatever the count.
    std::size_t flight_leader() const noexcept;
};

// Process-wide, like `air_ops_decks()`: the scene pass, the air-ops launch seam,
// the units host and the AI coordinator all need the same table and none of them
// owns the others.
class PlaneSquadronRegistry {
   public:
    void clear() noexcept;
    // Adds, or returns the existing record of that name.
    PlaneSquadronHostRecord& add(const std::string& name);
    PlaneSquadronHostRecord* find(const std::string& name) noexcept;
    const PlaneSquadronHostRecord* find(const std::string& name) const noexcept;
    // The squadron a member plane belongs to, by the member's name. This is the
    // host's stand-in for reading plane+9D4h.
    PlaneSquadronHostRecord* find_by_member_name(const std::string& member) noexcept;
    // Fill `member_units` from `member_names` once the units exist. This is the
    // resolver the comment below refers to and that nothing implemented, which
    // is why find_by_member_unit answered nothing for a SCENE-ROW squadron:
    // game_hosts_script_orders.cpp fills member_units for an air-ops launch,
    // but the scene path in game_hosts_scene_contents.cpp clears the array and
    // then fills only member_names and member_spawn_index, queueing the wings
    // into pending_squadron_members for create_units to make later. Measured
    // before this existed: USN04 answered 7 AI squadrons over 15 planes, its
    // four launches grouped and its one scene row seeding three of its own.
    //
    // `lookup` answers a unit index for a member name, or kPlaneSquadronNoUnit.
    // Idempotent, and it never overwrites a slot already filled, so calling it
    // after every unit-creating pass is safe. Returns the number of slots it
    // filled. docs/AI_TARGET_WEIGHT_TERMS.md.
    std::size_t resolve_member_units(
        const std::function<std::size_t(const std::string&)>& lookup);
    // The same by unit index, valid once `resolve_member_units` has run.
    PlaneSquadronHostRecord* find_by_member_unit(std::size_t unit) noexcept;
    const PlaneSquadronHostRecord* find_by_member_unit(std::size_t unit) const noexcept;
    // plane+9D8h for a member unit, or -1 when the unit is in no squadron.
    std::int32_t spawn_index_of_unit(std::size_t unit) const noexcept;

    std::size_t size() const noexcept { return records_.size(); }
    std::vector<PlaneSquadronHostRecord>& records() noexcept { return records_; }
    const std::vector<PlaneSquadronHostRecord>& records() const noexcept { return records_; }

    // Totals for the census a run prints. `total_planned_members` counts the
    // names 007F4580's loop produced, which is what exists before create_units
    // has run; `total_members` counts the units it resolved to.
    std::int32_t total_planned_members() const noexcept;
    std::int32_t total_members() const noexcept;

   private:
    std::vector<PlaneSquadronHostRecord> records_;
};

PlaneSquadronRegistry& plane_squadron_registry();

}  // namespace bsp

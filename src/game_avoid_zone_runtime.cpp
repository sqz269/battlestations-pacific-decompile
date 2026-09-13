#include "bsp/game_avoid_zone_runtime.hpp"
#include "bsp/avoid_zone_clearance.hpp"
#include "bsp/avoid_zone_arc.hpp"
#include "bsp/avoid_zone_clearance_distance.hpp"
#include "bsp/avoid_zone_manager_queries.hpp"
#include "bsp/avoid_zone_query_binding.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_scene_contents.hpp"
#include "bsp/native_renderer_worker_lifetime.hpp"
#include "bsp/native_tracked_critical_section_release.hpp"
#include "bsp/pose_refresh.hpp"
#include "bsp/scene_landscape_class.hpp"
#include "bsp/ship_ai_path_follower.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <map>
#include <stdexcept>

namespace bsp::game {
namespace {
void* allocate_record(void*, std::uint32_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
void* allocate_array(void*, std::uint32_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, bytes, bytes});
}
void free_storage(void*, void* value) noexcept { singleton_lifetime_free(value); }
const AvoidZoneAllocationAccess allocation{nullptr, &allocate_record, &allocate_array,
    &free_storage, &free_storage};

struct ZoneOwner {
    AvoidZoneNativeStorage native{};
    bool constructed{};
    AvoidZoneManagerZone identity;
    ~ZoneOwner() { if (constructed) avoid_zone_release_owned_storage(native, allocation); }
};

// The retained world matrix is the result of the scene reader's authored
// hierarchy composition. Use its valid-cache path, not a fabricated identity
// transform. The hierarchy identity queried by00923810 remains independent.
class ScenePath final : public AvoidZoneScenePathAccess {
public:
    ScenePath(const GameSceneEntityRecord& record, const GameSceneEntityRecord* parent)
        : parent_(parent), pose_{PoseRefreshParentSlot(unused_pose_parent_),
              local_, valid_, world_, derived_} {
        std::copy_n(record.local, 16, local_.begin());
        std::copy_n(record.world, 16, world_.begin());
        for (const auto& point : record.path_points_local) pointers_.push_back(&point);
        slots_ = {pointers_.data(), pointers_.data() + pointers_.size()};
    }
    const AvoidZoneScenePointSlots& point_slots() const override { return slots_; }
    PoseRefreshView& scene_pose() override { return pose_; }
    void invalid_parameter_00bf6713() override { _invalid_parameter_noinfo(); }
    void* call_00923810(std::int32_t generations) override {
        if (generations != 1) throw std::logic_error("Unsupported scene ancestor depth");
        return const_cast<GameSceneEntityRecord*>(parent_);
    }
    std::uint8_t call_parent_vtable_5c(void* parent, std::uint32_t code) override {
        if (parent != parent_ || !parent_ || parent_->class_id != 0x44)
            throw std::logic_error("Unreconstructed avoidance parent class predicate");
        return static_cast<std::uint8_t>(scene_landscape_is_kind_of_004f1360(
            parent_->class_id, static_cast<std::int32_t>(code)));
    }
private:
    const GameSceneEntityRecord* parent_;
    std::vector<const AvoidZoneScenePoint*> pointers_;
    AvoidZoneScenePointSlots slots_{};
    CameraMatrix local_, world_;
    PoseRefreshView* unused_pose_parent_{}; // never read while valid_ is nonzero
    std::uint8_t valid_{1}, derived_{};
    PoseRefreshView pose_;
};
} // namespace

struct GameAvoidZoneRuntime::Impl final : AvoidZoneClearanceAccess, ShipAiAvoidZoneSearchAccess {
    Impl(GameHostLog& output, const CameraAxesCrtAccess& math) : log(output), crt(math) {}
    ~Impl() { release_native_tracked_critical_section_0041cc80(&section); }
    GameHostLog& log;
    const CameraAxesCrtAccess& crt;
    TrackedCriticalSection* section{};
    bool loaded{};
    WorldMapBounds bounds{};
    AvoidZoneTable table;
    std::vector<std::unique_ptr<ZoneOwner>> owners;
    struct Group {
        std::vector<const AvoidZoneNativeStorage*> native;
        std::vector<std::uint32_t> handles;
        std::unique_ptr<AvoidZoneBoundaryQueries> boundary;
    };
    std::vector<Group> groups;
    std::map<std::uint32_t, ShipAiPathLateralRecord*> corners;
    std::map<std::uint32_t, ShipAiPathLateralAnchor> anchors;
    void require_ready() const {
        if (!loaded) throw std::logic_error("Avoid-zone geometry has not been loaded");
    }
    ZoneOwner& zone(std::uint32_t token) const {
        require_ready();
        if (token == 0 || token > owners.size()) throw std::out_of_range("Avoid-zone handle");
        return *owners[token - 1];
    }
    const AvoidZonePolygon& polygon(const ZoneOwner& zone) const {
        return table.groups.at(zone.identity.group_index).zones.at(zone.identity.zone_index);
    }
    TrackedCriticalSection* manager_critical_section_004218e0() override { return section; }
    const AvoidZoneTable& manager_004218e0() override {
        require_ready();
        return table;
    }
    AvoidZoneClearanceGroupView native_group(const AvoidZoneLayerGroup& semantic) override {
        // Preserve the identity and zone order of the selected semantic group.
        for (std::size_t index = 0; index < table.groups.size(); ++index) {
            if (&semantic != &table.groups[index]) continue;
            const auto& group = groups.at(index);
            return {group.native.data(), static_cast<std::uint32_t>(group.native.size())};
        }
        throw std::logic_error("Avoid-zone search group belongs to another manager");
    }
    const AvoidZoneAllocationAccess& allocation() const noexcept override {
        return bsp::game::allocation;
    }
    AvoidZoneClearanceGroupView selected_group_004120d0(std::uint32_t layer) override {
        const auto index = avoid_zone_group_for_layer_004120d0(table,
            static_cast<std::int32_t>(layer));
        const auto& group = groups.at(index);
        return {group.native.data(), static_cast<std::uint32_t>(group.native.size())};
    }
};

GameAvoidZoneRuntime::GameAvoidZoneRuntime(GameHostLog& log, const CameraAxesCrtAccess& crt)
    : impl_(std::make_unique<Impl>(log, crt)) {}
GameAvoidZoneRuntime::~GameAvoidZoneRuntime() = default;
bool GameAvoidZoneRuntime::ready() const noexcept { return impl_->loaded; }

void GameAvoidZoneRuntime::rebuild(const GameSceneContentsHost& scene,
    std::int32_t mode, std::uint8_t forced, std::int32_t session) {
    auto& self = *impl_;
    if (self.loaded || !self.owners.empty())
        throw std::logic_error("Avoid-zone rebuild requires quiescent fresh ownership");
    const ScenePropertyBlock* map = nullptr;
    for (const auto& block : scene.root_properties().blocks)
        if (_stricmp(block.first.c_str(), "Map") == 0) { map = &block.second; break; }
    WorldMapSettings settings;
    if (!map || !read_world_map_settings_004e6c00(*map, settings))
        throw std::runtime_error("Scene Map lacks native world-bound inputs");
    self.bounds = select_world_map_bounds_004d5ede(settings, mode, forced, session);
    self.section = create_native_tracked_critical_section_00bd1860();
    std::map<std::size_t, const GameSceneEntityRecord*> by_id;
    for (const auto& record : scene.entities()) by_id.emplace(record.scene_id, &record);
    std::size_t source_points = 0, surviving = 0, associated = 0;
    for (const auto& record : scene.entities()) {
        if (!record.generated || record.name.compare(0, 9, "AvoidZone") != 0) continue;
        //00424DBE creates a named group before0041D1E0 accepts/rejects its entity.
        const auto layer = avoid_zone_layer_from_entity_name(record.name);
        avoid_zone_group_find_or_create_00417ca0(self.table, layer);
        //0041D206 reads the original entity+54h Party, not path source kind.
        if (record.party != 2) continue;
        if (record.class_id != 0x47 || !record.path_points_retained || record.path_points_local.empty())
            throw std::runtime_error("Unsupported named avoidance path: " + record.name);
        const GameSceneEntityRecord* parent = nullptr;
        if (record.parent_scene_id != 0) {
            parent = by_id.at(record.parent_scene_id);
            if (parent->class_id != 0x44)
                throw std::runtime_error("Unreconstructed avoidance parent: " + parent->name);
        }
        auto owner = std::make_unique<ZoneOwner>();
        ScenePath path(record, parent);
        avoid_zone_construct_from_scene_path_0041ccd0(owner->native, path,
            static_cast<std::uint32_t>(layer), self.bounds.clip_minimum(),
            self.bounds.clip_maximum(), allocation, self.crt);
        owner->constructed = true;
        source_points += record.path_points_local.size();
        //0041D23E..0041D24B: zero-corner owners are destroyed, not appended.
        if (owner->native.corners.count == 0) {
            self.log.notef("avoid-zone geometry name=%s source=%zu clipped_empty=1",
                record.name.c_str(), record.path_points_local.size());
            continue;
        }
        surviving += static_cast<std::size_t>(owner->native.corners.count);
        associated += owner->native.associated_entity != nullptr ? 1u : 0u;
        self.log.notef("avoid-zone geometry name=%s party=%d layer=%d source=%zu corners=%d parent=%zu associated=%d",
            record.name.c_str(), record.party, layer, record.path_points_local.size(),
            owner->native.corners.count, record.parent_scene_id,
            owner->native.associated_entity != nullptr ? 1 : 0);
        self.owners.push_back(std::move(owner));
    }
    avoid_zone_group_find_or_create_00417ca0(self.table, 0);
    self.groups.resize(self.table.groups.size());
    for (std::size_t i = 0; i < self.owners.size(); ++i) {
        auto& owner = *self.owners[i];
        const auto g = avoid_zone_group_for_layer_004120d0(self.table,
            static_cast<std::int32_t>(owner.native.layer));
        auto& group = self.table.groups.at(g);
        owner.identity = {g, static_cast<std::int32_t>(group.zones.size())};
        group.zones.push_back(avoid_zone_polygon_snapshot(owner.native));
        self.groups[g].native.push_back(&owner.native);
        self.groups[g].handles.push_back(static_cast<std::uint32_t>(i + 1));
        for (std::int32_t c = 0; c < owner.native.corners.count; ++c) {
            auto* ptr = owner.native.corners.records[c];
            self.corners.emplace(static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(ptr)), ptr);
        }
    }
    for (auto& group : self.groups)
        group.boundary = std::make_unique<AvoidZoneBoundaryQueries>(group.native, self.crt);
    self.loaded = true;
    self.log.notef("summary avoid-zone geometry groups=%zu zones=%zu source_points=%zu corners=%zu associated=%zu "
        "bounds_nw=(%.9g,%.9g,%.9g) bounds_se=(%.9g,%.9g,%.9g) native_scene_creators=unresolved draft_layers=unresolved",
        self.groups.size(), self.owners.size(), source_points, surviving, associated,
        self.bounds.north_west[0], self.bounds.north_west[1], self.bounds.north_west[2],
        self.bounds.south_east[0], self.bounds.south_east[1], self.bounds.south_east[2]);
}

std::uint32_t GameAvoidZoneRuntime::manager_handle() const {
    impl_->require_ready(); return 1; // process-local token, never a native address
}
std::uint32_t GameAvoidZoneRuntime::containing(const std::array<float, 2>& p, std::uint32_t layer) const {
    impl_->require_ready();
    const auto hit = avoid_zone_manager_containing_point_00417e40(impl_->table, p,
        static_cast<std::int32_t>(layer));
    return hit ? impl_->groups.at(hit.group_index).handles.at(hit.zone_index) : 0;
}
std::uint32_t GameAvoidZoneRuntime::group_for_layer(std::uint32_t layer) const {
    impl_->require_ready(); return static_cast<std::uint32_t>(
        avoid_zone_group_for_layer_004120d0(impl_->table, static_cast<std::int32_t>(layer)) + 1);
}
std::array<float, 2> GameAvoidZoneRuntime::push_out(std::uint32_t token,
    const std::array<float, 2>& point, float margin) const {
    const auto& zone = impl_->zone(token);
    return avoid_zone_point_offset_00417580(zone.native, impl_->polygon(zone), point, margin);
}
std::array<float, 2> GameAvoidZoneRuntime::nearest(std::uint32_t group,
    const std::array<float, 2>& point, float slack, float push) {
    impl_->require_ready(); return impl_->groups.at(group - 1).boundary->nearest_boundary_point(point, slack, push);
}
bool GameAvoidZoneRuntime::segment(std::uint32_t layer, const std::array<float, 2>& toward,
    const std::array<float, 2>& from, std::uint32_t& zone, std::int32_t& edge) const {
    impl_->require_ready(); AvoidZoneManagerZone hit;
    if (!avoid_zone_manager_segment_hit_00417e90(impl_->table, static_cast<std::int32_t>(layer),
        toward, from, hit, edge)) return false;
    zone = impl_->groups.at(hit.group_index).handles.at(hit.zone_index); return true;
}
bool GameAvoidZoneRuntime::segment_point(std::uint32_t layer, const std::array<float, 2>& toward,
    const std::array<float, 2>& from, std::array<float, 2>& hit) const {
    impl_->require_ready(); return avoid_zone_manager_segment_hit_point_00417ef0(impl_->table,
        static_cast<std::int32_t>(layer), toward, from, hit);
}
AvoidZoneTangentCorners GameAvoidZoneRuntime::detour(std::uint32_t token,
    const std::array<float, 2>& far_point, std::int32_t edge, std::int32_t near_hint,
    std::int32_t side_hint, float margin) const {
    const auto& zone = impl_->zone(token);
    return avoid_zone_tangent_corners_00422500(impl_->table, impl_->polygon(zone),
        far_point, edge, near_hint, side_hint, margin);
}
bool GameAvoidZoneRuntime::outside(const std::array<float, 3>& point) const {
    impl_->require_ready(); return point_outside_world_map_0071c4f0(impl_->bounds, point);
}
std::uint32_t GameAvoidZoneRuntime::corner(std::uint32_t token, std::int32_t index) const {
    auto* value = ship_ai_lateral_record_at_00417610(impl_->zone(token).native.corners, index);
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(value));
}
void GameAvoidZoneRuntime::ensure_clearance(std::uint32_t zone, std::uint32_t corner) {
    auto* record = impl_->corners.at(corner);
    avoid_zone_ensure_corner_clearance_00423190(impl_->zone(zone).native, *record,
        *impl_, allocation, impl_->crt);
}
float GameAvoidZoneRuntime::corner_clearance(std::uint32_t corner) const {
    // 009EE63A reads the actual +20h cache; this accessor does not recompute.
    return impl_->corners.at(corner)->clearance_scale;
}
const ShipAiPathLateralAnchor* GameAvoidZoneRuntime::anchor(std::uint32_t corner) {
    if (corner == 0) return nullptr;
    auto& result = impl_->anchors[corner];
    result = ship_ai_lateral_anchor_projection(*impl_->corners.at(corner));
    return &result;
}
bool GameAvoidZoneRuntime::refresh_search(ShipAiAvoidZoneSearcher& cache,
    ShipAiAvoidZoneSegmentList& list, const ShipAiAvoidZoneQuery& query) {
    impl_->require_ready();
    return ship_ai_avoid_query_refresh_009d7050(cache, list, query, *impl_);
}
void GameAvoidZoneRuntime::clear_search(ShipAiAvoidZoneSegmentList& list) noexcept {
    avoid_zone_selected_segments_clear_004158a0(list.head, allocation);
}
bool GameAvoidZoneRuntime::search_segment(const ShipAiAvoidZoneSegmentList& list,
    const std::array<float, 2>& from, const std::array<float, 2>& toward,
    std::array<float, 2>& hit) const {
    impl_->require_ready();
    return avoid_zone_selected_segments_hit_004158e0(list.head, from, toward, hit);
}
bool GameAvoidZoneRuntime::search_arc(const ShipAiAvoidZoneSegmentList& list,
    const std::array<float, 2>& center, float radius, float start, float& end) const {
    impl_->require_ready();
    return avoid_zone_selected_segments_arc_00415970(list.head, center, radius,
        start, end, impl_->crt) != nullptr;
}
float GameAvoidZoneRuntime::search_clearance(const ShipAiAvoidZoneSegmentList& list,
    const std::array<float, 2>& center, float radius,
    const std::array<float, 2>& normal_a, const std::array<float, 2>& normal_b) const {
    impl_->require_ready();
    return avoid_zone_selected_segments_clearance_00415d70(list.head, center, radius,
        normal_a, normal_b, impl_->crt);
}
const AvoidZoneAllocationAccess& GameAvoidZoneRuntime::allocation_access() const noexcept {
    return allocation;
}
} // namespace bsp::game

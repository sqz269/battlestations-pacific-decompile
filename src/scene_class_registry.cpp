#include "bsp/scene_class_registry.hpp"

#include <utility>

namespace bsp {

namespace {

template <class T, class Read>
T& get_or_create_named(NamedSceneClassRegistry<T>& registry, const std::string& name,
                       const char* table, SceneClassLuaTables& lua, Read&& read) {
    // Native indexes the registry with operator[] first, which leaves a null
    // cell behind when the read below throws; the host inserts only on success.
    const auto found = registry.find(name);
    if (found != registry.end() && found->second) return *found->second;
    auto row = lua.row(table, name);
    auto cls = std::make_unique<T>();
    cls->name = name;
    read(*cls, *row);
    auto& cell = registry[name];
    cell = std::move(cls);
    return *cell;
}

SourcedSceneClass& get_or_create_sourced(SourcedSceneClassRegistry<SourcedSceneClass>& registry,
                                         const void* source, const std::string& name,
                                         std::int32_t index, const char* table,
                                         SceneClassLuaTables& lua, const SceneClassHooks& hooks) {
    auto& cell = registry[source];
    if (cell) return *cell;
    cell = std::make_unique<SourcedSceneClass>();
    cell->source = source;
    cell->name = name;
    // Published before the read, as native does (0047B438 / 00473C38, before 00B67980).
    auto row = lua.row(table, index);
    if (hooks.read_named_class_base) hooks.read_named_class_base(*row);
    return *cell;
}

} // namespace

void read_anim_class_00470820(AnimClass& cls, SceneClassLuaRow& row, const SceneClassHooks& hooks) {
    if (hooks.read_named_class_base) hooks.read_named_class_base(row);  // 00470841
    cls.radius = row.number("Radius");                                  // 00470861..00470866
}

void read_cloud_class_004774a0(CloudClass& cls, SceneClassLuaRow& row, const SceneClassHooks& hooks) {
    cls.cloud_type = row.integer_or("CloudType", 0);  // 004774C0..004774EB
    if (cls.cloud_type == 0) {
        cls.texture = row.string("Texture").value_or(std::string{});        // 00477501..00477556
        const float multiplier = row.number_or("ParticleMultiplier", 1.0f);  // 0047756E..00477594
        cls.mesh_resource = nullptr;                                        // 004775B1
        const std::string mesh = row.string("Mesh").value_or(std::string{});
        cls.particle_system = hooks.create_cloud_particles(mesh, cls.texture, multiplier);  // 004775FE
        return;
    }
    const std::string mesh = row.string("Mesh").value_or(std::string{});  // 0047764A..0047766F
    cls.mesh_resource = hooks.load_resource(mesh);                        // 004776A5..004776AA
}

AnimClass& get_or_create_anim_class_004722d0(NamedSceneClassRegistry<AnimClass>& registry,
                                             const std::string& name, SceneClassLuaTables& lua,
                                             const SceneClassHooks& hooks) {
    return get_or_create_named(registry, name, "AnimClass", lua,
                               [&](AnimClass& cls, SceneClassLuaRow& row) {
                                   read_anim_class_00470820(cls, row, hooks);
                               });
}

BuoyClass& get_or_create_buoy_class_00475710(NamedSceneClassRegistry<BuoyClass>& registry,
                                             const std::string& name, SceneClassLuaTables& lua,
                                             const SceneClassHooks& hooks) {
    return get_or_create_named(registry, name, "BuoyClass", lua,
                               [&](BuoyClass&, SceneClassLuaRow& row) {
                                   if (hooks.read_named_class_base) hooks.read_named_class_base(row);
                               });
}

CloudClass& get_or_create_cloud_class_00479970(NamedSceneClassRegistry<CloudClass>& registry,
                                               const std::string& name, SceneClassLuaTables& lua,
                                               const SceneClassHooks& hooks) {
    return get_or_create_named(registry, name, "CloudClass", lua,
                               [&](CloudClass& cls, SceneClassLuaRow& row) {
                                   read_cloud_class_004774a0(cls, row, hooks);
                               });
}

SourcedSceneClass& get_or_create_bullet_visual_class_00473bc0(
    SourcedSceneClassRegistry<SourcedSceneClass>& registry, const void* bullet_class,
    const std::string& bullet_name, std::int32_t bullet_index, SceneClassLuaTables& lua,
    const SceneClassHooks& hooks) {
    return get_or_create_sourced(registry, bullet_class, bullet_name, bullet_index, "Bullets", lua,
                                 hooks);
}

SourcedSceneClass& get_or_create_vehicle_visual_class_0047b3c0(
    SourcedSceneClassRegistry<SourcedSceneClass>& registry, const void* vehicle_class,
    const std::string& vehicle_name, std::int32_t vehicle_index, SceneClassLuaTables& lua,
    const SceneClassHooks& hooks) {
    return get_or_create_sourced(registry, vehicle_class, vehicle_name, vehicle_index,
                                 "VehicleClass", lua, hooks);
}

void reset_buoy_class_table_00474200(SceneClassLuaTables& lua) { lua.run_chunk("BuoyClass=nil\n"); }

void reset_cloud_class_table_00476410(SceneClassLuaTables& lua) { lua.run_chunk("CloudClass=nil\n"); }

} // namespace bsp

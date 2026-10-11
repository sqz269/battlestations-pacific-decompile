#pragma once

// The five Lua-described visual classes of partition segment 6 and their lazily
// filled registries: AnimClass, BuoyClass and CloudClass keyed by name, and the
// Bullets and VehicleClass visual classes keyed by the source class object.
// Evidence, layouts and divergences: docs/CC13_SCENE_BROWSER.md ("Segment 6:
// the visual class registries"). Every name is a hypothesis.

#include "bsp/input_settings.hpp"  // CaseInsensitiveLess

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace bsp {

// One row of a global Lua class table. Native reads go through a 14h-byte
// LuaObject: 00B67800 (GetByName) then 00B662B0 GetString (null when the key is
// not a string), 00B66270 GetNumber, 00B66330 GetFloatOrDefault and 00B66380
// GetIntegerOrDefault.
struct SceneClassLuaRow {
    virtual ~SceneClassLuaRow() = default;
    virtual std::optional<std::string> string(const char* key) = 0;
    virtual float number(const char* key) = 0;
    virtual float number_or(const char* key, float fallback) = 0;
    virtual std::int32_t integer_or(const char* key, std::int32_t fallback) = 0;
};

// The game Lua state: BSP_LuaStateOwner_GetGlobals (00B67980), the table by
// name (00B67800), then the row by string key (00B68100 / 00B67800) or by
// integer (00B677E0); `run_chunk` is the 006B8AD0 call of the reset routines.
struct SceneClassLuaTables {
    virtual ~SceneClassLuaTables() = default;
    virtual std::unique_ptr<SceneClassLuaRow> row(const char* table, const std::string& key) = 0;
    virtual std::unique_ptr<SceneClassLuaRow> row(const char* table, std::int32_t index) = 0;
    virtual void run_chunk(const char* chunk) = 0;
};

// Engine services the readers call. The base class fields that 0048F670 fills
// (Mesh resource +10h, FakeFireEfx +14h, fire point +18h..+24h) are not
// modelled here; `read_named_class_base` stands for that call.
struct SceneClassHooks {
    std::function<void(SceneClassLuaRow&)> read_named_class_base;              // 0048F670
    std::function<void*(const std::string& mesh)> load_resource;               // 007188A0
    // [[00E188A8]+19E8h]+3Ch (00BBDDF0), then 00BBD6C0(mesh, &texture, multiplier).
    std::function<void*(const std::string& mesh, const std::string& texture, float multiplier)>
        create_cloud_particles;
};

// 2Ch bytes, vtable 00CE5B44 {00470C60 dtor, 00470820 read, 00470C90 create
// entity (class id 3Eh), 00470890 MOV AL,1}. Radius at +28h.
struct AnimClass {
    std::string name;  // +04h
    float radius{};    // +28h
};

// 28h bytes, vtable 00CE5E14 {00474130, 0048F670 base read, 00474160 create
// entity (3Ch), 00473E20 MOV AL,1}. Only the base fields.
struct BuoyClass {
    std::string name;
};

// 38h bytes, vtable 00CE5FB4 {00477480, 004774A0 read, 00476360 create entity
// (3Dh), 00475970 MOV AL,1}. The constructor clears +28h, +2Ch and +30h.
struct CloudClass {
    std::string name;                 // +04h
    void* mesh_resource{};            // +10h (cloud_type != 0)
    void* particle_system{};          // +28h (cloud_type == 0)
    std::string texture;              // +2Ch
    std::int32_t cloud_type{};        // +34h
};

// 2Ch bytes. Bullets visual class: vtable 00CE5CB4 {004728C0, 0048F670,
// 004728F0 create entity (39h)}; built by 00472FF0 from a bullet class.
// VehicleClass visual class: vtable 00CE6254 {00479FB0, 0048F670, 00479FE0
// create entity (38h)}; built by 0047A800. Both keep the source at +28h and
// copy its name.
struct SourcedSceneClass {
    std::string name;            // +04h
    const void* source{};        // +28h
};

template <class T>
using NamedSceneClassRegistry = std::map<std::string, std::unique_ptr<T>, CaseInsensitiveLess>;
template <class T>
using SourcedSceneClassRegistry = std::map<const void*, std::unique_ptr<T>>;

// Address: 00470820. Original ABI: __thiscall, ECX = class, [ESP+4] = LuaObject
// row, RET 4. 0048F670 on the row, then Radius through GetNumber (no default)
// stored with FSTP to +28h.
void read_anim_class_00470820(AnimClass& cls, SceneClassLuaRow& row, const SceneClassHooks& hooks);

// Address: 004774A0. Original ABI: __thiscall, ECX = class, [ESP+4] = row,
// RET 4. Does not call 0048F670. CloudType (integer, default 0) to +34h. Zero:
// Texture (string) to +2Ch, ParticleMultiplier (float, default 1.0 from FLD1),
// +10h cleared, then Mesh and the particle-system factory into +28h. Non-zero:
// Mesh through 007188A0 into +10h.
void read_cloud_class_004774a0(CloudClass& cls, SceneClassLuaRow& row, const SceneClassHooks& hooks);

// Addresses: 004722D0 (AnimClass, registry 00472200 / index 00471F60),
// 00475710 (BuoyClass, 00475640 / 004753A0), 00479970 (CloudClass, 004798A0 /
// 00479600). Original ABI: __fastcall, ECX = name NativeString*, RET; EAX =
// class. On a miss: globals, the class table, its row for `name`, a new class
// with the name copied, the class read (vtable +4), then the registry cell.
// Every call, hit or miss, ends with vtable +0Ch, which is MOV AL,1 for all
// three classes and is not modelled.
AnimClass& get_or_create_anim_class_004722d0(NamedSceneClassRegistry<AnimClass>& registry,
                                             const std::string& name, SceneClassLuaTables& lua,
                                             const SceneClassHooks& hooks);
BuoyClass& get_or_create_buoy_class_00475710(NamedSceneClassRegistry<BuoyClass>& registry,
                                             const std::string& name, SceneClassLuaTables& lua,
                                             const SceneClassHooks& hooks);
CloudClass& get_or_create_cloud_class_00479970(NamedSceneClassRegistry<CloudClass>& registry,
                                               const std::string& name, SceneClassLuaTables& lua,
                                               const SceneClassHooks& hooks);

// Addresses: 00473BC0 (Bullets: registry 00473AF0 / index 004738C0, copy
// constructor 00472FF0 taking the name at source+10h and the row index at
// source+0Ch) and 0047B3C0 (VehicleClass: 0047B2F0 / 0047B0C0, 0047A800, name
// at source+50h, index at source+70h). Original ABI: __fastcall, ECX = source
// class, RET; EAX = class. On a miss the new class is published in the
// registry before the row `Table[index]` (00B677E0) is read through vtable +4
// (0048F670 for both). No vtable +0Ch call.
SourcedSceneClass& get_or_create_bullet_visual_class_00473bc0(
    SourcedSceneClassRegistry<SourcedSceneClass>& registry, const void* bullet_class,
    const std::string& bullet_name, std::int32_t bullet_index, SceneClassLuaTables& lua,
    const SceneClassHooks& hooks);
SourcedSceneClass& get_or_create_vehicle_visual_class_0047b3c0(
    SourcedSceneClassRegistry<SourcedSceneClass>& registry, const void* vehicle_class,
    const std::string& vehicle_name, std::int32_t vehicle_index, SceneClassLuaTables& lua,
    const SceneClassHooks& hooks);

// Addresses: 00474200 and 00476410. No arguments, RET. 006B8AD0(chunk, 0, 0, 2)
// with "BuoyClass=nil\n" (00CE5E24) / "CloudClass=nil\n". Callers: 0045F600 and
// BSP_Game_LoadMissionScene (004E0AB9 / 004E0ABE), each next to the other
// class-table resets.
void reset_buoy_class_table_00474200(SceneClassLuaTables& lua);
void reset_cloud_class_table_00476410(SceneClassLuaTables& lua);

} // namespace bsp

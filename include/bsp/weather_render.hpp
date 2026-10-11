#pragma once

// Weather and sky effects of partition segment 94 (00BBDEC0-00BC94D0): the 3D
// lightning storm, the camera water drops and the particle cloud class.
// Evidence and caveats: docs/CC13_WEATHER_RENDER.md. Names are hypotheses, not
// recovered symbols. These are new C++ interfaces, not drop-in replacements for
// the native thiscall entries; scene nodes, lights, materials and the process
// random generator are reached through the host interfaces below.
//
// Precision: the native code keeps intermediates on the x87 stack and spills to
// float at every FSTP dword. The reconstruction evaluates in double and rounds to
// float at the same spill points; results can differ from the image only where
// double rounding of an extended intermediate differs.

#include "bsp/random_threads.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace bsp::weather {

struct Vec3 {
    float x{};
    float y{};
    float z{};
};

// Shared kernels the weather code calls. Each maps to one native routine; the
// stream argument is the ECX selector the caller loads before 00BD2F10/00BD2FC0
// (RandomStream::primary is XOR ECX,ECX). Every draw is from the process-wide
// generator, so the order of calls below is part of the reconstructed contract.
class WeatherMath {
public:
    virtual ~WeatherMath() = default;
    virtual std::uint32_t random_u32(RandomStream stream) = 0;              // 00BD2FC0
    virtual float random_uniform(RandomStream stream, float lo, float hi) = 0; // 00BD2F10
    virtual float sqrt(float value) = 0;                                     // 00BF7030
    virtual float length(const Vec3& v) = 0;                                 // 00419440
    virtual Vec3 normalize(const Vec3& v) = 0;                               // 00419510
    virtual float reciprocal_length2(float x, float y) = 0;                  // 00419260
    virtual float asin_clamped(float value) = 0;                             // 0042CF10
};

// ---------------------------------------------------------------------------
// 3D lightning storm (object built by 00BC0D40 for 00873EB0, ticked by
// 00BC0510 from 00873C80). Six bolt meshes Effects\Lightning\villam_00{1..6}.MMOD.

struct LightningStormState {
    float flash_duration{0.1f};        // +00, 00D7A2F0
    float max_strike_interval{14.1f};  // +04, 00D64740
    float timer{};                     // +40, seconds until the next flash
    float flash_age{-1000.0f};         // +44, negative while dark (00D7A240)
    bool light_active{};               // +48 non-null (dynamic point light)
    std::int32_t flashes_left{};       // +4C, read unsigned by the intensity term
    std::uint32_t bolt{};              // +50, 0..5
};

class LightningHost {
public:
    virtual ~LightningHost() = default;
    virtual Vec3 camera_position() = 0;                 // camera+120h after 00B6DB70
    virtual Vec3 camera_forward() = 0;                  // 00B6FC60 (x and z read)
    virtual Vec3 camera_projected_x_axis() = 0;         // 00B70FE0 (x and z read)
    virtual Vec3 bolt_position(std::uint32_t bolt) = 0; // node+120h after 00B6DB70
    virtual void set_bolt_position(std::uint32_t bolt, const Vec3& p) = 0; // node vtable+30h
    virtual void set_bolt_visibility(std::uint32_t bolt, float factor) = 0; // 00B6DA70(f, 0)
    // 00B7BD30 + 00B7C710("dynamic_light_3Dlightning"), position +1ECh, radius
    // +1F8h, 00B7B090 under the particle-population lock, 004B62E0 diffuse
    // (1,1,1,1), then 00B7AF90(diffuse_scale).
    virtual void create_flash_light(const Vec3& position, float radius, float diffuse_scale) = 0;
    virtual void release_flash_light() = 0;             // 00B6DFA0
};

// 00BC0D40 tail: the first strike waits Uniform(primary, 0, max_strike_interval).
void lightning_storm_init_00bc0d40(LightningStormState& s, WeatherMath& math);

// 00BC0510(camera, delta, out thunder position), RET 0Ch. Returns true on the
// tick that starts a strike sequence (the first flash). `shader_lifetime` is the
// global 010909B4 bound to the bolt material's cLifetime parameter.
bool lightning_storm_update_00bc0510(LightningStormState& s, WeatherMath& math, LightningHost& host,
                                     float delta, Vec3& thunder_position, float& shader_lifetime);

// ---------------------------------------------------------------------------
// Camera water drops (GeneratedModel "waterdrops", 00BC2030; driven by the
// gameplay-effect Waterdrops component 00873100).

struct Waterdrop {               // 34h bytes
    float x{};                   // +00 screen space, -1..1
    float y{};                   // +04
    float size{};                // +08
    float height{};              // +0C size * camera aspect (camera+1C8h)
    float slide{};               // +10 Uniform(0.5,1) * sqrt(size^2 + height^2)
    float dir_x{};               // +14 unit vector from (0.01, 0.01) to the drop
    float dir_y{};               // +18
    float life_total{};          // +1C
    float life_left{};           // +20
    float field_24{};            // +24 rewritten 0 every tick
    float field_28{};            // +28 rewritten -1 every tick
    float stick{1.0f};           // +2C 1 normal, 1000 sticky, decays 0.2/s
    std::uint32_t variant{};     // +30 texture cell
};

struct WaterdropsSystem {
    std::vector<Waterdrop> drops;      // +184 begin / +188 size / +18C capacity
    std::int32_t requested{};          // +190 WaterdropCount after distance falloff
    float min_size{0.01f};             // +194 00D7A238
    float max_size{0.025f};            // +198 00D1FBBC
    float min_life{0.5f};              // +19C
    float max_life{4.0f};              // +1A0
    float sticky_percent{};            // +1A4 (not set by the constructor)
    std::int32_t live{};               // +1A8
    Vec3 last_camera{};                // +1AC
    Vec3 camera_step{};                // +1B8 old - new camera position
    float gravity{0.95f};              // +1C4 00CED9E0
    std::uint32_t variants{};          // +1CC
    float variant_step{};              // +1D0
    bool apply_at_bottom_only{true};   // +1D4 (constructor default 1)
    bool start_top_off_screen{};       // +1D5
    bool apply_at_top_only{};          // +1D6
    bool no_camera_move{};             // +1D7
};

// 010909B8: drops alive across every waterdrops system, capped at 3000.
struct WaterdropsBudget {
    std::uint32_t total{};
};

class WaterdropsHost {
public:
    virtual ~WaterdropsHost() = default;
    // 00BC1A20 prologue: vtable+5Ch on the model, 00B0D170(renderer, model, 0)
    // and 00B6DA70(0.0f, 0).
    virtual void attach_and_hide() = 0;
    virtual void set_visibility(float factor) = 0;     // 00B6DA70(f, 0)
    // 00B6FCB0 then 0042D0D0(v, view, normalize = 0).
    virtual Vec3 to_view_direction(const Vec3& world) = 0;
};

struct WaterdropsCamera {
    Vec3 position;          // camera+120h
    float matrix_01{};      // camera+F4h (row 0, column 1 of the +F0h matrix)
    float matrix_11{};      // camera+104h
    float aspect{};         // camera+1C8h
};

// 00BC1990: texture strip cells = vtable+3Ch / vtable+40h (unsigned), step 1/cells.
void waterdrops_set_texture_cells_00bc1990(WaterdropsSystem& w, std::uint32_t v3c, std::uint32_t v40);

// 00BC1A20(camera, unused), RET 8.
void waterdrops_spawn_00bc1a20(WaterdropsSystem& w, WaterdropsBudget& budget, WeatherMath& math,
                               WaterdropsHost& host, const WaterdropsCamera& camera);

// 00BC12F0(camera, x, y, z, delta), RET 14h.
void waterdrops_update_00bc12f0(WaterdropsSystem& w, WeatherMath& math, WaterdropsHost& host,
                                const Vec3& camera_position, float delta);

// 00BC1F30 budget release: total = max(total - requested, 0) as signed.
void waterdrops_release_budget_00bc1f30(const WaterdropsSystem& w, WaterdropsBudget& budget) noexcept;

// ---------------------------------------------------------------------------
// Particle cloud class (00BC4A60, vtable 00D64860; resource read by 00BC4530).

struct CloudSphere {
    Vec3 center;
    float radius{};
};

struct CloudPuff {
    Vec3 position;   // puff+0Ch, read back through 00BC7A70
    float size{};    // puff+1Ch, read back through 00BC7AB0
};

struct CloudClassGeometry {
    float density{};                         // +08 (constructor argument 6)
    std::vector<CloudPuff> puffs;            // +1C / +20 / +24 (puff pointers)
    float bounding_radius{};                 // +2C
    std::vector<CloudSphere> spheres;        // +30 / +34 / +38
    float max_sphere_radius{};               // +3C
    Vec3 bounds_min{1e10f, 1e10f, 1e10f};    // +44 (00CE4970)
    Vec3 bounds_max{-1e10f, -1e10f, -1e10f}; // +50 (00CE4ADC)
    Vec3 inv_extent{};                       // +6C
    Vec3 extent{};                           // +78
    Vec3 center{};                           // +84
};

class CloudSphereReader {
public:
    virtual ~CloudSphereReader() = default;
    virtual std::uint32_t read_u32() = 0;  // 00BE9A00
    virtual float read_float() = 0;        // 00BE99D0
};

class CloudPuffFactory {
public:
    virtual ~CloudPuffFactory() = default;
    // [010909B0] vtable+0Ch(index, &position, angle, size); empty when it
    // returns null. The returned record is what the object later reports.
    virtual std::optional<CloudPuff> create(std::int32_t index, const Vec3& position, float angle,
                                            float size) = 0;
};

// 00BC42E0: one CloudSystem element: count, then count * (x, y, z, radius).
void cloud_read_spheres_00bc42e0(CloudClassGeometry& c, CloudSphereReader& reader);

// 00BC32F0: true when every |component| <= radius and length^2 <= radius^2.
bool cloud_offset_within_00bc32f0(const Vec3& offset, float radius) noexcept;

// 00BC3EF0(&position, angle, size), RET 0Ch: rejects a puff that overlaps an
// existing one, else creates it and grows the bounds.
void cloud_add_puff_00bc3ef0(CloudClassGeometry& c, WeatherMath& math, CloudPuffFactory& factory,
                             const Vec3& position, float angle, float size);

// 00BC40F0(&center, rmin, rmax, count, size), RET 14h.
void cloud_scatter_puffs_00bc40f0(CloudClassGeometry& c, WeatherMath& math, CloudPuffFactory& factory,
                                  const Vec3& center, float rmin, float rmax, float count, float size);

// 00BC47D0 after the material and resource read: scatter every sphere, then
// derive the centre and extents.
void cloud_build_geometry_00bc47d0(CloudClassGeometry& c, WeatherMath& math, CloudPuffFactory& factory);

// ---------------------------------------------------------------------------
// Cloud instance (GeneratedModel "CloudInstance", 00BBE7D0, vtable 00D64528),
// stepped by the cloud manager 00BBD310 through 00BBE8B0. The documents written
// before cc13 called this object a shore-wave layer; see the weather doc.

struct CloudInstanceState {
    Vec3 scroll{};                // +1B8 wind drift accumulated since home
    float alpha{1.0f};            // +1C4 (00D7A24C)
    bool affected_by_wind{true};  // +1E8 (constructor default 1)
};

class CloudInstanceHost {
public:
    virtual ~CloudInstanceHost() = default;
    virtual Vec3 position() = 0;          // instance+120h after 00B6DB70
    virtual Vec3 camera_position() = 0;   // camera+120h after 00B6DB70
    virtual float class_radius() = 0;     // 00BC3010 on the cloud class at +184h
    virtual void set_position(const Vec3& p) = 0;  // vtable+30h
    virtual void set_visibility(float factor) = 0; // 00B6DA70(f, 0)
    virtual bool wind_enabled() = 0;      // 00BBCE70 on the manager (00BC3030)
    virtual Vec3 wind_direction() = 0;    // 00BBDD60 on the wind source (00BBCE50)
    virtual float wind_speed() = 0;       // 00BBDD70
};

enum class CloudInstanceStep {
    Hidden,    // fully faded; the native returns before the shader parameters
    Recycled,  // fully faded, home also out of range: snapped home, scroll cleared
    Visible,   // the native goes on to vtable+5Ch and the material parameters
};

// 00BBE9AD..00BBEA19: t = clamp((max(reach, 0) - 2000) / 600, 0, 1).
float cloud_instance_fade_00bbe8b0(float reach) noexcept;

// 00BBEB29..00BBEB90: u = 1 - (max(reach, 0) - 2000) * 0.002; recycle when u <= 0.
bool cloud_instance_home_out_of_range_00bbe8b0(float home_reach) noexcept;

// 00BBE8E8..00BBECD7 without the 00BBE640 lazy lighting and the parameter tail.
CloudInstanceStep cloud_instance_step_00bbe8b0(CloudInstanceState& s, WeatherMath& math,
                                               CloudInstanceHost& host, float delta);

}  // namespace bsp::weather
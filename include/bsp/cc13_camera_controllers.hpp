#pragma once
// Movie-camera keyframe modifiers: the `modifier` key of a MovCamNew keyframe
// (`goaround`, `gamecamera`, `fpscamera`), their Lua parse, their per-frame
// evaluation and the blend curves behind `theta`, `rho`, `radius`, `zoom`,
// `patterntime` and the `*linearblend` weights. Packet cc13_camera_controllers,
// docs/CC13_CAMERA_CONTROLLERS.md. Read from the listings; every name is a
// hypothesis, not a recovered symbol.
//
// Class family (13-slot vtables, one owner keyframe at +4h):
//   00D04414 base (0078E770), slots are no-ops apart from +4h SetOwner 0078E780
//   00D04448 type 1, 2Ch bytes, 0078E790; only the bit-stream reader 007A2370
//            creates it
//   00D04C8C type 2 `goaround`, 64h bytes, 007A0650
//   00D04CC0 type 3 `gamecamera` (fps byte 0) / `fpscamera` (fps byte 1),
//            7Ch bytes, 007A06D0
// Slots: +0 deleting dtor, +4 SetOwner, +8 step(elapsed, pose*) from 007911E0,
// +Ch evaluate(elapsed, pose*, offset) from 00795C10, +10h track tick (00797E40),
// +14h key tick (0078E630), +18h begin(camera, t) from 00791020, +1Ch ParseLua,
// +20h camera getter, +24h write bits (00791360), +28h read bits (007A2370),
// +30h type id.
//
// Status: reconstructed, build-tested. Not ABI-compatible (std::vector curves,
// host handles for entities and cameras). x87 chains are evaluated in double
// and rounded to float at each native FSTP; FSIN/FCOS/atan2 are std:: calls, so
// the last bit of an angle can differ from the image.

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "bsp/native_bit_cursor_numeric.hpp"
#include "bsp/native_bit_cursor_read.hpp"

namespace bsp::cc13cam {

// One curve: (time, value) pairs, appended in Lua order by 007A0A30
// (vector<pair<float,float>>::push_back; the vector object is 10h bytes with
// its first pointer at +4h). Times are not sorted or checked.
using ModifierCurve = std::vector<std::pair<float, float>>;

// A Lua value as the parse routines see it through the LuaObject wrappers
// (00B65FB0 IsNil, 00B66050 IsNumber, 00B661B0 IsTable, 00B66160 IsUserdata,
// 00B66270 GetNumber, 00B662D0 ToUserdata, 00B67720 by index, 00B67800 by name).
struct ModifierLuaValue {
    enum class Kind { Nil, Number, Table, Userdata, Other };
    Kind kind{Kind::Nil};
    // lua_tonumber; a numeric string is Kind::Number (lua_isnumber is true).
    double number{0.0};
    std::vector<ModifierLuaValue> array;                              // t[1], t[2], ...
    std::vector<std::pair<std::string, ModifierLuaValue>> fields;     // named keys
    std::size_t userdata{0};                                          // host entity handle

    const ModifierLuaValue& field(const char* name) const noexcept;
    const ModifierLuaValue& at(int lua_index) const noexcept;         // nil when absent
    bool is_nil() const noexcept { return kind == Kind::Nil; }
    // 00B66270: lua_tonumber spilled to float32 (0 for a non-number).
    float number_f32() const noexcept;
};

// The keyframe fields the modifiers read and write (MovieKeyframe in
// include/bsp/hud_movie_camera.hpp has the same image offsets).
struct ModifierKeyframe {
    std::size_t parent_1c{0};            // entity handle, 0 = none
    ModifierKeyframe* linked_20{nullptr};// the look-at partner key
    bool modifier_flag_25{false};
    int transform_2c{1};                 // 0 keepnone, 1 keepy, 2 keepz, 3 keepall
    std::array<float, 3> local_34{};
    std::array<float, 3> work_40{};
    std::array<float, 3> up_58{};
    std::array<float, 3> work_up_64{};
    std::array<float, 16> matrix_7c{};
    bool matrix_valid_bc{false};
    float zoom_c8{1.0f};
    float field_cc{0.0f};
    float smooth_d0{1.0f};               // written by `patterntime`
};

// ---------------------------------------------------------------------------
// Curve evaluation (inlined three times in 00791C90 and twice in 00795EE0)

// The quadratic ease the evaluators inline (same as 0078FCF0).
float modifier_ease(float f) noexcept;

struct CurveSample {
    float prev_value{0.0f};
    float next_value{0.0f};
    float fraction{0.0f};   // 0 when t is at or past the last key
};

// Scan for the first key with t < key.time. `prev` starts at (0, start_value).
// When t is at or past every key the fraction stays 0, so the value holds the
// last key's value.
CurveSample sample_modifier_curve(const ModifierCurve& curve, float t, float start_value) noexcept;

// value = w*next + prev*(1-w), w = ease(f)*(1-L) + f*L, w rounded to float.
float blend_modifier_curve(const CurveSample& s, float linear) noexcept;

// ---------------------------------------------------------------------------
// Type 1, the input-driven orbit (vtable 00D04448, 2Ch bytes, 0078E790).
// Only 007A2370 (the keyframe bit reader, type id 1) builds one, and its read
// slot is the base no-op, so it always starts from the constructor values.

// The 1Ch-byte block the step slot receives: camera+394h copied by 007911E0.
struct ModifierInput {
    std::uint8_t next_a_4{0}, next_b_5{0};   // both set: deck index +1
    std::uint8_t prev_a_6{0}, prev_b_7{0};   // both set: deck index -1
    float yaw_axis_8{0.0f};
    float pitch_axis_c{0.0f};
    float distance_axis_18{0.0f};
};

struct OrbitInputModifier {
    ModifierKeyframe* owner_4{nullptr};
    float yaw_8{0.0f};
    float yaw_rate_c{2.0f};          // 00CE3958
    float pitch_10{1.5707964f};      // 00CE3C64
    float pitch_rate_14{2.0f};
    float distance_18{50.0f};        // 00CEB4D4
    float distance_rate_1c{70.0f};   // 00CE77B4
    float last_time_20{-1.0f};       // 00D7A260
    int deck_index_24{-1};
    int deck_count_28{0};
};

struct OrbitInputHost {
    virtual ~OrbitInputHost() = default;
    // Row `index` of [[parent+538h]+BCh] (BSP_VehicleCameraArray, 5Ch rows),
    // grown by 005CD640 to index+1 first when shorter: +38h position, +18h up.
    virtual void deck_row(std::size_t parent, int index, std::array<float, 3>& position,
                          std::array<float, 3>& up) = 0;
    // [[parent+538h]+BCh]+4h, or 0 without that array.
    virtual int deck_count(std::size_t parent) = 0;
    // camera+528h (16 floats), times the parent's cached inverse (+110h, built
    // from +CCh by 00B63D50 when +10Ch is clear) through 00413920 when a parent
    // exists.
    virtual std::array<float, 16> camera_pose_in_parent(std::size_t camera, std::size_t parent) = 0;
};

inline constexpr int kOrbitInputTypeId = 1;      // slot +30h 0078E800
void orbit_input_construct_0078e790(OrbitInputModifier& m) noexcept;
// Slot +8h, RET 8: (time, input*). Called every key step by 007911E0 with
// time = clock - key start.
void orbit_input_step_007a0340(OrbitInputModifier& m, float time, const ModifierInput& in,
                               OrbitInputHost& host);
// Slot +10h, RET 4 (dt unused): distance rate = clamp(|distance| * 1.4, 5, 300).
void orbit_input_track_tick_00791640(OrbitInputModifier& m) noexcept;
// Slot +18h, RET 8: (camera, t). Seeds distance, pitch and yaw from the
// camera's pose relative to the owner's authored position, and the deck count.
void orbit_input_begin_007916c0(OrbitInputModifier& m, std::size_t camera, OrbitInputHost& host);

// ---------------------------------------------------------------------------
// Type 2, `goaround` (vtable 00D04C8C, 64h bytes)

struct GoAroundModifier {
    ModifierKeyframe* owner_4{nullptr};
    float radius_linear_8{0.6f};     // 00CE3D30
    float theta_linear_c{0.6f};
    float rho_linear_10{0.6f};
    ModifierCurve radius_14;         // `radius`, start value 100.0 (00CE3D08)
    ModifierCurve theta_24;          // `theta`, degrees -> radians
    ModifierCurve rho_34;            // `rho`, degrees -> radians
    ModifierCurve zoom_44;           // `zoom`, linear, start 1.0
    ModifierCurve patterntime_54;    // `patterntime`, linear, start 1.0
};

inline constexpr int kGoAroundTypeId = 2;        // slot +30h 0079DC30
void goaround_construct_007a0650(GoAroundModifier& m) noexcept;
// Slot +1Ch, ECX = modifier, one stack argument (the keyframe's Lua table), RET 4.
void goaround_parse_007a3100(GoAroundModifier& m, const ModifierLuaValue& table);
// Slot +Ch, RET 0Ch: (elapsed, pose*, offset); pose is unused.
void goaround_evaluate_00791c90(GoAroundModifier& m, float elapsed, float offset) noexcept;
// Slot +24h (ECX = modifier, the cursor itself) and slot +28h (the reader
// object; its cursor is at +4h). Weights +8h, +10h, +Ch as (0, 0, 1.0, 8 bits);
// each curve as an 8-bit count, then (time: 0, 1, 256.0, 18 bits) and
// (value: 0, 1, scale, 18 bits), scale 10000.0 for radius and 100.0 otherwise.
// The reader appends to the curves without clearing them.
void goaround_write_bits_007918b0(const GoAroundModifier& m, NativeBitCursor* cursor,
                                  const NativeBitNumericContext& ctx);
void goaround_read_bits_007a2690(GoAroundModifier& m, NativeBitCursor* cursor,
                                 const NativeBitNumericContext& ctx);

// ---------------------------------------------------------------------------
// Type 3, `gamecamera` / `fpscamera` (vtable 00D04CC0, 7Ch bytes)

// Entity kinds the evaluator and 0079CBD0 test through vtable +5Ch.
enum : int {
    kKind06 = 6, kKind08 = 8, kKind0F = 0x0F, kKind18 = 0x18, kKind45 = 0x45, kKind46 = 0x46,
};

struct GameCameraModifier {
    ModifierKeyframe* owner_4{nullptr};
    std::size_t camera_8{0};         // the game camera mover 0079CBD0 builds
    float last_time_c{-1.0f};        // 00D7A260; the evaluator's previous t
    float theta_linear_10{0.8f};     // 00CE74F8, `thetalinearblend`
    float rho_linear_14{0.8f};       // `rholinearblend`
    ModifierCurve theta_18;          // `theta` or `Vlook`
    ModifierCurve rho_28;            // `rho` or `Hlook`
    std::size_t target_4c{0};        // `targetentity.Ptr`, the observer at +38h
    int theta_entity_count_50{0};
    int rho_entity_count_54{0};
    std::vector<std::size_t> theta_entities_58;  // 18h-byte observers, +14h entity
    std::vector<std::size_t> rho_entities_68;
    bool fps_78{false};
};

// Entity reference marker in a curve value: count + 10000.0 (00CE4BD8), read
// back as ftol(value - 9999.900390625) (00D04658) when value >= 10000.0 (00CE3D64).
inline constexpr double kEntityRefBase = 10000.0;

struct GameCameraModifierHost {
    virtual ~GameCameraModifierHost() = default;
    virtual bool is_kind(std::size_t entity, int kind) = 0;              // vtable +5Ch
    virtual std::size_t squadron_leader_3d0(std::size_t entity) = 0;     // kind 18h
    // The target's +FCh in the parent's derived affine inverse (00414E10, 004142E0).
    virtual std::array<float, 3> point_in_parent_frame(std::size_t parent, std::size_t target) = 0;
    // The same through the parent's cached inverse at +110h (00B63D50 from +CCh).
    virtual std::array<float, 3> point_in_parent_cached_frame(std::size_t parent, std::size_t target) = 0;
    // Camera mover calls and fields.
    virtual void camera_set_angles_0042aa00(std::size_t camera, float rho, float theta, bool flag) = 0;
    virtual void camera_store_orbit_angles(std::size_t camera, float theta, float rho) = 0; // +3E8/+3E4, +3C8/+3C4
    virtual bool camera_3e8_is_zero(std::size_t camera) = 0;             // CMP dword [cam+3E8h],0
    virtual void camera_store_free_angles(std::size_t camera, float rho, float theta) = 0;  // +384h/+388h
    virtual void fps_attach_007c7800(std::size_t parent, std::size_t camera) = 0;
    virtual void camera_update_dc(std::size_t camera, float dt) = 0;     // vtable +DCh
    virtual std::array<float, 16> camera_matrix_114(std::size_t camera) = 0;  // vtable +114h
};

inline constexpr int kGameCameraTypeId = 3;      // slot +30h 0079DCF0
void gamecamera_construct_007a06d0(GameCameraModifier& m, bool fps) noexcept;
// Slot +1Ch. `entity_of` is 004BEF00 applied to a `Ptr` userdata.
void gamecamera_parse_007a3b20(GameCameraModifier& m, const ModifierLuaValue& table);
// 00792500 / 00792610: a key's value, or the angle to its referenced entity.
float gamecamera_theta_key_00792500(const GameCameraModifier& m, std::size_t index,
                                    GameCameraModifierHost& host);
float gamecamera_rho_key_00792610(const GameCameraModifier& m, std::size_t index,
                                  GameCameraModifierHost& host);
// Slots +24h / +28h: weights +14h then +10h, then the theta and rho curves,
// all with goaround's encodings (value scale 100.0). Entity markers travel as
// plain values and the entity lists are not written.
void gamecamera_write_bits_00792710(const GameCameraModifier& m, NativeBitCursor* cursor,
                                    const NativeBitNumericContext& ctx);
void gamecamera_read_bits_007a2a80(GameCameraModifier& m, NativeBitCursor* cursor,
                                   const NativeBitNumericContext& ctx);
// Slot +Ch, RET 0Ch.
void gamecamera_evaluate_00795ee0(GameCameraModifier& m, float elapsed, float offset,
                                  GameCameraModifierHost& host);

// 0079CBD0 (the ledger's BSP_FlightLeaderControlBlock_GetOrCreate_Provisional):
// which camera class the modifier builds for its parent, built once into +8h.
enum class GameCameraClass {
    None,
    Kind08_00650830,        // 410h bytes, then 0042F6C0 and the +384h..+398h seed
    Kind06_ShipCaptain_0064b650, // 404h bytes, then BSP_CameraMover_SetTargetAndMode(parent, 0)
    Kind0F_Fps_006065d0,    // fps byte set: 0042E030, then parent vtable +1C4h
    Kind0F_00605130,        // fps byte clear: 0042AAF0
    Kind46_0078e830,        // the 0078E830 mover (vtables 00D044B8/00D0449C/00D04494/00D0447C), 0042EB20
    Kind45_005176f0,        // 0042CB30
};
struct GameCameraChoice {
    GameCameraClass cls{GameCameraClass::None};
    std::size_t entity{0};  // after the kind-18h leader substitution
};
GameCameraChoice gamecamera_choose_camera_0079cbd0(const GameCameraModifier& m,
                                                   GameCameraModifierHost& host);

}  // namespace bsp::cc13cam

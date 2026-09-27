#pragma once
// The new movie camera (MovCamNew): the 570h-byte mover 005CC170 builds, its
// keyframe tracks and its per-frame update. Packet cc9_movie_camera_mover_bind,
// docs/HUD_PICK_SEGMENT_QUERY.md sections 8.2, 8.6 and 8.7. Read from the
// listings; descriptive names are hypotheses.
//
// Coverage (the doc's table has the address ranges):
// - 00799B00 keyframe defaults, 007A0EB0 parse (the keys the shipped
//   luaCamIngameMovieAuto passes), 007A42C0 add, 007A2CD0/007A0770 insert:
//   complete for those keys; `modifier`, `deckpos`, `upvector`,
//   `relativetotarget`, `event`, `finishscript`, `flyalt` and the `_thennone`
//   transforms are counted as unsupported and not applied.
// - 007911E0 step, 00791020 begin, 00795C10 weight, 0078FCF0 ease,
//   00798130 evaluate: complete apart from modifiers and flyalt.
// - 00795650 position: keepnone, keepy, keepz and keepall, the ship and plane
//   wanderer offsets; relativetotarget and the kind-20h sub-object matrix
//   are unsupported. `terrainavoid` and its clamp 00795B45 are modelled
//   under kMovieTerrainAvoidBound (packet cc9_movie_camera_keys, doc 10).
// - 0079D020 constructor, 007A0860 seed, 00798C80 fixed step (the +391h
//   latch), 0079A3B0 update up to 0079B866 (the single-player end; the editor
//   block 0079A584..0079B2A2 is unreachable without a selected keyframe).
// - Ground clearance: +4FCh is 0 from the constructor and nothing here sets it,
//   so 0078FAF0 runs on a zero state and the height offset stays 0 (applied).

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace bsp {

inline constexpr std::size_t kMovieNoParent = ~std::size_t{0};

// 00799B00, 104h bytes. Field comments name the image offset.
struct MovieKeyframe {
    bool parent_attached_24{false};
    bool wanderer_26{true};
    bool has_position_27{true};
    bool camera_28{true};
    int transform_2c{1};              // 0 keepnone, 1 keepy, 2 keepz, 3 keepall
    bool then_none_30{false};
    std::array<float, 3> local_34{};  // authored position
    std::array<float, 3> work_40{};   // copied at begin
    std::array<float, 3> eval_4c{};   // world position from 00795650
    std::array<float, 3> up_58{0.0f, 1.0f, 0.0f};
    std::array<float, 3> work_up_64{0.0f, 1.0f, 0.0f};
    std::array<float, 3> eval_up_70{0.0f, 1.0f, 0.0f};
    float fov_divisor_c0{-1.0f};      // 00D7A260
    float zoom_c4{1.0f};
    float zoom_c8{1.0f};
    float smooth_cc{1.0f};
    float smooth_d0{1.0f};
    float nonlinear_d4{0.5f};         // 00CE3800
    bool use_current_d8{false};
    bool terrain_avoid_d9{false};     // `terrainavoid`, 007A1488; 00799C4B stores 0
    float start_f0{0.0f};
    float blend_f4{0.0f};
    int state_f8{0};                  // 0 waiting, 1 blending, 2 held, 3 retired
    std::size_t parent_1c{kMovieNoParent};
};

// One track, 84h bytes at camera+414h (look-at) or camera+498h (camera).
struct MovieTrack {
    std::vector<std::unique_ptr<MovieKeyframe>> keys;  // the list at +68h, appended
    std::array<float, 3> pos_04{};
    std::array<float, 3> up_10{};
    float zoom_60{0.0f};
    float window_78{0.0f};   // 0079D12A / 0079D0F1 store 0.0
};

struct HudMovieCamera {
    bool running_391{false};  // set by 00798C80's first step (its +221h on the +170h sub-object)
    float clock_3b0{0.0f};
    std::array<float, 3> look_384{};
    MovieTrack target_414;
    MovieTrack camera_498;
    float fov_410{0.6981317f};  // 00CE7D20
    // +51Ch/+520h/+524h, 0078FAF0's (velocity, goal velocity, offset).
    std::array<float, 3> ground_51c{};
    // +528h..+567h: right, up (+538h), forward (+548h), position (+558h).
    std::array<float, 16> matrix_528{};
    std::array<float, 16> local_74{};
    unsigned long long fixed_steps{0};
    // 00798C80's else arm (camera+3C4h..+40Fh, the +170h sub-object's
    // +254h..+29Fh): a 16-entry ring of truncated stream-1 draws, its index,
    // a step count and a dt sum. No reader was found for any of them (the
    // disp32 scans of 0078D880..007A4860 find only 00798C80's own accesses).
    std::array<std::int32_t, 16> draw_ring_3c4{};
    int draw_index_404{0};
    int steps_408{0};
    float step_seconds_40c{0.0f};
};

// What the keyframes ask of their parent entity.
struct MovieCameraParentHost {
    virtual ~MovieCameraParentHost() = default;
    virtual bool destroyed_5e(std::size_t unit) = 0;          // 00799D70's +5Eh
    virtual bool is_kind(std::size_t unit, int kind) = 0;     // vtable +5Ch
    virtual std::size_t squadron_leader_3d0(std::size_t unit) = 0;
    // +CCh..+108h after 00B6DB70 / BSP_EntityPose_RefreshWorld.
    virtual bool world(std::size_t unit, std::array<float, 16>& m) = 0;
    virtual float class_height_a8(std::size_t unit) = 0;      // [[unit+538h]+A8h]
    virtual std::array<float, 3> plane_velocity_810(std::size_t unit) = 0;
    // 00903860(world+19CCh)(point, &out), the terrain height under `point`;
    // false when no Landscape answers. Read only for a `terrainavoid` keyframe.
    virtual bool ground_height_00903860(const std::array<float, 3>& point, float& out) = 0;
    virtual void unsupported(const char* what, std::uint32_t address) = 0;
};

// One MovCamNew_AddPosition table as 007A0EB0 reads it.
struct MovieKeyframeInput {
    std::string postype;
    bool has_position{false};
    std::optional<std::array<float, 3>> pos;     // `pos`, named x/y/z or 1..3
    bool has_parent{false};
    std::size_t parent{kMovieNoParent};          // resolved entity, or none
    std::optional<std::array<float, 3>> polar;   // distance, theta, rho (degrees)
    std::optional<float> starttime, blendtime, linearblend, nonlinearblend, zoom, smoothtime;
    std::optional<std::string> transformtype;
    std::optional<bool> wanderer;
    // `position.terrainavoid` when it is a boolean (007A1470..007A1488). The
    // Lua parse fills it only under kMovieTerrainAvoidBound; OFF lists the key
    // in unsupported_keys instead.
    std::optional<bool> terrainavoid;
    std::vector<std::string> unsupported_keys;
};

// 0079D020 (fields past the base 00432750); its 00BD2FD0(1, 123) is the caller's.
void movie_camera_construct_0079d020(HudMovieCamera& camera) noexcept;
// 007A0860(matrix): +528h = matrix; with an empty camera track, a camera and a
// target keyframe with +D8h = 1 at time 0.
void movie_camera_seed_007a0860(HudMovieCamera& camera, const std::array<float, 16>& world);
// 007A42C0 through 007A0EB0 and 007A2CD0. Returns the keyframes added.
int movie_camera_add_position_007a42c0(HudMovieCamera& camera, const MovieKeyframeInput& in,
                                       MovieCameraParentHost& host);
// 00798C80, the tick element's +8h slot (wave 3). Returns true on the first
// step with dt > 0, when it latches +391h and reseeds streams 1 and 0.
bool movie_camera_fixed_step_00798c80(HudMovieCamera& camera, float dt) noexcept;
// 00798CD9..00798D36, the else arm of 00798C80 on a step after the latch:
// +408h += 1, +40Ch += dt, then the 00798D07 draw (the caller's, stream 1,
// 0.0 to 65535.0 at 00D046A8) converted by 00BF7420 into +3C4h[+404h], the
// index wrapping after 15. The caller runs it only when +391h was already set
// on entry to 00798C80.
void movie_camera_store_step_draw_00798cd9(HudMovieCamera& camera, float dt,
                                           float draw) noexcept;
// 0079A3B0 up to 0079B866. Returns false when a track is empty (0079BBD5).
// On true, `world` is +74h after the 004134F0 copy and `fov` is 00B6FBB0's
// argument, +410h / zoom.
bool movie_camera_update_0079a3b0(HudMovieCamera& camera, MovieCameraParentHost& host,
                                  float dt, std::array<float, 16>& world, float& fov);

}  // namespace bsp

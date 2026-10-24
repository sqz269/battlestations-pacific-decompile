#pragma once

// The Lua `Spawn` native's placement: what BSP_LuaBinding_Spawn 00944680 does
// between reading its arguments and calling the creator 004C6BA0. Packet
// cc13-spawn1; evidence, the call table and the suggested host glue are in
// docs/CC13_LUA_SPAWN.md. Every name is a hypothesis, not a recovered symbol.
// These are new C++ interfaces, not binary replacements; build-tested only, not
// wired into any host.
//
// include/bsp/lua_binding_spawn.hpp (packet cc_lua_core) already reconstructs
// the argument marshalling and the creation tail against LuaBindingSpawnHost,
// whose find_spawn_position / place_spawn_directly were "contract: unread".
// This file is those two contracts:
//
//   00941FA0  ring points       one draw of the bearing, then a near and a far point
//   009426D0  line search       256 samples near->far, the first free one
//   00941C20  mask clear        zero the samples inside one entity's radius
//   00941AA0  ray/sphere        the entry and exit parameters 00941C20 uses
//   004BC120  edge crossing     the first world-box edge a segment crosses (shared
//                               with 008AF700)
//   00941D30  point legality    already reconstructed for SpawnNew (lua_spawn_new.hpp);
//                               here with Spawn's fixed radius block and the terrain probes
//   00942CB0  direct placement  the trailing-boolean branch: eight bearings, then a
//                               walk along the world border
//
// Precision: every x87 intermediate here is stored to a float before reuse, so
// the float arithmetic below matches the listing's store points; qword constants
// are applied in double and rounded at the store.
//
// RNG: the only draw is 00BD2F10 at 00941FC0 with ECX = 1 (stream 1, the
// gameplay stream; docs/CC13_RNG_STREAM_CENSUS.md). It goes through
// SpawnPlacementHost::uniform_00bd2f10 with kSpawnRingDrawKey, so a host routes it
// to a keyed stream of its own, never stream 0. 00942CB0 draws up to eight
// times per call (one per bearing attempt); 00944680's search draws once.

#include <cstdint>
#include <string>
#include <vector>

#include "bsp/lua_spawn_new.hpp"   // SpawnPlacementEntity, spawn_member_placement_legal_00941d30
#include "bsp/const_exact_image_bits.hpp"

namespace bsp {

// For the mission Lua host (cc9). True: the `Spawn` row 00944680 is dispatched
// through lua_spawn_resolve_position. Committed OFF; the suggested glue is in
// docs/CC13_LUA_SPAWN.md section 6.
inline constexpr bool kLuaSpawnBound = false;

// The RNG key of the one draw (00941FC0). The host prefixes its own consumer and
// the spawn's identity.
inline constexpr const char* kSpawnRingDrawKey = "ring";

// ---------------------------------------------------------------------------
// Constants, as the listing loads them
// ---------------------------------------------------------------------------
inline constexpr double kSpawnLineSampleStep = 1.0 / 256.0;   // 00D19918 double
inline constexpr int kSpawnLineSamples = 0x100;                // 009429D1 CMP ESI,100h
inline constexpr double kSpawnLineLengthEpsilon = 1.0e-10;     // 00CE3820 double
inline constexpr float kSpawnShallowStartBelow = 10.0f;        // 00CE38B8 float
inline constexpr float kSpawnGroundBlocksAbove = -10.0f;       // 00CE6848 float
inline constexpr double kSpawnTerrainWalkStep = 50.0;          // 00CE3938 double
inline constexpr double kSpawnOwnPartyVertical = 100.0;        // 00D7A220 double
inline constexpr float kSpawnOwnPartyRadius = 100.0f;          // 00CE3D08 float
inline constexpr double kSpawnOtherPartyVertical = 500.0;      // 00CE3840 double
inline constexpr float kSpawnOtherPartyRadius = 2000.0f;       // 00CFFD60 float
inline constexpr double kSpawnMaskScale = 255.0;               // 00CE4B48 double
inline constexpr double kSpawnDirectSweepStep = 0.25;          // 00D7A348 double
inline constexpr int kSpawnDirectAttempts = 8;                 // 00942F66 CMP ESI,8
inline constexpr double kSpawnDirectAircraftLift = 1000.0;     // 00CE47A0 double
inline constexpr double kSpawnDirectFallbackReach = 40000.0;   // 00D19920 double
inline constexpr float kSpawnBorderWalkStep = 100.0f;          // 00CE3D08 / 00CE65D8 (-100), first steps
inline constexpr double kSpawnBorderTurnStep = 100.0;          // 00D7A220 double, 009434EB, after a turn
inline constexpr int kSpawnBorderWalkTurns = 2;                // 009434F7 MOV EAX,2
inline constexpr float kSpawnGiveUpHeight = 10.0f;             // 00CE38B8, the (0,10,0) exit
inline constexpr float kSpawnProbeGroundDefault = -1000.0f;    // 00D7A240 float
// 00CE3D28 is float pi widened to double (00 00 00 60 FB 21 09 40), every load an m64; 00CE3D20
// is 180.0. No reader yet; the image applies them as FMUL then FDIV, not as one folded factor.
inline constexpr double kSpawnDegreesToRadians = 3.1415927410125732 / 180.0;  // 00CE3D28 / 00CE3D20
inline constexpr double kSpawnQuarterTurn = kConstExactImageBitsBound ? 0x1.921fb6p+0 : 1.5707963267948966;              // 00CE3830 double; image bits 3FF921FB60000000 at 00CE3830

// 00941D30's radius block as 00942CB0 builds it on its stack (00942D68..
// 00942E09): +0 100, +4 2000, +8 own-party vertical 100 (00CE3D08), +Ch other-party
// vertical 500 (00CE397C), +10h 0, +14h own-party square 10000 (00CE3D64),
// +18h other-party square 4000000 (00D19914). In SpawnNewExcludeRadius terms the
// horizontals are the square roots, which are exact.
SpawnNewExcludeRadius spawn_direct_exclude_block_00942cb0() noexcept;

// The terrain probe offsets 00941D30 adds to the point for a non-aircraft
// (table 00E0CF78, five float3, loop end 00E0CFBC): the point and four corners
// 10 m out on x and z.
inline constexpr float kSpawnTerrainProbes[5][3] = {
    {0.0f, 0.0f, 0.0f},
    {-10.0f, 0.0f, -10.0f},
    {-10.0f, 0.0f, 10.0f},
    {10.0f, 0.0f, 10.0f},
    {10.0f, 0.0f, -10.0f},
};

// The class name 00944EAD compares case-insensitively (00438E10, EDX = 00CEA760)
// against the hidden record's class (record+0Ch, docs/SCENE_RECORD_MAP.md). A
// match is 00942CB0's aircraft flag: no terrain probes, and a second sweep 1000 m up.
inline constexpr const char* kSpawnAircraftClassName = "PlaneSquadronGen";

// ---------------------------------------------------------------------------
// Inputs
// ---------------------------------------------------------------------------

// GGame+711Ch..+7130h ([00E188A8]), in the order the image stores them:
// 711C min x, 7120 min y, 7124 max z, 7128 max x, 712C max y, 7130 min z.
struct SpawnWorldBox {
    float min_x{0.0f};   // +711Ch
    float min_y{0.0f};   // +7120h
    float max_z{0.0f};   // +7124h
    float max_x{0.0f};   // +7128h
    float max_y{0.0f};   // +712Ch
    float min_z{0.0f};   // +7130h
};

// The three native calls the routines make into the world.
struct SpawnPlacementHost {
    virtual ~SpawnPlacementHost() = default;
    // 00BD2F10 BSP_Random_UniformFloatRange, ECX = 1 in the image.
    virtual float uniform_00bd2f10(const char* key, float low, float high) = 0;
    // 00903860 BSP_World_GroundHeightAt, ECX = [GGame+19CCh]: true when there is
    // ground under (x, z), with its height.
    virtual bool ground_height_00903860(const float point[3], float& height) = 0;
    // 0071C4F0 BSP_Game_PointOutsideMapBounds, ECX = GGame.
    virtual bool point_outside_map_0071c4f0(const float point[3]) = 0;
};

// Everything 00944680 hands the two placement branches. The angles are already
// in radians and already offset by the origin entity's yaw in mode 1.
struct SpawnPlacementRequest {
    float origin[3]{0.0f, 0.0f, 0.0f};  // slot 2: a position, or the entity's +FCh..+104h
    float near_distance{0.0f};          // slot 3, 0094489C; E+94h
    float far_distance{0.0f};           // slot 4; E+98h
    float height{0.0f};                 // slot 5; E+90h, the y of both ring points
    float angle_low{0.0f};              // slot 6 * pi / 180; E+38h
    float angle_high{0.0f};             // slot 7 * pi / 180; E+20h
    std::int32_t party{0};              // hidden record+10h, read at 00944BF5
    bool aircraft{false};               // record+0Ch == kSpawnAircraftClassName, 00944EB8
    bool direct{false};                 // the trailing boolean, 00944BEC
};

// ---------------------------------------------------------------------------
// The routines
// ---------------------------------------------------------------------------

// 00941FA0, __fastcall(ECX near_out, EDX far_out, origin*, near, far, height,
// angle_low, angle_high), RET 18h. a = U(angle_low, angle_high) on stream 1;
// near = origin + near * (-sin a, 0, cos a), far likewise, both with y = height
// (the origin.y + 0.0 store at 00942026 is overwritten at 0094207F).
struct SpawnRingPoints {
    float angle{0.0f};
    float near_point[3]{0.0f, 0.0f, 0.0f};
    float far_point[3]{0.0f, 0.0f, 0.0f};
};
SpawnRingPoints spawn_ring_points_00941fa0(const float origin[3], float near_distance,
                                           float far_distance, float height,
                                           float angle) noexcept;
SpawnRingPoints spawn_ring_draw_00941fa0(SpawnPlacementHost& host, const float origin[3],
                                         float near_distance, float far_distance,
                                         float height, float angle_low,
                                         float angle_high);

// 00941AA0, __fastcall(ECX start, EDX unit dir, center*, radius, t0*, t1*), RET 10h.
// Projects center onto the line; refuses when the squared miss distance is not
// below radius^2 (00941BCC FCOMI / JB; an unordered NaN compare intersects);
// otherwise t0,1 = proj -/+ sqrt(r^2 - d^2). The sums run x, y, z on the x87 stack.
bool spawn_ray_sphere_00941aa0(const float start[3], const float dir[3],
                               const float center[3], float radius, float& t0,
                               float& t1) noexcept;

// 00941C20, __fastcall(ECX start, EDX unit dir, length, center*, radius, mask*),
// RET 10h. Writes start.y into the caller's copy of the center first (00941C2A),
// then clamps t0, t1 to [0, length] and zeroes mask[floor(t0/len*255) ..
// floor(t1/len*255)] (00941D14 memset). `center` is taken by value for that reason.
void spawn_mask_clear_00941c20(const float start[3], const float dir[3], float length,
                               float center_copy[3], float radius,
                               unsigned char mask[kSpawnLineSamples]) noexcept;

// 009426D0, __fastcall(ECX out*, EDX party, start*, end*, ok*), RET 0Ch, returns
// out. Samples i/256 along start->end: a sample outside the world box is
// blocked; when start.y < 10 a sample is also blocked by ground above -10 found
// within its own 1/256 stretch, walked in 50 m steps. Each entity then blocks
// the samples inside a sphere of 100 m (own party, |dy| <= 100) or 2000 m (other
// party, |dy| <= 500) through 00941C20. out = start + L * (i/256) * dir for the
// first free i, with ok; with none, i = 256 (out = the end) and ok stays false.
struct SpawnLineResult {
    bool ok{false};
    int index{kSpawnLineSamples};
    float position[3]{0.0f, 0.0f, 0.0f};
    int blocked_by_box{0};
    int blocked_by_ground{0};
};
SpawnLineResult spawn_line_search_009426d0(const float start[3], const float end[3],
                                           std::int32_t party, const SpawnWorldBox& box,
                                           const std::vector<SpawnPlacementEntity>& entities,
                                           SpawnPlacementHost& host);

// 004BC120, __thiscall(GGame, a*, b*, cross_out*, edge_a_out*, edge_b_out*,
// crossed_out*), RET 18h. Tests a->b in (x, z) against the box edges in the order
// max-z, max-x, min-z, min-x through 004F3730 (the last two via the float3
// wrapper 004B6720) and stops at the first crossing. cross = (x, 0, z); the
// edge endpoints carry the y the listing stores, which is not uniform: the
// max-x edge ends at max y (004BC320), the min-z edge starts at max y (004BC36E).
// With no crossing the image leaves cross as 004B6720's uninitialised locals;
// `cross_written` is false then and `cross` holds the zero this file substitutes.
struct SpawnEdgeCrossing {
    bool crossed{false};
    bool cross_written{false};
    int edge{-1};   // 0 max-z, 1 max-x, 2 min-z, 3 min-x
    float cross[3]{0.0f, 0.0f, 0.0f};
    float edge_a[3]{0.0f, 0.0f, 0.0f};
    float edge_b[3]{0.0f, 0.0f, 0.0f};
};
SpawnEdgeCrossing spawn_edge_crossing_004bc120(const SpawnWorldBox& box, const float a[3],
                                               const float b[3]) noexcept;

// 00941D30 as 00942CB0 calls it: party in ECX, class 0, frame 0, the block of
// spawn_direct_exclude_block_00942cb0, the aircraft flag. Outside the map
// refuses; a non-aircraft also refuses on ground above -10 under any of the
// five kSpawnTerrainProbes; then lua_spawn_new's entity test.
bool spawn_point_legal_00941d30(const float point[3], std::int32_t party, bool aircraft,
                                const std::vector<SpawnPlacementEntity>& entities,
                                SpawnPlacementHost& host);

// 00944C44..00944D60, the search 00944680 runs without the trailing boolean:
// ring points, then 009426D0 near->far; failing that, the box edge near->far
// crosses (none: nil) and 009426D0 from the crossing toward each end of that edge.
struct SpawnSearchResult {
    bool ok{false};
    int stage{0};   // 1 ring line, 2 toward edge_a, 3 toward edge_b; 0 failed
    float position[3]{0.0f, 0.0f, 0.0f};
    SpawnRingPoints ring{};
};
SpawnSearchResult spawn_find_position_00944c44(const SpawnPlacementRequest& request,
                                               const SpawnWorldBox& box,
                                               const std::vector<SpawnPlacementEntity>& entities,
                                               SpawnPlacementHost& host);

// 00942CB0, __fastcall(ECX out*, EDX party, origin*, near, far, height,
// angle_low, angle_high, aircraft), RET 1Ch. Always produces a position:
//  1. up to eight bearings; each tries near + t * (far - near) for t = 0, .25,
//     .5, .75 at the ring height and, for an aircraft, the same 1000 m higher;
//  2. otherwise a border point: the segment from the last near point toward the
//     origin (near outside the box) or 40 km along near->far is cut by the box;
//     with no cut, an origin outside the box and 0071C4F0(near), the target is
//     clamped to [1 - W, W - 1] and cut again. W is the box's max x alone; the
//     routine treats the world as [-W, W] on both axes;
//  3. the border point at the ring height, snapped to W - 1 on its edge;
//  4. two walkers from it in opposite directions along the border, 100 m a step,
//     turning at each corner, three corners each;
//  5. (0, 10, 0).
struct SpawnDirectResult {
    int stage{0};   // 1 ring sweep, 2 lifted sweep, 3 border point, 4 walker, 5 give-up
    int attempts{0};
    int walker{-1};
    bool undefined_border{false};   // the image's result depends on uninitialised stack
    float position[3]{0.0f, 0.0f, 0.0f};
};
SpawnDirectResult spawn_place_directly_00942cb0(const SpawnPlacementRequest& request,
                                                const SpawnWorldBox& box,
                                                const std::vector<SpawnPlacementEntity>& entities,
                                                SpawnPlacementHost& host);

// 00944D98..00944DF2: the heading the creator's vtable+11Ch receives, from the
// origin to the placed point: atan2(dz, dx) (00BF701A _CIatan2 with ST0 = dx,
// ST1 = dz) + pi/2 (00CE3830). The position goes to vtable+118h.
float spawn_heading_00944db8(const float origin[3], const float position[3]) noexcept;

// 00944A16..00944A3F, placement mode 1: the origin entity's yaw is
// atan2(m[F4h], m[ECh]) - pi/2, i.e. its world +z row (x at +ECh, z at +F4h),
// added to both angles. Resolves the "provisional" note in lua_binding_spawn.hpp.
float spawn_relative_yaw_00944a2c(float row_z_x, float row_z_z) noexcept;

// 00944EAD: 00438E10 against kSpawnAircraftClassName.
bool spawn_class_is_aircraft_00944ead(const std::string& class_name) noexcept;

// The whole of 00944680's placement: the branch the trailing boolean picks and
// the heading. ok is false only on 00944F20's nil exit.
struct SpawnResolvedPosition {
    bool ok{false};
    bool direct{false};
    int stage{0};
    float position[3]{0.0f, 0.0f, 0.0f};
    float heading{0.0f};
};
SpawnResolvedPosition lua_spawn_resolve_position(const SpawnPlacementRequest& request,
                                                 const SpawnWorldBox& box,
                                                 const std::vector<SpawnPlacementEntity>& entities,
                                                 SpawnPlacementHost& host);

}  // namespace bsp

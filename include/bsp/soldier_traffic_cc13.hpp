#pragma once

// Soldier / traffic behaviour 0048A010-0048C230 and the SoldierGlobals.lua loader
// 0084CB70. Packet cc13-soldier1; evidence and the gameplay verdict are in
// docs/CC13_SOLDIER_TRAFFIC.md. Every name is a hypothesis, not a recovered symbol.
// These are new C++ interfaces over injected hosts, not binary replacements:
// build-tested only, not wired into any game host.
//
// Three object families run through this window:
//   * the landing-troop group (E4h bytes, vtable 00CE66E0, constructor 004A0410,
//     created by 004A4520 from BSP_LandingShip_BeginLandingAtPad 0074A990). Its
//     members run from the landing ship to the pad, fire, die and despawn
//     (0048A6D0, 0048AF00, 0048B120, 0048BF80, 0048C090, 0048C190);
//   * the scene traffic runtime (170h, docs/SCENE_TRAFFIC_RUNTIME.md), its view
//     gate 0048A650 and the member clear 0048A5A0;
//   * the landing lane (vtable 00CE6A68, constructor 004A66B0, created for a unit
//     by 004A6830 from BSP_UnitInstance_SEntityInit), whose path 0048C230 rebuilds.
// 0048A010 and 0048A1E0 are member-entity routines (the clip test and the
// muzzle effect); 0048AEC0 has no reference in the image.
//
// GAMEPLAY VERDICT (docs/CC13_SOLDIER_TRAFFIC.md section 5): NOT purely ambient.
//   * A projectile or blast that kills a landing-troop member (00428420 from
//     BSP_Projectile_TraceSegmentAndImpact, and BSP_Explosion_NotifyBlastSites
//     00428800) calls 00480C20, which applies the member type's `Damage` (+1Ch) to
//     the group's landing ship through vtable +1ACh, BSP_UnitInstance_AddDamage
//     0095DA00 (outside multiplayer mode 2). Where the members are and which state
//     they are in, both driven here with stream-0 draws, decides those hits.
//   * luaMW_IsLandingTrafficActive (0089AF40) returns CommandBuilding slot +210h
//     006F3A20 -> 0048CBB0, which reads the group's ship (+DCh) and list-0 count
//     (+BCh). No script of this installation calls it (621 .lua, .scn/.dat/.def).
//   * 0048C230 sets the lane end +190h that the lane tick 0049A410 compares
//     against to call BSP_LandingShip_FinishLanding 0074AD90 (Lua LandingFinished).
//     No RNG there.
//   * 0048A010, 0048A1E0 and the fake-effect visuals are presentation.
//
// RNG: every draw in the window is BSP_Random_UniformFloatRange 00BD2F10 or
// BSP_RandomThreads_NextU32 00BD2FC0 with ECX = 0 (XOR ECX,ECX before each call):
// the shared stream 0. The image interleaves these with every other stream-0 user.
// Each draw goes through SoldierTrafficRandom with a site key. A host that wires
// this code must route the PRESENTATION sites to a keyed presentation stream and
// never to stream 0 (cc9 RNG rule); the GAMEPLAY-COUPLED sites must stay behind an
// OFF switch, because image parity would need them on stream 0 in image order and
// any other routing changes the troops' exposure to hits. kSoldierRngSites says
// which is which.
//
// Precision: the image keeps x87 intermediates in extended precision and rounds at
// each float store; this code computes in double and rounds at the same stores.
// Qword constants stay double, dword constants float.

#include <array>
#include <cstdint>
#include <list>

namespace bsp {
namespace soldier_traffic_cc13 {

struct Vec3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

// ---------------------------------------------------------------------------
// SoldierGlobals (20h bytes, singleton 00F87544 behind 00485520; constructor
// 00484700 installs vtable 00CE6458, clears +1Ch and calls 0084CB70).
// ---------------------------------------------------------------------------
struct SoldierGlobalsOffsets {
    static constexpr std::uint32_t kSize = 0x20;                    // 00485578 PUSH 20h
    static constexpr std::uint32_t kVtable = 0x00;                  // 00CE6458
    static constexpr std::uint32_t kShadowClipDistance = 0x04;      // 0084CCB1
    static constexpr std::uint32_t kFireEfxClipDistance = 0x08;     // 0084CCE6
    static constexpr std::uint32_t kClipDistance = 0x0C;            // 0084CD1B
    static constexpr std::uint32_t kMaxParatrooperGradient = 0x10;  // 0084CD67, radians
    static constexpr std::uint32_t kParachuteOpenAltitudeMin = 0x14; // 0084CD91
    static constexpr std::uint32_t kParachuteOpenAltitudeMax = 0x18; // 0084CDC6
    static constexpr std::uint32_t kLoaded = 0x1C;                  // byte, 0084CBA2
};

struct SoldierGlobals {
    float shadow_clip_distance{0.0f};
    float fire_efx_clip_distance{0.0f};
    float clip_distance{0.0f};
    float max_paratrooper_gradient{0.0f};  // radians
    float parachute_open_altitude_min{0.0f};
    float parachute_open_altitude_max{0.0f};
    bool loaded{false};
};

// The Lua side of 0084CB70. One method per native step; a host backs them with a
// private Lua state, as the image does (BSP_LuaStateOwner_Construct/Open(41h),
// two RunScriptWithOverrides, GetGlobals, Close).
struct SoldierGlobalsLuaHost {
    virtual ~SoldierGlobalsLuaHost() = default;
    virtual void open_state_00b6a020(std::uint32_t library_mask) = 0;   // 41h
    virtual void run_script_00b69d40(const char* path) = 0;
    // BSP_LuaObject_GetNumber 00B66270 on SoldierGlobals[key]; the x87 result.
    virtual double soldier_globals_number_00b66270(const char* key) = 0;
    virtual void close_state_00b669a0() = 0;
};

// 0084CB70, __fastcall(ECX = SoldierGlobals*). Runs once: returns at once when
// +1Ch is set, otherwise sets it first. Reads six keys of the global table
// `SoldierGlobals` from Scripts\datatables\SoldierGlobals.lua after
// Scripts\global\luaMW_init.lua.
void load_soldier_globals_0084cb70(SoldierGlobals& globals, SoldierGlobalsLuaHost& lua);

// 0084CD50..0084CD67: FMUL qword [00CE3D28] (3.1415927410125732, float pi widened),
// FDIV qword [00CE3D20] (180.0), FSTP dword.
float paratrooper_gradient_radians_0084cd50(double degrees) noexcept;

// ---------------------------------------------------------------------------
// Data the window reads. Offsets are the image's; the C++ structs are views.
// ---------------------------------------------------------------------------

// Soldier type: base 0048DC40 (vtable 00CE66DC, one row of LandingTroopClass[n].Classes),
// derived 0049FC80 (vtable 00CE691C) adds the animation name vectors (8-byte strings).
struct SoldierTypeOffsets {
    static constexpr std::uint32_t kClass = 0x04;          // named class, vslot +8 instantiates
    static constexpr std::uint32_t kFakeEffects = 0x08;    // pointer array, count at +0Ch
    static constexpr std::uint32_t kFakeEffectCount = 0x0C;
    static constexpr std::uint32_t kWeight = 0x14;         // 0048DCA2
    static constexpr std::uint32_t kFireChance = 0x18;     // 0048DCD5
    static constexpr std::uint32_t kDamage = 0x1C;         // 0048DD0C; read by 00428420/00428800
    static constexpr std::uint32_t kMinDistance = 0x20;    // 0048DD43
    static constexpr std::uint32_t kMaxDistance = 0x24;    // 0048DD7A
    static constexpr std::uint32_t kOutingDelay = 0x28;    // 0048DE70
    static constexpr std::uint32_t kMaxEntity = 0x2C;      // 0048DDAD
    static constexpr std::uint32_t kMinInRow = 0x30;       // 0048DDDC
    static constexpr std::uint32_t kMaxInRow = 0x34;       // 0048DE0B
    static constexpr std::uint32_t kIsSoldier = 0x38;      // byte; Type == "Soldier" (provisional)
    static constexpr std::uint32_t kDefence = 0x39;        // byte, 0048DE39
    static constexpr std::uint32_t kAnimRun = 0x3C;        // vector header, first +40h last +44h
    static constexpr std::uint32_t kAnimFire = 0x4C;       // first +50h last +54h
    static constexpr std::uint32_t kAnimDie = 0x5C;        // first +60h last +64h
    static constexpr std::uint32_t kAnimIdle = 0x6C;       // first +70h last +74h
};

// FakeEffects row, reader 00487770 (BSP_SoldierFakeEffect_ConstructFromLua):
// effect handle +0 ("ExplosionEfx"), MaxTime +4, MinTime +8, Radius +0Ch, Weight +10h.
struct FakeEffectRow {
    float max_time{0.0f};
    float min_time{0.0f};
    float radius{0.0f};
    float weight{0.0f};
};

struct SoldierTypeView {
    float fire_chance{0.0f};    // +18h
    float outing_delay{0.0f};   // +28h
    bool is_soldier{false};     // +38h
    bool defence{false};        // +39h
    const FakeEffectRow* fake_effects{nullptr};  // +08h
    std::uint32_t fake_effect_count{0};          // +0Ch
    std::uint32_t anim_run_count{0};   // (+44h - +40h) >> 3
    std::uint32_t anim_fire_count{0};  // (+54h - +50h) >> 3
    std::uint32_t anim_die_count{0};   // (+64h - +60h) >> 3
    std::uint32_t anim_idle_count{0};  // (+74h - +70h) >> 3
};

// Landing-troop group (E4h bytes), constructor 004A0410 reading LandingTroopClass[n].
struct LandingTroopOffsets {
    static constexpr std::uint32_t kSize = 0xE4;            // 004A4520 operator_new(E4h)
    static constexpr std::uint32_t kVtable = 0x00;          // 00CE66E0: 0048EFB0, 004A2FC0, 0048BF80
    static constexpr std::uint32_t kLastReleaseTime = 0x18; // 0048BF9D, [00F876A4]
    static constexpr std::uint32_t kSpeed = 0x24;           // "Speed"
    static constexpr std::uint32_t kMaxDeflection = 0x28;   // "MaxDeflection"
    static constexpr std::uint32_t kDistance = 0x2C;        // "Distance"
    static constexpr std::uint32_t kRunMinTime = 0x30;
    static constexpr std::uint32_t kRunMaxTime = 0x34;
    static constexpr std::uint32_t kFireMinTime = 0x38;
    static constexpr std::uint32_t kFireMaxTime = 0x3C;
    static constexpr std::uint32_t kDieChance = 0x40;
    static constexpr std::uint32_t kClassIndex = 0x44;      // ship class +814h
    static constexpr std::uint32_t kPathParent = 0x54;      // terrain owner for 0087FA20/0087FB90
    static constexpr std::uint32_t kAabbMargin = 0x98;      // 10.0
    static constexpr std::uint32_t kAabb = 0x9C;            // min +9Ch, max +A8h
    static constexpr std::uint32_t kMemberLists = 0xB4;     // three lists, 0Ch apart
    static constexpr std::uint32_t kPad = 0xD8;             // pad record (four path slots, +14h counter)
    static constexpr std::uint32_t kShip = 0xDC;            // landing ship; 0048C190/0048CB30 clear it
    static constexpr std::uint32_t kBase = 0xE0;            // CommandBuilding; vslot +210h
};

struct LandingTroopView {
    float speed{0.0f};          // +24h
    float run_min_time{0.0f};   // +30h
    float run_max_time{0.0f};   // +34h
    float fire_min_time{0.0f};  // +38h
    float fire_max_time{0.0f};  // +3Ch
    float die_chance{0.0f};     // +40h
};

// One member node of the three lists (std::list node; payload from +8h).
struct TroopMemberOffsets {
    static constexpr std::uint32_t kType = 0x08;
    static constexpr std::uint32_t kEntity = 0x0C;
    static constexpr std::uint32_t kPathPosition = 0x14;
    static constexpr std::uint32_t kPath = 0x18;            // +1Ch end, +28h length scale
    static constexpr std::uint32_t kState = 0x34;
    static constexpr std::uint32_t kTimer = 0x38;
    static constexpr std::uint32_t kFakeEffectTimer = 0x3C;
    static constexpr std::uint32_t kHoldTimer = 0x40;
    static constexpr std::uint32_t kFade = 0x44;
};

// State word +34h. 0048B120's jump table 0048BF60 has seven entries.
enum class TroopMemberState : std::int32_t {
    Waiting = 0,  // outing delay +40h counts down at the path start
    Running = 1,  // advances +14h by Speed * dt
    Firing = 2,   // stationary, timer +38h
    Dying = 3,    // die animation, fade 6..1 s, then removed (Defence members respawn)
    Arrived = 4,  // one frame: placed at the path end, then Idle
    Idle = 5,     // at the path end; fires (soldier) or muzzle-rolls (vehicle)
    Leaving = 6,  // fade and removal
};

struct TroopMember {
    void* entity{nullptr};       // +0Ch, host handle
    const SoldierTypeView* type{nullptr};  // +08h
    float path_position{0.0f};   // +14h
    float path_end{0.0f};        // [+18h]+1Ch
    float path_length_scale{0.0f};  // [+18h]+28h
    TroopMemberState state{TroopMemberState::Waiting};
    float timer{0.0f};           // +38h
    float fake_effect_timer{0.0f};  // +3Ch
    float hold_timer{0.0f};      // +40h
    float fade{0.0f};            // +44h
};

using TroopMemberLists = std::array<std::list<TroopMember>, 3>;

// ---------------------------------------------------------------------------
// RNG sites. All are stream 0 in the image (ECX = 0).
// ---------------------------------------------------------------------------
enum class SoldierRngSite : std::uint8_t {
    FakeEffectPick_0048a7d7,      // U(0, sum of weights)
    FakeEffectTimer_0048a880,     // U(MinTime, MaxTime)
    FakeEffectOffsetZ_0048ac00,   // U(-Radius, Radius)
    FakeEffectOffsetX_0048ac47,   // U(-Radius, Radius)
    FakeEffectDieRoll_0048ad8b,   // U(0, 1) against dt/len * Speed * DieChance
    FakeEffectDieAnim_0048ae02,   // NextU32 % AnimDie count
    UnreferencedFireRoll_0048aed0, // 0048AEC0, no caller
    ShipGoneHold_0048b253,        // U(3, 15)
    ShipGoneDieTimer_0048b2ac,    // U(8, 14)
    ShipGoneDieAnim_0048b2fa,     // NextU32 % AnimDie count
    OutingRunAnim_0048b52c,       // NextU32 % AnimRun count
    WithdrawTimer_0048b91b,       // U(10, 20)
    FireTimer_0048b98f,           // U(FireMinTime, FireMaxTime)
    FireAnim_0048b9bc,            // NextU32 % AnimFire count
    VehicleMuzzleRoll_0048ba4d,   // U(0, 1) against FireChance * dt
    FiringMuzzleRoll_0048baaa,    // U(0, 1) against FireChance * dt
    RunTimer_0048bb01,            // U(RunMinTime, RunMaxTime)
    ResumeRunAnim_0048bbf0,       // NextU32 % AnimRun count
    DefenceRespawnDelay_0048bd9a, // U(0, OutingDelay)
    ShipLeftTimer_0048c1fd,       // U(3, 10)
};

struct SoldierRngSiteInfo {
    SoldierRngSite site;
    std::uint32_t call_address;
    bool gameplay_coupled;  // false: presentation, route to a keyed presentation stream
    const char* key;        // suggested presentation/keyed-stream key
};

inline constexpr std::array<SoldierRngSiteInfo, 20> kSoldierRngSites{{
    {SoldierRngSite::FakeEffectPick_0048a7d7, 0x0048A7D7u, true, "fx_pick"},
    {SoldierRngSite::FakeEffectTimer_0048a880, 0x0048A880u, true, "fx_timer"},
    {SoldierRngSite::FakeEffectOffsetZ_0048ac00, 0x0048AC00u, false, "fx_dz"},
    {SoldierRngSite::FakeEffectOffsetX_0048ac47, 0x0048AC47u, false, "fx_dx"},
    {SoldierRngSite::FakeEffectDieRoll_0048ad8b, 0x0048AD8Bu, true, "fx_die"},
    {SoldierRngSite::FakeEffectDieAnim_0048ae02, 0x0048AE02u, false, "fx_die_anim"},
    {SoldierRngSite::UnreferencedFireRoll_0048aed0, 0x0048AED0u, false, "fire_roll"},
    {SoldierRngSite::ShipGoneHold_0048b253, 0x0048B253u, true, "gone_hold"},
    {SoldierRngSite::ShipGoneDieTimer_0048b2ac, 0x0048B2ACu, true, "gone_die"},
    {SoldierRngSite::ShipGoneDieAnim_0048b2fa, 0x0048B2FAu, false, "gone_die_anim"},
    {SoldierRngSite::OutingRunAnim_0048b52c, 0x0048B52Cu, false, "out_run_anim"},
    {SoldierRngSite::WithdrawTimer_0048b91b, 0x0048B91Bu, true, "withdraw"},
    {SoldierRngSite::FireTimer_0048b98f, 0x0048B98Fu, true, "fire_timer"},
    {SoldierRngSite::FireAnim_0048b9bc, 0x0048B9BCu, false, "fire_anim"},
    {SoldierRngSite::VehicleMuzzleRoll_0048ba4d, 0x0048BA4Du, false, "veh_muzzle"},
    {SoldierRngSite::FiringMuzzleRoll_0048baaa, 0x0048BAAAu, false, "muzzle"},
    {SoldierRngSite::RunTimer_0048bb01, 0x0048BB01u, true, "run_timer"},
    {SoldierRngSite::ResumeRunAnim_0048bbf0, 0x0048BBF0u, false, "run_anim"},
    {SoldierRngSite::DefenceRespawnDelay_0048bd9a, 0x0048BD9Au, true, "respawn"},
    {SoldierRngSite::ShipLeftTimer_0048c1fd, 0x0048C1FDu, true, "ship_left"},
}};

struct SoldierTrafficRandom {
    virtual ~SoldierTrafficRandom() = default;
    // 00BD2F10 with ECX = 0 in the image.
    virtual float uniform_00bd2f10(SoldierRngSite site, float low, float high) = 0;
    // 00BD2FC0 with ECX = 0 in the image.
    virtual std::uint32_t next_u32_00bd2fc0(SoldierRngSite site) = 0;
};

// ---------------------------------------------------------------------------
// 0048A010: member-entity clip test, __thiscall(entity, const Vec3* position), RET 4.
// ---------------------------------------------------------------------------
struct EntityClipInputs {
    bool camera_present{false};       // [game+1ED4h] != 0
    bool visible_flag{false};         // entity+5Ch
    bool model_radius_present{false}; // entity+178h ->+160h ->+0Ch non-null
    float model_radius{0.0f};         // that object's vslot +48h result +0Ch
    Vec3 position{};                  // the argument
    Vec3 camera_position{};           // camera +FCh..+104h after the pose refresh
    float fov_aspect_product{0.0f};   // BSP_Camera_GetFovAspectProduct 00B6FC30 ([game+19FCh])
    float length{0.0f};               // 0042B2F0 over position - camera (the x87 result)
    float clip_distance{0.0f};        // SoldierGlobals +0Ch
    float clip_bias{0.0f};            // entity+17Ch
};

struct EntityClipQuery {
    // The box handed to 004B48E0 (position -/+ radius), computed before the
    // distance test but only queried when the distance test passes.
    Vec3 box_min{};
    Vec3 box_max{};
    bool distance_passes{false};
};

enum class EntityClipAction : std::uint8_t { None, Show_vslot11c, Hide_vslot118 };

// Returns the radius the box uses: entity model radius, else 10.0 [00CE38B8].
float entity_clip_radius_0048a02e(const EntityClipInputs& in) noexcept;
EntityClipQuery entity_clip_query_0048a010(const EntityClipInputs& in) noexcept;
// Final decision once the host has answered 004B48E0 (only asked when
// distance_passes). Without a camera the routine returns entity+5Ch untouched.
bool entity_clip_result_0048a010(const EntityClipInputs& in, const EntityClipQuery& query,
                                 bool box_in_view, EntityClipAction& action) noexcept;

// 0048A010 as a whole body (cc13-D1): every image call goes through the host,
// in the image's order. `position` is the [ESP+4] argument.
struct EntityClipHost {
    virtual ~EntityClipHost() = default;
    virtual bool camera_present_1ed4() = 0;                 // [[00E188A8]+1ED4h] != 0
    virtual bool visible_flag_5c() = 0;                     // entity+5Ch
    // [[entity+178h]+160h]+0Ch non-null -> its vslot +48h, MOVSS [EAX+0Ch].
    virtual bool model_radius_vslot48(float& radius) = 0;
    virtual bool camera_pose_valid_c8() = 0;                // camera+C8h
    virtual void refresh_camera_pose_00414db0() = 0;
    virtual Vec3 camera_position_fc() = 0;                  // camera+FCh..+104h
    virtual float fov_aspect_product_00b6fc30() = 0;        // ECX = [game+19FCh]
    virtual float clip_distance_00485520() = 0;             // SoldierGlobals +0Ch
    virtual float length_0042b2f0(const Vec3& v) = 0;
    virtual float clip_bias_17c() = 0;                      // entity+17Ch
    virtual bool box_in_view_004b48e0(const Vec3& box_min, const Vec3& box_max) = 0;
    virtual void show_vslot11c() = 0;
    virtual void hide_vslot118() = 0;
};
bool entity_clip_0048a010(const Vec3& position, EntityClipHost& host);

// ---------------------------------------------------------------------------
// 0048A1E0: member muzzle effect, __thiscall(entity, int variant), RET 4.
// ---------------------------------------------------------------------------
struct MuzzleEffectInputs {
    bool class_fire_point_valid{false};  // [entity+170h]+24h (NamedClassBase_ReadLua 0048F670)
    bool effect_present{false};          // entity+180h non-null
    bool effect_ready{false};            // 00865FE0(effect) result
    bool effect_flag9{false};            // effect+9
    bool camera_present{false};
    float distance{0.0f};                // 0042B2F0 over entity - camera
    float fov_aspect_product{0.0f};
    float fire_efx_clip_distance{0.0f};  // SoldierGlobals +8h
    bool is_kind_3a{false};              // entity vslot +5Ch(3Ah)
    int variant{0};                      // argument: 0, or AnimFire index + 1
    Vec3 fire_point{};                   // [entity+170h]+18h..+20h
};

struct MuzzleEffectPlan {
    bool release_previous{false};   // 00484620 on entity+180h (effect+9 set)
    bool spawn{false};
    Vec3 euler{};                   // 0047C2A0 angles: (0, -pi/2, 0) kind 3Ah, else (0, pi/2, 0)
    Vec3 translation{};             // matrix row 3
    bool post_rotate_y_pi{false};   // kind 3Ah: 0047C210 with Euler (pi, 0, 0)
    bool set_effect_flag9{false};   // non-3Ah branch sets new effect+9 = 1 (0048A574)
};

// The effect is BSP_PointEffect_CreateWithParentMatrix 008687C0 with the class
// effect +14h, parent [[entity+178h]+160h]+0Ch, flags (1, 0, 0), stored at entity+180h.
MuzzleEffectPlan plan_muzzle_effect_0048a1e0(const MuzzleEffectInputs& in) noexcept;

// 0048A1E0 as a whole body (cc13-D1) over host calls, in the image's order.
struct MuzzleEffectHost {
    virtual ~MuzzleEffectHost() = default;
    virtual bool class_fire_point_valid_24() = 0;   // [entity+170h]+24h
    virtual bool effect_present_180() = 0;          // entity+180h non-null
    virtual bool effect_finished_00865fe0() = 0;
    virtual bool effect_flag9() = 0;                // [entity+180h]+9
    virtual void release_effect_00484620() = 0;     // ECX = &entity+180h, argument 0
    virtual bool camera_present_1ed4() = 0;
    virtual bool camera_pose_valid_c8() = 0;
    virtual void refresh_camera_pose_00414db0() = 0;
    virtual bool entity_pose_valid_c8() = 0;
    virtual void refresh_entity_pose_00414db0() = 0;
    virtual Vec3 entity_position_fc() = 0;
    virtual Vec3 camera_position_fc() = 0;
    virtual float fov_aspect_product_00b6fc30() = 0;
    virtual float fire_efx_clip_distance_00485520() = 0;   // SoldierGlobals +8h
    virtual float length_0042b2f0(const Vec3& v) = 0;
    virtual bool is_kind_3a_vslot5c() = 0;
    virtual Vec3 class_fire_point_18() = 0;          // [entity+170h]+18h..+20h
    // 0047C2A0 + 004134F0 build M from `euler`; row 3 = `translation`; with
    // `post_rotate` 0047C2A0 + 0047C210 multiply M by Euler (pi, 0, 0); then
    // 00440490 copies the class effect +14h, 008687C0(EDX = [[entity+178h]+160h]+0Ch,
    // &M, 1, 0, 0), 004845D0 stores the result at entity+180h, 00440A30 frees the temp.
    virtual void create_effect_008687c0(const Vec3& euler, const Vec3& translation,
                                        bool post_rotate) = 0;
    virtual void set_new_effect_flag9() = 0;          // 0048A574, when entity+180h is non-null
};
void muzzle_effect_0048a1e0(int variant, MuzzleEffectHost& host);

// 0048AEC0 (no reference in the image): U(0,1) < FireChance * dt -> 0048A1E0(0);
// an unordered compare also calls (JAE skips).
bool unreferenced_fire_roll_0048aec0(SoldierTrafficRandom& rng, float fire_chance, float dt);

// ---------------------------------------------------------------------------
// 0048A650: traffic runtime slot 3, the view gate. __fastcall(runtime).
// ---------------------------------------------------------------------------
struct TrafficViewGateInputs {
    bool box_in_view{false};          // 004B48E0(game, runtime+150h)
    bool anchor_present{false};       // runtime+168h
    std::int32_t anchor_side{0};      // [runtime+168h]+54h
    std::int32_t side{0};             // runtime+16Ch
    bool local_player_slot_28_zero{false};  // [game+18CCh + [game+18ECh]*4]+28h == 0
};

struct TrafficViewGateResult {
    bool tick{false};
    bool remap_00487470{false};  // side changed: 00487470 re-runs the template remap
    std::int32_t side{0};        // new +16Ch
};

TrafficViewGateResult traffic_view_gate_0048a650(const TrafficViewGateInputs& in) noexcept;

// 0048A650 as a whole body (cc13-D1) over host calls. 00487470 (the template
// remap, ECX = runtime) is not reconstructed and stays a host call.
struct TrafficViewGateHost {
    virtual ~TrafficViewGateHost() = default;
    virtual bool box_in_view_004b48e0() = 0;          // (game, runtime+150h)
    virtual bool anchor_present_168() = 0;
    virtual std::int32_t anchor_side_54() = 0;        // [runtime+168h]+54h
    virtual std::int32_t side_16c() = 0;
    virtual void set_side_16c(std::int32_t side) = 0;
    virtual bool local_player_slot_28_zero() = 0;     // [game+18CCh + [game+18ECh]*4]+28h == 0
    virtual void remap_00487470() = 0;
};
bool traffic_view_gate_0048a650(TrafficViewGateHost& host);

// ---------------------------------------------------------------------------
// Member list hosts.
// ---------------------------------------------------------------------------
enum class TroopAnimSet : std::uint8_t { Run, Fire, Die, Idle };

struct TroopMemberHost {
    virtual ~TroopMemberHost() = default;
    // 004B0F80(entity, &set[index], random_start, force). With random_start the
    // image draws U(0, duration) from stream 0 inside 004B0F80 (004B102F).
    virtual void play_anim_004b0f80(TroopMember& m, TroopAnimSet set, std::uint32_t index,
                                    bool random_start, bool force) = 0;
    // 00B6DA70 on [[entity+178h]+160h]+0Ch.
    virtual void set_visibility_00b6da70(TroopMember& m, float factor, bool flag) = 0;
    virtual void set_entity_byte_184(TroopMember& m, bool value) = 0;
    virtual void muzzle_effect_0048a1e0(TroopMember& m, int variant) = 0;
    virtual bool entity_is_kind_3a(TroopMember& m) = 0;          // vslot +5Ch(3Ah)
    virtual void stop_entity_effect_00867b10(TroopMember& m) = 0; // entity+180h, then +9 = 1
    // Case 4: sample the path at `t`, build the basis (0085DAD0) with up
    // [00F8758C..94] or, for vehicles, the terrain normal 0087FB90; y from
    // 0087FA20 minus the path parent's +A8h; write entity+74h..+B0h, clear +C8h
    // and +10Ch and invalidate the child poses.
    virtual void place_at_path_end_0048b626(TroopMember& m, float t, bool align_to_terrain) = 0;
    // vslot +114h(0), then BSP_MissionEntity_Kill 00926D90(cause 2).
    virtual void release_and_kill_entity(TroopMember& m) = 0;
};

// 0048AEC0 with its call (cc13-D1): the roll, then host 0048A1E0(member, 0).
void fire_roll_0048aec0(TroopMember& m, float dt, SoldierTrafficRandom& rng,
                        TroopMemberHost& host);

struct LandingTroopState {
    LandingTroopView view{};
    bool ship_attached{false};   // +DCh != 0
    bool base_present{false};    // +E0h != 0
    // vslot +210h of the CommandBuilding at +E0h (006F3A20): any pad's group has
    // a ship and list-0 members. Asked by the host, once per call site.
    bool (*base_landing_active)(void* ctx){nullptr};
    void* base_ctx{nullptr};
    TroopMemberLists lists{};
};

// 0048B120, __thiscall(troop, float dt), RET 4: the member state machine.
void tick_troop_members_0048b120(LandingTroopState& troop, float dt, TroopMemberHost& host,
                                 SoldierTrafficRandom& rng);

// 0048A6D0, __thiscall(troop, float dt), RET 4: the fake effects and the die roll.
struct FakeEffectHost {
    virtual ~FakeEffectHost() = default;
    virtual Vec3 member_world_position(TroopMember& m) = 0;  // entity +FCh..+104h, refreshed
    virtual bool camera_present() = 0;
    virtual Vec3 camera_world_position() = 0;
    virtual float fov_aspect_product_00b6fc30() = 0;
    virtual float fire_efx_clip_distance() = 0;              // SoldierGlobals +8h
    // 008687C0: effect = FakeEffects[index]+0 (retained), parent the member model,
    // identity matrix with translation (dx, 0, dz); new effect +9 = 1, then released.
    virtual void spawn_fake_effect_008687c0(TroopMember& m, std::uint32_t index, float dx,
                                            float dz) = 0;
    virtual void play_die_anim_004b0f80(TroopMember& m, std::uint32_t index) = 0;  // (.., 0, 0)
};

void tick_fake_effects_0048a6d0(LandingTroopState& troop, float dt, FakeEffectHost& host,
                                SoldierTrafficRandom& rng);

// 0048AF00, __fastcall(troop): instantiate missing member entities.
struct MemberSpawnHost {
    virtual ~MemberSpawnHost() = default;
    virtual void* instantiate_class_vslot8(TroopMember& m) = 0;   // [type+4] vslot +8
    // vslot +98h with the parent of the pad's first path (007ACA30 twice, +30h, +3Ch)
    virtual void attach_to_pad_path_parent_vslot98(TroopMember& m) = 0;
    virtual void set_owner_vslot114(TroopMember& m) = 0;   // argument = the troop
    virtual void activate_vslota0(TroopMember& m) = 0;
    virtual void play_anim_004b0f80(TroopMember& m, TroopAnimSet set, std::uint32_t index,
                                    bool random_start, bool force) = 0;
};

void spawn_missing_members_0048af00(LandingTroopState& troop, MemberSpawnHost& host);

// Path-ownership helpers shared by 0048BF80 and 0048C090: a soldier member whose
// path is not the pad's first path (+1E4h) and not null owns it (vslot 0(1)).
struct MemberPathHost {
    virtual ~MemberPathHost() = default;
    virtual bool member_owns_path(TroopMember& m) = 0;
    virtual void delete_member_path(TroopMember& m) = 0;
    virtual void clear_owner_vslot114(void* entity) = 0;   // entity vslot +114h(0)
    virtual void kill_entity_00926d90(TroopMember& m) = 0;  // cause 2
};

// 0048BF80, troop vslot +8 (__thiscall(troop, entity), RET 4): a member entity
// is released by someone else. Stamps +18h with [00F876A4], clears the entity's
// owner, and erases the first node of each list that holds it (no kill).
void release_member_entity_0048bf80(LandingTroopState& troop, void* entity,
                                    float game_time_00f876a4, float& last_release_time_18,
                                    MemberPathHost& host);

// 0048C090, __fastcall(troop): tear down every member entity (owner cleared,
// killed with cause 2, node +0Ch = 0); the nodes stay.
void kill_all_members_0048c090(LandingTroopState& troop, MemberPathHost& host);

// 0048C190, __fastcall(troop): the ship left. +DCh = 0; every member fades from
// 0.98 [00CE6650] with timer U(3, 10).
void ship_left_0048c190(LandingTroopState& troop, SoldierTrafficRandom& rng);

// ---------------------------------------------------------------------------
// 0048A5A0, __fastcall(runtime): kill every member of the traffic list at
// +6Ch (sentinel +70h, count +74h): vslot +114h(0), Kill(2), node+8 = 0, then
// unlink and free all nodes. Shown here as the count it leaves.
// ---------------------------------------------------------------------------
struct TrafficMemberHost {
    virtual ~TrafficMemberHost() = default;
    virtual void release_and_kill(void* entity) = 0;
};
void clear_traffic_members_0048a5a0(std::list<void*>& members, TrafficMemberHost& host);

// ---------------------------------------------------------------------------
// 0048C230, __fastcall(lane): rebuild the landing lane onto the island under the
// ship's landing point. Called once from the lane tick 0049A410 when lane+180h is
// clear and ship+118Bh is set; its +190h is the distance at which 0049A410 calls
// BSP_LandingShip_FinishLanding.
// ---------------------------------------------------------------------------
struct LandingLaneInputs {
    Vec3 landing_point{};     // ship +11B4h..+11BCh
    Vec3 ship_forward{};      // ship +94h..+9Ch
    float start_184{0.0f};    // lane +184h
    float scale_188{0.0f};    // lane +188h
    float length_18c{0.0f};   // lane +18Ch
    float end_190{0.0f};      // lane +190h before
    float width_3c{0.0f};     // lane +3Ch
    Vec3 margin_138{};        // lane +138h..+140h
    std::int32_t first_index_24{0};
    std::int32_t last_index_28{0};
};

struct LandingLaneHost {
    virtual ~LandingLaneHost() = default;
    // The first entity of [game+19CCh]+34Ch (links +4h, entity +8h) answering
    // IsKindOf(44h) whose 0087FA50 height at the landing point exceeds -1000.0
    // [00CE6658] becomes lane+D8h (0048C2E3). Returns lane+D8h afterwards: the
    // previous value when nothing matched.
    virtual void* select_island_d8(const Vec3& landing_point, bool& found) = 0;
    virtual float island_height_0087fa50(void* island, float x, float z) = 0;
    // operator_new(210h), memset, 0047B660(0), vslot +98h(island, island+30h, identity);
    // copy the old path points (007AF800 to world, 007AD330 append).
    virtual void begin_new_path_copying_old(void* island) = 0;
    virtual void append_point_007ad330(const Vec3& p) = 0;
    virtual std::int32_t point_count() = 0;
    virtual float length_at_007b05e0(std::int32_t index) = 0;   // x87 result
    virtual Vec3 point_to_world_007af800(std::int32_t index) = 0;
    virtual void install_new_path() = 0;      // lane+20h = path+1E4h
    virtual void clear_members_0048a5a0() = 0;
};

struct LandingLaneResult {
    bool island_found{false};
    float end_190{0.0f};
    std::int32_t last_index_28{0};
    Vec3 aabb_min_19c{};
    Vec3 aabb_max_1a8{};
};

LandingLaneResult rebuild_landing_lane_0048c230(const LandingLaneInputs& in, LandingLaneHost& host);

}  // namespace soldier_traffic_cc13
}  // namespace bsp

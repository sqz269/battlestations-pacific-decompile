#pragma once

// Ocean, rain and effect-manager slice of GGame::OnMove (004E4A40).
// Addresses: 00865CF0, 00865AB0, 00BBDDD0, 00867EE0, 004D1100, 004B6260, 004BCAA0.
// Evidence and uncertainty are recorded in docs/GAME_WORLD_OCEAN.md.
//
// Nothing here is a binary-compatible replacement. The native routines are
// __thiscall members of classes whose full layouts are not recovered; the
// structs below project only the fields the reconstructed rules touch.

namespace bsp {

// Three packed floats, the shape every vector argument in this slice uses.
// Distinct from Vec3d in bsp/math.hpp, which models the double-precision
// helpers at 00401C20..00401CF0.
struct OceanVec3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

// 004F9B30, __fastcall(out = ECX, a = EDX, b = [ESP+4]), RET 4. Standard
// right-handed cross product; the operand order below matches the body.
OceanVec3 cross_004f9b30(const OceanVec3& a, const OceanVec3& b) noexcept;

// 00419440, __thiscall(v = ECX), result in ST0.
float length_00419440(const OceanVec3& v) noexcept;

// 00419510, __fastcall(out = ECX, v = EDX), returns out. A length of exactly
// zero or less yields the zero vector rather than a division; the ocean frame
// reset below depends on that.
OceanVec3 normalize_00419510(const OceanVec3& v) noexcept;

// 00D7A24C, the 1.0f the ocean writes into its scale field and its reset axes.
inline constexpr float kOceanUnit = 1.0f;

// 00CE3800, compared against the length of the freshly normalised up axis at
// 00BBD3DE. Because the value tested is a normalize result it is 1.0f or 0.0f,
// so any threshold in (0,1] selects the degenerate case; 0.5f is the literal.
inline constexpr float kOceanFrameDegenerateEpsilon = 0.5f;

// Double 30.0 at 00CE7630, folded into the shore-wave scroll step at 00BBEC5D.
// Loaded as a double and stored back as a float, which rounds once; 30.0 is
// exactly representable so a float multiply is equivalent.
inline constexpr float kShoreWaveScrollScale = 30.0f;

// The orthonormal basis 00BBD310 maintains on the ocean object.
struct OceanFrame {
    OceanVec3 right{}; // +44h, rebuilt from the reference axis and up
    OceanVec3 up{};    // +50h, re-orthogonalised against the reference axis
    float scale{kOceanUnit};   // +5Ch, rewritten to 1.0f every tick
    int visible_layer_count{0}; // +90h, cleared every tick, refilled by the layers
};

// Fields of the ocean object that the tick reads or writes. The object is the
// pointer at owner+3Ch, where owner is game+19E8h (00BBDDD0 dereferences it).
struct OceanState {
    bool enabled{false};      // +98h; false takes the hide path instead
    float layer_scale{0.0f};  // +34h, passed to every layer as its third argument
    int layer_count{0};       // +1Ch, count for the layer array at +18h
    OceanFrame frame{};
};

// 00BBD310 frame step. The reference axis is the row at transform+110h of the
// node passed in as the first argument (game+19FCh at the OnMove call site).
// Gram-Schmidt: up becomes the component of the previous up orthogonal to the
// reference axis, right becomes the third basis vector. A degenerate result
// resets to the canonical axes. Returns false when the ocean is disabled, in
// which case the native code only hides the scene node at +30h.
bool ocean_orthonormalize_00bbd310(OceanState& ocean, const OceanVec3& reference_axis) noexcept;

// 00BBEC06..00BBECA7, the only place the ocean tick spends the frame delta.
// Each shore-wave layer accumulates a world-space scroll offset at layer+1B8h
// by direction * delta * speed * 30.0f, then repositions its node to
// world position + scroll. Direction comes from 00BBDD60 and speed from
// 00BBDD70 on the layer's wave source.
OceanVec3 shore_wave_scroll_step_00bbec06(
    const OceanVec3& direction, float speed, float delta) noexcept;
void advance_shore_wave_scroll_00bbec06(
    OceanVec3& scroll, const OceanVec3& direction, float speed, float delta) noexcept;

// Fields of one effect instance that 00867D00 advances. Positions are latched
// from the instance's own scene node at instance+110h.
struct EffectSampleState {
    float age{0.0f};              // +80h, advanced by the delta unconditionally
    float sample_timer{0.0f};     // +48h
    float sample_interval{0.0f};  // +4Ch
    OceanVec3 previous_position{}; // +30h
    OceanVec3 current_position{};  // +3Ch
    OceanVec3 velocity{};          // +5Ch
    OceanVec3 displacement{};      // +68h
    int sample_count{0};          // +88h, the derived outputs need it above 1
    bool track_velocity{false};     // +28h
    bool track_displacement{false}; // +2Ch
};

// 00867D00 sampling rule. sampled_position is what the instance's node reports
// this frame; it is only consumed when the interval elapses. Returns true when
// a new sample was latched, which is also when the timer resets to zero.
bool advance_effect_sample_00867d00(
    EffectSampleState& state, float delta, const OceanVec3& sampled_position) noexcept;

// 004B6260, __thiscall on game+1EF0h (the network session object), returns AL.
// True iff session+F4h is non-null and session+29Ch is clear.
bool session_counts_mission_004b6260(bool session_object_present, bool session_flag_29c) noexcept;

// The pair of counters 004BCAA0 maintains at game+740h and game+73Ch through
// the subobject at game+650h (+F0h and +ECh of it).
struct MissionCounterState {
    int active{0}; // +740h
    int total{0};  // +73Ch
};

// 004BCAA0, __thiscall(game, char add), RET 4. Adding bumps both counters;
// removing decrements the active count with an unsigned floor at zero and
// leaves the total alone. The native body first checks the global at 00F8A2FC
// and its virtual +198h, then notifies through 007FA1B0.
void adjust_mission_counters_004bcaa0(MissionCounterState& counters, bool add) noexcept;

// The one-shot latch at game+1EE7h.
struct MissionStartLatch {
    bool counted{false}; // game+1EE7h
};

// Integration boundary. One method per native call site in this slice, in
// frame order. No default implementations: nothing here stands in for
// unrecovered game behaviour.
struct WorldOceanHost {
    virtual ~WorldOceanHost() = default;

    // --- one-shot at 004E4E00, inside the game state 0Dh block ---
    // 004B6260, ECX = game+1EF0h.
    virtual bool network_session_counts_mission() = 0;
    // game+624h != 0 at 004E4E18; suppresses the count.
    virtual bool mission_start_suppressed() = 0;
    // 004BCAA0(1), ECX = game.
    virtual void record_mission_started() = 0;

    // --- ocean and rain, gated on game+19E8h at 004E52F9 ---
    // game+19E8h != 0.
    virtual bool ocean_manager_present() = 0;
    // 00865CF0, a magic-static guarded by bit 0 of 00F875FC; returns the
    // weather descriptor at 00F875C0, constructed by 00865B80 with the
    // texture name "raindrop.tga".
    virtual void* weather_effect_descriptor() = 0;
    // 00865AB0, ECX = the descriptor, no stack arguments. Runs only when
    // descriptor+38h holds a live emitter. The native body re-reads the delta
    // from game+21F0h itself; it is passed here so the value is explicit.
    virtual void update_rain_spawner(void* descriptor, float scaled_delta) = 0;
    // 00BBDDD0(game+19FCh, delta), ECX = game+19E8h, RET 8.
    virtual void update_ocean(float scaled_delta) = 0;

    // --- calls owned by other packets that sit between the two blocks ---
    // 00740E10 (cDecalManager::Update), 0094C8F0 and 008EB110 at
    // 004E532A..004E535A. Modelled only so the tick order is preserved.
    virtual void updates_between_ocean_and_effects(float scaled_delta) = 0;

    // --- effect manager ---
    // 004D1100, a locked lazy singleton cached in 00F8765C, 28h bytes,
    // constructed by 004CF700.
    virtual void* effect_manager() = 0;
    // 00867EE0(delta, game+19FCh), ECX = the singleton, RET 8.
    virtual void update_effects(void* manager, float scaled_delta) = 0;
};

// Inputs the surrounding frame supplies.
struct WorldOceanTickInputs {
    int game_state{0};        // game+5D4h; the one-shot needs 0Dh
    float scaled_delta{0.0f}; // game+21F0h
};

// 004E4E00..004E4E2A. Runs once per entry into game state 0Dh and counts the
// mission as started. Despite sitting in the same in-mission block, it arms a
// session statistic, not the ocean; see docs/GAME_WORLD_OCEAN.md.
void arm_mission_start_latch_004e4e00(
    MissionStartLatch& latch, const WorldOceanTickInputs& inputs, WorldOceanHost& host);

// 004E52F9..004E5377, the ocean, rain and effect-manager portion of the world
// tick. Called from inside the simulation gate, after the award trackers and
// before the bot update.
void run_ocean_and_effects_tick(const WorldOceanTickInputs& inputs, WorldOceanHost& host);

} // namespace bsp

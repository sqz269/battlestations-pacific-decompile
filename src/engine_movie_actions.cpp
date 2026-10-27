#include "bsp/engine_movie_actions.hpp"

#include <cctype>

namespace bsp {

namespace {

bool equals_ci(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) !=
            std::tolower(static_cast<unsigned char>(b[i])))
            return false;
    }
    return true;
}

bool flag(const char* label) { return label && *label; }

} // namespace

// Labels and option lists in the argument order of 0045D480, recovered from
// the string pushes before each 00460AD0 call (arguments are built right to left).
const std::array<MovieActionTypeInfo, 32> kMovieActionTypes{{
    {"sSetTarget", "Target:", {"", "", ""}, "", "", "", true},
    {"cSetParent", "Parent:", {"ConvTime:", "", ""}, "", "", "Offset:", true},
    {"pEngineFailure", "", {"", "", ""}, "Destroy|Blown|Kill", "Type:", "", true},
    {"pForceRelease", "", {"", "", ""}, "", "", "", true},
    {"", "", {"", "", ""}, "", "", "", false},
    {"cUnParent", "", {"ConvTime:", "", ""}, "", "", "", true},
    {"sForceDepthCharge", "", {"PlatfrmID:", "", ""}, "", "", "", true},
    {"pSetGunFire", "", {"", "", ""}, "OpenFire|HoldFire", "On/Off:", "", true},
    {"uTogglePeriscope", "", {"", "", ""}, "Out|In", "In/out:", "", true},
    {"sForceArtilleryAngle", "", {"V.angle:", "H.angle:", ""}, "", "", "", true},
    {"sForceArtilleryTarget", "Target:", {"", "", ""}, "", "", "", true},
    {"uForceTorpedo", "", {"PlatfrmID:", "", ""}, "", "", "", true},
    {"aSetGunFireEnabled", "", {"", "", ""}, "All|AA_Flak|Artillery|Torpedo|DC", "Weapon:", "", true},
    {"aSetGunFireDisabled", "", {"", "", ""}, "All|AA_Flak|Artillery|Torpedo|DC", "Weapon:", "", true},
    {"aSetGunFireTargetOnly", "", {"", "", ""}, "TargetOnly|AllEnemy", "On/Off:", "", true},
    {"aTurnGun", "", {"GunID:", "H.Angle", "V.Angle"}, "", "", "", true},
    {"uSetDepth", "", {"", "", ""}, "Surface|Periscope|Underwater|Maxdepth", "Depth:", "", true},
    {"sForceAAFlak", "", {"PlatfrmID:", "", ""}, "", "", "", true},
    {"cFOV", "", {"FOV:", "", ""}, "", "", "", true},
    {"cBlackOut", "", {"Alpha:", "Time:", ""}, "", "", "", true},
    {"aParticleEffect", "", {"StartTime:", "", ""}, "JetEngineBig|RealGiantSmoke|GiantSmoke", "Name:", "Offset:", true},
    {"aDont_Move", "", {"", "", ""}, "Stay|Move", "Stay/Go:", "", true},
    {"cFOV_Now", "", {"FOV:", "", ""}, "", "", "", true},
    {"pSetGear", "", {"", "", ""}, "Auto|Down|Up", "Gear:", "", true},
    {"cCameraShake", "", {"Frequency:", "", ""}, "CameraPos|LookAtPos|CameraAndLookAt", "ShakeIt:", "Max:", true},
    {"cBinocular", "", {"", "", ""}, "Binocular|Periscope|OFF", "Action:", "", true},
    {"pTakeOffHack", "", {"Angle:", "", ""}, "", "", "", true},
    {"pWingTipEffect", "", {"", "", ""}, "On|Off", "Effect:", "", true},
    {"sMoveToPoint", "", {"", "", ""}, "Enable|Disable", "MoveTP:", "", true},
    {"cWideBlackBars", "", {"", "", ""}, "Enable|Disable", "Bar:", "", true},
    {"sUseCatapult", "", {"Dir:", "", ""}, "", "", "", true},
    {"aHideEntity", "", {"", "", ""}, "Hide|Show", "H/S:", "", true},
}};

std::optional<std::int32_t> find_movie_action_type_0045e5e0(std::string_view type) {
    for (std::size_t i = 0; i < kMovieActionTypes.size(); ++i) {
        const std::string_view name = kMovieActionTypes[i].name;
        if (name.empty()) continue;  // matches only a null token, which the reader never yields
        if (equals_ci(type, name)) return static_cast<std::int32_t>(i);
    }
    return std::nullopt;
}

std::optional<MovieAction> parse_movie_action_0045e5e0(MovieActionTokens& tokens) {
    tokens.expect("Action");                          // 0045E626
    tokens.expect("{");                               // 0045E632
    tokens.expect("Type");                            // 0045E63E
    tokens.read_token();
    const std::string type_name = tokens.read_token();  // 0045E64C
    tokens.expect("ActivateTime");                    // 0045E698
    tokens.read_token();
    MovieAction action;
    action.activate_time = tokens.read_float();       // 0045E6AB
    tokens.expect("Comment");                         // 0045E6BB
    tokens.read_token();
    action.comment = tokens.read_token();             // 0045E6C9, copied to action+8 by 0045DE50
    const auto type = find_movie_action_type_0045e5e0(type_name);  // 0045E7A0 loop
    if (!type || !kMovieActionTypes[static_cast<std::size_t>(*type)].constructible) return std::nullopt;
    action.type = *type;
    const auto& info = kMovieActionTypes[static_cast<std::size_t>(*type)];
    if (flag(info.refer_label)) {                     // record +10h, 0045E867..0045E881
        tokens.expect("ReferPath");
        tokens.read_token();
        action.refer_path = tokens.read_nonempty_string();
    }
    // Flag +48h, 0045E945..0045E95F. The table stands it in with the label (+38h):
    // in all 32 calls to 0045D480 the fifth flag is 1 exactly when that label is non-empty.
    if (flag(info.enum_label)) {
        tokens.expect("Enum");
        tokens.read_token();
        action.enum_text = tokens.read_nonempty_string();
    }
    static const char* const kNumKeys[3] = {"Num1", "Num2", "Num3"};  // 0045EA20, EA89, EAF2
    for (std::size_t i = 0; i < 3; ++i) {
        if (!flag(info.num_labels[i])) continue;      // record +1Ch, +28h, +34h
        tokens.expect(kNumKeys[i]);
        tokens.read_token();
        action.num[i] = tokens.read_float();
    }
    if (flag(info.offset_label)) {                    // record +54h, 0045EB5B..0045EBBB
        tokens.expect("OffsetX");
        tokens.read_token();
        action.offset_x = tokens.read_float();
        tokens.expect("OffsetY");
        tokens.read_token();
        action.offset_y = tokens.read_float();        // stored to +3Ch
        tokens.expect("OffsetZ");
        tokens.read_token();
        action.offset_z = tokens.read_float();        // stored to +38h
    }
    tokens.expect("}");                               // 0045EBE0
    return action;
}

bool movie_action_enum_is(const MovieAction& action, const char* value) {
    return equals_ci(action.enum_text, value);
}

namespace {

std::optional<std::uint32_t> gun_category_bit(const MovieAction& action) {
    if (movie_action_enum_is(action, "AA_Flak")) return 1u;
    if (movie_action_enum_is(action, "Artillery")) return 2u;
    if (movie_action_enum_is(action, "Torpedo")) return 4u;
    if (movie_action_enum_is(action, "DC")) return 8u;  // 00CE5448
    return std::nullopt;
}

} // namespace

std::uint32_t movie_gun_fire_enabled_mask_00465290(std::uint32_t disabled_mask,
                                                   const MovieAction& action) {
    if (movie_action_enum_is(action, "All")) return disabled_mask & ~0xFFu;  // byte store 0
    if (const auto bit = gun_category_bit(action)) return disabled_mask & ~*bit;
    return disabled_mask;
}

std::uint32_t movie_gun_fire_disabled_mask_00465350(std::uint32_t disabled_mask,
                                                    const MovieAction& action) {
    if (movie_action_enum_is(action, "All")) return disabled_mask | 0xFFu;
    if (const auto bit = gun_category_bit(action)) return disabled_mask | *bit;
    return disabled_mask;
}

std::optional<std::pair<std::int32_t, bool>> movie_set_gear_request_00464a30(const MovieAction& action) {
    if (movie_action_enum_is(action, "Auto")) return std::pair<std::int32_t, bool>{0, false};
    if (movie_action_enum_is(action, "Down")) return std::pair<std::int32_t, bool>{1, true};  // 00CE53DC
    if (movie_action_enum_is(action, "Up")) return std::pair<std::int32_t, bool>{2, true};    // 00CE53D8
    return std::nullopt;
}

std::pair<float, float> movie_blackout_level_duration_004660b0(const MovieAction& action) {
    return {action.num[0], action.num[1]};
}

void rewind_engine_movie_clock_0045c5b0(EngineMovieClock& clock) noexcept {
    clock.time = 0.0f;
    clock.history.fill(0.0f);
}

EngineMovieStep advance_engine_movie_clock_0045f970(EngineMovieClock& clock, float delta) noexcept {
    if (delta > 1.0f) delta = 0.01f;                                   // 0045F977..0045F988
    if (clock.state == 0 || clock.state == 3) return EngineMovieStep::Idle;  // 0045F98E..0045F99C
    float scaled = delta;
    if (clock.state == 2) scaled = static_cast<float>(static_cast<double>(delta) * 0.5);  // 0045F9AC
    float sum = 0.0f;                                                   // 0045F9D0..0045FA44
    for (const float sample : clock.history) sum = sum + sample;
    const float step = static_cast<float>((static_cast<double>(sum) + scaled) / 21.0);  // 0045FA46..0045FA58
    for (std::size_t i = 0; i + 1 < clock.history.size(); ++i) clock.history[i] = clock.history[i + 1];
    clock.history.back() = scaled;                                      // 0045FAB5
    clock.time = clock.time + step;                                     // 0045FAAE..0045FAC2
    return clock.length <= clock.time ? EngineMovieStep::Finished : EngineMovieStep::Running;  // 0045FAC9
}

std::size_t run_due_movie_actions_004560e0(const std::vector<MovieAction>& actions,
                                           std::size_t cursor, float time, bool entity_ready,
                                           const std::function<void(const MovieAction&)>& run) {
    while (cursor < actions.size()) {
        if (!(time > actions[cursor].activate_time)) break;  // unordered also stops  // 00456148 FCOMIP, JBE 004561D9
        if (entity_ready && run) run(actions[cursor]);    // 004561A0 CALL EDX
        ++cursor;
    }
    return cursor;
}

} // namespace bsp
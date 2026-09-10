// Plane class descriptor field reader. See include/bsp/plane_class_fields.hpp
// for the addresses, the evidence and the state this reached.

#include "bsp/plane_class_fields.hpp"

#include <cstring>

namespace bsp {
namespace {

using Kind = VehicleClassValueKind;

// The reader tests IsNil (00B65FB0) rather than the value's type everywhere a
// slot is gated, so a present but non-numeric value still goes through 00B66270.
bool present(const VehicleClassLuaValue& value) noexcept {
    return value.kind != Kind::Nil;
}

// 00B66050 answers true for any Lua number, integral or not.
bool is_number(const VehicleClassLuaValue& value) noexcept {
    return value.kind == Kind::Integer || value.kind == Kind::Number;
}

VehicleClassLuaValue field(VehicleClassFieldHost& lua, const GuiLuaRef& table, const char* key) {
    const GuiLuaRef ref = lua.get_by_name(table, key);
    const VehicleClassLuaValue value = lua.inspect(ref);
    lua.release(ref);
    return value;
}

VehicleClassLuaValue element(VehicleClassFieldHost& lua, const GuiLuaRef& table,
                             std::int32_t index) {
    const GuiLuaRef ref = lua.get_by_index(table, index);
    const VehicleClassLuaValue value = lua.inspect(ref);
    lua.release(ref);
    return value;
}

float number(VehicleClassFieldHost& lua, const GuiLuaRef& table, const char* key) {
    return vehicle_class_number_00b66270(field(lua, table, key));
}

float number_or(VehicleClassFieldHost& lua, const GuiLuaRef& table, const char* key,
                float fallback) {
    return vehicle_class_number_or_00b66330(field(lua, table, key), fallback);
}

// 007D2D39 / 007D2E27 / 007D3068 / 007D3382: an effect key is fetched twice, once
// for the IsNil test and once for GetInteger, and the slot keeps whatever it held
// when the first fetch says nil.
void effect_handle(VehicleClassFieldHost& lua, PlaneClassFieldHost& host,
                   const GuiLuaRef& row, const char* key, bool& has, std::int32_t& slot) {
    if (!present(field(lua, row, key))) {
        return;
    }
    has = true;
    slot = host.make_effect_handle(vehicle_class_integer_00b66290(field(lua, row, key)));
}

// 007D381A / 007D3990 / 007D3B06. One PartAnims sub-table folded into the 10h-byte
// record: element [1], element [2], then |[1] - [2]| unless element [3] is a
// number, in which case it replaces the difference.
void part_anim(VehicleClassFieldHost& lua, const GuiLuaRef& part_anims, const char* key,
               PlanePartAnimFields& out) {
    const GuiLuaRef table = lua.get_by_name(part_anims, key);
    if (!lua.inspect(table).is_table()) {
        lua.release(table);
        return;
    }
    out.present = true;
    out.first = vehicle_class_number_00b66270(element(lua, table, 1));
    out.second = vehicle_class_number_00b66270(element(lua, table, 2));
    out.duration = plane_class_part_anim_duration_007d3883(out.first, out.second);
    const VehicleClassLuaValue third = element(lua, table, 3);
    if (is_number(third)) {
        out.duration = vehicle_class_number_00b66270(third);
    }
    lua.release(table);
}

}  // namespace

// ---------------------------------------------------------------------------
// Key schema
// ---------------------------------------------------------------------------

const PlaneClassFieldSpec kPlaneClassFieldSchema[] = {
    {".ShortName", PlaneClassConversion::NativeString, 0x138, 0x007D1FB0, "-",
     "00B685C0 then the descriptor's own string resize at 007D1FE9"},
    {".NumEngines", PlaneClassConversion::NumberToInt, 0x140, 0x007D204D, "-",
     "00B66270 then the CRT float-to-int at 00BF7420"},
    {".JetEngines", PlaneClassConversion::BooleanOr, 0x144, 0x007D208B, "false",
     "00B662F0 with a PUSH 0 default at 007D2090; the slot is a byte"},
    {".Accel", PlaneClassConversion::NumberScaled, 0x164, 0x007D20C6, "-",
     "scaled in place by tuning +320h * +31Ch, or the tuning clamp at 007D213C"},
    {".YDrag", PlaneClassConversion::Number, 0x170, 0x007D2150, "-", ""},
    {".XDrag", PlaneClassConversion::Number, 0x174, 0x007D2189, "-", ""},
    {".ExtRotAccel", PlaneClassConversion::Number, 0x178, 0x007D21C2, "-", ""},
    {".StallRotAccel", PlaneClassConversion::Number, 0x17C, 0x007D21FB, "-", ""},
    {".WaterRotAccel", PlaneClassConversion::Number, 0x180, 0x007D2234, "-", ""},
    {".AirBrakeDrag", PlaneClassConversion::Number, 0x1DC, 0x007D226D, "-", ""},
    {".WheelBrake", PlaneClassConversion::Number, 0x1E0, 0x007D22A6, "-", ""},
    {".BombControlLimit", PlaneClassConversion::Number, 0x15C, 0x007D22DF, "-", ""},
    {".BombDelay", PlaneClassConversion::Number, 0x1F4, 0x007D2318, "-", ""},
    {".StallSpd", PlaneClassConversion::Number, 0x184, 0x007D2351, "-", ""},
    {".MaxSpd", PlaneClassConversion::Number, 0x188, 0x007D238A, "-", ""},
    {".TravelSpeed", PlaneClassConversion::NumberAndProduct, 0x18C, 0x007D23C3, "-",
     "007D2406 also writes +190h as tuning +334h times the value"},
    {".SwimHeight", PlaneClassConversion::Number, 0x194, 0x007D2413, "-", ""},
    {".MinWaterSpd", PlaneClassConversion::Number, 0x198, 0x007D244C, "-", ""},
    {".MaxWaterSpd", PlaneClassConversion::Number, 0x19C, 0x007D2485, "-", ""},
    {".WaterDecel", PlaneClassConversion::Number, 0x1A0, 0x007D24BE, "-", ""},
    {".WaterUnSpring", PlaneClassConversion::Number, 0x1A4, 0x007D24F7, "-", ""},
    {".RollSpd", PlaneClassConversion::Number, 0x1A8, 0x007D2530, "-", ""},
    {".PitchSpd", PlaneClassConversion::Number, 0x1AC, 0x007D2569, "-", ""},
    {".YawSpd", PlaneClassConversion::Number, 0x1B0, 0x007D25A2, "-", ""},
    {".TurnRollSpd", PlaneClassConversion::Number, 0x1C8, 0x007D25DB, "-", ""},
    {".YawLimitAngle", PlaneClassConversion::Number, 0x1CC, 0x007D2614, "-", ""},
    {".PitchLimitAngle", PlaneClassConversion::Number, 0x1D0, 0x007D264D, "-", ""},
    {".YawRollRatio", PlaneClassConversion::Number, 0x1B4, 0x007D2686, "-", ""},
    {".SlideRatio", PlaneClassConversion::Number, 0x1B8, 0x007D26BF, "-", ""},
    {".RollAccel", PlaneClassConversion::Number, 0x1BC, 0x007D26F8, "-", ""},
    {".PitchAccel", PlaneClassConversion::Number, 0x1C0, 0x007D2731, "-", ""},
    {".YawAccel", PlaneClassConversion::Number, 0x1C4, 0x007D276A, "-", ""},
    {".KameraMogotte", PlaneClassConversion::NumberOr, 0x168, 0x007D27A3, "DAT_00CE38B8 = 10.0f",
     "the same global the ship reader uses for CaptainCameraHeight"},
    {".KameraFolotte", PlaneClassConversion::NumberOr, 0x16C, 0x007D27E6, "DAT_00CF87C8 = 2.0f",
     "the same global the GearsPullTime default starts from"},
    {".DragPitchRatio", PlaneClassConversion::Number, 0x1D4, 0x007D2829, "-", ""},
    {".NegativePitchRatio", PlaneClassConversion::Number, 0x1D8, 0x007D2862, "-", ""},
    {".TurnRoll", PlaneClassConversion::Number, 0x25C, 0x007D289B, "-", ""},
    {".TurnRollLeader", PlaneClassConversion::Number, 0x260, 0x007D28D4, "-", ""},
    {".RollMaxforceLimit", PlaneClassConversion::Number, 0x274, 0x007D290D, "-", ""},
    {".PitchMaxforceLimit", PlaneClassConversion::Number, 0x278, 0x007D2946, "-", ""},
    {".TurnCircleRadius", PlaneClassConversion::Number, 0x268, 0x007D297F, "-", ""},
    {".WheelHeight", PlaneClassConversion::NumberGated, 0x1FC, 0x007D29B8, "untouched",
     "probed at 007D29B8, converted at 007D2A65 only when the gate passed"},
    {".GroundPitch", PlaneClassConversion::NumberGated, 0x200, 0x007D29E8, "untouched",
     "probed at 007D29E8; the gate also sets the byte at +1F8h to 1"},
    {".WaterPitch", PlaneClassConversion::Number, 0x204, 0x007D2AD7, "-", ""},
    {".GlideRate", PlaneClassConversion::Number, 0x208, 0x007D2B10, "-", ""},
    {".CarrierBased", PlaneClassConversion::BooleanOrFalse, 0x160, 0x007D2B4C, "false",
     "IsNil at 007D2B63 then 00B66250; the else branch clears the byte"},
    {".DropAngle", PlaneClassConversion::Number, 0x1F0, 0x007D2B93, "-", ""},
    {".KamikazeBulletClass", PlaneClassConversion::IntegerAndClass, 0x20C, 0x007D2BCA, "-",
     "the id lands at +20Ch and 006EA910's refcounted class at +210h"},
    {".Wreck", PlaneClassConversion::WreckClassOrNull, 0x21C, 0x007D2C7A, "0",
     "IsNil at 007D2C89, then GetString and 004B3D20 at 007D2CDF"},
    {".ExplosionEfx", PlaneClassConversion::EffectHandle, 0x214, 0x007D2D39, "untouched", ""},
    {".ShellsEfx", PlaneClassConversion::EffectHandle, 0x234, 0x007D2E27, "untouched", ""},
    {".EngineEfxes", PlaneClassConversion::EngineEffectVector, 0x250, 0x007D2F15, "empty",
     "007C3510 builds a 44h-byte record, 007CCCA0 appends it to +250h"},
    {".EngineFireEfx", PlaneClassConversion::EffectHandle, 0x220, 0x007D3068, "untouched",
     "the block is followed by the unconditional LowPlaneAlt lookup into +218h"},
    {".DamageSmokeEfx", PlaneClassConversion::EffectHandleVector, 0x224, 0x007D323E, "empty",
     "004D9C00 appends each handle to the vector at +224h"},
    {".WingTipEfx", PlaneClassConversion::EffectHandle, 0x5A4, 0x007D3382, "untouched", ""},
    {".BayDoor", PlaneClassConversion::Table, kPlaneClassNoOffset, 0x007D3473, "-",
     "a non-table writes all three defaults at 007D3550"},
    {".BayDoor.OpenAngle", PlaneClassConversion::Number, 0x5A8, 0x007D34A5,
     "DAT_00CE7D1C = -0.6981317f", ""},
    {".BayDoor.ClosedAngle", PlaneClassConversion::Number, 0x5AC, 0x007D34E1,
     "DAT_00CE7D20 = 0.6981317f", ""},
    {".BayDoor.TimeToOpen", PlaneClassConversion::Number, 0x5B0, 0x007D351D,
     "DAT_00D7A2F0 = 0.1f", ""},
    {".BowWaves", PlaneClassConversion::EffectRecordVector, 0x550, 0x007D3589, "empty",
     "007D1D30 appends a 10h-byte record whose first dword is the handle"},
    {".ParaReload", PlaneClassConversion::NumberReciprocalOrZero, 0x5B4, 0x007D371A, "0.0f",
     "FLD1 / FDIVRP at 007D3743; an absent key stores 0.0f, not an infinity"},
    {".GearsPullTime", PlaneClassConversion::NumberOrKindDefault, 0x5F8, 0x007D37A4,
     "2.0f, or 4.0f for kind 10h or 16h",
     "the IsKindOf calls at 007D3773 and 007D3782 pick the default"},
    {".PartAnims", PlaneClassConversion::Table, kPlaneClassNoOffset, 0x007D37E8, "-", ""},
    {".PartAnims.Gears", PlaneClassConversion::PartAnim, 0x5C8, 0x007D381A, "untouched", ""},
    {".PartAnims.Wings", PlaneClassConversion::PartAnim, 0x5D8, 0x007D3990, "untouched", ""},
    {".PartAnims.BayDoor", PlaneClassConversion::PartAnim, 0x5E8, 0x007D3B06, "untouched", ""},
    {".TurboTime", PlaneClassConversion::NumberOr, 0x5FC, 0x007D3CB6, "FLDZ, so 0.0f",
     "zeroed again at 007D3DB8 when TurboStrength is not above 1.0f"},
    {".TurboRechargingTime", PlaneClassConversion::NumberOr, 0x600, 0x007D3CF6, "FLDZ, so 0.0f",
     ""},
    {".TurboStrength", PlaneClassConversion::NumberOr, 0x604, 0x007D3D36, "FLD1, so 1.0f",
     "the default alone disables turbo, because the gate wants strictly above 1.0f"},
    {".TurboControlLimit", PlaneClassConversion::NumberOr, 0x608, 0x007D3D73, "FLD1, so 1.0f",
     ""},
};

std::size_t plane_class_field_count() noexcept {
    return sizeof(kPlaneClassFieldSchema) / sizeof(kPlaneClassFieldSchema[0]);
}

const PlaneClassFieldSpec* plane_class_find_field(const char* path) noexcept {
    if (path == nullptr) {
        return nullptr;
    }
    for (std::size_t i = 0; i < plane_class_field_count(); ++i) {
        if (std::strcmp(kPlaneClassFieldSchema[i].path, path) == 0) {
            return &kPlaneClassFieldSchema[i];
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// Conversion rules
// ---------------------------------------------------------------------------

PlaneClassAccelResult plane_class_accel_007d20f3(float lua_accel, float accel_factor,
                                                 float accel_multiplier) noexcept {
    PlaneClassAccelResult result;
    result.accel = lua_accel;
    result.clamped_accel_factor = accel_factor;
    // COMISS at 007D2100 against DAT_00D7A24C = 1.0f, JBE to the clamp.
    if (accel_factor > 1.0f) {
        result.accel = accel_multiplier * accel_factor * lua_accel;
    } else {
        result.clamped_accel_factor = 1.0f;
        result.clamped = true;
    }
    return result;
}

float plane_class_para_reload_rate_007d3743(bool value_present, float lua_para_reload) noexcept {
    if (!value_present) {
        return 0.0f;
    }
    return 1.0f / lua_para_reload;
}

float plane_class_gears_pull_default_007d375a(bool is_level_bomber,
                                              bool is_large_recon_plane) noexcept {
    // DAT_00CF87C8 = 2.0f at 007D375C, DAT_00CE3D34 = 4.0f at 007D3788.
    return (is_level_bomber || is_large_recon_plane) ? 4.0f : 2.0f;
}

float plane_class_part_anim_duration_007d3883(float first, float second) noexcept {
    const float difference = first - second;
    // 007D38AD subtracts from DAT_00D7A208 = -0.0f, which is a negation.
    return (difference > 0.0f) ? difference : (-0.0f - difference);
}

bool plane_class_turbo_enabled_007d3da4(float turbo_strength) noexcept {
    // COMISS 1.0f, TurboStrength then JC: the branch that keeps TurboTime is
    // taken only when 1.0f is below the strength, so a NaN also zeroes it.
    return 1.0f < turbo_strength;
}

// ---------------------------------------------------------------------------
// The reader sequence
// ---------------------------------------------------------------------------

void read_plane_class_fields_007d1f70(VehicleClassFieldHost& lua,
                                      PlaneClassFieldHost& host,
                                      const GuiLuaRef& row,
                                      PlaneClassFields& out) {
    // 007D1FB0: ShortName, copied into the descriptor's own string at +138h.
    {
        const VehicleClassLuaValue value = field(lua, row, "ShortName");
        const char* text = vehicle_class_string_or_00b685c0(value, "");
        out.short_name = (text != nullptr) ? text : "";
    }

    out.num_engines = static_cast<std::int32_t>(number(lua, row, "NumEngines"));
    out.jet_engines = vehicle_class_boolean_or_00b662f0(field(lua, row, "JetEngines"), false);

    // 007D20C6: Accel, then the tuning scale or the tuning clamp at 007D213C.
    {
        const float raw = number(lua, row, "Accel");
        const PlaneClassAccelResult scaled = plane_class_accel_007d20f3(
            raw, host.tuning_float(PlaneClassTuningOffsets::kAccelFactor),
            host.tuning_float(PlaneClassTuningOffsets::kAccelMultiplier));
        out.accel = scaled.accel;
        out.clamped_accel_factor = scaled.clamped;
        if (scaled.clamped) {
            host.set_tuning_float(PlaneClassTuningOffsets::kAccelFactor,
                                  scaled.clamped_accel_factor);
        }
    }

    out.y_drag = number(lua, row, "YDrag");
    out.x_drag = number(lua, row, "XDrag");
    out.ext_rot_accel = number(lua, row, "ExtRotAccel");
    out.stall_rot_accel = number(lua, row, "StallRotAccel");
    out.water_rot_accel = number(lua, row, "WaterRotAccel");
    out.air_brake_drag = number(lua, row, "AirBrakeDrag");
    out.wheel_brake = number(lua, row, "WheelBrake");
    out.bomb_control_limit = number(lua, row, "BombControlLimit");
    out.bomb_delay = number(lua, row, "BombDelay");
    out.stall_spd = number(lua, row, "StallSpd");
    out.max_spd = number(lua, row, "MaxSpd");

    // 007D23C3: TravelSpeed, and the derived slot at +190h no key names.
    out.travel_speed = number(lua, row, "TravelSpeed");
    out.travel_speed_scaled =
        host.tuning_float(PlaneClassTuningOffsets::kTravelSpeedFactor) * out.travel_speed;

    out.swim_height = number(lua, row, "SwimHeight");
    out.min_water_spd = number(lua, row, "MinWaterSpd");
    out.max_water_spd = number(lua, row, "MaxWaterSpd");
    out.water_decel = number(lua, row, "WaterDecel");
    out.water_un_spring = number(lua, row, "WaterUnSpring");
    out.roll_spd = number(lua, row, "RollSpd");
    out.pitch_spd = number(lua, row, "PitchSpd");
    out.yaw_spd = number(lua, row, "YawSpd");
    out.turn_roll_spd = number(lua, row, "TurnRollSpd");
    out.yaw_limit_angle = number(lua, row, "YawLimitAngle");
    out.pitch_limit_angle = number(lua, row, "PitchLimitAngle");
    out.yaw_roll_ratio = number(lua, row, "YawRollRatio");
    out.slide_ratio = number(lua, row, "SlideRatio");
    out.roll_accel = number(lua, row, "RollAccel");
    out.pitch_accel = number(lua, row, "PitchAccel");
    out.yaw_accel = number(lua, row, "YawAccel");

    // 007D27A8 and 007D27EB load the two defaults as globals, not immediates.
    out.kamera_mogotte = number_or(lua, row, "KameraMogotte", 10.0f);
    out.kamera_folotte = number_or(lua, row, "KameraFolotte", 2.0f);

    out.drag_pitch_ratio = number(lua, row, "DragPitchRatio");
    out.negative_pitch_ratio = number(lua, row, "NegativePitchRatio");
    out.turn_roll = number(lua, row, "TurnRoll");
    out.turn_roll_leader = number(lua, row, "TurnRollLeader");
    out.roll_maxforce_limit = number(lua, row, "RollMaxforceLimit");
    out.pitch_maxforce_limit = number(lua, row, "PitchMaxforceLimit");
    out.turn_circle_radius = number(lua, row, "TurnCircleRadius");

    // 007D29B8..007D2AC6: WheelHeight and GroundPitch are probed together and
    // written only when both are present. A row with one of the two writes
    // neither slot and leaves the byte at +1F8h alone.
    {
        const bool gate = present(field(lua, row, "WheelHeight")) &&
                          present(field(lua, row, "GroundPitch"));
        if (gate) {
            out.ground_data_present = true;
            out.wheel_height = number(lua, row, "WheelHeight");
            out.ground_pitch = number(lua, row, "GroundPitch");
        }
    }

    out.water_pitch = number(lua, row, "WaterPitch");
    out.glide_rate = number(lua, row, "GlideRate");

    // 007D2B4C: unlike the effect keys, an absent CarrierBased clears the byte.
    {
        const VehicleClassLuaValue value = field(lua, row, "CarrierBased");
        out.carrier_based = present(value) ? vehicle_class_boolean_00b66250(value) : false;
    }

    out.drop_angle = number(lua, row, "DropAngle");

    // 007D2BCA: the id is stored raw and then resolved against the Bullets table.
    out.kamikaze_bullet_class_id =
        vehicle_class_integer_00b66290(field(lua, row, "KamikazeBulletClass"));
    out.kamikaze_bullet_class = host.resolve_bullet_class(out.kamikaze_bullet_class_id);

    // 007D2C7A: Wreck, whose absent case does clear the slot.
    {
        const VehicleClassLuaValue value = field(lua, row, "Wreck");
        if (present(value)) {
            const char* name = vehicle_class_string_or_00b685c0(field(lua, row, "Wreck"), "");
            out.wreck_class = host.resolve_wreck_class((name != nullptr) ? name : "");
        } else {
            out.wreck_class = 0;
        }
    }

    effect_handle(lua, host, row, "ExplosionEfx", out.has_explosion_efx, out.explosion_efx);
    effect_handle(lua, host, row, "ShellsEfx", out.has_shells_efx, out.shells_efx);

    // 007D2F15: EngineEfxes, an indexed walk that stops at the first element that
    // is not an integer, so a gap truncates the vector.
    {
        const GuiLuaRef table = lua.get_by_name(row, "EngineEfxes");
        if (lua.inspect(table).is_table()) {
            for (std::int32_t i = 1; element(lua, table, i).is_integer(); ++i) {
                out.engine_efxes.push_back(host.make_effect_handle(
                    vehicle_class_integer_00b66290(element(lua, table, i))));
            }
        }
        lua.release(table);
    }

    effect_handle(lua, host, row, "EngineFireEfx", out.has_engine_fire_efx, out.engine_fire_efx);
    // 007D315C..007D3191: an effect named by a literal, read from no key at all.
    out.low_plane_alt_efx = host.find_effect_by_name("LowPlaneAlt");

    // 007D323E: DamageSmokeEfx, the same indexed walk into a handle vector.
    {
        const GuiLuaRef table = lua.get_by_name(row, "DamageSmokeEfx");
        if (lua.inspect(table).is_table()) {
            for (std::int32_t i = 1; element(lua, table, i).is_integer(); ++i) {
                out.damage_smoke_efx.push_back(host.make_effect_handle(
                    vehicle_class_integer_00b66290(element(lua, table, i))));
            }
        }
        lua.release(table);
    }

    effect_handle(lua, host, row, "WingTipEfx", out.has_wing_tip_efx, out.wing_tip_efx);

    // 007D3473: BayDoor. A missing or non-table value writes all three defaults,
    // which is the one block in this reader that defaults as a group.
    {
        const GuiLuaRef table = lua.get_by_name(row, "BayDoor");
        if (lua.inspect(table).is_table()) {
            out.bay_door_open_angle = number(lua, table, "OpenAngle");
            out.bay_door_closed_angle = number(lua, table, "ClosedAngle");
            out.bay_door_time_to_open = number(lua, table, "TimeToOpen");
        } else {
            out.bay_door_open_angle = -0.6981317f;
            out.bay_door_closed_angle = 0.6981317f;
            out.bay_door_time_to_open = 0.1f;
        }
        lua.release(table);
    }

    // 007D3589: BowWaves, whose walk ends on the first nil rather than on the
    // first non-integer, so a non-integer element is read as an integer.
    {
        const GuiLuaRef table = lua.get_by_name(row, "BowWaves");
        if (lua.inspect(table).is_table()) {
            for (std::int32_t i = 1; present(element(lua, table, i)); ++i) {
                out.bow_waves.push_back(host.make_effect_handle(
                    vehicle_class_integer_00b66290(element(lua, table, i))));
            }
        }
        lua.release(table);
    }

    // 007D371A: ParaReload, stored as its reciprocal.
    {
        const VehicleClassLuaValue value = field(lua, row, "ParaReload");
        out.para_reload_rate = plane_class_para_reload_rate_007d3743(
            present(value), vehicle_class_number_00b66270(value));
    }

    // 007D375A: the GearsPullTime default is chosen before the key is read.
    {
        const float fallback = plane_class_gears_pull_default_007d375a(
            host.is_kind_of(PlaneClassKindCodes::kLevelBomber),
            host.is_kind_of(PlaneClassKindCodes::kLargeReconPlane));
        out.gears_pull_time = number_or(lua, row, "GearsPullTime", fallback);
    }

    // 007D37E8: PartAnims. A missing table leaves all three records untouched.
    {
        const GuiLuaRef table = lua.get_by_name(row, "PartAnims");
        if (lua.inspect(table).is_table()) {
            part_anim(lua, table, "Gears", out.part_anim_gears);
            part_anim(lua, table, "Wings", out.part_anim_wings);
            part_anim(lua, table, "BayDoor", out.part_anim_bay_door);
        }
        lua.release(table);
    }

    out.turbo_time = number_or(lua, row, "TurboTime", 0.0f);
    out.turbo_recharging_time = number_or(lua, row, "TurboRechargingTime", 0.0f);
    out.turbo_strength = number_or(lua, row, "TurboStrength", 1.0f);
    out.turbo_control_limit = number_or(lua, row, "TurboControlLimit", 1.0f);
    if (!plane_class_turbo_enabled_007d3da4(out.turbo_strength)) {
        out.turbo_time = 0.0f;
    }
}

// ---------------------------------------------------------------------------
// Installed-file check
// ---------------------------------------------------------------------------

const PlaneClassKeyCount kPlaneClassKeyCounts[] = {
    {".ShortName", 72},        {".NumEngines", 72},
    {".JetEngines", 8},        {".Accel", 72},
    {".YDrag", 72},            {".XDrag", 72},
    {".ExtRotAccel", 72},      {".StallRotAccel", 72},
    {".WaterRotAccel", 72},    {".AirBrakeDrag", 72},
    {".WheelBrake", 72},       {".BombControlLimit", 72},
    {".BombDelay", 72},        {".StallSpd", 72},
    {".MaxSpd", 72},           {".TravelSpeed", 72},
    {".SwimHeight", 72},       {".MinWaterSpd", 72},
    {".MaxWaterSpd", 72},      {".WaterDecel", 72},
    {".WaterUnSpring", 72},    {".RollSpd", 72},
    {".PitchSpd", 72},         {".YawSpd", 72},
    {".TurnRollSpd", 72},      {".YawLimitAngle", 72},
    {".PitchLimitAngle", 72},  {".YawRollRatio", 72},
    {".SlideRatio", 72},       {".RollAccel", 72},
    {".PitchAccel", 72},       {".YawAccel", 72},
    {".KameraMogotte", 72},    {".KameraFolotte", 72},
    {".DragPitchRatio", 72},   {".NegativePitchRatio", 72},
    {".TurnRoll", 72},         {".TurnRollLeader", 72},
    {".RollMaxforceLimit", 72}, {".PitchMaxforceLimit", 72},
    {".TurnCircleRadius", 72}, {".WheelHeight", 51},
    {".GroundPitch", 51},      {".WaterPitch", 72},
    {".GlideRate", 72},        {".CarrierBased", 42},
    {".DropAngle", 72},        {".KamikazeBulletClass", 72},
    {".Wreck", 0},             {".ExplosionEfx", 72},
    {".ShellsEfx", 53},        {".EngineEfxes", 4},
    {".EngineFireEfx", 72},    {".DamageSmokeEfx", 72},
    {".WingTipEfx", 53},       {".BayDoor", 17},
    {".BayDoor.OpenAngle", 17}, {".BayDoor.ClosedAngle", 17},
    {".BayDoor.TimeToOpen", 17}, {".BowWaves", 71},
    {".ParaReload", 4},        {".GearsPullTime", 0},
    {".PartAnims", 60},        {".PartAnims.Gears", 59},
    {".PartAnims.Wings", 10},  {".PartAnims.BayDoor", 16},
    {".TurboTime", 40},        {".TurboRechargingTime", 38},
    {".TurboStrength", 40},    {".TurboControlLimit", 40},
};

std::size_t plane_class_key_count_count() noexcept {
    return sizeof(kPlaneClassKeyCounts) / sizeof(kPlaneClassKeyCounts[0]);
}

const char* const kPlaneClassUnprovidedPaths[] = {".Wreck", ".GearsPullTime"};

std::size_t plane_class_unprovided_path_count() noexcept {
    return sizeof(kPlaneClassUnprovidedPaths) / sizeof(kPlaneClassUnprovidedPaths[0]);
}

const PlaneClassKeyCount kPlaneClassUnconsumedPaths[] = {
    {".ArmorIndexes", 72},          {".Race", 72},
    {".TurnRollLowLimit", 72},      {".Type", 72},
    {".DogFightAimDistance", 70},   {".DogFightShootDistance", 70},
    {".StrafeAttackDistance", 70},  {".StrafeAttackHeight", 70},
    {".StrafeGoawayDistance", 70},  {".UnitlibViewDistance", 65},
    {".HP_Realistic", 36},          {".Unlock", 27},
    {".PlatformDirections", 22},    {".UnlockID", 17},
    {".UE_DLC", 3},
};

std::size_t plane_class_unconsumed_path_count() noexcept {
    return sizeof(kPlaneClassUnconsumedPaths) / sizeof(kPlaneClassUnconsumedPaths[0]);
}

}  // namespace bsp

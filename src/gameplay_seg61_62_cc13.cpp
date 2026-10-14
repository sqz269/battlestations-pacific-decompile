// Segments 61 and 62 reference models. Packet cc13-gpr61; docs/CC13_GPR_61.md.
// Read from the installed executable's bytes (Capstone disasm-raw; x87 traced with
// sc2_x87.py and the opcode bytes); the listing decides every comparison.
// Unrounded x87 intermediates are carried in double (the image keeps 80 bits);
// every FSTP to a float slot is an explicit f32().

#include "bsp/gameplay_seg61_62_cc13.hpp"

#include <cctype>
#include <cstring>

namespace bsp {
namespace gameplay_seg61_62_cc13 {

namespace {

float f32(double v) noexcept { return static_cast<float>(v); }

constexpr float kOne_00d7a24c = 1.0f;
constexpr float kZero_00d7a218 = 0.0f;
constexpr double kHalf_00d7a280 = 0.5;          // qword
constexpr char kInferiorFailure_00cf0b74[] = "InferiorFailure";
constexpr int kRouteChannel = 7;                // 00953E17 PUSH 7
constexpr int kNoSenderSlot = 8;                // 004129 / 0041C9 PUSH 8

// _stricmp (00BF7FBF) under the "C" locale: an ASCII case fold.
bool stricmp_equal(const char* a, const char* b) noexcept {
    for (;; ++a, ++b) {
        const int ca = std::tolower(static_cast<unsigned char>(*a));
        const int cb = std::tolower(static_cast<unsigned char>(*b));
        if (ca != cb) return false;
        if (ca == 0) return true;
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// 1. The vehicle-class family
// ---------------------------------------------------------------------------

// Each row read from its constructor's body (CALL base, two MOV imm32 vtables,
// then the zero stores).
const std::array<ClassDescriptorCtorRow, 21> kClassDescriptorCtors = {{
    {0x00951AA0, "ReconPlane", 0x007D1E50, 0x00D19BF4, 0x00D19BF0, {}, 0},
    {0x00951AF0, "Fighter", 0x007D1E50, 0x00D19C30, 0x00D19C2C, {}, 0},
    {0x00951B40, "DiveBomber", 0x007D1E50, 0x00D19C70, 0x00D19C6C, {}, 0},
    {0x00951C20, "TorpedoBomber", 0x007D1E50, 0x00D19F4C, 0x00D19F48, {}, 0},
    {0x00951D00, "Kamikaze", 0x007D1E50, 0x00D1A224, 0x00D1A220, {}, 0},
    {0x00951DE0, "LevelBomber", 0x007D1E50, 0x00D1A4F8, 0x00D1A4F4, {}, 0},
    {0x00951E30, "CommandBuilding", 0x00749180, 0x00D1A538, 0x00D1A534, {}, 0},
    {0x009536E0, "SmallReconPlane", 0x007D1E50, 0x00D1A5A8, 0x00D1A5A4, {}, 0},
    {0x00953770, "LargeReconPlane", 0x007D1E50, 0x00D1A5EC, 0x00D1A5E8, {}, 0},
    {0x0095FEE0, "AirField", 0x00749050, 0x00D1A9A0, 0x00D1A99C, {}, 0},
    {0x0095FF50, "Shipyard", 0x00749050, 0x00D1A9DC, 0x00D1A9D8, {}, 0},
    // 0095FFD7..0095FFEF: five dwords from one zeroed EAX.
    {0x0095FFC0, "LandVehicle", 0x00749050, 0x00D1AA18, 0x00D1AA14,
     {0x138, 0x140, 0x144, 0x148, 0x14C}, 5},
    {0x00960050, "DummyTargetVehicle", 0x00749050, 0x00D1AA58, 0x00D1AA54, {0x140}, 1},
    {0x00963B20, "Destroyer", 0x00963380, 0x00D1ACF8, 0x00D1ACF4, {}, 0},
    {0x00963BA0, "Cruiser", 0x00963380, 0x00D1AD38, 0x00D1AD34, {}, 0},
    {0x00963C20, "LandingShip", 0x00963380, 0x00D1AD78, 0x00D1AD74, {}, 0},
    {0x00963CB0, "Cargo", 0x00963380, 0x00D1ADBC, 0x00D1ADB8, {}, 0},
    {0x00963D30, "BattleShip", 0x00963380, 0x00D1ADF8, 0x00D1ADF4, {}, 0},
    {0x00963DB0, "Submarine", 0x00963380, 0x00D1AE38, 0x00D1AE34, {0x80C}, 1},
    {0x00963E40, "TorpedoBoat", 0x00963380, 0x00D1AE78, 0x00D1AE74, {}, 0},
    {0x00963EC0, "MotherShip", 0x00963380, 0x00D1AEBC, 0x00D1AEB8, {}, 0},
}};

const ClassDescriptorCtorRow* class_descriptor_ctor(std::uint32_t ctor) noexcept {
    for (const ClassDescriptorCtorRow& row : kClassDescriptorCtors) {
        if (row.ctor == ctor) return &row;
    }
    return nullptr;
}

// The CMP constants in each body's order. 00953330 / 009533B0 are the 7Dh / 7Eh
// session messages, listed with the message models below.
const std::array<IsKindOfRow, 24> kIsKindOfRows = {{
    {0x00953470, "MPlaneBomber", {0x10, 0x0F, 5, 4}, 4},
    {0x00953500, "MPlaneKamikaze", {0x17, 0x0F, 5, 4}, 4},
    {0x00953590, "MPlaneDiveBomber", {0x12, 0x0F, 5, 4}, 4},
    {0x00953620, "MPlaneTorpedoBomber", {0x11, 0x0F, 5, 4}, 4},
    {0x00953650, "MPlaneFighter", {0x13, 0x0F, 5, 4}, 4},
    {0x00953680, "MCommandBuilding", {0x1C, 0x1B, 5, 4}, 4},
    {0x009536B0, "MReconPlane", {0x14, 0x0F, 5, 4}, 4},
    {0x00953740, "MSmallReconPlane", {0x15, 0x14, 0x0F, 5, 4}, 5},
    {0x009537D0, "MLargeReconPlane", {0x16, 0x14, 0x0F, 5, 4}, 5},
    {0x0095FF30, "MAirfield", {0x45, 5, 4}, 3},
    {0x0095FFA0, "MShipyard", {0x46, 5, 4}, 3},
    {0x00960030, "MLandVehicle", {0x19, 5, 4}, 3},
    {0x009600A0, "MDummyTarget", {0x35, 5, 4}, 3},
    {0x009635E0, "ship base", {6, 5, 4}, 3},
    {0x00963B70, "MDestroyer", {7, 6, 5, 4}, 4},
    {0x00963BF0, "MCruiser", {0x0A, 6, 5, 4}, 4},
    {0x00963C80, "MLandingShip", {0x0C, 6, 5, 4}, 4},
    {0x00963D00, "MCargo", {0x0B, 6, 5, 4}, 4},
    {0x00963D80, "MBattleship", {0x0D, 6, 5, 4}, 4},
    {0x00963E10, "MSubmarine", {8, 6, 5, 4}, 4},
    {0x00963E90, "MTorpedoBoat", {0x0E, 6, 5, 4}, 4},
    {0x00963F10, "MMothership", {9, 6, 5, 4}, 4},
    {0x00953330, "session message 7Dh", {0x7D, 0x62, 0x49, 0x46}, 4},
    {0x009533B0, "session message 7Eh", {0x7E, 0x62, 0x49, 0x46}, 4},
}};

const IsKindOfRow* is_kind_of_row(std::uint32_t address) noexcept {
    for (const IsKindOfRow& row : kIsKindOfRows) {
        if (row.address == address) return &row;
    }
    return nullptr;
}

bool is_kind_of(const IsKindOfRow& row, std::int32_t id) noexcept {
    for (std::uint8_t i = 0; i < row.id_count; ++i) {
        if (id == static_cast<std::int32_t>(row.ids[i])) return true;
    }
    return false;
}

const std::array<PlaneInstanceCtorRow, 3> kPlaneInstanceCtors = {{
    {0x00951B60, "MPlaneDiveBomber",
     {0x00D19D28, 0x00D19D10, 0x00D19D08, 0x00D19D04, 0x00D19CFC, 0x00D19CE4, 0x00D19CE0,
      0x00D19CA0},
     0x12, 8},
    {0x00951C40, "MPlaneTorpedoBomber",
     {0x00D1A000, 0x00D19FE8, 0x00D19FE0, 0x00D19FDC, 0x00D19FD4, 0x00D19FBC, 0x00D19FB8,
      0x00D19F78},
     0x11, 9},
    {0x00951D20, "MPlaneKamikaze",
     {0x00D1A2D8, 0x00D1A2C0, 0x00D1A2B8, 0x00D1A2B4, 0x00D1A2AC, 0x00D1A294, 0x00D1A290,
      0x00D1A250},
     0x17, 0x0C},
}};

const std::array<ClassNameGetterRow, 3> kClassNameGetters = {{
    {0x00953720, 0x00D1A5D4, "MSmallReconPlane"},
    {0x009537B0, 0x00D1A618, "MLargeReconPlane"},
    {0x00963E00, 0x00D1AE68, "MSubmarine"},
}};

float ship_class_underwater_armour_009635d0(float field_6b4) noexcept { return field_6b4; }

bool landing_ship_class_slot_c12_clear_00963c70(std::uint8_t byte_809) noexcept {
    return byte_809 == 0;
}

bool vehicle_class_derive_constants_default_00951f20() noexcept {
    const std::uint8_t al = 1;  // 00876180: MOV AL, 1 ; RET
    return al != 0;
}

// 00939F20: CALL 00939CB0 at 00939F28, then MOV [ESI],00D196F0 at 00939F2D.
std::uint32_t* controller_construct_variant_c_00939f20(std::uint32_t* self, std::uintptr_t unit,
                                                       const ControllerVariantCtorHost& host) {
    host.base_construct(self, unit);
    *self = kControllerVariants[1].vtable;
    return self;
}

// 00939F40: CALL 00939CB0 at 00939F48, then MOV [ESI],00D196F4 at 00939F4D.
std::uint32_t* controller_construct_variant_d_00939f40(std::uint32_t* self, std::uintptr_t unit,
                                                       const ControllerVariantCtorHost& host) {
    host.base_construct(self, unit);
    *self = kControllerVariants[2].vtable;
    return self;
}

int controller_b_periscope_shape_00939e50(bool model_has_periszkop,
                                          const std::vector<std::uint32_t>& shape_flags) {
    if (!model_has_periszkop) return -1;                 // 00939EBF
    for (std::size_t i = 0; i < shape_flags.size(); ++i) {
        if ((shape_flags[i] & 2u) != 0) return static_cast<int>(i);  // 00939ED7 TEST AL, 2
    }
    return -1;
}

// ---------------------------------------------------------------------------
// 2. The ship repair task
// ---------------------------------------------------------------------------

void repair_set_fire_seconds_00939f90(RepairTask& task, float seconds) noexcept {
    task.fire_seconds_38 = seconds;    // MOVSS [ECX+38h]
}

void repair_set_water_seconds_00939fa0(RepairTask& task, float seconds) noexcept {
    task.water_seconds_34 = seconds;   // MOVSS [ECX+34h]
}

void repair_set_hull_repair_enabled_00939fd0(RepairTask& task, bool enabled) noexcept {
    task.hull_repair_enabled_45 = enabled;     // MOV [ECX+45h], AL
}

void repair_set_failure_repair_enabled_00939fe0(RepairTask& task, bool enabled) noexcept {
    task.failure_repair_enabled_46 = enabled;  // MOV [ECX+46h], AL
}

namespace {

// 0093A4B1..0093A4CB (fire) and 0093A533..0093A54D (water): FLD health; the
// water product FLD +34h FMUL +2Ch and the fire product +38h * +30h (FMULP ST3,
// DE C9+i) are added in extended, FCOMIP sum ? health, JBE returns. An
// unordered pair returns.
bool pending_exceeds_health(const RepairTask& task, float health) noexcept {
    const double sum = static_cast<double>(task.water_seconds_34) * task.water_tick_2c +
                       static_cast<double>(task.fire_seconds_38) * task.fire_tick_30;
    return sum > static_cast<double>(health);
}

}  // namespace

bool repair_add_fire_seconds_0093a470(RepairTask& task, float seconds, float unit_health_370,
                                      const RepairAddHost& host) {
    // 0093A484: the statistics call runs for every amount.
    if (host.warning_fire_channel_00983780) host.warning_fire_channel_00983780(seconds);
    // 0093A489..0093A4A4: FLD +38h, FLD s, FADDP ST2, FSTP tmp, FLD tmp, FST +38h.
    task.fire_seconds_38 = f32(static_cast<double>(task.fire_seconds_38) + seconds);
    task.byte_44 = 0;                                                 // 0093A494
    // 0093A4A7..0093A4AE: FLD +3Ch, FADDP with s, FSTP +3Ch.
    task.fire_total_3c = f32(static_cast<double>(task.fire_total_3c) + seconds);
    if (!pending_exceeds_health(task, unit_health_370)) return false;
    if (host.pending_exceeds_health_0090e6c0) host.pending_exceeds_health_0090e6c0();
    return true;
}

bool repair_add_water_seconds_0093a4f0(RepairTask& task, float seconds, float unit_health_370,
                                       const RepairAddHost& host) {
    // 0093A4F6 COMISS s, 0.0 ; JBE: only a positive amount reaches 009832F0.
    if (seconds > kZero_00d7a218 && host.warning_leak_channel_009832f0) {
        host.warning_leak_channel_009832f0(seconds);
    }
    // 0093A518..0093A530: FLD +34h, FADD s, FSTP tmp, FLD tmp, FST +34h, FST +40h.
    task.water_seconds_34 = f32(static_cast<double>(task.water_seconds_34) + seconds);
    task.water_last_40 = task.water_seconds_34;
    task.byte_44 = 0;                                                 // 0093A521
    if (!pending_exceeds_health(task, unit_health_370)) return false;
    if (host.pending_exceeds_health_0090e6c0) host.pending_exceeds_health_0090e6c0();
    return true;
}

// ---------------------------------------------------------------------------
// 3. InferiorFailure
// ---------------------------------------------------------------------------

namespace {

InferiorFailureMessage inferior_failure_message(std::uint32_t kind, std::uint32_t vtable) {
    InferiorFailureMessage msg;
    msg.kind = kind;
    msg.vtable = vtable;
    return msg;  // the remaining defaults are the stores of 009532E0
}

}  // namespace

InferiorFailureMessage inferior_failure_message_009532e0() noexcept {
    return inferior_failure_message(0x7D, 0x00D1A57C);
}

InferiorFailureMessage inferior_failure_message_00953360() noexcept {
    return inferior_failure_message(0x7E, 0x00D1A590);
}

bool inferior_failure_message_is_kind_of(std::uint32_t kind, std::int32_t id) noexcept {
    return id == static_cast<std::int32_t>(kind) || id == 0x62 || id == 0x49 || id == 0x46;
}

bool is_inferior_failure_name(const char* name) noexcept {
    // 00953DB9: a null property name (+4h) skips the compare.
    return name != nullptr && stricmp_equal(name, kInferiorFailure_00cf0b74);
}

bool unit_set_inferior_failure_00953da0(const char* name,
    const std::function<void(const InferiorFailureMessage&, int channel, int flags)>& route) {
    if (!is_inferior_failure_name(name)) return false;
    if (route) route(inferior_failure_message_009532e0(), kRouteChannel, 0);  // 00953E24
    return true;
}

bool unit_clear_inferior_failure_00953ea0(const char* name,
    const std::function<void(const InferiorFailureMessage&, int channel, int flags)>& route) {
    if (!is_inferior_failure_name(name)) return false;
    if (route) route(inferior_failure_message_00953360(), kRouteChannel, 0);  // 00953F24
    return true;
}

bool unit_query_inferior_failure_00953e60(const char* name, std::uint8_t byte_720,
                                          std::uint8_t* out) noexcept {
    if (!is_inferior_failure_name(name)) return false;
    if (out != nullptr) *out = byte_720;   // 00953E85..00953E8F
    return true;
}

// ---------------------------------------------------------------------------
// 4. MT_ROLEOWNER device walks
// ---------------------------------------------------------------------------

namespace {

bool kind_in_156(std::int32_t k) noexcept { return k == 1 || k == 5 || k == 6; }
bool kind_in_2346(std::int32_t k) noexcept { return k == 2 || k == 3 || k == 4 || k == 6; }

}  // namespace

std::vector<RoleStep> role_owner_walk_00953f60(const std::vector<RoleDevice>& devices,
    const RoleOwnerMessage& msg, std::int32_t unit_1bc) {
    std::vector<RoleStep> steps;
    if (msg.take_2c != 0) return steps;                 // 00953F64 CMP [EAX+2Ch], 0 ; JNE
    const bool same_slot = unit_1bc == msg.slot_28;     // 00953F74 SETE BL
    for (std::size_t i = 0; i < devices.size(); ++i) {
        const RoleDevice& d = devices[i];
        if (!d.is_kind_20h || !kind_in_156(d.descriptor_kind_80)) continue;
        // 00953FAD: on the same slot a device that is also in {2,3,4,6} stays.
        if (same_slot && d.is_kind_20h && kind_in_2346(d.descriptor_kind_80)) continue;
        steps.push_back({i, RoleAction::Release_00729f70, 0});
    }
    return steps;
}

std::vector<RoleStep> role_owner_walk_00954000(const std::vector<RoleDevice>& devices,
    const RoleOwnerMessage& msg) {
    std::vector<RoleStep> steps;
    const bool take = msg.take_2c == 1;                 // 00954009 CMP [EDI+2Ch], 1
    for (std::size_t i = 0; i < devices.size(); ++i) {
        const RoleDevice& d = devices[i];
        if (!d.is_kind_20h || !kind_in_2346(d.descriptor_kind_80)) continue;
        if (!take) {
            steps.push_back({i, RoleAction::Release_00729f70, 0});
            continue;
        }
        steps.push_back({i, RoleAction::Assign_vtable154, msg.slot_28});
        // 0095405E: the descriptor kind is re-read after the call.
        if (d.descriptor_kind_80 == 6) steps.push_back({i, RoleAction::SelectFireMode_0072d5b0, 0});
    }
    return steps;
}

std::vector<RoleStep> role_owner_walk_slots_009540d0(std::size_t device_count,
    const RoleOwnerMessage& msg) {
    std::vector<RoleStep> steps;
    const bool take = msg.take_2c == 1;                 // 009540F3 / 00954193
    for (std::size_t i = 0; i < device_count; ++i) {
        if (take) {
            steps.push_back({i, RoleAction::Assign_vtable154, msg.slot_28});
        } else {
            steps.push_back({i, RoleAction::Assign_vtable154, kNoSenderSlot});
            steps.push_back({i, RoleAction::Activate_vtable1e8, 0});
        }
    }
    return steps;
}

// ---------------------------------------------------------------------------
// 5. Unit state setters
// ---------------------------------------------------------------------------

SkillApplyPlan unit_skill_apply_level_009565a0(bool dead_5d, std::int32_t level,
    bool has_object_6dc, const std::vector<RoleDevice>& devices) {
    SkillApplyPlan plan;
    if (dead_5d) return plan;                           // 009565A3 CMP [ESI-32Fh], 0
    plan.applied = true;
    plan.level_390 = level;                             // 009565B9 MOV [ESI+4], EDI
    plan.notify_object_6dc = has_object_6dc;            // 009565C4 CALL [EAX+18h]
    for (std::size_t i = 0; i < devices.size(); ++i) {
        if (devices[i].is_kind_20h) plan.devices.push_back(i);  // 009565EA CALL [EAX+1C4h]
    }
    return plan;
}

bool pose_setter_broadcasts(bool is_ship, bool is_plane, std::int32_t world_1fe4,
                            std::uint8_t world_218c) noexcept {
    return !is_ship && !is_plane && world_1fe4 == 1 && world_218c == 0;
}

PoseSetPlan unit_set_world_position_009583c0(bool is_ship, bool is_plane,
    std::int32_t world_1fe4, std::uint8_t world_218c) noexcept {
    PoseSetPlan plan;
    plan.broadcast_relocate = pose_setter_broadcasts(is_ship, is_plane, world_1fe4, world_218c);
    plan.call_vtable_114 = true;      // 00958443
    plan.set_translation_a4 = true;   // 00958445..00958459
    return plan;
}

PoseSetPlan unit_set_heading_009584d0(bool is_ship, bool is_plane,
    std::int32_t world_1fe4, std::uint8_t world_218c) noexcept {
    PoseSetPlan plan;
    plan.broadcast_relocate = pose_setter_broadcasts(is_ship, is_plane, world_1fe4, world_218c);
    plan.rotate_heading_0047f200 = true;  // 00958556
    plan.set_previous_6a4 = false;        // the heading setter stores no position
    return plan;
}

PoseSetPlan unit_set_world_matrix_009585b0() noexcept {
    PoseSetPlan plan;
    plan.copy_matrix_004134f0 = true;     // 009585BC into +74h; +6A4h = matrix +30h..+38h
    return plan;
}

AudioFlagPlan unit_broadcast_controlled_audio_flag_00954990(bool is_ship, bool has_bbc,
                                                            bool flag) noexcept {
    AudioFlagPlan plan;
    // 009549A6..009549B7: NEG/SBB makes 0 or -1 and TEST with the non-zero
    // constant 00E19AB0, so the arm runs exactly when +BBCh is non-null.
    if (is_ship && has_bbc) {
        plan.call_bbc = true;
        plan.bbc_value = flag ? 0.0f : kOne_00d7a24c;   // 009549BD XORPS / 009549C2
    }
    plan.byte_6b4 = flag ? 1 : 0;                        // 00954A00
    return plan;
}

int unit_add_launched_child_00957450(LaunchedChildSlots& slots, std::uintptr_t child,
    const std::function<void(int slot, std::uintptr_t child)>& register_pair_00694a60) {
    int taken = -1;
    for (int i = 0; i < static_cast<int>(slots.child_550.size()); ++i) {
        if (slots.child_550[static_cast<std::size_t>(i)] == 0) {  // 00957460 CMP [EAX], 0
            taken = i;
            break;
        }
    }
    if (taken >= 0) {
        std::uintptr_t& cell = slots.child_550[static_cast<std::size_t>(taken)];
        if (cell != child) {                             // 00957481, cell is 0 here
            cell = child;                                // 0095749A
            if (child != 0 && register_pair_00694a60) register_pair_00694a60(taken, child);
        }
        slots.last_slot_630 = taken;                     // 009574A9
        slots.byte_62c = 0;                              // 009574AF
    }
    if (slots.pending_638 > 0) --slots.pending_638;      // 009574B7..009574C4
    return taken;
}

void unit_set_vehicle_class_009553d0(RefCountedClass*& slot_538, RefCountedClass* cls) {
    RefCountedClass* old = slot_538;
    if (old == cls) return;                              // 009553DB
    slot_538 = cls;
    if (cls != nullptr) ++cls->refs_4;                   // [00CE221C] InterlockedIncrement
    if (old != nullptr && --old->refs_4 == 0) {          // [00CE2220] InterlockedDecrement
        old->destroyed = true;                           // 0095540B CALL [vtable+0]
    }
}

std::uint32_t plane_create_unit_instance_ctor(std::uint32_t create_unit) noexcept {
    switch (create_unit) {
    case 0x00956240: return 0x00951D20;  // 0095628E
    case 0x00956390: return 0x00951B60;
    case 0x009564E0: return 0x00951C40;
    default: return 0;
    }
}

CreatedPlaneUnit plane_class_create_unit(std::uint32_t create_unit, RefCountedClass* cls) {
    CreatedPlaneUnit unit;
    unit.instance_ctor = plane_create_unit_instance_ctor(create_unit);
    // The memset leaves +538h zero, so the inlined swap always takes cls.
    unit_set_vehicle_class_009553d0(unit.class_538, cls);   // 00956299..009562D7
    unit.class_354 = cls;                                    // 009562DD
    return unit;
}

int plane_unregister_from_world_lists(std::vector<std::uintptr_t>& owner_list,
                                      std::uintptr_t unit) {
    for (std::size_t i = 0; i < owner_list.size(); ++i) {
        if (owner_list[i] == unit) {                     // 00956220 CMP [EAX+8], ESI
            owner_list.erase(owner_list.begin() + static_cast<std::ptrdiff_t>(i));
            return static_cast<int>(i);                      // 0095622F 004837D0
        }
    }
    return -1;
}

PlaneVisiblePlan plane_set_visible_00951f40(bool has_node_4a4, bool visible) noexcept {
    PlaneVisiblePlan plan;
    if (!has_node_4a4) return plan;                      // 00951F43
    plan.acts = true;
    plan.attach = visible;                               // 00710AD0 / 00710B80
    plan.visibility = visible ? 1.0f : 0.0f;             // FLD1 / FLDZ
    return plan;
}

// ---------------------------------------------------------------------------
// 5b. Submarine periscope break and the frozen-hull step
// ---------------------------------------------------------------------------

PeriscopeBreakPlan submarine_break_periscope_009327f0(float clock_f876a4,
                                                      float periscope_repair_time_4c4) noexcept {
    PeriscopeBreakPlan plan;
    plan.acts = true;
    plan.state_122c = 2;                                  // 009327F7
    plan.clock_fast_125c = 0.0f;                          // 00932801
    plan.clock_1258 = clock_f876a4;                       // 00932811
    // 00932819..00932832: FLD clock, FSTP tmp, FLD [settings+4C4h], FADD tmp, FSTP.
    plan.deadline_1254 = f32(static_cast<double>(periscope_repair_time_4c4) + clock_f876a4);
    return plan;
}

PeriscopeBreakPlan submarine_break_periscope_009373c0(bool has_periscope_shape_394,
                                                      float clock_f876a4,
                                                      float periscope_repair_time_4c4) noexcept {
    if (!has_periscope_shape_394) return PeriscopeBreakPlan{};   // 009373C4 CMP [EDI+394h], 0
    PeriscopeBreakPlan plan =
        submarine_break_periscope_009327f0(clock_f876a4, periscope_repair_time_4c4);
    plan.hide_mast_node_1214 = true;                      // 009373DF 00B6DA70(0.0, 0)
    plan.shape_collision_c47fc0_2 = true;                 // 00937430 00C47FC0(2)
    return plan;
}

FrozenForcesPlan controller_apply_frozen_forces_00937630(std::uint8_t unit_byte_1189) noexcept {
    FrozenForcesPlan plan;
    if (unit_byte_1189 == 0) {                            // 0093763A CMP [EDI+1189h], 0
        plan.hydro_forces_009329c0 = true;                // 0093764B
        return plan;
    }
    plan.zero_body_velocities = true;                     // 00937675 / 00937697
    plan.order_ring_param_0080e170_zero = true;           // 009376A4
    return plan;
}

// ---------------------------------------------------------------------------
// 5c. The vehicle-class registry
// ---------------------------------------------------------------------------

void class_registry_release_cached_0095b9e0(ClassRegistryModel& reg, std::int32_t index) {
    if (index < 0) return;  // the image indexes without a lower bound check
    const auto i = static_cast<std::size_t>(index);
    if (i >= reg.cache_4.size()) reg.cache_4.resize(i + 1, nullptr);   // 0095B9F9
    RefCountedClass*& cell = reg.cache_4[i];
    if (cell != nullptr && --cell->refs_4 == 0) cell->destroyed = true;  // 0095BA0D / 0095BA1D
    cell = nullptr;                                                      // 0095BA26
}

void class_registry_clear_0095ba30(ClassRegistryModel& reg) {
    reg.cache_4.clear();          // 0095BA3C 00437BD0(0)
    reg.party_rows_4010.clear();  // 0095BA4F 004359E0(0)
}

bool class_registry_party_row_clear_0095bab0(ClassRegistryModel& reg, std::int32_t index,
                                             std::int32_t local_player_index,
                                             std::int32_t player_party_28) {
    if (local_player_index < 0 || local_player_index > 7) return false;  // 0095BAC0 / 0095BAC5
    if (index < 0 || player_party_28 < 0 || player_party_28 > 2) return false;  // model guard
    const auto i = static_cast<std::size_t>(index);
    if (i >= reg.party_rows_4010.size()) reg.party_rows_4010.resize(i + 1);   // 0095BAE0
    return reg.party_rows_4010[i][static_cast<std::size_t>(player_party_28)] == 0;  // 0095BB01
}

bool unit_is_current_player_unit_00965810(std::uintptr_t unit, bool is_plane,
                                          std::uintptr_t plane_owner_9d4,
                                          std::uintptr_t current_player_unit) noexcept {
    if (unit == 0) return false;                          // 00965813
    if (is_plane) return plane_owner_9d4 == current_player_unit;   // 00965828..00965837
    return unit == current_player_unit;                   // 0096583B..00965844
}

// --- class registry loaders (cc13-gprx) ------------------------------------------

// Compare order of 00964020. Sizes: the PUSH before 00BF55BE or the ECX before
// 00470B80 in each arm.
const std::array<ClassPreloadRow, 18> kClassPreloadRows = {{
    {"Destroyer", 0x808, PreloadAlloc::crt_new_memset, 0x00963380, 0x00D1ACF8, 0x00D1ACF4, true},   // 0096414A
    {"Cruiser", 0x80C, PreloadAlloc::crt_new_memset, 0x00963380, 0x00D1AD38, 0x00D1AD34, true},     // 009641AC
    {"Cargo", 0x80C, PreloadAlloc::crt_new_memset, 0x00963380, 0x00D1ADBC, 0x00D1ADB8, true},       // 00964207
    {"BattleShip", 0x80C, PreloadAlloc::crt_new_memset, 0x00963380, 0x00D1ADF8, 0x00D1ADF4, true},  // 00964266
    {"Submarine", 0x840, PreloadAlloc::crt_new_memset, 0x00963DB0, 0, 0, false},       // 009642C5
    {"TorpedoBoat", 0x814, PreloadAlloc::crt_new_memset, 0x00963E40, 0, 0, false},     // 00964312
    {"MotherShip", 0x870, PreloadAlloc::pool_00470b80, 0x00963EC0, 0, 0, false},       // 0096435F
    {"ReconPlane", 0x60C, PreloadAlloc::pool_00470b80, 0x00951AA0, 0, 0, false},       // 0096439B
    {"Fighter", 0x60C, PreloadAlloc::pool_00470b80, 0x00951AF0, 0, 0, false},          // 009643D7
    {"DiveBomber", 0x60C, PreloadAlloc::pool_00470b80, 0x00951B40, 0, 0, false},       // 00964413
    {"TorpedoBomber", 0x60C, PreloadAlloc::pool_00470b80, 0x00951C20, 0, 0, false},    // 0096444F
    {"Kamikaze", 0x60C, PreloadAlloc::pool_00470b80, 0x00951D00, 0, 0, false},         // 0096448B
    {"Bomber", 0x60C, PreloadAlloc::pool_00470b80, 0x00951DE0, 0, 0, false},           // 009644C7
    {"AirField", 0x140, PreloadAlloc::pool_00470b80, 0x0095FEE0, 0, 0, false},         // 00964503
    {"Shipyard", 0x138, PreloadAlloc::pool_00470b80, 0x0095FF50, 0, 0, false},         // 0096453F
    {"LandVehicle", 0x150, PreloadAlloc::pool_00470b80, 0x0095FFC0, 0, 0, false},      // 0096457B
    {"LandFort", 0x180, PreloadAlloc::pool_00470b80, 0x00749180, 0, 0, false},         // 009645B0
    {"CommandBuilding", 0x1AC, PreloadAlloc::pool_00470b80, 0x00951E30, 0, 0, false},  // 009645E5
}};

const ClassPreloadRow* class_preload_row_00964020(const char* type) noexcept {
    // A null or empty type skips the raw chain and matches none of the native
    // compares, so it ends at 00964727.
    if (type == nullptr || *type == '\0') return nullptr;
    for (const ClassPreloadRow& row : kClassPreloadRows) {
        if (stricmp_equal(type, row.type)) return &row;
    }
    return nullptr;
}

ClassPreloadResult class_preload_00964020(const std::vector<ClassPreloadEntry>& table) {
    ClassPreloadResult out;
    for (const ClassPreloadEntry& e : table) {
        const ClassPreloadRow* row = class_preload_row_00964020(e.type.c_str());
        if (row == nullptr) return out;  // 00964727: AL = 0, the rest is not built
        out.built.emplace_back(e.key, row);
    }
    out.completed = true;  // 0096470F MOV AL,1
    return out;
}

std::uintptr_t class_get_or_create_from_lua_index_009653b0(std::int32_t index,
    bool entry_is_table,
    const std::function<std::uintptr_t(std::int32_t, bool)>& get_or_create_00964790) {
    if (!entry_is_table || !get_or_create_00964790) return 0;  // 00965414 IsTable
    return get_or_create_00964790(index, true);                 // 0096544C..00965450
}

bool class_name_matches_00965480(const char* name_data, std::int32_t name_length,
                                 const char* class_name) noexcept {
    if (name_data == nullptr) return class_name == nullptr || *class_name == '\0';
    if (class_name == nullptr) return name_length == 0;  // 00965695 CMP [EBX],EAX
    return stricmp_equal(name_data, class_name);         // 0096569D 00BF7FBF
}

ClassByNameResult class_find_by_name_00965480(const char* name_data, std::int32_t name_length,
    const std::vector<std::pair<std::uintptr_t, const char*>>& cache_with_names,
    const std::vector<LuaClassNameEntry>& vehicle_class_table,
    const std::function<std::uintptr_t(std::int32_t, bool)>& get_or_create_00964790) {
    ClassByNameResult out;
    for (const auto& [cls, name] : cache_with_names) {   // registry +4h, count +8h
        if (cls == 0) continue;
        if (class_name_matches_00965480(name_data, name_length, name)) {  // name = cls+54h
            out.cls = cls;
            out.from_cache = true;
            return out;
        }
    }
    for (const LuaClassNameEntry& e : vehicle_class_table) {
        if (!e.key_is_integer || !e.value_is_table) continue;   // 009655A9, 009655BA
        if (!e.name_is_string) continue;                          // 009655E6
        if (!class_name_matches_00965480(name_data, name_length, e.name)) continue;
        out.key = e.key;
        out.cls = get_or_create_00964790 ? get_or_create_00964790(e.key, true) : 0;  // 0096562A
        return out;
    }
    return out;  // IsUnbound: null
}

// --- section failure effect (cc13-gprx) ------------------------------------------

SectionFailureEffectAction repair_set_section_failure_effect_0093aa90(
    const SectionFailureEffectInputs& in) noexcept {
    // 0093AAC9 / 0093AB62: CMP section, count; JB.
    if (in.section >= in.slot_count) return SectionFailureEffectAction::invalid_index;
    if (!in.on) {
        return in.slot_held ? SectionFailureEffectAction::stop_and_release   // 0093AB0E..0093AB36
                            : SectionFailureEffectAction::none;              // 0093AAE9
    }
    if (in.slot_held) return SectionFailureEffectAction::none;                // 0093AB7B
    if (!in.section_found || !in.section_effect_name || !in.unit_node_4a4) {
        return SectionFailureEffectAction::none;                              // 0093AB90..0093ABB3
    }
    return SectionFailureEffectAction::create;                                // 0093ABB5..0093AC01
}

// ---------------------------------------------------------------------------
// 6. Math
// ---------------------------------------------------------------------------

float round_to_step_0093eee0(float x, float step) noexcept {
    // 0093EEE0..0093EEEE: FLD x, FLD step, FDIVP (DE FA: ST2 = x / step),
    // FADD qword 0.5; 00BF7420 stores the double and CVTTSD2SI truncates.
    const double q = static_cast<double>(x) / step + kHalf_00d7a280;
    const auto n = static_cast<std::int32_t>(q);
    // 0093EEFD FIMUL, then FSTP float.
    return f32(static_cast<double>(step) * n);
}

bool matrix44_not_equal_0093efc0(const float (&a)[16], const float (&b)[16]) noexcept {
    for (int i = 0; i < 16; ++i) {
        // FUCOMIP ; TEST AH,44h ; JP: unequal or unordered.
        if (!(a[i] == b[i])) return true;
    }
    return false;
}

Vec3 rotate_about_point_00952340(const Vec3& pivot, const Vec3& axis, const Vec3& point,
    float angle, const std::function<Vec3(const Vec3& v, const Vec3& axis, float angle)>&
                     rotate_about_axis_0085c3f0) {
    // 00952347..00952370: each difference stored as a float.
    Vec3 d{f32(static_cast<double>(point.x) - pivot.x),
           f32(static_cast<double>(point.y) - pivot.y),
           f32(static_cast<double>(point.z) - pivot.z)};
    if (rotate_about_axis_0085c3f0) d = rotate_about_axis_0085c3f0(d, axis, angle);
    // 00952381..0095239C: pivot + d per axis, FSTP float.
    return Vec3{f32(static_cast<double>(pivot.x) + d.x), f32(static_cast<double>(d.y) + pivot.y),
                f32(static_cast<double>(pivot.z) + d.z)};
}

}  // namespace gameplay_seg61_62_cc13
}  // namespace bsp

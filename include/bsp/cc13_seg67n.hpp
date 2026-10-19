#pragma once

// cc13_seg67n: the segment-67 rows (00996120-009F6060) that cc16's Codex lane
// oct18b-nC left open (reports/oct18b_nc_summary.json, open_rows): mostly the
// one-line vtable slots of the plane bot tasks (docs/BOT_TASKS.md), their states
// and the ship AI state family. Packet cc13-seg67n, docs/CC13_SEG67N.md; grant:
// cc9 gameplay lead 2026-10-18 addendum, standalone, nothing wired.
//
// Every name is a hypothesis, not a recovered symbol. These are new C++
// interfaces over the native records, not drop-in replacements of the original
// ABI; each function's comment gives the native ABI. Native objects are raw
// byte blocks touched only at the offsets the image reads or writes, and every
// call the image makes goes through a named Cc13Seg67nHost method at the same
// point with the same arguments. Nothing here is wired into the game.
// Build-tested only.
//
// Task classes by primary vtable (docs/BOT_TASKS.md "The thirteen classes" and
// the string after each table): 00D1F738 depthcharge, 00D1F9B0 dogfight,
// 00D1FC70 dropkamikaze, 00D1FD40 kamikaze, 00D1FFA0 land, 00D20210 levelbomb,
// 00D20400 immelmann, 00D20480 gain altitude (tick 009BA2F0), 00D20500 stop,
// 00D207C0 split s, 00D20B68 moveto, 00D20BE0 attackmove, 00D210E0 strafe,
// 00D21228 takeoff, 00D213C8 torpedo. task+310h is the current state object.

#include <cstddef>
#include <cstdint>

namespace bsp {

template <class T>
inline T& cc13_seg67n_at(void* base, std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}

// The eight-byte {type, value} record the image builds on the stack for the
// archive write slot +0Ch: type 0 name string, 1 dword, 2 float (its bits),
// 3 byte, 4 object reference (the same records as Cc13Seg67bArchiveValue).
struct Cc13Seg67nArchiveValue {
    std::int32_t type = 0;
    std::uintptr_t payload = 0;
};

// Image addresses the bodies return, compare or pass.
inline constexpr std::uint32_t kSeg67nStopCommandClass = 0x00E08F88u;   // 009B95B2
inline constexpr std::uint32_t kSeg67nStrStage = 0x00D20474u;           // "stage"
inline constexpr std::uint32_t kSeg67nStrTarget = 0x00CFD964u;          // "target"
inline constexpr std::uint32_t kSeg67nStrAlt = 0x00D204F8u;             // "alt"
// Static bot-task factory records (each built by a CG static initializer:
// 00CD4FA0 / 00CD4F70 / 00CD4FD0 / Unwind@00CD5130 store a factory vtable over
// a 0099EBB0-registered name).
inline constexpr std::uint32_t kSeg67nImmelmannFactory = 0x00F8A4E8u;
inline constexpr std::uint32_t kSeg67nGainAltitudeFactory = 0x00F8A4D8u;
inline constexpr std::uint32_t kSeg67nStopFactory = 0x00F8A4F8u;
inline constexpr std::uint32_t kSeg67nMoveToFactory = 0x00F8A584u;

class Cc13Seg67nHost {
public:
    virtual ~Cc13Seg67nHost() = default;

    // task->vtable[+8h](), __thiscall(ECX = task), RET, EAX. The base body is
    // 007B40C0 (XOR EAX,EAX), never overridden in the tables read.
    virtual std::uint32_t task_vcall_08(void* task) = 0;
    // state->vtable[+18h](), __thiscall(ECX = state), RET, AL. The base body is
    // 007B45E0 (XOR AL,AL; docs/BOT_TASK_STATES.md).
    virtual bool state_vcall_18(void* state) = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld: __thiscall(ECX = entity).
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    // 009BE3E0 BSP_BotStateFollow_WaitValue: __fastcall(ECX = follow state), ST0.
    virtual float bot_state_follow_wait_value_009be3e0(void* state) = 0;
    // 0099B720: the task base wait value (FLD [00D7A260] = -1.0; RET), ST0.
    virtual float bot_task_base_wait_value_0099b720(void* task) = 0;
    // 0099D060 BSP_BotTask_SaveBase: __thiscall(ECX = task, archive), RET 4
    // (body: cc13_seg67b bot_task_save_base_0099d060).
    virtual void bot_task_save_base_0099d060(void* task, void* archive) = 0;
    // archive->vtable[+0Ch](name, value), RET 10h: the two records by value.
    virtual void archive_vcall_0c_write(void* archive, Cc13Seg67nArchiveValue name,
                                        Cc13Seg67nArchiveValue value) = 0;
    // object->vtable[+114h](), __thiscall(ECX = object), RET, EAX.
    virtual void* object_vcall_114(void* object) = 0;
    // 0071BE40 BSP_WeaponDirector_CurrentCommand: __fastcall(ECX = director), EAX.
    virtual std::uint32_t weapon_director_current_command_0071be40(void* director) = 0;

    // ---- batch C ----
    // 0099B690 BSP_BotTask_Retire: __thiscall(ECX = task), RET (body: cc13_seg67b
    // bot_task_retire_0099b690).
    virtual void bot_task_retire_0099b690(void* task) = 0;
    // 009B94E0 BSP_BotTaskSplitS_RollStep: __thiscall(ECX = task; byte wrap), RET 4
    // (body: cc13_seg67b bot_task_split_s_roll_step_009b94e0).
    virtual void bot_task_split_s_roll_step_009b94e0(void* task, bool wrap) = 0;
    // 00438B10 BSP_Math_SubtractWrappedAngle: (float first, float second) as pushed,
    // first = [ESP+4] on entry; RET 8, ST0.
    virtual float math_subtract_wrapped_angle_00438b10(float first, float second) = 0;
    // 0085F990 BSP_DebugDraw_LineRgbaFloats: __fastcall(ECX = from, EDX = to,
    // colour0[4], colour1[4] by value), RET 20h. colour0 is the lower stack block.
    virtual void line_rgba_floats_0085f990(const float* from, const float* to,
                                           const float* colour0, const float* colour1) = 0;
    // 00860710 BSP_DebugDraw_Circle64: __fastcall(ECX = centre, EDX = rgba; float
    // radius), RET 4.
    virtual void debug_circle64_00860710(const float* centre, const float* rgba,
                                         float radius) = 0;
    // 00860BE0 BSP_DebugDraw_WireSphereMarker: __fastcall(ECX = centre, EDX = rgba;
    // radius, rgba*, detailed byte), RET 0Ch.
    virtual void wire_sphere_00860be0(const float* centre, const float* rgba_edx, float radius,
                                      const float* rgba_stack, std::uint8_t detailed) = 0;
    // 0085FFF0 BSP_DebugDraw_ArrowRgbaFloatsOneColour: __fastcall(ECX = from, EDX =
    // to; rgba[4] by value), RET 10h.
    virtual void arrow_rgba_floats_one_colour_0085fff0(const float* from, const float* to,
                                                       const float* rgba) = 0;
    // The dword at 00E188A8, read just before the 004C7070 call.
    virtual std::uint32_t global_dword_00e188a8() = 0;
    // 004C7070 BSP_DiagnosticPrintf: cdecl(a, float b, float c, fmt, double, double),
    // the caller pops 20h.
    virtual void diagnostic_printf_004c7070_2d(std::uint32_t a, float b, float c,
                                               const char* fmt, double v0, double v1) = 0;
    // 00419510 BSP_Vector3f_Normalize: __fastcall(ECX = out, EDX = v), RET, EAX = out.
    virtual const float* vector3f_normalize_00419510(float* out, const float* v) = 0;
};

// ---- the pilot bot (vtable 00D1F348) ----
// 0099A810: slot +44h, __thiscall(ECX = bot), RET, EAX. With tasks (count
// bot+5Ch != 0) it tail-jumps to the head task's vtable[+8h]; else 0.
std::uint32_t pilot_bot_head_task_slot08_0099a810(Cc13Seg67nHost& host, void* bot);
// 0099C2A0: __thiscall(ECX = task), RET, EAX = 1 or 0. Called only from the
// undefined code at 007BBB30 (unit+9D8h set, ECX = the head task of [unit+DF4h]).
std::uint32_t bot_task_current_state_query18_0099c2a0(Cc13Seg67nHost& host, void* task);

// ---- one-line task slots ----
// Slot +10h constants, __thiscall(ECX = task, unused), RET, EAX. The meaning
// of the value is unread (it is not the kind id at task+300h).
std::uint32_t bot_task_depth_charge_slot10_009a54f0() noexcept;   // 4
std::uint32_t bot_task_dogfight_slot10_009a99f0() noexcept;       // 7
std::uint32_t bot_task_drop_kamikaze_slot10_009addd0() noexcept;  // 4
std::uint32_t bot_task_kamikaze_slot10_009af070() noexcept;       // 8
std::uint32_t bot_task_land_slot10_009b33b0() noexcept;           // 2
std::uint32_t bot_task_level_bomb_slot10_009b7bc0() noexcept;     // 4
std::uint32_t bot_task_immelmann_slot10_009ba910() noexcept;      // 0Ah
std::uint32_t bot_task_gain_altitude_slot10_009baa80() noexcept;  // 0Ah
std::uint32_t bot_task_stop_slot10_009bac50() noexcept;           // 0Bh
std::uint32_t bot_task_moveto_slot10_009c3160() noexcept;         // 3 (moveto and attackmove)
std::uint32_t bot_task_strafe_slot10_009cc3e0() noexcept;         // 6
std::uint32_t bot_task_takeoff_slot10_009cf9f0() noexcept;        // 1
std::uint32_t bot_task_torpedo_slot10_009d3290() noexcept;        // 5
// Slot +5Ch, the task's static factory record, __thiscall(ECX = task), RET, EAX.
std::uint32_t bot_task_immelmann_factory_record_009ba8f0() noexcept;
std::uint32_t bot_task_gain_altitude_factory_record_009baa70() noexcept;
std::uint32_t bot_task_stop_factory_record_009bac20() noexcept;
std::uint32_t bot_task_moveto_factory_record_009c3140() noexcept;  // moveto and attackmove
// 009BAC40: stop slot +30h, XOR AL,AL; RET.
bool bot_task_stop_slot30_009bac40() noexcept;
// 009CFA30: takeoff slot +24h, MOV AL,1; RET.
bool bot_task_takeoff_slot24_009cfa30() noexcept;
// 009A54B0: depthcharge slot +48h, __thiscall(ECX = task), RET, EAX = task+48Ch.
void* bot_task_depth_charge_target_009a54b0(void* task);
// 009A9960: dogfight slot +2Ch, the hit notice, __thiscall(ECX = task, hit),
// RET 4, AL = 1: task+4E0h (approach+E8h) = 0.0.
bool bot_task_dogfight_hit_notice_009a9960(void* task);
// 009AA3B0: dogfight slot +0Ch, __thiscall(ECX = task, out), RET 4, EAX = out:
// the target (task+4ACh, else the target squadron task+4C4h) world position
// into out+0..+8 with out+0Ch = 1; neither -> out+0Ch = 0.
void* bot_task_dogfight_target_record_009aa3b0(Cc13Seg67nHost& host, void* task, void* out);
// 009ABBC0: slot 0 of 00D1FB00, 00D1FC60 and 00D1FC68 (the drop-kamikaze
// approach), __thiscall(ECX = object, out), RET 4, EAX = out = object+4Ch..+54h.
void* bot_point_at_4c_009abbc0(void* object, void* out);
// Wait values (slot +4Ch), ST0: the follow state's when current, else 0099B720.
float bot_task_drop_kamikaze_wait_value_009ae260(Cc13Seg67nHost& host, void* task);
float bot_task_kamikaze_wait_value_009af2f0(Cc13Seg67nHost& host, void* task);
float bot_task_moveto_wait_value_009c3550(Cc13Seg67nHost& host, void* task);
// 009CFA40: takeoff slot +4Ch, ST0: 0.0 when the squadron control block
// (task+404h) exists and its +3D0h is not this unit (task+3FCh), else 0099B720.
float bot_task_takeoff_wait_value_009cfa40(Cc13Seg67nHost& host, void* task);
// 009B36C0: land slot +24h, AL: the current state is land/begin (+5D8h),
// land/final (+5F8h) or land/park (+620h).
bool bot_task_land_in_landing_states_009b36c0(void* task);
// 009B9590: stop slot +40h (the still-valid predicate), EAX: 2 with no
// squadron object at task+2FCh, else 1 when its director's current command is
// 00E08F88, else 0.
std::uint32_t bot_task_stop_still_valid_009b9590(Cc13Seg67nHost& host, void* task);
// Slot +60h saves, __thiscall(ECX = task, archive), RET 4.
void bot_task_immelmann_save_009ba9e0(Cc13Seg67nHost& host, void* task, void* archive);
void bot_task_gain_altitude_save_009bab70(Cc13Seg67nHost& host, void* task, void* archive);
void bot_task_stop_save_009bac30(Cc13Seg67nHost& host, void* task, void* archive);
// 009BE140: slot 0 of 00D209C4 (the entity move-to target, ctor 009BE0F0),
// __thiscall(ECX = target), RET, EAX = target+10h.
void* move_to_target_entity_009be140(void* target);

// ---- batch C: maneuver ticks and debug draws ----
// 009B9AF0: immelmann (00D20400) slot +64h, __thiscall(ECX = task; float dt,
// unread), RET 4. Stage at task+3F8h (0, 1, 2).
void bot_task_immelmann_tick_009b9af0(Cc13Seg67nHost& host, void* task, float dt);
// 009B9CA0: split s (00D207C0) slot +64h, __thiscall(ECX = task; float dt,
// unread), RET 4. Stage at task+3F8h.
void bot_task_split_s_tick_009b9ca0(Cc13Seg67nHost& host, void* task, float dt);
// 009BE590: slot +14h of the follow-state vtables 00D1F6FC, 00D1FC08, 00D201A0,
// 00D20AB8, 00D20D28, 00D21320 (the debug draw), __thiscall(ECX = state), RET.
void bot_state_follow_debug_draw_009be590(Cc13Seg67nHost& host, void* state);

// ---- batch C: the ship AI state family (docs/SHIP_AI_STATES.md) ----
// 009DAA60: slot +18h (the debug draw) of 15 state vtables: RET 8, both
// arguments unread.
void ship_ai_state_debug_draw_noop_009daa60(void* state, std::uint32_t a, std::uint32_t b) noexcept;
// 009DAF60: the land state's (00D21658) slot +18h, __thiscall(ECX = state; two
// dwords, unread), RET 8: a yellow arrow from the unit ([[state+4h]+AA8h]) to
// state+18h.
void ship_ai_land_debug_draw_009daf60(Cc13Seg67nHost& host, void* state);
// Slot +24h, the command object the state serves, __thiscall(ECX unused), RET, EAX.
std::uint32_t ship_ai_stop_command_009dac80() noexcept;         // 00E08F88
std::uint32_t ship_ai_follow_command_009dadb0() noexcept;       // 00E08F60
std::uint32_t ship_ai_land_command_009daf50() noexcept;         // 00E08FA0
std::uint32_t ship_ai_moveonpath_command_009db050() noexcept;   // 00E08F80
// 00E08F78 (attackmove): kamikaze_attack 00D216B8 and the sub-state vtables
// 00D2171C, 00D2174C, 00D2177C, 00D217AC.
std::uint32_t ship_ai_kamikaze_attack_command_009db350() noexcept;
std::uint32_t ship_ai_attack_substate_command_009db5a0() noexcept;
std::uint32_t ship_ai_attack_substate_command_009db610() noexcept;
std::uint32_t ship_ai_attack_substate_command_009db690() noexcept;
std::uint32_t ship_ai_attack_substate_command_009db7e0() noexcept;
// 00E08F78 too: the state vtables 00D218F0, 00D21920, 00D2195C, 00D21994 (slot +24h).
std::uint32_t ship_ai_state_command_009e49f0() noexcept;
std::uint32_t ship_ai_state_command_009e4ca0() noexcept;
std::uint32_t ship_ai_state_command_009e50d0() noexcept;
std::uint32_t ship_ai_state_command_009e6060() noexcept;

}  // namespace bsp

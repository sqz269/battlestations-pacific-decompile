// cc13_seg67b: segment 67 bot-task and pilot-bot framework, second pass (packet
// cc13-seg67A). See include/bsp/cc13_seg67b.hpp and docs/CC13_SEG67B.md.
// Hypothesis names, build-tested only, nothing wired.

#include "bsp/cc13_seg67b.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/ship_ai_path_follower.hpp"
#include "bsp/geometry_helpers.hpp"

#include <cmath>
#include <cstring>
#include <string>

namespace bsp {
namespace {

template <class T>
T& at(void* base, std::size_t offset) {
    return cc13_seg67b_at<T>(base, offset);
}

void* ptr(void* base, std::size_t offset) {
    return static_cast<std::uint8_t*>(base) + offset;
}

void* ptr_at(void* base, std::size_t offset) {
    return reinterpret_cast<void*>(at<std::uintptr_t>(base, offset));
}

std::uint32_t float_bits(float f) {
    std::uint32_t u = 0;
    std::memcpy(&u, &f, sizeof u);
    return u;
}

Cc13Seg67bArchiveValue record(std::int32_t type, std::uintptr_t payload) {
    Cc13Seg67bArchiveValue v;
    v.type = type;
    v.payload = payload;
    return v;
}

Cc13Seg67bArchiveValue name_record(std::uint32_t string_address) {
    return record(0, string_address);
}

// Bot-task offsets (task = the whole object).
constexpr std::size_t kTaskEmbedded = 0x004;   // 0099BE30's object
constexpr std::size_t kTaskAvoidFlags = 0x2E4;
constexpr std::size_t kTaskKind = 0x300;
constexpr std::size_t kTask304 = 0x304;
constexpr std::size_t kTask308 = 0x308;
constexpr std::size_t kTaskSub30C = 0x30C;
constexpr std::size_t kTaskState = 0x310;
constexpr std::size_t kTaskGun = 0x314;
constexpr std::size_t kTask340 = 0x340;
constexpr std::size_t kTask344 = 0x344;
constexpr std::size_t kTask348 = 0x348;        // gun+34h
constexpr std::size_t kTask34C = 0x34C;        // gun+38h
constexpr std::size_t kTaskRocket = 0x38C;
constexpr std::size_t kTask3E8 = 0x3E8;
constexpr std::size_t kTask3EC = 0x3EC;
constexpr std::size_t kTask3F0 = 0x3F0;
constexpr std::size_t kTaskBot = 0x3F4;

// Pilot plan offsets.
constexpr std::size_t kPlanOwner25C = 0x25C;
constexpr std::size_t kPlanPitchTarget = 0x2BC;
constexpr std::size_t kPlanHeadingTarget = 0x2C0;
constexpr std::size_t kPlanHeadingMode = 0x2CC;
constexpr std::size_t kPlanPitchMode = 0x2D0;
constexpr std::size_t kPlanUnit = 0x2F0;

// Unit offsets.
constexpr std::size_t kUnitPitch = 0xC64;
constexpr std::size_t kUnitC68 = 0xC68;

// Constants read from the image.
constexpr float kOne_00d7a24c = 1.0f;
constexpr float kMinusOne_00d7a260 = -1.0f;
constexpr double kThree_00d7a2b0 = 3.0;
constexpr double kFloor_00ce3e20 = 0.07999999821186066;   // 3FB47AE140000000, float 0.08
constexpr float kFloor_00d05b50 = 0.08f;                  // 3DA3D70A
constexpr float kPointFour_00ce7804 = 0.4f;               // 3ECCCCCD
constexpr double kPlus650_00d1f3a0 = 650.0;

// The tail both constructors share (0099C7E2-0099C888 / 0099C9DB-0099CA86).
// row = [00F8A30C] + bot+34h * 248h + 0Ch; the image tests the sum against 0.
void construct_base_tuning_tail(Cc13Seg67bHost& host, void* task) {
    void* const bot = ptr_at(task, kTaskBot);
    const std::uintptr_t row = static_cast<std::uintptr_t>(
        at<std::int32_t>(bot, 0x34) * 0x248 + host.robots_table_00f8a30c() + 0x0C);
    if (row == 0) return;
    void* const r = reinterpret_cast<void*>(row);
    // 0099C7FD-0099C829: 009FAB60(row+228h, row+220h, row+224h, row+22Ch).
    host.bot_task_gun_init_aim_009fab60(ptr(task, kTaskGun), at<float>(r, 0x228),
                                        at<float>(r, 0x220), at<float>(r, 0x224),
                                        at<float>(r, 0x22C));
    // 0099C82E-0099C85E: x = float(row+220h * 3.0); 0.08 (qword) > x takes
    // [00D05B50] 0.08f. FCOMI + JBE: an unordered x keeps x.
    const float x = static_cast<float>(static_cast<double>(at<float>(r, 0x220)) * kThree_00d7a2b0);
    const float gun38 = kFloor_00ce3e20 > static_cast<double>(x) ? kFloor_00d05b50 : x;
    at<float>(task, kTask34C) = gun38;                                      // 0099C864
    at<std::uint32_t>(task, kTask348) = at<std::uint32_t>(r, 0x230);        // 0099C871
    at<float>(task, kTask340) = kPointFour_00ce7804;                       // 0099C87A
    // 0099C874-0099C888: float(row+230h + 650.0).
    at<float>(task, kTask344) =
        static_cast<float>(static_cast<double>(at<float>(r, 0x230)) + kPlus650_00d1f3a0);
}

// The head both constructors share (0099C709-0099C7DB / 0099C8C9-0099C99F).
void construct_base_head(Cc13Seg67bHost& host, void* task, void* bot, std::uint32_t kind) {
    // 0099C70D-0099C729: 0099BE30(task+4, bot+4 ? bot+4 - 310h : 0).
    const std::uintptr_t b4 = at<std::uintptr_t>(bot, 4);
    void* const base = b4 != 0 ? reinterpret_cast<void*>(b4 - 0x310) : nullptr;
    host.bot_task_embedded_construct_0099be30(ptr(task, kTaskEmbedded), base);
    at<std::uint32_t>(task, 0) = kBotTaskVtable;                           // 0099C743
    at<std::uint32_t>(task, kTaskKind) = kind;                             // 0099C74B
    at<float>(task, kTask304) = kOne_00d7a24c;                             // 0099C75D
    // 0099C765: U(0.0, 1.0) on stream 1 (the shared generator), negated.
    const float u1 = host.random_uniform_00bd2f10(1, 0.0f, 1.0f);
    at<float>(task, kTask308) = -u1;                                       // 0099C76A FCHS
    at<std::uint32_t>(task, kTaskSub30C) = kBotTaskSubVtable30C;           // 0099C772
    at<std::uint32_t>(task, kTaskState) = 0;                               // 0099C77C
    host.bot_task_gun_construct_009faad0(ptr(task, kTaskGun), task);       // 0099C790
    host.opportunistic_rocket_construct_009fcef0(ptr(task, kTaskRocket), task);  // 0099C79C
    // 0099C7BA: U(-1.0, 0.0) on stream 0.
    at<float>(task, kTask3E8) = host.random_uniform_00bd2f10(0, kMinusOne_00d7a260, 0.0f);
    at<std::uintptr_t>(task, kTaskBot) = reinterpret_cast<std::uintptr_t>(bot);
    at<float>(task, kTask3EC) = kMinusOne_00d7a260;
    at<std::uint8_t>(task, kTask3F0) = 1;
}

}  // namespace

// 0099EBB0 (0099EBB0-0099EBCE). Image: [f] = 00D1F3E0; +4h = +8h = 0;
// +0Ch = [00F8A314]; [00F8A314] = f. The stack argument is not read.
void* bot_task_factory_register_0099ebb0(Cc13Seg67bHost& host, void* factory) {
    at<std::uint32_t>(factory, 0) = kBotTaskFactoryVtable;
    at<std::uint32_t>(factory, 4) = 0;
    at<std::uint32_t>(factory, 8) = 0;
    at<std::uintptr_t>(factory, 0x0C) =
        reinterpret_cast<std::uintptr_t>(host.bot_task_factory_head_00f8a314());
    host.set_bot_task_factory_head_00f8a314(factory);
    return factory;
}

// 0099EBE0 (0099EBE0-0099EC34). Image: [f] = 00D1F3E0; unlink f from the list
// at 00F8A314 (head compare, then a walk on +0Ch that stops at a null next);
// then if +8h (the name buffer) is set, 00419CC0() and 00BD1510(pool, +8h,
// +4h + 1, 1).
void bot_task_factory_unregister_0099ebe0(Cc13Seg67bHost& host, void* factory) {
    at<std::uint32_t>(factory, 0) = kBotTaskFactoryVtable;
    void* node = host.bot_task_factory_head_00f8a314();
    if (node == factory) {
        host.set_bot_task_factory_head_00f8a314(ptr_at(factory, 0x0C));      // 0099EBF2
    } else if (ptr_at(node, 0x0C) != nullptr) {
        // 0099EC00-0099EC0D: walk until next == f or next->next is null.
        for (;;) {
            void* const next = ptr_at(node, 0x0C);
            if (next == factory) {
                at<std::uintptr_t>(node, 0x0C) = at<std::uintptr_t>(factory, 0x0C);  // 0099EC14
                break;
            }
            node = next;
            if (ptr_at(node, 0x0C) == nullptr) break;
        }
    }
    void* const name = ptr_at(factory, 8);
    if (name != nullptr) {
        void* const pool = host.sized_storage_pool_00419cc0();
        host.sized_storage_pool_return_block_00bd1510(pool, name,
                                                      at<std::uint32_t>(factory, 4) + 1u, 1u);
    }
}

// 0099B630 (0099B630-0099B64C). plan+2D0h != 0 -> plan+2BCh, else the unit's
// pitch (plan+2F0h)+C64h.
float pilot_command_pitch_target_or_live_0099b630(void* plan) {
    if (at<std::int32_t>(plan, kPlanPitchMode) != 0) return at<float>(plan, kPlanPitchTarget);
    return at<float>(ptr_at(plan, kPlanUnit), kUnitPitch);
}

// 0099B650 (0099B650-0099B65A). +25Ch = the argument.
void bot_task_set_owner_25c_0099b650(void* plan, std::uint32_t owner) {
    at<std::uint32_t>(plan, kPlanOwner25C) = owner;
}

// 0099B690 (0099B690-0099B69C). PUSH ECX supplies 0099A870's argument (it
// returns with RET 4): 0099A870(ECX = task+3F4h bot, task), a JMP to 0099A660.
void bot_task_retire_0099b690(Cc13Seg67bHost& host, void* task) {
    pilot_bot_retire_task_0099a660(host, ptr_at(task, kTaskBot), task);
}

// 009A1A20 (009A1A20-009A1A86). plan+2CCh == 2 -> plan+2C0h. Otherwise
// w = plan+2F0h ? 0099D0A0(plan, unit+C68h) : 0.0, then
// 00438AA0(unit->vtable[50h](), w): the slot +50h call takes no argument and
// the w pushed before it is 00438AA0's second argument. The image makes the
// +50h call through plan+2F0h even when it is null (that path would fault).
float bot_task_commanded_heading_009a1a20(Cc13Seg67bHost& host, void* plan) {
    if (at<std::int32_t>(plan, kPlanHeadingMode) == 2) return at<float>(plan, kPlanHeadingTarget);
    void* const unit = ptr_at(plan, kPlanUnit);
    float w = 0.0f;
    if (unit != nullptr) w = host.pilot_heading_diff_0099d0a0(plan, at<float>(unit, kUnitC68));
    const float heading = host.entity_vcall_50_heading(unit);
    return host.math_add_wrapped_angle_00438aa0(heading, w);
}

// 0099C6F0 (0099C6F0-0099C8A1). EH frame 00CADBA7. Draws: 0099C765 stream 1
// U(0, 1) (shared generator), 0099C7BA stream 0 U(-1, 0).
void* bot_task_construct_base_0099c6f0(Cc13Seg67bHost& host, void* task, void* bot,
                                       std::uint32_t kind) {
    construct_base_head(host, task, bot, kind);
    construct_base_tuning_tail(host, task);
    return task;
}

// 0099C8B0 (0099C8B0-0099CA9F). The same head, then 0099C9D9
// archive->vtable[10h]({0, "avoidFlags"}, {1, &task+2E4h}) before the tuning
// tail (which reads bot through task+3F4h here).
void* bot_task_construct_base_from_stream_0099c8b0(Cc13Seg67bHost& host, void* task,
                                                   void* bot, void* archive,
                                                   std::uint32_t kind) {
    construct_base_head(host, task, bot, kind);
    host.archive_vcall_10_read(archive, name_record(kStrAvoidFlags),
                               record(1, reinterpret_cast<std::uintptr_t>(
                                             ptr(task, kTaskAvoidFlags))));
    construct_base_tuning_tail(host, task);
    return task;
}

// 0099D060 (0099D060-0099D091). archive->vtable[0Ch]({0, "avoidFlags"},
// {1, task+2E4h}).
void bot_task_save_base_0099d060(Cc13Seg67bHost& host, void* task, void* archive) {
    host.archive_vcall_0c_write(archive, name_record(kStrAvoidFlags),
                                record(1, at<std::uint32_t>(task, kTaskAvoidFlags)));
}

// 00996120 (00996120-00996170). EH frame 00CAD358. [c] = 00D1EA00; if +4h
// (the decorated condition) is set, its vtable[4](1); [c] = 00D05840.
void bot_condition_timed_destruct_00996120(Cc13Seg67bHost& host, void* condition) {
    at<std::uint32_t>(condition, 0) = kBotConditionTimedVtable;
    void* const decorated = ptr_at(condition, 4);
    if (decorated != nullptr) host.object_vcall_04_delete(decorated, 1);
    at<std::uint32_t>(condition, 0) = kPureCallVtable;
}

// 00996270 (00996270-009962FE). e = condition+18h. 00414DB0(e) when the byte
// e+C8h is 0; dx = float(c+1Ch - e+FCh), dz = float(c+24h - e+104h);
// h = e->vtable[50h](); a = float(_CIatan2(dx, dz)) (ST1 = dx, ST0 = dz at
// 009962BA, so atan2(dx, dz)); d = 00438B10(a, h); returns c+28h > |d|
// (FCOMI + JBE: unordered -> 0).
// UNCERTAINTY: std::atan2 in double stands for the CRT's FPATAN-based
// _CIatan2 (64-bit significand); the float store can differ by one ulp in rare
// cases. The subtractions are exact-or-once-rounded in either precision mode.
bool bot_condition_turn_to_test_00996270(Cc13Seg67bHost& host, void* condition) {
    void* const e = ptr_at(condition, 0x18);
    if (at<std::uint8_t>(e, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(e);
    const float dx = static_cast<float>(static_cast<double>(at<float>(condition, 0x1C)) -
                                        static_cast<double>(at<float>(e, 0xFC)));
    const float dz = static_cast<float>(static_cast<double>(at<float>(condition, 0x24)) -
                                        static_cast<double>(at<float>(e, 0x104)));
    const float h = host.entity_vcall_50_heading(e);
    const float a = static_cast<float>(std::atan2(static_cast<double>(dx), static_cast<double>(dz)));
    const float d = host.math_subtract_wrapped_angle_00438b10(a, h);
    return at<float>(condition, 0x28) > std::fabs(d);
}

// 00996460 (00996460-009964FF). archive->vtable[0Ch]({0,"type"}, {0,"timed"});
// when +4h is set: archive->vtable[4]({0,"decorated"}), decorated->vtable[0Ch]
// (archive), archive->vtable[8](); then archive->vtable[0Ch]({0,"timeLeft"},
// {2, float(c+8h - [00F876A4])}).
void bot_condition_timed_save_00996460(Cc13Seg67bHost& host, void* condition,
                                       void* archive) {
    host.archive_vcall_0c_write(archive, name_record(kStrType), name_record(kStrTimed));
    void* const decorated = ptr_at(condition, 4);
    if (decorated != nullptr) {
        host.archive_vcall_04_begin(archive, name_record(kStrDecorated));
        host.object_vcall_0c_save(decorated, archive);
        host.archive_vcall_08_end(archive);
    }
    const float left = static_cast<float>(static_cast<double>(at<float>(condition, 8)) -
                                          static_cast<double>(host.global_clock_00f876a4()));
    host.archive_vcall_0c_write(archive, name_record(kStrTimeLeft), record(2, float_bits(left)));
}

// 009965D0 (009965D0-00996634). EH frame 00CAD378. archive->vtable[0Ch](name,
// {4, observer+14h}); then 0042BCA0 on the by-value observer (callee-destroyed).
void bot_target_condition_save_target_ref_009965d0(Cc13Seg67bHost& host, void* archive,
                                                   Cc13Seg67bArchiveValue name,
                                                   void* observer) {
    host.archive_vcall_0c_write(archive, name, record(4, at<std::uint32_t>(observer, 0x14)));
    host.target_observer_destruct_0042bca0(observer);
}

// 00996670 (00996670-009966B2). EH frame 00CAD398. 0042BCA0(c+4); [c] = 00D05840.
void bot_target_condition_destruct_00996670(Cc13Seg67bHost& host, void* condition) {
    host.target_observer_destruct_0042bca0(ptr(condition, 4));
    at<std::uint32_t>(condition, 0) = kPureCallVtable;
}

// 00996770 (00996770-009967B2). Same body as 00996670 with EH frame 00CAD3D8.
void bot_target_condition_destruct_00996770(Cc13Seg67bHost& host, void* condition) {
    host.target_observer_destruct_0042bca0(ptr(condition, 4));
    at<std::uint32_t>(condition, 0) = kPureCallVtable;
}

// 00996720 (00996720-00996762). 007B5C70(c, arg); [c] = 00D1EA40; +1Ch..+28h =
// the four floats (MOVSS copies: bits preserved).
void* bot_condition_turn_to_construct_00996720(Cc13Seg67bHost& host, void* condition,
                                               void* arg, float x, float y, float z,
                                               float limit) {
    host.bot_condition_construct_base_007b5c70(condition, arg);
    at<std::uint32_t>(condition, 0) = kBotConditionTurnToVtable;
    std::memcpy(ptr(condition, 0x1C), &x, 4);
    std::memcpy(ptr(condition, 0x20), &y, 4);
    std::memcpy(ptr(condition, 0x24), &z, 4);
    std::memcpy(ptr(condition, 0x28), &limit, 4);
    return condition;
}

// 009968D0 (009968D0-00996B97). EH frame 00CAD444. A native string
// {length, buffer} = {0, 0} is read with archive->vtable[10h]({0,"type"}, {0,&s}).
//  - buffer set and _stricmp(buffer, "simpletimed") == 0: new(8), [o] = 00D1E9F0,
//    +4h = float([00F876A4] + 0.0 (qword 00D7A258)).
//  - else 00425850(s, "timed"): new(0Ch), [o] = 00D1EA00, +4h = 0, +8h = the same sum.
//  - else "entity" or "entityPair": no object (ESI stays 0).
//  - else "turnAway": new(28h), 007B5D30(o, 0, [00F87574], [00F87578], [00F8757C]).
//  - else "turnTo": new(2Ch), 00996720(o, 0, the same three floats, 0.0).
//  - else "turnToHdg": new(24h), 007B5C70(o, 0), [o] = 00D1EA30, +1Ch = +20h = 0.0.
//  - else "farAway": new(2Ch), 007B5E20(o, 0, &[00F87574], 0.0).
//  - else no object.
// A failed new also gives no object. Then o->vtable[8](archive) at 00996B53 is
// called unconditionally (a null object faults in the image; the host receives
// the null). Finally, with a buffer, 00419CC0() and 00BD1510(pool, buffer,
// length + 1, 1).
void* bot_condition_create_from_archive_009968d0(Cc13Seg67bHost& host, void* archive) {
    std::uint32_t s[2] = {0u, 0u};   // {length, buffer}
    host.archive_vcall_10_read(archive, name_record(kStrType),
                               record(0, reinterpret_cast<std::uintptr_t>(&s[0])));
    const char* const buffer = reinterpret_cast<const char*>(static_cast<std::uintptr_t>(s[1]));
    void* o = nullptr;
    const float* const v = host.global_vector_00f87574();
    if (buffer != nullptr && host.crt_stricmp_00bf7fbf(buffer, kStrSimpleTimed) == 0) {
        o = host.operator_new_00bf681b(8);
        if (o != nullptr) {
            at<std::uint32_t>(o, 0) = kBotConditionSimpleTimedVtable;
            at<float>(o, 4) = static_cast<float>(
                static_cast<double>(host.global_clock_00f876a4()) + 0.0);
        }
    } else if (host.native_string_equals_insensitive_00425850(&s[0], kStrTimed)) {
        o = host.operator_new_00bf681b(0x0C);
        if (o != nullptr) {
            at<std::uint32_t>(o, 0) = kBotConditionTimedVtable;
            at<std::uint32_t>(o, 4) = 0;
            at<float>(o, 8) = static_cast<float>(
                static_cast<double>(host.global_clock_00f876a4()) + 0.0);
        }
    } else if (host.native_string_equals_insensitive_00425850(&s[0], kStrEntity) ||
               host.native_string_equals_insensitive_00425850(&s[0], kStrEntityPair)) {
        o = nullptr;   // 009969C5 / 009969DB JNE 00996B53 with ESI = 0
    } else if (host.native_string_equals_insensitive_00425850(&s[0], kStrTurnAway)) {
        o = host.operator_new_00bf681b(0x28);
        if (o != nullptr) {
            host.bot_condition_point_off_nose_construct_007b5d30(o, nullptr, v[0], v[1], v[2]);
        }
    } else if (host.native_string_equals_insensitive_00425850(&s[0], kStrTurnTo)) {
        o = host.operator_new_00bf681b(0x2C);
        if (o != nullptr) {
            bot_condition_turn_to_construct_00996720(host, o, nullptr, v[0], v[1], v[2], 0.0f);
        }
    } else if (host.native_string_equals_insensitive_00425850(&s[0], kStrTurnToHdg)) {
        o = host.operator_new_00bf681b(0x24);
        if (o != nullptr) {
            host.bot_condition_construct_base_007b5c70(o, nullptr);
            at<std::uint32_t>(o, 0) = kBotConditionTurnToHdgVtable;
            at<float>(o, 0x1C) = 0.0f;
            at<float>(o, 0x20) = 0.0f;
        }
    } else if (host.native_string_equals_insensitive_00425850(&s[0], kStrFarAway)) {
        o = host.operator_new_00bf681b(0x2C);
        if (o != nullptr) {
            host.bot_condition_point_distance_construct_007b5e20(o, nullptr, v, 0.0f);
        }
    }
    host.object_vcall_08_load(o, archive);                                   // 00996B5B
    if (buffer != nullptr) {
        void* const pool = host.sized_storage_pool_00419cc0();
        host.sized_storage_pool_return_block_00bd1510(
            pool, const_cast<char*>(buffer), s[0] + 1u, 1u);
    }
    return o;
}

// 00996BB0 (00996BB0-00996BFB). EH frame 00CAD458. o = 009968D0(ECX = &reader);
// 00410630(&reader); returns o.
void* bot_condition_load_from_reader_00996bb0(Cc13Seg67bHost& host, void* reader) {
    void* const o = bot_condition_create_from_archive_009968d0(host, reader);
    host.native_string_destruct_00410630(reader);
    return o;
}

// 00996C00 (00996C00-00996C93). If archive->vtable[14h]({0,"decorated"}):
// vtable[4]({0,"decorated"}), c+4h = 009968D0(archive), vtable[8](). Then
// vtable[10h]({0,"timeLeft"}, {2, &t}) and c+8h = float([00F876A4] + t).
// t is the stack slot of the entry PUSH ECX, so before the read it holds the
// condition pointer's bits; an archive that does not write it leaves those.
void bot_condition_timed_load_00996c00(Cc13Seg67bHost& host, void* condition, void* archive) {
    std::uint32_t t_bits = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(condition));
    if (host.archive_vcall_14_has(archive, name_record(kStrDecorated))) {
        host.archive_vcall_04_begin(archive, name_record(kStrDecorated));
        at<std::uintptr_t>(condition, 4) = reinterpret_cast<std::uintptr_t>(
            bot_condition_create_from_archive_009968d0(host, archive));
        host.archive_vcall_08_end(archive);
    }
    host.archive_vcall_10_read(archive, name_record(kStrTimeLeft),
                               record(2, reinterpret_cast<std::uintptr_t>(&t_bits)));
    float t = 0.0f;
    std::memcpy(&t, &t_bits, 4);
    at<float>(condition, 8) = static_cast<float>(
        static_cast<double>(host.global_clock_00f876a4()) + static_cast<double>(t));
}

// 00996CA0 (00996CA0-00996CF5). When c+18h is set and its vtable[5Ch](1)
// answers nonzero: a by-value observer is copy-constructed from c+4h
// (00796BD0), its vtable set to 00CE3CF0, and 009965D0(archive, {0,"pID"},
// that copy) writes it (and destroys the copy).
void bot_target_condition_save_00996ca0(Cc13Seg67bHost& host, void* condition, void* archive) {
    void* const target = ptr_at(condition, 0x18);
    if (target == nullptr || !host.object_vcall_5c(target, 1)) return;
    alignas(4) std::uint8_t copy[0x18] = {};
    host.entity_observer_copy_construct_00796bd0(copy, ptr(condition, 4));
    at<std::uint32_t>(copy, 0) = kTargetObserverVtable;                      // 00996CD6
    bot_target_condition_save_target_ref_009965d0(host, archive, name_record(kStrPID), copy);
}

// 00996D00 (00996D00-00996D69). 00996CA0, then {0,"type"} = {0,"turnAway"} and
// {0,"pos"} = {5, &c+1Ch} (type 5, a float[3] by address).
void bot_condition_turn_away_save_00996d00(Cc13Seg67bHost& host, void* condition, void* archive) {
    bot_target_condition_save_00996ca0(host, condition, archive);
    host.archive_vcall_0c_write(archive, name_record(kStrType), name_record(kStrTurnAway));
    host.archive_vcall_0c_write(archive, name_record(kStrPos),
                                record(5, reinterpret_cast<std::uintptr_t>(ptr(condition, 0x1C))));
}

// 00996D70 (00996D70-00996E0D). 00996CA0, {0,"type"} = {0,"turnToHdg"},
// {0,"heading"} = {2, c+1Ch}, {0,"angle"} = {2, c+20h}.
void bot_condition_turn_to_heading_save_00996d70(Cc13Seg67bHost& host, void* condition,
                                                 void* archive) {
    bot_target_condition_save_00996ca0(host, condition, archive);
    host.archive_vcall_0c_write(archive, name_record(kStrType), name_record(kStrTurnToHdg));
    host.archive_vcall_0c_write(archive, name_record(kStrHeading),
                                record(2, at<std::uint32_t>(condition, 0x1C)));
    host.archive_vcall_0c_write(archive, name_record(kStrAngle),
                                record(2, at<std::uint32_t>(condition, 0x20)));
}

// 00996E10 (00996E10-00996EA9). 00996CA0, {0,"type"} = {0,"turnTo"},
// {0,"pos"} = {5, &c+1Ch}, {0,"angle"} = {2, c+28h}.
void bot_condition_turn_to_save_00996e10(Cc13Seg67bHost& host, void* condition, void* archive) {
    bot_target_condition_save_00996ca0(host, condition, archive);
    host.archive_vcall_0c_write(archive, name_record(kStrType), name_record(kStrTurnTo));
    host.archive_vcall_0c_write(archive, name_record(kStrPos),
                                record(5, reinterpret_cast<std::uintptr_t>(ptr(condition, 0x1C))));
    host.archive_vcall_0c_write(archive, name_record(kStrAngle),
                                record(2, at<std::uint32_t>(condition, 0x28)));
}

// 00996EB0 (00996EB0-00996F49). 00996CA0, {0,"type"} = {0,"farAway"},
// {0,"pos"} = {5, &c+1Ch}, {0,"dist2"} = {2, c+28h}.
void bot_condition_far_away_save_00996eb0(Cc13Seg67bHost& host, void* condition, void* archive) {
    bot_target_condition_save_00996ca0(host, condition, archive);
    host.archive_vcall_0c_write(archive, name_record(kStrType), name_record(kStrFarAway));
    host.archive_vcall_0c_write(archive, name_record(kStrPos),
                                record(5, reinterpret_cast<std::uintptr_t>(ptr(condition, 0x1C))));
    host.archive_vcall_0c_write(archive, name_record(kStrDist2),
                                record(2, at<std::uint32_t>(condition, 0x28)));
}

// 00996F50 (00996F50-00996FA3). archive->vtable[10h](name, {4, &e}) where e is
// the slot of the entry PUSH ECX (the archive pointer's bits until written).
// If observer+14h != e: 006952A0(old, observer) when old is set; +14h = e;
// 00694A60(e, observer) when e is set.
void bot_target_condition_load_target_00996f50(Cc13Seg67bHost& host, void* archive,
                                               void* observer, Cc13Seg67bArchiveValue name) {
    std::uintptr_t e = reinterpret_cast<std::uintptr_t>(archive);
    host.archive_vcall_10_read(archive, name, record(4, reinterpret_cast<std::uintptr_t>(&e)));
    void* const old = ptr_at(observer, 0x14);
    if (reinterpret_cast<std::uintptr_t>(old) == e) return;
    if (old != nullptr) host.observer_unregister_pair_006952a0(old, observer);
    at<std::uintptr_t>(observer, 0x14) = e;
    if (e != 0) host.observer_register_pair_00694a60(reinterpret_cast<void*>(e), observer);
}

namespace {
// The head every target-condition load shares (00996FB0-00996FF3 and its
// copies): when vtable[14h]({0,"pID"}) answers, 00996F50(archive, c+4h, {0,"pID"}).
void load_target_head(Cc13Seg67bHost& host, void* condition, void* archive) {
    if (host.archive_vcall_14_has(archive, name_record(kStrPID))) {
        bot_target_condition_load_target_00996f50(host, archive, ptr(condition, 4),
                                                  name_record(kStrPID));
    }
}
void read_field(Cc13Seg67bHost& host, void* archive, std::uint32_t name, std::int32_t type,
                void* field) {
    host.archive_vcall_10_read(archive, name_record(name),
                               record(type, reinterpret_cast<std::uintptr_t>(field)));
}
}  // namespace

// 00996FB0 (00996FB0-00996FF5). The pID head only.
void bot_target_condition_load_00996fb0(Cc13Seg67bHost& host, void* condition, void* archive) {
    load_target_head(host, condition, archive);
}

// 00997090 (00997090-00997101). pID head, then {0,"pos"} <- {5, &c+1Ch}.
void bot_condition_turn_away_load_00997090(Cc13Seg67bHost& host, void* condition, void* archive) {
    load_target_head(host, condition, archive);
    read_field(host, archive, kStrPos, 5, ptr(condition, 0x1C));
}

// 00997110 (00997110-009971AD). pID head, {0,"heading"} <- {2, &c+1Ch},
// {0,"angle"} <- {2, &c+20h}.
void bot_condition_turn_to_heading_load_00997110(Cc13Seg67bHost& host, void* condition,
                                                 void* archive) {
    load_target_head(host, condition, archive);
    read_field(host, archive, kStrHeading, 2, ptr(condition, 0x1C));
    read_field(host, archive, kStrAngle, 2, ptr(condition, 0x20));
}

// 009971B0 (009971B0-0099724D). pID head, {0,"pos"} <- {5, &c+1Ch},
// {0,"angle"} <- {2, &c+28h}.
void bot_condition_turn_to_load_009971b0(Cc13Seg67bHost& host, void* condition, void* archive) {
    load_target_head(host, condition, archive);
    read_field(host, archive, kStrPos, 5, ptr(condition, 0x1C));
    read_field(host, archive, kStrAngle, 2, ptr(condition, 0x28));
}

// 00997250 (00997250-009972ED). pID head, {0,"pos"} <- {5, &c+1Ch},
// {0,"dist2"} <- {2, &c+28h}.
void bot_condition_far_away_load_00997250(Cc13Seg67bHost& host, void* condition, void* archive) {
    load_target_head(host, condition, archive);
    read_field(host, archive, kStrPos, 5, ptr(condition, 0x1C));
    read_field(host, archive, kStrDist2, 2, ptr(condition, 0x28));
}


// ---------------------------------------------------------------------------
// States: the follow state and shared bodies.

namespace {
constexpr std::uint32_t kBotStateFollowVtable = 0x00D20AB8u;
constexpr std::uint32_t kFollowMember18Vtable = 0x00CF89B4u;
constexpr float kPointSix_00ce3d30 = 0.6000000238418579f;   // 3F19999A
constexpr float kHundred_00ce3d08 = 100.0f;                 // 42C80000
}  // namespace

// 009C2980 (009C2980-009C2A52). EH frame 00CAF80B. +4h = approach; +0Ch, +10h,
// +14h, +8h = 0; [s] = 00D20AB8; +1Ch, +20h, +24h, +2Ch = 0; byte +28h = 1;
// +18h = 00CF89B4 (the member's vtable); +6Ch = 0042E740() + 380h; +70h = 0.6;
// +74h = -U(0, 0.6) on stream 1 (009C2A0A, the shared generator); +88h = 100.0;
// bytes +84h, +85h = 0; +8Ch = 1.0. The float argument is not read.
void* bot_state_follow_construct_009c2980(Cc13Seg67bHost& host, void* state, void* approach,
                                          float unread) {
    (void)unread;
    at<std::uintptr_t>(state, 0x04) = reinterpret_cast<std::uintptr_t>(approach);
    at<std::uint32_t>(state, 0x0C) = 0;
    at<std::uint32_t>(state, 0x10) = 0;
    at<std::uint32_t>(state, 0x14) = 0;
    at<std::uint32_t>(state, 0x08) = 0;
    at<std::uint32_t>(state, 0x00) = kBotStateFollowVtable;
    at<std::uint32_t>(state, 0x1C) = 0;
    at<std::uint32_t>(state, 0x20) = 0;
    at<std::uint32_t>(state, 0x24) = 0;
    at<std::uint32_t>(state, 0x2C) = 0;
    at<std::uint8_t>(state, 0x28) = 1;
    at<std::uint32_t>(state, 0x18) = kFollowMember18Vtable;
    void* const tuning = host.game_tuning_singleton_0042e740();             // 009C29DB
    at<std::uintptr_t>(state, 0x6C) = reinterpret_cast<std::uintptr_t>(tuning) + 0x380u;
    at<float>(state, 0x70) = kPointSix_00ce3d30;
    const float u = host.random_uniform_00bd2f10(1, 0.0f, kPointSix_00ce3d30);   // 009C2A0A
    at<float>(state, 0x74) = -u;
    at<float>(state, 0x88) = kHundred_00ce3d08;
    at<std::uint8_t>(state, 0x84) = 0;
    at<std::uint8_t>(state, 0x85) = 0;
    at<float>(state, 0x8C) = kOne_00d7a24c;
    return state;
}

// 009BDE40 (009BDE40-009BDE5D). 007B8A90((s+4h)+4h) (the unit's turbo byte),
// then ((s+4h)+1Ch)+4Ch = 0.0.
void bot_state_follow_exit_009bde40(Cc13Seg67bHost& host, void* state) {
    void* const approach = ptr_at(state, 4);
    host.plane_clear_turbo_byte_007b8a90(ptr_at(approach, 4));
    at<float>(ptr_at(approach, 0x1C), 0x4C) = 0.0f;
}

// 009BDE80 (009BDE80-009BDEA1). MOVSS copies of the three arguments to +30h,
// +34h, +38h.
void bot_state_moveto_set_ranges_009bde80(void* state, float a, float b, float c) {
    std::memcpy(ptr(state, 0x30), &a, 4);
    std::memcpy(ptr(state, 0x34), &b, 4);
    std::memcpy(ptr(state, 0x38), &c, 4);
}

// ---------------------------------------------------------------------------
// Teardown bodies. Each is the image's member-destructor sequence in its
// order; EH state bookkeeping is not modelled.

namespace {
enum class Td : std::uint8_t {
    kStateBase,   // 007B45F0(p+off)
    kFollow,      // 006CDD70(p+off+18h), then 007B45F0(p+off)
    kMoveTo,      // 007B65E0(p+off)
    kRegistry,    // 004116D0(p ? p+off : null), the image's null-adjusted cast
    kCallback,    // 00695870(p+off)
    kVtable,      // [p] = off (a vtable address)
    kObserver,    // 0042BCA0(p+off)
};
struct TdOp {
    Td kind;
    std::uint32_t off;
};

void teardown(Cc13Seg67bHost& host, void* p, const TdOp* ops, std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
        const TdOp& o = ops[i];
        switch (o.kind) {
            case Td::kStateBase: host.bot_state_base_destruct_007b45f0(ptr(p, o.off)); break;
            case Td::kFollow:
                host.member_destruct_006cdd70(ptr(p, o.off + 0x18u));
                host.bot_state_base_destruct_007b45f0(ptr(p, o.off));
                break;
            case Td::kMoveTo: host.bot_state_moveto_destruct_007b65e0(ptr(p, o.off)); break;
            case Td::kRegistry:
                host.state_registry_destruct_004116d0(p != nullptr ? ptr(p, o.off) : nullptr);
                break;
            case Td::kCallback: host.callback_owner_destroy_00695870(ptr(p, o.off)); break;
            case Td::kVtable: at<std::uint32_t>(p, 0) = o.off; break;
            case Td::kObserver: host.target_observer_destruct_0042bca0(ptr(p, o.off)); break;
        }
    }
}

template <std::size_t N>
void teardown(Cc13Seg67bHost& host, void* p, const TdOp (&ops)[N]) {
    teardown(host, p, ops, N);
}

constexpr TdOp kFollowBody[] = {{Td::kFollow, 0}};
}  // namespace

// 009A3930, 009AD390, 009B55D0, 009C40F0, 009D0B20: 006CDD70(s+18h) then
// 007B45F0(s) (009A3958/009A3967, 009AD3B8/009AD3C7, 009B55F8/009B5607,
// 009C4118/009C4127, 009D0B48/009D0B57).
void bot_state_follow_destruct_009a3930(Cc13Seg67bHost& host, void* state) { teardown(host, state, kFollowBody); }
void bot_state_follow_destruct_009ad390(Cc13Seg67bHost& host, void* state) { teardown(host, state, kFollowBody); }
void bot_state_follow_destruct_009b55d0(Cc13Seg67bHost& host, void* state) { teardown(host, state, kFollowBody); }
void bot_state_dive_bomb_destruct_members_009c40f0(Cc13Seg67bHost& host, void* state) { teardown(host, state, kFollowBody); }
void bot_state_follow_destruct_009d0b20(Cc13Seg67bHost& host, void* state) { teardown(host, state, kFollowBody); }

// 009C2880 (009C2880-009C291E): +134h base; +9Ch follow; +84h base; registry
// +70h; [b] = 00D209CC; 0042BCA0(b+30h).
void bot_task_moveto_destruct_approach_block_009c2880(Cc13Seg67bHost& host, void* block) {
    static constexpr TdOp ops[] = {{Td::kStateBase, 0x134}, {Td::kFollow, 0x9C},
                                   {Td::kStateBase, 0x84},  {Td::kRegistry, 0x70},
                                   {Td::kVtable, 0x00D209CCu}, {Td::kObserver, 0x30}};
    teardown(host, block, ops);
}

// 009B6480 (009B6480-009B656E).
void bot_task_level_bomb_destruct_approach_block_009b6480(Cc13Seg67bHost& host, void* block) {
    static constexpr TdOp ops[] = {{Td::kStateBase, 0x2E0}, {Td::kStateBase, 0x2BC},
                                   {Td::kFollow, 0x218},    {Td::kStateBase, 0x1FC},
                                   {Td::kStateBase, 0x1D4}, {Td::kFollow, 0x13C},
                                   {Td::kMoveTo, 0xF0},     {Td::kRegistry, 0xDC},
                                   {Td::kCallback, 0x30}};
    teardown(host, block, ops);
}

// 009C70A0 (009C70A0-009C71D1).
void bot_task_dive_bomb_destruct_approach_block_009c70a0(Cc13Seg67bHost& host, void* block) {
    static constexpr TdOp ops[] = {{Td::kStateBase, 0x3C4}, {Td::kStateBase, 0x3A4},
                                   {Td::kStateBase, 0x380}, {Td::kStateBase, 0x35C},
                                   {Td::kStateBase, 0x33C}, {Td::kStateBase, 0x30C},
                                   {Td::kFollow, 0x26C},    {Td::kFollow, 0x1CC},
                                   {Td::kFollow, 0x134},    {Td::kMoveTo, 0xF8},
                                   {Td::kRegistry, 0xE4},   {Td::kCallback, 0x30}};
    teardown(host, block, ops);
}

// 009A4680 (009A4680-009A47A1).
void bot_task_depth_charge_destruct_approach_block_009a4680(Cc13Seg67bHost& host, void* block) {
    static constexpr TdOp ops[] = {{Td::kStateBase, 0x3A8}, {Td::kStateBase, 0x38C},
                                   {Td::kFollow, 0x2EC},    {Td::kStateBase, 0x2CC},
                                   {Td::kStateBase, 0x2A8}, {Td::kStateBase, 0x284},
                                   {Td::kFollow, 0x1E4},    {Td::kFollow, 0x14C},
                                   {Td::kMoveTo, 0x110},    {Td::kRegistry, 0xFC},
                                   {Td::kCallback, 0x7C}};
    teardown(host, block, ops);
}

// 009ADAE0 (009ADAE0-009ADBE1).
void bot_task_drop_kamikaze_destruct_approach_block_009adae0(Cc13Seg67bHost& host, void* block) {
    static constexpr TdOp ops[] = {{Td::kStateBase, 0x334}, {Td::kStateBase, 0x310},
                                   {Td::kFollow, 0x26C},    {Td::kFollow, 0x1C8},
                                   {Td::kStateBase, 0x1AC}, {Td::kFollow, 0x114},
                                   {Td::kMoveTo, 0xD8},     {Td::kRegistry, 0xC4},
                                   {Td::kCallback, 0x30}};
    teardown(host, block, ops);
}

// 009A93E0 (009A93E0-009A94DD): the members, then 009A6B90(b) at 009A94C8.
void bot_task_dogfight_destruct_approach_block_009a93e0(Cc13Seg67bHost& host, void* block) {
    static constexpr TdOp ops[] = {{Td::kStateBase, 0x338}, {Td::kStateBase, 0x310},
                                   {Td::kStateBase, 0x2EC}, {Td::kStateBase, 0x2AC},
                                   {Td::kStateBase, 0x284}, {Td::kFollow, 0x1EC},
                                   {Td::kFollow, 0x154},    {Td::kMoveTo, 0x118},
                                   {Td::kRegistry, 0x104}};
    teardown(host, block, ops);
    bot_approach_dogfight_destruct_body_009a6b90(host, block);
}

// 009C9A80 (009C9A80-009C9B26): ends with [b] = 00D20E84.
void bot_task_retreat_destruct_approach_block_009c9a80(Cc13Seg67bHost& host, void* block) {
    static constexpr TdOp ops[] = {{Td::kFollow, 0xCC},     {Td::kStateBase, 0xB4},
                                   {Td::kStateBase, 0x9C},  {Td::kStateBase, 0x80},
                                   {Td::kRegistry, 0x6C},   {Td::kVtable, 0x00D20E84u}};
    teardown(host, block, ops);
}

// 009D2390 (009D2390-009D249C).
void bot_task_torpedo_destruct_approach_block_009d2390(Cc13Seg67bHost& host, void* block) {
    static constexpr TdOp ops[] = {{Td::kFollow, 0x348},    {Td::kStateBase, 0x318},
                                   {Td::kStateBase, 0x2E0}, {Td::kStateBase, 0x2BC},
                                   {Td::kFollow, 0x220},    {Td::kFollow, 0x188},
                                   {Td::kMoveTo, 0x14C},    {Td::kRegistry, 0x138},
                                   {Td::kCallback, 0xB4}};
    teardown(host, block, ops);
}

// 009CBE90 (009CBE90-009CBF7E).
void bot_task_strafe_destruct_approach_block_009cbe90(Cc13Seg67bHost& host, void* block) {
    static constexpr TdOp ops[] = {{Td::kStateBase, 0x2E0}, {Td::kStateBase, 0x298},
                                   {Td::kStateBase, 0x278}, {Td::kStateBase, 0x254},
                                   {Td::kFollow, 0x1BC},    {Td::kFollow, 0x124},
                                   {Td::kMoveTo, 0xE8},     {Td::kRegistry, 0xD4},
                                   {Td::kCallback, 0x58}};
    teardown(host, block, ops);
}

// 009ACE50 (009ACE50-009ACF1E).
void bot_task_kamikaze_destruct_approach_block_009ace50(Cc13Seg67bHost& host, void* block) {
    static constexpr TdOp ops[] = {{Td::kStateBase, 0x280}, {Td::kStateBase, 0x25C},
                                   {Td::kFollow, 0x1C4},    {Td::kFollow, 0x12C},
                                   {Td::kMoveTo, 0xF0},     {Td::kRegistry, 0xDC},
                                   {Td::kCallback, 0x68}};
    teardown(host, block, ops);
}

// 009CF850 (009CF850-009CF8D3): ends with [b] = 00D21148.
void bot_task_takeoff_destruct_approach_block_009cf850(Cc13Seg67bHost& host, void* block) {
    static constexpr TdOp ops[] = {{Td::kStateBase, 0xC0},  {Td::kStateBase, 0x98},
                                   {Td::kStateBase, 0x78},  {Td::kStateBase, 0x50},
                                   {Td::kRegistry, 0x3C},   {Td::kVtable, 0x00D21148u}};
    teardown(host, block, ops);
}

// 009BCB70 (009BCB70-009BCBF8): ends with [b] = 00D20838.
void bot_approach_move_on_path_destroy_009bcb70(Cc13Seg67bHost& host, void* block) {
    static constexpr TdOp ops[] = {{Td::kStateBase, 0x104}, {Td::kFollow, 0x6C},
                                   {Td::kStateBase, 0x54},  {Td::kRegistry, 0x40},
                                   {Td::kVtable, 0x00D20838u}};
    teardown(host, block, ops);
}

// 009A6B90 (009A6B90-009A6BEF): 00653390(b+B8h), 006CDD70(b+A0h), 00695870(b+2Ch).
void bot_approach_dogfight_destruct_body_009a6b90(Cc13Seg67bHost& host, void* block) {
    host.member_destruct_00653390(ptr(block, 0xB8));
    host.member_destruct_006cdd70(ptr(block, 0xA0));
    host.callback_owner_destroy_00695870(ptr(block, 0x2C));
}

// 009BAF60 (009BAF7D-009BAFA8): [t] = 00D20388; +3F8h (the condition) set ->
// its vtable[4](1); 007B4030(t).
void bot_task_conditioned_destruct_body_009baf60(Cc13Seg67bHost& host, void* task) {
    at<std::uint32_t>(task, 0) = 0x00D20388u;
    void* const condition = ptr_at(task, 0x3F8);
    if (condition != nullptr) host.object_vcall_04_delete(condition, 1);
    host.bot_task_destruct_base_007b4030(task);
}

// 009C2920 (009C2920-009C2970): 009C2880(t ? t+3F8h : null), 007B4030(t).
void bot_task_moveto_destruct_body_009c2920(Cc13Seg67bHost& host, void* task) {
    bot_task_moveto_destruct_approach_block_009c2880(
        host, task != nullptr ? ptr(task, 0x3F8) : nullptr);
    host.bot_task_destruct_base_007b4030(task);
}

namespace {
void observer_member_body(Cc13Seg67bHost& host, void* member, std::uint32_t vtable) {
    at<std::uint32_t>(member, 0) = vtable;
    void* const entity = ptr_at(member, 0x14);
    if (entity != nullptr) host.observer_unregister_pair_006952a0(entity, member);
    host.callback_owner_destroy_00695870(member);
}
}  // namespace

// 009D60F0 (009D60F0-009D6147): [m] = 00D21510; +14h set -> 006952A0(+14h, m);
// 00695870(m).
void ship_ai_observer_member_destruct_body_009d60f0(Cc13Seg67bHost& host, void* member) {
    observer_member_body(host, member, 0x00D21510u);
}

// 009D5EB0 (009D5EB0-009D5F07): the same with [m] = 00D214F8.
void ship_ai_path_node_observer_destruct_body_009d5eb0(Cc13Seg67bHost& host, void* member) {
    observer_member_body(host, member, 0x00D214F8u);
}

// 009DB380 / 009DB3D0 / 009DB420: 009D60F0(o+8), then [o] = 00D056D0.
void ship_ai_observer_member_destruct_a_009db380(Cc13Seg67bHost& host, void* object) {
    ship_ai_observer_member_destruct_body_009d60f0(host, ptr(object, 8));
    at<std::uint32_t>(object, 0) = 0x00D056D0u;
}
void ship_ai_observer_member_destruct_b_009db3d0(Cc13Seg67bHost& host, void* object) {
    ship_ai_observer_member_destruct_body_009d60f0(host, ptr(object, 8));
    at<std::uint32_t>(object, 0) = 0x00D056D0u;
}
void ship_ai_observer_member_destruct_c_009db420(Cc13Seg67bHost& host, void* object) {
    ship_ai_observer_member_destruct_body_009d60f0(host, ptr(object, 8));
    at<std::uint32_t>(object, 0) = 0x00D056D0u;
}

// ---------------------------------------------------------------------------
// The moveto-style state's archive pair and the task-base predicates.

namespace {
constexpr std::uint32_t kStrTarget = 0x00CFD964u;    // "target"
constexpr std::uint32_t kStrMinAlt = 0x00D209FCu;    // "minAlt"
constexpr std::uint32_t kStrDiffAlt = 0x00D209F4u;   // "diffAlt"
constexpr std::uint32_t kStrRefDist = 0x00D209ECu;   // "refDist"
constexpr std::uint32_t kStateMember18BaseVtable = 0x00CE3CD4u;
constexpr std::uint32_t kStateVtable_00d20aec = 0x00D20AECu;
constexpr std::uint32_t kStateMember18Vtable_00d20ad4 = 0x00D20AD4u;
}  // namespace

// 009BEAA0 (009BEAA0-009BEB92). EH frame 00CAF6F8. handle->vtable[0Ch]
// ({0,"target"}, {4, s+2Ch}) on the handle itself (009BEAEF); then, on the
// handle's writer (handle+8h), vtable[0Ch]({0,"minAlt"}, {2, s+30h}) at
// 009BEB19, ({0,"diffAlt"}, {2, s+34h}) at 009BEB43, ({0,"refDist"}, {2, s+38h})
// at 009BEB6D; finally 004107B0(&handle) (009BEB7B). The float payloads are
// MOVSS bit copies.
void bot_approach_save_altitude_leg_009beaa0(Cc13Seg67bHost& host, void* state, void* handle) {
    host.archive_vcall_0c_write(handle, name_record(kStrTarget),
                                record(4, at<std::uint32_t>(state, 0x2C)));
    host.archive_vcall_0c_write(ptr_at(handle, 8), name_record(kStrMinAlt),
                                record(2, at<std::uint32_t>(state, 0x30)));
    host.archive_vcall_0c_write(ptr_at(handle, 8), name_record(kStrDiffAlt),
                                record(2, at<std::uint32_t>(state, 0x34)));
    host.archive_vcall_0c_write(ptr_at(handle, 8), name_record(kStrRefDist),
                                record(2, at<std::uint32_t>(state, 0x38)));
    host.archive_writer_ref_destroy_004107b0(handle);
}

// 009C2B80 (009C2B80-009C2C91). EH frame 00CAF87B. +4h = approach; +0Ch, +10h,
// +14h, +8h = 0; the +18h member: [+18h] = 00CE3CD4, +1Ch, +20h, +24h = 0, byte
// +28h = 0; [s] = 00D20AEC; [+18h] = 00D20AD4. handle->vtable[10h]({0,"target"},
// {4, &t}) (009C2C07); +2Ch = t; on the handle's reader (handle+8h)
// vtable[10h]({0,"minAlt"}, {2, &s+30h}), ({0,"diffAlt"}, {2, &s+34h}),
// ({0,"refDist"}, {2, &s+38h}) (009C2C28, 009C2C42, 009C2C5C); when +2Ch is set,
// 00694A60(ECX = +2Ch, EDX = s+18h) (009C2C67); then 00410630(&handle).
// UNCERTAINTY: t is the reused argument slot, which the image overwrites with an
// ESP value (009C2BD3) before the read; an archive that does not write it leaves
// that stack address in +2Ch. Here t starts at 0.
void* bot_state_construct_base_009c2b80(Cc13Seg67bHost& host, void* state, void* approach,
                                        void* handle) {
    at<std::uintptr_t>(state, 0x04) = reinterpret_cast<std::uintptr_t>(approach);
    at<std::uint32_t>(state, 0x0C) = 0;
    at<std::uint32_t>(state, 0x10) = 0;
    at<std::uint32_t>(state, 0x14) = 0;
    at<std::uint32_t>(state, 0x08) = 0;
    at<std::uint32_t>(state, 0x18) = kStateMember18BaseVtable;
    at<std::uint32_t>(state, 0x1C) = 0;
    at<std::uint32_t>(state, 0x20) = 0;
    at<std::uint32_t>(state, 0x24) = 0;
    at<std::uint8_t>(state, 0x28) = 0;
    at<std::uint32_t>(state, 0x00) = kStateVtable_00d20aec;
    at<std::uint32_t>(state, 0x18) = kStateMember18Vtable_00d20ad4;
    std::uintptr_t t = 0;
    host.archive_vcall_10_read(handle, name_record(kStrTarget),
                               record(4, reinterpret_cast<std::uintptr_t>(&t)));
    at<std::uintptr_t>(state, 0x2C) = t;
    void* const reader = ptr_at(handle, 8);
    read_field(host, reader, kStrMinAlt, 2, ptr(state, 0x30));
    read_field(host, ptr_at(handle, 8), kStrDiffAlt, 2, ptr(state, 0x34));
    read_field(host, ptr_at(handle, 8), kStrRefDist, 2, ptr(state, 0x38));
    void* const target = ptr_at(state, 0x2C);
    if (target != nullptr) host.observer_register_pair_00694a60(target, ptr(state, 0x18));
    host.native_string_destruct_00410630(handle);
    return state;
}

// 0099C210 (0099C210-0099C229). [00F8A30C] + (task+3F4h)+34h * 248h + 0Ch.
std::uintptr_t bot_task_pilot_robots_row_0099c210(Cc13Seg67bHost& host, void* task) {
    return static_cast<std::uintptr_t>(at<std::int32_t>(ptr_at(task, kTaskBot), 0x34) * 0x248) +
           host.robots_table_00f8a30c() + 0x0Cu;
}

// 0099C230 (0099C230-0099C266). g = [00E198C4]; when g, g+8Ch and the byte
// (g+8Ch)+4h are all set and ((g+8Ch)+34h) equals task+2F4h (the unit) or
// task+2FCh (the squadron): 0. Otherwise 1.
bool bot_task_break_off_allowed_for_unit_0099c230(Cc13Seg67bHost& host, void* task) {
    void* const g = host.global_object_00e198c4();
    if (g == nullptr) return true;
    void* const holder = ptr_at(g, 0x8C);
    if (holder == nullptr) return true;
    if (at<std::uint8_t>(holder, 4) == 0) return true;
    const std::uint32_t unit = at<std::uint32_t>(holder, 0x34);
    if (unit == at<std::uint32_t>(task, 0x2F4) || unit == at<std::uint32_t>(task, 0x2FC)) {
        return false;
    }
    return true;
}

// 0099C270 (0099C270-0099C293). task+2F4h (the unit) set and the byte
// unit+9C2h + [00F876B8]*8 set -> 1, else 0.
bool bot_task_tick_gate_passes_0099c270(Cc13Seg67bHost& host, void* task) {
    void* const unit = ptr_at(task, 0x2F4);
    if (unit == nullptr) return false;
    const std::size_t side = host.global_side_index_00f876b8();
    return at<std::uint8_t>(unit, 0x9C2 + side * 8) != 0;
}

// ---------------------------------------------------------------------------
// The dogfight approach's target setter and the conditioned task family's
// archive bodies.

namespace {
constexpr std::uint32_t kStrCondition = 0x00D2033Cu;        // "condition"
constexpr std::uint32_t kStrSafePitch = 0x00D2035Cu;        // "safePitch"
constexpr std::uint32_t kStrRefPitch = 0x00D20350u;         // "refPitch"
constexpr std::uint32_t kStrStarted = 0x00D20348u;          // "started"
constexpr std::uint32_t kStrBarrelRollSpeed = 0x00D20374u;  // "barrelRollSpeed"
constexpr std::uint32_t kStrLeft = 0x00D20368u;             // "left"
constexpr std::uint32_t kStrTimer = 0x00D06BE4u;            // "timer"
constexpr std::uint32_t kBotTaskConditionedVtable = 0x00D20388u;
}  // namespace

// 009A7650 (009A7650-009A76DD). edi = target. When approach+B4h differs from it
// (009A7659; 009A7667 repeats the compare on the reloaded value): 006952A0(ECX =
// old, EDX = approach+A0h) when the old one is set (009A7677), +B4h = target
// (009A767E), 00694A60(ECX = target, EDX = approach+A0h) when the target is set
// (009A7687). Then 009FB5E0(ECX = approach+2Ch, [approach+B4h]) (009A7698),
// 009FA350(ECX = approach+2Ch) (009A769F), and 007B7870(ECX = [approach+20h],
// target, the three floats of 00F87574 by value) (009A76D3).
void bot_approach_dogfight_set_target_009a7650(Cc13Seg67bHost& host, void* approach,
                                               void* target) {
    void* const old = ptr_at(approach, 0xB4);
    if (old != target) {
        void* const member = ptr(approach, 0xA0);
        if (old != nullptr) host.observer_unregister_pair_006952a0(old, member);
        at<std::uintptr_t>(approach, 0xB4) = reinterpret_cast<std::uintptr_t>(target);
        if (target != nullptr) host.observer_register_pair_00694a60(target, member);
    }
    host.approach_ref_set_target_009fb5e0(ptr(approach, 0x2C), ptr_at(approach, 0xB4));
    host.approach_ref_reset_aim_offset_009fa350(ptr(approach, 0x2C));
    const float* const g = host.global_vector_00f87574();
    const float point[3] = {g[0], g[1], g[2]};
    host.opportunistic_rocket_set_target_007b7870(ptr_at(approach, 0x20), target, point);
}

// 009B9EA0 (009B9EA0-009B9F1B). EH frame 00CAF0A8. 00BD5010(ECX = archive, &ref,
// {0,"condition"}) (009B9EDA); [task+3F8h]->vtable[0Ch](&ref) (009B9EF7; no null
// check); 004107B0(&ref) (009B9F05). The base task save 0099D060 is not called.
void bot_task_conditioned_save_009b9ea0(Cc13Seg67bHost& host, void* task, void* archive) {
    std::uint32_t ref[3] = {};
    host.archive_writer_ref_open_00bd5010(archive, ref, name_record(kStrCondition));
    host.object_vcall_0c_save(ptr_at(task, 0x3F8), ref);
    host.archive_writer_ref_destroy_004107b0(ref);
}

// 009B9F20 (009B9F20-009B9FC7). 009B9EA0(task, archive); then archive->vtable[0Ch]
// ({0,"safePitch"}, {2, +400h}) (009B9F5F), ({0,"refPitch"}, {2, +3FCh})
// (009B9F92), ({0,"started"}, {3, byte +404h}) (009B9FC1).
// UNCERTAINTY: the type-3 payload's upper three bytes are uninitialised stack in
// the image (only the byte at 009B9F9F is stored); here they are 0.
void bot_task_pull_up_save_009b9f20(Cc13Seg67bHost& host, void* task, void* archive) {
    bot_task_conditioned_save_009b9ea0(host, task, archive);
    host.archive_vcall_0c_write(archive, name_record(kStrSafePitch),
                                record(2, at<std::uint32_t>(task, 0x400)));
    host.archive_vcall_0c_write(archive, name_record(kStrRefPitch),
                                record(2, at<std::uint32_t>(task, 0x3FC)));
    host.archive_vcall_0c_write(archive, name_record(kStrStarted),
                                record(3, at<std::uint8_t>(task, 0x404)));
}

// 009BA1F0 (009BA1F0-009BA264). 009B9F20(task, archive); archive->vtable[0Ch]
// ({0,"barrelRollSpeed"}, {2, +408h}) (009BA22F); ({0,"started"}, {3, byte +404h})
// (009BA25E), a second write of the key 009B9F20 already wrote.
// UNCERTAINTY: as 009B9F20 for the type-3 payload's upper bytes.
void bot_task_barrel_roll_save_009ba1f0(Cc13Seg67bHost& host, void* task, void* archive) {
    bot_task_pull_up_save_009b9f20(host, task, archive);
    host.archive_vcall_0c_write(archive, name_record(kStrBarrelRollSpeed),
                                record(2, at<std::uint32_t>(task, 0x408)));
    host.archive_vcall_0c_write(archive, name_record(kStrStarted),
                                record(3, at<std::uint8_t>(task, 0x404)));
}

// 009BA270 (009BA270-009BA2E4). 009B9EA0(task, archive); archive->vtable[0Ch]
// ({0,"left"}, {3, byte +400h}) (009BA2AB); ({0,"timer"}, {2, +3FCh}) (009BA2DE).
// UNCERTAINTY: as 009B9F20 for the type-3 payload's upper bytes.
void bot_task_flik_flak_save_009ba270(Cc13Seg67bHost& host, void* task, void* archive) {
    bot_task_conditioned_save_009b9ea0(host, task, archive);
    host.archive_vcall_0c_write(archive, name_record(kStrLeft),
                                record(3, at<std::uint8_t>(task, 0x400)));
    host.archive_vcall_0c_write(archive, name_record(kStrTimer),
                                record(2, at<std::uint32_t>(task, 0x3FC)));
}

// 009BA830 (009BA830-009BA8B3). EH frame 00CAF0C8. 0099C8B0(ECX = task, bot,
// archive, 0Fh) (009BA85A); [task] = 00D20388 (009BA871); 00BD4FE0(ECX = archive,
// the outgoing argument block, {0,"condition"}) (009BA88F); +3F8h =
// 00996BB0(that block by value) (009BA894, 009BA89D), which destroys the block.
void* bot_task_conditioned_construct_from_stream_009ba830(Cc13Seg67bHost& host, void* task,
                                                          void* bot, void* archive) {
    bot_task_construct_base_from_stream_0099c8b0(host, task, bot, archive, 0x0F);
    at<std::uint32_t>(task, 0x00) = kBotTaskConditionedVtable;
    std::uint32_t ref[3] = {};
    host.archive_reader_ref_open_00bd4fe0(archive, ref, name_record(kStrCondition));
    at<std::uintptr_t>(task, 0x3F8) =
        reinterpret_cast<std::uintptr_t>(bot_condition_load_from_reader_00996bb0(host, ref));
    return task;
}

namespace {
constexpr std::uint32_t kDogfightTaskVtable = 0x00D1F9B0u;
constexpr std::uint32_t kDogfightApproachVtable = 0x00D1F9ACu;
constexpr std::uint32_t kDogfightTask4FCVtable = 0x00D1F9A8u;
}  // namespace

// 009A9810 (009A9810-009A98B6). EH frame 00CAE476. 0099C6F0(ECX = task, bot, 2)
// (009A9836); 009A94E0(ECX = task+3F8h, bot, arg) (009A9851); [task] = 00D1F9B0,
// [task+3F8h] = 00D1F9AC, [task+4FCh] = 00D1F9A8 (009A985E-009A986A);
// 009F9980(ECX = task+3F8h, task) (009A9874); s = 007B8AD0(ECX = [task+3FCh]) ?
// task+510h : task+54Ch (009A987F-009A988E); task+310h = s; s->vtable[4]()
// (009A989F).
void* bot_task_dogfight_construct_009a9810(Cc13Seg67bHost& host, void* task, void* bot,
                                           void* arg) {
    bot_task_construct_base_0099c6f0(host, task, bot, 2);
    bot_task_dogfight_construct_approach_block_009a94e0(host, ptr(task, 0x3F8), bot, arg);
    at<std::uint32_t>(task, 0x000) = kDogfightTaskVtable;
    at<std::uint32_t>(task, 0x3F8) = kDogfightApproachVtable;
    at<std::uint32_t>(task, 0x4FC) = kDogfightTask4FCVtable;
    host.bot_approach_bind_to_task_009f9980(ptr(task, 0x3F8), task);
    void* const state = host.unit_is_squadron_flight_leader_007b8ad0(ptr_at(task, 0x3FC))
                            ? ptr(task, 0x510)
                            : ptr(task, 0x54C);
    at<std::uintptr_t>(task, 0x310) = reinterpret_cast<std::uintptr_t>(state);
    host.state_vcall_04_enter(state);
    return task;
}

// 009AB490 (009AB490-009AB4FF). arg = unit ? [unit+9D4h] : 0 (009AB4A7-009AB4B0);
// t = operator new(758h) (009AB4BB); null -> 0; else 009A9810(ECX = t, bot, arg).
void* bot_task_dogfight_create_for_squadron_009ab490(Cc13Seg67bHost& host, void* bot,
                                                     void* unit) {
    void* const arg = unit != nullptr ? ptr_at(unit, 0x9D4) : nullptr;
    void* const t = host.operator_new_00bf681b(0x758);
    if (t == nullptr) return nullptr;
    return bot_task_dogfight_construct_009a9810(host, t, bot, arg);
}

// 009AB500 (009AB500-009AB565). t = operator new(758h); null -> 0; else
// 009A9810(ECX = t, bot, arg = EDX).
void* bot_task_dogfight_create_009ab500(Cc13Seg67bHost& host, void* bot, void* arg) {
    void* const t = host.operator_new_00bf681b(0x758);
    if (t == nullptr) return nullptr;
    return bot_task_dogfight_construct_009a9810(host, t, bot, arg);
}

// 009AB570 (009AB570-009AB62C). With a target: target->vtable[5Ch](0Fh) ->
// 009AB490(bot, target) (009AB599, 009AB5A3); else target->vtable[5Ch](18h) ->
// 009AB500(bot, target) (009AB5C2, 009AB5CC). Otherwise (no target, or neither
// kind): t = operator new(758h) (009AB5E7); null -> 0; else 009A9810(t, bot, 0).
void* bot_task_make_dogfight_009ab570(Cc13Seg67bHost& host, void* bot, void* target) {
    if (target != nullptr) {
        if (host.object_vcall_5c(target, 0x0F)) {
            return bot_task_dogfight_create_for_squadron_009ab490(host, bot, target);
        }
        if (host.object_vcall_5c(target, 0x18)) {
            return bot_task_dogfight_create_009ab500(host, bot, target);
        }
    }
    void* const t = host.operator_new_00bf681b(0x758);
    if (t == nullptr) return nullptr;
    return bot_task_dogfight_construct_009a9810(host, t, bot, nullptr);
}

// 009BA1E0: a 5-byte JMP to 009B9EA0 (the leave-alone task adds no fields).
void bot_task_leave_alone_save_009ba1e0(Cc13Seg67bHost& host, void* task, void* archive) {
    bot_task_conditioned_save_009b9ea0(host, task, archive);
}

// ---------------------------------------------------------------------------
// The state-name registrars: one 00411E70(ECX = approach+registry, name, state)
// per state, in the image's order. The names are debug text (nothing looks a
// state up by name).

namespace {
struct StateNamePair {
    std::uint32_t name;    // string address in the image
    std::uint32_t offset;  // state object, approach-relative
};

void register_state_names(Cc13Seg67bHost& host, void* approach, std::uint32_t registry,
                          const StateNamePair* pairs, std::size_t count) {
    for (std::size_t i = 0; i < count; ++i) {
        host.state_registry_add_00411e70(ptr(approach, registry), pairs[i].name,
                                         ptr(approach, pairs[i].offset));
    }
}

constexpr StateNamePair kDepthChargeStateNamePairs[] = {
    {0x00D1F608u, 0x110},  // "moveto (DepthCharge)"   009A30A8
    {0x00D1F5F0u, 0x14C},  // "follow (DepthCharge)"   009A30BB
    {0x00D1F5DCu, 0x1E4},  // "DepthCharge/done"       009A30CE
    {0x00D1F5C4u, 0x284},  // "DepthCharge/attackrun"  009A30E1
    {0x00D1F5B0u, 0x2A8},  // "DepthCharge/goaway"     009A30F4
    {0x00D1F5A0u, 0x2CC},  // "DepthCharge/aim"        009A3107
    {0x00D1F58Cu, 0x2EC},  // "DepthCharge/prepare"    009A311A
    {0x00D1F578u, 0x38C},  // "DepthCharge/leave"      009A312D
    {0x00D1F564u, 0x3A8},  // "DepthCharge/turnto"     009A3140
};
constexpr StateNamePair kDogfightStateNamePairs[] = {
    {0x00D1F828u, 0x118},  // "moveto (dogfight)"      009A6A78
    {0x00D1F814u, 0x154},  // "follow (dogfight)"      009A6A8B
    {0x00D1F804u, 0x284},  // "dogfight/aim"           009A6A9E
    {0x00D1F7F0u, 0x1EC},  // "dogfight/prepare"       009A6AB1
    {0x00D1F7DCu, 0x2AC},  // "dogfight/maneuver"      009A6AC4
    {0x00D1F7C8u, 0x2EC},  // "dogfight/attackrun"     009A6AD7
    {0x00D1F7B4u, 0x310},  // "dogfight/avoid_roll"    009A6AEA
    {0x00D1F7A0u, 0x338},  // "dogfight/avoid_turn"    009A6AFD
};
constexpr StateNamePair kDropKamikazeStateNamePairs[] = {
    {0x00D1FABCu, 0x0D8},  // "moveto (DropKamikaze)"  009AB758
    {0x00D1FAA4u, 0x114},  // "follow (DropKamikaze)"  009AB76B
    {0x00D1FA90u, 0x26C},  // "DropKamikaze/done"      009AB77E
    {0x00D1FA7Cu, 0x334},  // "DropKamikaze/goAway"    009AB791
    {0x00D1FA64u, 0x1AC},  // "DropKamikaze/attackrun" 009AB7A4
    {0x00D1FA4Cu, 0x1C8},  // "DropKamikaze/prepare"   009AB7B7
    {0x00D1FA34u, 0x310},  // "DropKamikaze/release"   009AB7CA
};
constexpr StateNamePair kLevelBombStateNamePairs[] = {
    {0x00D2008Cu, 0x0F0},  // "moveto (LevelBomb)"     009B42E8
    {0x00D20078u, 0x13C},  // "follow (LevelBomb)"     009B42FB
    {0x00D20064u, 0x1D4},  // "LevelBomb/attackrun"    009B430E
    {0x00D20050u, 0x2E0},  // "LevelBomb/goaway"       009B4321
    {0x00D20040u, 0x1FC},  // "LevelBomb/aim"          009B4334
    {0x00D2002Cu, 0x218},  // "LevelBomb/prepare"      009B4347
    {0x00D20018u, 0x2BC},  // "LevelBomb/release"      009B435A
};
}  // namespace

// 009A3090 (009A3090-009A3147). Registry approach+FCh; nine pairs above.
void bot_task_depth_charge_register_state_names_009a3090(Cc13Seg67bHost& host, void* approach) {
    register_state_names(host, approach, 0xFC, kDepthChargeStateNamePairs,
                         sizeof(kDepthChargeStateNamePairs) / sizeof(kDepthChargeStateNamePairs[0]));
}

// 009A6A60 (009A6A60-009A6B04). Registry approach+104h; eight pairs above.
void bot_task_dogfight_register_state_names_009a6a60(Cc13Seg67bHost& host, void* approach) {
    register_state_names(host, approach, 0x104, kDogfightStateNamePairs,
                         sizeof(kDogfightStateNamePairs) / sizeof(kDogfightStateNamePairs[0]));
}

// 009AB740 (009AB740-009AB7D1). Registry approach+C4h; seven pairs above.
void bot_task_drop_kamikaze_register_state_names_009ab740(Cc13Seg67bHost& host,
                                                          void* approach) {
    register_state_names(
        host, approach, 0xC4, kDropKamikazeStateNamePairs,
        sizeof(kDropKamikazeStateNamePairs) / sizeof(kDropKamikazeStateNamePairs[0]));
}

// 009B42D0 (009B42D0-009B4361). Registry approach+DCh; seven pairs above.
void bot_task_level_bomb_register_state_names_009b42d0(Cc13Seg67bHost& host, void* approach) {
    register_state_names(host, approach, 0xDC, kLevelBombStateNamePairs,
                         sizeof(kLevelBombStateNamePairs) / sizeof(kLevelBombStateNamePairs[0]));
}

// ---------------------------------------------------------------------------
// The timed state constructors: base +4h = approach, +0Ch, +10h, +14h, +8h = 0,
// the vtable, a period of [00D7A24C] (1.0), a countdown of -00BD2F10(1, 0, 1)
// (shared RNG, stream 1) and a zeroed float, at offsets per state.

namespace {
constexpr std::uint32_t kDepthChargeAttackRunVtable = 0x00D1F64Cu;
constexpr std::uint32_t kDogfightAttackRunVtable = 0x00D1F8B0u;
constexpr std::uint32_t kDogfightManeuverVtable = 0x00D1F960u;
constexpr std::uint32_t kLevelBombAttackRunVtable = 0x00D20100u;
constexpr std::uint32_t kMoveOnPathCircleVtable = 0x00D208D8u;
constexpr float kHalf_00ce3800 = 0.5f;

void construct_state_base(void* state, void* approach) {
    at<std::uintptr_t>(state, 0x04) = reinterpret_cast<std::uintptr_t>(approach);
    at<std::uint32_t>(state, 0x0C) = 0;
    at<std::uint32_t>(state, 0x10) = 0;
    at<std::uint32_t>(state, 0x14) = 0;
    at<std::uint32_t>(state, 0x08) = 0;
}

void* construct_timed_state(Cc13Seg67bHost& host, void* state, void* approach,
                            std::uint32_t vtable, std::size_t period, std::size_t countdown,
                            std::size_t zeroed) {
    construct_state_base(state, approach);
    at<std::uint32_t>(state, 0) = vtable;
    at<float>(state, period) = kOne_00d7a24c;
    at<float>(state, countdown) = -host.random_uniform_00bd2f10(1, 0.0f, 1.0f);
    at<float>(state, zeroed) = 0.0f;
    return state;
}
}  // namespace

// 009A36A0 (009A36A0-009A371F). EH frame 00CADE78. Vtable 00D1F64C (009A36E3);
// +18h = 1.0 (009A36F5); 009A36FA 00BD2F10(1, 0, 1); +1Ch = -r (009A3704);
// +20h = 0 (009A370B).
void* bot_state_depth_charge_attack_run_construct_009a36a0(Cc13Seg67bHost& host, void* state,
                                                           void* approach) {
    return construct_timed_state(host, state, approach, kDepthChargeAttackRunVtable, 0x18, 0x1C,
                                 0x20);
}

// 009A70E0 (009A70E0-009A715F). The 009A36A0 body with vtable 00D1F8B0 (009A7123);
// 009A713A RNG; +18h / +1Ch / +20h.
void* bot_state_dogfight_attack_run_construct_009a70e0(Cc13Seg67bHost& host, void* state,
                                                       void* approach) {
    return construct_timed_state(host, state, approach, kDogfightAttackRunVtable, 0x18, 0x1C,
                                 0x20);
}

// 009A71E0 (009A71E0-009A74AE). The dogfight attack run's tick, slot +0Ch of 00D1F8B0.
// a = [state+4h] (the approach), unit = a+4h, plan = a+18h, direction = a+1Ch.
//  - 009A71EF-009A7260: h0 = pi/2 [00CE3830] - atan2(dz, dx) to the approach's target
//    point (vtable[0]), + 2pi [00CE3828] when negative; dx, dz are stored as floats.
//  - 009A7260-009A7301: dt < +1Ch (FCOMI, JB; a NaN dt takes it too) -> +1Ch = +1Ch - dt
//    (009A735A DE E9); else +1Ch = +1Ch + (+18h - dt) and +20h = (-A.x x B.y x B.z) x
//    pi/6 [00CEC730] from 007F0280(ECX = a+0Ch; unit, (100, 60, 120), &A, &B, (0, 0, 0),
//    0) (00CE3D08, 00CEB4B0, 00D05804).
//  - 009A7301-009A732A: plan+2C0h = 00438AA0(h0, +20h), plan+2CCh = 2.
//  - 009A7334-009A7411: r = min(a+D8h, 2000) ([00CF0DD8] compare, [00CFFD60] store);
//    hgt = 1400 [00D1F8D0] - unit+100h, raised to 50 ([00CE3938], [00CEB4D4]);
//    009FB800(ECX = state+4h; ctl+394h, 00419010(0.1, 0.35, 0.4, 1, hgt / r)) with
//    [00D7A2F0], [00CF6560], [00CE7804].
//  - 009A7411-009A743F: throttle 0.98 [00CE6650], air brake 0, both active, +2D8h = 0.
//  - 009A7449-009A747C: direction+40h = tuning+670h; 009FABE0(direction; 009A1A20(plan),
//    0099B630(plan)) - the 0099B630 result is pushed first and stays as the pitch.
//  - 009A7481-009A74A0: a+DCh = 1.0 [00D7A24C], a+E0h = 0.1 [00D7A2F0].
// x87 is modelled in float (24-bit); the double constants are rounded where the image
// stores. The three pi qwords are widened floats, used at their image values. No direct
// 00BD2F10 draws; 007F0280 and 009A1A20 each have one indirect call that was not chased.
// Unreferenced by address except through its vtable slot.
void bot_state_dogfight_attack_run_tick_009a71e0(Cc13Seg67bHost& host, void* state, float dt) {
    void* const a = at<void*>(state, 4);
    void* const unit = at<void*>(a, 4);
    if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);
    float pt[3];
    const float* const p = host.approach_vcall_00_target_point(a, pt);
    const float dz = p[2] - at<float>(unit, 0x104);
    const float dx = p[0] - at<float>(unit, 0xFC);
    // 009A723E FSUBR qword [00CE3830], 009A7252 FADD qword [00CE3828]: widened floats.
    float h0 = static_cast<float>(1.5707963705062866 - host.crt_atan2_00bf701a(dz, dx));
    if (0.0f > h0) h0 = static_cast<float>(h0 + 6.2831854820251465);

    const float c = at<float>(state, 0x1C);
    if (dt < c || std::isnan(dt) || std::isnan(c)) {
        at<float>(state, 0x1C) = c - dt;
    } else {
        at<float>(state, 0x1C) = c + (at<float>(state, 0x18) - dt);
        const float extents[3] = {100.0f, 60.0f, 120.0f};
        const float weights[3] = {0.0f, 0.0f, 0.0f};
        float out_a[3] = {0.0f, 0.0f, 0.0f};
        float out_b[3] = {0.0f, 0.0f, 0.0f};
        host.bot_near_field_probe_007f0280(at<void*>(a, 0x0C), unit, extents, out_a, out_b,
                                           weights, false);
        // With a null unit the image multiplies uninitialised stack (007F0280 skips the
        // zeroing at 007F02A7, JE 007F0B08); this pre-zeroes and gets 0. The unit is the
        // bot's own entity, so that never happens in practice.
        const float o = -out_a[0] * out_b[1] * out_b[2];
        // 009A72F8 FMUL qword [00CEC730], pi/6 as a widened float.
        at<float>(state, 0x20) = static_cast<float>(o * 0.5235987901687622);
    }
    void* const plan = at<void*>(a, 0x18);
    at<float>(plan, 0x2C0) = host.math_add_wrapped_angle_00438aa0(h0, at<float>(state, 0x20));
    at<std::uint32_t>(plan, 0x2CC) = 2;

    float r = at<float>(a, 0xD8);
    if (r > 2000.0f) r = 2000.0f;
    void* const u2 = at<void*>(a, 4);
    if (at<std::uint8_t>(u2, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(u2);
    float hgt = 1400.0f - at<float>(u2, 0x100);
    if (50.0f > hgt) hgt = 50.0f;
    const float factor = host.interpolate_clamped_00419010(0.1f, 0.35f, 0.4f, 1.0f, hgt / r);
    host.pilot_bot_command_pitch_from_altitude_009fb800(
        ptr(state, 4), at<float>(at<void*>(a, 0x0C), 0x394), factor);

    void* const plan2 = at<void*>(at<void*>(state, 4), 0x18);
    at<float>(plan2, 0x278) = 0.98f;
    at<std::uint8_t>(plan2, 0x27C) = 1;
    at<float>(plan2, 0x2A8) = 0.0f;
    at<std::uint8_t>(plan2, 0x2AC) = 1;
    at<std::uint32_t>(plan2, 0x2D8) = 0;

    void* const a2 = at<void*>(state, 4);
    at<float>(at<void*>(a2, 0x1C), 0x40) =
        at<float>(host.game_tuning_singleton_0042e740(), 0x670);
    void* const plan3 = at<void*>(a2, 0x18);
    const float pitch = pilot_command_pitch_target_or_live_0099b630(plan3);
    const float heading = bot_task_commanded_heading_009a1a20(host, plan3);
    host.bot_set_direction_009fabe0(at<void*>(a2, 0x1C), heading, pitch);
    at<float>(at<void*>(state, 4), 0xDC) = 1.0f;
    at<float>(at<void*>(state, 4), 0xE0) = 0.1f;
}

// 009A84E0 (009A84E0-009A855F). Vtable 00D1F960 (009A8523); +28h = 1.0 (009A8535);
// 009A853A RNG; +2Ch = -r (009A8544); +34h = 0 (009A854B). +18h..+24h and +30h
// are not written.
void* bot_state_dogfight_maneuver_construct_009a84e0(Cc13Seg67bHost& host, void* state,
                                                     void* approach) {
    return construct_timed_state(host, state, approach, kDogfightManeuverVtable, 0x28, 0x2C,
                                 0x34);
}

// 009B49B0 (009B49B0-009B4A2F). Vtable 00D20100 (009B49F3); 009B4A0A RNG;
// +18h / +1Ch / +20h.
void* bot_state_level_bomb_attack_run_construct_009b49b0(Cc13Seg67bHost& host, void* state,
                                                         void* approach) {
    return construct_timed_state(host, state, approach, kLevelBombAttackRunVtable, 0x18, 0x1C,
                                 0x20);
}

// 009BC960 (009BC960-009BC9DF). Base as above; [s] = 00D208D8 (009BC9A8);
// r = 00BD2F10(1, 0, 1) (009BC9AE); 009BC9BB FCOMIP r against [00CE3800] (0.5),
// JB -> 0, else 1, into byte +18h (009BC9CC). NaN takes the JB arm (0).
void* bot_state_move_on_path_circle_construct_009bc960(Cc13Seg67bHost& host, void* state,
                                                       void* approach) {
    construct_state_base(state, approach);
    at<std::uint32_t>(state, 0) = kMoveOnPathCircleVtable;
    const float r = host.random_uniform_00bd2f10(1, 0.0f, 1.0f);
    at<std::uint8_t>(state, 0x18) = (r >= kHalf_00ce3800) ? 1 : 0;
    return state;
}

namespace {
constexpr float kRollDone_00ce398c = 0.3490658700466156f;   // 20 degrees
constexpr double kRollBias_00d03dd0 = 1.0471975803375244;   // 60 degrees, float-rounded
constexpr double kRollSpan_00d20330 = 0.8726646453142166;   // 50 degrees
}  // namespace

// 009B9680 (009B9680-009B9736). u = task+2F4h. d = float(task+400h - u+C64h)
// (009B968D, 009B9693); 009B969B FCOMIP 0 against d, JBE (0 <= d or NaN) -> the
// command arm; else, when [00CE398C] > |u+C68h| (009B96C1 COMISS, JBE -> the
// command arm), 1. The command arm (009B96CB): +2C8h = 0, +2D0h = 1, byte +2A4h =
// 1, +2A0h = 0 - (|u+C68h| - double [00D03DD0]) * 1 / double [00D20330]
// (009B9706-009B9729, DC E9 / DE E9: ST(i) - ST(0)), +2D4h = 0; returns 0.
// UNCERTAINTY: the +2A0h chain is modelled at 24-bit precision (main thread);
// under 53-bit the quotient can differ in the last bit.
bool bot_task_maneuver_roll_completed_009b9680(Cc13Seg67bHost&, void* task) {
    return bot_task_maneuver_roll_completed_009b9680(task);
}

bool bot_task_maneuver_roll_completed_009b9680(void* task) {
    void* const unit = ptr_at(task, 0x2F4);
    const float d = at<float>(task, 0x400) - at<float>(unit, 0xC64);
    if (0.0f > d) {
        if (kRollDone_00ce398c > std::fabs(at<float>(unit, 0xC68))) return true;
    }
    at<float>(task, 0x2C8) = 0.0f;
    at<std::int32_t>(task, 0x2D0) = 1;
    const float a = std::fabs(at<float>(unit, 0xC68));
    at<std::uint8_t>(task, 0x2A4) = 1;
    const float x = static_cast<float>(static_cast<double>(a) - kRollBias_00d03dd0);
    const float q = static_cast<float>(static_cast<double>(x * 1.0f) / kRollSpan_00d20330);
    at<float>(task, 0x2A0) = 0.0f - q;
    at<std::int32_t>(task, 0x2D4) = 0;
    return false;
}

namespace {
constexpr double kClimbMargin_00ce3d78 = 1.5;
constexpr double kSixthPi_00cec730 = 0.5235987901687622;   // pi/6, float-rounded
constexpr float kRollPi_00d7a264 = 3.1415927410125732f;
constexpr double kZero_00d7a258 = 0.0;
constexpr double kBarrelBias_00d7a280 = 0.5;
constexpr double kGainAltTinySq_00ce3820 = 1e-10;
constexpr double kGainAltFar_00ce47a0 = 1000.0;
constexpr double kThirdPi_00d03dd0 = 1.0471975803375244;   // pi/3, float-rounded
constexpr float kGainAltJitterLo_00ce3cb4 = -0.1f;
constexpr float kGainAltJitterHi_00d7a2f0 = 0.1f;
constexpr double kOne_00d7a210 = 1.0;
constexpr double kGainAltPitch_00cedcd0 = 0.7853981852531433;  // pi/4, float-rounded

// The COMISS/FCOMIP clamp to [-1, 1] these ticks repeat: the low bound first, so a
// NaN passes through.
float clamp_unit_pm1(float x) {
    if (-1.0f > x) return -1.0f;
    if (x > 1.0f) return 1.0f;
    return x;
}

// The same pattern on [0, 1].
float clamp_unit_01(float x) {
    if (0.0f > x) return 0.0f;
    if (x > 1.0f) return 1.0f;
    return x;
}
}  // namespace

// 009B9740 (009B9740-009B99DB), barrelroll. u = task+2F4h, cls = task+2F8h.
//  - 009B9752-009B9791: +2E4h &= 0FBh (imm32: bits 8-31 clear too); +27Ch = 1.0, +280h = 1; +2ACh = 0, +2B0h = 1;
//    +2DCh = 0; +288h = 0, +28Ch = 1; +2D8h = 0.
//  - 009B97A4-009B97BC: e = cls+1ECh x 1.5 (qword 00CE3D78) - u+C64h, stored float.
//  - 009B9797-009B97D1: until byte +404h is set it takes 009B9680's answer; while that
//    is 0 the body skips to the end test.
//  - 009B97D7-009B97E9: +294h = +408h (the roll speed), +298h = 1, +2D0h = 0.
//  - 009B97EF-009B98CB: q = bank / (pi/6) (009B980F D8 F9: ST1 / ST0); |q| < 1 ->
//    c = clamp01(e / cls+278h) x (1 - |q|) (009B9850 DE E1: 1 - |q|) + 0.0 (qword).
//  - 009B98CF-009B9993: q2 = 00438B10(bank, pi) / (pi/6); |q2| < 1 -> c += clamp01(-e /
//    cls+278h) x (1 - |q2|) (009B993B FCHS).
//  - 009B9993-009B99B9: +2A0h = c + 0.5 (qword), +2A4h = 1, +2D4h = 0; byte +26Ch bit 0
//    -> 0099B690.
//  - 009B99BE-009B99D0: (+3F8h)->vtable[0]() true -> 0099B690.
// x87 is modelled at 24-bit precision (each FMUL/FSUB/FDIV rounded to float); 53-bit is
// the alternative. No draw. Only reference: vtable 00D206D0 slot +64h (00D20734, image
// scan); no target-equal twin.
void bot_task_barrel_roll_tick_009b9740(Cc13Seg67bHost& host, void* task, float) {
    at<std::uint32_t>(task, 0x2E4) &= 0xFBu;  // 81 A6 E4 02 00 00 FB 00 00 00 (imm32)
    at<float>(task, 0x27C) = 1.0f;
    at<std::uint8_t>(task, 0x280) = 1;
    at<float>(task, 0x2AC) = 0.0f;
    at<std::uint8_t>(task, 0x2B0) = 1;
    at<std::int32_t>(task, 0x2DC) = 0;
    at<float>(task, 0x288) = 0.0f;
    at<std::uint8_t>(task, 0x28C) = 1;
    at<std::int32_t>(task, 0x2D8) = 0;
    const float climb = static_cast<float>(
        static_cast<double>(at<float>(ptr_at(task, 0x2F8), 0x1EC)) * kClimbMargin_00ce3d78);
    const float excess = climb - at<float>(ptr_at(task, 0x2F4), 0xC64);

    bool run = at<std::uint8_t>(task, 0x404) != 0;
    if (!run) {
        run = bot_task_maneuver_roll_completed_009b9680(host, task);
        at<std::uint8_t>(task, 0x404) = run ? 1 : 0;
    }
    if (run) {
        at<std::uint8_t>(task, 0x298) = 1;
        at<float>(task, 0x294) = at<float>(task, 0x408);
        at<std::int32_t>(task, 0x2D0) = 0;
        const float bank = at<float>(ptr_at(task, 0x2F4), 0xC68);
        float c = 0.0f;
        const float q = static_cast<float>(static_cast<double>(bank) / kSixthPi_00cec730);
        const float aq = q > 0.0f ? q : -0.0f - q;
        if (1.0f > aq) {
            const float w = 1.0f - aq;
            const float g = clamp_unit_01(excess / at<float>(ptr_at(task, 0x2F8), 0x278));
            c = static_cast<float>(static_cast<double>(g * w) + kZero_00d7a258);
        }
        const float q2 = static_cast<float>(
            static_cast<double>(host.math_subtract_wrapped_angle_00438b10(bank, kRollPi_00d7a264)) /
            kSixthPi_00cec730);
        const float aq2 = q2 > 0.0f ? q2 : -0.0f - q2;
        if (1.0f > aq2) {
            const float w2 = 1.0f - aq2;
            const float g2 = clamp_unit_01(-excess / at<float>(ptr_at(task, 0x2F8), 0x278));
            c = g2 * w2 + c;
        }
        at<std::uint8_t>(task, 0x2A4) = 1;
        at<float>(task, 0x2A0) = static_cast<float>(static_cast<double>(c) + kBarrelBias_00d7a280);
        at<std::int32_t>(task, 0x2D4) = 0;
        if ((at<std::uint8_t>(task, 0x26C) & 1) != 0) host.bot_task_retire_0099b690(task);
    }
    if (host.condition_vcall_00_test(ptr_at(task, 0x3F8))) host.bot_task_retire_0099b690(task);
}

// 009BA2F0 (009BA2F0-009BA6E3), gainaltitude. u = task+2F4h, cls = task+2F8h, the
// target t = task+3F8h (may be 0), the altitude margin task+3FCh.
//  - 009BA300: +2E4h &= 0FBh (imm32: bits 8-31 clear too).
//  - With t (009BA31E-009BA41F): d = t - u in x and z; |d| (1e-10 guard); |d| > 1000.0
//    (qword 00CE47A0, FCOMIP JBE, NaN -> near) -> +2C4h = _CIatan2(dx, dz), +2D0h = 2;
//    else +2C8h = 0, +2D0h = 1.
//  - Without t (009BA424-009BA583): the mean of three [-1, 1]-clamped terms,
//    u+C68h / (pi/3) (qword 00D03DD0), one 00BD2F10 draw (ECX = 1) U(-0.1 [00CE3CB4],
//    0.1 [00D7A2F0]) at 009BA48C, and u+9ECh; the divisor 0.0 + 1.0 (qword) + 1 + 1 is
//    tested for zero by FUCOMIP/LAHF/TEST 44h/JNP (unordered divides). +294h = the
//    mean, +298h = 1, +2D0h = 0.
//  - 009BA589-009BA654: f = (u->vtable[38h]() - 007C4810(cls)) / (cls+18Ch -
//    007C4810(cls)) clamped to [0, 1]; +2D4h = 2, +2C0h = f x pi/4 (qword 00CEDCD0);
//    +27Ch = 1.0, +280h = 1; +2ACh = 0, +2B0h = 1; +2DCh = 0.
//  - 009BA655-009BA6D7: with t, u+100h > t+100h + task+3FCh (JA) -> 0099B690; without,
//    u+100h > task+3FCh (JBE skips) -> 0099B690.
// x87 at 24-bit precision as above (the speed fraction's two qword temporaries hold
// 24-bit results); 53-bit is the alternative. Only reference: vtable 00D20480 slot +64h
// (00D204E4, image scan); no target-equal twin.
void bot_task_gain_altitude_tick_009ba2f0(Cc13Seg67bHost& host, void* task, float) {
    at<std::uint32_t>(task, 0x2E4) &= 0xFBu;  // 81 A6 E4 02 00 00 FB 00 00 00 (imm32)
    if (ptr_at(task, 0x3F8) != nullptr) {
        void* const unit = ptr_at(task, 0x2F4);
        if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);
        const float px = at<float>(unit, 0xFC);
        const float pz = at<float>(unit, 0x104);
        void* const target = ptr_at(task, 0x3F8);
        if (at<std::uint8_t>(target, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(target);
        const float dx = at<float>(target, 0xFC) - px;
        const float dz = at<float>(target, 0x104) - pz;
        const float s = dz * dz + dx * dx;
        const float len = static_cast<double>(s) > kGainAltTinySq_00ce3820
                              ? static_cast<float>(std::sqrt(static_cast<double>(s)))
                              : 0.0f;
        if (static_cast<double>(len) > kGainAltFar_00ce47a0) {
            at<std::int32_t>(task, 0x2D0) = 2;
            at<float>(task, 0x2C4) = host.crt_atan2_00bf701a(dx, dz);
        } else {
            at<float>(task, 0x2C8) = 0.0f;
            at<std::int32_t>(task, 0x2D0) = 1;
        }
    } else {
        void* const unit = ptr_at(task, 0x2F4);
        const float r = clamp_unit_pm1(static_cast<float>(
            static_cast<double>(at<float>(unit, 0xC68)) / kThirdPi_00d03dd0));
        float sum = r + 0.0f;
        float count = static_cast<float>(0.0 + kOne_00d7a210);
        const float j = clamp_unit_pm1(host.random_uniform_00bd2f10(
            1, kGainAltJitterLo_00ce3cb4, kGainAltJitterHi_00d7a2f0));
        sum = j + sum;
        count = count + 1.0f;
        const float w = clamp_unit_pm1(at<float>(ptr_at(task, 0x2F4), 0x9EC));
        sum = w + sum;
        count = 1.0f + count;
        at<float>(task, 0x294) = (count == 0.0f) ? 0.0f : sum / count;
        at<std::uint8_t>(task, 0x298) = 1;
        at<std::int32_t>(task, 0x2D0) = 0;
    }

    void* const cls = ptr_at(task, 0x2F8);
    const float speed = host.entity_vcall_38_float(ptr_at(task, 0x2F4));
    const float num = speed - host.plane_class_min_control_speed_007c4810(cls);
    const float den = at<float>(cls, 0x18C) - host.plane_class_min_control_speed_007c4810(cls);
    const float f = clamp_unit_01(num / den);
    at<std::int32_t>(task, 0x2D4) = 2;
    at<float>(task, 0x2C0) = static_cast<float>(static_cast<double>(f) * kGainAltPitch_00cedcd0);
    at<float>(task, 0x27C) = 1.0f;
    at<std::uint8_t>(task, 0x280) = 1;
    at<float>(task, 0x2AC) = 0.0f;
    at<std::uint8_t>(task, 0x2B0) = 1;
    at<std::int32_t>(task, 0x2DC) = 0;

    void* const unit = ptr_at(task, 0x2F4);
    void* const target = ptr_at(task, 0x3F8);
    if (target != nullptr) {
        if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);
        if (at<std::uint8_t>(target, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(target);
        const float ceiling = at<float>(target, 0x100) + at<float>(task, 0x3FC);
        if (at<float>(unit, 0x100) > ceiling) host.bot_task_retire_0099b690(task);
    } else {
        if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);
        if (at<float>(unit, 0x100) > at<float>(task, 0x3FC)) host.bot_task_retire_0099b690(task);
    }
}

namespace {
constexpr std::uint32_t kTightTurnTaskVtable = 0x00D205E0u;
}  // namespace

// 009BB380 (009BB380-009BB498). EH frame 00CAF276. bot = [unit+DF4h]; t = operator
// new(40Ch) (009BB3AF). With t: c = operator new(2Ch) (009BB3CF); with c:
// 007B5C70(ECX = c, unit) (009BB40A), [c] = 00D1EA40, c+1Ch..+24h = the point (read
// before the call), c+28h = `limit` (009BB415-009BB43C); else c = 0. Then
// 009BAFC0(ECX = t, bot, c) (009BB452), [t] = 00D205E0, byte t+408h = `flag`,
// byte t+404h = 1 (009BB45B-009BB467). Without t, t = 0. Finally
// 00999F50(ECX = bot, t) (009BB47F), with or without a task.
// UNCERTAINTY: the EH frees of a throwing constructor are not modelled.
void pilot_bot_prepend_tight_turn_to_point_009bb380(Cc13Seg67bHost& host, void* unit,
                                                    const float* point, float limit,
                                                    std::uint8_t flag) {
    void* const bot = ptr_at(unit, 0xDF4);
    void* t = host.operator_new_00bf681b(0x40C);
    if (t != nullptr) {
        void* c = host.operator_new_00bf681b(0x2C);
        if (c != nullptr) {
            const float p[3] = {point[0], point[1], point[2]};
            host.bot_condition_construct_base_007b5c70(c, unit);
            at<std::uint32_t>(c, 0x00) = kBotConditionTurnToVtable;
            at<float>(c, 0x1C) = p[0];
            at<float>(c, 0x20) = p[1];
            at<float>(c, 0x24) = p[2];
            at<float>(c, 0x28) = limit;
        }
        host.bot_task_pull_up_construct_009bafc0(t, bot, c);
        at<std::uint32_t>(t, 0x000) = kTightTurnTaskVtable;
        at<std::uint8_t>(t, 0x408) = flag;
        at<std::uint8_t>(t, 0x404) = 1;
    }
    host.pilot_bot_prepend_active_task_00999f50(bot, t);
}

// ---------------------------------------------------------------------------
// Small plan, land-task and ship-AI bodies.

namespace {
constexpr std::uint32_t kMoveToCircleVtable = 0x00D20A9Cu;
constexpr double kTwoPi_00ce3828 = 6.2831854820251465;
constexpr double kMinusPi_00ce3d18 = -3.1415927410125732;
constexpr double kPi_00ce3d28 = 3.1415927410125732;
}  // namespace

// 009C25D0 (009C25D0-009C264F). The 009BC960 body with vtable 00D20A9C
// (009C2618): base, r = 00BD2F10(1, 0, 1) (009C261E), 009C262B FCOMIP r against
// [00CE3800] (0.5), JB -> 0 else 1, into byte +18h (009C263C).
void* bot_state_move_to_circle_construct_009c25d0(Cc13Seg67bHost& host, void* state,
                                                  void* approach) {
    construct_state_base(state, approach);
    at<std::uint32_t>(state, 0) = kMoveToCircleVtable;
    const float r = host.random_uniform_00bd2f10(1, 0.0f, 1.0f);
    at<std::uint8_t>(state, 0x18) = (r >= kHalf_00ce3800) ? 1 : 0;
    return state;
}

// 009AFA50 (009AFA50-009AFA8E). u = [task+3FCh]; when (u+72Ch)->vtable[38h]() is
// false (009AFA64): task+620h. Else 007B8AD0(u) (009AFA78) ? task+4C4h : task+500h.
void* bot_task_land_pick_cruise_state_009afa50(Cc13Seg67bHost& host, void* task) {
    void* const unit = ptr_at(task, 0x3FC);
    if (!host.object_vcall_38(ptr(unit, 0x72C))) return ptr(task, 0x620);
    return host.unit_is_squadron_flight_leader_007b8ad0(unit) ? ptr(task, 0x4C4)
                                                              : ptr(task, 0x500);
}

// 0099B260 (0099B260-0099B283). byte s+8h ? s+4h : s+0h, returned through a
// MOVSS/FLD stack copy.
float pilot_bot_override_or_value_0099b260(void* slot) {
    return at<std::uint8_t>(slot, 8) != 0 ? at<float>(slot, 4) : at<float>(slot, 0);
}

// 0099A9B0 (0099A9B0-0099A9DE). The plan observes entity +14h with itself as the
// observer: when it changes, 006952A0(ECX = old, EDX = plan) when old is set
// (0099A9C5), +14h = entity (0099A9CC), 00694A60(ECX = entity, EDX = plan) when
// set (0099A9D5).
void pilot_plan_set_observed_endpoint_0099a9b0(Cc13Seg67bHost& host, void* plan, void* entity) {
    void* const old = ptr_at(plan, 0x14);
    if (entity == old) return;
    if (old != nullptr) host.observer_unregister_pair_006952a0(old, plan);
    at<std::uintptr_t>(plan, 0x14) = reinterpret_cast<std::uintptr_t>(entity);
    if (entity != nullptr) host.observer_register_pair_00694a60(entity, plan);
}

// 0099A9E0 (0099A9E0-0099AA17). +18h = src+18h (FLD/FSTP, 0099A9E4/0099A9ED), then
// the 0099A9B0 swap to src+14h (0099A9F4-0099AA0E).
// UNCERTAINTY: the x87 copy quiets a signalling NaN; the C++ copy keeps its bits.
void pilot_plan_copy_endpoint_0099a9e0(Cc13Seg67bHost& host, void* plan, void* src) {
    at<float>(plan, 0x18) = at<float>(src, 0x18);
    pilot_plan_set_observed_endpoint_0099a9b0(host, plan, ptr_at(src, 0x14));
}

// 0099BA10 (0099BA10-0099BA8B). r = float(fmod(*in, double [00CE3828])) via
// 00BF857A (_CIfmod, ST1 mod ST0) (0099BA1C). 0099BA37 FCOMIP of double [00CE3D18]
// (-pi) against r, JB (-pi < r, or NaN) -> the second test; else *out =
// float(r + 2pi) (0099BA3B). Second: 0099BA5C FCOMI r against double [00CE3D28]
// (pi), JBE (r <= pi, or NaN) -> *out = r; else *out = float(r - 2pi) (0099BA62).
// Returns out.
// UNCERTAINTY: the sums are rounded once to 24 bits in the image (main thread);
// here a double sum is rounded to float, which can differ by double rounding.
float* math_normalize_angle_to_pi_by_ref_0099ba10(float* out, const float* in) {
    const float r = static_cast<float>(std::fmod(static_cast<double>(*in), kTwoPi_00ce3828));
    if (kMinusPi_00ce3d18 >= static_cast<double>(r)) {
        *out = static_cast<float>(static_cast<double>(r) + kTwoPi_00ce3828);
    } else if (static_cast<double>(r) > kPi_00ce3d28) {
        *out = static_cast<float>(static_cast<double>(r) - kTwoPi_00ce3828);
    } else {
        *out = r;
    }
    return out;
}

// 009DA880 (009DA880-009DA8CC). EH frame 00CB052B. 0064A610(ECX = node+4Ch)
// (009DA8A8); 009D5EB0(ECX = node+34h) (009DA8B8).
void ship_ai_path_node_destroy_009da880(Cc13Seg67bHost& host, void* node) {
    host.observed_owner_base_destroy_0064a610(ptr(node, 0x4C));
    ship_ai_path_node_observer_destruct_body_009d5eb0(host, ptr(node, 0x34));
}

// 009DF8F0 (009DF8F0-009DF909). n = operator new(0Ch) (009DF8F2); when n: [n] = n
// (009DF8FE); then, when n+4h is non-zero, [n+4h] = n (009DF907). The second test
// is on the address, so a null n writes 0 to address 4, as in the image.
void* ship_ai_traffic_list_head_node_009df8f0(Cc13Seg67bHost& host) {
    void* const n = host.operator_new_00bf681b(0x0C);
    if (n != nullptr) at<std::uintptr_t>(n, 0) = reinterpret_cast<std::uintptr_t>(n);
    const std::uintptr_t next = reinterpret_cast<std::uintptr_t>(n) + 4u;
    if (next != 0) {
        *reinterpret_cast<std::uintptr_t*>(next) = reinterpret_cast<std::uintptr_t>(n);
    }
    return n;
}

namespace {
constexpr float kMinusOne_00d7a260_f = -1.0f;
}  // namespace

// 00999AE0 (00999AE0-00999B2B). Walks the bot's task vector with a cursor taken
// once from begin +58h (00999AE7); after each entry the end = [+58h] + 4 x [+5Ch]
// is re-read and the loop continues while cursor != end (JNZ, 00999B07-00999B15).
// Null entries are skipped. The first task whose vtable[34h]() is false (00999B01)
// answers by a tail JMP to its vtable[4Ch]() (00999B2B). With none: [00D7A260]
// (-1.0) (00999B17).
// UNCERTAINTY: if a vtable[34h] call moved or shrank the vector, the stale cursor
// is followed exactly as the image does (it may step past the end).
float pilot_bot_first_task_wait_value_00999ae0(Cc13Seg67bHost& host, void* bot) {
    auto end_of = [bot] {
        return at<std::uintptr_t>(bot, 0x58) + 4u * at<std::uint32_t>(bot, 0x5C);
    };
    std::uintptr_t cursor = at<std::uintptr_t>(bot, 0x58);
    if (cursor == end_of()) return kMinusOne_00d7a260_f;
    do {
        void* const task = *reinterpret_cast<void**>(cursor);
        if (task != nullptr && !host.object_vcall_34(task)) {
            return host.object_vcall_4c_float(task);
        }
        cursor += 4u;
    } while (cursor != end_of());
    return kMinusOne_00d7a260_f;
}

// 0099BB00 (0099BB00-0099BB37). cur = unit+BC4h; 0099BB11 FCOMIP cur against v,
// JBE (cur <= v, or NaN) -> unit+BC4h = v; else unit+BC4h = cur (re-stored).
void plane_latch_yaw_rate_multiplier_0099bb00(void* unit, float v) {
    const float cur = at<float>(unit, 0xBC4);
    at<float>(unit, 0xBC4) = (cur > v) ? cur : v;
}

// 009DB470 (009DB470-009DB522). EH frame 00CB05FD. [b+D0h] = 00D05704 (009DB49F);
// [b+D4h]->vtable[8]() when set (009DB4B0); 009D60F0(b+60h), [b+58h] = 00D056D0
// (009DB4C2, 009DB4C7); 009D60F0(b+3Ch), [b+34h] = 00D056D0 (009DB4DC, 009DB4E3);
// 004116D0(b ? b+20h : 0) (009DB4F8); 009D60F0(b+8h), [b] = 00D056D0 (009DB508,
// 009DB511).
void ship_ai_brain_destruct_members_009db470(Cc13Seg67bHost& host, void* brain) {
    at<std::uint32_t>(brain, 0xD0) = kBotTaskSubVtable30C;
    void* const owned = ptr_at(brain, 0xD4);
    if (owned != nullptr) host.object_vcall_08_noargs(owned);
    ship_ai_observer_member_destruct_a_009db380(host, ptr(brain, 0x58));
    ship_ai_observer_member_destruct_a_009db380(host, ptr(brain, 0x34));
    host.state_registry_destruct_004116d0(brain != nullptr ? ptr(brain, 0x20) : nullptr);
    ship_ai_observer_member_destruct_a_009db380(host, brain);
}

// ---------------------------------------------------------------------------
// Ship-AI destructor bodies (009E5110-009E8CC1). EH state bookkeeping is not
// modelled. 009E48A0 is the std::list destructor instantiation (library, bound
// through the host).

namespace {
constexpr std::uint32_t kObserverPairVtable_00d056d0 = 0x00D056D0u;
constexpr std::uint32_t kShipAiBaseVtable_00d21990 = 0x00D21990u;
constexpr std::uint32_t kShipAiBrainVtable_00d21a40 = 0x00D21A40u;
constexpr std::uint32_t kShipAiBrainB5CVtable_00d21a3c = 0x00D21A3Cu;

void free_and_null(Cc13Seg67bHost& host, void* o, std::size_t off) {
    void* const p = ptr_at(o, off);
    if (p != nullptr) host.crt_free_00bf65ac(p);
    at<std::uint32_t>(o, off) = 0;
}
}  // namespace

// 009E5110 (009E5110-009E5122). 009E48A0(o+8h) (009E5116); [o] = 00D056D0.
void ship_ai_list_owner_destruct_009e5110(Cc13Seg67bHost& host, void* o) {
    host.std_list_destroy_009e48a0(ptr(o, 0x08));
    at<std::uint32_t>(o, 0) = kObserverPairVtable_00d056d0;
}

// 009E5130 (009E5130-009E51AB). EH frame 00CB080B. [o+1504h] = 00D05704
// (009E515E); [o+1508h]->vtable[8]() when set (009E516F); +14F4h, +14E0h, +14CCh,
// +14C0h = 00D056D0 (009E5176-009E518B); 009E48A0(o+10h) (009E5191); [o+8h] and
// [o] = 00D056D0 (009E519A, 009E519D).
void ship_ai_planner_destruct_009e5130(Cc13Seg67bHost& host, void* o) {
    at<std::uint32_t>(o, 0x1504) = kBotTaskSubVtable30C;
    void* const owned = ptr_at(o, 0x1508);
    if (owned != nullptr) host.object_vcall_08_noargs(owned);
    at<std::uint32_t>(o, 0x14F4) = kObserverPairVtable_00d056d0;
    at<std::uint32_t>(o, 0x14E0) = kObserverPairVtable_00d056d0;
    at<std::uint32_t>(o, 0x14CC) = kObserverPairVtable_00d056d0;
    at<std::uint32_t>(o, 0x14C0) = kObserverPairVtable_00d056d0;
    host.std_list_destroy_009e48a0(ptr(o, 0x10));
    at<std::uint32_t>(o, 0x08) = kObserverPairVtable_00d056d0;
    at<std::uint32_t>(o, 0x00) = kObserverPairVtable_00d056d0;
}

// 009E5460 (009E5460-009E5524). EH frame 00CB0887. [o] = 00D21990 (009E547E);
// free(+AC4h) and +AC4h = 0 when set (009E5499, 009E54A1); 0041CC80(o+4h)
// (009E54AA); 006E0860(o+AF8h) (009E54B5); free(+AE0h) when set, then +AE0h,
// +AE4h, +AE8h = 0 (009E54C5-009E54D9); free(+AD0h) when set, then +AD0h, +AD4h,
// +AD8h = 0 (009E54EA-009E5501); 009E0130(o+8h) (009E550F).
void ship_ai_base_destruct_009e5460(Cc13Seg67bHost& host, void* o) {
    at<std::uint32_t>(o, 0) = kShipAiBaseVtable_00d21990;
    void* const p = ptr_at(o, 0xAC4);
    if (p != nullptr) {
        host.crt_free_00bf65ac(p);
        at<std::uint32_t>(o, 0xAC4) = 0;
    }
    host.critical_section_destroy_owned_0041cc80(ptr(o, 0x04));
    host.observer_member_9348_destruct_006e0860(ptr(o, 0xAF8));
    free_and_null(host, o, 0xAE0);
    at<std::uint32_t>(o, 0xAE4) = 0;
    at<std::uint32_t>(o, 0xAE8) = 0;
    free_and_null(host, o, 0xAD0);
    at<std::uint32_t>(o, 0xAD4) = 0;
    at<std::uint32_t>(o, 0xAD8) = 0;
    host.member_array_destruct_009e0130(ptr(o, 0x08));
}

// 009E8C10 (009E8C10-009E8CC1). EH frame 00CB0A53. [b] = 00D21A40, [b+B5Ch] =
// 00D21A3C, [b+21FCh] = 00D056D0 (009E8C37-009E8C56); 009DB470(b+2124h)
// (009E8C5C); 009E5130(b+C18h) (009E8C6C); +C0Ch, +C04h, +BE0h, +B8Ch, +B80h,
// +B70h = 00D056D0 (009E8C71-009E8C91); 004116D0(b+B5Ch) (009E8C9C, no null
// adjustment); 009E5460(b) (009E8CAB).
void ship_ai_brain_destruct_009e8c10(Cc13Seg67bHost& host, void* brain) {
    at<std::uint32_t>(brain, 0x000) = kShipAiBrainVtable_00d21a40;
    at<std::uint32_t>(brain, 0xB5C) = kShipAiBrainB5CVtable_00d21a3c;
    at<std::uint32_t>(brain, 0x21FC) = kObserverPairVtable_00d056d0;
    ship_ai_brain_destruct_members_009db470(host, ptr(brain, 0x2124));
    ship_ai_planner_destruct_009e5130(host, ptr(brain, 0xC18));
    at<std::uint32_t>(brain, 0xC0C) = kObserverPairVtable_00d056d0;
    at<std::uint32_t>(brain, 0xC04) = kObserverPairVtable_00d056d0;
    at<std::uint32_t>(brain, 0xBE0) = kObserverPairVtable_00d056d0;
    at<std::uint32_t>(brain, 0xB8C) = kObserverPairVtable_00d056d0;
    at<std::uint32_t>(brain, 0xB80) = kObserverPairVtable_00d056d0;
    at<std::uint32_t>(brain, 0xB70) = kObserverPairVtable_00d056d0;
    host.state_registry_destruct_004116d0(ptr(brain, 0xB5C));
    ship_ai_base_destruct_009e5460(host, brain);
}

namespace {
constexpr double kTiny_00ce3820 = 1e-10;   // bytes BB BD D7 D9 DF 7C DB 3D
}  // namespace

// 009BEBA0 (009BEBA0-009BECC8). ctl = [a+0Ch]; byte ctl+3ADh == 0 -> a+2Ch = 0
// (009BEBAA-009BEBB7). t = [a+44h]: when set, 00414DB0(t) unless byte t+C8h
// (009BEBCE), then a+48h..+50h = t+FCh..+104h (009BEBD3-009BEBEB). a+4Ch =
// float(ctl+394h - a+2Ch) (009BEBF1-009BEBFD). u = [a+4h]: 00414DB0(u) unless byte
// u+C8h (009BEC0B); dx = float(a+48h - u+FCh), dz = float(a+50h - u+104h); d2 =
// float(dz*dz + dx*dx) (009BEC2D-009BEC3D); d = d2 > double [00CE3820] ?
// float(sqrt(d2)) (00BF7030) : 0 (JBE, so NaN -> 0); a+64h = d (009BEC80);
// 009FD050(ECX = a, d, a+48h) (009BEC85). When a+28h > [00D7A218] (0.0; COMISS,
// JBE -> skip): a+4Ch = 009FC260(ECX = 009BD400(), [a+4h], a+48h, a+28h)
// (009BECA9-009BECBF).
void bot_approach_refresh_target_009beba0(Cc13Seg67bHost& host, void* a) {
    void* const ctl = ptr_at(a, 0x0C);
    if (at<std::uint8_t>(ctl, 0x3AD) == 0) at<float>(a, 0x2C) = 0.0f;
    void* const t = ptr_at(a, 0x44);
    if (t != nullptr) {
        if (at<std::uint8_t>(t, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(t);
        at<float>(a, 0x48) = at<float>(t, 0xFC);
        at<float>(a, 0x4C) = at<float>(t, 0x100);
        at<float>(a, 0x50) = at<float>(t, 0x104);
    }
    at<float>(a, 0x4C) = at<float>(ctl, 0x394) - at<float>(a, 0x2C);
    void* const u = ptr_at(a, 0x04);
    if (at<std::uint8_t>(u, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(u);
    const float dx = at<float>(a, 0x48) - at<float>(u, 0xFC);
    const float dz = at<float>(a, 0x50) - at<float>(u, 0x104);
    const float d2 = dz * dz + dx * dx;
    const float d = static_cast<double>(d2) > kTiny_00ce3820
                        ? static_cast<float>(std::sqrt(static_cast<double>(d2)))
                        : 0.0f;
    at<float>(a, 0x64) = d;
    host.bot_approach_refresh_path_leg_009fd050(a, d, ptr(a, 0x48));
    const float w = at<float>(a, 0x28);
    if (w > 0.0f) {
        void* const unit = ptr_at(a, 0x04);
        void* const manager = host.bot_path_leg_manager_get_009bd400();
        at<float>(a, 0x4C) =
            host.bot_path_leg_manager_altitude_009fc260(manager, unit, ptr(a, 0x48), w);
    }
}

// 009BD400 (009BD400-009BD4BC). EH frame 00CAF623. [00F8A720] set -> return it.
// Else cs = [00415350()+10h]; when cs: EnterCriticalSection(cs) (import 00CE2218,
// 009BD43F) and cs+18h += 1. When [00F8A720] is still 0 (009BD451): m = operator
// new(10h) (009BD45C), m = m ? 009FAA40(ECX = m) : 0 (009BD473), [00F8A720] = m
// (009BD481), 00BD0C30(ECX = 00415350(), m) (009BD494). When cs: cs+18h -= 1 and
// LeaveCriticalSection(cs) (import 00CE2210, 009BD4A2). Returns [00F8A720].
// The guard local's vtable 00CE37FC (009BD430) has no other effect.
void* bot_path_leg_manager_get_body_009bd400(Cc13Seg67bHost& host) {
    if (host.path_leg_manager_00f8a720() != nullptr) return host.path_leg_manager_00f8a720();
    void* const cs = ptr_at(host.singleton_manager_00415350(), 0x10);
    if (cs != nullptr) {
        host.enter_critical_section_ce2218(cs);
        at<std::int32_t>(cs, 0x18) += 1;
    }
    if (host.path_leg_manager_00f8a720() == nullptr) {
        void* m = host.operator_new_00bf681b(0x10);
        m = m != nullptr ? host.bot_path_leg_manager_construct_009faa40(m) : nullptr;
        host.set_path_leg_manager_00f8a720(m);
        host.singleton_register_00bd0c30(host.singleton_manager_00415350(),
                                         host.path_leg_manager_00f8a720());
    }
    if (cs != nullptr) {
        at<std::int32_t>(cs, 0x18) -= 1;
        host.leave_critical_section_ce2210(cs);
    }
    return host.path_leg_manager_00f8a720();
}

// ---------------------------------------------------------------------------
// The depth-charge task's archive and factory bodies.

namespace {
constexpr std::uint32_t kStrTargetLock = 0x00D1F624u;   // "_targetLock"
constexpr std::uint32_t kStrStateMoveTo = 0x00D1F720u;  // "_state_moveTo"
constexpr std::uint32_t kDepthChargeApproachVtable = 0x00D1F620u;
constexpr std::uint32_t kDepthChargeTaskVtable = 0x00D1F738u;
constexpr std::uint32_t kDepthChargeTaskApproachVtable = 0x00D1F734u;
constexpr std::uint32_t kDepthChargeTaskRegistryVtable = 0x00D1F730u;
}  // namespace

// 009A35D0 (009A35D0-009A3668). EH frame 00CADE5B. f = (0042E740())+4B8h
// (009A35F2); 009F9CE0(ECX = a, arg, f) (009A3603); [a] = 00D1F620, a+2Ch = 0
// (009A361A, 009A3620); 00BD4FE0(ECX = archive, the outgoing block,
// {0,"_targetLock"}) (009A3639); 009FB3E0(ECX = a+7Ch, that block by value)
// (009A3641); 009A31A0(ECX = a) (009A3650). Returns a.
void* bot_approach_depth_charge_construct_from_stream_009a35d0(Cc13Seg67bHost& host, void* a,
                                                               void* arg, void* archive) {
    const float f = at<float>(host.game_tuning_singleton_0042e740(), 0x4B8);
    host.bot_approach_construct_speed_reference_009f9ce0(a, arg, f);
    at<std::uint32_t>(a, 0x00) = kDepthChargeApproachVtable;
    at<std::uint32_t>(a, 0x2C) = 0;
    std::uint32_t ref[3] = {};
    host.archive_reader_ref_open_00bd4fe0(archive, ref, name_record(kStrTargetLock));
    host.approach_ref_construct_from_stream_009fb3e0(ptr(a, 0x7C), ref);
    host.bot_approach_depth_charge_pick_aim_offset_009a31a0(a);
    return a;
}

// 009A5E60 (009A5E60-009A5ECF). 00BD5010(ECX = archive, the outgoing block,
// {0,"_targetLock"}) (009A5E8B), 009FACC0(ECX = a+7Ch, block by value) (009A5E93);
// 00BD5010(archive, block, {0,"_state_moveTo"}) (009A5EBA), 009BEAA0(ECX = a+110h,
// block by value) (009A5EC5). Each callee destroys its block.
void bot_approach_depth_charge_save_009a5e60(Cc13Seg67bHost& host, void* a, void* archive) {
    std::uint32_t ref[3] = {};
    host.archive_writer_ref_open_00bd5010(archive, ref, name_record(kStrTargetLock));
    host.approach_ref_save_009facc0(ptr(a, 0x7C), ref);
    std::uint32_t ref2[3] = {};
    host.archive_writer_ref_open_00bd5010(archive, ref2, name_record(kStrStateMoveTo));
    bot_approach_save_altitude_leg_009beaa0(host, ptr(a, 0x110), ref2);
}

// 009A5ED0 (009A5ED0-009A5EFF). 0099D060(ECX = task, archive) (009A5ED9);
// 009A5E60(ECX = task+3F8h, archive) (009A5EE5); 00411450(ECX = task+4F4h,
// task+30Ch, archive) (009A5EF8).
void bot_task_depth_charge_save_009a5ed0(Cc13Seg67bHost& host, void* task, void* archive) {
    bot_task_save_base_0099d060(host, task, archive);
    bot_approach_depth_charge_save_009a5e60(host, ptr(task, 0x3F8), archive);
    host.bot_state_registry_write_current_00411450(ptr(task, 0x4F4), ptr(task, 0x30C), archive);
}

// 009A67C0 (009A67C0-009A6810). EH frame 00CAE108. 009A4680(task ? task+3F8h : 0)
// (009A67ED); 007B4030(ECX = task) (009A67FC).
void bot_task_depth_charge_destruct_body_009a67c0(Cc13Seg67bHost& host, void* task) {
    bot_task_depth_charge_destruct_approach_block_009a4680(
        host, task != nullptr ? ptr(task, 0x3F8) : nullptr);
    host.bot_task_destruct_base_007b4030(task);
}

// 009A6820 (009A6820-009A68B0). EH frame 00CAE136. 0099C8B0(ECX = task, bot,
// archive, 1) (009A684C); 009A5000(ECX = task+3F8h, bot, archive) (009A6863);
// [task] = 00D1F738, [task+3F8h] = 00D1F734, [task+4F4h] = 00D1F730
// (009A687B-009A6887); 00411170(ECX = task+4F4h, task+30Ch, archive) (009A688D);
// 009F9980(ECX = task+3F8h, task) (009A6895). Returns task.
void* bot_task_depth_charge_construct_from_stream_009a6820(Cc13Seg67bHost& host, void* task,
                                                           void* bot, void* archive) {
    bot_task_construct_base_from_stream_0099c8b0(host, task, bot, archive, 1);
    bot_task_depth_charge_construct_approach_block_from_stream_009a5000(host, ptr(task, 0x3F8),
                                                                        bot, archive);
    at<std::uint32_t>(task, 0x000) = kDepthChargeTaskVtable;
    at<std::uint32_t>(task, 0x3F8) = kDepthChargeTaskApproachVtable;
    at<std::uint32_t>(task, 0x4F4) = kDepthChargeTaskRegistryVtable;
    host.bot_state_registry_read_current_00411170(ptr(task, 0x4F4), ptr(task, 0x30C), archive);
    host.bot_approach_bind_to_task_009f9980(ptr(task, 0x3F8), task);
    return task;
}

// 009A6970 (009A6970-009A69D5). t = operator new(7BCh) (009A6991); null -> 0;
// else 009A5240(ECX = t, bot, target) (009A69AD).
void* bot_task_make_depth_charge_009a6970(Cc13Seg67bHost& host, void* bot, void* target) {
    void* const t = host.operator_new_00bf681b(0x7BC);
    if (t == nullptr) return nullptr;
    return host.depth_charge_task_construct_009a5240(t, bot, target);
}

// 009A69E0 (009A69E0-009A6A46). t = operator new(7BCh) (009A69FB); null -> 0;
// else 009A6820(ECX = t, bot, archive) (009A6A1E).
void* bot_task_load_depth_charge_009a69e0(Cc13Seg67bHost& host, void* bot, void* archive) {
    void* const t = host.operator_new_00bf681b(0x7BC);
    if (t == nullptr) return nullptr;
    return bot_task_depth_charge_construct_from_stream_009a6820(host, t, bot, archive);
}

// 009A19B0 (009A19B0-009A19D7). plan+2F0h null -> 0 (FLDZ, 009A19BB); else
// 0099D0A0(ECX = plan, [plan+2F0h]+C68h) (009A19D1).
float pilot_command_heading_diff_from_live_plane_009a19b0(Cc13Seg67bHost& host, void* plan) {
    void* const plane = ptr_at(plan, 0x2F0);
    if (plane == nullptr) return 0.0f;
    return host.pilot_heading_diff_0099d0a0(plan, at<float>(plane, 0xC68));
}

// 0099B670 (0099B670-0099B688). n = [[p+2F4h]+C50h]; when n, 007DF4F0(ECX = n, p)
// (0099B683).
void pilot_bot_avoid_vehicles_0099b670(Cc13Seg67bHost& host, void* p) {
    void* const n = ptr_at(ptr_at(p, 0x2F4), 0xC50);
    if (n != nullptr) host.vehicle_avoidance_007df4f0(n, p);
}

// ---------------------------------------------------------------------------
// Dogfight, drop-kamikaze and kamikaze task destructor bodies, stream
// constructors and factories.

namespace {
constexpr std::uint32_t kDropKamikazeTaskVtable = 0x00D1FC70u;
constexpr std::uint32_t kDropKamikazeApproachVtable = 0x00D1FC68u;
constexpr std::uint32_t kDropKamikazeRegistryVtable = 0x00D1FC64u;
}  // namespace

// 009AB370 (009AB370-009AB3C0). 009A93E0(task ? task+3F8h : 0) (009AB39D);
// 007B4030(ECX = task) (009AB3AC).
void bot_task_dogfight_destruct_body_009ab370(Cc13Seg67bHost& host, void* task) {
    bot_task_dogfight_destruct_approach_block_009a93e0(
        host, task != nullptr ? ptr(task, 0x3F8) : nullptr);
    host.bot_task_destruct_base_007b4030(task);
}

// 009AEB80 (009AEB80-009AEBD0). 009ADAE0(task ? task+3F8h : 0) (009AEBAD);
// 007B4030(task) (009AEBBC).
void bot_task_drop_kamikaze_destruct_body_009aeb80(Cc13Seg67bHost& host, void* task) {
    bot_task_drop_kamikaze_destruct_approach_block_009adae0(
        host, task != nullptr ? ptr(task, 0x3F8) : nullptr);
    host.bot_task_destruct_base_007b4030(task);
}

// 009AF610 (009AF610-009AF660). 009ACE50(task ? task+3F8h : 0) (009AF63D);
// 007B4030(task) (009AF64C).
void bot_task_kamikaze_destruct_body_009af610(Cc13Seg67bHost& host, void* task) {
    bot_task_kamikaze_destruct_approach_block_009ace50(
        host, task != nullptr ? ptr(task, 0x3F8) : nullptr);
    host.bot_task_destruct_base_007b4030(task);
}

// 009AB3D0 (009AB3D0-009AB463). EH frame. 0099C8B0(ECX = task, bot, archive, 2)
// (009AB3FC); 009A9670(ECX = task+3F8h, bot, archive) (009AB413); [task] =
// 00D1F9B0, [task+3F8h] = 00D1F9AC, [task+4FCh] = 00D1F9A8 (009AB426-009AB432);
// 009F9980(ECX = task+3F8h, task) (009AB439); 00411170(ECX = task+4FCh, task+30Ch,
// archive) (009AB448). Returns task.
void* bot_task_dogfight_construct_from_stream_009ab3d0(Cc13Seg67bHost& host, void* task,
                                                       void* bot, void* archive) {
    bot_task_construct_base_from_stream_0099c8b0(host, task, bot, archive, 2);
    bot_task_dogfight_construct_approach_block_from_stream_009a9670(host, ptr(task, 0x3F8), bot,
                                                                    archive);
    at<std::uint32_t>(task, 0x000) = kDogfightTaskVtable;
    at<std::uint32_t>(task, 0x3F8) = kDogfightApproachVtable;
    at<std::uint32_t>(task, 0x4FC) = kDogfightTask4FCVtable;
    host.bot_approach_bind_to_task_009f9980(ptr(task, 0x3F8), task);
    host.bot_state_registry_read_current_00411170(ptr(task, 0x4FC), ptr(task, 0x30C), archive);
    return task;
}

// 009AEAE0 (009AEAE0-009AEB73). EH frame. The 009AB3D0 shape with kind 5, 009AD920
// (009AEB23), vtables 00D1FC70 / +3F8h 00D1FC68 / +4BCh 00D1FC64
// (009AEB36-009AEB42), 009F9980 (009AEB49) and 00411170(task+4BCh, task+30Ch,
// archive) (009AEB58).
void* bot_task_drop_kamikaze_construct_from_bag_009aeae0(Cc13Seg67bHost& host, void* task,
                                                         void* bot, void* archive) {
    bot_task_construct_base_from_stream_0099c8b0(host, task, bot, archive, 5);
    bot_task_drop_kamikaze_construct_approach_block_from_stream_009ad920(host, ptr(task, 0x3F8),
                                                                         bot, archive);
    at<std::uint32_t>(task, 0x000) = kDropKamikazeTaskVtable;
    at<std::uint32_t>(task, 0x3F8) = kDropKamikazeApproachVtable;
    at<std::uint32_t>(task, 0x4BC) = kDropKamikazeRegistryVtable;
    host.bot_approach_bind_to_task_009f9980(ptr(task, 0x3F8), task);
    host.bot_state_registry_read_current_00411170(ptr(task, 0x4BC), ptr(task, 0x30C), archive);
    return task;
}

// 009AB630 (009AB630-009AB696). __stdcall(bot, archive): t = operator new(758h) (009AB64B); null -> 0;
// else 009AB3D0(t, bot, archive) (009AB66E).
void* bot_task_dogfight_factory_create_009ab630(Cc13Seg67bHost& host, void* bot, void* archive) {
    void* const t = host.operator_new_00bf681b(0x758);
    if (t == nullptr) return nullptr;
    return bot_task_dogfight_construct_from_stream_009ab3d0(host, t, bot, archive);
}

// 009AEC50 (009AEC50-009AECB6). __stdcall(bot, archive): t = operator new(74Ch) (009AEC6B); null -> 0;
// else 009AEAE0(t, bot, archive) (009AEC8E).
void* bot_task_make_drop_kamikaze_from_bag_009aec50(Cc13Seg67bHost& host, void* bot,
                                                    void* archive) {
    void* const t = host.operator_new_00bf681b(0x74C);
    if (t == nullptr) return nullptr;
    return bot_task_drop_kamikaze_construct_from_bag_009aeae0(host, t, bot, archive);
}

// 009AEBE0 (009AEBE0-009AEC45). __fastcall(ECX = bot, EDX = target): t = operator
// new(74Ch) (009AEC01); null -> 0; else 009ADBF0(ECX = t, bot, target) (009AEC1D).
void* bot_task_make_drop_kamikaze_009aebe0(Cc13Seg67bHost& host, void* bot, void* target) {
    void* const t = host.operator_new_00bf681b(0x74C);
    if (t == nullptr) return nullptr;
    return host.drop_kamikaze_task_construct_009adbf0(t, bot, target);
}

// 009AF720 (009AF720-009AF785). __fastcall(ECX = bot, EDX = target): t = operator
// new(6A0h) (009AF741); null -> 0; else 009AEEF0(ECX = t, bot, target) (009AF75D).
void* bot_task_make_kamikaze_009af720(Cc13Seg67bHost& host, void* bot, void* target) {
    void* const t = host.operator_new_00bf681b(0x6A0);
    if (t == nullptr) return nullptr;
    return host.kamikaze_task_construct_009aeef0(t, bot, target);
}

// 009A98F0 (009A98F0-009A9904). 009A98F6 FCOMIP of +18h against +1Ch: JBE
// (+18h <= +1Ch, or NaN) -> 0, else 1.
bool bot_state_dogfight_aim_is_bored_009a98f0(void* state) {
    return at<float>(state, 0x18) > at<float>(state, 0x1C);
}

// 009B1E30 (009B1E30-009B1E4E). +30h null -> 0; else ([+30h])->vtable[5Ch](9) ? 1 : 0
// (009B1E40).
bool land_target_is_kind9_009b1e30(Cc13Seg67bHost& host, void* o) {
    void* const t = ptr_at(o, 0x30);
    if (t == nullptr) return false;
    return host.object_vcall_5c(t, 9);
}

// ---------------------------------------------------------------------------
// Land, level-bomb, move-on-path and conditioned-task destructor bodies, stream
// constructors and factories.

namespace {
constexpr std::uint32_t kLandTaskVtable = 0x00D1FFA0u;
constexpr std::uint32_t kLandApproachVtable = 0x00D1FF94u;
constexpr std::uint32_t kLandRegistryVtable = 0x00D1FF90u;
constexpr std::uint32_t kLevelBombTaskVtable = 0x00D20210u;
constexpr std::uint32_t kLevelBombApproachVtable = 0x00D2020Cu;
constexpr std::uint32_t kLevelBombRegistryVtable = 0x00D20208u;
constexpr std::uint32_t kMoveOnPathTaskVtable = 0x00D20910u;
constexpr std::uint32_t kMoveOnPathApproachVtable = 0x00D20904u;
constexpr std::uint32_t kMoveOnPathRegistryVtable = 0x00D20900u;
constexpr std::uint32_t kMoveOnPathApproachBaseVtable = 0x00D20838u;
constexpr std::uint32_t kStopTaskVtable = 0x00D20500u;
constexpr std::uint32_t kImmelmannTaskVtable = 0x00D20400u;
constexpr std::uint32_t kGainAltitudeTaskVtable = 0x00D20480u;
constexpr std::uint32_t kStrStage = 0x00D20474u;            // "stage"
constexpr std::uint32_t kStrAlt = 0x00D204F8u;              // "alt"
constexpr std::uint32_t kStrAltOffset = 0x00D20894u;        // "altOffset"
constexpr std::uint32_t kStrTravelAltOffset = 0x00D20884u;  // "travelAltOffset"
}  // namespace

// 009B3F50 (009B3F50-009B3FE3). EH frame. [t] = 00D1FFA0, [t+3F8h] = 00D1FF94,
// [t+4B0h] = 00D1FF90 (009B3F76-009B3F82); u = [t+3FCh]: when u and 007B8AD0(u)
// (009B3F9E), 006C8800(ECX = [u+9D4h]) (009B3FB3); 009B2C80(ECX = t+3F8h)
// (009B3FBF); 007B4030(ECX = t) (009B3FCE).
void bot_task_land_destruct_body_009b3f50(Cc13Seg67bHost& host, void* task) {
    at<std::uint32_t>(task, 0x000) = kLandTaskVtable;
    at<std::uint32_t>(task, 0x3F8) = kLandApproachVtable;
    at<std::uint32_t>(task, 0x4B0) = kLandRegistryVtable;
    void* const unit = ptr_at(task, 0x3FC);
    if (unit != nullptr && host.unit_is_squadron_flight_leader_007b8ad0(unit)) {
        host.air_ops_retire_landing_squadron_006c8800(ptr_at(unit, 0x9D4));
    }
    host.landing_approach_destroy_009b2c80(ptr(task, 0x3F8));
    host.bot_task_destruct_base_007b4030(task);
}

// 009B4120 (009B4120-009B41B1). EH frame. 0099C8B0(ECX = task, bot, archive, 3)
// (009B414B); 009B3040(ECX = task+3F8h, [bot+50h], archive) (009B4165); [task] =
// 00D1FFA0, [task+3F8h] = 00D1FF94, [task+4B0h] = 00D1FF90 (009B417D-009B4189);
// 00411170(ECX = task+4B0h, task+30Ch, archive) (009B418F); 009F9980(ECX =
// task+3F8h, task) (009B4197). Returns task.
void* bot_task_land_construct_from_stream_009b4120(Cc13Seg67bHost& host, void* task, void* bot,
                                                   void* archive) {
    bot_task_construct_base_from_stream_0099c8b0(host, task, bot, archive, 3);
    bot_task_land_construct_approach_block_from_stream_009b3040(host, ptr(task, 0x3F8),
                                                                ptr_at(bot, 0x50), archive);
    at<std::uint32_t>(task, 0x000) = kLandTaskVtable;
    at<std::uint32_t>(task, 0x3F8) = kLandApproachVtable;
    at<std::uint32_t>(task, 0x4B0) = kLandRegistryVtable;
    host.bot_state_registry_read_current_00411170(ptr(task, 0x4B0), ptr(task, 0x30C), archive);
    host.bot_approach_bind_to_task_009f9980(ptr(task, 0x3F8), task);
    return task;
}

// 009B4230 (009B4230-009B4296). __stdcall(bot, archive): t = operator new(670h)
// (009B4246); null -> 0; else 009B4120(t, bot, archive) (009B426E).
void* bot_task_land_factory_create_009b4230(Cc13Seg67bHost& host, void* bot, void* archive) {
    void* const t = host.operator_new_00bf681b(0x670);
    if (t == nullptr) return nullptr;
    return bot_task_land_construct_from_stream_009b4120(host, t, bot, archive);
}

// 009B8E80 (009B8E80-009B8ED0). 009B6480(task ? task+3F8h : 0) (009B8EAD);
// 007B4030(task) (009B8EBC).
void bot_task_level_bomb_destruct_body_009b8e80(Cc13Seg67bHost& host, void* task) {
    bot_task_level_bomb_destruct_approach_block_009b6480(
        host, task != nullptr ? ptr(task, 0x3F8) : nullptr);
    host.bot_task_destruct_base_007b4030(task);
}

// 009B8EE0 (009B8EE0-009B8F73). EH frame. 0099C8B0(task, bot, archive, 4)
// (009B8F0C); 009B77C0(ECX = task+3F8h, bot, archive) (009B8F23); vtables
// 00D20210 / +3F8h 00D2020C / +4D4h 00D20208 (009B8F36-009B8F42); 009F9980
// (009B8F49); 00411170(task+4D4h, task+30Ch, archive) (009B8F58).
void* bot_task_level_bomb_construct_from_stream_009b8ee0(Cc13Seg67bHost& host, void* task,
                                                         void* bot, void* archive) {
    bot_task_construct_base_from_stream_0099c8b0(host, task, bot, archive, 4);
    bot_task_level_bomb_construct_approach_block_from_stream_009b77c0(host, ptr(task, 0x3F8),
                                                                      bot, archive);
    at<std::uint32_t>(task, 0x000) = kLevelBombTaskVtable;
    at<std::uint32_t>(task, 0x3F8) = kLevelBombApproachVtable;
    at<std::uint32_t>(task, 0x4D4) = kLevelBombRegistryVtable;
    host.bot_approach_bind_to_task_009f9980(ptr(task, 0x3F8), task);
    host.bot_state_registry_read_current_00411170(ptr(task, 0x4D4), ptr(task, 0x30C), archive);
    return task;
}

// 009B90A0 (009B90A0-009B9106). __stdcall(bot, archive): new(6F8h) (009B90B6);
// null -> 0; else 009B8EE0(t, bot, archive) (009B90DE).
void* bot_task_level_bomb_factory_create_009b90a0(Cc13Seg67bHost& host, void* bot,
                                                  void* archive) {
    void* const t = host.operator_new_00bf681b(0x6F8);
    if (t == nullptr) return nullptr;
    return bot_task_level_bomb_construct_from_stream_009b8ee0(host, t, bot, archive);
}

// 009B9030 (009B9030-009B9095). __fastcall(ECX = bot, EDX = target): new(6F8h)
// (009B9048); null -> 0; else 009B7990(ECX = t, bot, target) (009B906D).
void* bot_task_make_level_bomb_009b9030(Cc13Seg67bHost& host, void* bot, void* target) {
    void* const t = host.operator_new_00bf681b(0x6F8);
    if (t == nullptr) return nullptr;
    return bot_task_level_bomb_construct_009b7990(host, t, bot, target);
}

// 009BD9D0 (009BD9D0-009BDA20). 009BCB70(task ? task+3F8h : 0) (009BD9FD);
// 007B4030(task) (009BDA0C).
void bot_task_move_on_path_destroy_009bd9d0(Cc13Seg67bHost& host, void* task) {
    bot_approach_move_on_path_destroy_009bcb70(host, task != nullptr ? ptr(task, 0x3F8)
                                                                     : nullptr);
    host.bot_task_destruct_base_007b4030(task);
}

// 009BDA30 (009BDA30-009BDAF2). EH frame. 0099C8B0(task, bot, archive, 6)
// (009BDA5C); 009BCD50(ECX = task+3F8h, bot, archive) (009BDA73); [task] =
// 00D20910, [task+3F8h] = 00D20904, [task+438h] = 00D20900, +518h = [00D7A24C] 1.0
// (009BDA96-009BDAB0); 009BDAB8 00BD2F10(1, 0, 1), +51Ch = -r (009BDAC0);
// 009F9980(ECX = task+3F8h, task) (009BDAC8); 00411170(task+438h, task+30Ch,
// archive) (009BDAD7).
void* bot_task_move_on_path_construct_from_stream_009bda30(Cc13Seg67bHost& host, void* task,
                                                           void* bot, void* archive) {
    bot_task_construct_base_from_stream_0099c8b0(host, task, bot, archive, 6);
    bot_task_move_on_path_construct_approach_block_from_stream_009bcd50(host, ptr(task, 0x3F8), bot,
                                                                        archive);
    at<std::uint32_t>(task, 0x000) = kMoveOnPathTaskVtable;
    at<std::uint32_t>(task, 0x3F8) = kMoveOnPathApproachVtable;
    at<std::uint32_t>(task, 0x438) = kMoveOnPathRegistryVtable;
    at<float>(task, 0x518) = kOne_00d7a24c;
    at<float>(task, 0x51C) = -host.random_uniform_00bd2f10(1, 0.0f, 1.0f);
    host.bot_approach_bind_to_task_009f9980(ptr(task, 0x3F8), task);
    host.bot_state_registry_read_current_00411170(ptr(task, 0x438), ptr(task, 0x30C), archive);
    return task;
}

// 009BDBB0 (009BDBB0-009BDC0F). __fastcall(ECX = bot): new(520h) (009BDBC7); null
// -> 0; else 009BCE50(ECX = t, bot) (009BDBE9).
void* bot_task_move_on_path_create_009bdbb0(Cc13Seg67bHost& host, void* bot) {
    void* const t = host.operator_new_00bf681b(0x520);
    if (t == nullptr) return nullptr;
    return host.move_on_path_task_construct_009bce50(t, bot);
}

// 009BDC10 (009BDC10-009BDC76). __stdcall(bot, archive): new(520h) (009BDC2B);
// null -> 0; else 009BDA30(t, bot, archive) (009BDC4E).
void* bot_task_move_on_path_create_from_stream_009bdc10(Cc13Seg67bHost& host, void* bot,
                                                        void* archive) {
    void* const t = host.operator_new_00bf681b(0x520);
    if (t == nullptr) return nullptr;
    return bot_task_move_on_path_construct_from_stream_009bda30(host, t, bot, archive);
}

// 009BADB0 (009BADB0-009BAE09). __fastcall(ECX = bot): t = new(3F8h) (009BADCF);
// null -> 0; else 0099C6F0(ECX = t, bot, 0Fh) (009BADEC), [t] = 00D20500 (009BADF1).
void* bot_task_make_stop_009badb0(Cc13Seg67bHost& host, void* bot) {
    void* const t = host.operator_new_00bf681b(0x3F8);
    if (t == nullptr) return nullptr;
    bot_task_construct_base_0099c6f0(host, t, bot, 0x0F);
    at<std::uint32_t>(t, 0) = kStopTaskVtable;
    return t;
}

// 009BACB0 (009BACB0-009BAD23). EH frame. __fastcall(ECX = bot, EDX = target; float
// alt), RET 4: t = new(400h) (009BACD2); null -> 0; else 0099C6F0(ECX = t, bot, 0Fh)
// (009BACEF), [t] = 00D20480 (009BACFA), +3F8h = target (009BAD00), +3FCh = alt
// (009BAD06). The base constructor takes two 00BD2F10 draws: stream 1 at 0099C765
// (U(0, 1), negated, into +308h) and stream 0 at 0099C7BA (U(-1, 0) into +3E8h).
void* bot_task_make_gain_altitude_009bacb0(Cc13Seg67bHost& host, void* bot, void* target,
                                           float alt) {
    void* const t = host.operator_new_00bf681b(0x400);
    if (t == nullptr) return nullptr;
    bot_task_construct_base_0099c6f0(host, t, bot, 0x0F);
    at<std::uint32_t>(t, 0) = kGainAltitudeTaskVtable;
    at<void*>(t, 0x3F8) = target;
    at<float>(t, 0x3FC) = alt;
    return t;
}

// 009BAD30 (009BAD30-009BADA8). EH frame. __fastcall(ECX = bot), RET: t = new(3FCh)
// (009BAD4F); null -> 0; else 0099C6F0(ECX = t, bot, 0Fh) (009BAD6E), [t] = 00D20400
// (009BAD74), +3F8h = 0 (009BAD7A). The base constructor takes the same two draws.
void* bot_task_make_immelmann_009bad30(Cc13Seg67bHost& host, void* bot) {
    void* const t = host.operator_new_00bf681b(0x3FC);
    if (t == nullptr) return nullptr;
    bot_task_construct_base_0099c6f0(host, t, bot, 0x0F);
    at<std::uint32_t>(t, 0) = kImmelmannTaskVtable;
    at<std::uint32_t>(t, 0x3F8) = 0;
    return t;
}

// 009BA960 (009BA960-009BA9DD). EH frame. 0099C8B0(task, bot, archive, 0Fh)
// (009BA98B); [task] = 00D20400 (009BA995); archive->vtable[10h]({0,"stage"}, {1,
// &slot}) (009BA9BC), where slot is the `archive` argument's own stack slot; +3F8h
// = slot (009BA9C6). An archive that leaves the dword unwritten stores the archive
// pointer itself, as in the image.
void* bot_task_immelmann_construct_from_stream_009ba960(Cc13Seg67bHost& host, void* task,
                                                        void* bot, void* archive) {
    bot_task_construct_base_from_stream_0099c8b0(host, task, bot, archive, 0x0F);
    at<std::uint32_t>(task, 0) = kImmelmannTaskVtable;
    std::uint32_t slot = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(archive));
    host.archive_vcall_10_read(archive, name_record(kStrStage),
                               record(1, reinterpret_cast<std::uintptr_t>(&slot)));
    at<std::uint32_t>(task, 0x3F8) = slot;
    return task;
}

// 009BAAB0 (009BAAB0-009BAB6C). EH frame. 0099C8B0(task, bot, archive, 0Fh)
// (009BAADB); [task] = 00D20480 (009BAAE9); archive->vtable[10h]({0,"target"}, {4,
// &slot}) (009BAB14), slot being the `archive` argument's stack slot, which
// 009BAAE5 first overwrites with an ESP value; +3F8h = slot (009BAB1A);
// archive->vtable[10h]({0,"alt"}, {2, task+3FCh}) (009BAB55).
// UNCERTAINTY: an unwritten "target" leaves a stack address in +3F8h in the image;
// here the slot starts at 0.
void* bot_task_gain_altitude_construct_from_stream_009baab0(Cc13Seg67bHost& host, void* task,
                                                            void* bot, void* archive) {
    bot_task_construct_base_from_stream_0099c8b0(host, task, bot, archive, 0x0F);
    at<std::uint32_t>(task, 0) = kGainAltitudeTaskVtable;
    std::uintptr_t slot = 0;
    host.archive_vcall_10_read(archive, name_record(kStrTarget),
                               record(4, reinterpret_cast<std::uintptr_t>(&slot)));
    at<std::uintptr_t>(task, 0x3F8) = slot;
    host.archive_vcall_10_read(archive, name_record(kStrAlt),
                               record(2, reinterpret_cast<std::uintptr_t>(ptr(task, 0x3FC))));
    return task;
}

// 009BD300 (009BD300-009BD36C). archive->vtable[0Ch]({0,"altOffset"}, {2, a+2Ch})
// (009BD336); ({0,"travelAltOffset"}, {2, a+30h}) (009BD366).
void bot_approach_move_on_path_save_offsets_009bd300(Cc13Seg67bHost& host, void* a,
                                                     void* archive) {
    host.archive_vcall_0c_write(archive, name_record(kStrAltOffset),
                                record(2, at<std::uint32_t>(a, 0x2C)));
    host.archive_vcall_0c_write(archive, name_record(kStrTravelAltOffset),
                                record(2, at<std::uint32_t>(a, 0x30)));
}

// 009BC7E0 (009BC7E0-009BC867). f = (0042E740())+37Ch (009BC7E9); 009F9CE0(ECX =
// a, arg, f) (009BC7FA); [a] = 00D20838 (009BC803); archive->vtable[10h]
// ({0,"altOffset"}, {2, a+2Ch}) (009BC833) and ({0,"travelAltOffset"}, {2, a+30h})
// (009BC85F). Returns a.
void* bot_approach_move_on_path_construct_from_stream_009bc7e0(Cc13Seg67bHost& host, void* a,
                                                               void* arg, void* archive) {
    const float f = at<float>(host.game_tuning_singleton_0042e740(), 0x37C);
    host.bot_approach_construct_speed_reference_009f9ce0(a, arg, f);
    at<std::uint32_t>(a, 0) = kMoveOnPathApproachBaseVtable;
    read_field(host, archive, kStrAltOffset, 2, ptr(a, 0x2C));
    read_field(host, archive, kStrTravelAltOffset, 2, ptr(a, 0x30));
    return a;
}

// ---------------------------------------------------------------------------
// Level-bomb state predicates.

namespace {
constexpr float kMinusHalf_00ce69d0 = -0.5f;
}  // namespace

// 009B7A80 (009B7A80-009B7AA2). byte +A0h set -> 0; else +98h > [00CE69D0] (-0.5)
// (COMISS, JBE -> 0, so NaN -> 0) -> 1.
bool bot_state_level_bomb_prepare_is_waiting_009b7a80(void* state) {
    if (at<std::uint8_t>(state, 0xA0) != 0) return false;
    return at<float>(state, 0x98) > kMinusHalf_00ce69d0;
}

// 009B7B10 (009B7B10-009B7B33). byte +18h set -> 1; byte +19h clear -> 0; else
// byte ([+4h]+0Ch)+3AEh set -> 1, clear -> 0.
bool bot_state_level_bomb_aim_should_release_009b7b10(void* state) {
    if (at<std::uint8_t>(state, 0x18) != 0) return true;
    if (at<std::uint8_t>(state, 0x19) == 0) return false;
    return at<std::uint8_t>(ptr_at(ptr_at(state, 0x04), 0x0C), 0x3AE) != 0;
}

// 009B7B40 (009B7B40-009B7B5D). byte +19h set and byte ([+4h]+0Ch)+3AEh clear -> 1;
// else 0.
bool bot_state_level_bomb_aim_should_abort_009b7b40(void* state) {
    if (at<std::uint8_t>(state, 0x19) == 0) return false;
    return at<std::uint8_t>(ptr_at(ptr_at(state, 0x04), 0x0C), 0x3AE) == 0;
}

namespace {
constexpr std::uint32_t kLevelBombMoveToVtable = 0x00D201E0u;
constexpr std::uint32_t kLevelBombMoveTo18Vtable = 0x00D201C8u;
constexpr std::uint32_t kArchiveReaderRefVtable_00ce3768 = 0x00CE3768u;

// The timer tail both level-bomb move-to constructors share (009B6C58-009B6CAB,
// 009B74DB-009B752B).
void level_bomb_move_to_tail(Cc13Seg67bHost& host, void* s) {
    at<std::uint32_t>(s, 0x00) = kLevelBombMoveToVtable;
    at<std::uint32_t>(s, 0x18) = kLevelBombMoveTo18Vtable;
    at<float>(s, 0x44) = kOne_00d7a24c;
    at<float>(s, 0x48) = -host.random_uniform_00bd2f10(1, 0.0f, 1.0f);
    at<float>(s, 0x3C) = 0.0f;
    at<float>(s, 0x40) = kOne_00d7a24c;
}
}  // namespace

// 009B6C10 (009B6C10-009B6CBF). EH frame. 009C2AC0(ECX = s, approach, target,
// near, far, third) (009B6C53; the three trailing arguments are re-pushed through
// FLD/FSTP); [s] = 00D201E0, [s+18h] = 00D201C8, +44h = 1.0, 009B6C8D 00BD2F10(1, 0,
// 1), +48h = -r, +3Ch = 0, +40h = 1.0. Returns s.
// UNCERTAINTY: the FLD/FSTP re-push quiets a signalling-NaN bit pattern; the
// C++ passes the bits unchanged.
void* bot_state_level_bomb_move_to_construct_009b6c10(Cc13Seg67bHost& host, void* s,
                                                      void* approach, void* target, float near_,
                                                      float far_, float third_) {
    host.bot_state_move_to_construct_009c2ac0(s, approach, target, near_, far_, third_);
    level_bomb_move_to_tail(host, s);
    return s;
}

// 009B7480 (009B7480-009B7550). EH frame. Copies the reader handle into the
// outgoing block ({00CE3768, count*, reader*}, the base vtable 00CE374C being
// overwritten at 009B74AA) and increments [count*] (009B74C0); 009C2B80(ECX = s,
// approach, the copy by value) (009B74D6), which destroys the copy; the 009B6C10
// tail (009B74EC-009B752B); 00410630(&handle) (009B7538). Returns s.
void* bot_state_level_bomb_run_construct_from_stream_009b7480(Cc13Seg67bHost& host, void* s,
                                                              void* approach, void* handle) {
    std::uint32_t copy[3] = {kArchiveReaderRefVtable_00ce3768, at<std::uint32_t>(handle, 4),
                             at<std::uint32_t>(handle, 8)};
    *reinterpret_cast<std::int32_t*>(static_cast<std::uintptr_t>(copy[1])) += 1;
    bot_state_construct_base_009c2b80(host, s, approach, copy);
    level_bomb_move_to_tail(host, s);
    host.native_string_destruct_00410630(handle);
    return s;
}

// ---------------------------------------------------------------------------
// Dive-bomb and move-to task/state bodies, the leader-velocity copy and the
// move-to target.

namespace {
constexpr std::uint32_t kDogfightMoveToVtable = 0x00D20B24u;
constexpr std::uint32_t kDogfightMoveTo18Vtable = 0x00D20B0Cu;
constexpr std::uint32_t kMoveToTaskVtable = 0x00D20B68u;
constexpr std::uint32_t kMoveToTaskApproachVtable = 0x00D20B58u;
constexpr std::uint32_t kMoveToTaskRegistryVtable = 0x00D20B54u;
constexpr std::uint32_t kDiveBombAttackRunVtable = 0x00D20C68u;
constexpr std::uint32_t kMoveToTargetVtable = 0x00D209C4u;
}  // namespace

// 009C8B60 (009C8B60-009C8BB0). 009C70A0(task ? task+3F8h : 0) (009C8B8D);
// 007B4030(task) (009C8B9C).
void bot_task_dive_bomb_destruct_body_009c8b60(Cc13Seg67bHost& host, void* task) {
    bot_task_dive_bomb_destruct_approach_block_009c70a0(
        host, task != nullptr ? ptr(task, 0x3F8) : nullptr);
    host.bot_task_destruct_base_007b4030(task);
}

// 009C8C70 (009C8C70-009C8CD5). __fastcall(ECX = bot, EDX = target): new(7E0h)
// (009C8C91); null -> 0; else 009C7710(ECX = t, bot, target) (009C8CAD).
void* bot_task_make_dive_bomb_009c8c70(Cc13Seg67bHost& host, void* bot, void* target) {
    void* const t = host.operator_new_00bf681b(0x7E0);
    if (t == nullptr) return nullptr;
    return bot_task_dive_bomb_construct_009c7710(host, t, bot, target);
}

// 009C2CA0 (009C2CA0-009C2CE0). 009C2AC0(ECX = s, approach, target, near, far,
// third) (009C2CC9; FLD/FSTP re-push); [s] = 00D20B24, [s+18h] = 00D20B0C. Returns s.
// UNCERTAINTY: the re-push quiets a signalling-NaN bit pattern.
void* bot_state_dogfight_move_to_construct_009c2ca0(Cc13Seg67bHost& host, void* s,
                                                    void* approach, void* target, float near_,
                                                    float far_, float third_) {
    host.bot_state_move_to_construct_009c2ac0(s, approach, target, near_, far_, third_);
    at<std::uint32_t>(s, 0x00) = kDogfightMoveToVtable;
    at<std::uint32_t>(s, 0x18) = kDogfightMoveTo18Vtable;
    return s;
}

// 009C2D00 (009C2D00-009C2D86). EH frame. The 009B7480 handle copy
// (009C2D1E-009C2D3A), 009C2B80(ECX = s, approach, copy) (009C2D50), [s] =
// 00D20B24, [s+18h] = 00D20B0C, 00410630(&handle) (009C2D6E). Returns s.
void* bot_state_move_to_construct_from_stream_009c2d00(Cc13Seg67bHost& host, void* s,
                                                       void* approach, void* handle) {
    std::uint32_t copy[3] = {kArchiveReaderRefVtable_00ce3768, at<std::uint32_t>(handle, 4),
                             at<std::uint32_t>(handle, 8)};
    *reinterpret_cast<std::int32_t*>(static_cast<std::uintptr_t>(copy[1])) += 1;
    bot_state_construct_base_009c2b80(host, s, approach, copy);
    at<std::uint32_t>(s, 0x00) = kDogfightMoveToVtable;
    at<std::uint32_t>(s, 0x18) = kDogfightMoveTo18Vtable;
    host.native_string_destruct_00410630(handle);
    return s;
}

// 009C3990 (009C3990-009C3A52). EH frame. 0099C8B0(task, bot, archive, 7)
// (009C39BC); 009C2EF0(ECX = task+3F8h, bot, archive) (009C39D3); [task] =
// 00D20B68, [task+3F8h] = 00D20B58, [task+468h] = 00D20B54, +548h = 1.0
// (009C39F6-009C3A10); 009C3A18 00BD2F10(1, 0, 1), +54Ch = -r (009C3A20);
// 00411170(task+468h, task+30Ch, archive) (009C3A2F); 009F9980(task+3F8h, task)
// (009C3A37).
void* bot_task_move_to_construct_from_stream_009c3990(Cc13Seg67bHost& host, void* task,
                                                      void* bot, void* archive) {
    bot_task_construct_base_from_stream_0099c8b0(host, task, bot, archive, 7);
    host.move_to_construct_approach_block_from_stream_009c2ef0(ptr(task, 0x3F8), bot, archive);
    at<std::uint32_t>(task, 0x000) = kMoveToTaskVtable;
    at<std::uint32_t>(task, 0x3F8) = kMoveToTaskApproachVtable;
    at<std::uint32_t>(task, 0x468) = kMoveToTaskRegistryVtable;
    at<float>(task, 0x548) = kOne_00d7a24c;
    at<float>(task, 0x54C) = -host.random_uniform_00bd2f10(1, 0.0f, 1.0f);
    host.bot_state_registry_read_current_00411170(ptr(task, 0x468), ptr(task, 0x30C), archive);
    host.bot_approach_bind_to_task_009f9980(ptr(task, 0x3F8), task);
    return task;
}

// 009C3CA0 (009C3CA0-009C3D06). __stdcall(bot, archive): new(550h) (009C3CBB); null
// -> 0; else 009C3990(t, bot, archive) (009C3CDE).
void* bot_task_move_to_factory_create_009c3ca0(Cc13Seg67bHost& host, void* bot, void* archive) {
    void* const t = host.operator_new_00bf681b(0x550);
    if (t == nullptr) return nullptr;
    return bot_task_move_to_construct_from_stream_009c3990(host, t, bot, archive);
}

// 009C4140 (009C4140-009C41BF). The 009A36A0 body with vtable 00D20C68 (009C4183);
// 009C419A RNG; +18h / +1Ch / +20h.
void* bot_state_dive_bomb_attack_run_construct_009c4140(Cc13Seg67bHost& host, void* state,
                                                        void* approach) {
    return construct_timed_state(host, state, approach, kDiveBombAttackRunVtable, 0x18, 0x1C,
                                 0x20);
}

// 009C7850 (009C7850-009C7864). 007B9320(ECX = [[s+4h]+4h], 0) (009C7858); AL
// clear -> 1, set -> 0 (NEG/SBB/ADD).
bool bot_state_dive_bomb_aim_glide_out_of_bombs_009c7850(Cc13Seg67bHost& host, void* state) {
    return !host.weapon_controller_has_general_bomb_ordnance_007b9320(
        ptr_at(ptr_at(state, 0x04), 0x04), 0);
}

// 009BE050 (009BE050-009BE099). c = unit+AB0h; c+18h..+20h = v[0..2] (FLD/FSTP);
// c+24h..+2Ch = 00F87574[0..2] (MOVSS); 007D9C10(ECX = c) (009BE092).
void plane_unit_copy_leader_velocity_009be050(Cc13Seg67bHost& host, void* unit,
                                              const float* v) {
    void* const c = ptr(unit, 0xAB0);
    at<float>(c, 0x18) = v[0];
    at<float>(c, 0x1C) = v[1];
    at<float>(c, 0x20) = v[2];
    const float* const g = host.global_vector_00f87574();
    at<float>(c, 0x24) = g[0];
    at<float>(c, 0x28) = g[1];
    at<float>(c, 0x2C) = g[2];
    host.plane_flight_controller_refresh_body_frame_007d9c10(c);
}

// 009BE0F0 (009BE0F0-009BE136). [t] = 00D209C4, t+10h = entity (009BE0FA,
// 009BE100); when entity: 00414DB0(entity) unless byte entity+C8h (009BE110), then
// t+4h..+0Ch = entity+FCh..+104h (009BE115-009BE12D). Returns t.
void* move_to_target_construct_from_entity_009be0f0(Cc13Seg67bHost& host, void* t,
                                                    void* entity) {
    at<std::uint32_t>(t, 0x00) = kMoveToTargetVtable;
    at<std::uintptr_t>(t, 0x10) = reinterpret_cast<std::uintptr_t>(entity);
    if (entity != nullptr) {
        if (at<std::uint8_t>(entity, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(entity);
        at<float>(t, 0x04) = at<float>(entity, 0xFC);
        at<float>(t, 0x08) = at<float>(entity, 0x100);
        at<float>(t, 0x0C) = at<float>(entity, 0x104);
    }
    return t;
}

// ---------------------------------------------------------------------------
// Level-bomb and dive-bomb task constructors.

namespace {
constexpr std::uint32_t kDiveBombTaskVtable = 0x00D20E18u;
constexpr std::uint32_t kDiveBombApproachVtable = 0x00D20E10u;
constexpr std::uint32_t kDiveBombRegistryVtable = 0x00D20E0Cu;

// The shared tail: [task], [task+3F8h] and [task+reg] vtables, the initial state
// (leader ? leader_state : member_state) into +310h with its enter, then 009F9980.
void construct_task_tail(Cc13Seg67bHost& host, void* task, std::uint32_t vt, std::uint32_t vt_ap,
                         std::size_t reg, std::uint32_t vt_reg, std::size_t leader_state,
                         std::size_t member_state) {
    at<std::uint32_t>(task, 0x000) = vt;
    at<std::uint32_t>(task, 0x3F8) = vt_ap;
    at<std::uint32_t>(task, reg) = vt_reg;
    void* const state = host.unit_is_squadron_flight_leader_007b8ad0(ptr_at(task, 0x3FC))
                            ? ptr(task, leader_state)
                            : ptr(task, member_state);
    at<std::uintptr_t>(task, 0x310) = reinterpret_cast<std::uintptr_t>(state);
    host.state_vcall_04_enter(state);
    host.bot_approach_bind_to_task_009f9980(ptr(task, 0x3F8), task);
}
}  // namespace

// 009B7990 (009B7990-009B7A36). EH frame. 0099C6F0(ECX = task, bot, 4) (009B79B6);
// 009B75E0(ECX = task+3F8h, bot, target) (009B79D1); [task] = 00D20210, [task+3F8h]
// = 00D2020C, [task+4D4h] = 00D20208 (009B79E1-009B79ED); 007B8AD0([task+3FCh])
// (009B79F7) ? +4E8h : +534h -> +310h, its vtable[4]() (009B7A17); 009F9980(ECX =
// task+3F8h, task) (009B7A1C).
void* bot_task_level_bomb_construct_009b7990(Cc13Seg67bHost& host, void* task, void* bot,
                                             void* target) {
    bot_task_construct_base_0099c6f0(host, task, bot, 4);
    bot_approach_level_bomb_construct_009b75e0(host, ptr(task, 0x3F8), bot, target);
    construct_task_tail(host, task, kLevelBombTaskVtable, kLevelBombApproachVtable, 0x4D4,
                        kLevelBombRegistryVtable, 0x4E8, 0x534);
    return task;
}

// 009C7710 (009C7710-009C77B6). EH frame. 0099C6F0(task, bot, 8) (009C7736);
// 009C73A0(ECX = task+3F8h, bot, target) (009C7751); vtables 00D20E18 / +3F8h
// 00D20E10 / +4DCh 00D20E0C (009C7761-009C776D); 007B8AD0 ? +4F0h : +52Ch -> +310h,
// enter (009C7797); 009F9980 (009C779C).
void* bot_task_dive_bomb_construct_009c7710(Cc13Seg67bHost& host, void* task, void* bot,
                                            void* target) {
    bot_task_construct_base_0099c6f0(host, task, bot, 8);
    bot_task_dive_bomb_construct_approach_block_009c73a0(host, ptr(task, 0x3F8), bot, target);
    construct_task_tail(host, task, kDiveBombTaskVtable, kDiveBombApproachVtable, 0x4DC,
                        kDiveBombRegistryVtable, 0x4F0, 0x52C);
    return task;
}

// ---------------------------------------------------------------------------
// Land minimum speed and park-edge rule, the takeoff factory, and the ship-AI
// plan's signed-node walks.

namespace {
constexpr float kZero_s15 = 0.0f;
}  // namespace

// 009CFF40 (009CFF40-009CFFA6). EH frame 00CB01BB. t = operator new(4D4h)
// (009CFF61); null -> 0 (009CFF93); else 009CF8E0(ECX = t, owner, flag)
// (009CFF7D), whose EAX is returned.
void* pilot_bot_create_takeoff_task_009cff40(Cc13Seg67bHost& host, void* owner,
                                             std::uint32_t flag) {
    void* const t = host.operator_new_00bf681b(0x4D4);
    if (t == nullptr) return nullptr;
    return bot_task_takeoff_construct_009cf8e0(host, t, owner, flag);
}

namespace {
// The shared walk of 009D5AE0 and 009D5B90: from plan+20h, a leaf (both links
// null) ends it with 0; a node whose signed byte +14h passes `hit` ends it with
// 1; an undecided node gets the 009D5930 direction rule, stored into +4h; then
// the walk follows +24h on +1, +20h on -1, else ends with 0.
// 009D5B06-009D5B56 (and 009D5BB6-009D5C06), the walks' inline direction rule: a decided
// node keeps +4h; no minus link -> (plus link ? +1 : 0); no plus link -> -1; otherwise
// plus = (+50h + +38h) + plus->+3Ch and minus = (+4Ch + +34h) + minus->+3Ch, each FADD
// rounded to 24 bits on the main thread, the sum stored float, and FCOMIP plus,minus / JC gives +1 when plus < minus or
// the pair is unordered, else -1.
void ship_ai_path_walk_inline_direction(ShipAiPathNode& n) {
    if (n.direction != 0) return;
    if (n.link_minus == nullptr) {
        n.direction = n.link_plus != nullptr ? 1 : 0;
    } else if (n.link_plus == nullptr) {
        n.direction = -1;
    } else {
        using cc13x87::x87_add24;
        const float minus = static_cast<float>(
            x87_add24(x87_add24(n.cost_minus, n.link_minus_length), n.link_minus->remaining_length));
        const float plus = static_cast<float>(
            x87_add24(x87_add24(n.cost_plus, n.link_plus_length), n.link_plus->remaining_length));
        n.direction = !(plus >= minus) ? 1 : -1;
    }
}

template <bool Image, typename Hit>
bool ship_ai_path_walk_for_signed_node(ShipAiPathPlanBlock& plan, Hit hit) {
    ShipAiPathNode* n = plan.head;
    while (n != nullptr) {
        if (n->link_minus == nullptr && n->link_plus == nullptr) return false;
        if (hit(static_cast<std::int8_t>(n->field_14))) return true;
        if constexpr (Image) {
            ship_ai_path_walk_inline_direction(*n);
        } else {
            ship_ai_path_node_decide_direction_009d5930(*n);
        }
        n = ship_ai_path_node_next_009d9ee6(*n);
    }
    return false;
}
}  // namespace

// 009D5AE0 (009D5AE0-009D5B87). The walk above with `hit` = signed byte +14h > 0
// (009D5B00 CMP / JG). The direction rule is inline at 009D5B06-009D5B56.
// 009D5B4F JC takes the +1 arm when the costs are unordered (NaN), as the image's
// 009D9E6C does; the C++ shared rule (ship_ai_path_node_direction_009d9e6c, whose `<`
// deviates from the image) returns -1 there. The inline rule is behind
// kShipAiPathSideWalkNanCc13Bound (cc13-fixture8 pinned the OFF difference).
bool ship_ai_path_any_positive_side_ahead_009d5ae0(ShipAiPathPlanBlock& plan) {
    return ship_ai_path_walk_for_signed_node<kShipAiPathSideWalkNanCc13Bound>(
        plan, [](std::int8_t b) { return b > 0; });
}

bool ship_ai_path_any_positive_side_ahead_image_009d5ae0(ShipAiPathPlanBlock& plan) {
    return ship_ai_path_walk_for_signed_node<true>(plan, [](std::int8_t b) { return b > 0; });
}

// 009D5B90 (009D5B90-009D5C37). The same walk with `hit` = signed byte +14h < 0
// (009D5BB0 CMP / JL). Inline rule 009D5BB6-009D5C06.
// As 009D5AE0, the JC at 009D5BFF takes +1 on unordered costs (the C++ shared rule
// gives -1); behind the same switch.
bool ship_ai_path_any_negative_side_ahead_009d5b90(ShipAiPathPlanBlock& plan) {
    return ship_ai_path_walk_for_signed_node<kShipAiPathSideWalkNanCc13Bound>(
        plan, [](std::int8_t b) { return b < 0; });
}

bool ship_ai_path_any_negative_side_ahead_image_009d5b90(ShipAiPathPlanBlock& plan) {
    return ship_ai_path_walk_for_signed_node<true>(plan, [](std::int8_t b) { return b < 0; });
}

// ---------------------------------------------------------------------------
// Task destructor bodies, the gun-bot destructor body and the move-to approach
// block from a stream.

// 009CA1A0 (009CA1A0-009CA1F0), 009CD1F0 (009CD1F0-009CD240), 009CFEE0
// (009CFEE0-009CFF30) and 009D4DB0 (009D4DB0-009D4E00): EH frame; the approach
// block's destructor on (task ? task+3F8h : 0) (009CA1CD 009C9A80, 009CD21D
// 009CBE90, 009CFF0D 009CF850, 009D4DDD 009D2390), then 007B4030(task)
// (009CA1DC, 009CD22C, 009CFF1C, 009D4DEC).
void bot_task_retreat_destruct_body_009ca1a0(Cc13Seg67bHost& host, void* task) {
    bot_task_retreat_destruct_approach_block_009c9a80(
        host, task != nullptr ? ptr(task, 0x3F8) : nullptr);
    host.bot_task_destruct_base_007b4030(task);
}

void bot_task_strafe_destruct_body_009cd1f0(Cc13Seg67bHost& host, void* task) {
    bot_task_strafe_destruct_approach_block_009cbe90(
        host, task != nullptr ? ptr(task, 0x3F8) : nullptr);
    host.bot_task_destruct_base_007b4030(task);
}

void bot_task_takeoff_destruct_body_009cfee0(Cc13Seg67bHost& host, void* task) {
    bot_task_takeoff_destruct_approach_block_009cf850(
        host, task != nullptr ? ptr(task, 0x3F8) : nullptr);
    host.bot_task_destruct_base_007b4030(task);
}

void bot_task_torpedo_destruct_body_009d4db0(Cc13Seg67bHost& host, void* task) {
    bot_task_torpedo_destruct_approach_block_009d2390(
        host, task != nullptr ? ptr(task, 0x3F8) : nullptr);
    host.bot_task_destruct_base_007b4030(task);
}

// 009F3E80 (009F3E80-009F3EF5). EH frame 00CB0CCF. [b+2260h] = 00D05704
// (009F3EAF); [b+2264h]->vtable[8]() when set (009F3EC0); 009E8C10(b ? b+58h : 0)
// (009F3ED2); 007293D0(b) (009F3EE1).
void ship_ai_gun_bot_destruct_body_009f3e80(Cc13Seg67bHost& host, void* bot) {
    void* const owned = ptr_at(bot, 0x2264);
    at<std::uint32_t>(bot, 0x2260) = kBotTaskSubVtable30C;
    if (owned != nullptr) host.object_vcall_08_noargs(owned);
    ship_ai_brain_destruct_009e8c10(host, bot != nullptr ? ptr(bot, 0x58) : nullptr);
    host.tick_subnode_owner_destruct_base_007293d0(bot);
}

namespace {
constexpr std::uint32_t kMoveToBlockVtable_00d20b48 = 0x00D20B48u;
constexpr std::uint32_t kMoveToRegistryVtable_00d20b44 = 0x00D20B44u;
constexpr std::uint32_t kMoveToStateVtable_00d20a80 = 0x00D20A80u;
constexpr std::uint32_t kStrMoveToMoveTo_00d20998 = 0x00D20998u;  // "moveto (moveto)"
constexpr std::uint32_t kStrMoveToFollow_00d20988 = 0x00D20988u;  // "follow (moveto)"
constexpr std::uint32_t kStrMoveToCircle_00d20978 = 0x00D20978u;  // "circle (moveto)"
}  // namespace

// 009C2EF0 (009C2EF0-009C2FD2). EH frame 00CAF93D. 009C1DC0(ECX = block,
// [bot+50h], archive) (009C2F1F); 00411E20(block+70h) (009C2F2F); [block] =
// 00D20B48, [block+70h] = 00D20B44 (009C2F3A, 009C2F40); the inline state at
// +84h: +88h = block, +90h/+94h/+98h/+8Ch = 0, [+84h] = 00D20A80 (009C2F46-
// 009C2F55); 009C2980(ECX = block+9Ch, block, [00CE3D08] 100.0) (009C2F73);
// 009C25D0(ECX = block+134h, block) (009C2F84); 00411E70(block+70h, name, state)
// for "moveto (moveto)" +84h, "follow (moveto)" +9Ch and "circle (moveto)" +134h
// (009C2F96, 009C2FA3, 009C2FB6). Returns block.
void* bot_task_move_to_construct_approach_block_from_stream_009c2ef0(Cc13Seg67bHost& host,
                                                                     void* block, void* bot,
                                                                     void* archive) {
    bot_approach_move_to_construct_from_stream_009c1dc0(host, block, ptr_at(bot, 0x50), archive);
    host.bot_state_registry_construct_00411e20(ptr(block, 0x70));
    at<std::uint32_t>(block, 0x00) = kMoveToBlockVtable_00d20b48;
    at<std::uint32_t>(block, 0x70) = kMoveToRegistryVtable_00d20b44;
    at<std::uintptr_t>(block, 0x88) = reinterpret_cast<std::uintptr_t>(block);
    at<std::uint32_t>(block, 0x90) = 0;
    at<std::uint32_t>(block, 0x94) = 0;
    at<std::uint32_t>(block, 0x98) = 0;
    at<std::uint32_t>(block, 0x8C) = 0;
    at<std::uint32_t>(block, 0x84) = kMoveToStateVtable_00d20a80;
    bot_state_follow_construct_009c2980(host, ptr(block, 0x9C), block, kHundred_00ce3d08);
    bot_state_move_to_circle_construct_009c25d0(host, ptr(block, 0x134), block);
    host.state_registry_add_00411e70(ptr(block, 0x70), kStrMoveToMoveTo_00d20998, ptr(block, 0x84));
    host.state_registry_add_00411e70(ptr(block, 0x70), kStrMoveToFollow_00d20988, ptr(block, 0x9C));
    host.state_registry_add_00411e70(ptr(block, 0x70), kStrMoveToCircle_00d20978,
                                     ptr(block, 0x134));
    return block;
}

// ---------------------------------------------------------------------------
// Ship-AI path node walks, the navigation goal with a penalty, and two debug
// colour records.

// 009D5A80 (009D5A80-009D5ADB). Recursive. With both +28h and +2Ch null: true
// (009D5A8A-009D5A8F). Else for c = +28h (009D5A83): when c is set and byte
// c+31h is set (009D5A97), true when float c+34h > [00D7A24C] 1.0 (009D5AA2
// COMISS JA; NaN falls through) or when 009D5A80(c) is true (009D5AAB). Then
// the same for c = +2Ch with byte c+32h and float c+38h (009D5AB4-009D5AD6).
// Else false (009D5AD8).
bool ship_ai_path_node_subtree_complete_009d5a80(void* node) {
    void* const a = ptr_at(node, 0x28);
    if (a == nullptr && ptr_at(node, 0x2C) == nullptr) return true;
    if (a != nullptr && at<std::uint8_t>(a, 0x31) != 0) {
        if (at<float>(a, 0x34) > kOne_00d7a24c) return true;
        if (ship_ai_path_node_subtree_complete_009d5a80(a)) return true;
    }
    void* const b = ptr_at(node, 0x2C);
    if (b != nullptr && at<std::uint8_t>(b, 0x32) != 0) {
        if (at<float>(b, 0x38) > kOne_00d7a24c) return true;
        if (ship_ai_path_node_subtree_complete_009d5a80(b)) return true;
    }
    return false;
}

// 009D9440 (009D9440-009D954B). Recursive. 009D5930(node) (009D9446), then on
// +4h: +1 (009D944E): when +38h > 1.0 (009D945C COMISS JBE; NaN recurses), the
// heading of (plus - node) in XZ, inlined as 00414EB0's formula (009D9465-
// 009D94C1); else 009D9440(plus) (009D94DC). -1 (009D94EE): when +34h > 1.0
// (009D94F8), 00414EB0 on (minus - node) (009D951F); else 009D9440(minus)
// (009D9533). Any other direction: 0.0 (009D9545 FLDZ).
// UNCERTAINTY: inherited from 009D5930 through ship_ai_path_node_direction_009d9e6c:
// the image's JC takes the +1 arm on unordered (NaN) costs (009D5974, 009D5B4F,
// 009D5BFF); the shared rule returns -1 there.
float ship_ai_path_node_run_heading_009d9440(ShipAiPathNode& node) {
    ship_ai_path_node_decide_direction_009d5930(node);
    if (node.direction == 1) {
        if (node.link_plus_length > kOne_00d7a24c) {
            const std::array<float, 2> d{{node.link_plus->x - node.x,
                                          node.link_plus->z - node.z}};
            return heading_angle_00414eb0(d);
        }
        return ship_ai_path_node_run_heading_009d9440(*node.link_plus);
    }
    if (node.direction == -1) {
        if (node.link_minus_length > kOne_00d7a24c) {
            const std::array<float, 2> d{{node.link_minus->x - node.x,
                                          node.link_minus->z - node.z}};
            return heading_angle_00414eb0(d);
        }
        return ship_ai_path_node_run_heading_009d9440(*node.link_minus);
    }
    return kZero_s15;
}

// 009DE280 (009DE280-009DE2EF). 009DE050(ECX = nav, goal, 1, 0) (009DE28C); when
// +1C4h != 3 (009DE291): +368h = +360h = 0.0 (009DE29D, 009DE2A5); +1D8h =
// heading (009DE2B9) and 00605070(&+1D8h) wraps it (009DE2BD); +1CCh = 0
// (009DE2C8), +1C4h = 3 (009DE2D2), +250h = +2B8h = penalty (009DE2DC, 009DE2E4).
void ship_ai_set_navigation_goal_with_penalty_009de280(Cc13Seg67bHost& host, void* nav,
                                                       float heading, const float* goal,
                                                       float penalty) {
    host.ship_ai_set_navigation_goal_009de050(nav, goal, 1, 0);
    if (at<std::int32_t>(nav, 0x1C4) != 3) {
        at<float>(nav, 0x368) = kZero_s15;
        at<float>(nav, 0x360) = kZero_s15;
    }
    at<float>(nav, 0x1D8) = heading;
    host.math_wrap_angle_in_place_00605070(&at<float>(nav, 0x1D8));
    at<std::uint32_t>(nav, 0x1CC) = 0;
    at<std::int32_t>(nav, 0x1C4) = 3;
    at<float>(nav, 0x250) = penalty;
    at<float>(nav, 0x2B8) = penalty;
}

// 009D5E10 (009D5E10-009D5E58) and 009D5FD0 (009D5FD0-009D6013): function-local
// static colours. When bit 0 of the guard dword is clear (009D5E15 / 009D5FD5),
// it is set (OR) and the record filled: A at 00F8A68C = {0.5, 0.5, 0.5, 1.0}
// (00CE3800, 00D7A24C), guard 00F8A69C; B at 00F8A6DC = {0.0, 1.0, 1.0, 1.0},
// guard 00F8A6EC. Returns the record's address.
float* ship_ai_debug_colour_record_a_009d5e10(Cc13Seg67bHost& host) {
    std::uint8_t* const block = host.ship_ai_debug_static_block_00f8a68c();
    if ((at<std::uint8_t>(block, 0x10) & 1u) == 0) {
        at<std::uint32_t>(block, 0x10) |= 1u;
        at<float>(block, 0x00) = kHalf_00ce3800;
        at<float>(block, 0x04) = kHalf_00ce3800;
        at<float>(block, 0x08) = kHalf_00ce3800;
        at<float>(block, 0x0C) = kOne_00d7a24c;
    }
    return &at<float>(block, 0x00);
}

float* ship_ai_debug_colour_record_b_009d5fd0(Cc13Seg67bHost& host) {
    std::uint8_t* const block = host.ship_ai_debug_static_block_00f8a68c();
    if ((at<std::uint8_t>(block, 0x60) & 1u) == 0) {
        at<std::uint32_t>(block, 0x60) |= 1u;
        at<float>(block, 0x50) = kZero_s15;
        at<float>(block, 0x54) = kOne_00d7a24c;
        at<float>(block, 0x58) = kOne_00d7a24c;
        at<float>(block, 0x5C) = kOne_00d7a24c;
    }
    return &at<float>(block, 0x50);
}

// ---------------------------------------------------------------------------
// Ship-AI approach accessors on [nested] = brain.

// 009E5E30 (009E5E30-009E5E4E). t = [[nested]+B20h] (009E5E32); true when t is
// set and t->vtable[5Ch](6) is true (009E5E43); else false.
bool ship_ai_approach_target_is_kind_6_009e5e30(Cc13Seg67bHost& host, void* nested) {
    void* const t = ptr_at(ptr_at(nested, 0), 0xB20);
    return t != nullptr && host.object_vcall_5c(t, 6);
}

namespace {
constexpr std::uint32_t kMoveToApproachVtable_00d209cc = 0x00D209CCu;
constexpr std::uint32_t kObservedEndpointVtable_00ce3cf0 = 0x00CE3CF0u;
constexpr float kFifty_00ceb4d4 = 50.0f;                      // 42480000
constexpr std::uint32_t kStrClosingComplete_00d20a54 = 0x00D20A54u;  // "closingComplete"
constexpr std::uint32_t kStrUpdateTrgPosTimer_00d20a40 = 0x00D20A40u;  // "_updateTrgPosTimer"
constexpr std::uint32_t kStrClosingTime_00d20a34 = 0x00D20A34u;  // "closingTime"
constexpr std::uint32_t kStrTargetPos_00d20a28 = 0x00D20A28u;    // "targetpos"
constexpr std::uint32_t kStrTargetEntity_00d04f30 = 0x00D04F30u;  // "targetentity"
}  // namespace

// 009C1DC0 (009C1DC0-009C1FC2). EH frame 00CAF73B. 009F9CE0(ECX = a, arg,
// tuning+37Ch) (009C1DF6, tuning = 0042E740()); [a] = 00D209CC (009C1DFE). The
// observed endpoint at +30h: +34h/+38h/+3Ch/+44h = 0, byte +40h = 1, [+30h] =
// 00CE3CF0 (009C1E06-009C1E1A). The timer pair: +54h = 1.0 (009C1E3D), +58h =
// -00BD2F10(1, 0, 1) (009C1E42-009C1E51); +68h = [00CEB4D4] 50.0 (009C1E57).
// Reads (archive->vtable[10h]): "altOffset" {2, &+2Ch} (009C1E8B),
// "closingComplete" {3, &+5Ch} (009C1EBF); 00BD4FE0(archive, block,
// "_updateTrgPosTimer") (009C1EE3) and 00410D80(ECX = +54h, block by value)
// (009C1EEA); "closingTime" {2, &+60h} (009C1F21), "targetpos" {5, &+48h}
// (009C1F55), "targetentity" {4, &e} (009C1F7D) where e is the stack slot of
// `arg`. Then the 0099A9B0 swap of +44h to e with +30h as the observer
// (009C1F7F-009C1FA0) and 009BEBA0(a) (009C1FA7). Returns a.
// UNCERTAINTY: 009C1E64 and 009C1F5C (`MOV [ESP+2Ch],ESP`) overwrite the arg slot
// with a stack address before the targetentity read, so in the image a missing key
// leaves e = that stack address; here e starts as arg (as 009C2B80's slot). Shared
// RNG stream 1. EH unwinding not modelled.
void* bot_approach_move_to_construct_from_stream_009c1dc0(Cc13Seg67bHost& host, void* a,
                                                          void* arg, void* archive) {
    const float f = at<float>(host.game_tuning_singleton_0042e740(), 0x37C);
    host.bot_approach_construct_speed_reference_009f9ce0(a, arg, f);
    at<std::uint32_t>(a, 0x00) = kMoveToApproachVtable_00d209cc;
    at<std::uint32_t>(a, 0x34) = 0;
    at<std::uint32_t>(a, 0x38) = 0;
    at<std::uint32_t>(a, 0x3C) = 0;
    at<std::uint32_t>(a, 0x44) = 0;
    at<std::uint8_t>(a, 0x40) = 1;
    at<std::uint32_t>(a, 0x30) = kObservedEndpointVtable_00ce3cf0;
    at<float>(a, 0x54) = kOne_00d7a24c;
    at<float>(a, 0x58) = -host.random_uniform_00bd2f10(1, 0.0f, 1.0f);
    at<float>(a, 0x68) = kFifty_00ceb4d4;
    read_field(host, archive, kStrAltOffset, 2, ptr(a, 0x2C));
    read_field(host, archive, kStrClosingComplete_00d20a54, 3, ptr(a, 0x5C));
    std::uint32_t ref[3] = {};
    host.archive_reader_ref_open_00bd4fe0(archive, ref, name_record(kStrUpdateTrgPosTimer_00d20a40));
    host.time_freq_pair_read_00410d80(ptr(a, 0x54), ref);
    read_field(host, archive, kStrClosingTime_00d20a34, 2, ptr(a, 0x60));
    read_field(host, archive, kStrTargetPos_00d20a28, 5, ptr(a, 0x48));
    void* e = arg;
    read_field(host, archive, kStrTargetEntity_00d04f30, 4, &e);
    pilot_plan_set_observed_endpoint_0099a9b0(host, ptr(a, 0x30), e);
    bot_approach_refresh_target_009beba0(host, a);
    return a;
}

// 009C33D0 (009C33D0-009C350C). Writes (archive->vtable[0Ch]): "altOffset" {2,
// +2Ch} (009C340F), "closingComplete" {3, byte +5Ch} (009C3443); 00BD5010(archive,
// block, "_updateTrgPosTimer") (009C3467) and 00410E00(ECX = +54h, block by value)
// (009C346F); "closingTime" {2, +60h} (009C34AA), "targetpos" {5, &+48h}
// (009C34DE), "targetentity" {4, [+44h]} (009C3505). RET 4.
// UNCERTAINTY: the byte record leaves the payload's upper three bytes as stack
// garbage (009C3419 MOV byte); here they are zero.
void bot_approach_move_to_save_009c33d0(Cc13Seg67bHost& host, void* a, void* archive) {
    host.archive_vcall_0c_write(archive, name_record(kStrAltOffset),
                                record(2, float_bits(at<float>(a, 0x2C))));
    host.archive_vcall_0c_write(archive, name_record(kStrClosingComplete_00d20a54),
                                record(3, at<std::uint8_t>(a, 0x5C)));
    std::uint32_t ref[3] = {};
    host.archive_writer_ref_open_00bd5010(archive, ref, name_record(kStrUpdateTrgPosTimer_00d20a40));
    host.time_freq_pair_write_00410e00(ptr(a, 0x54), ref);
    host.archive_vcall_0c_write(archive, name_record(kStrClosingTime_00d20a34),
                                record(2, float_bits(at<float>(a, 0x60))));
    host.archive_vcall_0c_write(archive, name_record(kStrTargetPos_00d20a28),
                                record(5, reinterpret_cast<std::uintptr_t>(ptr(a, 0x48))));
    host.archive_vcall_0c_write(archive, name_record(kStrTargetEntity_00d04f30),
                                record(4, at<std::uint32_t>(a, 0x44)));
}

// ---------------------------------------------------------------------------
// The move-on-path approach block from a stream.

namespace {
constexpr std::uint32_t kMoveOnPathBlockVtable_00d208f8 = 0x00D208F8u;
constexpr std::uint32_t kMoveOnPathRegistryVtable_00d208f4 = 0x00D208F4u;
constexpr std::uint32_t kMoveOnPathStateVtable_00d208bc = 0x00D208BCu;
constexpr std::uint32_t kStrPathPath_00d20860 = 0x00D20860u;    // "path (path)"
constexpr std::uint32_t kStrPathFollow_00d20850 = 0x00D20850u;  // "follow (path)"
constexpr std::uint32_t kStrPathCircle_00d20840 = 0x00D20840u;  // "circle (path)"

// The tail 009BCC60 and 009BCD50 share (009BCC8D-009BCD1E and 009BCD84-009BCE10):
// the registry at +40h, the vtables, the path state +54h, follow +6Ch, circle
// +104h and the three registrations.
void move_on_path_block_tail(Cc13Seg67bHost& host, void* block) {
    host.bot_state_registry_construct_00411e20(ptr(block, 0x40));
    at<std::uint32_t>(block, 0x00) = kMoveOnPathBlockVtable_00d208f8;
    at<std::uint32_t>(block, 0x40) = kMoveOnPathRegistryVtable_00d208f4;
    at<std::uintptr_t>(block, 0x58) = reinterpret_cast<std::uintptr_t>(block);
    at<std::uint32_t>(block, 0x60) = 0;
    at<std::uint32_t>(block, 0x64) = 0;
    at<std::uint32_t>(block, 0x68) = 0;
    at<std::uint32_t>(block, 0x5C) = 0;
    at<std::uint32_t>(block, 0x54) = kMoveOnPathStateVtable_00d208bc;
    bot_state_follow_construct_009c2980(host, ptr(block, 0x6C), block, kHundred_00ce3d08);
    bot_state_move_on_path_circle_construct_009bc960(host, ptr(block, 0x104), block);
    host.state_registry_add_00411e70(ptr(block, 0x40), kStrPathPath_00d20860, ptr(block, 0x54));
    host.state_registry_add_00411e70(ptr(block, 0x40), kStrPathFollow_00d20850, ptr(block, 0x6C));
    host.state_registry_add_00411e70(ptr(block, 0x40), kStrPathCircle_00d20840, ptr(block, 0x104));
}
}  // namespace

// 009BCD50 (009BCD50-009BCE2B). EH frame 00CAF5D7. 009BC7E0(ECX = block,
// [bot+50h], archive) (009BCD7F); 00411E20(block+40h) (009BCD8F); [block] =
// 00D208F8, [block+40h] = 00D208F4 (009BCD97, 009BCD9D); the inline state at
// +54h: +58h = block, +60h/+64h/+68h/+5Ch = 0, [+54h] = 00D208BC (009BCDA3-
// 009BCDB2); 009C2980(ECX = block+6Ch, block, [00CE3D08] 100.0) (009BCDCD);
// 009BC960(ECX = block+104h, block) (009BCDDE); 00411E70(block+40h, name, state)
// for "path (path)" +54h, "follow (path)" +6Ch and "circle (path)" +104h
// (009BCDF0, 009BCDFD, 009BCE10). RET 8, EAX = block.
// UNCERTAINTY: EH unwinding not modelled.
void* bot_task_move_on_path_construct_approach_block_from_stream_009bcd50(Cc13Seg67bHost& host,
                                                                          void* block, void* bot,
                                                                          void* archive) {
    bot_approach_move_on_path_construct_from_stream_009bc7e0(host, block, ptr_at(bot, 0x50),
                                                             archive);
    move_on_path_block_tail(host, block);
    return block;
}

namespace {
constexpr std::uint32_t kLevelBombBlockVtable_00d20204 = 0x00D20204u;
constexpr std::uint32_t kLevelBombRegistryVtable_00d20200 = 0x00D20200u;
constexpr std::uint32_t kLevelBombAimVtable_00d2015c = 0x00D2015Cu;
constexpr std::uint32_t kLevelBombPrepareVtable_00d201a0 = 0x00D201A0u;
constexpr std::uint32_t kLevelBombReleaseVtable_00d20140 = 0x00D20140u;
constexpr std::uint32_t kLevelBombGoAwayVtable_00d2011c = 0x00D2011Cu;

// The tail 009B75E0 and 009B77C0 share (009B768B-009B7792 and 009B7845-009B794C):
// follow +13Ch, attackrun +1D4h, aim +1FCh, prepare +218h, release +2BCh, goaway
// +2E0h, then 009B42D0.
void level_bomb_block_tail(Cc13Seg67bHost& host, void* block) {
    bot_state_follow_construct_009c2980(host, ptr(block, 0x13C), block, kHundred_00ce3d08);
    bot_state_level_bomb_attack_run_construct_009b49b0(host, ptr(block, 0x1D4), block);
    at<std::uintptr_t>(block, 0x200) = reinterpret_cast<std::uintptr_t>(block);
    at<std::uint32_t>(block, 0x208) = 0;
    at<std::uint32_t>(block, 0x20C) = 0;
    at<std::uint32_t>(block, 0x210) = 0;
    at<std::uint32_t>(block, 0x204) = 0;
    at<std::uint32_t>(block, 0x1FC) = kLevelBombAimVtable_00d2015c;
    at<std::uint8_t>(block, 0x215) = 0;
    at<std::uint8_t>(block, 0x214) = 0;
    bot_state_follow_construct_009c2980(host, ptr(block, 0x218), block, kHundred_00ce3d08);
    at<std::uint32_t>(block, 0x218) = kLevelBombPrepareVtable_00d201a0;
    at<std::uint8_t>(block, 0x2B8) = 0;
    at<float>(block, 0x2B0) = kMinusOne_00d7a260_f;
    at<std::uintptr_t>(block, 0x2C0) = reinterpret_cast<std::uintptr_t>(block);
    at<std::uint32_t>(block, 0x2C8) = 0;
    at<std::uint32_t>(block, 0x2CC) = 0;
    at<std::uint32_t>(block, 0x2D0) = 0;
    at<std::uint32_t>(block, 0x2C4) = 0;
    at<std::uint32_t>(block, 0x2BC) = kLevelBombReleaseVtable_00d20140;
    at<std::uintptr_t>(block, 0x2E4) = reinterpret_cast<std::uintptr_t>(block);
    at<std::uint32_t>(block, 0x2EC) = 0;
    at<std::uint32_t>(block, 0x2F0) = 0;
    at<std::uint32_t>(block, 0x2F4) = 0;
    at<std::uint32_t>(block, 0x2E8) = 0;
    at<std::uint32_t>(block, 0x2E0) = kLevelBombGoAwayVtable_00d2011c;
    const float turn = at<float>(ptr_at(ptr_at(block, 0x2E4), 0x08), 0x268);
    at<float>(block, 0x2F8) = turn + turn;
    bot_task_level_bomb_register_state_names_009b42d0(host, block);
}
}  // namespace

// 009B77C0 (009B77C0-009B7967). EH frame 00CAEFC8. 009B4690(ECX = block, [bot+50h],
// archive) (009B77ED); 00411E20(block+DCh) (009B7800); [block] = 00D20204,
// [block+DCh] = 00D20200 (009B7817, 009B781D); 00BD4FE0(archive, outgoing block,
// "_state_moveTo") (009B7834) and 009B7480(ECX = block+F0h, block, that block by
// value) (009B7840); 009C2980(ECX = block+13Ch, block, 100.0) (009B785B);
// 009B49B0(ECX = block+1D4h, block) (009B786C). Inline state +1FCh: +200h =
// block, +208h/+20Ch/+210h/+204h = 0, vtable 00D2015C, bytes +215h/+214h = 0
// (009B7871-009B789F). 009C2980(ECX = block+218h, block, 100.0) (009B78BD), then
// [+218h] = 00D201A0, byte +2B8h = 0, +2B0h = [00D7A260] -1.0 (009B78C2-009B78D6).
// Inline state +2BCh: +2C0h = block, +2C8h/+2CCh/+2D0h/+2C4h = 0, vtable 00D20140
// (009B78DE-009B78FC). Inline state +2E0h: +2E4h = block, +2ECh/+2F0h/+2F4h/+2E8h
// = 0, vtable 00D2011C, +2F8h = float(2 x [[+2E4h]+8]+268h) (009B7906-009B793F,
// FADD ST0, ST0). 009B42D0(block) (009B794C). RET 8, EAX = block.
// The state names are the 009B42D0 registrations (LevelBomb/aim, /prepare,
// /release, /goaway). UNCERTAINTY: EH unwinding not modelled.
void* bot_task_level_bomb_construct_approach_block_from_stream_009b77c0(Cc13Seg67bHost& host,
                                                                        void* block, void* bot,
                                                                        void* archive) {
    bot_approach_level_bomb_construct_from_stream_009b4690(host, block, ptr_at(bot, 0x50), archive);
    host.bot_state_registry_construct_00411e20(ptr(block, 0xDC));
    at<std::uint32_t>(block, 0x000) = kLevelBombBlockVtable_00d20204;
    at<std::uint32_t>(block, 0x0DC) = kLevelBombRegistryVtable_00d20200;
    std::uint32_t ref[3] = {};
    host.archive_reader_ref_open_00bd4fe0(archive, ref, name_record(kStrStateMoveTo));
    bot_state_level_bomb_run_construct_from_stream_009b7480(host, ptr(block, 0xF0), block, ref);
    level_bomb_block_tail(host, block);
    return block;
}

// ---------------------------------------------------------------------------
// The land approach block from a stream.

namespace {
constexpr std::uint32_t kLandBlockVtable_00d1ff88 = 0x00D1FF88u;
constexpr std::uint32_t kLandRegistryVtable_00d1ff84 = 0x00D1FF84u;

// An embedded land state as 009B30DD-009B31E7 lays each one out: +4h = owner
// block, +0Ch/+10h/+14h/+8h = 0, byte +18h = 0, then the vtable.
void land_inline_state(void* block, std::size_t off, std::uint32_t vtable) {
    at<std::uintptr_t>(block, off + 0x04) = reinterpret_cast<std::uintptr_t>(block);
    at<std::uint32_t>(block, off + 0x0C) = 0;
    at<std::uint32_t>(block, off + 0x10) = 0;
    at<std::uint32_t>(block, off + 0x14) = 0;
    at<std::uint32_t>(block, off + 0x08) = 0;
    at<std::uint8_t>(block, off + 0x18) = 0;
    at<std::uint32_t>(block, off + 0x00) = vtable;
}

// The tail 009B2E50 and 009B3040 share (009B2ECE-009B3010 and 009B30C2-009B3204):
// follow +108h, six embedded states, bytes +274h/+275h, then 009AF9A0.
void land_block_tail(Cc13Seg67bHost& host, void* block) {
    bot_state_follow_construct_009c2980(host, ptr(block, 0x108), block, kHundred_00ce3d08);
    land_inline_state(block, 0x1A0, 0x00D1FEA4u);
    land_inline_state(block, 0x1C0, 0x00D1FEF4u);
    land_inline_state(block, 0x1E0, 0x00D1FF14u);
    land_inline_state(block, 0x200, 0x00D1FF44u);
    land_inline_state(block, 0x228, 0x00D1FF60u);
    land_inline_state(block, 0x254, 0x00D1FED4u);
    at<std::uint8_t>(block, 0x274) = 0;
    at<std::uint8_t>(block, 0x275) = 0;
    host.land_approach_register_states_009af9a0(block);
}
}  // namespace

// 009B3040 (009B3040-009B321F). EH frame 00CAEBF6. 009AFFF0(ECX = block, arg,
// archive) (009B306A; the land task passes [bot+50h] as arg); 00411E20(block+B8h)
// (009B307D); [block] = 00D1FF88, [block+B8h] = 00D1FF84 (009B3094, 009B309A);
// 00BD4FE0(archive, outgoing block, "_state_moveTo") (009B30B1) and 009C2B80(ECX =
// block+CCh, block, that block by value) (009B30BD); 009C2980(ECX = block+108h,
// block, 100.0) (009B30D8). Six embedded states: +1A0h 00D1FEA4, +1C0h 00D1FEF4,
// +1E0h 00D1FF14, +200h 00D1FF44, +228h 00D1FF60, +254h 00D1FED4 (009B30DD-
// 009B31E7); bytes +274h/+275h = 0 (009B31F1, 009B31F7); 009AF9A0(block)
// (009B3204). RET 8, EAX = block.
// UNCERTAINTY: EH unwinding not modelled.
void* bot_task_land_construct_approach_block_from_stream_009b3040(Cc13Seg67bHost& host,
                                                                  void* block, void* arg,
                                                                  void* archive) {
    bot_approach_land_construct_from_stream_009afff0(host, block, arg, archive);
    host.bot_state_registry_construct_00411e20(ptr(block, 0xB8));
    at<std::uint32_t>(block, 0x00) = kLandBlockVtable_00d1ff88;
    at<std::uint32_t>(block, 0xB8) = kLandRegistryVtable_00d1ff84;
    std::uint32_t ref[3] = {};
    host.archive_reader_ref_open_00bd4fe0(archive, ref, name_record(kStrStateMoveTo));
    bot_state_construct_base_009c2b80(host, ptr(block, 0xCC), block, ref);
    land_block_tail(host, block);
    return block;
}

namespace {
// An embedded state without the byte at +18h: +4h = owner block, +0Ch/+10h/+14h/
// +8h = 0, then the vtable (the order of 009A50EA-009A5108 and its siblings).
void plain_inline_state(void* block, std::size_t off, std::uint32_t vtable) {
    at<std::uintptr_t>(block, off + 0x04) = reinterpret_cast<std::uintptr_t>(block);
    at<std::uint32_t>(block, off + 0x0C) = 0;
    at<std::uint32_t>(block, off + 0x10) = 0;
    at<std::uint32_t>(block, off + 0x14) = 0;
    at<std::uint32_t>(block, off + 0x08) = 0;
    at<std::uint32_t>(block, off + 0x00) = vtable;
}

constexpr std::uint32_t kDepthChargeBlockVtable_00d1f71c = 0x00D1F71Cu;
constexpr std::uint32_t kDepthChargeRegistryVtable_00d1f718 = 0x00D1F718u;
constexpr std::uint32_t kDepthChargeDonePrepareVtable_00d1f6fc = 0x00D1F6FCu;
constexpr float kFive_00ce3850 = 5.0f;  // 40A00000

// The tail 009A4DC0 and 009A5000 share (009A4E5C-009A4FD3 and 009A5085-009A51FC):
// follow +14Ch, done +1E4h, attackrun +284h, goaway +2A8h, aim +2CCh, prepare
// +2ECh, leave +38Ch, turnto +3A8h, then 009A3090.
void depth_charge_block_tail(Cc13Seg67bHost& host, void* block) {
    bot_state_follow_construct_009c2980(host, ptr(block, 0x14C), block, kHundred_00ce3d08);
    bot_state_follow_construct_009c2980(host, ptr(block, 0x1E4), block, kHundred_00ce3d08);
    at<std::uint32_t>(block, 0x1E4) = kDepthChargeDonePrepareVtable_00d1f6fc;
    at<float>(block, 0x27C) = kMinusOne_00d7a260_f;
    at<std::uint32_t>(block, 0x280) = 0;
    bot_state_depth_charge_attack_run_construct_009a36a0(host, ptr(block, 0x284), block);
    plain_inline_state(block, 0x2A8, 0x00D1F6A0u);
    plain_inline_state(block, 0x2CC, 0x00D1F6D8u);
    at<std::uint8_t>(block, 0x2E4) = 0;
    at<std::uint8_t>(block, 0x2E5) = 0;
    at<float>(block, 0x2E8) = kZero_s15;
    bot_state_follow_construct_009c2980(host, ptr(block, 0x2EC), block, kHundred_00ce3d08);
    at<std::uint32_t>(block, 0x2EC) = kDepthChargeDonePrepareVtable_00d1f6fc;
    at<float>(block, 0x384) = kMinusOne_00d7a260_f;
    at<std::uint32_t>(block, 0x388) = 0;
    plain_inline_state(block, 0x38C, 0x00D1F668u);
    at<float>(block, 0x3A4) = kFive_00ce3850;
    plain_inline_state(block, 0x3A8, 0x00D1F684u);
    at<float>(block, 0x3C0) = kZero_s15;
    bot_task_depth_charge_register_state_names_009a3090(host, block);
}
}  // namespace

// 009A5000 (009A5000-009A5217). EH frame 00CAE0D4. 009A35D0(ECX = block, [bot+50h],
// archive) (009A502D); 00411E20(block+FCh) (009A5040); [block] = 00D1F71C,
// [block+FCh] = 00D1F718 (009A5057, 009A505D); 00BD4FE0(archive, outgoing block,
// "_state_moveTo") (009A5074) and 009C2B80(ECX = block+110h, block, that block by
// value) (009A5080); 009C2980(ECX = block+14Ch, block, 100.0) (009A509B);
// 009C2980(ECX = block+1E4h, ...) (009A50B8), then [+1E4h] = 00D1F6FC, +27Ch =
// -1.0, +280h = 0 (009A50BD-009A50D3); 009A36A0(ECX = block+284h, block)
// (009A50E5); embedded +2A8h 00D1F6A0 (009A50EA-009A5108); embedded +2CCh 00D1F6D8
// with bytes +2E4h/+2E5h = 0 and +2E8h = 0.0 (009A5115-009A5149); 009C2980(ECX =
// block+2ECh, ...) (009A5169), then [+2ECh] = 00D1F6FC, +384h = -1.0, +388h = 0
// (009A516E-009A5184); embedded +38Ch 00D1F668 with +3A4h = [00CE3850] 5.0
// (009A518A-009A51BA); embedded +3A8h 00D1F684 with +3C0h = 0.0 (009A51C2-
// 009A51ED); 009A3090(block) (009A51FC). RET 8, EAX = block.
// UNCERTAINTY: EH unwinding not modelled.
void* bot_task_depth_charge_construct_approach_block_from_stream_009a5000(Cc13Seg67bHost& host,
                                                                          void* block, void* bot,
                                                                          void* archive) {
    bot_approach_depth_charge_construct_from_stream_009a35d0(host, block, ptr_at(bot, 0x50),
                                                             archive);
    host.bot_state_registry_construct_00411e20(ptr(block, 0xFC));
    at<std::uint32_t>(block, 0x000) = kDepthChargeBlockVtable_00d1f71c;
    at<std::uint32_t>(block, 0x0FC) = kDepthChargeRegistryVtable_00d1f718;
    std::uint32_t ref[3] = {};
    host.archive_reader_ref_open_00bd4fe0(archive, ref, name_record(kStrStateMoveTo));
    bot_state_construct_base_009c2b80(host, ptr(block, 0x110), block, ref);
    depth_charge_block_tail(host, block);
    return block;
}

// ---------------------------------------------------------------------------
// The dogfight approach block from a stream.

namespace {
constexpr std::uint32_t kDogfightBlockVtable_00d1f9a4 = 0x00D1F9A4u;
constexpr std::uint32_t kDogfightRegistryVtable_00d1f9a0 = 0x00D1F9A0u;

// The tail 009A94E0 and 009A9670 share (009A9560-009A9637 and 009A96F2-009A97C9):
// follow +154h, prepare +1ECh, aim +284h, maneuver +2ACh, attackrun +2ECh,
// avoid_roll +310h, avoid_turn +338h, then 009A6A60.
void dogfight_block_tail(Cc13Seg67bHost& host, void* block) {
    bot_state_follow_construct_009c2980(host, ptr(block, 0x154), block, kHundred_00ce3d08);
    bot_state_follow_construct_009c2980(host, ptr(block, 0x1EC), block, kHundred_00ce3d08);
    plain_inline_state(block, 0x284, 0x00D1F8D8u);
    bot_state_dogfight_maneuver_construct_009a84e0(host, ptr(block, 0x2AC), block);
    bot_state_dogfight_attack_run_construct_009a70e0(host, ptr(block, 0x2EC), block);
    plain_inline_state(block, 0x310, 0x00D1F918u);
    plain_inline_state(block, 0x338, 0x00D1F938u);
    bot_task_dogfight_register_state_names_009a6a60(host, block);
}
}  // namespace

// 009A9670 (009A9670-009A97E4). EH frame 00CAE456. 009A6D40(ECX = block, bot,
// archive) (009A969A); 00411E20(block+104h) (009A96AD); [block] = 00D1F9A4,
// [block+104h] = 00D1F9A0 (009A96C4, 009A96CA); 00BD4FE0(archive, outgoing block,
// "_state_moveTo") (009A96E1) and 009C2D00(ECX = block+118h, block, that block
// by value) (009A96ED); 009C2980(ECX = block+154h, block, 100.0) (009A9708) and
// (ECX = block+1ECh) (009A9723); embedded dogfight/aim +284h 00D1F8D8
// (009A9728-009A9746); 009A84E0(ECX = block+2ACh, block) (009A975C); 009A70E0(ECX =
// block+2ECh, block) (009A976D); embedded avoid_roll +310h 00D1F918 and avoid_turn
// +338h 00D1F938 (009A9772-009A97B8); 009A6A60(block) (009A97C9). RET 8, EAX =
// block. The state names are the 009A6A60 registrations.
// UNCERTAINTY: EH unwinding not modelled.
void* bot_task_dogfight_construct_approach_block_from_stream_009a9670(Cc13Seg67bHost& host,
                                                                      void* block, void* bot,
                                                                      void* archive) {
    bot_approach_dogfight_construct_from_stream_009a6d40(host, block, bot, archive);
    host.bot_state_registry_construct_00411e20(ptr(block, 0x104));
    at<std::uint32_t>(block, 0x000) = kDogfightBlockVtable_00d1f9a4;
    at<std::uint32_t>(block, 0x104) = kDogfightRegistryVtable_00d1f9a0;
    std::uint32_t ref[3] = {};
    host.archive_reader_ref_open_00bd4fe0(archive, ref, name_record(kStrStateMoveTo));
    bot_state_move_to_construct_from_stream_009c2d00(host, ptr(block, 0x118), block, ref);
    dogfight_block_tail(host, block);
    return block;
}

namespace {
constexpr double kHundred_00d7a220 = 100.0;  // 4059000000000000
}  // namespace

// 009B75E0 (009B75E0-009B77AD). EH frame 00CAEF38. 009B44F0(ECX = block, [bot+50h],
// target) (009B760F); 00411E20(block+DCh) (009B7622); [block] = 00D20204,
// [block+DCh] = 00D20200 (009B766D, 009B7673); 009B6C10(ECX = block+F0h, block,
// target, near = float(+B0h - double [00D7A220] 100.0), far = +B0h, third = +B4h)
// (009B7627-009B7686); then the 009B77C0 tail (009B768B-009B7792). RET 8, EAX =
// block.
// UNCERTAINTY: `near` is rounded once at 24 bits in the image; here a double
// difference is rounded to float. EH unwinding not modelled.
void* bot_approach_level_bomb_construct_009b75e0(Cc13Seg67bHost& host, void* block, void* bot,
                                                 void* target) {
    bot_approach_level_bomb_construct_base_009b44f0(host, block, ptr_at(bot, 0x50), target);
    host.bot_state_registry_construct_00411e20(ptr(block, 0xDC));
    const float third = at<float>(block, 0xB4);
    const float far_ = at<float>(block, 0xB0);
    const float near_ = static_cast<float>(static_cast<double>(far_) - kHundred_00d7a220);
    at<std::uint32_t>(block, 0x000) = kLevelBombBlockVtable_00d20204;
    at<std::uint32_t>(block, 0x0DC) = kLevelBombRegistryVtable_00d20200;
    bot_state_level_bomb_move_to_construct_009b6c10(host, ptr(block, 0xF0), block, target, near_,
                                                    far_, third);
    level_bomb_block_tail(host, block);
    return block;
}

namespace {
constexpr float kFiveHundred_00ce397c = 500.0f;  // 43FA0000
constexpr float kThousand_00ce3804 = 1000.0f;    // 447A0000
}  // namespace

// 009A94E0 (009A94E0-009A9652). EH frame 00CAE3C6. 009A6C10(ECX = block, bot, arg)
// (009A950A); 00411E20(block+104h) (009A951D); [block] = 00D1F9A4, [block+104h] =
// 00D1F9A0 (009A9544, 009A9550); 009C2CA0(ECX = block+118h, block, arg, [00CE397C]
// 500.0, [00CE3D08] 100.0, [00CE3804] 1000.0) (009A9522-009A955B); then the tail
// shared with 009A9670 (009A9560-009A9637). RET 8, EAX = block.
// UNCERTAINTY: EH unwinding not modelled.
void* bot_task_dogfight_construct_approach_block_009a94e0(Cc13Seg67bHost& host, void* block,
                                                          void* bot, void* arg) {
    bot_approach_dogfight_construct_009a6c10(host, block, bot, arg);
    host.bot_state_registry_construct_00411e20(ptr(block, 0x104));
    at<std::uint32_t>(block, 0x000) = kDogfightBlockVtable_00d1f9a4;
    at<std::uint32_t>(block, 0x104) = kDogfightRegistryVtable_00d1f9a0;
    bot_state_dogfight_move_to_construct_009c2ca0(host, ptr(block, 0x118), block, arg,
                                                  kFiveHundred_00ce397c, kHundred_00ce3d08,
                                                  kThousand_00ce3804);
    dogfight_block_tail(host, block);
    return block;
}

namespace {
constexpr std::uint32_t kDiveBombBlockVtable_00d20e08 = 0x00D20E08u;
constexpr std::uint32_t kDiveBombRegistryVtable_00d20e04 = 0x00D20E04u;
constexpr std::uint32_t kDiveBombDonePrepareVtable_00d20d28 = 0x00D20D28u;

// 009C7612-009C76C4: the ten registrations into block+E4h, in order.
constexpr StateNamePair kDiveBombStateNamePairs[] = {
    {0x00D20DF0u, 0x0F8},  // "moveto (DiveBomb)"
    {0x00D20DDCu, 0x134},  // "follow (DiveBomb)"
    {0x00D20DCCu, 0x26C},  // "DiveBomb/done"
    {0x00D20DBCu, 0x30C},  // "DiveBomb/goaway"
    {0x00D20DA8u, 0x33C},  // "DiveBomb/aimdive"
    {0x00D20D94u, 0x35C},  // "DiveBomb/aimglide"
    {0x00D20D80u, 0x380},  // "DiveBomb/flyabove"
    {0x00D20D6Cu, 0x1CC},  // "DiveBomb/prepare"
    {0x00D20D58u, 0x3A4},  // "DiveBomb/turndown"
    {0x00D20D44u, 0x3C4},  // "DiveBomb/attackrun"
};
}  // namespace

// 009C73A0 (009C73A0-009C76DF). EH frame 00CAFC32. 009C3EA0(ECX = block, [bot+50h],
// target) (009C73CD); 00411E20(block+E4h) (009C73E0); [block] = 00D20E08,
// [block+E4h] = 00D20E04 (009C740B, 009C7415); 009C2AC0(ECX = block+F8h, block,
// target, float(+A8h - double [00D7A220] 100.0), +A8h, +B4h) (009C73E5-009C7436);
// 009C2980(ECX = block+134h, block, 100.0) (009C7451); 009C2980 at +1CCh and at
// +26Ch, each then given vtable 00D20D28, byte +9Ch = 0 and +98h = -1.0
// (009C746E-009C74C2). Embedded goaway +30Ch (00D20CA0): +330h = 100.0, +328h =
// 0.0, +32Ch = float(2 x [[+310h]+B4h]) (009C74CA-009C7523). Embedded aimdive
// +33Ch (00D20CE8): byte +354h = 0, +358h = 0.0, byte +355h = 1 (009C7529-
// 009C755F). Embedded aimglide +35Ch (00D20CC8): +378h = 0.0, byte +374h = 0,
// +37Ch = [00CE3850] 5.0 (009C7566-009C75A4). Embedded flyabove +380h (00D20D04):
// bytes +398h..+39Bh = 0, +3A0h = 0 (009C75AC-009C75D4). Embedded turndown +3A4h
// (00D20C84) (009C75D7-009C75F5). 009C4140(ECX = block+3C4h, block) (009C760D).
// Ten 00411E70 registrations (009C7612-009C76C4). RET 8, EAX = block.
// UNCERTAINTY: `near` is rounded once at 24 bits in the image; here a double
// difference is rounded to float. EH unwinding not modelled.
void* bot_task_dive_bomb_construct_approach_block_009c73a0(Cc13Seg67bHost& host, void* block,
                                                           void* bot, void* target) {
    bot_approach_dive_bomb_construct_009c3ea0(host, block, ptr_at(bot, 0x50), target);
    host.bot_state_registry_construct_00411e20(ptr(block, 0xE4));
    const float third = at<float>(block, 0xB4);
    const float far_ = at<float>(block, 0xA8);
    at<std::uint32_t>(block, 0x000) = kDiveBombBlockVtable_00d20e08;
    at<std::uint32_t>(block, 0x0E4) = kDiveBombRegistryVtable_00d20e04;
    const float near_ = static_cast<float>(static_cast<double>(far_) - kHundred_00d7a220);
    host.bot_state_move_to_construct_009c2ac0(ptr(block, 0xF8), block, target, near_, far_, third);
    bot_state_follow_construct_009c2980(host, ptr(block, 0x134), block, kHundred_00ce3d08);
    bot_state_follow_construct_009c2980(host, ptr(block, 0x1CC), block, kHundred_00ce3d08);
    at<std::uint32_t>(block, 0x1CC) = kDiveBombDonePrepareVtable_00d20d28;
    at<std::uint8_t>(block, 0x268) = 0;
    at<float>(block, 0x264) = kMinusOne_00d7a260_f;
    bot_state_follow_construct_009c2980(host, ptr(block, 0x26C), block, kHundred_00ce3d08);
    at<std::uint32_t>(block, 0x26C) = kDiveBombDonePrepareVtable_00d20d28;
    at<std::uint8_t>(block, 0x308) = 0;
    at<float>(block, 0x304) = kMinusOne_00d7a260_f;
    plain_inline_state(block, 0x30C, 0x00D20CA0u);
    const float b4 = at<float>(ptr_at(block, 0x310), 0xB4);
    at<float>(block, 0x330) = kHundred_00ce3d08;
    at<float>(block, 0x328) = kZero_s15;
    at<float>(block, 0x32C) = b4 + b4;
    plain_inline_state(block, 0x33C, 0x00D20CE8u);
    at<std::uint8_t>(block, 0x354) = 0;
    at<float>(block, 0x358) = kZero_s15;
    at<std::uint8_t>(block, 0x355) = 1;
    plain_inline_state(block, 0x35C, 0x00D20CC8u);
    at<float>(block, 0x378) = kZero_s15;
    at<std::uint8_t>(block, 0x374) = 0;
    at<float>(block, 0x37C) = kFive_00ce3850;
    plain_inline_state(block, 0x380, 0x00D20D04u);
    at<std::uint8_t>(block, 0x398) = 0;
    at<std::uint8_t>(block, 0x399) = 0;
    at<std::uint8_t>(block, 0x39A) = 0;
    at<std::uint8_t>(block, 0x39B) = 0;
    at<std::uint32_t>(block, 0x3A0) = 0;
    plain_inline_state(block, 0x3A4, 0x00D20C84u);
    bot_state_dive_bomb_attack_run_construct_009c4140(host, ptr(block, 0x3C4), block);
    register_state_names(host, block, 0xE4, kDiveBombStateNamePairs,
                         sizeof(kDiveBombStateNamePairs) / sizeof(kDiveBombStateNamePairs[0]));
    return block;
}

// ---------------------------------------------------------------------------
// The drop-kamikaze approach block from a stream.

namespace {
constexpr std::uint32_t kDropKamikazeBlockVtable_00d1fc60 = 0x00D1FC60u;
constexpr std::uint32_t kDropKamikazeRegistryVtable_00d1fc5c = 0x00D1FC5Cu;
constexpr std::uint32_t kDropKamikazeFollowLikeVtable_00d1fc08 = 0x00D1FC08u;

// The tail 009AD720 and 009AD920 share (009AD7CB-009AD8E8 and 009AD9A5-009ADAC2):
// follow +114h, the five embedded or follow-built states, then 009AB740.
void drop_kamikaze_block_tail(Cc13Seg67bHost& host, void* block) {
    bot_state_follow_construct_009c2980(host, ptr(block, 0x114), block, kHundred_00ce3d08);
    plain_inline_state(block, 0x1AC, 0x00D1FBECu);
    at<float>(block, 0x1C4) = kZero_s15;
    bot_state_follow_construct_009c2980(host, ptr(block, 0x1C8), block, kHundred_00ce3d08);
    at<std::uint32_t>(block, 0x1C8) = kDropKamikazeFollowLikeVtable_00d1fc08;
    at<std::uint8_t>(block, 0x268) = 0;
    at<float>(block, 0x260) = kMinusOne_00d7a260_f;
    bot_state_follow_construct_009c2980(host, ptr(block, 0x26C), block, kHundred_00ce3d08);
    at<std::uint32_t>(block, 0x26C) = kDropKamikazeFollowLikeVtable_00d1fc08;
    at<std::uint8_t>(block, 0x30C) = 0;
    at<float>(block, 0x304) = kMinusOne_00d7a260_f;
    plain_inline_state(block, 0x310, 0x00D1FC40u);
    plain_inline_state(block, 0x334, 0x00D1FC24u);
    at<std::uint8_t>(block, 0x34C) = 0;
    bot_task_drop_kamikaze_register_state_names_009ab740(host, block);
}
}  // namespace

// 009AD920 (009AD920-009ADAD9). EH frame 00CAE798. 009ABBE0(ECX = block, [bot+50h],
// archive) (009AD94D); 00411E20(block+C4h) (009AD960); [block] = 00D1FC60,
// [block+C4h] = 00D1FC5C (009AD977, 009AD97D); 00BD4FE0(archive, outgoing block,
// "_state_moveTo") (009AD994) and 009C2B80(ECX = block+D8h, block, that block by
// value) (009AD9A0); 009C2980(ECX = block+114h, block, 100.0) (009AD9BB); embedded
// +1ACh (00D1FBEC) with +1C4h = 0.0 (009AD9C0-009AD9EB); 009C2980 at +1C8h and at
// +26Ch, each then given vtable 00D1FC08, byte +A0h = 0 and +98h = -1.0
// (009ADA0B-009ADA5D); embedded +310h (00D1FC40) (009ADA65-009ADA83); embedded
// +334h (00D1FC24) with byte +34Ch = 0 (009ADA8D-009ADAB5); 009AB740(block)
// (009ADAC2). RET 8, EAX = block. The state names are the 009AB740
// registrations.
// UNCERTAINTY: EH unwinding not modelled.
void* bot_task_drop_kamikaze_construct_approach_block_from_stream_009ad920(Cc13Seg67bHost& host,
                                                                           void* block, void* bot,
                                                                           void* archive) {
    bot_approach_drop_kamikaze_construct_base_from_bag_009abbe0(host, block, ptr_at(bot, 0x50),
                                                                archive);
    host.bot_state_registry_construct_00411e20(ptr(block, 0xC4));
    at<std::uint32_t>(block, 0x000) = kDropKamikazeBlockVtable_00d1fc60;
    at<std::uint32_t>(block, 0x0C4) = kDropKamikazeRegistryVtable_00d1fc5c;
    std::uint32_t ref[3] = {};
    host.archive_reader_ref_open_00bd4fe0(archive, ref, name_record(kStrStateMoveTo));
    bot_state_construct_base_009c2b80(host, ptr(block, 0xD8), block, ref);
    drop_kamikaze_block_tail(host, block);
    return block;
}

// 009AD720 (009AD720-009AD903). EH frame 00CAE708. 009ABAB0(ECX = block, [bot+50h],
// target) (009AD74F); 00411E20(block+C4h) (009AD762); [block] = 00D1FC60,
// [block+C4h] = 00D1FC5C (009AD7AD, 009AD7B3); 009C2AC0(ECX = block+D8h, block,
// target, float(+A8h - double [00D7A220] 100.0), +A8h, +ACh) (009AD767-009AD7C6);
// then the tail shared with 009AD920 (009AD7CB-009AD8E8). RET 8, EAX = block.
// UNCERTAINTY: `near` is rounded once at 24 bits in the image; here a double
// difference is rounded to float. EH unwinding not modelled.
void* bot_approach_drop_kamikaze_construct_009ad720(Cc13Seg67bHost& host, void* block, void* bot,
                                                    void* target) {
    host.bot_approach_drop_kamikaze_construct_base_009abab0(block, ptr_at(bot, 0x50), target);
    host.bot_state_registry_construct_00411e20(ptr(block, 0xC4));
    const float third = at<float>(block, 0xAC);
    const float far_ = at<float>(block, 0xA8);
    const float near_ = static_cast<float>(static_cast<double>(far_) - kHundred_00d7a220);
    at<std::uint32_t>(block, 0x000) = kDropKamikazeBlockVtable_00d1fc60;
    at<std::uint32_t>(block, 0x0C4) = kDropKamikazeRegistryVtable_00d1fc5c;
    host.bot_state_move_to_construct_009c2ac0(ptr(block, 0xD8), block, target, near_, far_, third);
    drop_kamikaze_block_tail(host, block);
    return block;
}

// ---------------------------------------------------------------------------
// The direct move-on-path, depth-charge and land approach blocks.

// 009BCC60 (009BCC60-009BCD34). EH frame 00CAF587. 009BC780(ECX = block, [bot+50h])
// (009BCC88); then the tail shared with 009BCD50 (009BCC8D-009BCD1E). RET 4, EAX =
// block.
// UNCERTAINTY: EH unwinding not modelled.
void* bot_approach_move_on_path_construct_with_states_009bcc60(Cc13Seg67bHost& host, void* block,
                                                               void* bot) {
    host.bot_approach_move_on_path_construct_009bc780(block, ptr_at(bot, 0x50));
    move_on_path_block_tail(host, block);
    return block;
}

// 009A4DC0 (009A4DC0-009A4FEE). EH frame 00CAE034. 009A3390(ECX = block, [bot+50h],
// target) (009A4DEF); 00411E20(block+FCh) (009A4E02); [block] = 00D1F71C,
// [block+FCh] = 00D1F718 (009A4E22, 009A4E28); d = float(+3Ch + +34h) (009A4E11-
// 009A4E2E); 009C2AC0(ECX = block+110h, block, target, d, d, +40h) (009A4E57); then
// the tail shared with 009A5000 (009A4E5C-009A4FD3). RET 8, EAX = block.
// UNCERTAINTY: EH unwinding not modelled.
void* bot_approach_depth_charge_construct_with_states_009a4dc0(Cc13Seg67bHost& host,
                                                               void* block, void* bot,
                                                               void* target) {
    host.bot_approach_depth_charge_construct_009a3390(block, ptr_at(bot, 0x50), target);
    host.bot_state_registry_construct_00411e20(ptr(block, 0xFC));
    const float third = at<float>(block, 0x40);
    const float d = at<float>(block, 0x3C) + at<float>(block, 0x34);
    at<std::uint32_t>(block, 0x000) = kDepthChargeBlockVtable_00d1f71c;
    at<std::uint32_t>(block, 0x0FC) = kDepthChargeRegistryVtable_00d1f718;
    host.bot_state_move_to_construct_009c2ac0(ptr(block, 0x110), block, target, d, d, third);
    depth_charge_block_tail(host, block);
    return block;
}

namespace {
constexpr float kEightHundred_00ce3950 = 800.0f;  // 44480000
}  // namespace

// 009B2E50 (009B2E50-009B302A). EH frame 00CAEB66. 009AFE70(ECX = block, arg1,
// arg2) (009B2E7B); 00411E20(block+B8h) (009B2E8E); [block] = 00D1FF88,
// [block+B8h] = 00D1FF84 (009B2EB8, 009B2EC3); 009C2AC0(ECX = block+CCh, block,
// [block+30h], 100.0, 100.0, [00CE3950] 800.0) (009B2E93-009B2EC9); then the tail
// shared with 009B3040 (009B2ECE-009B3010). RET 8, EAX = block.
// UNCERTAINTY: EH unwinding not modelled. The roles of arg1 and arg2 are those
// 009AFE70 gives them.
void* bot_approach_land_construct_composite_009b2e50(Cc13Seg67bHost& host, void* block,
                                                     void* arg1, void* arg2) {
    bot_approach_land_construct_lower_009afe70(host, block, arg1, arg2);
    host.bot_state_registry_construct_00411e20(ptr(block, 0xB8));
    at<std::uint32_t>(block, 0x00) = kLandBlockVtable_00d1ff88;
    at<std::uint32_t>(block, 0xB8) = kLandRegistryVtable_00d1ff84;
    host.bot_state_move_to_construct_009c2ac0(ptr(block, 0xCC), block, ptr_at(block, 0x30),
                                              kHundred_00ce3d08, kHundred_00ce3d08,
                                              kEightHundred_00ce3950);
    land_block_tail(host, block);
    return block;
}

// ---------------------------------------------------------------------------
// Strafe, torpedo and takeoff task constructors.

namespace {
constexpr std::uint32_t kStrafeTaskVtable_00d210e0 = 0x00D210E0u;
constexpr std::uint32_t kStrafeApproachVtable_00d210d8 = 0x00D210D8u;
constexpr std::uint32_t kStrafeRegistryVtable_00d210d4 = 0x00D210D4u;
constexpr std::uint32_t kTorpedoTaskVtable_00d213c8 = 0x00D213C8u;
constexpr std::uint32_t kTorpedoApproachVtable_00d213c0 = 0x00D213C0u;
constexpr std::uint32_t kTorpedoRegistryVtable_00d213bc = 0x00D213BCu;
constexpr std::uint32_t kTakeoffTaskVtable_00d21228 = 0x00D21228u;
constexpr std::uint32_t kTakeoffApproachVtable_00d2121c = 0x00D2121Cu;
constexpr std::uint32_t kTakeoffRegistryVtable_00d21218 = 0x00D21218u;
constexpr float kZero_00d7a218 = 0.0f;  // 00000000
}  // namespace

// 009CC230 (009CC230-009CC2D6). EH frame. 0099C6F0(ECX = task, bot, 0Ah) (009CC256);
// 009CC020(ECX = task+3F8h, bot, target) (009CC271); [task] = 00D210E0,
// [task+3F8h] = 00D210D8, [task+4CCh] = 00D210D4 (009CC281-009CC28D);
// 007B8AD0([task+3FCh]) (009CC297) ? +4E0h : +51Ch -> +310h, its vtable[4]()
// (009CC2B7); 009F9980(ECX = task+3F8h, task) (009CC2BC). RET 8, EAX = task.
// UNCERTAINTY: EH unwinding not modelled.
void* bot_task_strafe_construct_009cc230(Cc13Seg67bHost& host, void* task, void* bot,
                                         void* target) {
    bot_task_construct_base_0099c6f0(host, task, bot, 0x0A);
    strafe_construct_approach_block_009cc020(host, ptr(task, 0x3F8), bot, target);
    construct_task_tail(host, task, kStrafeTaskVtable_00d210e0, kStrafeApproachVtable_00d210d8,
                        0x4CC, kStrafeRegistryVtable_00d210d4, 0x4E0, 0x51C);
    return task;
}

// 009D3050 (009D3050-009D30F6). EH frame 00CB03A6. 0099C6F0(task, bot, 0Eh)
// (009D3076); 009D2DA0(ECX = task+3F8h, bot, target) (009D3091); [task] =
// 00D213C8, [task+3F8h] = 00D213C0, [task+530h] = 00D213BC (009D30A1-009D30AD);
// 007B8AD0 (009D30B7) ? +544h : +580h -> +310h, enter (009D30D7); 009F9980
// (009D30DC). RET 8, EAX = task.
// UNCERTAINTY: EH unwinding not modelled.
void* bot_task_torpedo_construct_009d3050(Cc13Seg67bHost& host, void* task, void* bot,
                                          void* target) {
    bot_task_construct_base_0099c6f0(host, task, bot, 0x0E);
    torpedo_construct_approach_block_009d2da0(host, ptr(task, 0x3F8), bot, target);
    construct_task_tail(host, task, kTorpedoTaskVtable_00d213c8, kTorpedoApproachVtable_00d213c0,
                        0x530, kTorpedoRegistryVtable_00d213bc, 0x544, 0x580);
    return task;
}

// 009CF8E0 (009CF8E0-009CF9B3). EH frame 00CB0126. 0099C6F0(task, owner, 0Dh)
// (009CF905); 009CF710(ECX = task+3F8h, [owner+50h]) (009CF91E); [task] =
// 00D21228, [task+3F8h] = 00D2121C, [task+434h] = 00D21218 (009CF929-009CF935).
// u = [task+3FCh]: byte u+904h set and u+900h == 5 (009CF93F, 009CF94D) -> +4B8h;
// else flag byte set (009CF95E) -> +448h; else u+AA0h > [00D7A218] 0.0 (009CF975
// COMISS JA; NaN falls to +470h) -> +490h, else +470h. That state -> +310h and its
// vtable[4]() (009CF995); 009F9980(ECX = task+3F8h, task) (009CF99A). RET 8, EAX =
// task.
// UNCERTAINTY: EH unwinding not modelled.
void* bot_task_takeoff_construct_009cf8e0(Cc13Seg67bHost& host, void* task, void* owner,
                                          std::uint32_t flag) {
    bot_task_construct_base_0099c6f0(host, task, owner, 0x0D);
    takeoff_construct_approach_block_009cf710(host, ptr(task, 0x3F8), ptr_at(owner, 0x50));
    at<std::uint32_t>(task, 0x000) = kTakeoffTaskVtable_00d21228;
    at<std::uint32_t>(task, 0x3F8) = kTakeoffApproachVtable_00d2121c;
    at<std::uint32_t>(task, 0x434) = kTakeoffRegistryVtable_00d21218;
    void* const unit = ptr_at(task, 0x3FC);
    void* state;
    if (at<std::uint8_t>(unit, 0x904) != 0 && at<std::int32_t>(unit, 0x900) == 5) {
        state = ptr(task, 0x4B8);
    } else if (static_cast<std::uint8_t>(flag) != 0) {
        state = ptr(task, 0x448);
    } else {
        state = at<float>(unit, 0xAA0) > kZero_00d7a218 ? ptr(task, 0x490) : ptr(task, 0x470);
    }
    at<std::uintptr_t>(task, 0x310) = reinterpret_cast<std::uintptr_t>(state);
    host.state_vcall_04_enter(state);
    host.bot_approach_bind_to_task_009f9980(ptr(task, 0x3F8), task);
    return task;
}

// ---------------------------------------------------------------------------
// The strafe, torpedo and takeoff approach blocks (the hooks 009CC230, 009D3050 and
// 009CF8E0 called before batch 26).

namespace {
constexpr std::uint32_t kStrafeBlockVtable_00d210d0 = 0x00D210D0u;
constexpr std::uint32_t kStrafeRegistryVtable_00d210cc = 0x00D210CCu;

constexpr StateNamePair kStrafeStateNamePairs[] = {
    {0x00D210BCu, 0x0E8},  // "moveto (strafe)"    009CC171
    {0x00D210ACu, 0x124},  // "follow (strafe)"    009CC189
    {0x00D2109Cu, 0x1BC},  // "strafe/prepare"     009CC19C
    {0x00D21088u, 0x254},  // "strafe/gotowards"   009CC1A9
    {0x00D2107Cu, 0x278},  // "strafe/aim"         009CC1B6
    {0x00D2106Cu, 0x298},  // "strafe/goaway"      009CC1C9
    {0x00D21058u, 0x2E0},  // "strafe/attackrun"   009CC1DC
};

constexpr std::uint32_t kTorpedoBlockVtable_00d213b8 = 0x00D213B8u;
constexpr std::uint32_t kTorpedoRegistryVtable_00d213b4 = 0x00D213B4u;
constexpr std::uint32_t kTorpedoDonePrepareVtable_00d21320 = 0x00D21320u;
constexpr double kFifteen_00cf3f20 = 15.0;           // 402E000000000000
constexpr float kOneHundredTwenty_00d05804 = 120.0f;  // 42F00000

constexpr StateNamePair kTorpedoStateNamePairs[] = {
    {0x00D213A0u, 0x14C},  // "moveto (torpedo)"   009D2F92
    {0x00D2138Cu, 0x188},  // "follow (torpedo)"   009D2FAA
    {0x00D2137Cu, 0x220},  // "torpedo/done"       009D2FBD
    {0x00D2136Cu, 0x2E0},  // "torpedo/goaway"     009D2FCA
    {0x00D21360u, 0x318},  // "torpedo/aim"        009D2FD7
    {0x00D2134Cu, 0x2BC},  // "torpedo/attackrun"  009D2FEA
    {0x00D2133Cu, 0x348},  // "torpedo/prepare"    009D2FFD
};

constexpr std::uint32_t kTakeoffBlockVtable_00d21210 = 0x00D21210u;
constexpr std::uint32_t kTakeoffRegistryVtable_00d2120c = 0x00D2120Cu;

constexpr StateNamePair kTakeoffStateNamePairs[] = {
    {0x00D211FCu, 0x050},  // "takeoff/prepare"      009CF7E9
    {0x00D211ECu, 0x078},  // "takeoff/Takeoff"      009CF7FB
    {0x00D211D8u, 0x098},  // "takeoff/SlowTakeoff"  009CF80E
    {0x00D211C8u, 0x0C0},  // "takeoff/parking"      009CF81B
};
}  // namespace

// 009CC020 (BSP_BotTaskStrafe_ConstructApproachBlock): __thiscall(ECX = block, bot,
// target), RET 8, EAX = block. Evidence (disk bytes): 009CA4A0(ECX = block,
// [bot+50h], target) (009CC04F); 00411E20(ECX = block+D4h) (009CC062); vtables
// 00D210D0 / +D4h 00D210CC (009CC0A6-009CC0AC); 009C2AC0(ECX = block+E8h, block,
// target, near = +40h, far = +40h, third = +38h) (009CC067-009CC0B3: [esp] and
// [esp+4] take +40h, [esp+8] takes +38h); 009C2980(block, 100.0) at +124h and +1BCh
// with no vtable overwrite (009CC0CE, 009CC0E9). Embedded gotowards +254h (00D20FE8):
// +26Ch = +270h = [00CE3950] 800.0, byte +274h = 0 (009CC0EE-009CC11B). Embedded aim
// +278h (00D21020): +290h = [00CE3800] 0.5 (009CC11E-009CC143). 009CB500(ECX =
// block+298h, block) (009CC154) and 009CAC80(ECX = block+2E0h, block) (009CC165);
// both draw from the RNG 00BD2F10 inside their own bodies. Seven 00411E70
// registrations into +D4h (009CC171-009CC1E3).
// UNCERTAINTY: the floats pass through x87 FLD/FSTP; an SNaN would be quieted there
// and is copied here. EH unwinding not modelled.
void* strafe_construct_approach_block_009cc020(Cc13Seg67bHost& host, void* block, void* bot,
                                               void* target) {
    bot_approach_strafe_construct_009ca4a0(host, block, ptr_at(bot, 0x50), target);
    host.bot_state_registry_construct_00411e20(ptr(block, 0xD4));
    const float third = at<float>(block, 0x38);
    const float far_ = at<float>(block, 0x40);
    at<std::uint32_t>(block, 0x000) = kStrafeBlockVtable_00d210d0;
    at<std::uint32_t>(block, 0x0D4) = kStrafeRegistryVtable_00d210cc;
    host.bot_state_move_to_construct_009c2ac0(ptr(block, 0xE8), block, target, far_, far_, third);
    bot_state_follow_construct_009c2980(host, ptr(block, 0x124), block, kHundred_00ce3d08);
    bot_state_follow_construct_009c2980(host, ptr(block, 0x1BC), block, kHundred_00ce3d08);
    plain_inline_state(block, 0x254, 0x00D20FE8u);
    at<float>(block, 0x26C) = kEightHundred_00ce3950;
    at<float>(block, 0x270) = kEightHundred_00ce3950;
    at<std::uint8_t>(block, 0x274) = 0;
    plain_inline_state(block, 0x278, 0x00D21020u);
    at<float>(block, 0x290) = kHalf_00ce3800;
    bot_state_strafe_go_away_construct_009cb500(host, ptr(block, 0x298), block);
    bot_state_strafe_attack_run_construct_009cac80(host, ptr(block, 0x2E0), block);
    register_state_names(host, block, 0xD4, kStrafeStateNamePairs,
                         sizeof(kStrafeStateNamePairs) / sizeof(kStrafeStateNamePairs[0]));
    return block;
}

// 009D2DA0 (BSP_BotTaskTorpedo_ConstructStates): __thiscall(ECX = block, bot, target),
// RET 8, EAX = block. Evidence (disk bytes): 009D0380(ECX = block, [bot+50h], target)
// (009D2DCF); 00411E20(ECX = block+138h) (009D2DE2); vtables 00D213B8 / +138h 00D213B4
// (009D2DFC-009D2E02); FCOMI double [00CF3F20] 15.0 against +134h, JBE (15.0 <= +134h
// or unordered) picks +7Ch, otherwise +80h (009D2DE7-009D2E14); 009C2AC0(ECX =
// block+14Ch, block, target, near = far = float(+78h + +74h), third = the pick)
// (009D2E19-009D2E57); 009C2980(block, 100.0) at +188h (009D2E72) and at +220h, the
// latter then given vtable 00D21320 and +2B8h = -1.0 (009D2E8F-009D2EA3); 009D06D0(ECX
// = block+2BCh, block) (009D2EB7). Embedded goaway +2E0h (00D212E0): +308h = 100.0,
// +314h = +310h = 0.0, +300h = +2FCh = [00D05804] 120.0, +2F8h = 0.0 (009D2EBC-
// 009D2F04). Embedded aim +318h (00D212FC): +334h = 0.5, +330h = [block+4], +338h =
// 1.0, bytes +33Ch/+33Dh = 0, +340h = 0.0 (009D2F09-009D2F51). 009C2980(block, 100.0)
// at +348h, then vtable 00D21320 and +3E0h = -1.0 (009D2F6C-009D2F83). Seven 00411E70
// registrations into +138h (009D2F92-009D3004).
// UNCERTAINTY: the sum is rounded once at 24-bit precision in the image (modelled);
// at 53 bits it would round to double then to float. EH unwinding not modelled.
void* torpedo_construct_approach_block_009d2da0(Cc13Seg67bHost& host, void* block, void* bot,
                                                void* target) {
    bot_approach_torpedo_construct_009d0380(host, block, ptr_at(bot, 0x50), target);
    host.bot_state_registry_construct_00411e20(ptr(block, 0x138));
    const float elapsed = at<float>(block, 0x134);
    at<std::uint32_t>(block, 0x000) = kTorpedoBlockVtable_00d213b8;
    at<std::uint32_t>(block, 0x138) = kTorpedoRegistryVtable_00d213b4;
    // JBE after FCOMI(15.0, +134h): taken when 15.0 <= +134h or the compare is unordered.
    const bool above = kFifteen_00cf3f20 > static_cast<double>(elapsed);
    const float third = above ? at<float>(block, 0x80) : at<float>(block, 0x7C);
    const float sum = at<float>(block, 0x78) + at<float>(block, 0x74);
    host.bot_state_move_to_construct_009c2ac0(ptr(block, 0x14C), block, target, sum, sum, third);
    bot_state_follow_construct_009c2980(host, ptr(block, 0x188), block, kHundred_00ce3d08);
    bot_state_follow_construct_009c2980(host, ptr(block, 0x220), block, kHundred_00ce3d08);
    at<std::uint32_t>(block, 0x220) = kTorpedoDonePrepareVtable_00d21320;
    at<float>(block, 0x2B8) = kMinusOne_00d7a260_f;
    bot_state_torpedo_attack_run_construct_009d06d0(host, ptr(block, 0x2BC), block);
    plain_inline_state(block, 0x2E0, 0x00D212E0u);
    at<float>(block, 0x308) = kHundred_00ce3d08;
    at<float>(block, 0x314) = kZero_s15;
    at<float>(block, 0x310) = kZero_s15;
    at<float>(block, 0x300) = kOneHundredTwenty_00d05804;
    at<float>(block, 0x2FC) = kOneHundredTwenty_00d05804;
    at<float>(block, 0x2F8) = kZero_s15;
    plain_inline_state(block, 0x318, 0x00D212FCu);
    at<float>(block, 0x334) = kHalf_00ce3800;
    at<std::uint32_t>(block, 0x330) = at<std::uint32_t>(block, 0x004);
    at<float>(block, 0x338) = kOne_00d7a24c;
    at<std::uint8_t>(block, 0x33C) = 0;
    at<std::uint8_t>(block, 0x33D) = 0;
    at<float>(block, 0x340) = kZero_s15;
    bot_state_follow_construct_009c2980(host, ptr(block, 0x348), block, kHundred_00ce3d08);
    at<std::uint32_t>(block, 0x348) = kTorpedoDonePrepareVtable_00d21320;
    at<float>(block, 0x3E0) = kMinusOne_00d7a260_f;
    register_state_names(host, block, 0x138, kTorpedoStateNamePairs,
                         sizeof(kTorpedoStateNamePairs) / sizeof(kTorpedoStateNamePairs[0]));
    return block;
}

// 009CF710 (BSP_TakeoffTask_ConstructStates): __thiscall(ECX = block, arg), RET 4, EAX =
// block. Evidence (disk bytes): 009CD420(ECX = block, arg) (009CF735); 00411E20(ECX =
// block+3Ch) (009CF745); vtables 00D21210 / +3Ch 00D2120C (009CF750-009CF756).
// Embedded prepare +50h (00D2116C): bytes +68h/+69h = 0, +6Ch = +70h = +74h = 0.0
// (009CF74A-009CF781). Embedded Takeoff +78h (00D211A4) (009CF786-009CF798). Embedded
// SlowTakeoff +98h (00D21188) (009CF79F-009CF7BD). Embedded parking +C0h (00D21150):
// byte +D8h = 0 (009CF7C7-009CF7E2). Four 00411E70 registrations into +3Ch
// (009CF7E9-009CF822).
// UNCERTAINTY: EH unwinding not modelled.
void* takeoff_construct_approach_block_009cf710(Cc13Seg67bHost& host, void* block, void* arg) {
    bot_approach_takeoff_construct_009cd420(host, block, arg);
    host.bot_state_registry_construct_00411e20(ptr(block, 0x3C));
    at<std::uint32_t>(block, 0x00) = kTakeoffBlockVtable_00d21210;
    at<std::uint32_t>(block, 0x3C) = kTakeoffRegistryVtable_00d2120c;
    plain_inline_state(block, 0x50, 0x00D2116Cu);
    at<std::uint8_t>(block, 0x68) = 0;
    at<std::uint8_t>(block, 0x69) = 0;
    at<float>(block, 0x6C) = kZero_s15;
    at<float>(block, 0x70) = kZero_s15;
    at<float>(block, 0x74) = kZero_s15;
    plain_inline_state(block, 0x78, 0x00D211A4u);
    plain_inline_state(block, 0x98, 0x00D21188u);
    plain_inline_state(block, 0xC0, 0x00D21150u);
    at<std::uint8_t>(block, 0xD8) = 0;
    register_state_names(host, block, 0x3C, kTakeoffStateNamePairs,
                         sizeof(kTakeoffStateNamePairs) / sizeof(kTakeoffStateNamePairs[0]));
    return block;
}

// ---------------------------------------------------------------------------
// The strafe goaway/attackrun and torpedo attackrun state constructors, and the takeoff
// approach base.

namespace {
constexpr std::uint32_t kStrafeGoAwayVtable_00d2103c = 0x00D2103Cu;
constexpr std::uint32_t kStrafeAttackRunVtable_00d21004 = 0x00D21004u;
constexpr std::uint32_t kTorpedoAttackRunVtable_00d212c4 = 0x00D212C4u;
constexpr std::uint32_t kTakeoffApproachVtable_00d21148 = 0x00D21148u;
constexpr float kTwo_00ce3958 = 2.0f;                 // 40000000
constexpr float kPointThree_00ce69c8 = 0.3f;          // 3E99999A
constexpr float kPointNine_00ce3860 = 0.9f;           // 3F666666
constexpr float kSpeedReference_00ce4c04 = 9999.0f;   // 461C3C00
constexpr float kFifteen_00ce5380 = 15.0f;            // 41700000
constexpr float kTwentyFive_00ce89cc = 25.0f;         // 41C80000
}  // namespace

// 009CB500 (BSP_BotStateStrafeGoAway_Construct): __thiscall(ECX = state, owner block),
// RET 4, EAX = state. Evidence (disk bytes): +4h = block, +0Ch/+10h/+14h/+8h = 0
// (009CB51D-009CB52F); vtable 00D2103C (009CB547); byte +24h = 0, +28h = [00CE3958]
// 2.0, +2Ch = [00CE3850] 5.0 (009CB54F-009CB569); +30h = 00BD2F10(1, 0.0, 5.0)
// (009CB56E-009CB573); +34h = 2.0, +38h = 5.0, +3Ch = 00BD2F10(1, 0.0, 5.0) (009CB576-
// 009CB5AC); +40h = [00D7A24C] 1.0, +44h = -00BD2F10(1, 0.0, 1.0) (009CB5AF-009CB5D6);
// +18h = 00BD2F10(1, [00CE69C8] 0.3, [00CE3860] 0.9) (009CB5D9-009CB5F9).
// UNCERTAINTY: the RNG result leaves in ST0 and is rounded to float by FSTP; the host
// hook returns a float. EH unwinding not modelled.
// cc9 models this inline (per-name RNG stand-ins, not a named body) in
// GameUnitsHost::motion_step_00825f20 (src/game_hosts_units.cpp), the strafe install
// st_install_009cc230, block "goaway 009CB500".
void* bot_state_strafe_go_away_construct_009cb500(Cc13Seg67bHost& host, void* state,
                                                  void* block) {
    construct_state_base(state, block);
    at<std::uint32_t>(state, 0x00) = kStrafeGoAwayVtable_00d2103c;
    at<std::uint8_t>(state, 0x24) = 0;
    at<float>(state, 0x28) = kTwo_00ce3958;
    at<float>(state, 0x2C) = kFive_00ce3850;
    at<float>(state, 0x30) = host.random_uniform_00bd2f10(1, 0.0f, kFive_00ce3850);
    at<float>(state, 0x34) = kTwo_00ce3958;
    at<float>(state, 0x38) = kFive_00ce3850;
    at<float>(state, 0x3C) = host.random_uniform_00bd2f10(1, 0.0f, kFive_00ce3850);
    at<float>(state, 0x40) = kOne_00d7a24c;
    at<float>(state, 0x44) = -host.random_uniform_00bd2f10(1, 0.0f, 1.0f);
    at<float>(state, 0x18) =
        host.random_uniform_00bd2f10(1, kPointThree_00ce69c8, kPointNine_00ce3860);
    return state;
}

// 009CAC80 (BSP_BotStateStrafeAttackRun_Construct) and 009D06D0
// (BSP_BotStateTorpedoAttackRun_Construct): __thiscall(ECX = state, owner block), RET 4,
// EAX = state. Evidence (disk bytes): +4h = block, +0Ch/+10h/+14h/+8h = 0 (009CAC9D-
// 009CACAF / 009D06ED-009D06FF); vtable 00D21004 (009CACC3) / 00D212C4 (009D0713); +18h
// = [00D7A24C] 1.0 (009CACD5 / 009D0725); +1Ch = -00BD2F10(1, 0.0, 1.0) (009CACDA-
// 009CACE4 / 009D072A-009D0734); +20h = 0.0 (009CACEB / 009D073B).
// UNCERTAINTY: as 009CB500 for the ST0 result. EH unwinding not modelled.
void* bot_state_strafe_attack_run_construct_009cac80(Cc13Seg67bHost& host, void* state,
                                                     void* block) {
    return construct_timed_state(host, state, block, kStrafeAttackRunVtable_00d21004, 0x18,
                                 0x1C, 0x20);
}

void* bot_state_torpedo_attack_run_construct_009d06d0(Cc13Seg67bHost& host, void* state,
                                                      void* block) {
    return construct_timed_state(host, state, block, kTorpedoAttackRunVtable_00d212c4, 0x18,
                                 0x1C, 0x20);
}

// 009CD420 (BSP_BotApproachTakeoff_Construct): __thiscall(ECX = approach, plane), RET 4,
// EAX = approach. Evidence (disk bytes): 009F9CE0(ECX = approach, plane, [00CE4C04]
// 9999.0) (009CD420-009CD433); vtable 00D21148, +34h = 0.0 (009CD43D-009CD443); when
// plane is non-null and plane+BF4h is non-zero: 00414DB0(plane) when byte plane+C8h is
// 0, then +34h = plane+100h (009CD448-009CD469); +30h = [00CE5380] 15.0 (009CD46C);
// vtable[5Ch](13h) -> +30h = 15.0 (009CD479-009CD490); vtable[5Ch](10h) -> +30h =
// [00CE89CC] 25.0 (009CD495-009CD4AC); +2Ch = float(+30h + +34h) (009CD4B1-009CD4BA).
// The vtable calls at 009CD479 dereference the plane without the null test.
// UNCERTAINTY: the sum is rounded once at 24-bit precision in the image (modelled).
// cc9 models this inline in GameUnitsHost::Impl::install_takeoff_task_0099a4a0
// (src/game_hosts_units.cpp), not as a named body.
void* bot_approach_takeoff_construct_009cd420(Cc13Seg67bHost& host, void* approach,
                                              void* plane) {
    host.bot_approach_construct_speed_reference_009f9ce0(approach, plane,
                                                         kSpeedReference_00ce4c04);
    at<std::uint32_t>(approach, 0x00) = kTakeoffApproachVtable_00d21148;
    at<float>(approach, 0x34) = kZero_s15;
    if (plane != nullptr && at<std::uint32_t>(plane, 0xBF4) != 0) {
        if (at<std::uint8_t>(plane, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(plane);
        at<float>(approach, 0x34) = at<float>(plane, 0x100);
    }
    at<float>(approach, 0x30) = kFifteen_00ce5380;
    if (host.object_vcall_5c(plane, 0x13)) at<float>(approach, 0x30) = kFifteen_00ce5380;
    if (host.object_vcall_5c(plane, 0x10)) at<float>(approach, 0x30) = kTwentyFive_00ce89cc;
    at<float>(approach, 0x2C) = at<float>(approach, 0x30) + at<float>(approach, 0x34);
    return approach;
}

// ---------------------------------------------------------------------------
// The strafe approach base.

namespace {
constexpr std::uint32_t kStrafeApproachVtable_00d20fc8 = 0x00D20FC8u;
constexpr float kOnePointZeroFive_00ce780c = 1.05f;      // 3F866666
constexpr float kOnePointOne_00ce6448 = 1.1f;            // 3F8CCCCD
constexpr double kPointEight_00ce3d40 = 0.800000011920929;   // 3FE99999A0000000
constexpr double kOnePointFour_00d045f0 = 1.399999976158142;  // 3FF6666660000000
constexpr float kThirtySixHundred_00cfdeb0 = 3600.0f;    // 45610000
}  // namespace

// 009CA4A0 (BSP_BotApproachStrafe_Construct): __thiscall(ECX = approach, arg, target),
// RET 8, EAX = approach. Evidence (disk bytes): 009F9CE0(ECX = approach, arg, tuning
// +65Ch) (009CA4C0-009CA4D6); vtable 00D20FC8 (009CA4F6). With u = [approach+14h]
// reloaded before each use: A = 00BD2F10(1, [00CE3860] 0.9, [00CE780C] 1.05) x u+DCh
// (009CA4DB-009CA50A); B = float(tuning+658h x double [00CE3D40] 0.8) (009CA50E-
// 009CA51F); FCOMI then JBE keeps A when A <= B or unordered, else B (009CA523-
// 009CA53F); +34h = that x +24h (009CA548-009CA557); +38h = RNG(1, 0.9, [00CE6448]
// 1.1) x u+230h x +24h (009CA55A-009CA586); +44h = [00CFDEB0] 3600.0, byte +50h = 0,
// +3Ch = RNG(1, 0.9, 1.1) x u+D8h x +24h, +54h = 0 (009CA589-009CA5C6); 009FB200(ECX
// = approach+58h, target) (009CA5CD); +CCh = 1.0, +D0h = -RNG(1, 0, 1) (009CA5D2-
// 009CA604); +4Ch = 007C2610(ECX = [approach+4]) (009CA60A-009CA612); +54h = target
// when target is non-null and passes vtable[5Ch](41h), else 0 (009CA615-009CA62C);
// 009CA3B0(ECX = approach) (009CA62F); when [approach+4] passes vtable[5Ch](10h) or
// (16h): +34h and +3Ch scaled by double [00D045F0] 1.4 (009CA634-009CA668).
// UNCERTAINTY: 24-bit precision modelled (each FMUL of a chain rounds to float; at 53
// bits a chain would round once at the end). The double constants are float-exact, so
// their products are exact in double and round once. The RNG results are rounded from
// ST0 by the hook. EH unwinding not modelled.
// cc9 models this inline (per-name RNG stand-ins, not a named body) in
// GameUnitsHost::motion_step_00825f20 (src/game_hosts_units.cpp), the strafe install
// st_install_009cc230 (009CA4DB-009CA668).
void* bot_approach_strafe_construct_009ca4a0(Cc13Seg67bHost& host, void* approach, void* arg,
                                             void* target) {
    const float speed = at<float>(host.game_tuning_singleton_0042e740(), 0x65C);
    host.bot_approach_construct_speed_reference_009f9ce0(approach, arg, speed);
    void* u = ptr_at(approach, 0x14);
    at<std::uint32_t>(approach, 0x00) = kStrafeApproachVtable_00d20fc8;
    const float a =
        host.random_uniform_00bd2f10(1, kPointNine_00ce3860, kOnePointZeroFive_00ce780c) *
        at<float>(u, 0xDC);
    const float b = static_cast<float>(
        static_cast<double>(at<float>(host.game_tuning_singleton_0042e740(), 0x658)) *
        kPointEight_00ce3d40);
    // JBE after FCOMI(A, B): A is kept when A <= B or the compare is unordered.
    const float m = a > b ? b : a;
    at<float>(approach, 0x34) = m * at<float>(approach, 0x24);
    u = ptr_at(approach, 0x14);
    const float r2 = host.random_uniform_00bd2f10(1, kPointNine_00ce3860, kOnePointOne_00ce6448);
    at<float>(approach, 0x38) = r2 * at<float>(u, 0x230) * at<float>(approach, 0x24);
    u = ptr_at(approach, 0x14);
    const float r3 = host.random_uniform_00bd2f10(1, kPointNine_00ce3860, kOnePointOne_00ce6448);
    at<float>(approach, 0x44) = kThirtySixHundred_00cfdeb0;
    at<std::uint8_t>(approach, 0x50) = 0;
    at<float>(approach, 0x3C) = r3 * at<float>(u, 0xD8) * at<float>(approach, 0x24);
    at<std::uint32_t>(approach, 0x54) = 0;
    host.approach_target_ref_construct_009fb200(ptr(approach, 0x58), target);
    at<float>(approach, 0xCC) = kOne_00d7a24c;
    at<float>(approach, 0xD0) = -host.random_uniform_00bd2f10(1, 0.0f, 1.0f);
    at<float>(approach, 0x4C) =
        host.unit_min_kind21_component_speed_007c2610(ptr_at(approach, 0x4));
    const bool keep = target != nullptr && host.object_vcall_5c(target, 0x41);
    at<std::uintptr_t>(approach, 0x54) = keep ? reinterpret_cast<std::uintptr_t>(target) : 0;
    host.bot_approach_strafe_seed_glide_009ca3b0(approach);
    void* owner = ptr_at(approach, 0x4);
    if (host.object_vcall_5c(owner, 0x10) || host.object_vcall_5c(owner, 0x16)) {
        at<float>(approach, 0x34) = static_cast<float>(
            static_cast<double>(at<float>(approach, 0x34)) * kOnePointFour_00d045f0);
        at<float>(approach, 0x3C) = static_cast<float>(
            kOnePointFour_00d045f0 * static_cast<double>(at<float>(approach, 0x3C)));
    }
    return approach;
}

// ---------------------------------------------------------------------------
// The torpedo approach base (reset).

namespace {
constexpr std::uint32_t kTorpedoApproachVtable_00d212a4 = 0x00D212A4u;
constexpr double kPointSevenFive_00cec9d8 = 0.75;
constexpr float kSixtySeven_00d212a0 = 67.0f;           // 42860000
constexpr float kQuarter_00ce3868 = 0.25f;              // 3E800000
constexpr float kOnePointTwoFive_00cf29a8 = 1.25f;      // 3FA00000
constexpr float kOnePointFive_00ce380c = 1.5f;          // 3FC00000
constexpr float kTen_00ce38b8 = 10.0f;                  // 41200000
constexpr float kNineNines_00cf87d0 = 999999.0f;        // 497423F0
constexpr double kOnePointThree_00d21298 = 1.2999999523162842;  // 3FF4CCCCC0000000
constexpr float kMinusPointOne_00ce3cb4 = -0.1f;        // BDCCCCCD
}  // namespace

// 009D0380 (BSP_BotApproachTorpedo_Reset): __thiscall(ECX = approach, arg, target),
// RET 8, EAX = approach. Evidence (disk bytes): 009F9CE0(ECX = approach, arg, tuning
// +440h) (009D03A1-009D03B7); vtable 00D212A4, +2Ch = 0, +58h = 24h, +5Ch = 0.0
// (009D03C4-009D03D4); L = float(007BCE20([approach+4]) x double [00CEC9D8] 0.75)
// (009D03D9-009D03E7); FCOMI then JBE keeps L when L <= desc+18Ch or unordered, else
// desc+18Ch (desc = [approach+8]) (009D03EB-009D040B); +70h = 00BD2F10(1, 0.9, 1.1) x
// that (009D0411-009D0449); +74h = [00D212A0] 67.0, +78h = RNG(1, 0, [00CE3868] 0.25)
// x rec+0h (rec = [approach+14h]) (009D043B-009D0475); +7Ch = rec+4h x +24h, +80h =
// +24h x rec+8h, +84h = rec+0Ch (009D047D-009D04A0); seed = float(RNG(1, [00CF29A8]
// 1.25, [00CE380C] 1.5) x desc+268h + +80h), +94h = +9Ch = 0.0, +A0h = [00CE38B8] 10.0,
// +88h = seed, +A4h = 0FFh, bytes +A8h/+A9h = 0, +AAh = 1, +8Ch = +90h = float(seed x
// double [00D21298] 1.3), +ACh = +B0h = [00CF87D0] 999999.0 (009D04A6-009D054D);
// 009FB200(ECX = approach+B4h, target) (009D0555); +128h = 1.0, +12Ch = -RNG(1, 0, 1)
// (009D055A-009D0590); bytes +130h/+131h = 0, +134h = 3600.0 (009D05AB-009D05C1); with
// N = +7Ch and F = +80h saved before the draws: +7Ch = N + RNG(1, [00CE3CB4] -0.1, 0.5)
// x (F - N) (009D0596-009D05ED, FSUBP DE EA = F - N); +80h = F - RNG(1, -0.1, 0.5) x
// (F - new +7Ch) (009D05F0-009D0625, FSUBRP DE E2 = F - N'); 009D02A0(ECX = approach)
// (009D062B); 009D0160(ECX = approach) (009D0632); bytes +30h..+54h = 1 (009D063B-
// 009D065B).
// The fragment helpers in src/torpedo_approach_update.cpp model these steps at 53-bit
// precision and take desc+18Ch on an unordered compare; this body follows the image's
// JBE and models 24-bit precision.
// UNCERTAINTY: 24-bit precision modelled (at 53 bits the product-plus-sum and the leg
// expressions would round once at the end). The double constants are float-exact, so
// their products are exact in double and round once. RNG results rounded from ST0 by
// the hook. EH unwinding not modelled.
// cc9 models the draws inline (per-name RNG stand-ins, not a named body) in
// run_torpedo_task_arm_009d4850 (src/game_hosts_units.cpp), under the switches
// kTorpedoResetDrawsBound (009D03D9-009D0475, 009D0581-009D0625) and
// kTorpedoResetEngageDrawsBound (009D04A6-009D053F).
void* bot_approach_torpedo_construct_009d0380(Cc13Seg67bHost& host, void* approach, void* arg,
                                              void* target) {
    const float speed = at<float>(host.game_tuning_singleton_0042e740(), 0x440);
    host.bot_approach_construct_speed_reference_009f9ce0(approach, arg, speed);
    at<std::uint32_t>(approach, 0x00) = kTorpedoApproachVtable_00d212a4;
    at<std::uint32_t>(approach, 0x2C) = 0;
    at<std::uint32_t>(approach, 0x58) = 0x24;
    at<float>(approach, 0x5C) = kZero_s15;
    const float limit = static_cast<float>(
        static_cast<double>(host.unit_min_torpedo_hit_velocity_007bce20(ptr_at(approach, 0x4))) *
        kPointSevenFive_00cec9d8);
    const float travel = at<float>(ptr_at(approach, 0x8), 0x18C);
    // JBE after FCOMI(L, travel): L is kept when L <= travel or the compare is unordered.
    const float cap = limit > travel ? travel : limit;
    at<float>(approach, 0x70) =
        host.random_uniform_00bd2f10(1, kPointNine_00ce3860, kOnePointOne_00ce6448) * cap;
    void* rec = ptr_at(approach, 0x14);
    at<float>(approach, 0x74) = kSixtySeven_00d212a0;
    at<float>(approach, 0x78) =
        host.random_uniform_00bd2f10(1, 0.0f, kQuarter_00ce3868) * at<float>(rec, 0x0);
    rec = ptr_at(approach, 0x14);
    void* desc = ptr_at(approach, 0x8);
    const float scale = at<float>(approach, 0x24);
    at<float>(approach, 0x7C) = at<float>(rec, 0x4) * scale;
    at<float>(approach, 0x80) = scale * at<float>(rec, 0x8);
    at<float>(approach, 0x84) = at<float>(rec, 0xC);
    const float seed =
        host.random_uniform_00bd2f10(1, kOnePointTwoFive_00cf29a8, kOnePointFive_00ce380c) *
            at<float>(desc, 0x268) +
        at<float>(approach, 0x80);
    at<float>(approach, 0x94) = kZero_s15;
    at<float>(approach, 0x9C) = kZero_s15;
    at<float>(approach, 0xA0) = kTen_00ce38b8;
    at<std::uint32_t>(approach, 0xA4) = 0xFF;
    at<float>(approach, 0x88) = seed;
    at<std::uint8_t>(approach, 0xA8) = 0;
    at<std::uint8_t>(approach, 0xA9) = 0;
    at<std::uint8_t>(approach, 0xAA) = 1;
    const float engage =
        static_cast<float>(static_cast<double>(seed) * kOnePointThree_00d21298);
    at<float>(approach, 0x8C) = engage;
    at<float>(approach, 0x90) = engage;
    at<float>(approach, 0xAC) = kNineNines_00cf87d0;
    at<float>(approach, 0xB0) = kNineNines_00cf87d0;
    host.approach_target_ref_construct_009fb200(ptr(approach, 0xB4), target);
    at<float>(approach, 0x128) = kOne_00d7a24c;
    at<float>(approach, 0x12C) = -host.random_uniform_00bd2f10(1, 0.0f, 1.0f);
    const float n = at<float>(approach, 0x7C);
    at<std::uint8_t>(approach, 0x130) = 0;
    at<std::uint8_t>(approach, 0x131) = 0;
    at<float>(approach, 0x134) = kThirtySixHundred_00cfdeb0;
    const float d1 = host.random_uniform_00bd2f10(1, kMinusPointOne_00ce3cb4, kHalf_00ce3800);
    const float f = at<float>(approach, 0x80);
    at<float>(approach, 0x7C) = n + d1 * (f - n);
    const float f2 = at<float>(approach, 0x80);
    const float d2 = host.random_uniform_00bd2f10(1, kMinusPointOne_00ce3cb4, kHalf_00ce3800);
    at<float>(approach, 0x80) = f2 - d2 * (f2 - at<float>(approach, 0x7C));
    host.approach_torpedo_aim_error_draw_009d02a0(approach);
    host.approach_torpedo_reset_run_time_009d0160(approach);
    for (std::size_t off = 0x30; off <= 0x54; ++off) at<std::uint8_t>(approach, off) = 1;
    return approach;
}

// ---------------------------------------------------------------------------
// The dive-bomb approach base.

namespace {
constexpr std::uint32_t kDiveBombApproachVtable_00d20c48 = 0x00D20C48u;
constexpr double kPointNineFive_00ceffb0 = 0.949999988079071;  // 3FEE666660000000
constexpr float kPointEight_00ce74f8 = 0.8f;                   // 3F4CCCCD
constexpr float kOnePointSix_00d06bb4 = 1.6f;                  // 3FCCCCCD
constexpr float kOnePointEight_00cf4848 = 1.8f;                // 3FE66666
constexpr double kHalf_00d7a280 = 0.5;
constexpr double kTwoHundredFifty_00cf8850 = 250.0;
}  // namespace

// 009C3EA0 (BSP_DiveBombApproach_Construct): __thiscall(ECX = approach, plane, target),
// RET 8, EAX = approach. Evidence (disk bytes): 009F9CE0(ECX = approach, plane, tuning
// +4D8h) (009C3EBF-009C3ED5); vtable 00D20C48, +2Ch = 0 (009C3EE2-009C3EE8);
// 009FB200(ECX = approach+30h, target) (009C3EEF); with desc = [approach+8] and rec =
// [approach+14h]: +A4h = float(desc+188h x double [00CEFFB0] 0.95) (009C3EF4-009C3F16);
// +A8h = 00BD2F10(1, rec+38h, rec+3Ch) (009C3F1C-009C3F2E); +ACh = tuning+4CCh
// (009C3F34-009C3F3F); +B0h = tuning+4D0h - tuning+4CCh (009C3F45-009C3F68); +B4h =
// RNG(1, [00CE3D30] 0.6, [00CE74F8] 0.8) x desc+268h (009C3F6E-009C3F97); T = RNG(1,
// [00D06BB4] 1.6, [00CF4848] 1.8) x desc+268h, +C0h = 0.0, +C4h = 3600.0, +CCh = 0FFh,
// +B8h = +BCh = T, byte +D0h = 0 (009C3F9D-009C3FF5); +D4h = the larger of float(+A8h
// + 250.0) and float((+ACh + +A8h) x 0.5), the mean on a tie or an unordered compare
// (009C3FFB-009C4045, FCOMI then JBE); 00414DB0(plane) when byte plane+C8h is 0
// (009C404D-009C4058); +D8h/+DCh/+E0h = plane+FCh/+100h/+104h (009C405D-009C407D);
// 009C3DA0(ECX = approach) (009C4083).
// src/dive_bomb_task.cpp's dive_bomb_dive_entry_height_009c4045 has the +D4h step at
// 53-bit precision but is bsp_game-only, so it is not called here.
// UNCERTAINTY: 24-bit precision modelled. The double constants are float-exact, so their
// products and the 250.0 sum are exact in double and round once, as at 24 bits. RNG
// results rounded from ST0 by the hook. The plane is dereferenced without a null test,
// as in the image. EH unwinding not modelled.
// cc9 models the +A8h draw inline (per-name RNG stand-in, not a named body) in
// run_dive_bomb_task_arm_009c8790 (src/game_hosts_units.cpp), under the switch
// kReleaseAltitudeDrawBound (packet cc8_dive_race).
void* bot_approach_dive_bomb_construct_009c3ea0(Cc13Seg67bHost& host, void* approach,
                                                void* plane, void* target) {
    const float speed = at<float>(host.game_tuning_singleton_0042e740(), 0x4D8);
    host.bot_approach_construct_speed_reference_009f9ce0(approach, plane, speed);
    at<std::uint32_t>(approach, 0x00) = kDiveBombApproachVtable_00d20c48;
    at<std::uint32_t>(approach, 0x2C) = 0;
    host.approach_target_ref_construct_009fb200(ptr(approach, 0x30), target);
    void* desc = ptr_at(approach, 0x8);
    void* rec = ptr_at(approach, 0x14);
    at<float>(approach, 0xA4) = static_cast<float>(
        static_cast<double>(at<float>(desc, 0x188)) * kPointNineFive_00ceffb0);
    at<float>(approach, 0xA8) =
        host.random_uniform_00bd2f10(1, at<float>(rec, 0x38), at<float>(rec, 0x3C));
    at<float>(approach, 0xAC) = at<float>(host.game_tuning_singleton_0042e740(), 0x4CC);
    void* tuning_a = host.game_tuning_singleton_0042e740();
    void* tuning_b = host.game_tuning_singleton_0042e740();
    at<float>(approach, 0xB0) = at<float>(tuning_a, 0x4D0) - at<float>(tuning_b, 0x4CC);
    desc = ptr_at(approach, 0x8);
    at<float>(approach, 0xB4) =
        host.random_uniform_00bd2f10(1, kPointSix_00ce3d30, kPointEight_00ce74f8) *
        at<float>(desc, 0x268);
    desc = ptr_at(approach, 0x8);
    const float t =
        host.random_uniform_00bd2f10(1, kOnePointSix_00d06bb4, kOnePointEight_00cf4848) *
        at<float>(desc, 0x268);
    at<float>(approach, 0xC0) = kZero_s15;
    at<float>(approach, 0xC4) = kThirtySixHundred_00cfdeb0;
    at<std::uint32_t>(approach, 0xCC) = 0xFF;
    at<float>(approach, 0xB8) = t;
    at<std::uint8_t>(approach, 0xD0) = 0;
    at<float>(approach, 0xBC) = t;
    const float a8 = at<float>(approach, 0xA8);
    const float mean =
        static_cast<float>(static_cast<double>(at<float>(approach, 0xAC) + a8) * kHalf_00d7a280);
    const float margin = static_cast<float>(static_cast<double>(a8) + kTwoHundredFifty_00cf8850);
    // JBE after FCOMI(margin, mean): the mean is kept on a tie or an unordered compare.
    at<float>(approach, 0xD4) = margin > mean ? margin : mean;
    if (at<std::uint8_t>(plane, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(plane);
    at<float>(approach, 0xD8) = at<float>(plane, 0xFC);
    at<float>(approach, 0xDC) = at<float>(plane, 0x100);
    at<float>(approach, 0xE0) = at<float>(plane, 0x104);
    host.approach_dive_bomb_aim_error_draw_009c3da0(approach);
    return approach;
}

// ---------------------------------------------------------------------------
// The level-bomb approach base.

namespace {
constexpr std::uint32_t kLevelBombApproachVtable_00d200a8 = 0x00D200A8u;
constexpr float kSixteen_00ce6454 = 16.0f;               // 41800000
constexpr float kPointSixFive_00d07fc4 = 0.65f;          // 3F266666
constexpr float kTwelveHundred_00cfd714 = 1200.0f;       // 44960000
}  // namespace

// 009B44F0 (BSP_BotApproachLevelBomb_ConstructBase): __thiscall(ECX = approach, plane,
// target), RET 8, EAX = approach. Evidence (disk bytes): 009F9CE0(ECX = approach, plane,
// tuning+458h) (009B4512-009B4528); vtable 00D200A8, +2Ch = 0 (009B4537-009B453D);
// 009FB200(ECX = approach+30h, target) (009B4540); +A4h = [00CE6454] 16.0 (009B456A);
// r = 00BD2F10(1, [00CE69C8] 0.3, [00D07FC4] 0.65) kept as a double (009B4572-
// 009B4577); +A8h = float(007C1FB0(ECX = plane) x r) (009B457B-009B4591); with desc =
// [approach+8]: +ACh = desc+188h, +B0h = [00CFD714] 1200.0 (009B4586-009B45A9); T =
// RNG(1, [00D06BB4] 1.6, [00CF4848] 1.8) x desc+268h, +C0h = 0.0, +C8h = 0FFh, bytes
// +CCh/+CDh = 0, +B4h = T, byte +CEh = 1, +B8h = +BCh = float(T x double [00D045F0]
// 1.4) (009B45B1-009B4619); 00414DB0(plane) when byte plane+C8h is 0 (009B461F-
// 009B4629); +D0h/+D4h/+D8h = plane+FCh/+100h/+104h (009B462E-009B464E); 009B4400(ECX =
// approach) (009B4654).
// UNCERTAINTY: 24-bit precision modelled; the products of float-exact operands are exact
// in double and round once. The RNG and 007C1FB0 results are rounded from ST0 by the
// hooks. The plane is dereferenced without a null test, as in the image. EH unwinding
// not modelled.
// cc9 models this inline (per-name RNG stand-ins, not a named body) in
// GameUnitsHost::motion_step_00825f20 (src/game_hosts_units.cpp), the level-bomb
// install block "009B7990 -> 009B75E0 -> 009B44F0" (009B451D-009B4591).
void* bot_approach_level_bomb_construct_base_009b44f0(Cc13Seg67bHost& host, void* approach,
                                                      void* plane, void* target) {
    const float speed = at<float>(host.game_tuning_singleton_0042e740(), 0x458);
    host.bot_approach_construct_speed_reference_009f9ce0(approach, plane, speed);
    at<std::uint32_t>(approach, 0x00) = kLevelBombApproachVtable_00d200a8;
    at<std::uint32_t>(approach, 0x2C) = 0;
    host.approach_target_ref_construct_009fb200(ptr(approach, 0x30), target);
    at<float>(approach, 0xA4) = kSixteen_00ce6454;
    const double r =
        host.random_uniform_00bd2f10(1, kPointThree_00ce69c8, kPointSixFive_00d07fc4);
    at<float>(approach, 0xA8) = static_cast<float>(
        static_cast<double>(host.unit_longest_rack_stick_time_007c1fb0(plane)) * r);
    void* desc = ptr_at(approach, 0x8);
    at<float>(approach, 0xAC) = at<float>(desc, 0x188);
    at<float>(approach, 0xB0) = kTwelveHundred_00cfd714;
    const float t =
        host.random_uniform_00bd2f10(1, kOnePointSix_00d06bb4, kOnePointEight_00cf4848) *
        at<float>(desc, 0x268);
    at<float>(approach, 0xC0) = kZero_s15;
    at<std::uint32_t>(approach, 0xC8) = 0xFF;
    at<std::uint8_t>(approach, 0xCC) = 0;
    at<std::uint8_t>(approach, 0xCD) = 0;
    at<float>(approach, 0xB4) = t;
    at<std::uint8_t>(approach, 0xCE) = 1;
    const float u = static_cast<float>(static_cast<double>(t) * kOnePointFour_00d045f0);
    at<float>(approach, 0xB8) = u;
    at<float>(approach, 0xBC) = u;
    if (at<std::uint8_t>(plane, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(plane);
    at<float>(approach, 0xD0) = at<float>(plane, 0xFC);
    at<float>(approach, 0xD4) = at<float>(plane, 0x100);
    at<float>(approach, 0xD8) = at<float>(plane, 0x104);
    host.approach_level_bomb_aim_error_draw_009b4400(approach);
    return approach;
}

// ---------------------------------------------------------------------------
// The dogfight approach base.

namespace {
constexpr std::uint32_t kDogfightApproachVtable_00d1f844 = 0x00D1F844u;
constexpr std::uint32_t kDogfightEmbeddedA0Vtable_00cf89b4 = 0x00CF89B4u;
constexpr std::uint32_t kDogfightObserverVtable_00cf6514 = 0x00CF6514u;
constexpr float kNinetyNine_00d059a0 = 99.0f;  // 42C60000
}  // namespace

// 009A6C10 (BSP_BotApproachDogfight_Construct): __thiscall(ECX = approach, bot, arg),
// RET 8, EAX = approach. Evidence (disk bytes): 009F9CE0(ECX = approach, [bot+50h],
// tuning+650h) (009A6C32-009A6C4B); vtable 00D1F844 (009A6C54); 009FB200(ECX =
// approach+2Ch, [arg+3D0h]) (009A6C50-009A6C66); +A4h/+A8h/+ACh/+B4h = 0, byte +B0h = 1,
// +A0h = 00CF89B4 (009A6C71-009A6C90); 007F0F80(ECX = approach+B8h, arg) then [+B8h] =
// 00CF6514 (009A6CA4-009A6CB1); +D4h = +D8h = [00CE4C04] 9999.0, +DCh = 0.0, +E0h =
// 1.0, +E4h = -1.0, byte +D0h = 0, +E8h = [00D059A0] 99.0 (009A6CB8-009A6D10);
// 009FA350(ECX = approach+2Ch) (009A6D18).
// UNCERTAINTY: EH unwinding not modelled.
void* bot_approach_dogfight_construct_009a6c10(Cc13Seg67bHost& host, void* approach, void* bot,
                                               void* arg) {
    const float speed = at<float>(host.game_tuning_singleton_0042e740(), 0x650);
    host.bot_approach_construct_speed_reference_009f9ce0(approach, ptr_at(bot, 0x50), speed);
    at<std::uint32_t>(approach, 0x00) = kDogfightApproachVtable_00d1f844;
    host.approach_target_ref_construct_009fb200(ptr(approach, 0x2C), ptr_at(arg, 0x3D0));
    at<std::uint32_t>(approach, 0xA4) = 0;
    at<std::uint32_t>(approach, 0xA8) = 0;
    at<std::uint32_t>(approach, 0xAC) = 0;
    at<std::uint32_t>(approach, 0xB4) = 0;
    at<std::uint8_t>(approach, 0xB0) = 1;
    at<std::uint32_t>(approach, 0xA0) = kDogfightEmbeddedA0Vtable_00cf89b4;
    host.observer_member_initialize_007f0f80(ptr(approach, 0xB8), arg);
    at<std::uint32_t>(approach, 0xB8) = kDogfightObserverVtable_00cf6514;
    at<float>(approach, 0xD4) = kSpeedReference_00ce4c04;
    at<float>(approach, 0xD8) = kSpeedReference_00ce4c04;
    at<float>(approach, 0xDC) = kZero_s15;
    at<float>(approach, 0xE0) = kOne_00d7a24c;
    at<float>(approach, 0xE4) = kMinusOne_00d7a260_f;
    at<std::uint8_t>(approach, 0xD0) = 0;
    at<float>(approach, 0xE8) = kNinetyNine_00d059a0;
    host.approach_target_ref_reset_aim_offset_009fa350(ptr(approach, 0x2C));
    return approach;
}

// ---------------------------------------------------------------------------
// The drop-kamikaze approach base from a stream.

namespace {
constexpr std::uint32_t kDropKamikazeApproachVtable_00d1fb00 = 0x00D1FB00u;
constexpr std::uint32_t kStrTargetLockPlain_00d1fb5c = 0x00D1FB5Cu;   // "targetLock"
constexpr std::uint32_t kStrStartDist_00d1fb50 = 0x00D1FB50u;         // "startDist"
constexpr std::uint32_t kStrAttackDist_00d1fb44 = 0x00D1FB44u;        // "attackDist"
constexpr std::uint32_t kStrTargetDist_00d1fb38 = 0x00D1FB38u;        // "targetDist"
constexpr std::uint32_t kStrInsideAttackRange_00d1fb24 = 0x00D1FB24u; // "insideAttackRange"
constexpr std::uint32_t kStrTargetHdg_00d1fb18 = 0x00D1FB18u;         // "targetHdg"
constexpr std::uint32_t kStrCurrentAvoidFlag_00d1fb04 = 0x00D1FB04u;  // "currentAvoidFlag"
}  // namespace

// 009ABBE0 (BSP_BotApproachDropKamikaze_ConstructBaseFromBag): __thiscall(ECX = approach,
// plane, archive), RET 8, EAX = approach. Evidence (disk bytes): 009F9CE0(ECX = approach,
// plane, tuning+458h) (009ABBFE-009ABC14); vtable 00D1FB00, +2Ch = 0 (009ABC2F-
// 009ABC35); 00BD4FE0(ECX = archive, the outgoing block, {0,"targetLock"}) (009ABC4C);
// 009FB3E0(ECX = approach+30h, that block by value) (009ABC54); +A4h = desc+18Ch (desc =
// [approach+8]), +A8h = [00CFD714] 1200.0 (009ABC59-009ABC73); archive vtable[10h] reads
// {0,"startDist"} <- {2, &+ACh} (009ABCB5), {0,"attackDist"} <- {2, &+B0h} (009ABCEC),
// {0,"targetDist"} <- {2, &+B4h} (009ABD23), {0,"insideAttackRange"} <- {3, &+C0h}
// (009ABD5A), {0,"targetHdg"} <- {2, &+B8h} (009ABD91), {0,"currentAvoidFlag"} <- {1,
// &+BCh} (009ABDC8).
// UNCERTAINTY: EH unwinding not modelled.
void* bot_approach_drop_kamikaze_construct_base_from_bag_009abbe0(Cc13Seg67bHost& host,
                                                                  void* approach, void* plane,
                                                                  void* archive) {
    const float speed = at<float>(host.game_tuning_singleton_0042e740(), 0x458);
    host.bot_approach_construct_speed_reference_009f9ce0(approach, plane, speed);
    at<std::uint32_t>(approach, 0x00) = kDropKamikazeApproachVtable_00d1fb00;
    at<std::uint32_t>(approach, 0x2C) = 0;
    std::uint32_t ref[3] = {};
    host.archive_reader_ref_open_00bd4fe0(archive, ref, name_record(kStrTargetLockPlain_00d1fb5c));
    host.approach_ref_construct_from_stream_009fb3e0(ptr(approach, 0x30), ref);
    at<float>(approach, 0xA4) = at<float>(ptr_at(approach, 0x8), 0x18C);
    at<float>(approach, 0xA8) = kTwelveHundred_00cfd714;
    read_field(host, archive, kStrStartDist_00d1fb50, 2, ptr(approach, 0xAC));
    read_field(host, archive, kStrAttackDist_00d1fb44, 2, ptr(approach, 0xB0));
    read_field(host, archive, kStrTargetDist_00d1fb38, 2, ptr(approach, 0xB4));
    read_field(host, archive, kStrInsideAttackRange_00d1fb24, 3, ptr(approach, 0xC0));
    read_field(host, archive, kStrTargetHdg_00d1fb18, 2, ptr(approach, 0xB8));
    read_field(host, archive, kStrCurrentAvoidFlag_00d1fb04, 1, ptr(approach, 0xBC));
    return approach;
}

// ---------------------------------------------------------------------------
// The land approach base.

namespace {
constexpr std::uint32_t kLandApproachVtable_00d1fdb8 = 0x00D1FDB8u;
}  // namespace

// 009AFE70 (BSP_BotApproachLand_ConstructLower): __thiscall(ECX = approach, plane, air
// ops), RET 8, EAX = approach. Evidence (disk bytes): 009F9CE0(ECX = approach, plane,
// tuning+52Ch) (009AFE74-009AFE8A); vtable 00D1FDB8, +2Ch = air ops, +30h = [air ops
// +7Ch] (009AFE93-009AFE9F); +34h = 006C0B50(ECX = air ops, [plane+9D4h]) (009AFEA2-
// 009AFEB6); byte +44h = 0, +4Ch = +48h = 1.0, +50h = 1 (009AFEB9-009AFEC7); +54h =
// 00BD2F10(1, 0.9, 1.1) x tuning+4E4h (009AFECE-009AFEFB); +58h = RNG(1, 0.9, 1.1) x
// tuning+4E0h (009AFEFE-009AFF39); +60h = 0.0, +64h = [00CE38B8] 10.0, +5Ch =
// float(desc+18Ch x double [00CEC9D8] 0.75) (desc = [approach+8]), +A4h = 0.0, +ACh =
// 0.5 (009AFF2B-009AFF77); +B0h = -RNG(1, 0, 0.5) (009AFF7F-009AFF8B); 00414DB0 on
// [approach+30h] when its byte +C8h is 0, then +38h/+3Ch/+40h = its +FCh/+100h/+104h
// (009AFF91-009AFFBC); 00414DB0 on [approach+4] when its byte +C8h is 0, then +3Ch =
// its +100h (009AFFBF-009AFFDC); 009AFAF0(ECX = approach, 0) (009AFFDF).
// UNCERTAINTY: 24-bit precision modelled; the 0.75 product is exact in double. RNG
// results rounded from ST0 by the hook. EH unwinding not modelled (the body has no EH
// frame).
// cc9 models the draws inline (per-name RNG stand-ins, not a named body) in
// GameUnitsHost::Impl::install_land_task_0099a3dd (src/game_hosts_units.cpp), block
// "009B41C0 -> 009B3240 -> 009B2E50 -> 009AFE70".
void* bot_approach_land_construct_lower_009afe70(Cc13Seg67bHost& host, void* approach,
                                                 void* plane, void* air_ops) {
    const float speed = at<float>(host.game_tuning_singleton_0042e740(), 0x52C);
    host.bot_approach_construct_speed_reference_009f9ce0(approach, plane, speed);
    at<std::uint32_t>(approach, 0x00) = kLandApproachVtable_00d1fdb8;
    at<std::uintptr_t>(approach, 0x2C) = reinterpret_cast<std::uintptr_t>(air_ops);
    at<std::uint32_t>(approach, 0x30) = at<std::uint32_t>(air_ops, 0x7C);
    at<std::uintptr_t>(approach, 0x34) =
        host.air_ops_queue_incoming_006c0b50(air_ops, ptr_at(plane, 0x9D4));
    at<std::uint8_t>(approach, 0x44) = 0;
    at<float>(approach, 0x4C) = kOne_00d7a24c;
    at<float>(approach, 0x48) = kOne_00d7a24c;
    at<std::uint32_t>(approach, 0x50) = 1;
    void* tuning = host.game_tuning_singleton_0042e740();
    at<float>(approach, 0x54) =
        host.random_uniform_00bd2f10(1, kPointNine_00ce3860, kOnePointOne_00ce6448) *
        at<float>(tuning, 0x4E4);
    tuning = host.game_tuning_singleton_0042e740();
    at<float>(approach, 0x58) =
        host.random_uniform_00bd2f10(1, kPointNine_00ce3860, kOnePointOne_00ce6448) *
        at<float>(tuning, 0x4E0);
    at<float>(approach, 0x60) = kZero_s15;
    at<float>(approach, 0x64) = kTen_00ce38b8;
    at<float>(approach, 0x5C) = static_cast<float>(
        static_cast<double>(at<float>(ptr_at(approach, 0x8), 0x18C)) * kPointSevenFive_00cec9d8);
    at<float>(approach, 0xA4) = kZero_s15;
    at<float>(approach, 0xAC) = kHalf_00ce3800;
    at<float>(approach, 0xB0) = -host.random_uniform_00bd2f10(1, 0.0f, kHalf_00ce3800);
    void* landing = ptr_at(approach, 0x30);
    if (at<std::uint8_t>(landing, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(landing);
    at<float>(approach, 0x38) = at<float>(landing, 0xFC);
    at<float>(approach, 0x3C) = at<float>(landing, 0x100);
    at<float>(approach, 0x40) = at<float>(landing, 0x104);
    void* owner = ptr_at(approach, 0x4);
    if (at<std::uint8_t>(owner, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(owner);
    at<float>(approach, 0x3C) = at<float>(owner, 0x100);
    host.bot_approach_land_geometry_009afaf0(approach, 0);
    return approach;
}

// ---------------------------------------------------------------------------
// The land approach base from a stream.

namespace {
constexpr std::uint32_t kStrAirBaseEntity_00d1fe78 = 0x00D1FE78u;        // "airBaseEntity"
constexpr std::uint32_t kStrUpdateLandQueueTimer_00d1fe60 = 0x00D1FE60u; // "updateLandQueueTimer"
constexpr std::uint32_t kStrApproachAngle_00d1fe50 = 0x00D1FE50u;        // "approachAngle"
constexpr std::uint32_t kStrApproachPitch_00d1fe40 = 0x00D1FE40u;        // "approachPitch"
constexpr std::uint32_t kStrStandbyHeightAdd_00d1fe2c = 0x00D1FE2Cu;     // "standbyheightadd"
}  // namespace

// 009AFFF0 (BSP_BotApproachLand_ConstructFromStream): __thiscall(ECX = approach, plane,
// archive), RET 8, EAX = approach. Evidence (disk bytes): 009F9CE0(ECX = approach, plane,
// tuning+52Ch) (009AFFF6-009B000C); vtable 00D1FDB8, byte +44h = 0, +4Ch = +48h = 1.0,
// +50h = 1 (009B0011-009B003A); +5Ch = float(desc+18Ch x double [00CEC9D8] 0.75), +64h
// = [00CE38B8] 10.0, +A4h = 0.0, +ACh = 1.0 (009B003D-009B006A); +B0h = -00BD2F10(1, 0,
// 1) (009B0068-009B0081); archive vtable[10h]({0,"airBaseEntity"}, {4, &slot}), +30h =
// slot (009B0084-009B00B3); +2Ch = 006BCD20(ECX = slot, DL = 0) (009B00B1-009B00BB); +34h
// = 006C0B50(ECX = +2Ch, [plane+9D4h]) (009B00BE-009B00D8); 00BD4FE0(ECX = archive, the
// outgoing block, {0,"updateLandQueueTimer"}) (009B00F1) and 00410D80(ECX = +ACh, that
// block) (009B00F8); reads {0,"approachAngle"} <- {2, &+54h} (009B012F),
// {0,"approachPitch"} <- {2, &+58h} (009B0163), {0,"standbyheightadd"} <- {2, &+60h}
// (009B0197); 00414DB0 on [approach+30h] when its byte +C8h is 0, then +38h/+3Ch/+40h =
// its +FCh/+100h/+104h (009B0199-009B01C4); 00414DB0 on [approach+4] when its byte +C8h
// is 0, then +3Ch = its +100h (009B01C7-009B01E4); 009AFAF0(ECX = approach, 0)
// (009B01E7).
// UNCERTAINTY: an unwritten "airBaseEntity" leaves a stack address in the slot in the
// image; here the slot starts at 0. 006C0B50 runs on +2Ch without a null test, as in the
// image. RNG result rounded from ST0 by the hook. No EH frame.
void* bot_approach_land_construct_from_stream_009afff0(Cc13Seg67bHost& host, void* approach,
                                                       void* plane, void* archive) {
    const float speed = at<float>(host.game_tuning_singleton_0042e740(), 0x52C);
    host.bot_approach_construct_speed_reference_009f9ce0(approach, plane, speed);
    at<std::uint32_t>(approach, 0x00) = kLandApproachVtable_00d1fdb8;
    at<std::uint8_t>(approach, 0x44) = 0;
    at<float>(approach, 0x4C) = kOne_00d7a24c;
    at<float>(approach, 0x48) = kOne_00d7a24c;
    at<std::uint32_t>(approach, 0x50) = 1;
    at<float>(approach, 0x5C) = static_cast<float>(
        static_cast<double>(at<float>(ptr_at(approach, 0x8), 0x18C)) * kPointSevenFive_00cec9d8);
    at<float>(approach, 0x64) = kTen_00ce38b8;
    at<float>(approach, 0xA4) = kZero_s15;
    at<float>(approach, 0xAC) = kOne_00d7a24c;
    at<float>(approach, 0xB0) = -host.random_uniform_00bd2f10(1, 0.0f, 1.0f);
    std::uintptr_t slot = 0;
    host.archive_vcall_10_read(archive, name_record(kStrAirBaseEntity_00d1fe78),
                               record(4, reinterpret_cast<std::uintptr_t>(&slot)));
    at<std::uintptr_t>(approach, 0x30) = slot;
    void* const air_ops = host.air_ops_get_block_006bcd20(reinterpret_cast<void*>(slot), false);
    at<std::uintptr_t>(approach, 0x2C) = reinterpret_cast<std::uintptr_t>(air_ops);
    at<std::uintptr_t>(approach, 0x34) =
        host.air_ops_queue_incoming_006c0b50(air_ops, ptr_at(plane, 0x9D4));
    std::uint32_t ref[3] = {};
    host.archive_reader_ref_open_00bd4fe0(archive, ref,
                                          name_record(kStrUpdateLandQueueTimer_00d1fe60));
    host.time_freq_pair_read_00410d80(ptr(approach, 0xAC), ref);
    read_field(host, archive, kStrApproachAngle_00d1fe50, 2, ptr(approach, 0x54));
    read_field(host, archive, kStrApproachPitch_00d1fe40, 2, ptr(approach, 0x58));
    read_field(host, archive, kStrStandbyHeightAdd_00d1fe2c, 2, ptr(approach, 0x60));
    void* landing = ptr_at(approach, 0x30);
    if (at<std::uint8_t>(landing, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(landing);
    at<float>(approach, 0x38) = at<float>(landing, 0xFC);
    at<float>(approach, 0x3C) = at<float>(landing, 0x100);
    at<float>(approach, 0x40) = at<float>(landing, 0x104);
    void* owner = ptr_at(approach, 0x4);
    if (at<std::uint8_t>(owner, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(owner);
    at<float>(approach, 0x3C) = at<float>(owner, 0x100);
    host.bot_approach_land_geometry_009afaf0(approach, 0);
    return approach;
}


// ---------------------------------------------------------------------------
// The dogfight approach base from a stream (packet cc13-seg67C).

namespace {
constexpr std::uint32_t kStrTargetLockUnderscore_00d1f624 = 0x00D1F624u;  // "_targetLock"
constexpr std::uint32_t kStrSquadName_00d1f888 = 0x00D1F888u;             // "squadName"
constexpr std::uint32_t kStrInAttackRange_00d1f878 = 0x00D1F878u;         // "inattackrange"
constexpr std::uint32_t kStrTargetFindAngle_00d1f868 = 0x00D1F868u;       // "targetfindangle"
constexpr std::uint32_t kStrTargetFindWeight_00d1f854 = 0x00D1F854u;      // "targetfindweight"
constexpr std::uint32_t kStrWasHitTimer_00d1f848 = 0x00D1F848u;           // "washittimer"
}  // namespace

// 009A6D40 (BSP_BotApproachDogfight_ConstructFromStream): __thiscall(ECX = approach, bot,
// archive), RET 8, EAX = approach. EH frame 00CAE237. Evidence (disk bytes):
// 009F9CE0(ECX = approach, [bot+50h], tuning+650h) (009A6D62-009A6D7B); vtable 00D1F844
// (009A6D92); 00BD4FE0(ECX = archive, the outgoing block, {0,"_targetLock"}) (009A6DAA)
// and 009FB3E0(ECX = approach+2Ch, that block by value) (009A6DB2). The observer member
// at +A0h is built inline: +A4h/+A8h/+ACh/+B4h = 0, byte +B0h = 1, [+A0h] = 00CF89B4
// (009A6DB7-009A6DD2); so is the one at +B8h: +BCh/+C0h/+C4h/+CCh = 0, byte +C8h = 1,
// [+B8h] = 00CF6514 (009A6DD8-009A6DEF). (The direct twin 009A6C10 calls 007F0F80 for
// +B8h instead.) archive->vtable[10h]({0,"squadName"}, {4, &slot}) (009A6E1F), where the
// slot is the image's `bot` argument slot [ESP+38h]; then the +B8h observer is swapped
// to that entity: unchanged -> skip; old != 0 -> 006952A0(old, +B8h); +CCh = e; e != 0
// -> 00694A60(e, +B8h) (009A6E21-009A6E46). The +A0h observer: +B4h != 0 ->
// 006952A0(+B4h, +A0h), +B4h = 0 (009A6E4B-009A6E59); t = [+44h] (the target ref's
// entity); t != 0 -> k = t->vtable[5Ch](0Fh) ? t : 0 (NEG AL / SBB / AND), and the
// same swap of +B4h to k (009A6E60-009A6E95). The target position +48h..+50h is copied
// BEFORE the plane refresh (009A6E9A-009A6EBF); plane = [+4h], 00414DB0(plane) unless
// byte plane+C8h (009A6EC9); dx/dy/dz = float(plane+FCh/+100h/+104h - copy)
// (009A6ECE-009A6EF4). s = float((dy*dy + dx*dx) + dz*dz) (009A6EF8-009A6F1A: FMUL
// ST0,ST0; FLD ST1; FMULP ST2; FXCH; FADD ST2; FADD ST1); +D4h = s > double [00CE3820]
// 1e-10 ? float(_CIsqrt(s)) : 0.0 (FCOMI then JBE: NaN -> 0) (009A6F1E-009A6F5C).
// Then FLDZ squared is added: h = float(dz*dz + (dx*dx + 0.0)) (009A6F5A-009A6F6A), and
// +D8h = h > 1e-10 ? float(_CIsqrt(h)) : 0.0 (JBE) (009A6F6E-009A6F9F). Reads
// {0,"inattackrange"} <- {3, &+D0h} (009A6FDA), {0,"targetfindangle"} <- {2, &+DCh}
// (009A7012), {0,"targetfindweight"} <- {2, &+E0h} (009A704A), {0,"washittimer"} <- {2,
// &+E8h} (009A7082); +E4h = [00D7A260] -1.0 (009A7084-009A7091).
// Stack seed, labelled: 009A6D87, 009A6D98 and 009A6DFA (`MOV [ESP+..],ESP`) overwrite
// the `bot` slot before the squadName read. The nearest write, 009A6DFA, leaves the
// address of the outgoing field record {4, &slot}, so a missing squadName key registers
// the +B8h observer on that stack address. Here the slot is seeded the same way, with
// the address of this body's field record.
// UNCERTAINTY: 24-bit precision modelled: the squares and sums round to float at each
// step, as float arithmetic does here (the image keeps dx*dx and dz*dz on the x87 stack
// and spills them as doubles, which are float-exact). The x87 stack's wider exponent
// range is not modelled (it matters only for squares beyond FLT_MAX). The square root is
// std::sqrt in double rounded to float, which is the correctly rounded float root the
// 24-bit FSQRT gives. EH unwinding not modelled.
void* bot_approach_dogfight_construct_from_stream_009a6d40(Cc13Seg67bHost& host, void* approach,
                                                           void* bot, void* archive) {
    const float speed = at<float>(host.game_tuning_singleton_0042e740(), 0x650);
    host.bot_approach_construct_speed_reference_009f9ce0(approach, ptr_at(bot, 0x50), speed);
    at<std::uint32_t>(approach, 0x00) = kDogfightApproachVtable_00d1f844;
    std::uint32_t ref[3] = {};
    host.archive_reader_ref_open_00bd4fe0(archive, ref,
                                          name_record(kStrTargetLockUnderscore_00d1f624));
    host.approach_ref_construct_from_stream_009fb3e0(ptr(approach, 0x2C), ref);
    void* const a0 = ptr(approach, 0xA0);
    at<std::uint32_t>(a0, 0x04) = 0;
    at<std::uint32_t>(a0, 0x08) = 0;
    at<std::uint32_t>(a0, 0x0C) = 0;
    at<std::uint32_t>(a0, 0x14) = 0;
    at<std::uint8_t>(a0, 0x10) = 1;
    at<std::uint32_t>(a0, 0x00) = kDogfightEmbeddedA0Vtable_00cf89b4;
    void* const b8 = ptr(approach, 0xB8);
    at<std::uint32_t>(b8, 0x04) = 0;
    at<std::uint32_t>(b8, 0x08) = 0;
    at<std::uint32_t>(b8, 0x0C) = 0;
    at<std::uint32_t>(b8, 0x14) = 0;
    at<std::uint8_t>(b8, 0x10) = 1;
    at<std::uint32_t>(b8, 0x00) = kDogfightObserverVtable_00cf6514;
    // The image's slot holds the address of the outgoing field record (009A6DFA).
    std::uint32_t field_record[2] = {4u, 0u};
    std::uintptr_t slot = reinterpret_cast<std::uintptr_t>(field_record);
    host.archive_vcall_10_read(archive, name_record(kStrSquadName_00d1f888),
                               record(4, reinterpret_cast<std::uintptr_t>(&slot)));
    {
        const std::uintptr_t old = at<std::uintptr_t>(b8, 0x14);
        if (old != slot) {
            if (old != 0) host.observer_unregister_pair_006952a0(reinterpret_cast<void*>(old), b8);
            at<std::uintptr_t>(b8, 0x14) = slot;
            if (slot != 0) host.observer_register_pair_00694a60(reinterpret_cast<void*>(slot), b8);
        }
    }
    if (ptr_at(a0, 0x14) != nullptr) {
        host.observer_unregister_pair_006952a0(ptr_at(a0, 0x14), a0);
        at<std::uint32_t>(a0, 0x14) = 0;
    }
    void* const t = ptr_at(approach, 0x44);
    if (t != nullptr) {
        void* const k = host.object_vcall_5c(t, 0x0F) ? t : nullptr;
        void* const old = ptr_at(a0, 0x14);
        if (old != k) {
            if (old != nullptr) host.observer_unregister_pair_006952a0(old, a0);
            at<std::uintptr_t>(a0, 0x14) = reinterpret_cast<std::uintptr_t>(k);
            if (k != nullptr) host.observer_register_pair_00694a60(k, a0);
        }
    }
    const float tx = at<float>(approach, 0x48);
    const float ty = at<float>(approach, 0x4C);
    const float tz = at<float>(approach, 0x50);
    void* const plane = ptr_at(approach, 0x04);
    if (at<std::uint8_t>(plane, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(plane);
    const float dx = at<float>(plane, 0xFC) - tx;
    const float dy = at<float>(plane, 0x100) - ty;
    const float dz = at<float>(plane, 0x104) - tz;
    const float dx2 = dx * dx;
    const float dz2 = dz * dz;
    const float dy2 = dy * dy;
    const float s = (dy2 + dx2) + dz2;
    at<float>(approach, 0xD4) =
        static_cast<double>(s) > kTiny_00ce3820
            ? static_cast<float>(std::sqrt(static_cast<double>(s)))
            : 0.0f;
    const float h = dz2 + (dx2 + 0.0f);
    at<float>(approach, 0xD8) =
        static_cast<double>(h) > kTiny_00ce3820
            ? static_cast<float>(std::sqrt(static_cast<double>(h)))
            : 0.0f;
    read_field(host, archive, kStrInAttackRange_00d1f878, 3, ptr(approach, 0xD0));
    read_field(host, archive, kStrTargetFindAngle_00d1f868, 2, ptr(approach, 0xDC));
    read_field(host, archive, kStrTargetFindWeight_00d1f854, 2, ptr(approach, 0xE0));
    read_field(host, archive, kStrWasHitTimer_00d1f848, 2, ptr(approach, 0xE8));
    at<float>(approach, 0xE4) = kMinusOne_00d7a260_f;
    return approach;
}


// ---------------------------------------------------------------------------
// The level-bomb approach base from a stream (packet cc13-seg67C).

namespace {
constexpr std::uint32_t kStrReleaseTime_00d200d8 = 0x00D200D8u;   // "releaseTime"
constexpr std::uint32_t kStrReleaseError_00d200c8 = 0x00D200C8u;  // "releaseError"
constexpr std::uint32_t kStrBayDoorState_00d200b8 = 0x00D200B8u;  // "bayDoorState"
constexpr std::uint32_t kStrCalcHitPos_00d200ac = 0x00D200ACu;    // "calcHitPos"
}  // namespace

// 009B4690 (BSP_BotApproachLevelBomb_ConstructFromStream): __thiscall(ECX = approach,
// arg, archive), RET 8, EAX = approach; the caller 009B77C0 passes [bot+50h] as arg.
// EH frame 00CAED2B. The stream twin of 009B44F0. Evidence (disk bytes):
// 009F9CE0(ECX = approach, arg, tuning+458h) (009B46AE-009B46C4); vtable 00D200A8, +2Ch
// = 0 (009B46DF-009B46E5); 00BD4FE0(ECX = archive, the outgoing block, {0,"targetLock"})
// (009B46FC) and 009FB3E0(ECX = approach+30h, that block by value) (009B4704); +A4h =
// [00CE6454] 16.0, +ACh = desc+188h (desc = [approach+8], FLD/FSTP of a float, exact),
// +B0h = [00CFD714] 1200.0 (009B4709-009B4733). Then archive->vtable[10h] reads, in
// order: {0,"releaseTime"} <- {2, &+A8h} (009B4775), {0,"startDist"} <- {2, &+B4h}
// (009B47AC), {0,"releaseError"} <- {2, &+C4h} (009B47E3), {0,"attackDist"} <- {2,
// &+B8h} (009B481A), {0,"targetDist"} <- {2, &+BCh} (009B4851), {0,"bayDoorState"} <- {3,
// &+CCh} (009B4888), {0,"insideAttackRange"} <- {3, &+CDh} (009B48BF), {0,"targetHdg"}
// <- {2, &+C0h} (009B48F6), {0,"currentAvoidFlag"} <- {1, &+C8h} (009B492D),
// {0,"calcHitPos"} <- {5, &+D0h} (009B4964). Unlike 009B44F0 it makes no RNG draws and
// does not call the aim-error redraw 009B4400; the stream supplies those fields.
// UNCERTAINTY: EH unwinding not modelled.
void* bot_approach_level_bomb_construct_from_stream_009b4690(Cc13Seg67bHost& host,
                                                             void* approach, void* arg,
                                                             void* archive) {
    const float speed = at<float>(host.game_tuning_singleton_0042e740(), 0x458);
    host.bot_approach_construct_speed_reference_009f9ce0(approach, arg, speed);
    at<std::uint32_t>(approach, 0x00) = kLevelBombApproachVtable_00d200a8;
    at<std::uint32_t>(approach, 0x2C) = 0;
    std::uint32_t ref[3] = {};
    host.archive_reader_ref_open_00bd4fe0(archive, ref, name_record(kStrTargetLockPlain_00d1fb5c));
    host.approach_ref_construct_from_stream_009fb3e0(ptr(approach, 0x30), ref);
    at<float>(approach, 0xA4) = kSixteen_00ce6454;
    at<float>(approach, 0xAC) = at<float>(ptr_at(approach, 0x8), 0x188);
    at<float>(approach, 0xB0) = kTwelveHundred_00cfd714;
    read_field(host, archive, kStrReleaseTime_00d200d8, 2, ptr(approach, 0xA8));
    read_field(host, archive, kStrStartDist_00d1fb50, 2, ptr(approach, 0xB4));
    read_field(host, archive, kStrReleaseError_00d200c8, 2, ptr(approach, 0xC4));
    read_field(host, archive, kStrAttackDist_00d1fb44, 2, ptr(approach, 0xB8));
    read_field(host, archive, kStrTargetDist_00d1fb38, 2, ptr(approach, 0xBC));
    read_field(host, archive, kStrBayDoorState_00d200b8, 3, ptr(approach, 0xCC));
    read_field(host, archive, kStrInsideAttackRange_00d1fb24, 3, ptr(approach, 0xCD));
    read_field(host, archive, kStrTargetHdg_00d1fb18, 2, ptr(approach, 0xC0));
    read_field(host, archive, kStrCurrentAvoidFlag_00d1fb04, 1, ptr(approach, 0xC8));
    read_field(host, archive, kStrCalcHitPos_00d200ac, 5, ptr(approach, 0xD0));
    return approach;
}


// ---------------------------------------------------------------------------
// The move-on-path approach's path entity (packet cc13-seg67C).

namespace {
constexpr std::uint32_t kMoveOnPathCommandClass_00e08f80 = 0x00E08F80u;  // `moveonpath`
}  // namespace

// 009BD0B0 (BSP_BotApproachMoveOnPath_PathEntity, 009BD0B0-009BD119): __fastcall(ECX =
// approach), RET, EAX. Evidence (disk bytes): ctl = [approach+0Ch]; ctl == 0 -> 0
// (009BD0B3-009BD0B8). director = ctl->vtable[114h]() (009BD0C2, no stack arguments);
// [director+54h] != 00E08F80 -> 0 (009BD0C4-009BD0CB). ctl->vtable[114h]() again
// (009BD0DA); the `PUSH 0` at 009BD0D8 is the stack argument of the following
// 0071BFF0(ECX = that director, 0) (009BD0DE, RET 4), not of vtable[114h]. cursor == 0
// -> 0 (009BD0E5). path = [cursor+4]; the NEG/SBB/TEST 00E19B95 at 009BD0EE-009BD0F2 is
// a base-cast null test, so path == 0 -> 0. path->vtable[28h]() == 0 -> 0 (009BD0FE-
// 009BD102); else path->vtable[28h]() is called a second time (009BD10C) and its result
// goes to 007ACA30 by a tail JMP (009BD111), whose EAX is returned.
// UNCERTAINTY: none beyond the host hooks; the director, cursor and path are
// dereferenced exactly where the image does.
void* bot_approach_move_on_path_path_entity_009bd0b0(Cc13Seg67bHost& host, void* approach) {
    void* const ctl = ptr_at(approach, 0x0C);
    if (ctl == nullptr) return nullptr;
    void* const director = host.object_vcall_114(ctl);
    if (at<std::uint32_t>(director, 0x54) != kMoveOnPathCommandClass_00e08f80) return nullptr;
    void* const director2 = host.object_vcall_114(ptr_at(approach, 0x0C));
    void* const cursor = host.weapon_director_command_path_cursor_0071bff0(director2, 0);
    if (cursor == nullptr) return nullptr;
    void* const path = ptr_at(cursor, 0x04);
    if (path == nullptr) return nullptr;
    if (host.object_vcall_28(path) == nullptr) return nullptr;
    return host.path_object_get_entity_007aca30(host.object_vcall_28(ptr_at(cursor, 0x04)));
}


// ---------------------------------------------------------------------------
// The ship-AI debug box (packet cc13-seg67C).

namespace {
constexpr double kTwenty_00ce3d88 = 20.0;  // bytes 00 00 00 00 00 00 34 40
void debug_line_xz(Cc13Seg67bHost& host, float x0, float z0, float x1, float z1) {
    const float from[3] = {x0, 0.0f, z0};
    const float to[3] = {x1, 0.0f, z1};
    const float colour[4] = {kOne_00d7a24c, kOne_00d7a24c, kOne_00d7a24c, kOne_00d7a24c};
    host.debug_line_0085fb30(from, to, colour);
}
}  // namespace

// 009D7260 (BSP_ShipAiDebug_DrawBoxLines, 009D7260-009D7770): __fastcall(ECX = box), RET.
// Evidence (disk bytes): byte box+68h set -> return (009D7266-009D726A). With c =
// (+44h, +48h), a = (+4Ch, +50h), b = (+54h, +58h), h1 = +5Ch and h2 = +60h, the image
// forms u = h2 x b and v = h1 x a (each product spilled to a float) and the corners
// p1 = (c + v) + u (009D72C2-009D72EC), p2 = (c + v) - u (009D7330-009D735A), p3 =
// (c - v) - u (009D73C5-009D73EF) and p4 = (c - v) + u (009D7429-009D7480), every sum
// spilled to a float. Four lines through 0085FB30(ECX = from, EDX = to, colour by value
// [00D7A24C] 1.0 x 4), RET 10h, with y = 0.0 at both ends: p1 -> p2 (009D74B3), p2 -> p3
// (009D753F), p3 -> p4 (009D75CB), p4 -> p1 (009D7657). Then with m = +88h != 0: e =
// float(h2 + double [00CE3D88] 20.0) (009D766D-009D7676); m == 1 -> end = c - e x b
// (009D767C-009D76A6), else end = e x b + c (009D76A8-009D76CA); a fifth line c -> end
// (009D7767).
// UNCERTAINTY: 24-bit precision modelled (each product and sum rounds to float, as in
// the image's spills); e is rounded once from the double sum, which equals the 24-bit
// FADD except in double-rounding ties.
void ship_ai_debug_draw_box_lines_009d7260(Cc13Seg67bHost& host, void* box) {
    if (at<std::uint8_t>(box, 0x68) != 0) return;
    const float c0 = at<float>(box, 0x44), c1 = at<float>(box, 0x48);
    const float a0 = at<float>(box, 0x4C), a1 = at<float>(box, 0x50);
    const float b0 = at<float>(box, 0x54), b1 = at<float>(box, 0x58);
    const float h1 = at<float>(box, 0x5C), h2 = at<float>(box, 0x60);
    const float u0 = b0 * h2, u1 = h2 * b1;
    const float v0 = a0 * h1, v1 = h1 * a1;
    const float p1x = (c0 + v0) + u0, p1z = u1 + (c1 + v1);
    const float p2x = (v0 + c0) - u0, p2z = (c1 + v1) - u1;
    const float p3x = (c0 - v0) - u0, p3z = (c1 - v1) - u1;
    const float p4x = (c0 - v0) + u0, p4z = (c1 - v1) + u1;
    debug_line_xz(host, p1x, p1z, p2x, p2z);
    debug_line_xz(host, p2x, p2z, p3x, p3z);
    debug_line_xz(host, p3x, p3z, p4x, p4z);
    debug_line_xz(host, p4x, p4z, p1x, p1z);
    const std::int32_t mode = at<std::int32_t>(box, 0x88);
    if (mode == 0) return;
    const float e = static_cast<float>(static_cast<double>(h2) + kTwenty_00ce3d88);
    float ex, ez;
    if (mode == 1) {
        ex = c0 - b0 * e;
        ez = c1 - e * b1;
    } else {
        ex = e * b0 + c0;
        ez = c1 + e * b1;
    }
    debug_line_xz(host, c0, c1, ex, ez);
}


// ---------------------------------------------------------------------------
// The ship-AI debug draws (packet cc13-seg67C).

namespace {
constexpr float kTwenty_00ce3930 = 20.0f;  // 41A00000
constexpr std::uint32_t kDebugCircleSegments = 0x40u;
// A function-local static colour in the block at 00F8A68C: when bit 0 of the guard
// dword is clear it is set (OR) and the record filled. Returns the record.
float* debug_static_colour(Cc13Seg67bHost& host, std::size_t record, std::size_t guard,
                           float r, float g, float b, float a) {
    std::uint8_t* const block = host.ship_ai_debug_static_block_00f8a68c();
    if ((at<std::uint8_t>(block, guard) & 1u) == 0) {
        at<std::uint32_t>(block, guard) |= 1u;
        at<float>(block, record + 0x0) = r;
        at<float>(block, record + 0x4) = g;
        at<float>(block, record + 0x8) = b;
        at<float>(block, record + 0xC) = a;
    }
    return &at<float>(block, record);
}
// Red 00F8A6A0 (guard 00F8A6B0), green 00F8A6B4 (00F8A6C4), blue 00F8A6C8 (00F8A6D8),
// yellow 00F8A6F0 (00F8A700). Cyan 00F8A6DC (00F8A6EC) is 009D5FD0's record.
float* debug_red(Cc13Seg67bHost& h) { return debug_static_colour(h, 0x14, 0x24, 1, 0, 0, 1); }
float* debug_green(Cc13Seg67bHost& h) { return debug_static_colour(h, 0x28, 0x38, 0, 1, 0, 1); }
float* debug_blue(Cc13Seg67bHost& h) { return debug_static_colour(h, 0x3C, 0x4C, 0, 0, 1, 1); }
float* debug_cyan(Cc13Seg67bHost& h) { return debug_static_colour(h, 0x50, 0x60, 0, 1, 1, 1); }
float* debug_yellow(Cc13Seg67bHost& h) { return debug_static_colour(h, 0x64, 0x74, 1, 1, 0, 1); }

// One child link of 009D98C0: the block at 009D98E5-009D9AB2 (child +20h, flag +31h)
// and its copy at 009D9AD4-009D9CAB (child +24h, flag +32h).
void draw_path_tree_link(Cc13Seg67bHost& host, void* node, std::size_t child_offset,
                         std::size_t flag_offset) {
    void* const child = ptr_at(node, child_offset);
    const float p[3] = {at<float>(node, 0x18), kTen_00ce38b8, at<float>(node, 0x1C)};
    const float q[3] = {at<float>(child, 0x18), kTen_00ce38b8, at<float>(child, 0x1C)};
    const float white[4] = {kOne_00d7a24c, kOne_00d7a24c, kOne_00d7a24c, kOne_00d7a24c};
    float* const green = debug_green(host);
    host.wire_sphere_00860be0(q, green, kTwenty_00ce3930, white, 1);
    if (at<std::uint8_t>(node, flag_offset) != 0) {
        host.debug_line_0085fb30(p, q, debug_green(host));
    } else {
        float* const red = debug_red(host);
        host.line_rgba_floats_0085f990(p, q, red, red);
    }
    ship_ai_debug_draw_path_tree_009d98c0(host, child);
}
}  // namespace

// 009D98C0 (BSP_ShipAiDebug_DrawPathTree, 009D98C0-009D9CB6): __fastcall(ECX = node), RET.
// Evidence (disk bytes): for each child in turn, [node+20h] (009D98DA) then [node+24h]
// (009D9ACA), when non-zero: P = (node+18h, [00CE38B8] 10.0, node+1Ch) and Q = (child+18h,
// 10.0, child+1Ch); the static green record 00F8A6B4 = {0, 1, 0, 1} (guard 00F8A6C4);
// 00860BE0(ECX = Q, EDX = green, [00CE3930] 20.0, a stack white {1, 1, 1, 1}, 1)
// (009D9982 / 009D9B71). Byte node+31h (+32h for the second child) set -> 0085FB30(ECX
// = P, EDX = Q, green by value) (009D99F8 / 009D9BE7); else the static red 00F8A6A0 =
// {1, 0, 0, 1} (guard 00F8A6B0) and 0085F990(ECX = P, EDX = Q, red, red by value)
// (009D9AAA / 009D9CA3). Then the recursion on the child (009D9AB2 / 009D9BEF).
// The static colours are the image's function-local statics, written through the
// 00F8A68C block hook at the image's guard tests.
// UNCERTAINTY: none beyond the host hooks.
void ship_ai_debug_draw_path_tree_009d98c0(Cc13Seg67bHost& host, void* node) {
    if (ptr_at(node, 0x20) != nullptr) draw_path_tree_link(host, node, 0x20, 0x31);
    if (ptr_at(node, 0x24) != nullptr) draw_path_tree_link(host, node, 0x24, 0x32);
}

// 009DD9C0 (BSP_ShipAiDebug_DrawPlan, 009DD9C0-009DDB9E): __fastcall(ECX = plan), RET.
// Evidence (disk bytes): [plan+20h] != 0 -> 009D98C0 on it (009DD9CD). Byte
// [plan+3Ch]+174h compared as a dword with [00E0E300]; unequal -> return (009DD9D2-
// 009DD9E2). 00860BB0(ECX = (plan+60h, 0, plan+64h), EDX = (0, 1, 0), plan+4Ch, a stack
// white {1, 1, 1, 1}, 40h) (009DDA4F). The static yellow 00F8A6F0 = {1, 1, 0, 1}
// (guard 00F8A700) and 00860BE0(ECX = (plan+50h, 0, plan+54h), EDX = yellow,
// [00CE38B8] 10.0, yellow, 1) (009DDAF2); the static blue 00F8A6C8 = {0, 0, 1, 1}
// (guard 00F8A6D8) and 00860BE0(ECX = (plan+58h, 0, plan+5Ch), EDX = blue, 10.0, blue,
// 1) (009DDB95).
// UNCERTAINTY: none beyond the host hooks. The image tests the guard bit twice after
// setting it (009DDA6B, 009DDB0E); the second test always passes, so it is not repeated.
void ship_ai_debug_draw_plan_009dd9c0(Cc13Seg67bHost& host, void* plan) {
    void* const tree = ptr_at(plan, 0x20);
    if (tree != nullptr) ship_ai_debug_draw_path_tree_009d98c0(host, tree);
    const std::uint32_t side = at<std::uint8_t>(ptr_at(plan, 0x3C), 0x174);
    if (side != static_cast<std::uint32_t>(host.global_debug_side_00e0e300())) return;
    const float white[4] = {kOne_00d7a24c, kOne_00d7a24c, kOne_00d7a24c, kOne_00d7a24c};
    const float axis[3] = {0.0f, kOne_00d7a24c, 0.0f};
    const float centre[3] = {at<float>(plan, 0x60), 0.0f, at<float>(plan, 0x64)};
    host.circle_around_axis_00860bb0(centre, axis, at<float>(plan, 0x4C), white,
                                     kDebugCircleSegments);
    float* const yellow = debug_yellow(host);
    const float a[3] = {at<float>(plan, 0x50), 0.0f, at<float>(plan, 0x54)};
    host.wire_sphere_00860be0(a, yellow, kTen_00ce38b8, yellow, 1);
    float* const blue = debug_blue(host);
    const float b[3] = {at<float>(plan, 0x58), 0.0f, at<float>(plan, 0x5C)};
    host.wire_sphere_00860be0(b, blue, kTen_00ce38b8, blue, 1);
}

// 009DDCC0 (BSP_ShipAiDebug_Draw, 009DDCC0-009DE04B): __fastcall(ECX = ship AI), RET.
// Evidence (disk bytes): with a stack white {1, 1, 1, 1} and the static yellow
// 00F8A6F0 (guard 00F8A700): 00860BE0(ECX = (ai+18Ch, 0, ai+190h), EDX = yellow,
// [00CE3850] 5.0, white, 1) (009DDD59) and the same for (ai+194h, 0, ai+198h)
// (009DDDEE), the white refilled and the guard re-tested between them. u = [ai+3FCh];
// when u != 0 and ([00E0E300] < 0 or byte u+174h == [00E0E300]) (009DDDF3-009DDE14):
// 009DD9C0 on byte ai+2FCh ? [ai+2F8h] : [ai+2F4h] (009DDE1A-009DDE31); the static
// cyan 00F8A6DC (guard 00F8A6EC); 00860BB0(ECX = (ai+18Ch, 5.0, ai+190h), EDX = (0, 1,
// 0), ai+3C8h, cyan, 40h) (009DDEC5) and the same around (ai+194h, 5.0, ai+198h)
// (009DDF59); the static red 00F8A6A0, 004166E0(ECX = ai+A3Ch, red) (009DDFA3); the
// static yellow, 004166E0(ECX = ai+A5Ch, yellow) (009DDFED); byte ai+A90h set ->
// 00860BE0(ECX = ai+A94h, EDX = 009D5FD0(), 5.0, 009D5E10(), 1), with 009D5E10 called
// first (009DDFFC-009DE019); then 009D7260 on each of the [ai+604h] boxes at ai+608h,
// the count re-read each step (009DE01F-009DE042).
// UNCERTAINTY: none beyond the host hooks.
void ship_ai_debug_draw_009ddcc0(Cc13Seg67bHost& host, void* ai) {
    {
        const float white[4] = {kOne_00d7a24c, kOne_00d7a24c, kOne_00d7a24c, kOne_00d7a24c};
        float* const yellow = debug_yellow(host);
        const float c[3] = {at<float>(ai, 0x18C), 0.0f, at<float>(ai, 0x190)};
        host.wire_sphere_00860be0(c, yellow, kFive_00ce3850, white, 1);
    }
    {
        const float white[4] = {kOne_00d7a24c, kOne_00d7a24c, kOne_00d7a24c, kOne_00d7a24c};
        float* const yellow = debug_yellow(host);
        const float c[3] = {at<float>(ai, 0x194), 0.0f, at<float>(ai, 0x198)};
        host.wire_sphere_00860be0(c, yellow, kFive_00ce3850, white, 1);
    }
    void* const u = ptr_at(ai, 0x3FC);
    if (u == nullptr) return;
    const std::int32_t side = host.global_debug_side_00e0e300();
    if (side >= 0 && static_cast<std::int32_t>(at<std::uint8_t>(u, 0x174)) != side) return;
    ship_ai_debug_draw_plan_009dd9c0(
        host, at<std::uint8_t>(ai, 0x2FC) != 0 ? ptr_at(ai, 0x2F8) : ptr_at(ai, 0x2F4));
    const float axis[3] = {0.0f, kOne_00d7a24c, 0.0f};
    {
        float* const cyan = debug_cyan(host);
        const float c[3] = {at<float>(ai, 0x18C), kFive_00ce3850, at<float>(ai, 0x190)};
        host.circle_around_axis_00860bb0(c, axis, at<float>(ai, 0x3C8), cyan,
                                         kDebugCircleSegments);
    }
    {
        float* const cyan = debug_cyan(host);
        const float c[3] = {at<float>(ai, 0x194), kFive_00ce3850, at<float>(ai, 0x198)};
        host.circle_around_axis_00860bb0(c, axis, at<float>(ai, 0x3C8), cyan,
                                         kDebugCircleSegments);
    }
    host.avoid_zone_debug_draw_selected_segments_004166e0(ptr(ai, 0xA3C), debug_red(host));
    host.avoid_zone_debug_draw_selected_segments_004166e0(ptr(ai, 0xA5C), debug_yellow(host));
    if (at<std::uint8_t>(ai, 0xA90) != 0) {
        float* const a = ship_ai_debug_colour_record_a_009d5e10(host);
        float* const b = ship_ai_debug_colour_record_b_009d5fd0(host);
        host.wire_sphere_00860be0(&at<float>(ai, 0xA94), b, kFive_00ce3850, a, 1);
    }
    for (std::int32_t i = 0; i < at<std::int32_t>(ai, 0x604); ++i) {
        const std::size_t slot = 0x608 + 4 * static_cast<std::size_t>(i);
        ship_ai_debug_draw_box_lines_009d7260(host, ptr_at(ai, slot));
    }
}

// ---------------------------------------------------------------------------
// Approach and task saves (cc13-seg67C). Each mirrors the stream constructor's
// field list: 009A6D40 (dogfight), 009ABBE0 (drop kamikaze) and 009AFFF0 (land).
// Every field goes through archive slot +0Ch as write(name, value). The approach-level
// saves at 009AA570, 009AE470 and 009B3B40 (approach save plus "_state_moveTo") have
// no rel32 or absolute-dword reference in the image and are not reconstructed.

// 009AA410 (009AA410-009AA561). 00BD5010(archive, block, {0,"_targetLock"})
// (009AA43C), 009FACC0(ECX = a+2Ch, block) (009AA444); {0,"squadName"} = {4, [a+CCh]}
// (009AA471); {0,"inattackrange"} = {3, byte a+D0h} (009AA4A8); {0,"targetfindangle"} =
// {2, a+DCh} (009AA4E3); {0,"targetfindweight"} = {2, a+E0h} (009AA51E);
// {0,"washittimer"} = {2, a+E8h} (009AA559). RET 4.
// UNCERTAINTY: each byte record stores only the low byte (MOV byte [EAX+4]); the payload's
// upper three bytes are stack garbage in the image and zero here.
void bot_approach_dogfight_save_009aa410(Cc13Seg67bHost& host, void* a, void* archive) {
    std::uint32_t ref[3] = {};
    host.archive_writer_ref_open_00bd5010(archive, ref,
                                          name_record(kStrTargetLockUnderscore_00d1f624));
    host.approach_ref_save_009facc0(ptr(a, 0x2C), ref);
    host.archive_vcall_0c_write(archive, name_record(kStrSquadName_00d1f888),
                                record(4, at<std::uint32_t>(a, 0xCC)));
    host.archive_vcall_0c_write(archive, name_record(kStrInAttackRange_00d1f878),
                                record(3, at<std::uint8_t>(a, 0xD0)));
    host.archive_vcall_0c_write(archive, name_record(kStrTargetFindAngle_00d1f868),
                                record(2, at<std::uint32_t>(a, 0xDC)));
    host.archive_vcall_0c_write(archive, name_record(kStrTargetFindWeight_00d1f854),
                                record(2, at<std::uint32_t>(a, 0xE0)));
    host.archive_vcall_0c_write(archive, name_record(kStrWasHitTimer_00d1f848),
                                record(2, at<std::uint32_t>(a, 0xE8)));
}

// 009AA5C0 (009AA5C0-009AA625). 0099D060(ECX = task, archive) (009AA5CA); 009AA410(ECX =
// task+3F8h, archive) (009AA5D8); 00BD5010(archive, block, {0,"_state_moveTo"})
// (009AA5FF), 009BEAA0(ECX = task+3F8h+118h, block) (009AA60A); 00411450(ECX =
// task+4FCh, task+30Ch, archive) (009AA61D). RET 4.
void bot_task_dogfight_save_009aa5c0(Cc13Seg67bHost& host, void* task, void* archive) {
    bot_task_save_base_0099d060(host, task, archive);
    bot_approach_dogfight_save_009aa410(host, ptr(task, 0x3F8), archive);
    std::uint32_t ref[3] = {};
    host.archive_writer_ref_open_00bd5010(archive, ref, name_record(kStrStateMoveTo));
    bot_approach_save_altitude_leg_009beaa0(host, ptr(task, 0x3F8 + 0x118), ref);
    host.bot_state_registry_write_current_00411450(ptr(task, 0x4FC), ptr(task, 0x30C), archive);
}

// 009AE2D0 (009AE2D0-009AE465). 00BD5010(archive, block, {0,"targetLock"}) (009AE2FB),
// 009FACC0(ECX = a+30h, block) (009AE303); {0,"startDist"} = {2, a+ACh} (009AE341);
// {0,"attackDist"} = {2, a+B0h} (009AE37C); {0,"targetDist"} = {2, a+B4h} (009AE3B7);
// {0,"insideAttackRange"} = {3, byte a+C0h} (009AE3EE); {0,"targetHdg"} = {2, a+B8h}
// (009AE429); {0,"currentAvoidFlag"} = {1, a+BCh} (009AE460). RET 4.
// UNCERTAINTY: each byte record stores only the low byte (MOV byte [EAX+4]); the payload's
// upper three bytes are stack garbage in the image and zero here.
void bot_approach_drop_kamikaze_save_009ae2d0(Cc13Seg67bHost& host, void* a, void* archive) {
    std::uint32_t ref[3] = {};
    host.archive_writer_ref_open_00bd5010(archive, ref, name_record(kStrTargetLockPlain_00d1fb5c));
    host.approach_ref_save_009facc0(ptr(a, 0x30), ref);
    host.archive_vcall_0c_write(archive, name_record(kStrStartDist_00d1fb50),
                                record(2, at<std::uint32_t>(a, 0xAC)));
    host.archive_vcall_0c_write(archive, name_record(kStrAttackDist_00d1fb44),
                                record(2, at<std::uint32_t>(a, 0xB0)));
    host.archive_vcall_0c_write(archive, name_record(kStrTargetDist_00d1fb38),
                                record(2, at<std::uint32_t>(a, 0xB4)));
    host.archive_vcall_0c_write(archive, name_record(kStrInsideAttackRange_00d1fb24),
                                record(3, at<std::uint8_t>(a, 0xC0)));
    host.archive_vcall_0c_write(archive, name_record(kStrTargetHdg_00d1fb18),
                                record(2, at<std::uint32_t>(a, 0xB8)));
    host.archive_vcall_0c_write(archive, name_record(kStrCurrentAvoidFlag_00d1fb04),
                                record(1, at<std::uint32_t>(a, 0xBC)));
}

// 009AE4C0 (009AE4C0-009AE525). 0099D060(ECX = task, archive) (009AE4CA); 009AE2D0(ECX =
// task+3F8h, archive) (009AE4D8); 00BD5010(archive, block, {0,"_state_moveTo"})
// (009AE4FF), 009BEAA0(ECX = task+3F8h+D8h, block) (009AE50A); 00411450(ECX = task+4BCh,
// task+30Ch, archive) (009AE51D). RET 4.
void bot_task_drop_kamikaze_save_009ae4c0(Cc13Seg67bHost& host, void* task, void* archive) {
    bot_task_save_base_0099d060(host, task, archive);
    bot_approach_drop_kamikaze_save_009ae2d0(host, ptr(task, 0x3F8), archive);
    std::uint32_t ref[3] = {};
    host.archive_writer_ref_open_00bd5010(archive, ref, name_record(kStrStateMoveTo));
    bot_approach_save_altitude_leg_009beaa0(host, ptr(task, 0x3F8 + 0xD8), ref);
    host.bot_state_registry_write_current_00411450(ptr(task, 0x4BC), ptr(task, 0x30C), archive);
}

// 009B37F0 (009B37F0-009B38FF). {0,"airBaseEntity"} = {4, [a+30h]} (009B381E);
// 00BD5010(archive, block, {0,"updateLandQueueTimer"}) (009B3842), 00410E00(ECX =
// a+ACh, block) (009B384D); {0,"approachAngle"} = {2, a+54h} (009B3888);
// {0,"approachPitch"} = {2, a+58h} (009B38C0); {0,"standbyheightadd"} = {2, a+60h}
// (009B38F8). RET 4.
void bot_approach_land_save_009b37f0(Cc13Seg67bHost& host, void* a, void* archive) {
    host.archive_vcall_0c_write(archive, name_record(kStrAirBaseEntity_00d1fe78),
                                record(4, at<std::uint32_t>(a, 0x30)));
    std::uint32_t ref[3] = {};
    host.archive_writer_ref_open_00bd5010(archive, ref,
                                          name_record(kStrUpdateLandQueueTimer_00d1fe60));
    host.time_freq_pair_write_00410e00(ptr(a, 0xAC), ref);
    host.archive_vcall_0c_write(archive, name_record(kStrApproachAngle_00d1fe50),
                                record(2, at<std::uint32_t>(a, 0x54)));
    host.archive_vcall_0c_write(archive, name_record(kStrApproachPitch_00d1fe40),
                                record(2, at<std::uint32_t>(a, 0x58)));
    host.archive_vcall_0c_write(archive, name_record(kStrStandbyHeightAdd_00d1fe2c),
                                record(2, at<std::uint32_t>(a, 0x60)));
}

// 009B3B90 (009B3B90-009B3BF5). 0099D060(ECX = task, archive) (009B3B9A); 009B37F0(ECX =
// task+3F8h, archive) (009B3BA8); 00BD5010(archive, block, {0,"_state_moveTo"})
// (009B3BCF), 009BEAA0(ECX = task+3F8h+CCh, block) (009B3BDA); 00411450(ECX = task+4B0h,
// task+30Ch, archive) (009B3BED). RET 4.
void bot_task_land_save_009b3b90(Cc13Seg67bHost& host, void* task, void* archive) {
    bot_task_save_base_0099d060(host, task, archive);
    bot_approach_land_save_009b37f0(host, ptr(task, 0x3F8), archive);
    std::uint32_t ref[3] = {};
    host.archive_writer_ref_open_00bd5010(archive, ref, name_record(kStrStateMoveTo));
    bot_approach_save_altitude_leg_009beaa0(host, ptr(task, 0x3F8 + 0xCC), ref);
    host.bot_state_registry_write_current_00411450(ptr(task, 0x4B0), ptr(task, 0x30C), archive);
}

// 0099A660 (0099A660-0099A71C), reached through the thunk 0099A870. The bot's active
// task array {data +58h, count +5Ch} and retired array {data +64h, count +68h,
// capacity +6Ch}. Find the first active slot equal to task (0099A673-0099A680); when
// the retired array is full, capacity = 2 * capacity + 2 (stored before the
// allocation), 00BF55BE -> operator new(capacity * 4), the count entries copied, the
// old buffer through 00BF6989 -> _free when non-null (0099A68A-0099A6D7); append the
// found slot's value (0099A6DB-0099A6EC); shift the active entries after it down by
// one and decrement +5Ch (0099A6F0-0099A716). RET 4.
void pilot_bot_retire_task_0099a660(Cc13Seg67bHost& host, void* bot, void* task) {
    auto* const active = reinterpret_cast<std::uintptr_t*>(ptr_at(bot, 0x58));
    const std::uint32_t n = at<std::uint32_t>(bot, 0x5C);
    const std::uintptr_t key = reinterpret_cast<std::uintptr_t>(task);
    std::uint32_t i = 0;
    while (i != n && active[i] != key) ++i;
    // A task not in the array leaves the image reading the slot past the end and
    // shifting without bound (the erase loop's end test can never match); the
    // callers pass a held task, and this body stops there.
    if (i == n) return;
    std::uint32_t count = at<std::uint32_t>(bot, 0x68);
    if (count == at<std::uint32_t>(bot, 0x6C)) {
        const std::uint32_t capacity = count + count + 2;
        at<std::uint32_t>(bot, 0x6C) = capacity;
        auto* const grown = static_cast<std::uintptr_t*>(host.operator_new_00bf681b(capacity * 4));
        auto* const old = reinterpret_cast<std::uintptr_t*>(ptr_at(bot, 0x64));
        for (std::uint32_t k = 0; k < count; ++k) {
            if (grown != nullptr) grown[k] = old[k];
        }
        if (old != nullptr) host.crt_free_00bf65ac(old);
        at<std::uintptr_t>(bot, 0x64) = reinterpret_cast<std::uintptr_t>(grown);
    }
    auto* const retired = reinterpret_cast<std::uintptr_t*>(ptr_at(bot, 0x64));
    if (reinterpret_cast<std::uintptr_t>(retired) + 4u * count != 0) retired[count] = active[i];
    at<std::uint32_t>(bot, 0x68) = count + 1;
    for (std::uint32_t k = i; k + 1 < n; ++k) active[k] = active[k + 1];
    at<std::uint32_t>(bot, 0x5C) = n - 1;
}

// 009B84E0 (009B84E0-009B8759). The field list of 009B4690: 00BD5010 {0,"targetLock"}
// (009B850B) + 009FACC0(ECX = a+30h, block) (009B8513); releaseTime {2,+A8h} (009B8551),
// startDist {2,+B4h} (009B858C), releaseError {2,+C4h} (009B85C7), attackDist {2,+B8h}
// (009B8602), targetDist {2,+BCh} (009B863D), bayDoorState {3,+CCh} (009B8674),
// insideAttackRange {3,+CDh} (009B86AB), targetHdg {2,+C0h} (009B86E6),
// currentAvoidFlag {1,+C8h} (009B871D), calcHitPos {5, &a+D0h} (009B8754). RET 4.
// UNCERTAINTY: each byte record stores only the low byte (MOV byte [EAX+4]); the payload's
// upper three bytes are stack garbage in the image and zero here.
void bot_approach_level_bomb_save_009b84e0(Cc13Seg67bHost& host, void* a, void* archive) {
    std::uint32_t ref[3] = {};
    host.archive_writer_ref_open_00bd5010(archive, ref, name_record(kStrTargetLockPlain_00d1fb5c));
    host.approach_ref_save_009facc0(ptr(a, 0x30), ref);
    host.archive_vcall_0c_write(archive, name_record(kStrReleaseTime_00d200d8),
                                record(2, at<std::uint32_t>(a, 0xA8)));
    host.archive_vcall_0c_write(archive, name_record(kStrStartDist_00d1fb50),
                                record(2, at<std::uint32_t>(a, 0xB4)));
    host.archive_vcall_0c_write(archive, name_record(kStrReleaseError_00d200c8),
                                record(2, at<std::uint32_t>(a, 0xC4)));
    host.archive_vcall_0c_write(archive, name_record(kStrAttackDist_00d1fb44),
                                record(2, at<std::uint32_t>(a, 0xB8)));
    host.archive_vcall_0c_write(archive, name_record(kStrTargetDist_00d1fb38),
                                record(2, at<std::uint32_t>(a, 0xBC)));
    host.archive_vcall_0c_write(archive, name_record(kStrBayDoorState_00d200b8),
                                record(3, at<std::uint8_t>(a, 0xCC)));
    host.archive_vcall_0c_write(archive, name_record(kStrInsideAttackRange_00d1fb24),
                                record(3, at<std::uint8_t>(a, 0xCD)));
    host.archive_vcall_0c_write(archive, name_record(kStrTargetHdg_00d1fb18),
                                record(2, at<std::uint32_t>(a, 0xC0)));
    host.archive_vcall_0c_write(archive, name_record(kStrCurrentAvoidFlag_00d1fb04),
                                record(1, at<std::uint32_t>(a, 0xC8)));
    host.archive_vcall_0c_write(archive, name_record(kStrCalcHitPos_00d200ac),
                                record(5, reinterpret_cast<std::uintptr_t>(ptr(a, 0xD0))));
}

// 009B87B0 (009B87B0-009B8815). 0099D060(ECX = task, archive) (009B87BA); 009B84E0(ECX =
// task+3F8h, archive) (009B87C8); 00BD5010(archive, block, {0,"_state_moveTo"})
// (009B87EF), 009BEAA0(ECX = task+3F8h+F0h, block) (009B87FA); 00411450(ECX = task+4D4h,
// task+30Ch, archive) (009B880D). RET 4. The approach-level twin 009B8760 has no
// reference in the image.
void bot_task_level_bomb_save_009b87b0(Cc13Seg67bHost& host, void* task, void* archive) {
    bot_task_save_base_0099d060(host, task, archive);
    bot_approach_level_bomb_save_009b84e0(host, ptr(task, 0x3F8), archive);
    std::uint32_t ref[3] = {};
    host.archive_writer_ref_open_00bd5010(archive, ref, name_record(kStrStateMoveTo));
    bot_approach_save_altitude_leg_009beaa0(host, ptr(task, 0x3F8 + 0xF0), ref);
    host.bot_state_registry_write_current_00411450(ptr(task, 0x4D4), ptr(task, 0x30C), archive);
}

namespace {
constexpr std::uint32_t kKamikazeApproachVtable_00d1fd34 = 0x00D1FD34u;
constexpr std::uint32_t kKamikazeRegistryVtable_00d1fd30 = 0x00D1FD30u;
constexpr std::uint32_t kKamikazeGoTowardsVtable_00d1fb84 = 0x00D1FB84u;
constexpr std::uint32_t kKamikazeAimVtable_00d1fba0 = 0x00D1FBA0u;
constexpr std::uint32_t kStrKamikazeGoTowards_00d1fd1c = 0x00D1FD1Cu;  // "Kamikaze/gotowards"
constexpr std::uint32_t kStrKamikazeAim_00d1fd0c = 0x00D1FD0Cu;        // "Kamikaze/aim"
constexpr std::uint32_t kStrKamikazeFollow_00d1fcfc = 0x00D1FCFCu;     // "Kamikaze/follow"
constexpr std::uint32_t kStrKamikazePrepare_00d1fce8 = 0x00D1FCE8u;    // "Kamikaze/prepare"
constexpr std::uint32_t kStrKamikazeMoveTo_00d1fcd8 = 0x00D1FCD8u;     // "Kamikaze/moveto"
constexpr float kFollowArg_00ce3d08 = 100.0f;      // [00CE3D08]
constexpr float kGoTowards18_00cffd60 = 2000.0f;   // [00CFFD60]
}  // namespace

// 009AED20 (009AED20-009AEEB1). EH frame 00CAE95C. 009AB920(ECX = block, [bot+50h],
// target) (009AED4F); 00411E20(block+DCh) (009AED62); [block] = 00D1FD34, [block+DCh] =
// 00D1FD30 (009AEDA6, 009AEDAC); 009C2AC0(ECX = block+F0h, block, target, +4Ch, +4Ch,
// +48h) (009AEDB3; the three floats go out as [esp] = +4Ch, [esp+4] = +4Ch, [esp+8] =
// +48h, the second spilled through the target's own argument slot); 009C2980(ECX =
// block+12Ch, block, 100.0) (009AEDCE) and 009C2980(ECX = block+1C4h, block, 100.0)
// (009AEDE9); the gotowards state at +25Ch inline: vtable 00D1FB84, +4h = block,
// +8h..+14h = 0, +18h = 2000.0 [00CFFD60], bytes +1Ch/+1Dh = 0, +20h = 0.0
// (009AEDEE-009AEE1F); the aim state at +280h inline: vtable 00D1FBA0, +4h = block,
// +8h..+14h = 0, bytes +18h/+19h = 0 (009AEE24-009AEE42); 00411E70(ECX = block+DCh)
// registers gotowards (+25Ch), aim (+280h), follow (+12Ch), prepare (+1C4h) and moveto
// (+F0h) in that order (009AEE52-009AEE98). RET 8, EAX = block.
// RNG: the draws of 009AB920 and 009C2AC0 are the hooks'; then one shared-stream
// U(0, 0.6) in each 009C2980, follow (+12Ch) first and prepare (+1C4h) second.
// Whole body: cc9 models this construction inline in kz_install_009af720
// (game_hosts_units.cpp ~26514, the 009AED20 note at ~26556, kz_moveto_near /
// kz_moveto_speed_range).
// UNCERTAINTY: EH unwinding not modelled.
void* bot_approach_kamikaze_construct_009aed20(Cc13Seg67bHost& host, void* block, void* bot,
                                               void* target) {
    host.bot_approach_kamikaze_construct_base_009ab920(block, ptr_at(bot, 0x50), target);
    host.bot_state_registry_construct_00411e20(ptr(block, 0xDC));
    const float f48 = at<float>(block, 0x48);
    const float f4c = at<float>(block, 0x4C);
    at<std::uint32_t>(block, 0x000) = kKamikazeApproachVtable_00d1fd34;
    at<std::uint32_t>(block, 0x0DC) = kKamikazeRegistryVtable_00d1fd30;
    host.bot_state_move_to_construct_009c2ac0(ptr(block, 0xF0), block, target, f4c, f4c, f48);
    bot_state_follow_construct_009c2980(host, ptr(block, 0x12C), block, kFollowArg_00ce3d08);
    bot_state_follow_construct_009c2980(host, ptr(block, 0x1C4), block, kFollowArg_00ce3d08);
    void* const go = ptr(block, 0x25C);
    at<std::uintptr_t>(go, 0x04) = reinterpret_cast<std::uintptr_t>(block);
    at<std::uint32_t>(go, 0x0C) = 0;
    at<std::uint32_t>(go, 0x10) = 0;
    at<std::uint32_t>(go, 0x14) = 0;
    at<float>(go, 0x18) = kGoTowards18_00cffd60;
    at<std::uint32_t>(go, 0x08) = 0;
    at<std::uint32_t>(go, 0x00) = kKamikazeGoTowardsVtable_00d1fb84;
    at<std::uint8_t>(go, 0x1C) = 0;
    at<std::uint8_t>(go, 0x1D) = 0;
    at<float>(go, 0x20) = 0.0f;
    void* const aim = ptr(block, 0x280);
    at<std::uintptr_t>(aim, 0x04) = reinterpret_cast<std::uintptr_t>(block);
    at<std::uint32_t>(aim, 0x0C) = 0;
    at<std::uint32_t>(aim, 0x10) = 0;
    at<std::uint32_t>(aim, 0x14) = 0;
    at<std::uint32_t>(aim, 0x08) = 0;
    at<std::uint32_t>(aim, 0x00) = kKamikazeAimVtable_00d1fba0;
    at<std::uint8_t>(aim, 0x18) = 0;
    at<std::uint8_t>(aim, 0x19) = 0;
    void* const reg = ptr(block, 0xDC);
    host.state_registry_add_00411e70(reg, kStrKamikazeGoTowards_00d1fd1c, go);
    host.state_registry_add_00411e70(reg, kStrKamikazeAim_00d1fd0c, aim);
    host.state_registry_add_00411e70(reg, kStrKamikazeFollow_00d1fcfc, ptr(block, 0x12C));
    host.state_registry_add_00411e70(reg, kStrKamikazePrepare_00d1fce8, ptr(block, 0x1C4));
    host.state_registry_add_00411e70(reg, kStrKamikazeMoveTo_00d1fcd8, ptr(block, 0xF0));
    return block;
}

// 009C1C30 (009C1C30-009C1DBB). EH frame 00CAF71B. 009F9CE0(ECX = a, unit,
// tuning+37Ch) (009C1C68); [a] = 00D209CC; a+2Ch = U(0, tuning+540h) on stream 1
// (009C1C8F); 004BEF00(ECX = a+30h, target->vtable[0]()) (009C1CA9), [a+30h] =
// 00CE3CF0; a+48h..+50h = target+4h..+0Ch; a+54h = 0.5 [00CE3800]; a+58h = -U(0, 0.5)
// on stream 1 (009C1CEE); a+60h = 0, byte a+5Ch = 0, a+68h = 50.0 [00CEB4D4]
// (009C1CFA-009C1D0D). unit set and 007B8AD0(unit) (the flight leader): 00414DB0(unit)
// unless byte unit+C8h; d = 00414C60(unit+FCh - a+48h, unit+104h - a+50h) (each
// difference spilled to a float; x*x + y*y rounded to float, > double 1e-10 ? sqrt :
// 0, JBE); t = [[a+8h]+268h]; t > d (FCOMI, JA) keeps d, else d = t (so NaN -> t);
// a+60h = d / [[a+8h]+18Ch] (009C1D18-009C1D9A). 009BEBA0(a) (009C1D9F). RET 8, EAX = a.
// RNG: two stream-1 draws, +2Ch then +58h, after 009F9CE0's (the hook's).
// Whole body: cc9 models this construction inline in
// GameUnitsHost::Impl::run_moveto_task_install_009c3000 with per-name stand-ins
// ("#mt2c", "#mt58").
// UNCERTAINTY: 24-bit precision modelled; EH unwinding not modelled.
void* bot_approach_moveto_construct_009c1c30(Cc13Seg67bHost& host, void* a, void* unit,
                                             void* target) {
    void* tuning = host.game_tuning_singleton_0042e740();
    host.bot_approach_construct_speed_reference_009f9ce0(a, unit, at<float>(tuning, 0x37C));
    at<std::uint32_t>(a, 0x00) = kMoveToApproachVtable_00d209cc;
    tuning = host.game_tuning_singleton_0042e740();
    at<float>(a, 0x2C) = host.random_uniform_00bd2f10(1, 0.0f, at<float>(tuning, 0x540));
    host.observer_construct_004bef00(ptr(a, 0x30), host.object_vcall_00(target));
    at<std::uint32_t>(a, 0x30) = kObservedEndpointVtable_00ce3cf0;
    at<float>(a, 0x48) = at<float>(target, 0x04);
    at<float>(a, 0x4C) = at<float>(target, 0x08);
    at<float>(a, 0x50) = at<float>(target, 0x0C);
    at<float>(a, 0x54) = kHalf_00ce3800;
    at<float>(a, 0x58) = -host.random_uniform_00bd2f10(1, 0.0f, kHalf_00ce3800);
    at<float>(a, 0x60) = 0.0f;
    at<std::uint8_t>(a, 0x5C) = 0;
    at<float>(a, 0x68) = kFifty_00ceb4d4;
    if (unit != nullptr && host.unit_is_squadron_flight_leader_007b8ad0(unit)) {
        if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);
        const float dx = at<float>(unit, 0xFC) - at<float>(a, 0x48);
        const float dz = at<float>(unit, 0x104) - at<float>(a, 0x50);
        const float xx = dx * dx;
        const float zz = dz * dz;
        const float s = xx + zz;
        float d = static_cast<double>(s) > kTiny_00ce3820
                      ? static_cast<float>(std::sqrt(static_cast<double>(s)))
                      : 0.0f;
        void* const row = ptr_at(a, 0x08);
        const float t = at<float>(row, 0x268);
        if (!(t > d)) d = t;
        at<float>(a, 0x60) = d / at<float>(row, 0x18C);
    }
    bot_approach_refresh_target_009beba0(host, a);
    return a;
}

// 009CA2B0 (009CA2B0-009CA30F). EH frame 00CAFDFB. t = operator new(564h) (009CA2CE);
// null -> 0; else 009C9D00(ECX = t, bot) (009CA2E9), EAX = its result.
// UNCERTAINTY: EH unwinding (the delete on a throwing constructor) not modelled.
void* bot_task_make_retreat_009ca2b0(Cc13Seg67bHost& host, void* bot) {
    void* const t = host.operator_new_00bf681b(0x564);
    if (t == nullptr) return nullptr;
    return host.bot_task_retreat_construct_009c9d00(t, bot);
}

// 009CA780 (009CA780-009CA814). row = [a+14h], ref = a+58h: ref+50h, +4Ch, +48h =
// row+E4h (StrafeTargetPointSelectPrec, in that store order), byte ref+41h = 1
// (009CA786-009CA7A0); ref+64h..+70h = row+E8h..+F4h (009CA7A7-009CA7D2); 009FA260(ECX
// = ref) (009CA7D7); 007B7870(ECX = [a+20h], [a+70h], (a+80h, a+84h, a+88h) by value)
// (009CA80E). Whole body: cc9 models it inline in the strafe go-towards enter (the
// "009CA866 -> 009CA780" site, game_hosts_units.cpp ~24450) with the pick on a stand-in seed and without the
// 007B7870 tail.
void bot_approach_strafe_rearm_target_point_009ca780(Cc13Seg67bHost& host, void* a) {
    void* const row = ptr_at(a, 0x14);
    void* const ref = ptr(a, 0x58);
    const float spread = at<float>(row, 0xE4);
    at<float>(ref, 0x50) = spread;
    at<float>(ref, 0x4C) = spread;
    at<float>(ref, 0x48) = spread;
    at<std::uint8_t>(ref, 0x41) = 1;
    at<float>(ref, 0x64) = at<float>(row, 0xE8);
    at<float>(ref, 0x68) = at<float>(row, 0xEC);
    at<float>(ref, 0x6C) = at<float>(row, 0xF0);
    at<float>(ref, 0x70) = at<float>(row, 0xF4);
    host.approach_ref_pick_aim_offset_009fa260(ref);
    const float point[3] = {at<float>(a, 0x80), at<float>(a, 0x84), at<float>(a, 0x88)};
    host.opportunistic_rocket_set_target_007b7870(ptr_at(a, 0x20), ptr_at(a, 0x70), point);
}

// 009CD300 (009CD300-009CD365). EH frame 00CB004B. t = operator new(6FCh) (009CD321);
// null -> 0; else 009CC230(ECX = t, bot, target) (009CD33D).
// UNCERTAINTY: EH unwinding not modelled.
void* bot_task_make_strafe_009cd300(Cc13Seg67bHost& host, void* bot, void* target) {
    void* const t = host.operator_new_00bf681b(0x6FC);
    if (t == nullptr) return nullptr;
    return bot_task_strafe_construct_009cc230(host, t, bot, target);
}

// 009D4E30 (009D4E30-009D4E95). EH frame 00CB041B. t = operator new(7DCh) (009D4E51);
// null -> 0; else 009D3050(ECX = t, bot, target) (009D4E6D).
// UNCERTAINTY: EH unwinding not modelled.
void* bot_task_make_torpedo_009d4e30(Cc13Seg67bHost& host, void* bot, void* target) {
    void* const t = host.operator_new_00bf681b(0x7DC);
    if (t == nullptr) return nullptr;
    return bot_task_torpedo_construct_009d3050(host, t, bot, target);
}

// 009DDBA0 (009DDBA0-009DDBB2), labelled `~SubAllocator` by Ghidra's library match but
// game code: `eh vector destructor iterator`(ai+60h, 68h, 2, 009DBDB0) (00BF7C6E at
// 009DDBAD), which destroys the two path plans last to first. Reached only from the
// unwind funclets 00CB0630 and 00CB06C0.
void ship_ai_destruct_path_plans_009ddba0(Cc13Seg67bHost& host, void* ai) {
    for (std::size_t i = 2; i-- > 0;) {
        host.ship_ai_path_plan_destruct_009dbdb0(ptr(ai, 0x60 + 0x68 * i));
    }
}

namespace {
constexpr std::uint32_t kShipAiStateBaseVtable_00d218c0 = 0x00D218C0u;
constexpr std::uint32_t kShipAiStateSubVtable_00d21880 = 0x00D21880u;
constexpr std::uint32_t kSubAttackRegistryVtable_00d2198c = 0x00D2198Cu;
constexpr std::uint32_t kSubAttackVtable_00d2195c = 0x00D2195Cu;
constexpr std::uint32_t kSubAttackApproachVtable_00d218f0 = 0x00D218F0u;
constexpr std::uint32_t kSubAttackFireVtable_00d21920 = 0x00D21920u;
constexpr std::uint32_t kStrApproach_00d21950 = 0x00D21950u;  // "approach"
constexpr std::uint32_t kStrFire_00ce6798 = 0x00CE6798u;      // "fire"
constexpr std::uint32_t kAttackMoveVtable_00d219d0 = 0x00D219D0u;
constexpr std::uint32_t kAttackMoveApproachVtable_00d21994 = 0x00D21994u;
constexpr std::uint32_t kAttackMove14c0Vtable_00d2174c = 0x00D2174Cu;
constexpr std::uint32_t kAttackMove14ccVtable_00d2177c = 0x00D2177Cu;
constexpr std::uint32_t kAttackMove14e0Vtable_00d217ac = 0x00D217ACu;
constexpr std::uint32_t kAttackMove14f4Vtable_00d2171c = 0x00D2171Cu;

// The 1Ch-byte state base the two sub_attack children and the parent share:
// +4h = owner, +0Ch..+14h = 0, +1Ch = 0, byte +18h = 1, +8h = 00D21880.
void ship_ai_state_base_init(void* s, void* owner) {
    at<std::uintptr_t>(s, 0x04) = reinterpret_cast<std::uintptr_t>(owner);
    at<std::uint32_t>(s, 0x0C) = 0;
    at<std::uint32_t>(s, 0x10) = 0;
    at<std::uint32_t>(s, 0x14) = 0;
    at<std::uint32_t>(s, 0x1C) = 0;
    at<std::uint8_t>(s, 0x18) = 1;
    at<std::uint32_t>(s, 0x08) = kShipAiStateSubVtable_00d21880;
}
}  // namespace

// 009E4F90 (009E4F90-009E50BD). EH frame 00CB07A7. Parent p: [p] = 00D218C0 and the
// state base on (p, brain) (009E4FAF-009E4FCF); 00411E20(p+20h) (009E4FDF), [p+20h] =
// 00D2198C; [p] = 00D2195C (009E4FED). Child +34h: the state base on (brain), [+34h] =
// 00D218F0 (009E4FF3-009E500D). Child +58h: the same, [+58h] = 00D21920 (009E5017-
// 009E5034). p+C8h = 1.0 (009E5059); p+CCh = -U(0, 1) on stream 1 (00BD2F10 at
// 009E5061); p+D0h = 00D05704, p+D4h = &p+34h, then (p+34h)->vtable[4]() (009E5068-
// 009E5085); 00411E70(p+20h) registers "approach" (+34h) and "fire" (+58h) (009E5097,
// 009E50A4). RET 4, EAX = p.
// Every ship's brain reaches it: 00810DD0 -> 009F3F20 -> 009F3BA0 -> 009F39C0
// (009F3AC3), with no kind gate.
// UNCERTAINTY: EH unwinding not modelled.
void* ship_ai_sub_attack_construct_009e4f90(Cc13Seg67bHost& host, void* p, void* brain) {
    at<std::uintptr_t>(p, 0x04) = reinterpret_cast<std::uintptr_t>(brain);
    at<std::uint32_t>(p, 0x00) = kShipAiStateBaseVtable_00d218c0;
    ship_ai_state_base_init(p, brain);
    host.bot_state_registry_construct_00411e20(ptr(p, 0x20));
    at<std::uint32_t>(p, 0x20) = kSubAttackRegistryVtable_00d2198c;
    at<std::uint32_t>(p, 0x00) = kSubAttackVtable_00d2195c;
    void* const approach = ptr(p, 0x34);
    ship_ai_state_base_init(approach, brain);
    at<std::uint32_t>(approach, 0x00) = kSubAttackApproachVtable_00d218f0;
    void* const fire = ptr(p, 0x58);
    ship_ai_state_base_init(fire, brain);
    at<std::uint32_t>(fire, 0x00) = kSubAttackFireVtable_00d21920;
    at<float>(p, 0xC8) = kOne_00d7a24c;
    const float r = host.random_uniform_00bd2f10(1, 0.0f, 1.0f);
    at<std::uint32_t>(p, 0xD0) = kBotTaskSubVtable30C;
    at<float>(p, 0xCC) = -r;
    at<std::uintptr_t>(p, 0xD4) = reinterpret_cast<std::uintptr_t>(approach);
    host.state_vcall_04_enter(approach);
    host.state_registry_add_00411e70(ptr(p, 0x20), kStrApproach_00d21950, approach);
    host.state_registry_add_00411e70(ptr(p, 0x20), kStrFire_00ce6798, fire);
    return p;
}

// 009E5CA0 (009E5CA0-009E5CEC). EH frame 00CB08E8. [sub+4h] = owner, [sub] = 00D21994,
// 009E5530(ECX = sub+8h, owner) (009E5CD6). RET 4, EAX = sub.
// RNG: 009E5530 is a hook; its binding must take the sixty stream-1 U(0, 2.0
// [00CE3958]) draws (009E55A1, one per ring slot +48h).
// UNCERTAINTY: EH unwinding not modelled.
void* ship_ai_attackmove_approach_construct_009e5ca0(Cc13Seg67bHost& host, void* sub,
                                                     void* owner) {
    at<std::uintptr_t>(sub, 0x04) = reinterpret_cast<std::uintptr_t>(owner);
    at<std::uint32_t>(sub, 0x00) = kAttackMoveApproachVtable_00d21994;
    host.ship_ai_attackmove_ring_construct_009e5530(ptr(sub, 0x08), owner);
    return sub;
}

// 009E8450 (009E8450-009E8531). EH frame 00CB096B. [s+4h] = owner, [s] = 00D219D0;
// 009E5CA0(ECX = s+8h, owner) (009E8486); the sub-states +14C0h (00D2174C, byte +14C8h
// = 0), +14CCh (00D2177C), +14E0h (00D217AC) and +14F4h (00D2171C), each with owner at
// +4h (009E848B-009E84CA); s+14FCh = 1.0; s+1500h = -U(0, 1) on stream 1 (00BD2F10 at
// 009E84F8); s+1504h = 00D05704, s+1508h = &s+14F4h, then (s+14F4h)->vtable[4]()
// (009E84FF-009E851C). RET 4, EAX = s.
// RNG: 009E5530's sixty U(0, 2.0) (009E55A1), then 009E8450's one. Every ship's brain reaches
// it through 009F39C0 (009F3AB2).
// UNCERTAINTY: EH unwinding not modelled.
void* ship_ai_attackmove_construct_009e8450(Cc13Seg67bHost& host, void* s, void* owner) {
    const std::uintptr_t o = reinterpret_cast<std::uintptr_t>(owner);
    at<std::uintptr_t>(s, 0x04) = o;
    at<std::uint32_t>(s, 0x00) = kAttackMoveVtable_00d219d0;
    ship_ai_attackmove_approach_construct_009e5ca0(host, ptr(s, 0x08), owner);
    at<std::uintptr_t>(s, 0x14C4) = o;
    at<std::uint32_t>(s, 0x14C0) = kAttackMove14c0Vtable_00d2174c;
    at<std::uint8_t>(s, 0x14C8) = 0;
    at<std::uintptr_t>(s, 0x14D0) = o;
    at<std::uint32_t>(s, 0x14CC) = kAttackMove14ccVtable_00d2177c;
    at<std::uintptr_t>(s, 0x14E4) = o;
    at<std::uint32_t>(s, 0x14E0) = kAttackMove14e0Vtable_00d217ac;
    void* const last = ptr(s, 0x14F4);
    at<std::uintptr_t>(last, 0x04) = o;
    at<std::uint32_t>(last, 0x00) = kAttackMove14f4Vtable_00d2171c;
    at<float>(s, 0x14FC) = kOne_00d7a24c;
    const float r = host.random_uniform_00bd2f10(1, 0.0f, 1.0f);
    at<std::uint32_t>(s, 0x1504) = kBotTaskSubVtable30C;
    at<float>(s, 0x1500) = -r;
    at<std::uintptr_t>(s, 0x1508) = reinterpret_cast<std::uintptr_t>(last);
    host.state_vcall_04_enter(last);
    return s;
}

namespace {
constexpr std::uint32_t kBrainVtable_00d21a40 = 0x00D21A40u;
constexpr std::uint32_t kBrainRegistryVtable_00d21a3c = 0x00D21A3Cu;
constexpr std::uint32_t kBrainCruiseVtable_00d21598 = 0x00D21598u;
constexpr std::uint32_t kBrainStopVtable_00d215c8 = 0x00D215C8u;
constexpr std::uint32_t kBrainFollowVtable_00d215f8 = 0x00D215F8u;
constexpr std::uint32_t kBrainLandVtable_00d21658 = 0x00D21658u;
constexpr std::uint32_t kBrainMoveToPosVtable_00d21628 = 0x00D21628u;
constexpr std::uint32_t kBrainMoveOnPathVtable_00d21688 = 0x00D21688u;
constexpr std::uint32_t kBrainKamikazeVtable_00d216b8 = 0x00D216B8u;
constexpr float kMinus99_00cf5bfc = -99.0f;
constexpr std::uint32_t kStrCruise_00cfb568 = 0x00CFB568u;
constexpr std::uint32_t kStrStop_00cfb5b8 = 0x00CFB5B8u;
constexpr std::uint32_t kStrFollow_00cfb52c = 0x00CFB52Cu;
constexpr std::uint32_t kStrLand_00cfb614 = 0x00CFB614u;
constexpr std::uint32_t kStrMoveToPos_00d21ab8 = 0x00D21AB8u;
constexpr std::uint32_t kStrMoveOnPath_00cfb654 = 0x00CFB654u;
constexpr std::uint32_t kStrAttackMove_00cecca8 = 0x00CECCA8u;
constexpr std::uint32_t kStrSubAttack_00d21aac = 0x00D21AACu;
constexpr std::uint32_t kStrKamikazeAttack_00d21a9c = 0x00D21A9Cu;
constexpr std::uint32_t kGunBotAiVtable_00d21808 = 0x00D21808u;
constexpr std::uint32_t kGunBotAiObserverVtable_00d217f0 = 0x00D217F0u;
constexpr std::uint32_t kShipAiVtable_00d21ae8 = 0x00D21AE8u;
constexpr std::uint32_t kShipAiObserverVtable_00d21acc = 0x00D21ACCu;
constexpr std::uint32_t kShipAiBrainVtable_00d21ac8 = 0x00D21AC8u;
constexpr std::uint32_t kShipAiBrainRegistryVtable_00d21ac4 = 0x00D21AC4u;
constexpr std::uint32_t kShipAiSize_2268 = 0x2268u;
constexpr std::uint32_t kBrainRecordVtable_00d21990 = 0x00D21990u;
constexpr std::uint32_t kBrainRecordAf8Vtable_00d21898 = 0x00D21898u;
}  // namespace

// 009F1160 (009F1160-009F1413). EH frame 00CB0B65. The brain record r:
// [r] = 00D21990; 009E4330(r+8h, ship) (009F118D); r+AA8h = ship, r+AACh = [ship+538h],
// r+AB0h = [ship+73Ch]; r+AB4h = ship->vtable[+5Ch](8) ? ship : 0 (009F11BF-009F11C9);
// r+AB8h = [ship+738h]; r+AC4h = r+AB4h ? new(4) holding r+AB4h (null if the
// allocation fails) : 0 (009F11DB-009F11F7). Defaults (009F11FD-009F1261): byte +AC8h
// = 0; +AD0h..+AD8h, +AE0h..+AE8h = 0; +AF0h = 1.0; +AF4h = 3; +AFCh..+B04h, +B0Ch = 0;
// byte +B08h = 1; +AF8h = 00D21898.
// 009F126B..009F1401 (the goal defaults, the seven stream-1 draws, the critical
// section) is the existing ship_ai_brain_produce_tail_009f126b
// (src/ship_ai_brain_producers.cpp), reached through the hook. RET 4, EAX = r.
// UNCERTAINTY: EH unwinding not modelled.
void* ship_ai_brain_record_construct_009f1160(Cc13Seg67bHost& host, void* r, void* ship) {
    at<std::uint32_t>(r, 0x000) = kBrainRecordVtable_00d21990;
    host.ship_ai_nav_block_construct_009e4330(ptr(r, 0x008), ship);
    at<std::uintptr_t>(r, 0xAA8) = reinterpret_cast<std::uintptr_t>(ship);
    at<std::uint32_t>(r, 0xAAC) = at<std::uint32_t>(ship, 0x538);
    at<std::uint32_t>(r, 0xAB0) = at<std::uint32_t>(ship, 0x73C);
    const bool submarine = host.object_vcall_5c(ship, 8);
    const std::uintptr_t sub = submarine ? reinterpret_cast<std::uintptr_t>(ship) : 0u;
    at<std::uintptr_t>(r, 0xAB4) = sub;
    at<std::uint32_t>(r, 0xAB8) = at<std::uint32_t>(ship, 0x738);
    std::uintptr_t holder = 0;
    if (sub != 0) {
        void* const cell = host.operator_new_00bf681b(4);
        if (cell != nullptr) {
            at<std::uintptr_t>(cell, 0) = sub;
            holder = reinterpret_cast<std::uintptr_t>(cell);
        }
    }
    at<std::uintptr_t>(r, 0xAC4) = holder;
    at<std::uint8_t>(r, 0xAC8) = 0;
    at<std::uint32_t>(r, 0xAD0) = 0;
    at<std::uint32_t>(r, 0xAD4) = 0;
    at<std::uint32_t>(r, 0xAD8) = 0;
    at<std::uint32_t>(r, 0xAE0) = 0;
    at<std::uint32_t>(r, 0xAE4) = 0;
    at<std::uint32_t>(r, 0xAE8) = 0;
    at<float>(r, 0xAF0) = kOne_00d7a24c;
    at<std::uint32_t>(r, 0xAF4) = 3;
    at<std::uint32_t>(r, 0xAFC) = 0;
    at<std::uint32_t>(r, 0xB00) = 0;
    at<std::uint32_t>(r, 0xB04) = 0;
    at<std::uint32_t>(r, 0xB0C) = 0;
    at<std::uint8_t>(r, 0xB08) = 1;
    at<std::uint32_t>(r, 0xAF8) = kBrainRecordAf8Vtable_00d21898;
    host.ship_ai_brain_produce_tail_009f126b(r);
    return r;
}

// 009F39C0 (009F39C0-009F3B97). EH frame 00CB0C64. The brain b (ai+58h):
// 009F1160(b, ship) (009F39E5); 00411E20(b+B5Ch) (009F39FA); [b] = 00D21A40,
// [b+B5Ch] = 00D21A3C. The state objects, each with owner b at +4h: cruise +B70h
// (00D21598, +B78h = +B7Ch = -99.0 [00CF5BFC]), stop +B80h (00D215C8), follow +B8Ch
// (00D215F8; byte +BB8h = 1, byte +B94h = 0), land +BE0h (00D21658; +BECh = 1.0
// [00D7A24C], bytes +BF0h, +BF1h = 0), movetopos +C04h (00D21628), moveonpath +C0Ch
// (00D21688; byte +C14h = 0) (009F3A07-009F3A9F); 009E8450(b+C18h, b) (009F3AB2);
// 009E4F90(b+2124h, b) (009F3AC3); kamikaze_attack +21FCh (00D216B8; byte +2204h = 0).
// 00411E70(b+B5Ch) registers, in order: "cruise", "stop", "follow", "land", "movetopos",
// "moveonpath", "attackmove", "sub_attack", "kamikaze_attack" (009F3AEC-009F3B7E).
// RET 4, EAX = b.
// RNG, stream 1, in order: 009F1160's seven, 009E5530's sixty U(0, 2.0) (009E55A1),
// then 009E8450's one, then 009E4F90's one: 69 for every ship (no kind gate).
// Whole body allowed: cc9 models the construction inline (game_hosts_ship_ai.cpp,
// seed_brain_draws_009f1160 with keyed per-name stand-ins, and the follow and land
// state defaults); it cites this constructor's stores.
// UNCERTAINTY: EH unwinding not modelled. The state objects' unwritten fields (cruise
// +B74h, land +BE8h, follow +B98h..+BB7h) are left as the allocation held them.
void* ship_ai_brain_construct_009f39c0(Cc13Seg67bHost& host, void* b, void* ship) {
    const std::uintptr_t self = reinterpret_cast<std::uintptr_t>(b);
    ship_ai_brain_record_construct_009f1160(host, b, ship);
    void* const registry = ptr(b, 0xB5C);
    host.bot_state_registry_construct_00411e20(registry);
    at<std::uint32_t>(b, 0x000) = kBrainVtable_00d21a40;
    at<std::uint32_t>(b, 0xB5C) = kBrainRegistryVtable_00d21a3c;
    void* const cruise = ptr(b, 0xB70);
    at<std::uintptr_t>(cruise, 0x04) = self;
    at<std::uint32_t>(cruise, 0x00) = kBrainCruiseVtable_00d21598;
    at<float>(cruise, 0x08) = kMinus99_00cf5bfc;
    at<float>(cruise, 0x0C) = kMinus99_00cf5bfc;
    at<std::uintptr_t>(b, 0xB84) = self;
    at<std::uint32_t>(b, 0xB80) = kBrainStopVtable_00d215c8;
    at<std::uintptr_t>(b, 0xB90) = self;
    at<std::uint32_t>(b, 0xB8C) = kBrainFollowVtable_00d215f8;
    at<std::uint8_t>(b, 0xBB8) = 1;
    at<std::uint8_t>(b, 0xB94) = 0;
    void* const land = ptr(b, 0xBE0);
    at<std::uintptr_t>(land, 0x04) = self;
    at<std::uint32_t>(land, 0x00) = kBrainLandVtable_00d21658;
    at<float>(land, 0x0C) = kOne_00d7a24c;
    at<std::uint8_t>(land, 0x10) = 0;
    at<std::uint8_t>(land, 0x11) = 0;
    at<std::uintptr_t>(b, 0xC08) = self;
    at<std::uint32_t>(b, 0xC04) = kBrainMoveToPosVtable_00d21628;
    at<std::uintptr_t>(b, 0xC10) = self;
    at<std::uint32_t>(b, 0xC0C) = kBrainMoveOnPathVtable_00d21688;
    at<std::uint8_t>(b, 0xC14) = 0;
    ship_ai_attackmove_construct_009e8450(host, ptr(b, 0xC18), b);
    ship_ai_sub_attack_construct_009e4f90(host, ptr(b, 0x2124), b);
    at<std::uintptr_t>(b, 0x2200) = self;
    at<std::uint32_t>(b, 0x21FC) = kBrainKamikazeVtable_00d216b8;
    at<std::uint8_t>(b, 0x2204) = 0;
    host.state_registry_add_00411e70(registry, kStrCruise_00cfb568, cruise);
    host.state_registry_add_00411e70(registry, kStrStop_00cfb5b8, ptr(b, 0xB80));
    host.state_registry_add_00411e70(registry, kStrFollow_00cfb52c, ptr(b, 0xB8C));
    host.state_registry_add_00411e70(registry, kStrLand_00cfb614, land);
    host.state_registry_add_00411e70(registry, kStrMoveToPos_00d21ab8, ptr(b, 0xC04));
    host.state_registry_add_00411e70(registry, kStrMoveOnPath_00cfb654, ptr(b, 0xC0C));
    host.state_registry_add_00411e70(registry, kStrAttackMove_00cecca8, ptr(b, 0xC18));
    host.state_registry_add_00411e70(registry, kStrSubAttack_00d21aac, ptr(b, 0x2124));
    host.state_registry_add_00411e70(registry, kStrKamikazeAttack_00d21a9c, ptr(b, 0x21FC));
    return b;
}

// 009F3BA0 (009F3BA0-009F3C6E). EH frame 00CB0C91. 0072BBD0(ai, 0) (009F3BC1);
// [ai] = 00D21808, [ai+1Ch] = 00D217F0; 009F39C0(ai+58h, ship) (009F3BE5); [ai] =
// 00D21AE8, [ai+1Ch] = 00D21ACC, [ai+58h] = 00D21AC8, [ai+BB4h] = 00D21AC4; ai+2260h =
// 00D05704, ai+2264h = &ai+BC8h (the brain's cruise state), then its vtable[4]
// (009F3C0D-009F3C27); byte ai+11h = 1; 008FBC80(ai, ship ? ship+310h : 0)
// (009F3C43); ai+B18h = ai+B14h = 0.0 (009F3C50, 009F3C58). RET 4, EAX = ai.
// Whole body allowed: cc9 models the initial cruise state inline
// (game_hosts_ship_ai.cpp, active_state_ai_offset under kShipAiInitialCruiseStateBound).
// UNCERTAINTY: EH unwinding not modelled.
void* ship_ai_gun_bot_construct_009f3ba0(Cc13Seg67bHost& host, void* ai, void* ship) {
    host.gun_bot_construct_base_0072bbd0(ai, 0);
    at<std::uint32_t>(ai, 0x00) = kGunBotAiVtable_00d21808;
    at<std::uint32_t>(ai, 0x1C) = kGunBotAiObserverVtable_00d217f0;
    ship_ai_brain_construct_009f39c0(host, ptr(ai, 0x58), ship);
    at<std::uint32_t>(ai, 0x00) = kShipAiVtable_00d21ae8;
    at<std::uint32_t>(ai, 0x1C) = kShipAiObserverVtable_00d21acc;
    at<std::uint32_t>(ai, 0x58) = kShipAiBrainVtable_00d21ac8;
    at<std::uint32_t>(ai, 0xBB4) = kShipAiBrainRegistryVtable_00d21ac4;
    void* const cruise = ptr(ai, 0xBC8);
    at<std::uint32_t>(ai, 0x2260) = kBotTaskSubVtable30C;
    at<std::uintptr_t>(ai, 0x2264) = reinterpret_cast<std::uintptr_t>(cruise);
    host.state_vcall_04_enter(cruise);
    at<std::uint8_t>(ai, 0x11) = 1;
    host.gun_bot_attach_008fbc80(ai, ship != nullptr ? ptr(ship, 0x310) : nullptr);
    at<float>(ai, 0xB18) = 0.0f;
    at<float>(ai, 0xB14) = 0.0f;
    return ai;
}

// 009F3F20 (009F3F20-009F3F7F). EH frame 00CB0CEB. operator new(2268h) (009F3F3E);
// null -> EAX = 0; else 009F3BA0(t, ship) (009F3F59) and EAX = t.
// cc9 reaches it only inline: register_generated_units (game_hosts_ship_ai.cpp ~12258,
// the 00810DD0 -> 009F3F20 chain) and the unit+740h note at ~365.
// UNCERTAINTY: EH unwinding (the delete on a throwing constructor) not modelled.
void* ship_ai_allocate_brain_009f3f20(Cc13Seg67bHost& host, void* ship) {
    void* const t = host.operator_new_00bf681b(kShipAiSize_2268);
    if (t == nullptr) {
        return nullptr;
    }
    return ship_ai_gun_bot_construct_009f3ba0(host, t, ship);
}

namespace {
constexpr std::uint32_t kStrPlaneId_00cf8770 = 0x00CF8770u;  // "planeID"
constexpr std::uint32_t kObserverNodeVtable_00ce75cc = 0x00CE75CCu;
constexpr float kObserverNodeDefault_00cf8b3c = 256.0f;
void* pilot_task_at(void* bot, std::uint32_t i) {
    return reinterpret_cast<void*>(
        at<std::uintptr_t>(reinterpret_cast<void*>(at<std::uintptr_t>(bot, 0x58)), i * 4u));
}
}  // namespace

// 00999A10 (00999A10-00999A5A). The pilot bot's task array is [+58h] with the count
// at +5Ch. False with no task or a null head; false unless head->vtable[+38h]();
// false when head->vtable[+34h](); true with more than one task; else true unless
// head->vtable[+40h]() == 1 (00999A3D-00999A4F). No RNG.
bool pilot_bot_head_task_blocks_switch_00999a10(Cc13Seg67bHost& host, void* bot) {
    if (at<std::uint32_t>(bot, 0x5C) == 0) return false;
    void* const head = pilot_task_at(bot, 0);
    if (head == nullptr) return false;
    if (!host.object_vcall_38(head)) return false;
    if (host.object_vcall_34(head)) return false;
    if (at<std::uint32_t>(bot, 0x5C) > 1) return true;
    return host.task_vcall_40(head) != 1;
}

// 00999A60 (00999A60-00999A70). AL = byte [[p+4h]+184h] == 0.
bool pilot_park_predicate_plane_byte184_zero_00999a60(const void* p) {
    const void* const plane = reinterpret_cast<const void*>(
        *reinterpret_cast<const std::uintptr_t*>(static_cast<const char*>(p) + 4));
    return *(static_cast<const std::uint8_t*>(plane) + 0x184) == 0;
}

// 00999A80 (00999A80-00999A9C). [p] = 00D05840 (the pure-call base); flags bit 0 ->
// _free(p) (00999A91). RET 4, EAX = p.
void* pilot_park_predicate_scalar_delete_00999a80(Cc13Seg67bHost& host, void* p,
                                                  std::uint32_t flags) {
    at<std::uint32_t>(p, 0) = kPureCallVtable;
    if ((flags & 1u) != 0) host.crt_free_00bf65ac(p);
    return p;
}

// 00999B30 (00999B30-00999B66). Walks the task array; a non-null task whose
// vtable[+28h]() returns AL != 0 ends the walk with that AL. With no match EAX holds
// the count [bot+5Ch] (reloaded at 00999B54, or loaded at 00999B37 when the array is
// empty), so AL = the count's low byte: the image has no explicit false.
// UNCERTAINTY: the no-match value is the register left over, modelled as such.
std::uint8_t pilot_bot_any_task_slot28_00999b30(Cc13Seg67bHost& host, void* bot) {
    for (std::uint32_t i = 0; i < at<std::uint32_t>(bot, 0x5C); ++i) {
        void* const task = pilot_task_at(bot, i);
        if (task == nullptr) continue;
        const std::uint8_t al = host.task_vcall_28_flag(task);
        if (al != 0) return al;
    }
    return static_cast<std::uint8_t>(at<std::uint32_t>(bot, 0x5C) & 0xFFu);
}

// 00999C70 (00999C70-00999CB2). The first non-null task whose vtable[+3Ch]() is
// non-zero, else 0.
void* pilot_bot_find_task_by_slot3c_00999c70(Cc13Seg67bHost& host, void* bot) {
    for (std::uint32_t i = 0; i < at<std::uint32_t>(bot, 0x5C); ++i) {
        void* const task = pilot_task_at(bot, i);
        if (task != nullptr && host.task_vcall_3c(task) != 0) return task;
    }
    return nullptr;
}

// 0099A630 (0099A630-0099A657). +4h, +8h, +0Ch, +14h = 0; byte +10h = 1; [n] = 00CE75CC;
// +18h = 256.0 [00CF8B3C]. EAX = n.
void* pilot_bot_init_observer_node_0099a630(void* n) {
    at<std::uint32_t>(n, 0x04) = 0;
    at<std::uint32_t>(n, 0x08) = 0;
    at<std::uint32_t>(n, 0x0C) = 0;
    at<std::uint32_t>(n, 0x14) = 0;
    at<std::uint8_t>(n, 0x10) = 1;
    at<std::uint32_t>(n, 0x00) = kObserverNodeVtable_00ce75cc;
    at<float>(n, 0x18) = kObserverNodeDefault_00cf8b3c;
    return n;
}

// 0099A940 (0099A940-0099A979). archive->vtable[+10h]("planeID", {4, &slot}), then
// p+4h = slot. The slot is the stack word the record points at (the PUSH ECX at
// 0099A940); before the read 0099A94A stores the record's own stack address into it.
// UNCERTAINTY: an archive that does not write the slot leaves p+4h = that stack
// address in the image; seeded to 0 here (labelled).
void pilot_park_predicate_read_plane_id_0099a940(Cc13Seg67bHost& host, void* p,
                                                 void* archive) {
    std::uint32_t slot = 0;
    host.archive_vcall_10_read(archive, name_record(kStrPlaneId_00cf8770),
                               record(4, reinterpret_cast<std::uintptr_t>(&slot)));
    at<std::uint32_t>(p, 0x04) = slot;
}

// 0099A980 (0099A980-0099A9AD). archive->vtable[+0Ch]("planeID", {4, [p+4h]}).
void pilot_park_predicate_write_plane_id_0099a980(Cc13Seg67bHost& host, void* p,
                                                  void* archive) {
    host.archive_vcall_0c_write(archive, name_record(kStrPlaneId_00cf8770),
                                record(4, at<std::uint32_t>(p, 0x04)));
}

// 0099B3D0 (0099B3D0-0099B3DA): byte plan+2DCh = value.
void pilot_plan_set_byte_2dc_0099b3d0(void* plan, std::uint8_t value) {
    at<std::uint8_t>(plan, 0x2DC) = value;
}

// 0099B3E0 (0099B3E0-0099B3EA): byte plan+2E5h = value (the turbo request).
void pilot_plan_set_turbo_request_0099b3e0(void* plan, std::uint8_t value) {
    at<std::uint8_t>(plan, 0x2E5) = value;
}

// 00997340 (00997340-00997379). With byte [[task+2F4h]+C24h] set, 009FC7C0(task+314h,
// dt) (00997360); then 009FD0E0(task+38Ch, dt) (00997373). Unreferenced (no caller or
// table entry found); BSP_PilotBot_Update 00999979/0099998C makes the same two calls.
void bot_task_tick_gun_and_rocket_00997340(Cc13Seg67bHost& host, void* task, float dt) {
    void* const unit = reinterpret_cast<void*>(at<std::uintptr_t>(task, 0x2F4));
    if (at<std::uint8_t>(unit, 0xC24) != 0) {
        host.bot_task_gun_tick_009fc7c0(ptr(task, 0x314), dt);
    }
    host.opportunistic_rocket_tick_009fd0e0(ptr(task, 0x38C), dt);
}

// 009995A0 (009995A0-009995C6), PilotBot vtable 00D1F348 slot +4h. 008FBC80(bot, node)
// (009995A8); bot+9Ch = bot+B8h = 256.0 [00CF8B3C].
void pilot_bot_attach_009995a0(Cc13Seg67bHost& host, void* bot, void* node) {
    host.gun_bot_attach_008fbc80(bot, node);
    at<float>(bot, 0x9C) = kObserverNodeDefault_00cf8b3c;
    at<float>(bot, 0xB8) = kObserverNodeDefault_00cf8b3c;
}

namespace {
std::uint32_t* array_data(void* a) {
    return reinterpret_cast<std::uint32_t*>(at<std::uintptr_t>(a, 0x0));
}
// The reallocation shared by the four bodies: capacity = 2 * capacity + extra, then
// operator new[](capacity * 4).
std::uint32_t* array_reserve(Cc13Seg67bHost& host, void* a, std::uint32_t extra) {
    const std::uint32_t cap = at<std::uint32_t>(a, 0x8) * 2u + extra;
    at<std::uint32_t>(a, 0x8) = cap;
    return static_cast<std::uint32_t*>(host.operator_new_array_00bf55be(cap * 4u));
}
void array_release(Cc13Seg67bHost& host, void* a, std::uint32_t* fresh) {
    if (array_data(a) != nullptr) host.operator_delete_array_00bf6989(array_data(a));
    at<std::uintptr_t>(a, 0x0) = reinterpret_cast<std::uintptr_t>(fresh);
}
}  // namespace

// 00999700 (00999700-00999801). idx = (where - data) / 4. Full (count == capacity):
// capacity = 2 * capacity + 1, new[] (0099972B), copy [0, idx), the value at idx, then
// [idx, count) one up, delete[] the old data (009997A4), count += 1. Otherwise
// data[count] = data[count - 1], shift (idx, count) up one, data[idx] = value, count
// += 1. Each store into new storage is behind the placement-new null test. RET 8, EAX =
// data + idx.
// UNCERTAINTY: with count == 0 and spare capacity the image reads data[-1] and its
// shift counter wraps (unbounded loop); unreachable for these arrays, and the body
// stops after the one store (labelled).
std::uint32_t* ptr_array_insert_at_00999700(Cc13Seg67bHost& host, void* a,
                                            std::uint32_t* where, const std::uint32_t* value) {
    const std::uint32_t idx = static_cast<std::uint32_t>(where - array_data(a));
    const std::uint32_t count = at<std::uint32_t>(a, 0x4);
    if (count == at<std::uint32_t>(a, 0x8)) {
        std::uint32_t* const fresh = array_reserve(host, a, 1);
        std::uint32_t* const old = array_data(a);
        if (fresh != nullptr) {
            for (std::uint32_t i = 0; i < idx; ++i) fresh[i] = old[i];
            fresh[idx] = *value;
            for (std::uint32_t i = idx; i < count; ++i) fresh[i + 1] = old[i];
        }
        array_release(host, a, fresh);
        at<std::uint32_t>(a, 0x4) = count + 1;
        return array_data(a) + idx;
    }
    std::uint32_t* const data = array_data(a);
    if (count != 0) {
        data[count] = data[count - 1];
        for (std::uint32_t i = count - 1; i > idx; --i) data[i] = data[i - 1];
    }
    data[idx] = *value;
    at<std::uint32_t>(a, 0x4) = count + 1;
    return data + idx;
}

// 00999810 (00999810-0099985F). capacity = 2 * capacity + 2, new[] (00999823), copy
// [0, count), delete[] the old data (00999853), data = new. RET.
void ptr_array_grow_00999810(Cc13Seg67bHost& host, void* a) {
    std::uint32_t* const fresh = array_reserve(host, a, 2);
    const std::uint32_t count = at<std::uint32_t>(a, 0x4);
    if (fresh != nullptr) {
        for (std::uint32_t i = 0; i < count; ++i) fresh[i] = array_data(a)[i];
    }
    array_release(host, a, fresh);
}

// 00999CC0 (00999CC0-00999D31). Full: the 00999810 growth inline (2 * capacity + 2,
// 00999CD8, 00999D0D); then data[count] = value (placement null test), count += 1.
void ptr_array_push_back_00999cc0(Cc13Seg67bHost& host, void* a, const std::uint32_t* value) {
    if (at<std::uint32_t>(a, 0x4) == at<std::uint32_t>(a, 0x8)) {
        ptr_array_grow_00999810(host, a);
    }
    std::uint32_t* const data = array_data(a);
    if (data != nullptr || at<std::uint32_t>(a, 0x4) != 0) {
        data[at<std::uint32_t>(a, 0x4)] = *value;
    }
    at<std::uint32_t>(a, 0x4) += 1;
}

namespace {
constexpr std::uint32_t kStrBot_00cf904c = 0x00CF904Cu;            // "_bot"
constexpr std::uint32_t kStrCommandName_00d1f390 = 0x00D1F390u;    // "commandName"
constexpr std::uint32_t kPilotPlanTableVtable_00d056e8 = 0x00D056E8u;
}  // namespace

// 0099AA20 (0099AA20-0099ABC3). EH frame 00CADB68. archive->vtable[+4h]("_bot");
// 008FE660(bot, archive) (0099AA62); archive->vtable[+8h](). Then for i = 1, 2, ...
// while archive->vtable[+14h]({1, i}) (0099AA8C): begin {1, i}; a counted string s =
// {0, 0} read under "commandName" ({0, &s}, 0099AAE8); factory = 0099C690(&s)
// (0099AAEE); task = factory->vtable[+4h](bot, archive) (0099AAFC); push the task onto
// the bot's task array {+58h, +5Ch, +60h} (growth 2 * capacity + 2 inline, the
// 00999CC0 pattern, 0099AB0A-0099AB71); end; when s's buffer is set, 00BD1510(00419CC0(),
// buffer, length + 1, 1) (0099ABA0-0099ABA7). RET 4.
// UNCERTAINTY: EH unwinding not modelled; a factory miss (0099C690 = 0) dereferences
// null in the image as here.
void pilot_bot_load_tasks_0099aa20(Cc13Seg67bHost& host, void* bot, void* archive) {
    host.archive_vcall_04_begin(archive, name_record(kStrBot_00cf904c));
    host.gun_bot_read_base_properties_008fe660(bot, archive);
    host.archive_vcall_08_end(archive);
    for (std::uint32_t i = 1;; ++i) {
        if (!host.archive_vcall_14_has(archive, record(1, i))) return;
        host.archive_vcall_04_begin(archive, record(1, i));
        struct CountedString {
            std::uint32_t length;
            std::uintptr_t buffer;
        } s{0, 0};
        host.archive_vcall_10_read(archive, name_record(kStrCommandName_00d1f390),
                                   record(0, reinterpret_cast<std::uintptr_t>(&s)));
        void* const factory = host.bot_task_factory_find_0099c690(&s);
        void* const task = host.factory_vcall_04_load(factory, bot, archive);
        const std::uint32_t value =
            static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(task));
        ptr_array_push_back_00999cc0(host, ptr(bot, 0x58), &value);
        host.archive_vcall_08_end(archive);
        if (s.buffer != 0) {
            host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_00419cc0(),
                                                          reinterpret_cast<void*>(s.buffer),
                                                          s.length + 1, 1);
        }
    }
}

namespace {
// 0099C459-0099C49F and 0099C5B3-0099C5FB: s = (dx*dx + dy*dy) + dz*dz spilled to a
// float; float(sqrt(s)) (00BF7030) when s > 1e-10 [00CE3820, double], else 0 (FCOMI
// JBE, so NaN -> 0).
float lead_distance(const float* a, const float* b) {
    const float dx = a[0] - b[0];
    const float dy = a[1] - b[1];
    const float dz = a[2] - b[2];
    const float s = (dx * dx + dy * dy) + dz * dz;
    return static_cast<double>(s) > kTiny_00ce3820
               ? static_cast<float>(std::sqrt(static_cast<double>(s)))
               : 0.0f;
}
}  // namespace

// 0099C3C0 (0099C3C0-0099C683). l = {+0h owner entity, +4h float speed, byte +8h, +0Ch,
// +10h..+18h}. Owner and target each refreshed through 00414DB0 when byte +C8h is clear;
// P = owner+FCh..+104h, T = target+FCh..+104h. Pass 1: t = |T - P| / speed
// (0099C4AF). With byte l+8h set, v = target->vtable[+34h](&scratch) (0099C4C4);
// otherwise target->vtable[+38h]() first, its ST0 dropped (0099C525/0099C527), then the
// same +34h call; Q = v * t + T. Pass 2: s = |Q - P| (0099C5CF); l+0Ch = s, the
// distance before the division (XMM0 loaded at 0099C605, stored at 0099C60E); t =
// s / speed; v = target->vtable[+34h](&scratch) again (0099C623); l+10h..+18h = v * t +
// T. RET 4.
// UNCERTAINTY: 24-bit x87 modelled (every intermediate is a float spill); the two
// branches differ only by the discarded +38h call.
void bot_lead_point_on_target_0099c3c0(Cc13Seg67bHost& host, void* l, void* target) {
    void* const owner = reinterpret_cast<void*>(at<std::uintptr_t>(l, 0x0));
    if (at<std::uint8_t>(owner, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(owner);
    const float p[3] = {at<float>(owner, 0xFC), at<float>(owner, 0x100),
                        at<float>(owner, 0x104)};
    if (at<std::uint8_t>(target, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(target);
    const float t_pos[3] = {at<float>(target, 0xFC), at<float>(target, 0x100),
                            at<float>(target, 0x104)};
    const float speed = at<float>(l, 0x4);
    const float t1 = lead_distance(t_pos, p) / speed;
    if (at<std::uint8_t>(l, 0x8) == 0) {
        static_cast<void>(host.entity_vcall_38_float(target));
    }
    float scratch[3] = {};
    const float* v = host.entity_vcall_34_velocity(target, scratch);
    const float q[3] = {v[0] * t1 + t_pos[0], v[1] * t1 + t_pos[1], v[2] * t1 + t_pos[2]};
    const float s2 = lead_distance(q, p);
    at<float>(l, 0x0C) = s2;
    const float t2 = s2 / speed;
    float scratch2[3] = {};
    v = host.entity_vcall_34_velocity(target, scratch2);
    at<float>(l, 0x10) = v[0] * t2 + t_pos[0];
    at<float>(l, 0x14) = v[1] * t2 + t_pos[1];
    at<float>(l, 0x18) = v[2] * t2 + t_pos[2];
}

// 0099BDB0 (0099BDB0-0099BDE0), the element constructor 0099BE30 passes to the array
// constructor 00BF7CD1 (three 0C8h elements). [t] = 00D056E8; 24 float pairs at +4h..+C3h
// zeroed (ESI = 17h down to 0, JNS); the count +C4h = 0. EAX = t.
void* pilot_plan_table_construct_0099bdb0(void* t) {
    at<std::uint32_t>(t, 0x00) = kPilotPlanTableVtable_00d056e8;
    for (std::uint32_t i = 0; i < 24; ++i) {
        at<float>(t, 0x08 + i * 8) = 0.0f;
        at<float>(t, 0x04 + i * 8) = 0.0f;
    }
    at<std::uint32_t>(t, 0xC4) = 0;
    return t;
}

// 0099B1B0 (0099B1B0-0099B1D5). (plane+72Ch)->vtable[+38h]() true -> tail JMP 007D7A40
// (ECX = plane+AB0h) (0099B1CF). No caller or table entry found (game_hosts_units.cpp
// notes the same).
void plane_command_band_repair_if_controlled_0099b1b0(Cc13Seg67bHost& host, void* plane) {
    if (host.object_vcall_38(ptr(plane, 0x72C))) {
        host.pilot_plan_reset_command_timer_007d7a40(ptr(plane, 0xAB0));
    }
}

// 0099B2A0 (0099B2A0-0099B30E), the bare-array twin of 0099B450 (docs/PILOT_CONTROLS.md).
// Five slots {value +0h, desired +4h, byte +8h}: both floats from unit+9F0h, +9E4h,
// +9ECh, +9E8h, +9F4h in that slot order, the byte cleared. RET 4.
void pilot_bot_seed_slot_array_0099b2a0(void* slots, const void* unit) {
    static constexpr std::uint32_t kSource[5] = {0x9F0, 0x9E4, 0x9EC, 0x9E8, 0x9F4};
    const char* const u = static_cast<const char*>(unit);
    for (std::uint32_t i = 0; i < 5; ++i) {
        float v;
        std::memcpy(&v, u + kSource[i], sizeof v);
        void* const slot = ptr(slots, i * 0x0C);
        at<float>(slot, 0x4) = v;
        at<float>(slot, 0x0) = v;
        at<std::uint8_t>(slot, 0x8) = 0;
    }
}

// 0099C2C0 (0099C2C0-0099C3B3), the bot-task base of the command-still-valid slot.
// 2 when task+2FCh (the unit) is null or task->vtable[+3Ch]() (the task's command) is
// null. director = unit->vtable[+114h](); match = 0071BE40(director) == command
// (0099C2FD). When 0071BE40(director) (called again, 0099C30A) is 00E08F78 and
// command->vtable[+0Ch]() is 1 or 2, or else when match holds, the aim test runs;
// otherwise 0. The aim test: task->vtable[+0Ch](&aim) (0099C339); aim byte +0Ch clear ->
// 1; else p = 006F7DD0(0071EB60(director), &scratch) (0099C351, 0099C358), d = p - aim
// spilled to floats, 1 when 100.0 [00D7A220, double] > 0042B2F0(&d) (FCOMPI, JBE), else
// 0 (an unordered length gives 0).
// The scratch push at 0099C34E precedes the 0071EB60 call but is 006F7DD0's argument
// (0071EB60 is a plain RET).
// UNCERTAINTY: x87 at 24-bit; the double constant makes the compare exact either way.
std::uint32_t bot_task_is_command_still_valid_base_0099c2c0(Cc13Seg67bHost& host, void* task) {
    void* const unit = reinterpret_cast<void*>(at<std::uintptr_t>(task, 0x2FC));
    if (unit == nullptr) return 2;
    const std::uint32_t command = host.task_vcall_3c(task);
    if (command == 0) return 2;
    void* const director = host.object_vcall_114(unit);
    const bool match = host.weapon_director_current_command_0071be40(director) == command;
    bool check = false;
    if (host.weapon_director_current_command_0071be40(director) == 0x00E08F78u) {
        const std::uint32_t kind =
            host.command_vcall_0c(reinterpret_cast<void*>(static_cast<std::uintptr_t>(command)));
        check = kind == 1 || kind == 2;
    }
    if (!check && !match) return 0;
    alignas(4) std::uint8_t aim[0x10] = {};
    host.task_vcall_0c_aim(task, aim);
    if (aim[0x0C] == 0) return 1;
    float aim_xyz[3];
    std::memcpy(aim_xyz, aim, sizeof aim_xyz);
    float scratch[3] = {};
    const float* const p = host.target_descriptor_resolve_position_006f7dd0(
        host.entity_command_active_target_descriptor_0071eb60(director), scratch);
    const float d[3] = {p[0] - aim_xyz[0], p[1] - aim_xyz[1], p[2] - aim_xyz[2]};
    const float len = host.vector_length_0042b2f0(d);
    return kHundred_00d7a220 > static_cast<double>(len) ? 1u : 0u;
}

// ---- section 3.45: one-line table slots (disk bytes; each body is its whole listing) ----
// 009A54E0: MOV EAX,00F8A370 (the static depth-charge bot-task factory record);
// 009A9950: MOV EAX,00F8A3E8 (the static "Dogfight" factory record); 009A5500: "DepthCharge"
// 00CFA700; 009ADDE0: "DropKamikaze" 00D1FADC; 009AF080: "Kamikaze"
// 00CFA714; 009B33A0: 00F8A438 (land factory record); 009B7BD0: "LevelBomb" 00D06EB0; 009B9140: FLD [00CE38B8]
// 10.0; 009BA920: "immelmann" 00D20468; 009BCFB0: 00F8A534 (move-on-path factory record); 009C79C0:
// "DiveBomb" 00D06E1C; 009C9E60: 3. Byte twins of existing bodies are ledger-only:
// 009BAC60 / 009C3170 / 009C9E70 (entity_orders.cpp 006F8C00 /
// 006F8920 / 006F8C90) and 009C3150 (kMoveToCommandClass, 009DAE20).
std::uint32_t bot_task_depth_charge_factory_record_009a54e0() noexcept { return 0x00F8A370u; }
std::uint32_t bot_task_dogfight_factory_record_009a9950() noexcept { return 0x00F8A3E8u; }
std::uint32_t bot_task_depth_charge_name_009a5500() noexcept { return 0x00CFA700u; }
std::uint32_t bot_task_drop_kamikaze_name_009adde0() noexcept { return 0x00D1FADCu; }
std::uint32_t bot_task_kamikaze_name_009af080() noexcept { return 0x00CFA714u; }
std::uint32_t bot_task_land_factory_record_009b33a0() noexcept { return 0x00F8A438u; }
std::uint32_t bot_task_level_bomb_name_009b7bd0() noexcept { return 0x00D06EB0u; }
float bot_task_pull_up_slot68_009b9140() noexcept { return kTen_00ce38b8; }
std::uint32_t bot_task_immelmann_name_009ba920() noexcept { return 0x00D20468u; }
std::uint32_t bot_task_moveonpath_factory_record_009bcfb0() noexcept { return 0x00F8A534u; }
std::uint32_t bot_task_dive_bomb_name_009c79c0() noexcept { return 0x00D06E1Cu; }
std::uint32_t bot_task_retreat_kind_009c9e60() noexcept { return 3u; }

// 009CE290 and 009D0C00: a bare RET.
void bot_state_takeoff_exit_009ce290(void*) noexcept {}
void bot_state_torpedo_go_away_enter_009d0c00(void*) noexcept {}

// 009A9C80: EAX = [task+4ACh].
std::uint32_t bot_task_dogfight_slot48_value_009a9c80(const void* task) {
    return *reinterpret_cast<const std::uint32_t*>(static_cast<const char*>(task) + 0x4AC);
}

// 009A9D30 (009A9D30-009A9D43): AL = [task+310h] == task+6A4h (the go-away state current).
bool bot_task_dogfight_in_go_away_009a9d30(const void* task) {
    const char* const t = static_cast<const char*>(task);
    return *reinterpret_cast<const std::uintptr_t*>(t + 0x310) ==
           reinterpret_cast<std::uintptr_t>(t + 0x6A4);
}

// 009B9480 (009B9480-009B9498): word task+414h = 0; unit = [task+2FCh]; non-null -> tail
// JMP 007EFB30(unit).
void bot_task_leave_alone_flush_009b9480(Cc13Seg67bHost& host, void* task) {
    at<std::uint16_t>(task, 0x414) = 0;
    void* const unit = reinterpret_cast<void*>(at<std::uintptr_t>(task, 0x2FC));
    if (unit != nullptr) host.pilot_control_raise_force_release_flag_007efb30(unit);
}

// 009BCD40: ECX -= 40h, JMP 009BCE30. 009BD090: ECX -= 3F8h, JMP 009BDB90. 009BD0A0:
// ECX -= 438h, JMP 009BDB90. The flags argument passes through.
void* bot_approach_moveonpath_registry_dtor_thunk_009bcd40(Cc13Seg67bHost& host, void* self,
                                                           std::uint32_t flags) {
    return host.scalar_deleting_dtor_009bce30(static_cast<char*>(self) - 0x40, flags);
}
void* bot_task_moveonpath_approach_view_dtor_thunk_009bd090(Cc13Seg67bHost& host, void* self,
                                                            std::uint32_t flags) {
    return host.scalar_deleting_dtor_009bdb90(static_cast<char*>(self) - 0x3F8, flags);
}
void* bot_task_moveonpath_registry_view_dtor_thunk_009bd0a0(Cc13Seg67bHost& host, void* self,
                                                            std::uint32_t flags) {
    return host.scalar_deleting_dtor_009bdb90(static_cast<char*>(self) - 0x438, flags);
}

namespace {
float follow_wait_value_or_base(Cc13Seg67bHost& host, void* task, std::uint32_t follow) {
    if (at<std::uintptr_t>(task, 0x310) == reinterpret_cast<std::uintptr_t>(ptr(task, follow))) {
        return host.bot_state_follow_wait_value_009be3e0(ptr(task, follow));
    }
    return host.bot_task_base_wait_value_0099b720(task);
}
}  // namespace

// 009A5D50 (009A5D50-009A5D78): the current state [task+310h] is task+544h -> JMP
// 009BE3E0(task+544h); task+6E4h -> 009BE3E0(task+6E4h); else JMP 0099B720(task).
float bot_task_depth_charge_wait_value_009a5d50(Cc13Seg67bHost& host, void* task) {
    if (at<std::uintptr_t>(task, 0x310) == reinterpret_cast<std::uintptr_t>(ptr(task, 0x544))) {
        return host.bot_state_follow_wait_value_009be3e0(ptr(task, 0x544));
    }
    return follow_wait_value_or_base(host, task, 0x6E4);
}
// 009BD2E0: the same test on task+464h; 009CA0A0: on task+4C4h.
float bot_task_moveonpath_follow_wait_value_009bd2e0(Cc13Seg67bHost& host, void* task) {
    return follow_wait_value_or_base(host, task, 0x464);
}
float bot_task_retreat_wait_value_009ca0a0(Cc13Seg67bHost& host, void* task) {
    return follow_wait_value_or_base(host, task, 0x4C4);
}

namespace {
void* state_plane(void* s) {
    void* const task = reinterpret_cast<void*>(at<std::uintptr_t>(s, 0x4));
    return reinterpret_cast<void*>(at<std::uintptr_t>(task, 0x4));
}
}  // namespace

// 009C44C0 (009C44C0-009C44D1): byte plane+844h = 0, byte state+1Ch = 0.
void bot_state_dive_bomb_turn_down_enter_009c44c0(void* s) {
    at<std::uint8_t>(state_plane(s), 0x844) = 0;
    at<std::uint8_t>(s, 0x1C) = 0;
}
// 009B1E90: byte plane+844h = 1. Its byte-identical twins 009C44E0, 009C58A0 and 009C6210
// (the dive-bomb exits) are ledger-only rows citing this body.
void bot_state_land_final_exit_009b1e90(void* s) { at<std::uint8_t>(state_plane(s), 0x844) = 1; }
// 009D1350: byte [[state+4h]+A8h] = 0.
void bot_state_torpedo_aim_exit_009d1350(void* s) {
    at<std::uint8_t>(reinterpret_cast<void*>(at<std::uintptr_t>(s, 0x4)), 0xA8) = 0;
}

// 009D4EA0 / 009D4EC0: +40h / +48h = value, byte +4Ch = 1, RET 4. 009D4F00: FLD +48h.
void command_record_set_float40_009d4ea0(void* r, float value) {
    at<float>(r, 0x40) = value;
    at<std::uint8_t>(r, 0x4C) = 1;
}
void command_record_set_float48_009d4ec0(void* r, float value) {
    at<float>(r, 0x48) = value;
    at<std::uint8_t>(r, 0x4C) = 1;
}
float command_record_get_float48_009d4f00(const void* r) {
    return *reinterpret_cast<const float*>(static_cast<const char*>(r) + 0x48);
}

// 009D4F70: EAX = ai + A98h + [ai+B40h] * 54h (IMUL, so the index is signed).
void* ship_ai_current_order_slot_009d4f70(void* ai) {
    const std::int32_t index = at<std::int32_t>(ai, 0xB40);
    return static_cast<char*>(ai) + 0xA98 + index * 0x54;
}

// 009DABB0..009DABF0, unreferenced: byte +3F4h, dword +3F8h, byte +3FCh, byte +3FDh
// from the argument (RET 4); byte +B38h = 1 (RET).
void ship_ai_set_torpedo_avoidance_request_009dabb0(void* b, std::uint8_t v) {
    at<std::uint8_t>(b, 0x3F4) = v;
}
void ship_ai_set_ship_avoidance_party_filter_009dabc0(void* b, std::uint32_t v) {
    at<std::uint32_t>(b, 0x3F8) = v;
}
void ship_ai_set_land_avoidance_request_009dabd0(void* b, std::uint8_t v) {
    at<std::uint8_t>(b, 0x3FC) = v;
}
void ship_ai_set_drive_bypass_009dabe0(void* b, std::uint8_t v) { at<std::uint8_t>(b, 0x3FD) = v; }
void ship_ai_set_speed_commanded_flag_009dabf0(void* b) { at<std::uint8_t>(b, 0xB38) = 1; }

// 009F5260: [o] = 00D21B40, [o+4h] = arg, RET 4, EAX = o.
void* auto_target_base_construct_009f5260(void* o, std::uint32_t arg) {
    at<std::uint32_t>(o, 0x0) = 0x00D21B40u;
    at<std::uint32_t>(o, 0x4) = arg;
    return o;
}

// 00999D40 (00999D40-00999DEE), ledger name BSP_PtrArray_PushBackOdd: an insert at the
// front. Full: capacity = 2 * capacity + 1, new[] (00999D5A), new[0] = value, [0, count)
// one up, delete[] (00999DA5), count += 1. Otherwise data[count] = data[count - 1],
// shift [1, count) up, data[0] = value, count += 1. RET 4.
// UNCERTAINTY: as 00999700 for count == 0 with spare capacity (the image's counter
// wraps at 00999DC9); the body only stores data[0] (labelled).
void ptr_array_push_front_00999d40(Cc13Seg67bHost& host, void* a, const std::uint32_t* value) {
    const std::uint32_t count = at<std::uint32_t>(a, 0x4);
    if (count == at<std::uint32_t>(a, 0x8)) {
        std::uint32_t* const fresh = array_reserve(host, a, 1);
        if (fresh != nullptr) {
            fresh[0] = *value;
            for (std::uint32_t i = 0; i < count; ++i) fresh[i + 1] = array_data(a)[i];
        }
        array_release(host, a, fresh);
        at<std::uint32_t>(a, 0x4) = count + 1;
        return;
    }
    std::uint32_t* const data = array_data(a);
    if (count != 0) {
        data[count] = data[count - 1];
        for (std::uint32_t i = count - 1; i != 0; --i) data[i] = data[i - 1];
    }
    data[0] = *value;
    at<std::uint32_t>(a, 0x4) = count + 1;
}

// ============================================================================
// Medium rows, batch 1 (docs/CC13_SEG67B.md 3.46)
// ============================================================================

namespace {

// The shared shape of the nine Describe slots (each 38 instructions, the same bytes
// apart from the SEH handler and the registry LEA): 009A1A90(task, &temp) for the base
// name, 00411430(task + registry, [task+310h]) for the state name, then
// 007B7E70(out, base, name). The temporary is released inline: capacity (temp+18h)
// >= 10h frees the heap buffer at temp+4h through 00BF65AC.
void* describe_with_registry(Cc13Seg67bHost& host, void* task, void* out,
                             std::size_t registry) {
    alignas(4) std::uint8_t temp[0x1C] = {};
    const void* const base = host.bot_task_debug_name_009a1a90(task, temp);
    const char* const name =
        host.state_registry_name_or_undefined_00411430(ptr(task, registry), ptr_at(task, 0x310));
    host.std_string_plus_cstr_007b7e70(out, base, name);
    if (at<std::uint32_t>(temp, 0x18) >= 0x10) host.crt_free_00bf65ac(ptr_at(temp, 0x4));
    return out;
}

}  // namespace

// 009A68C0 (009A68C0-009A693F): registry task+4F4h (009A68F7).
void* bot_task_depth_charge_describe_009a68c0(Cc13Seg67bHost& host, void* task, void* out) {
    return describe_with_registry(host, task, out, 0x4F4);
}

// 009AF670 (009AF670-009AF6EF): registry task+4D4h (009AF6A7).
void* bot_task_kamikaze_describe_009af670(Cc13Seg67bHost& host, void* task, void* out) {
    return describe_with_registry(host, task, out, 0x4D4);
}

// 009AF890 (009AF890-009AF912): registry task+4BCh (009AF8C7).
void* bot_task_drop_kamikaze_describe_009af890(Cc13Seg67bHost& host, void* task, void* out) {
    return describe_with_registry(host, task, out, 0x4BC);
}

// 009B8F80 (009B8F80-009B8FFF): registry task+4D4h (009B8FB7).
void* bot_task_level_bomb_describe_009b8f80(Cc13Seg67bHost& host, void* task, void* out) {
    return describe_with_registry(host, task, out, 0x4D4);
}

// 009BDB00 (009BDB00-009BDB7F): registry task+438h (009BDB37).
void* bot_task_move_on_path_describe_009bdb00(Cc13Seg67bHost& host, void* task, void* out) {
    return describe_with_registry(host, task, out, 0x438);
}

// 009C3A60 (009C3A60-009C3ADF): registry task+468h (009C3A97).
void* bot_task_move_to_describe_009c3a60(Cc13Seg67bHost& host, void* task, void* out) {
    return describe_with_registry(host, task, out, 0x468);
}

// 009C8BC0 (009C8BC0-009C8C3F): registry task+4DCh (009C8BF7).
void* bot_task_dive_bomb_describe_009c8bc0(Cc13Seg67bHost& host, void* task, void* out) {
    return describe_with_registry(host, task, out, 0x4DC);
}

// 009CA200 (009CA200-009CA27F): registry task+464h (009CA237).
void* bot_task_retreat_describe_009ca200(Cc13Seg67bHost& host, void* task, void* out) {
    return describe_with_registry(host, task, out, 0x464);
}

// 009CD250 (009CD250-009CD2CF): registry task+4CCh (009CD287).
void* bot_task_strafe_describe_009cd250(Cc13Seg67bHost& host, void* task, void* out) {
    return describe_with_registry(host, task, out, 0x4CC);
}

// 009A79F0 (009A79F0-009A7A36), the enter of the dogfight avoid state at vtable
// 00D1F8F8: +24h byte = 0; +18h = U(0.75, 1.5) (00CEE07C lo at [ESP], 00CE380C hi at
// [ESP+4]; one draw, stream 1) x AvoidTime (row+214h, row = [[state+4]+14h]); +20h and
// +1Ch = 0. The same timer arithmetic as the avoid enters 009A7DE0 / 009A8020, which
// cc9 models as dogfight_avoid_timer_009a7de0 (src/dogfight_task.cpp, bsp_game only).
// The FMUL is a 24 x 24-bit product, exact in both precisions.
void bot_state_dogfight_avoid_enter_009a79f0(Cc13Seg67bHost& host, void* state) {
    void* const row = ptr_at(ptr_at(state, 0x4), 0x14);
    at<std::uint8_t>(state, 0x24) = 0;                                          // 009A7A04
    const float draw = host.random_uniform_00bd2f10(1, 0.75f, 1.5f);            // 009A7A19
    at<float>(state, 0x18) = draw * at<float>(row, 0x214);                       // 009A7A1E
    at<float>(state, 0x20) = 0.0f;                                               // 009A7A2B
    at<float>(state, 0x1C) = 0.0f;                                               // 009A7A30
}

// 009AAF30 (009AAF30-009AAF96), dogfight task slot +54h. 0099B660 (empty) first.
// Then the altitude block of bot_task_update_cruise_profile (src/bot_tasks.cpp ~158)
// with the cruise offset 640h (kDogfightCruisingAlt) behind the 007B8AD0 gate, read
// here from the listing: c = [task+404h]+37Ch; unless c+11h, when 0 > c+4h (COMISS,
// JBE: NaN skips) and c+2Dh is clear, c+18h = tuning+640h (Pilot/Dogfight/CruisingAlt)
// and c+31h = 1; c+2Dh = 0 either way. 0042E740 is called before the c+11h test. Then
// 009AAC70(ECX = task+3F8h, 0.0) and the tail JMP 0099B740(task). The cc9 dogfight host
// has not bound this slot (include/bsp/dogfight_task.hpp ~59).
void bot_task_dogfight_update_cruise_profile_009aaf30(Cc13Seg67bHost& host, void* task) {
    host.bot_task_update_cruise_profile_base_0099b660(task);                     // 009AAF33
    if (host.unit_is_squadron_flight_leader_007b8ad0(ptr_at(task, 0x3FC))) {     // 009AAF3E
        void* const tuning = host.game_tuning_singleton_0042e740();             // 009AAF47
        void* const c = ptr(ptr_at(task, 0x404), 0x37C);
        if (at<std::uint8_t>(c, 0x11) == 0) {                                   // 009AAF58
            if (0.0f > at<float>(c, 0x4) && at<std::uint8_t>(c, 0x2D) == 0) {     // 009AAF61
                at<std::uint8_t>(c, 0x31) = 1;                                  // 009AAF73
                at<float>(c, 0x18) = at<float>(tuning, 0x640);                  // 009AAF77
            }
            at<std::uint8_t>(c, 0x2D) = 0;                                      // 009AAF7A
        }
    }
    host.dogfight_update_approach_009aac70(ptr(task, 0x3F8), 0.0f);              // 009AAF8A
    host.bot_task_abandon_if_stale_0099b740(task);                               // 009AAF92 JMP
}

// 009B35E0 (009B35E0-009B3660). 0099B6E0 (a RET) first; then 008608B0 with ECX =
// the local (+430h, +434h + +458h, +438h) and the stack floats radius = +440h and
// (+460h, +464h, +468h), each moved by FLD/FSTP; RET 10h. Then
// 006C2400(ECX = [task+42Ch]).
void bot_task_land_debug_draw_009b35e0(Cc13Seg67bHost& host, void* task) {
    host.bot_task_debug_draw_base_0099b6e0(task);                                // 009B35E6
    const float centre[3] = {at<float>(task, 0x430),
                             at<float>(task, 0x434) + at<float>(task, 0x458),    // 009B3609
                             at<float>(task, 0x438)};
    host.debug_draw_circle_with_heading_008608b0(centre, at<float>(task, 0x440),
                                                 at<float>(task, 0x460), at<float>(task, 0x464),
                                                 at<float>(task, 0x468));        // 009B364C
    host.air_ops_debug_draw_runway_distances_006c2400(ptr_at(task, 0x42C));      // 009B3657
}

// 009B93D0 (009B93D0-009B93EB): when [task+3F8h]->vtable[0]() returns AL != 0,
// 0099B690(task). The RET 4 argument is not read.
void bot_task_conditioned_tick_009b93d0(Cc13Seg67bHost& host, void* task, float dt) {
    (void)dt;
    if (host.condition_vcall_00_test(ptr_at(task, 0x3F8))) host.bot_task_retire_0099b690(task);
}

// 009B93F0 (009B93F0-009B9410): [task+2F8h]+1ACh x pi (double 00CE3D28, the float
// pi widened) + 1.0 (double 00D7A210), rounded to a float through [ESP]. Modelled at
// 24-bit precision as two float operations; UNCERTAINTY: a 53-bit control word
// would round the product once less.
float bot_task_tight_turn_slot68_009b93f0(void* task) {
    const float v = at<float>(ptr_at(task, 0x2F8), 0x1AC);
    const float product = v * 3.14159274101257324f;
    return product + 1.0f;
}

// 009B9420 (009B9420-009B947B): word +414h = 0; [[task+2F4h]+DF4h]+80h = 0.0 (the
// unit is not null-tested for this first store); 007ED3C0([task+2FCh]) when set;
// with a unit, 007BCBE0(unit, 0) and the +80h store again (the pointers re-read).
void bot_task_leave_alone_update_cruise_profile_009b9420(Cc13Seg67bHost& host, void* task) {
    at<std::uint16_t>(task, 0x414) = 0;                                          // 009B942C
    at<float>(ptr_at(ptr_at(task, 0x2F4), 0xDF4), 0x80) = 0.0f;                  // 009B943B
    if (void* const control = ptr_at(task, 0x2FC)) {
        host.pilot_control_clear_release_orders_007ed3c0(control);              // 009B944D
    }
    if (void* const unit = ptr_at(task, 0x2F4)) {
        host.unit_set_queued_release_orders_007bcbe0(unit, 0);                  // 009B945E
        at<float>(ptr_at(ptr_at(task, 0x2F4), 0xDF4), 0x80) = 0.0f;              // 009B9472
    }
}

// 009B9FD0 (009B9FD0-009BA00F). 009B9F20(task, archive) (009B9FD9); archive->vtable[0Ch]
// ({0,"left"} 00D20368, {3, byte +408h}) (009BA00B).
// UNCERTAINTY: as 009B9F20 for the type-3 payload's upper bytes (only CL is stored).
void bot_task_tight_turn_save_009b9fd0(Cc13Seg67bHost& host, void* task, void* archive) {
    bot_task_pull_up_save_009b9f20(host, task, archive);
    host.archive_vcall_0c_write(archive, name_record(kStrLeft),
                                record(3, at<std::uint8_t>(task, 0x408)));
}

// 009BD380 (009BD380-009BD3AF). 0099D060(task, archive) (009BD389); 009BD300(ECX =
// task+3F8h, archive) (009BD395); 00411450(ECX = task+438h, task+30Ch, archive) (009BD3A8).
void bot_task_move_on_path_save_009bd380(Cc13Seg67bHost& host, void* task, void* archive) {
    bot_task_save_base_0099d060(host, task, archive);
    bot_approach_move_on_path_save_offsets_009bd300(host, ptr(task, 0x3F8), archive);
    host.bot_state_registry_write_current_00411450(ptr(task, 0x438), ptr(task, 0x30C), archive);
}

// 009C3510 (009C3510-009C353F). As 009BD380 with 009C33D0 and the registry task+468h.
void bot_task_move_to_save_009c3510(Cc13Seg67bHost& host, void* task, void* archive) {
    bot_task_save_base_0099d060(host, task, archive);
    bot_approach_move_to_save_009c33d0(host, ptr(task, 0x3F8), archive);
    host.bot_state_registry_write_current_00411450(ptr(task, 0x468), ptr(task, 0x30C), archive);
}

// 009CA0C0 (009CA0C0-009CA0E9): out = (+45Ch, 0.0, +460h) through MOVSS, byte out+0Ch = 1,
// EAX = out.
void* bot_task_retreat_target_position_009ca0c0(void* task, void* out) {
    at<float>(out, 0x0) = at<float>(task, 0x45C);
    at<float>(out, 0x4) = 0.0f;
    at<float>(out, 0x8) = at<float>(task, 0x460);
    at<std::uint8_t>(out, 0xC) = 1;
    return out;
}

// 009CA480 (009CA480-009CA49B): null -> 0; entity->vtable[5Ch](41h) AL != 0 ->
// entity, else 0. The kind-0Fh casts (004BEAD0 and twins) are the same shape with 0Fh.
void* entity_as_kind41_009ca480(Cc13Seg67bHost& host, void* entity) {
    if (entity == nullptr) return nullptr;
    return host.object_vcall_5c(entity, 0x41) ? entity : nullptr;
}

// ============================================================================
// Medium rows, batch 2 (docs/CC13_SEG67B.md 3.47)
// ============================================================================

namespace {

// The shared shape of the three plain state constructors 009BC870, 009BC8A0 and
// 009C6180: +4h = approach; +0Ch, +10h, +14h, +8h = 0 (in that order); vptr.
void* construct_plain_state(void* state, void* approach, std::uint32_t vtable) {
    at<std::uintptr_t>(state, 0x4) = reinterpret_cast<std::uintptr_t>(approach);
    at<std::uint32_t>(state, 0xC) = 0;
    at<std::uint32_t>(state, 0x10) = 0;
    at<std::uint32_t>(state, 0x14) = 0;
    at<std::uint32_t>(state, 0x8) = 0;
    at<std::uint32_t>(state, 0x0) = vtable;
    return state;
}

// 009BC9F0 / 009C2670 (the same 23 instructions at two addresses): a
// local white rgba (1, 1, 1, 1) from [00D7A24C]; unit = [approach+8h] (read before
// the call); approach->vtable[0](&local) (EAX unused); 00860710(ECX = &local, EDX =
// &white; radius = unit+268h).
void circle_state_debug_draw(Cc13Seg67bHost& host, void* state) {
    const float white[4] = {kOne_00d7a24c, kOne_00d7a24c, kOne_00d7a24c, kOne_00d7a24c};
    void* const approach = ptr_at(state, 0x4);
    void* const unit = ptr_at(approach, 0x8);
    float point[3] = {};
    host.approach_vcall_00_target_point(approach, point);
    host.debug_circle64_00860710(point, white, at<float>(unit, 0x268));
}

}  // namespace

// 009B94E0 (009B94E0-009B9581), split-S roll step. d = [task+2F8h], u = [task+2F4h]:
// twice = d+1A8h / d+1BCh doubled (FADD ST0,ST0); off = -pi/2 (qword 00CF48A0, FSUBR)
// - u+C64h, rounded to a float; sq = off * off; v = wrap ? 00438B10(pi [00D7A264],
// u+C68h) : u+C68h; byte +298h = 1; +294h = (-v / twice) * sq; +2D0h = 0.
// 24-bit precision is modelled as float operations; UNCERTAINTY: at 53 bits the
// quotient and the products round once less. Caller: the split s tick 009B9CA0 (no
// Ghidra function; vtable 00D207C0 +64h) at 009B9D05, with wrap = ([task+3F8h] == 1);
// cc9 does not model the split s task.
namespace {
template <class Subtract>
void split_s_roll_step(void* task, std::uint8_t wrap, Subtract wrapped_angle_subtract_00438b10) {
    void* const desc = ptr_at(task, 0x2F8);
    void* const unit = ptr_at(task, 0x2F4);
    const float rate = at<float>(desc, 0x1A8) / at<float>(desc, 0x1BC);         // 009B94F7
    const float twice = rate + rate;                                            // 009B950B
    const float bank = at<float>(unit, 0xC68);
    const float off = static_cast<float>(-1.5707963705062866 -
                                         static_cast<double>(at<float>(unit, 0xC64)));  // 009B9517
    const float sq = off * off;                                                 // 009B952B
    const float v = wrap != 0 ? wrapped_angle_subtract_00438b10(3.14159274101257324f, bank)
                              : bank;                                           // 009B9547
    at<std::uint8_t>(task, 0x298) = 1;                                          // 009B955C
    const float q = -v / twice;                                                 // 009B9563
    at<float>(task, 0x294) = q * sq;                                            // 009B9569
    at<std::uint32_t>(task, 0x2D0) = 0;                                         // 009B9573
}
}  // namespace

void bot_task_split_s_roll_step_009b94e0(Cc13Seg67bHost& host, void* task, std::uint8_t wrap) {
    split_s_roll_step(task, wrap,
                      [&host](float a, float b) { return host.wrapped_angle_subtract_00438b10(a, b); });
}

void bot_task_split_s_roll_step_009b94e0(void* task, std::uint8_t wrap,
                                         float (*wrapped_angle_subtract_00438b10)(float, float)) {
    split_s_roll_step(task, wrap, wrapped_angle_subtract_00438b10);
}

// 009B95D0 (009B95D0-009B963D), the stop task's tick: the plan slots in image order.
void bot_task_stop_tick_009b95d0(void* task, float dt) {
    (void)dt;
    at<float>(task, 0x27C) = 0.0f;
    at<std::uint8_t>(task, 0x280) = 1;
    at<float>(task, 0x2AC) = kOne_00d7a24c;
    at<std::uint8_t>(task, 0x2B0) = 1;
    at<std::uint32_t>(task, 0x2DC) = 0;
    at<float>(task, 0x288) = 0.0f;
    at<std::uint8_t>(task, 0x28C) = 1;
    at<std::uint32_t>(task, 0x2D8) = 0;
    at<float>(task, 0x2A0) = 0.0f;
    at<std::uint8_t>(task, 0x2A4) = 1;
    at<std::uint32_t>(task, 0x2D4) = 0;
    at<float>(task, 0x294) = 0.0f;
    at<std::uint8_t>(task, 0x298) = 1;
    at<std::uint32_t>(task, 0x2D0) = 0;
}

// 009BAE10 (009BAE10-009BAE74): operator new(400h); null -> 0; else 009BAAB0(t, bot, archive).
void* bot_task_gain_altitude_factory_create_009bae10(Cc13Seg67bHost& host, void* bot,
                                                     void* archive) {
    void* const t = host.operator_new_00bf681b(0x400);
    if (t == nullptr) return nullptr;
    return bot_task_gain_altitude_construct_from_stream_009baab0(host, t, bot, archive);
}

// 009BAE80 (009BAE80-009BAEE4): operator new(3FCh); null -> 0; else 009BA960(t, bot, archive).
void* bot_task_immelmann_factory_create_009bae80(Cc13Seg67bHost& host, void* bot, void* archive) {
    void* const t = host.operator_new_00bf681b(0x3FC);
    if (t == nullptr) return nullptr;
    return bot_task_immelmann_construct_from_stream_009ba960(host, t, bot, archive);
}

// 009BAEF0 (009BAEF0-009BAF4E): operator new(3F8h); null -> 0; else 0099C8B0(t, bot,
// archive, kind 0Fh) and vptr = 00D20500 (009BAF37).
void* bot_task_stop_factory_create_009baef0(Cc13Seg67bHost& host, void* bot, void* archive) {
    void* const t = host.operator_new_00bf681b(0x3F8);
    if (t == nullptr) return nullptr;
    bot_task_construct_base_from_stream_0099c8b0(host, t, bot, archive, 0x0F);
    at<std::uint32_t>(t, 0x0) = 0x00D20500u;
    return t;
}

// 009BC6B0 (009BC6B0-009BC6EC): registry approach+40h; "path (path)" 00D20860 ->
// +54h, "follow (path)" 00D20850 -> +6Ch, "circle (path)" 00D20840 -> +104h.
void bot_approach_move_on_path_register_state_names_009bc6b0(Cc13Seg67bHost& host,
                                                             void* approach) {
    static constexpr StateNamePair kPairs[] = {
        {0x00D20860u, 0x54}, {0x00D20850u, 0x6C}, {0x00D20840u, 0x104}};
    register_state_names(host, approach, 0x40, kPairs, 3);
}

// 009BC870 / 009BC8A0 (11 and 12 instructions): vtables 00D208A0 and 00D208BC.
void* bot_state_move_on_path_base_construct_009bc870(void* state, void* approach) {
    return construct_plain_state(state, approach, 0x00D208A0u);
}
void* bot_state_move_on_path_path_construct_009bc8a0(void* state, void* approach) {
    return construct_plain_state(state, approach, 0x00D208BCu);
}

// 009C6180 (009C6180-009C61AC): the plain header with vtable 00D20D04, then bytes
// +18h..+1Bh = 0 and +20h = 0 (+1Ch is not written).
void* bot_state_dive_bomb_fly_above_construct_009c6180(void* state, void* approach) {
    construct_plain_state(state, approach, 0x00D20D04u);
    at<std::uint8_t>(state, 0x18) = 0;
    at<std::uint8_t>(state, 0x19) = 0;
    at<std::uint8_t>(state, 0x1A) = 0;
    at<std::uint8_t>(state, 0x1B) = 0;
    at<std::uint32_t>(state, 0x20) = 0;
    return state;
}

// 009BC9F0 (009BC9F0-009BCA40) and 009C2670 (009C2670-009C26C0).
void bot_state_move_on_path_circle_debug_draw_009bc9f0(Cc13Seg67bHost& host, void* state) {
    circle_state_debug_draw(host, state);
}
void bot_state_move_to_circle_debug_draw_009c2670(Cc13Seg67bHost& host, void* state) {
    circle_state_debug_draw(host, state);
}

// 009BCF70 (009BCF70-009BCFAC): the first half of 009BD0B0. c = [approach+0Ch]; null
// -> 0; c->vtable[114h]()+54h != 00E08F80 (moveonpath) -> 0; else
// 0071BFF0(ECX = c->vtable[114h](), 0) (the PUSH 0 at 009BCF9B is 0071BFF0's argument).
void* bot_approach_move_on_path_command_path_cursor_009bcf70(Cc13Seg67bHost& host,
                                                             void* approach) {
    void* const ctl = ptr_at(approach, 0x0C);
    if (ctl == nullptr) return nullptr;
    void* const director = host.object_vcall_114(ctl);
    if (at<std::uint32_t>(director, 0x54) != kMoveOnPathCommandClass_00e08f80) return nullptr;
    return host.weapon_director_command_path_cursor_0071bff0(
        host.object_vcall_114(ptr_at(approach, 0x0C)), 0);
}

// 009BD1D0 (009BD1D0-009BD27C). 0099B6E0 (a RET) first. Two image statics are set once
// under guards: 00F8A508 = (0, 0, 1, 1) (guard 00F8A518) and 00F8A51C = (1, 1, 0, 1)
// (guard 00F8A52C); this body passes equal local constants. Then
// 00860BE0(ECX = (task+3F8h)->vtable[0](&local), EDX = 00F8A51C; radius 30.0
// [00CE38C8], 00F8A508, detailed 1).
void bot_task_move_on_path_debug_draw_009bd1d0(Cc13Seg67bHost& host, void* task) {
    static constexpr float kBlue_00f8a508[4] = {0.0f, 0.0f, 1.0f, 1.0f};
    static constexpr float kYellow_00f8a51c[4] = {1.0f, 1.0f, 0.0f, 1.0f};
    host.bot_task_debug_draw_base_0099b6e0(task);                                // 009BD1D6
    float point[3] = {};
    const float* const centre = host.approach_vcall_00_target_point(ptr(task, 0x3F8), point);
    host.wire_sphere_00860be0(centre, kYellow_00f8a51c, 30.0f, kBlue_00f8a508, 1);  // 009BD273
}

// 009BDED0 (009BDED0-009BDEFE): the observed entity at approach+2Ch, the observer node at
// approach+18h. Same entity -> nothing; old -> 006952A0(old, node); store; new ->
// 00694A60(new, node).
void bot_approach_move_to_set_observed_target_009bded0(Cc13Seg67bHost& host, void* approach,
                                                       void* entity) {
    void* const old = ptr_at(approach, 0x2C);
    if (entity == old) return;
    if (old != nullptr) host.observer_unregister_pair_006952a0(old, ptr(approach, 0x18));
    at<std::uintptr_t>(approach, 0x2C) = reinterpret_cast<std::uintptr_t>(entity);
    if (entity != nullptr) host.observer_register_pair_00694a60(entity, ptr(approach, 0x18));
}

// 009C5870 (009C5870-009C5892), the aimdive enter: +1Ch = 0; byte +18h = (approach+D1h
// == 0); byte [[approach+4h]+844h] = 0. The cc9 dive-bomb host clears its shared
// db_aim_rearm_1c / db_aim_pull_out_18 only on the aimglide enter (it cites this
// enter's 009C5876 store) and keeps no aimdive enter of its own.
void bot_state_dive_bomb_aim_dive_enter_009c5870(void* state) {
    void* const approach = ptr_at(state, 0x4);
    at<float>(state, 0x1C) = 0.0f;
    at<std::uint8_t>(state, 0x18) = at<std::uint8_t>(approach, 0xD1) == 0 ? 1 : 0;
    at<std::uint8_t>(ptr_at(approach, 0x4), 0x844) = 0;
}

// 009C6220 (009C6220-009C6266): a = *ECX (through FSTP/FLD float), b = *EDX, c = -0.0
// [00D7A208] - b (SUBSS). c > a (FCOMI, JA; unordered falls through) -> c; else a <= b
// or unordered (JBE) -> a; else b. A clamp of a to [-b, b] that keeps a on NaN.
float math_clamp_symmetric_009c6220(const float* a, const float* b) {
    const float av = *a;
    const float bv = *b;
    const float c = -0.0f - bv;
    if (c > av) return c;
    if (!(av > bv)) return av;
    return bv;
}

// 009D58A0 (009D58A0-009D58EA), the path node's destructor body: vptr = 00D214F4;
// for link_minus (+20h): when its +2Ch is 0, vtable[0](1), else its +28h = 0; +20h = 0.
// For link_plus (+24h): when its +28h is 0, vtable[0](1), else its +2Ch = 0; +24h = 0.
void ship_ai_path_node_destruct_body_009d58a0(Cc13Seg67bHost& host, void* node) {
    at<std::uint32_t>(node, 0x0) = kShipAiPathNodeVtable;
    if (void* const minus = ptr_at(node, 0x20)) {
        if (at<std::uint32_t>(minus, 0x2C) == 0) {
            host.path_node_vcall_00_scalar_delete(minus, 1);                    // 009D58BE
        } else {
            at<std::uint32_t>(minus, 0x28) = 0;                                 // 009D58C2
        }
        at<std::uint32_t>(node, 0x20) = 0;
    }
    if (void* const plus = ptr_at(node, 0x24)) {
        if (at<std::uint32_t>(plus, 0x28) == 0) {
            host.path_node_vcall_00_scalar_delete(plus, 1);                     // 009D58DA
        } else {
            at<std::uint32_t>(plus, 0x2C) = 0;                                  // 009D58E2
        }
        at<std::uint32_t>(node, 0x24) = 0;
    }
}

// 009D59C0 (009D59C0-009D5A17): 009D5930 on the node; direction +1: link_plus_length
// (+38h) > 1.0 (COMISS, JBE: NaN recurses) -> &link_plus->x, else the tail JMP repeats
// on link_plus; direction -1: the same with +34h and link_minus; otherwise &node->x.
// The tail recursion is a loop here.
const float* ship_ai_path_node_run_end_point_009d59c0(ShipAiPathNode& node) {
    ShipAiPathNode* n = &node;
    for (;;) {
        ship_ai_path_node_decide_direction_009d5930(*n);                       // 009D59C3
        if (n->direction == 1) {
            if (n->link_plus_length > kOne_00d7a24c) return &n->link_plus->x;  // 009D59D5
            n = n->link_plus;
        } else if (n->direction == -1) {
            if (n->link_minus_length > kOne_00d7a24c) return &n->link_minus->x;
            n = n->link_minus;
        } else {
            return &n->x;                                                      // 009D5A13
        }
    }
}

// 009D5CF0 (009D5CF0-009D5D20): plan = [task+2F4h]; plan+1Ch (search_state) == 0 -> 1;
// 009D5AE0(plan) or 009D5B90(plan, re-read) -> 0; else 1.
bool ship_ai_path_has_no_sided_node_009d5cf0(void* task) {
    auto& plan = *static_cast<ShipAiPathPlanBlock*>(ptr_at(task, 0x2F4));
    if (plan.search_state == 0) return true;
    if (ship_ai_path_any_positive_side_ahead_009d5ae0(plan)) return false;
    auto& again = *static_cast<ShipAiPathPlanBlock*>(ptr_at(task, 0x2F4));
    return !ship_ai_path_any_negative_side_ahead_009d5b90(again);
}

// 009D5D60 (009D5D60-009D5DC9): +2Ch, +28h, +24h, +20h, +1Ch, +18h, +3Ch, +30h, +34h,
// +38h = 0 (image order); byte +40h = 0; +48h = 00BD2F10(ECX = 1; 0.0, 2.0 [00CE3958]),
// one draw on stream 1; +44h = 1000.0 [00CE3804]. Unreferenced (no xrefs, no absolute
// or rel32 references in the image). The same record reset is inlined 60 times in
// 009E5530's first pass, the sixty draws of section 3.42.
void target_ref_record_reset_009d5d60(Cc13Seg67bHost& host, void* record) {
    static constexpr std::size_t kZeroed[] = {0x2C, 0x28, 0x24, 0x20, 0x1C, 0x18, 0x3C, 0x30, 0x34, 0x38};
    for (const std::size_t off : kZeroed) {
        at<float>(record, off) = 0.0f;
    }
    at<std::uint8_t>(record, 0x40) = 0;
    at<float>(record, 0x48) = host.random_uniform_00bd2f10(1, 0.0f, kTwo_00ce3958);  // 009D5DB3
    at<float>(record, 0x44) = 1000.0f;                                             // 009D5DC3
}

// 009D6530 (009D6530-009D654A): vptr = 00D05704; +4h = object; object->vtable[4]()
// (no null test). EAX = holder.
void* ref_holder_construct_009d6530(Cc13Seg67bHost& host, void* holder, void* object) {
    at<std::uint32_t>(holder, 0x0) = 0x00D05704u;
    at<std::uintptr_t>(holder, 0x4) = reinterpret_cast<std::uintptr_t>(object);
    host.ref_vcall_04_add_ref(object);
    return holder;
}

// ---------------------------------------------------------------------------
// Medium rows, batch 3 (docs 3.48)
// ---------------------------------------------------------------------------

// 009DA950 (009DA950-009DA9C1): +0h = 0, then the 009D5D60 record reset with the same
// store order and the same single stream-1 U(0, 2.0 [00CE3958]) draw (009DA9A9) into
// +48h; +44h = 1000.0 [00CE3804]. EAX = record. Unreferenced (no xrefs, no absolute or
// rel32 references in the image).
void* ship_ai_path_node_init_fields_009da950(Cc13Seg67bHost& host, void* record) {
    at<std::uint32_t>(record, 0x0) = 0;                                          // 009DA95C
    target_ref_record_reset_009d5d60(host, record);
    return record;
}

// 009DBD60 (009DBD60-009DBD7F): vptr = 00D17DC0; +8h = 00438E40(ECX = "NavigatorBot"
// 00D2145C); vptr = 00D217DC. EAX = object. Unreferenced. Its destructor body 009DBD80 is
// the 008FC570 byte twin (ledger-only).
void* navigator_bot_named_object_construct_009dbd60(Cc13Seg67bHost& host, void* object) {
    static constexpr char kNavigatorBot_00d2145c[] = "NavigatorBot";
    at<std::uint32_t>(object, 0x0) = 0x00D17DC0u;                                // 009DBD68
    at<std::uintptr_t>(object, 0x8) =
        reinterpret_cast<std::uintptr_t>(host.string_duplicate_00438e40(kNavigatorBot_00d2145c));
    at<std::uint32_t>(object, 0x0) = 0x00D217DCu;                                // 009DBD76
    return object;
}

// 009DE1B0 (009DE1B0-009DE20A): when +1C4h != 0, +368h = +360h = 0.0 and +1C4h = 0;
// then 009DA4E0(blk); +1D4h = throttle clamped to [-1, 1] by COMISS: -1.0 [00D7A260] >
// t -> -1 (JA 009DE1EF), else t > 1.0 [00D7A24C] -> 1 (JBE 009DE1FC); an unordered t
// falls through both tests and is stored unchanged. Unreferenced.
void ship_ai_reset_path_and_set_throttle_009de1b0(Cc13Seg67bHost& host, void* blk,
                                                  float throttle) {
    if (at<std::uint32_t>(blk, 0x1C4) != 0) {
        at<float>(blk, 0x368) = 0.0f;
        at<float>(blk, 0x360) = 0.0f;
        at<std::uint32_t>(blk, 0x1C4) = 0;
    }
    host.ship_ai_clear_path_plan_009da4e0(blk);                                 // 009DE1D9
    float t = throttle;
    if (kMinusOne_00d7a260 > t) {
        t = kMinusOne_00d7a260;
    } else if (t > kOne_00d7a24c) {
        t = kOne_00d7a24c;
    }
    at<float>(blk, 0x1D4) = t;                                                   // 009DE201
}

// 009DFB90 (009DFB90-009DFBD1): the observer member's observed entity is +14h, the member
// itself is the observer (EDX = member). Same entity -> no re-registration; otherwise
// 006952A0(old, member) when old is set, store, 00694A60(new, member) when new is set.
// Then +18h..+20h = point[0..2] (FLD/FSTP, bit copies). Unreferenced.
void ship_ai_observer_member_set_observed_with_point_009dfb90(Cc13Seg67bHost& host,
                                                              void* member, void* entity,
                                                              const float* point) {
    void* const old = ptr_at(member, 0x14);
    if (old != entity) {
        if (old != nullptr) host.observer_unregister_pair_006952a0(old, member);  // 009DFBA5
        at<std::uintptr_t>(member, 0x14) = reinterpret_cast<std::uintptr_t>(entity);
        if (entity != nullptr) host.observer_register_pair_00694a60(entity, member);  // 009DFBB5
    }
    at<float>(member, 0x18) = point[0];
    at<float>(member, 0x1C) = point[1];
    at<float>(member, 0x20) = point[2];
}

// 009E2C40 (009E2C40-009E2CA1): 009DDCC0(ECX = bot+60h, the ship AI); name =
// 00411430(ECX = bot+BB4h; [bot+2264h]); 004C7070([00E188A8], float(x), float(y), name)
// (cdecl, CVTSI2SS; the global is read after the name call); then
// [bot+2264h]->vtable[18h](x, y + 15), the state re-read. Sole caller: the unreferenced
// adapter at 009E2CB0 (no Ghidra function; ECX = bot, EDX = x, stack y, RET 4).
void ship_ai_bot_debug_draw_with_state_name_009e2c40(Cc13Seg67bHost& host, void* bot,
                                                     std::int32_t x, std::int32_t y) {
    ship_ai_debug_draw_009ddcc0(host, static_cast<std::uint8_t*>(bot) + 0x60);  // 009E2C48
    const char* const name = host.state_registry_name_or_undefined_00411430(
        static_cast<std::uint8_t*>(bot) + 0xBB4, ptr_at(bot, 0x2264));           // 009E2C5A
    host.diagnostic_printf_004c7070(host.global_dword_00e188a8(), static_cast<float>(x),
                                    static_cast<float>(y), name);                // 009E2C84
    host.state_vcall_18_debug_text(ptr_at(bot, 0x2264), x, y + 15);            // 009E2C9C
}

// 009E2F10 (009E2F10-009E2F89), slot 1 of the brain record member vtable 00D21898
// (brain+AF8h, ship_ai_brain_record_construct_009f1160). obs = [member+14h]; when byte
// obs+C8h == 0, 00414DB0(obs) first; 004142E0(ECX = member+18h; &tmp, obs+CCh) and
// tmp copied back into +18h..+20h (MOVSS); byte +24h = 1. Then, when
// released->vtable[4]() == [member+14h] (re-read): +14h = 0 and 006952A0(ECX = that
// entity, EDX = member). obs is dereferenced without a null test.
void ship_ai_goal_member_on_observed_released_009e2f10(Cc13Seg67bHost& host, void* member,
                                                       void* released) {
    void* const obs = ptr_at(member, 0x14);
    if (at<std::uint8_t>(obs, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(obs);  // 009E2F25
    float tmp[3];
    host.vector_transform_affine_point_004142e0(&at<float>(member, 0x18), tmp,
                                                static_cast<std::uint8_t*>(obs) + 0xCC);  // 009E2F3B
    at<float>(member, 0x18) = tmp[0];
    at<float>(member, 0x1C) = tmp[1];
    at<float>(member, 0x20) = tmp[2];
    at<std::uint8_t>(member, 0x24) = 1;                                          // 009E2F64
    void* const entity = host.object_vcall_04_get(released);                     // 009E2F6D
    if (entity == ptr_at(member, 0x14)) {
        at<std::uintptr_t>(member, 0x14) = 0;
        host.observer_unregister_pair_006952a0(entity, member);                 // 009E2F7F
    }
}

// 009E69D0 (009E69D0-009E6A29): d = 00438B10(angle, +8h); +34h = 00419010(0, peak,
// pi [00D7A264], 0, |d|), |d| by clearing the sign bit (AND 7FFFFFFFh). A ramp from peak
// at zero error to 0 at pi. Unreferenced.
void heading_error_ramp_set_34_009e69d0(Cc13Seg67bHost& host, void* object, float angle,
                                        float peak) {
    static constexpr float kPi_00d7a264 = 3.14159274f;
    const float d = host.wrapped_angle_subtract_00438b10(angle, at<float>(object, 0x8));  // 009E69E4
    at<float>(object, 0x34) =
        host.interpolate_clamped_00419010(0.0f, peak, kPi_00d7a264, 0.0f, std::fabs(d));  // 009E6A20
}

// 009E6A30 (009E6A30-009E6A87): as 009E69D0 with the span from the stack:
// +38h = 00419010(0, peak, span, 0, |00438B10(angle, +8h)|). Unreferenced.
void heading_error_ramp_set_38_009e6a30(Cc13Seg67bHost& host, void* object, float angle,
                                        float span, float peak) {
    const float d = host.wrapped_angle_subtract_00438b10(angle, at<float>(object, 0x8));  // 009E6A44
    at<float>(object, 0x38) =
        host.interpolate_clamped_00419010(0.0f, peak, span, 0.0f, std::fabs(d));  // 009E6A7E
}

// 009F3CC0 (009F3CC0-009F3CDC), vtable 00D21AE8 slot 5: s = [bot+2264h]; s ?
// 00411430(ECX = bot+BB4h; s) : the empty string 00CE3A0C. The C++ empty string is a
// stand-in for that image address.
const char* ship_ai_gun_bot_sub_state_name_009f3cc0(Cc13Seg67bHost& host, void* bot) {
    void* const s = ptr_at(bot, 0x2264);
    if (s == nullptr) return "";                                                 // 009F3CD7
    return host.state_registry_name_or_undefined_00411430(
        static_cast<std::uint8_t*>(bot) + 0xBB4, s);                             // 009F3CD1
}

// ---------------------------------------------------------------------------
// Medium rows, batch 4 (docs 3.49)
// ---------------------------------------------------------------------------

namespace {

// The describe tail shared by 009AB220, 009B3FF0, 009CFDD0 and 009D4C90: the text
// 004B8490 wrote into a 100h-byte stack buffer is measured inline (the byte loop) and
// appended through 007B7CC0(ECX = out; buffer, length).
void describe_append(Cc13Seg67bHost& host, void* out, const char* buffer) {
    host.string_append_ptr_count_007b7cc0(out, buffer, std::strlen(buffer));
}

void* describe_with_speed(Cc13Seg67bHost& host, void* task, void* out, std::size_t registry) {
    describe_with_registry(host, task, out, registry);
    // [task+3FCh]->vtable[38h](), FSTP to a float, then widened to the double vararg.
    const float speed = host.entity_vcall_38_float(ptr_at(task, 0x3FC));
    char buffer[0x100];
    host.format_string256_f64_ints_004b8490(buffer, " spd:%f", static_cast<double>(speed),
                                            nullptr, 0);
    describe_append(host, out, buffer);
    return out;
}

// The statics 00F8A3F8 (white, guard bit 1 of 00F8A408) and 00F8A40C (yellow, guard bit 1
// of 00F8A41C), lazily set to these values by both kamikaze draws.
void kamikaze_target_sphere_draw(Cc13Seg67bHost& host, void* state) {
    static constexpr float kWhite_00f8a3f8[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    static constexpr float kYellow_00f8a40c[4] = {1.0f, 1.0f, 0.0f, 1.0f};
    if (at<std::uint8_t>(host.debug_feature_owner_0051f460(), 0x2C) == 0) return;
    float point[3] = {};
    const float* const centre = host.approach_vcall_00_target_point(ptr_at(state, 0x4), point);
    host.wire_sphere_00860be0(centre, kYellow_00f8a40c, 5.0f, kWhite_00f8a3f8, 1);
}

}  // namespace

// 009AB220 (009AB220-009AB362): registry task+4FCh (009AB260). Then, when [task+310h] ==
// task+6A4h, "_follow" 00D1FA2C (byte +6E0h set) or "_goaway" 00D1FA24 is formatted and
// appended (009AB2B7-009AB30B); then "_autoaim" 00D1FA18 when byte +35Dh is set
// (009AB310-009AB344).
void* bot_task_dogfight_describe_009ab220(Cc13Seg67bHost& host, void* task, void* out) {
    describe_with_registry(host, task, out, 0x4FC);
    char buffer[0x100];
    if (ptr_at(task, 0x310) == ptr(task, 0x6A4)) {
        host.format_string256_004b8490(buffer,
                                       at<std::uint8_t>(task, 0x6E0) != 0 ? "_follow" : "_goaway");
        describe_append(host, out, buffer);
    }
    if (at<std::uint8_t>(task, 0x35D) != 0) {
        host.format_string256_004b8490(buffer, "_autoaim");
        describe_append(host, out, buffer);
    }
    return out;
}

// 009B3FF0 (009B3FF0-009B40F1): registry task+4B0h (009B4030), then " spd:%f" 00D20010
// with float([task+3FCh]->vtable[38h]()) (009B4096-009B40D3).
void* bot_task_land_describe_009b3ff0(Cc13Seg67bHost& host, void* task, void* out) {
    return describe_with_speed(host, task, out, 0x4B0);
}

// 009CFDD0 (009CFDD0-009CFED1): the 009B3FF0 shape with registry task+434h (009CFE10).
void* bot_task_takeoff_describe_009cfdd0(Cc13Seg67bHost& host, void* task, void* out) {
    return describe_with_speed(host, task, out, 0x434);
}

// 009D4C90 (009D4C90-009D4DA3): registry task+530h (009D4CD0), then " dr:%f  dir:%d
// lr:%d-%d" 00D21440 with float(009D1500(ECX = task+3F8h)), [task+45Ch], [task+460h],
// [task+464h] (the three dwords read after the call, 009D4D3A-009D4D4A).
void* bot_task_torpedo_describe_009d4c90(Cc13Seg67bHost& host, void* task, void* out) {
    describe_with_registry(host, task, out, 0x530);
    const float dr = host.torpedo_approach_time_to_target_009d1500(ptr(task, 0x3F8));  // 009D4D31
    const std::int32_t ints[3] = {at<std::int32_t>(task, 0x45C), at<std::int32_t>(task, 0x460),
                                  at<std::int32_t>(task, 0x464)};
    char buffer[0x100];
    host.format_string256_f64_ints_004b8490(buffer, " dr:%f  dir:%d lr:%d-%d",
                                            static_cast<double>(dr), ints, 3);
    describe_append(host, out, buffer);
    return out;
}

// 009AC7C0 (009AC7C0-009AC86F), vtable 00D1FB98 slot: when byte
// [0051F460()]+2Ch is set, 00860BE0(ECX = [state+4h]->vtable[0](&local), EDX = yellow
// 00F8A40C; 5.0 [00CE3850], white 00F8A3F8, 1). 009ACD90 (at 00D1FBB4) is its exact
// twin, ledger-only.
void bot_state_kamikaze_go_towards_debug_draw_009ac7c0(Cc13Seg67bHost& host, void* state) {
    kamikaze_target_sphere_draw(host, state);
}

// 009BB4A0 (009BB4A0-009BB528). 00414DB0(unit) when byte unit+C8h is 0; dx = float(p[0] -
// unit+FCh), dz = float(p[2] - unit+104h); h = unit->vtable[50h]() (before the atan2);
// a = float(_CIatan2(dx, dz)) (ST1 = dx, ST0 = dz at 009BB4E5); d = 00438B10(a, h);
// 009BB380(unit, p; limit, byte (0 > d)) (FCOMI + JA: unordered -> 0). Unreferenced.
// UNCERTAINTY: std::atan2 in double stands for _CIatan2, as in 00996270.
void pilot_bot_tight_turn_toward_point_009bb4a0(Cc13Seg67bHost& host, void* unit,
                                                const float* point, float limit) {
    if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);
    const float dx = static_cast<float>(static_cast<double>(point[0]) -
                                        static_cast<double>(at<float>(unit, 0xFC)));
    const float dz = static_cast<float>(static_cast<double>(point[2]) -
                                        static_cast<double>(at<float>(unit, 0x104)));
    const float h = host.entity_vcall_50_heading(unit);                          // 009BB4D7
    const float a = static_cast<float>(std::atan2(static_cast<double>(dx), static_cast<double>(dz)));
    const float d = host.math_subtract_wrapped_angle_00438b10(a, h);             // 009BB4F6
    pilot_bot_prepend_tight_turn_to_point_009bb380(host, unit, point, limit,
                                                   0.0f > d ? 1 : 0);            // 009BB51E
}

// 009BB530 (009BB530-009BB63A). Returns at once when pi/4 [00CEDCD0] > [[unit+538h]+25Ch]
// (unordered continues). Then the 009BB4A0 deltas, but a = float(_CIatan2(dz, dx)) (ST1 =
// dz, ST0 = dx at 009BB584); a = float(pi/2 [00CE3830] - a); 0 > a -> a = float(a + 2pi
// [00CE3828]); h = unit->vtable[50h]() (after the atan2); d = 00438B10(a, h); m = d > 0 ?
// d : -0.0 - d (SUBSS); m > threshold (else return; unordered returns) ->
// 009BB380(unit, p; limit, byte (0 > d)) (COMISS). Unreferenced.
// UNCERTAINTY: as 009BB4A0; the double constants are applied in extended precision and
// rounded once on the float stores.
void pilot_bot_tight_turn_if_off_heading_009bb530(Cc13Seg67bHost& host, void* unit,
                                                  const float* point, float threshold,
                                                  float limit) {
    static constexpr double kQuarterPi_00cedcd0 = 0.7853981852531433;
    static constexpr double kHalfPi_00ce3830 = 1.5707963705062866;
    if (kQuarterPi_00cedcd0 > static_cast<double>(at<float>(ptr_at(unit, 0x538), 0x25C))) return;
    if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);
    const float dx = static_cast<float>(static_cast<double>(point[0]) -
                                        static_cast<double>(at<float>(unit, 0xFC)));
    const float dz = static_cast<float>(static_cast<double>(point[2]) -
                                        static_cast<double>(at<float>(unit, 0x104)));
    float a = static_cast<float>(std::atan2(static_cast<double>(dz), static_cast<double>(dx)));
    a = static_cast<float>(kHalfPi_00ce3830 - static_cast<double>(a));
    if (0.0f > a) a = static_cast<float>(static_cast<double>(a) + kTwoPi_00ce3828);
    const float h = host.entity_vcall_50_heading(unit);                          // 009BB5BA
    const float d = host.math_subtract_wrapped_angle_00438b10(a, h);             // 009BB5CA
    const float m = d > 0.0f ? d : -0.0f - d;
    if (!(m > threshold)) return;                                                // 009BB60B
    pilot_bot_prepend_tight_turn_to_point_009bb380(host, unit, point, limit,
                                                   0.0f > d ? 1 : 0);            // 009BB630
}

// 009D9390 (009D9390-009D9430). Byte +44h set -> 0. Otherwise +44h = 1; zone = [+8h]
// null -> 0; s = (signed char) +14h, 0 -> 0; exactly one of +28h / +2Ch and exactly one
// of +20h / +24h must be non-zero, else 0. Then 00423520(ECX = zone; &out,
// (s < 0) + (short) +0Ch, s > 0 ? plus_distance : minus_distance) and +18h, +1Ch =
// out[0], out[1]; returns 1. Sole caller: 009DA008, inside the unreferenced code at
// 009D9F90 (no Ghidra function).
bool ship_ai_path_node_resolve_turn_point_009d9390(Cc13Seg67bHost& host, void* node,
                                                   float plus_distance, float minus_distance) {
    if (at<std::uint8_t>(node, 0x44) != 0) return false;
    void* const zone = ptr_at(node, 0x8);
    at<std::uint8_t>(node, 0x44) = 1;                                            // 009D93A5
    if (zone == nullptr) return false;
    const std::int8_t s = at<std::int8_t>(node, 0x14);
    if (s == 0) return false;
    if ((at<std::uint32_t>(node, 0x28) != 0) == (at<std::uint32_t>(node, 0x2C) != 0)) return false;
    if ((at<std::uint32_t>(node, 0x20) != 0) == (at<std::uint32_t>(node, 0x24) != 0)) return false;
    const float distance = s > 0 ? plus_distance : minus_distance;
    const std::int32_t index = (s < 0 ? 1 : 0) + at<std::int16_t>(node, 0xC);
    float out[2] = {};
    const float* const p =
        host.avoid_zone_corner_offset_point_00423520(zone, out, index, distance);  // 009D9411
    at<float>(node, 0x18) = p[0];
    at<float>(node, 0x1C) = p[1];
    return true;
}

// 009E7FA0 (009E7FA0-009E7FBD): 009DE280(ECX = brain+8h; heading, goal, penalty), the
// arguments re-pushed in order. Unreferenced (docs/SHIP_AI_OPEN_ITEMS.md).
void ship_ai_set_navigation_goal_with_penalty_brain_009e7fa0(Cc13Seg67bHost& host, void* brain,
                                                             float heading, const float* goal,
                                                             float penalty) {
    ship_ai_set_navigation_goal_with_penalty_009de280(host, ptr(brain, 0x8), heading, goal,
                                                      penalty);                 // 009E7FB8
}

// 009EAAD0 (009EAAD0-009EABDA), vtable slot at 00D21974. brain = [sm+4h] (re-read per
// call). When [sm+D4h] == sm+58h: yturn = (+BCh > 0.0) (COMISS, unordered -> 0) first;
// aft = 009E9A50(brain, 0.0); fore = 009E9910(brain, 0.0); close = byte +90h != 0; fwdt
// = byte +88h != 0; name = 00411430(ECX = sm+20h; [sm+D4h]); 004C7070([00E188A8],
// float(x), float(y), "%s %d/%d, fwdt:%d, close:%d, Yturn:%d" 00D21A50, name, fore, aft,
// fwdt, close, yturn). Otherwise the same with "%s %d/%d" 00D21A44 and name, fore, aft.
void ship_ai_sub_attack_debug_text_009eaad0(Cc13Seg67bHost& host, void* sm, std::int32_t x,
                                            std::int32_t y) {
    if (ptr_at(sm, 0xD4) == ptr(sm, 0x58)) {
        const std::int32_t yturn = at<float>(sm, 0xBC) > 0.0f ? 1 : 0;           // 009EAAEB
        const std::int32_t aft = host.sub_tubes_ready_aft_009e9a50(ptr_at(sm, 0x4), 0.0f);
        const std::int32_t fore = host.sub_tubes_ready_fore_009e9910(ptr_at(sm, 0x4), 0.0f);
        const std::int32_t close = at<std::uint8_t>(sm, 0x90) != 0 ? 1 : 0;
        const std::int32_t fwdt = at<std::uint8_t>(sm, 0x88) != 0 ? 1 : 0;
        const char* const name =
            host.state_registry_name_or_undefined_00411430(ptr(sm, 0x20), ptr_at(sm, 0xD4));
        const std::int32_t ints[5] = {fore, aft, fwdt, close, yturn};
        host.diagnostic_printf_name_ints_004c7070(
            host.global_dword_00e188a8(), static_cast<float>(x), static_cast<float>(y),
            "%s %d/%d, fwdt:%d, close:%d, Yturn:%d", name, ints, 5);             // 009EAB6C
        return;
    }
    const std::int32_t aft = host.sub_tubes_ready_aft_009e9a50(ptr_at(sm, 0x4), 0.0f);
    const std::int32_t fore = host.sub_tubes_ready_fore_009e9910(ptr_at(sm, 0x4), 0.0f);
    const char* const name =
        host.state_registry_name_or_undefined_00411430(ptr(sm, 0x20), ptr_at(sm, 0xD4));
    const std::int32_t ints[2] = {fore, aft};
    host.diagnostic_printf_name_ints_004c7070(host.global_dword_00e188a8(),
                                              static_cast<float>(x), static_cast<float>(y),
                                              "%s %d/%d", name, ints, 2);                // 009EABD0
}

// ---------------------------------------------------------------------------
// The three former false twins of 3.48 (docs 3.50). Each matched another body's bytes
// outside the rel32 operands, but its CALL target differs.
// ---------------------------------------------------------------------------

// 009DFA60 (009DFA60-009DFA82): p = [slot]; p -> 009DA880(ECX = p), 00BF65AC(p) and
// [slot] = 0. 0046A090 makes the same moves around 00469010. Unreferenced.
void ship_ai_path_node_delete_009dfa60(Cc13Seg67bHost& host, void* slot) {
    void* const p = ptr_at(slot, 0x0);
    if (p == nullptr) return;
    ship_ai_path_node_destroy_009da880(host, p);                                 // 009DFA6C
    host.crt_free_00bf65ac(p);                                                   // 009DFA72
    at<std::uintptr_t>(slot, 0x0) = 0;
}

// 009DFA90 (009DFA90-009DFAB2): the 009DFA60 shape over 0064A610 (the observed-owner
// base destroy). Unreferenced.
void observed_owner_delete_009dfa90(Cc13Seg67bHost& host, void* slot) {
    void* const p = ptr_at(slot, 0x0);
    if (p == nullptr) return;
    host.observed_owner_base_destroy_0064a610(p);                                // 009DFA9C
    host.crt_free_00bf65ac(p);                                                   // 009DFAA2
    at<std::uintptr_t>(slot, 0x0) = 0;
}

// ---------------------------------------------------------------------------
// Medium rows, batch 5 (docs 3.51)
// ---------------------------------------------------------------------------

namespace {

// 009B8260 / 009C80D0 share this shape: 0099B6E0(task); 00860BE0(ECX = task+marker, EDX
// = red; 10.0 [00CE38B8], red, 0); then 00860BE0(ECX = (task+3F8h)->vtable[0](&local),
// EDX = white; 10.0, white, 0). Each colour is a lazily initialised static (its guard
// test is emitted twice); the values are passed as equal local constants.
void bomb_task_markers_debug_draw(Cc13Seg67bHost& host, void* task, std::size_t marker) {
    static constexpr float kRed[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    static constexpr float kWhite[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    host.bot_task_debug_draw_base_0099b6e0(task);
    host.wire_sphere_00860be0(static_cast<const float*>(ptr(task, marker)), kRed, 10.0f, kRed, 0);
    float point[3] = {};
    const float* const centre = host.approach_vcall_00_target_point(ptr(task, 0x3F8), point);
    host.wire_sphere_00860be0(centre, kWhite, 10.0f, kWhite, 0);
}

// The clamp both rect distances use: -e > v -> -e (FCHS of the float extent); v > e ->
// e; else v, so an unordered v is kept.
float clamp_to_extent(float v, float e) {
    if (-e > v) return -e;
    if (v > e) return e;
    return v;
}

// 009D7780 / 009D7930 after the delta: project (dx, dz) on the two axes, clamp, rebuild
// the closest point from the centre +20h/+24h, and return its distance to the point (0
// at or under 1e-10 [00CE3820] squared, unordered included). Every x87 step is modelled
// as a float operation (24-bit precision control); at 53 bits the products and sums
// would round once less.
float rect_distance_from_delta(const void* rc, const float* p, float dx, float dz,
                               std::size_t axis_a, std::size_t axis_b, std::size_t ext_a,
                               std::size_t ext_b) {
    void* const r = const_cast<void*>(rc);
    const float u = at<float>(r, axis_a + 4) * dz + at<float>(r, axis_a) * dx;
    const float cu = clamp_to_extent(u, at<float>(r, ext_a));
    const float v = at<float>(r, axis_b + 4) * dz + at<float>(r, axis_b) * dx;
    const float cv = clamp_to_extent(v, at<float>(r, ext_b));
    const float bx = at<float>(r, axis_b) * cv;
    const float bz = cv * at<float>(r, axis_b + 4);
    const float ax = at<float>(r, axis_a) * cu;
    const float az = cu * at<float>(r, axis_a + 4);
    const float qx = (at<float>(r, 0x20) + ax) + bx;
    const float qz = (at<float>(r, 0x24) + az) + bz;
    const float ex = qx - p[0];                                                  // FSUBRP
    const float ez = qz - p[1];
    const float d2 = ex * ex + ez * ez;
    if (!(static_cast<double>(d2) > 1e-10)) return 0.0f;
    return std::sqrt(d2);                                                        // 00BF7030
}

}  // namespace

// 009B8260 (009B8260-009B838F), at 00D20214: the marker at task+4C8h.
void bot_task_level_bomb_debug_draw_009b8260(Cc13Seg67bHost& host, void* task) {
    bomb_task_markers_debug_draw(host, task, 0x4C8);
}

// 009C80D0 (009C80D0-009C81FF), at 00D20E1C: the marker at task+4D0h.
void bot_task_dive_bomb_debug_draw_009c80d0(Cc13Seg67bHost& host, void* task) {
    bomb_task_markers_debug_draw(host, task, 0x4D0);
}

// 009D7780 (009D7780-009D7922): dx = p[0] - +20h, dz = p[1] - +24h; axes +28h/+2Ch and
// +30h/+34h with half-extents +38h and +3Ch. Unreferenced.
float oriented_rect_distance_to_point_009d7780(const void* rc, const float* point) {
    void* const rect = const_cast<void*>(rc);
    const float dx = point[0] - at<float>(rect, 0x20);
    const float dz = point[1] - at<float>(rect, 0x24);
    return rect_distance_from_delta(rect, point, dx, dz, 0x28, 0x30, 0x38, 0x3C);
}

// 009D7930 (009D7930-009D7AE2): dx = p[0] - +44h, dz = p[1] - +48h. Byte +68h set -> the
// distance of that delta itself (009D7A8D). Otherwise axes +4Ch/+50h and +54h/+58h with
// half-extents +5Ch and +60h, but the closest point is rebuilt from +20h/+24h, not from
// +44h/+48h (009D7A4B, 009D7A55). Unreferenced.
float oriented_rect2_distance_to_point_009d7930(const void* rc, const float* point) {
    void* const rect = const_cast<void*>(rc);
    const float dx = point[0] - at<float>(rect, 0x44);
    const float dz = point[1] - at<float>(rect, 0x48);
    if (at<std::uint8_t>(rect, 0x68) != 0) {
        const float d2 = dz * dz + dx * dx;
        if (!(static_cast<double>(d2) > 1e-10)) return 0.0f;
        return std::sqrt(d2);
    }
    return rect_distance_from_delta(rect, point, dx, dz, 0x4C, 0x54, 0x5C, 0x60);
}

// 009D6E50 (009D6E50-009D7033). x = *angle clamped to [-2.0 [00CE7D7C], 2.0 [00CE3958]]
// (COMISS; unordered kept) and stored back. Byte profile+100h set -> 0.0. Otherwise i =
// _ftol((x + 2.0) * 16.0 + 0.5) clamped to [0, 64]; b0 = profile[i]; 0 -> 0.0.
// Left walk (i > 0): while the running minimum lmin > 0 and j > 0, c = profile[--j]; c <
// lmin -> lmin = c, lpos = j; else c > b0 and j > lpeak (0) -> lpeak = j. Right walk (i <
// 64): while rmin > 0 and j < 64, c = profile[++j]; c < rmin -> rmin = c, rpos = j; else
// c > b0 and j < rpeak (64) -> rpeak = j. Then: rmin > lmin -> (lpos, lmin); rmin < lmin
// -> (rpos, rmin); equal and below b0 -> the nearer side, the right on a tie; equal to
// b0 -> lpeak == 0 && rpeak == 64 ? (i < 64 - i ? 0 : 64) : (lpeak < 64 - rpeak ? lpeak
// : rpeak). *angle = float(pos * 0.0625 - 2.0), then 00415690(angle, &-2.0, &2.0).
// Returns float(the chosen byte). Unreferenced.
// UNCERTAINTY: the index sum is modelled in float (24-bit precision control); _ftol
// truncates after a double store, which a float value survives exactly.
float byte_profile65_sample_009d6e50(Cc13Seg67bHost& host, const void* profile, float* angle) {
    static constexpr float kLow_00ce7d7c = -2.0f;
    const auto* const bytes = static_cast<const std::uint8_t*>(profile);
    float x = *angle;
    if (kLow_00ce7d7c > x) {
        x = kLow_00ce7d7c;
    } else if (x > kTwo_00ce3958) {
        x = kTwo_00ce3958;
    }
    *angle = x;                                                                  // 009D6E8A
    if (bytes[0x100] != 0) return 0.0f;
    std::int32_t i = static_cast<std::int32_t>((x + 2.0f) * 16.0f + 0.5f);      // 009D6EBB
    if (i < 0) i = 0;
    else if (i > 0x40) i = 0x40;
    const std::uint8_t b0 = bytes[i];
    if (b0 == 0) return 0.0f;
    std::uint8_t lmin = b0, rmin = b0;
    std::int32_t lpos = i, rpos = i, lpeak = 0, rpeak = 0x40;
    if (i > 0) {
        std::int32_t j = i;
        while (lmin != 0) {
            const std::uint8_t c = bytes[j - 1];
            --j;
            if (c < lmin) {
                lmin = c;
                lpos = j;
            } else if (c > b0 && j > lpeak) {
                lpeak = j;
            }
            if (j <= 0) break;
        }
    }
    if (i < 0x40) {
        std::int32_t j = i;
        while (rmin != 0) {
            const std::uint8_t c = bytes[j + 1];
            ++j;
            if (c < rmin) {
                rmin = c;
                rpos = j;
            } else if (c > b0 && j < rpeak) {
                rpeak = j;
            }
            if (j >= 0x40) break;
        }
    }
    std::int32_t pos;
    std::uint8_t result;
    if (rmin > lmin) {
        pos = lpos;
        result = lmin;
    } else if (rmin < lmin) {
        pos = rpos;
        result = rmin;
    } else if (lmin < b0) {
        result = lmin;
        pos = (i - lpos) < (rpos - i) ? lpos : rpos;                             // 009D6FA9
    } else {
        result = b0;
        if (lpeak == 0 && rpeak == 0x40) {
            pos = i < 0x40 - i ? 0 : 0x40;                                       // 009D6FC6
        } else {
            pos = lpeak < 0x40 - rpeak ? lpeak : rpeak;                          // 009D6FDE
        }
    }
    *angle = static_cast<float>(pos) * 0.0625f - 2.0f;                           // 009D7011
    const float low = kLow_00ce7d7c;
    const float high = kTwo_00ce3958;
    host.math_clamp_in_place_00415690(angle, &low, &high);                       // 009D7013
    return static_cast<float>(result);
}

// ---------------------------------------------------------------------------
// Batch 6 (docs 3.52)
// ---------------------------------------------------------------------------

// 009BC5C0 (009BC5C0-009BC674), at 00D20670. A local string is set up inline (capacity
// 0Fh, size 0, empty buffer) and 00408720 assigns "leavealone" 00D206C0 (10 chars) to
// it; base = 009A1A90(task, &temp); out = 004D0860(out, base, &name), base on the left;
// then temp, then name are released inline (capacity +18h >= 10h frees +4h through
// 00BF65AC; the ten-character name never allocates).
void* bot_task_leave_alone_describe_009bc5c0(Cc13Seg67bHost& host, void* task, void* out) {
    alignas(4) std::uint8_t name[0x1C] = {};
    at<std::uint32_t>(name, 0x18) = 0xF;
    host.string_assign_counted_00408720(name, "leavealone", 10);                 // 009BC5FD
    alignas(4) std::uint8_t temp[0x1C] = {};
    const void* const base = host.bot_task_debug_name_009a1a90(task, temp);       // 009BC60D
    host.string_plus_string_004d0860(out, base, name);                           // 009BC622
    if (at<std::uint32_t>(temp, 0x18) >= 0x10) host.crt_free_00bf65ac(ptr_at(temp, 0x4));
    if (at<std::uint32_t>(name, 0x18) >= 0x10) host.crt_free_00bf65ac(ptr_at(name, 0x4));
    return out;
}

// 009BDD70 (009BDD70-009BDDB2): 00411E70 on registry approach+70h with "moveto (moveto)"
// 00D20998 -> +84h, "follow (moveto)" 00D20988 -> +9Ch, "circle (moveto)" 00D20978 ->
// +134h. Unreferenced. The 009BC6B0 shape.
void bot_approach_move_to_register_state_names_009bdd70(Cc13Seg67bHost& host, void* approach) {
    static constexpr StateNamePair kPairs[] = {
        {0x00D20998u, 0x84}, {0x00D20988u, 0x9C}, {0x00D20978u, 0x134}};
    register_state_names(host, approach, 0x70, kPairs, 3);
}

// 009D6650 (009D6650-009D66A9): 0075B430(ECX = message; 9Dh); word +18h = 0, byte +1Ah =
// 0; +4h = 1; vptr 00D0339C; +24h = a, +28h = b, +2Ch = c, +1Ch = e, +20h = d (MOVSS bit
// copies). Unreferenced (unit_message_arms.cpp lists 9Dh as unresolved).
void* session_message_9d_construct_009d6650(Cc13Seg67bHost& host, void* message, float a,
                                            float b, float c, float d, std::int32_t e) {
    host.session_message_construct_base_0075b430(message, 0x9D);                 // 009D6658
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint32_t>(message, 0x4) = 1;
    at<std::uint32_t>(message, 0x0) = 0x00D0339Cu;
    at<float>(message, 0x24) = a;
    at<float>(message, 0x28) = b;
    at<float>(message, 0x2C) = c;
    at<std::int32_t>(message, 0x1C) = e;
    at<float>(message, 0x20) = d;
    return message;
}

// 009DBDB0 (009DBDB0-009DBDE9): vptr = 00D2152C; [+20h] -> its vtable[0](1); then +24h,
// +20h, +34h, +38h, +1Ch = 0 and +30h = 1.0e7 [00CF58EC]. Pushed by address as the
// element destructor of the `eh vector destructor iterator` calls at 009DBDF2,
// 009DBF41, 009DDBA1 and 009E023E.
void ship_ai_path_plan_destruct_body_009dbdb0(Cc13Seg67bHost& host, void* plan) {
    at<std::uint32_t>(plan, 0x0) = 0x00D2152Cu;
    if (void* const owned = ptr_at(plan, 0x20)) host.path_node_vcall_00_scalar_delete(owned, 1);
    at<std::uint32_t>(plan, 0x24) = 0;
    at<std::uint32_t>(plan, 0x20) = 0;
    at<std::uint32_t>(plan, 0x34) = 0;
    at<std::uint32_t>(plan, 0x38) = 0;
    at<std::uint32_t>(plan, 0x1C) = 0;
    at<float>(plan, 0x30) = 1.0e7f;                                              // 009DBDE3
}

// 009E5220 (009E5220-009E5235): +4h = 009DF8F0() (the 0Ch self-linked sentinel; ECX is
// passed but unread), +8h = 0. EAX = list. 0041E1B0 is the same shape over 0041BF50, which
// allocates 10h. Unreferenced.
void* ship_ai_traffic_list_construct_009e5220(Cc13Seg67bHost& host, void* list) {
    at<std::uintptr_t>(list, 0x4) =
        reinterpret_cast<std::uintptr_t>(ship_ai_traffic_list_head_node_009df8f0(host));  // 009E5223
    at<std::uint32_t>(list, 0x8) = 0;
    return list;
}

// 009CFA10 (009CFA10-009CFA2E). 0099B6E0 (a RET) first; then, when [[task+3FCh]+BF4h]
// is set, the tail JMP 006C2400(ECX = it).
void bot_task_takeoff_debug_draw_009cfa10(Cc13Seg67bHost& host, void* task) {
    host.bot_task_debug_draw_base_0099b6e0(task);                                // 009CFA13
    if (void* const holder = ptr_at(ptr_at(task, 0x3FC), 0xBF4)) {
        host.air_ops_debug_draw_runway_distances_006c2400(holder);              // 009CFA29 JMP
    }
}

// ---------------------------------------------------------------------------
// Debug draws, batch 16 (docs 3.62). The colour records are lazily initialised statics
// (guard test emitted twice); their values are passed here as equal local constants.
// 009A9E10 and 009CC8F0 are not bodies here: both equal 007B7290
// (bot_task_rocket_debug_draw_007b7290, cc13_seg53U.cpp) with only the 00F8xxxx
// statics moved.
// ---------------------------------------------------------------------------

namespace {

constexpr float kDebugRed[4] = {1.0f, 0.0f, 0.0f, 1.0f};
constexpr float kDebugGreen[4] = {0.0f, 1.0f, 0.0f, 1.0f};
constexpr float kDebugWhite[4] = {1.0f, 1.0f, 1.0f, 1.0f};
constexpr float kDebugBlue[4] = {0.0f, 0.0f, 1.0f, 1.0f};
constexpr float kDebugYellow[4] = {1.0f, 1.0f, 0.0f, 1.0f};
constexpr float kDebugMarkerRadius_00ce38b8 = 10.0f;
constexpr float kTorpedoFanY_00ce3930 = 20.0f;
constexpr float kTorpedoFanRadiusLong_00ce3804 = 1000.0f;
constexpr float kTorpedoFanRadius_00d1f980 = 700.0f;
constexpr double kTorpedoFanPi_00ce3d28 = 3.1415927410125732;      // widened float pi
constexpr double kTorpedoFanSectors_00cee930 = 18.0;
constexpr double kTorpedoFanHalfPi_00ce3830 = 1.5707963705062866;  // widened float pi/2
constexpr double kTorpedoFanTwoPi_00ce3828 = 6.2831854820251465;   // widened float 2pi
constexpr float kTorpedoHeadingLength_00ce3938 = 50.0f;

// The bearing-to-angle step both 009D4230 loops use: a = float(pi/2 - a); 0 > a (FLDZ,
// FCOMPI, JBE: unordered keeps a) -> a = float(a + 2pi).
float debug_bearing_angle(float a) {
    a = static_cast<float>(kTorpedoFanHalfPi_00ce3830 - static_cast<double>(a));  // DC 2D FSUBR
    if (0.0f > a) a = static_cast<float>(static_cast<double>(a) + kTorpedoFanTwoPi_00ce3828);
    return a;
}

void* refreshed_entity(Cc13Seg67bHost& host, void* e) {
    if (at<std::uint8_t>(e, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(e);
    return e;
}

}  // namespace

// 009A5AF0 (009A5AF0-009A5CF4), depth-charge task slot +4h (00D1F73C): 0099B6E0; a red
// marker at task+4E8h; a white marker at (task+3F8h)->vtable[0](&local); a green marker
// at the target's (task+3FCh, pose refreshed at 009A5C2F) x and z with y = task+434h +
// task+42Ch (009A5C3C FLD [EDI+3Ch], FADD [EDI+34h], EDI = task+3F8h). The target's
// +100h is stored and then overwritten (dead). Every sphere: radius 10.0 [00CE38B8],
// the same colour in EDX and on the stack, detailed byte 0.
void bot_task_depth_charge_debug_draw_009a5af0(Cc13Seg67bHost& host, void* task) {
    host.bot_task_debug_draw_base_0099b6e0(task);                                 // 009A5AF7
    host.wire_sphere_00860be0(static_cast<const float*>(ptr(task, 0x4E8)), kDebugRed,
                              kDebugMarkerRadius_00ce38b8, kDebugRed, 0);         // 009A5B80
    float point[3] = {};
    const float* const aim = host.approach_vcall_00_target_point(ptr(task, 0x3F8), point);
    host.wire_sphere_00860be0(aim, kDebugWhite, kDebugMarkerRadius_00ce38b8, kDebugWhite,
                              0);                                                 // 009A5C19
    void* const t = refreshed_entity(host, ptr_at(task, 0x3FC));
    const float marker[3] = {at<float>(t, 0xFC), at<float>(task, 0x434) + at<float>(task, 0x42C),
                             at<float>(t, 0x104)};
    host.wire_sphere_00860be0(marker, kDebugGreen, kDebugMarkerRadius_00ce38b8, kDebugGreen,
                              0);                                                 // 009A5CEC
}

// 009D4230 (009D4230-009D4843), torpedo task slot +4h (00D213CC). 0099B6E0; a sphere at
// (task+3F8h)->vtable[0](&local), EDX green, stack white, radius 10.0, detailed 0. With
// task+4C4h null it returns there (009D42E6). Otherwise a fan of 36 spokes from C =
// (u.x, 20.0 [00CE3930], u.z), u = task+4C4h with its pose refreshed:
//   a_i = float(float(i x pi) / 18.0) (FILD, FMUL qword [00CE3D28], FDIV qword [00CEE930]),
//   then the bearing step; r = task+45Ch == i ? 1000.0 [00CE3804] : 700.0 [00D1F980];
//   P_i = (C.x + cos(a_i) r, 20.0, C.z + r sin(a_i)); line C -> P_i (0085F990, ECX = C,
//   EDX = P_i). Spoke 0: colour0 white, colour1 = byte task+428h ? green : red. Spokes
//   1..35: both colours = byte task+428h+i ? green : red.
// Then a heading line from the unit at task+3FCh (pose refreshed): h = the bearing step of
// task+458h; E = (p.x + cos(h) x 50.0, p.y + 0.0 x 50.0, p.z + 50.0 x sin(h)) ([00CE3938],
// 009D4744 DC C9); line p -> E, red and red.
// UNCERTAINTY: FCOS/FSIN run at extended precision (precision control does not apply);
// std::cos/std::sin in double then the float store can differ by an ulp in rare cases.
// The FMUL/FDIV of a_i are modelled at 24 bits; 53-bit is the alternative.
void bot_task_torpedo_debug_draw_009d4230(Cc13Seg67bHost& host, void* task) {
    host.bot_task_debug_draw_base_0099b6e0(task);                                 // 009D4239
    float point[3] = {};
    const float* const aim = host.approach_vcall_00_target_point(ptr(task, 0x3F8), point);
    host.wire_sphere_00860be0(aim, kDebugGreen, kDebugMarkerRadius_00ce38b8, kDebugWhite,
                              0);                                                 // 009D42D9
    void* const u = ptr_at(task, 0x4C4);
    if (u == nullptr) return;                                                     // 009D42E6
    refreshed_entity(host, u);
    const float c[3] = {at<float>(u, 0xFC), kTorpedoFanY_00ce3930, at<float>(u, 0x104)};
    for (std::int32_t i = 0; i < 36; ++i) {
        const float step = static_cast<float>(static_cast<float>(i) * kTorpedoFanPi_00ce3d28);
        const float a = debug_bearing_angle(
            static_cast<float>(static_cast<double>(step) / kTorpedoFanSectors_00cee930));
        const float ca = static_cast<float>(std::cos(static_cast<double>(a)));    // 009D437E
        const float sa = static_cast<float>(std::sin(static_cast<double>(a)));    // 009D4394
        // 009D439A CMP [EBP+45Ch], EBX: EBX is i, so the spoke whose index equals
        // task+45Ch is the long one.
        const float r = at<std::int32_t>(task, 0x45C) == i ? kTorpedoFanRadiusLong_00ce3804
                                                           : kTorpedoFanRadius_00d1f980;
        const float tip[3] = {ca * r + c[0], kTorpedoFanY_00ce3930, r * sa + c[2]};
        const bool set = at<std::uint8_t>(task, 0x428 + static_cast<std::size_t>(i)) != 0;
        const float* const sector = set ? kDebugGreen : kDebugRed;
        host.line_rgba_floats_0085f990(c, tip, i == 0 ? kDebugWhite : sector, sector);
    }
    void* const unit = refreshed_entity(host, ptr_at(task, 0x3FC));
    const float p[3] = {at<float>(unit, 0xFC), at<float>(unit, 0x100), at<float>(unit, 0x104)};
    const float h = debug_bearing_angle(at<float>(task, 0x458));
    const float ch = static_cast<float>(std::cos(static_cast<double>(h)));        // 009D471B
    const float sh = static_cast<float>(std::sin(static_cast<double>(h)));        // 009D472D
    const float end[3] = {ch * kTorpedoHeadingLength_00ce3938 + p[0],
                          0.0f * kTorpedoHeadingLength_00ce3938 + p[1],
                          kTorpedoHeadingLength_00ce3938 * sh + p[2]};
    host.line_rgba_floats_0085f990(p, end, kDebugRed, kDebugRed);                 // 009D4837
}

// 009DB060 (009DB060-009DB2A8), the ship-AI slot at 00D216A0; __thiscall(ECX = this; two
// stack arguments, unread), RET 8. owner = this+4h; cursor = 0071BFF0(owner+AB8h, 0);
// path = cursor+4h; null -> return (NEG/SBB/TEST 00E19B95). A second 0071BFF0 call gives
// the index k = cursor+8h. q = path->vtable[8](&q, k) with q.y = 0; a line from the unit
// at owner+AA8h (pose refreshed, its +FCh position) to q, colour0 blue, colour1 yellow.
// Then while k < path->vtable[0Ch]() - 1 (signed, re-read each pass): a = point k, b =
// point k + 1, each with y = 0; line a -> b, yellow and yellow.
void ship_ai_debug_draw_command_path_009db060(Cc13Seg67bHost& host, void* self) {
    void* const owner = ptr_at(self, 0x4);
    void* const cursor = host.weapon_director_command_path_cursor_0071bff0(
        ptr_at(owner, 0xAB8), 0);                                                 // 009DB074
    // EDI = cursor+4h; the path pointer is re-read from it before every virtual call.
    if (ptr_at(cursor, 0x4) == nullptr) return;                                   // 009DB08A
    void* const cursor2 = host.weapon_director_command_path_cursor_0071bff0(
        ptr_at(owner, 0xAB8), 0);                                                 // 009DB09B
    std::int32_t k = at<std::int32_t>(cursor2, 0x8);
    float q[3] = {};
    host.path_vcall_08_point(ptr_at(cursor, 0x4), q, k);                          // 009DB0B0
    q[1] = 0.0f;
    void* const unit = refreshed_entity(host, ptr_at(owner, 0xAA8));
    host.line_rgba_floats_0085f990(static_cast<const float*>(ptr(unit, 0xFC)), q, kDebugBlue,
                                   kDebugYellow);                                 // 009DB196
    while (k < host.path_vcall_0c_count(ptr_at(cursor, 0x4)) - 1) {
        float a[3] = {};
        host.path_vcall_08_point(ptr_at(cursor, 0x4), a, k);                      // 009DB1BD
        a[1] = 0.0f;
        ++k;
        float b[3] = {};
        host.path_vcall_08_point(ptr_at(cursor, 0x4), b, k);                      // 009DB1D8
        b[1] = 0.0f;
        host.line_rgba_floats_0085f990(a, b, kDebugYellow, kDebugYellow);         // 009DB288
    }
}

// 009E53B0 (009E53B0-009E5455). Unreferenced (image scan). 009E52E0's layout with arg1
// at +18h instead of +14h, and no 00694A60 registration: +4h..+0Ch = 0, +14h = 0, +10h = 1,
// vtable 00CF5C94, +1Ch = b, +18h = a; +78h = a != 0 ? the third argument's bits : -1.0
// [00D7A260] (MOVSS, a bit copy); +69h, +74h, +75h = 0; +88h, +8Ch = 0; +68h = 1; +70h,
// +7Ch = 0.0; +80h, +84h = -1000.0 [00D7A240]. The SEH frame covers no call.
void* ship_ai_obstacle_node_construct_unregistered_009e53b0(void* node, void* a, void* b,
                                                            std::uint32_t lifetime_bits) {
    at<std::uint32_t>(node, 0x4) = 0;
    at<std::uint32_t>(node, 0x8) = 0;
    at<std::uint32_t>(node, 0xC) = 0;
    at<std::uint32_t>(node, 0x14) = 0;
    at<std::uint8_t>(node, 0x10) = 1;
    at<std::uint32_t>(node, 0x0) = 0x00CF5C94u;                                   // 009E53DB
    at<std::uintptr_t>(node, 0x1C) = reinterpret_cast<std::uintptr_t>(b);
    at<std::uintptr_t>(node, 0x18) = reinterpret_cast<std::uintptr_t>(a);
    at<std::uint32_t>(node, 0x78) = a != nullptr ? lifetime_bits : 0xBF800000u;   // -1.0f
    at<std::uint8_t>(node, 0x69) = 0;
    at<std::uint32_t>(node, 0x88) = 0;
    at<std::uint8_t>(node, 0x75) = 0;
    at<std::uint32_t>(node, 0x8C) = 0;
    at<std::uint8_t>(node, 0x74) = 0;
    at<std::uint8_t>(node, 0x68) = 1;
    at<float>(node, 0x7C) = 0.0f;
    at<std::uint32_t>(node, 0x84) = 0xC47A0000u;                                  // -1000.0f
    at<std::uint32_t>(node, 0x80) = 0xC47A0000u;
    at<float>(node, 0x70) = 0.0f;
    return node;
}

// 009E8900 (009E8900-009E8BE1), the ship-AI debug slot at 00D219E8. label = " move"
// (00D21A34) when the state +1508h is this+8h, " kamikaze" (00D21A28) for this+14C0h,
// " submerged" (00D21A1C) for this+14CCh, " wandering" (00D21A10) for this+14E0h, else
// empty; then " (target lost)" (00D21A00) appended while byte [this+4h]+B28h is clear.
// 004C7070([00E188A8], float(x), float(y), "%s" (00CEB484), label or "" (00F8A718));
// orange (1.0, 0.5, 0.0, 1.0) arrow 0085FFF0 from the unit ([this+4h]+AA8h, pose
// refreshed, its +FCh) to the point [this+4h]+B2Ch; then state->vtable[18h](x, y), the
// state re-read. No direct or reachable 00BD2F10 draw; the state's slot is not chased.
// UNCERTAINTY: the native string calls (0041E870 assign, 0041DD40 resize, _memcpy,
// 00425E10 append, the 00419CC0/00BD1510 pool returns) are modelled by std::string; the
// pool traffic is not replayed through the host.
void ship_ai_debug_draw_state_label_009e8900(Cc13Seg67bHost& host, void* self,
                                             std::int32_t x, std::int32_t y) {
    static constexpr float kOrange[4] = {1.0f, 0.5f, 0.0f, 1.0f};
    std::string label;
    void* const state = ptr_at(self, 0x1508);
    auto* const base = static_cast<std::uint8_t*>(self);
    if (state == base + 0x8) {
        label = " move";
    } else if (state == base + 0x14C0) {
        label = " kamikaze";
    } else if (state == base + 0x14CC) {
        label = " submerged";
    } else if (state == base + 0x14E0) {
        label = " wandering";
    }
    void* const owner = ptr_at(self, 0x4);
    if (at<std::uint8_t>(owner, 0xB28) == 0) label += " (target lost)";          // 009E8A30
    const char* const text = label.empty() ? "" : label.c_str();                 // 009E8A9F
    host.diagnostic_printf_name_ints_004c7070(host.global_dword_00e188a8(),
                                              static_cast<float>(x), static_cast<float>(y),
                                              "%s", text, nullptr, 0);           // 009E8AD0
    const float to[3] = {at<float>(owner, 0xB2C), at<float>(owner, 0xB30),
                         at<float>(owner, 0xB34)};
    void* const unit = refreshed_entity(host, ptr_at(owner, 0xAA8));
    host.debug_arrow_0085fff0(static_cast<const float*>(ptr(unit, 0xFC)), to, kOrange);  // 009E8B94
    host.state_vcall_18_debug_text(ptr_at(self, 0x1508), x, y);                  // 009E8BAA
}

}  // namespace bsp

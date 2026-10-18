// cc13_seg67b: segment 67 bot-task and pilot-bot framework, second pass (packet
// cc13-seg67A). See include/bsp/cc13_seg67b.hpp and docs/CC13_SEG67B.md.
// Hypothesis names, build-tested only, nothing wired.

#include "bsp/cc13_seg67b.hpp"

#include <cmath>
#include <cstring>

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
// returns with RET 4): 0099A870(ECX = task+3F4h bot, task).
void bot_task_retire_0099b690(Cc13Seg67bHost& host, void* task) {
    host.pilot_bot_retire_task_0099a870(ptr_at(task, kTaskBot), task);
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

}  // namespace bsp

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
    host.dogfight_construct_approach_block_009a94e0(ptr(task, 0x3F8), bot, arg);
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

// 00999AE0 (00999AE0-00999B2B). Walks the bot's task vector (begin +58h, count
// +5Ch, both reloaded per step, 00999B07-00999B15); null entries are skipped. The
// first task whose vtable[34h]() is false (00999B01) answers by a tail JMP to its
// vtable[4Ch]() (00999B2B). With none: [00D7A260] (-1.0) (00999B17).
float pilot_bot_first_task_wait_value_00999ae0(Cc13Seg67bHost& host, void* bot) {
    for (std::uint32_t i = 0; i < at<std::uint32_t>(bot, 0x5C); ++i) {
        void* const task = reinterpret_cast<void*>(
            at<std::uintptr_t>(ptr_at(bot, 0x58), i * 4u));
        if (task == nullptr) continue;
        if (!host.object_vcall_34(task)) return host.object_vcall_4c_float(task);
    }
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
    host.depth_charge_construct_approach_block_from_stream_009a5000(ptr(task, 0x3F8), bot,
                                                                    archive);
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

}  // namespace bsp

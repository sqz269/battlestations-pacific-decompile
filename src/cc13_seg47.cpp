// Packet cc13-seg47 (cc13_seg47 marker). See include/bsp/cc13_seg47.hpp and
// docs/CC13_SEG47.md. Each function carries its address, original ABI and
// the simplifications it makes. Reconstructed and build-tested only.

#include "bsp/cc13_seg47.hpp"
#include "bsp/cc13_x87_model.hpp"

#include <cmath>
#include <cstring>
#include <initializer_list>

namespace bsp::cc13_seg47 {

namespace {

ArchiveKey name_key(const char* name) noexcept {
    return ArchiveKey{0, reinterpret_cast<std::uintptr_t>(name)};
}
ArchiveKey index_key(std::int32_t index) noexcept {
    return ArchiveKey{1, static_cast<std::uintptr_t>(static_cast<std::uint32_t>(index))};
}
ArchiveKey value_key(std::int32_t kind, void* target) noexcept {
    return ArchiveKey{kind, reinterpret_cast<std::uintptr_t>(target)};
}

// Image strings (read from the PE).
constexpr const char kType_00ce4ad0[] = "type";
constexpr const char kEntity_00ce5818[] = "entity";
constexpr const char kSimple_00d05344[] = "simple";
constexpr const char kPathIntf_00d05398[] = "pathIntf";
constexpr const char kIndex_00d05390[] = "index";
constexpr const char kMode_00d05388[] = "mode";
constexpr const char kForward_00d05380[] = "forward";
constexpr const char kNetPath_00d05660[] = "NetPath";
constexpr const char kPathPoints_00d05674[] = "PathPoints";
constexpr const char kPointFormat_00d05668[] = "Point%002i";
constexpr const char kPathPointType_00d056c0[] = "PathPointType";
constexpr const char kRotRefEntity_00d052c4[] = "RotRefEntity";
constexpr const char kRotRefPos_00d056b4[] = "RotRefPos";
constexpr const char kRotRefRot_00d056a8[] = "RotRefRot";
constexpr const char kSustainBefore_00d05698[] = "SustainBefore";
constexpr const char kSustainAfter_00d05688[] = "SustainAfter";
constexpr const char kSpeed_00ce69a4[] = "Speed";
constexpr const char kParty_00ce5804[] = "Party";

// A scene property's number: tag +4 == 0 holds an int (CVTSI2SS), anything
// else a float, both at +0Ch.
float property_number(Raw property) noexcept {
    if (property.at<std::int32_t>(4) == 0) return static_cast<float>(property.at<std::int32_t>(0xC));
    return property.at<float>(0xC);
}

// The refcounted release the path-source references inline: InterlockedDecrement
// on +4 and, at zero, vtable slot 0 with no stack argument.
void release_path_source(Seg47Host& host, Raw source) {
    if (host.interlocked_decrement_ce2220(reinterpret_cast<long*>(source.field(4))) == 0)
        host.path_source_vcall_destroy_00(source);
}

}  // namespace

// ============================================================================
// Path band 007B2DD0-007B3A9B
// ============================================================================

// 007B2DD0, slot +1Ch of vtable 00D05440, __thiscall(ECX = source; archive),
// RET 4. While archive->has({1, i}) reads point i ({5, &point}) and pushes it
// onto the float[3] vector at source+8 (004215D0).
void owned_points_path_source_load_007b2dd0(Seg47Host& host, Raw source, Raw archive) {
    for (std::int32_t i = 0;; ++i) {
        if (!host.archive_has_14(archive, index_key(i))) break;
        float point[3];
        host.archive_read_10(archive, index_key(i), value_key(5, point));
        host.vector12_push_back_004215d0(source.sub(8), point);
    }
}

// 007B2E40, __fastcall(ECX = out, EDX = arg), EAX = out, SEH frame.
// operator new(18h) (00BF681B); non-null -> *out = 007B2AB0(memory, arg),
// else *out = 0. Unreferenced in the image.
void** owned_points_path_source_new_007b2e40(Seg47Host& host, void** out, void* arg) {
    void* const memory = host.operator_new_00bf681b(0x18);
    if (memory != nullptr) {
        *out = host.owned_points_path_source_construct_007b2ab0(memory, arg);
        return out;
    }
    *out = nullptr;
    return out;
}

// 007B2EC0, __fastcall(ECX = out, EDX = archive), EAX = out, SEH frame.
// Reads the {0,"type"} string into a stack NativeString {+0 length, +4 data}.
// "entity" (_stricmp on non-null data) builds the 0Ch source: vptr 00CEB130,
// +4 = 1, vptr 00D0534C, +8 = 0. Otherwise "simple" (00425850) builds the 18h
// source through 007B2D80. Any other kind leaves the source null and the
// binary then calls through it (no guard; kept: the host call receives null).
// Then source->load(archive) (+1Ch), *out = source, and the string data is
// returned to the sized pool with length + 1.
void** path_source_create_from_archive_kind_007b2ec0(Seg47Host& host, void** out, Raw archive) {
    alignas(4) std::uint8_t string_storage[8] = {};
    Raw string{string_storage};
    host.archive_read_10(archive, name_key(kType_00ce4ad0), value_key(0, string_storage));
    std::uint8_t* source = nullptr;
    const char* const data = static_cast<const char*>(string.ptr(4));
    if (data != nullptr && host.stricmp_00bf7fbf(data, kEntity_00ce5818) == 0) {
        auto* const memory = static_cast<std::uint8_t*>(host.operator_new_00bf681b(0xC));
        if (memory != nullptr) {
            Raw m{memory};
            m.at<std::uint32_t>(0) = 0x00CEB130u;
            m.at<std::uint32_t>(4) = 1;
            m.at<std::uint32_t>(0) = 0x00D0534Cu;
            m.at<std::uint32_t>(8) = 0;
        }
        source = memory;
    } else if (host.native_string_equals_insensitive_00425850(string, kSimple_00d05344)) {
        void* const memory = host.operator_new_00bf681b(0x18);
        source = memory != nullptr
                     ? static_cast<std::uint8_t*>(host.owned_points_path_source_construct_empty_007b2d80(memory))
                     : nullptr;
    }
    host.path_source_vcall_load_1c(Raw{source}, archive);
    *out = source;
    if (string.ptr(4) != nullptr) {
        void* const pool = host.sized_pool_00419cc0();
        host.sized_pool_return_block_00bd1510(pool, string.ptr(4), string.at<std::uint32_t>(0) + 1, 1);
    }
    return out;
}

// 007B3000, __thiscall(ECX = holder; archive), RET 4, SEH frame.
// When the archive has {0,"pathIntf"}: enter it, build a source with
// 007B2EC0, assign it to the intrusive reference at holder+4 (increment the
// new, release the old when they differ), release the temporary, leave.
// Then reads "index" -> holder+8 (int), "mode" -> holder+0Ch (int, through a
// stack local), "forward" -> holder+10h (kind 3).
void path_source_ref_load_from_archive_007b3000(Seg47Host& host, Raw holder, Raw archive) {
    if (host.archive_has_14(archive, name_key(kPathIntf_00d05398))) {
        host.archive_enter_04(archive, name_key(kPathIntf_00d05398));
        void* temporary = nullptr;
        void* const created = *path_source_create_from_archive_kind_007b2ec0(host, &temporary, archive);
        void* const old = holder.ptr(4);
        if (old != created) {
            holder.ptr(4) = created;
            if (created != nullptr)
                host.interlocked_increment_ce221c(reinterpret_cast<long*>(static_cast<std::uint8_t*>(created) + 4));
            if (old != nullptr) release_path_source(host, raw(old));
        }
        if (temporary != nullptr) release_path_source(host, raw(temporary));
        host.archive_leave_08(archive);
    }
    host.archive_read_10(archive, name_key(kIndex_00d05390), value_key(1, holder.field(8)));
    std::int32_t mode = 0;  // uninitialised stack slot in the image
    host.archive_read_10(archive, name_key(kMode_00d05388), value_key(1, &mode));
    holder.at<std::int32_t>(0xC) = mode;
    host.archive_read_10(archive, name_key(kForward_00d05380), value_key(3, holder.field(0x10)));
}

// 007B3200 BSP_NetPath_Construct, __thiscall(ECX = entity; id), RET 4, EAX =
// entity. 0077EED0(entity, id); the embedded path component at +310h (vptr
// 00D053D8, +318h/+31Ch/+320h/+330h = 0, +324h = entity, +32Ch = 0.0f,
// +328h = 1); the six vtables; class id +C4h = 49h.
Raw net_path_construct_007b3200(Seg47Host& host, Raw entity, std::uint16_t id) {
    host.unit_owner_entity_construct_0077eed0(entity, id);
    entity.at<std::uint32_t>(0x318) = 0;
    entity.at<std::uint32_t>(0x31C) = 0;
    entity.at<std::uint32_t>(0x320) = 0;
    entity.at<std::uint32_t>(0x330) = 0;
    entity.at<std::uint32_t>(0x310) = 0x00D053D8u;
    entity.ptr(0x324) = entity.p;
    entity.at<float>(0x32C) = 0.0f;
    entity.at<std::uint8_t>(0x328) = 1;
    entity.at<std::uint32_t>(0x000) = 0x00D054D0u;
    entity.at<std::uint32_t>(0x010) = 0x00D054B4u;
    entity.at<std::uint32_t>(0x024) = 0x00D054ACu;
    entity.at<std::uint32_t>(0x170) = 0x00D054A8u;
    entity.at<std::uint32_t>(0x1E4) = 0x00D054A0u;
    entity.at<std::uint32_t>(0x310) = 0x00D0546Cu;
    entity.at<std::uint32_t>(0x0C4) = 0x49;
    return entity;
}

// 007B32B0, __thiscall, EAX = 00D05660 "NetPath".
const char* net_path_class_name_007b32b0() { return kNetPath_00d05660; }

// 007B32C0, __thiscall, EAX = 12h.
std::int32_t net_path_sub170_slot0_007b32c0() { return 0x12; }

// 007B32D0 / 007B32E0: RET 4, no body.
void net_path_slot180_noop_007b32d0(void*) {}
void net_path_slot18c_noop_007b32e0(void*) {}

// 007B3350, __thiscall(ECX = replicator): MOV AX, [[ECX+14h]+174h]. Only AX
// is written; EAX's high half keeps the entity pointer's (callers read AX).
std::uint16_t path_replicator_entity_net_id_007b3350(Raw replicator) {
    return replicator.deref(0x14).at<std::uint16_t>(0x174);
}

// 007B3370, __thiscall(ECX = entity): 00928560, then push_back onto the lists
// at [entity+30h]+30h and [entity+30h]+384h.
void net_path_register_in_world_lists_007b3370(Seg47Host& host, Raw entity) {
    host.entity_register_00928560(entity);
    host.unit_list_push_back_00484540(entity.deref(0x30).sub(0x30), entity.p);
    host.unit_list_push_back_00484540(entity.deref(0x30).sub(0x384), entity.p);
}

// 007B33A0, __thiscall(ECX = entity): 00928570, then for each of the two lists
// walk node = list+4, node = node+4 to the node whose +8 is the entity and
// erase it (004837D0); the first absent match falls through to the second.
void net_path_unregister_from_world_lists_007b33a0(Seg47Host& host, Raw entity) {
    host.entity_unregister_00928570(entity);
    for (std::size_t list_offset : {std::size_t{0x30}, std::size_t{0x384}}) {
        const Raw list = entity.deref(0x30).sub(list_offset);
        for (Raw node = list.deref(4); node; node = node.deref(4)) {
            if (node.ptr(8) == entity.p) {
                host.unit_list_erase_node_004837d0(list, node.p);
                break;
            }
        }
    }
}

// 007B3400 BSP_NetPath_Destruct, __thiscall(ECX = entity): re-stamps the six
// vtables (+310h gets 00D0546C then 00D053D8), 007B1A80(entity+310h), then
// tail-jumps to 0077E380(entity).
void net_path_destruct_007b3400(Seg47Host& host, Raw entity) {
    entity.at<std::uint32_t>(0x000) = 0x00D054D0u;
    entity.at<std::uint32_t>(0x010) = 0x00D054B4u;
    entity.at<std::uint32_t>(0x024) = 0x00D054ACu;
    entity.at<std::uint32_t>(0x170) = 0x00D054A8u;
    entity.at<std::uint32_t>(0x1E4) = 0x00D054A0u;
    entity.at<std::uint32_t>(0x310) = 0x00D0546Cu;
    entity.at<std::uint32_t>(0x310) = 0x00D053D8u;
    host.path_component_destruct_007b1a80(entity.sub(0x310));
    host.unit_owner_entity_destruct_0077e380(entity);
}

// 007B34B0, __thiscall(ECX = array; const float[3]*), RET 4. node =
// array->vtable[20h](); copies the point into it; push_back(array+4, &node).
void point3_ptr_array_add_point_007b34b0(Seg47Host& host, Raw array, const float* point) {
    float* const node = host.point_array_vcall_new_point_20(array);
    node[0] = point[0];
    node[1] = point[1];
    node[2] = point[2];
    void* const value = node;
    host.ptr_vector_push_back_007b3190(array.sub(4), &value);
}

// 007B3730, __thiscall(ECX = component; property holder), RET 4.
// 007B34F0(component, holder); table = find([holder+8], "PathPoints")+0Ch
// (the image also stores it to an unused local first). For i < size()
// (00415870, re-read each pass): the "Point%002i" sub-table, then with the
// checked-iterator test on component+8..+0Ch (00BF6713 on failure, then the
// read proceeds as in the image) record = vector[i]:
//   +20h PathPointType (int +0Ch), +24h the RotRefEntity entity (00925A90 on
//   [[00E188A8]+19CCh] with the string value), +28h..+30h RotRefPos,
//   +34h/+38h RotRefRot, +3Ch SustainBefore, +40h SustainAfter, +44h Speed
//   (int or float by the tag).
void camera_path_load_path_points_007b3730(Seg47Host& host, Raw component, void* holder) {
    host.point_array_load_from_scene_007b34f0(component, holder);
    void* const table = host.property_bag_find_008f2260(raw(holder).ptr(8), kPathPoints_00d05674).ptr(0xC);
    std::int32_t i = 0;
    if (host.vector_size_00415870(component) <= 0) return;
    do {
        char name[0x80];
        host.format_int_004f9a80(name, kPointFormat_00d05668, i);
        void* const point_bag = host.property_bag_find_008f2260(table, name).ptr(0xC);
        auto* const begin = static_cast<std::uint8_t*>(component.ptr(8));
        if (begin == nullptr ||
            static_cast<std::uint32_t>(i) >=
                static_cast<std::uint32_t>((static_cast<std::uint8_t*>(component.ptr(0xC)) - begin) >> 2)) {
            host.invalid_parameter_00bf6713();
        }
        const Raw record = raw(reinterpret_cast<void**>(component.ptr(8))[i]);
        record.at<std::int32_t>(0x20) =
            host.property_bag_find_008f2260(point_bag, kPathPointType_00d056c0).at<std::int32_t>(0xC);
        const char* const entity_name =
            host.property_string_value_008f0df0(host.property_bag_find_008f2260(point_bag, kRotRefEntity_00d052c4));
        record.ptr(0x24) = host.entity_registry_find_by_name_00925a90(
            raw(host.global_pointer(0x00E188A8u)).ptr(0x19CC), entity_name);
        const Raw pos = host.property_bag_find_008f2260(point_bag, kRotRefPos_00d056b4);
        record.at<float>(0x28) = pos.at<float>(0xC);
        record.at<float>(0x2C) = pos.at<float>(0x10);
        record.at<float>(0x30) = pos.at<float>(0x14);
        const Raw rot = host.property_bag_find_008f2260(point_bag, kRotRefRot_00d056a8);
        record.at<float>(0x34) = rot.at<float>(0xC);
        record.at<float>(0x38) = rot.at<float>(0x10);
        record.at<float>(0x3C) = property_number(host.property_bag_find_008f2260(point_bag, kSustainBefore_00d05698));
        record.at<float>(0x40) = property_number(host.property_bag_find_008f2260(point_bag, kSustainAfter_00d05688));
        record.at<float>(0x44) = property_number(host.property_bag_find_008f2260(point_bag, kSpeed_00ce69a4));
        ++i;
    } while (i < host.vector_size_00415870(component));
}

// 007B38D0, __thiscall(ECX = entity): 009277E0(entity); when the holder
// [entity+C0h] is set and its +4 == 1, entity+54h = find([holder+8],
// "Party")+0Ch; then 007B34F0(entity+1E4h, [entity+C0h]).
void scene_path_entity_load_holder_properties_007b38d0(Seg47Host& host, Raw entity) {
    host.scene_entity_activate_009277e0(entity);
    const Raw holder = entity.deref(0xC0);
    if (holder && holder.at<std::int32_t>(4) == 1)
        entity.at<std::uint32_t>(0x54) = host.property_bag_find_008f2260(holder.ptr(8), kParty_00ce5804).at<std::uint32_t>(0xC);
    host.point_array_load_from_scene_007b34f0(entity.sub(0x1E4), entity.ptr(0xC0));
}

// 007B3920, __thiscall(ECX = entity): 00923840(entity); 007B34F0(entity+170h, [entity+C0h]).
void entity_path_activate_load_points_007b3920(Seg47Host& host, Raw entity) {
    host.scene_entity_activate_00923840(entity);
    host.point_array_load_from_scene_007b34f0(entity.sub(0x170), entity.ptr(0xC0));
}

// 007B3940, __thiscall(ECX = entity): 0077F0E0(entity); 007B34F0(entity+310h, [entity+C0h]).
void net_path_activate_load_points_007b3940(Seg47Host& host, Raw entity) {
    host.net_entity_activate_0077f0e0(entity);
    host.point_array_load_from_scene_007b34f0(entity.sub(0x310), entity.ptr(0xC0));
}

// 007B3960, __thiscall(ECX = entity; peer), RET 4, SEH frame, 120h-byte frame.
// msg = 007B2950(stack message, entity); 00779FC0(entity, peer, msg); then the
// inline destructor: vptr 00D0377C, operator delete(+114h) when set, +114h..
// +11Ch zeroed, 007668E0(msg).
void net_path_send_create_message_007b3960(Seg47Host& host, Raw entity, void* peer) {
    alignas(4) std::uint8_t message[0x120];
    void* const msg = host.path_create_message_construct_007b2950(message, entity);
    host.entity_send_message_to_peer_00779fc0(entity, peer, msg);
    Raw m{message};
    m.at<std::uint32_t>(0) = 0x00D0377Cu;
    if (m.ptr(0x114) != nullptr) host.operator_delete_00bf65ac(m.ptr(0x114));
    m.at<std::uint32_t>(0x114) = 0;
    m.at<std::uint32_t>(0x118) = 0;
    m.at<std::uint32_t>(0x11C) = 0;
    host.net_entity_create_message_destruct_007668e0(message);
}

// 007B3A00, __thiscall(ECX = message), EAX, SEH frame. operator new(33Ch)
// (00BF55BE), memset(0, 33Ch) before the null test (as in the image), then
// 007B3200(memory, (uint16)[message+1Ch]); null -> 0.
void* path_create_message_create_entity_007b3a00(Seg47Host& host, Raw message) {
    void* const memory = host.operator_new_00bf55be(0x33C);
    host.memset_00bf79f0(memory, 0, 0x33C);
    if (memory == nullptr) return nullptr;
    return net_path_construct_007b3200(host, raw(memory), message.at<std::uint16_t>(0x1C)).p;
}

// 007B3A80, __thiscall(ECX = entity): 009277E0(entity); 007B3730(entity+1E4h, [entity+C0h]).
void camera_path_activate_007b3a80(Seg47Host& host, Raw entity) {
    host.scene_entity_activate_009277e0(entity);
    camera_path_load_path_points_007b3730(host, entity.sub(0x1E4), entity.ptr(0xC0));
}

// ============================================================================
// Bot task / rocket band 007B3BA0-007B7FD0
// ============================================================================

namespace {
constexpr float kOne_00d7a24c = 1.0f;
constexpr float kRunLo_00ce3860 = 0.9f;          // 3F666666
constexpr float kGoAwayHi_00ce780c = 1.05f;      // 3F866666
constexpr double kTuning664Scale_00ce3d40 = 0.800000011920929;  // double 3FE99999A0000000
constexpr float kFireHi_00ce6448 = 1.1f;         // 3F8CCCCD
constexpr float kRunSpeedLo_00ce74f8 = 0.8f;     // 3F4CCCCD
constexpr float kApproach44_00cfdeb0 = 3600.0f;  // 45610000
constexpr float kFollowArg_00ce3d08 = 100.0f;    // 42C80000
constexpr float kAimWindow_00ced560 = 1400.0f;   // 44AF0000
constexpr const char kNotDefined_00d05770[] = "not defined";
constexpr const char kRocket_00cfa6b4[] = "Rocket";
constexpr const char kMoveTo_00d058f4[] = "moveto (Rocket)";
constexpr const char kFollow_00d058e4[] = "follow (Rocket)";
constexpr const char kPrepare_00d058d4[] = "Rocket/prepare";
constexpr const char kGoTowards_00d058c0[] = "Rocket/gotowards";
constexpr const char kAim_00d058b4[] = "Rocket/aim";
constexpr const char kGoAway_00d058a4[] = "Rocket/goaway";
constexpr const char kAttackRun_00d05890[] = "Rocket/attackrun";

// The MSVC 2005 std::string release the inline destructors make: capacity
// (+18h) >= 10h -> operator delete(+4).
void string_release_inline(Seg47Host& host, Raw string) {
    if (string.at<std::uint32_t>(0x18) >= 0x10) host.operator_delete_00bf65ac(string.ptr(4));
}
}  // namespace

// 007B3BA0, __thiscall(ECX = element): vptr = 00D056E8 (the base destructor body).
void pilot_plan_element_destruct_007b3ba0(Raw element) { element.at<std::uint32_t>(0) = 0x00D056E8u; }

// 007B3CB0, __thiscall(ECX = task), EAX: 007B8AD0([task+3FCh]) ? task+4E4h : task+520h.
Raw bot_rocket_task_state_block_for_role_007b3cb0(Seg47Host& host, Raw task) {
    return host.plane_query_007b8ad0(task.deref(0x3FC)) ? task.sub(0x4E4) : task.sub(0x520);
}

// 007B3CD0, __thiscall(ECX = holder), SEH frame: vptr 00D056EC; when [holder+14h]
// is set 006952A0(ECX = [holder+14h], EDX = holder); then 00695870(holder).
void bot_observer_holder_destruct_007b3cd0(Seg47Host& host, Raw holder) {
    holder.at<std::uint32_t>(0) = 0x00D056ECu;
    if (holder.ptr(0x14) != nullptr) host.observer_unregister_pair_006952a0(holder.ptr(0x14), holder);
    host.callback_owner_destroy_00695870(holder);
}

// 007B3D60, __thiscall(ECX = timer; float lo, float hi), RET 8, EAX = timer.
// +0 = lo, +4 = hi, +8 = U(0, hi) on stream 1 (00BD2F10 at 007B3D8D).
// Unreferenced in the image.
Raw random_timer_construct_007b3d60(Seg47Host& host, Raw timer, float lo, float hi) {
    timer.at<float>(0) = lo;
    timer.at<float>(4) = hi;
    timer.at<float>(8) = host.random_uniform_00bd2f10(1, 0.0f, hi);
    return timer;
}

// 007B3DC0 RET; 007B3DD0 RET 4; 007B3DE0 MOV AL,1; 007B3DF0 RET.
void bot_task_state_base_exit_007b3dc0() {}
void empty_one_arg_virtual_007b3dd0(void*) {}
bool bot_task_state_base_predicate10_007b3de0() { return true; }
void bot_task_state_base_slot14_007b3df0() {}

// 007B3E20, __thiscall(ECX = holder): vptr 00D05704; when [holder+4] is set,
// tail-jumps to its vtable +08h.
void owned_object_holder_destruct_007b3e20(Seg47Host& host, Raw holder) {
    holder.at<std::uint32_t>(0) = 0x00D05704u;
    if (holder.ptr(4) != nullptr) host.owned_object_vcall_release_08(holder.deref(4));
}

// 007B3F70, __thiscall(ECX = holder), SEH frame: 007B3CD0(holder+44h), then
// 004B7EF0(holder+24h).
void bot_observer_holder_destruct_full_007b3f70(Seg47Host& host, Raw holder) {
    bot_observer_holder_destruct_007b3cd0(host, holder.sub(0x44));
    host.observer_holder_destruct_004b7ef0(holder.sub(0x24));
}

// 007B40C0 XOR EAX; 007B40E0 "not defined"; 007B40F0 MOV AL,1; 007B4100 XOR AL;
// 007B4120 XOR AL; 007B4130 XOR EAX; 007B4140 EAX = 1; 007B4150 XOR EAX;
// 007B4160 RET 4. All __thiscall, ECX unused.
std::int32_t bot_task_base_slot08_007b40c0() { return 0; }
const char* bot_task_base_name_007b40e0() { return kNotDefined_00d05770; }
bool bot_task_base_slot20_007b40f0() { return true; }
bool bot_task_base_slot28_007b4100() { return false; }
bool bot_task_base_slot44_007b4120() { return false; }
std::int32_t bot_task_base_slot48_007b4130() { return 0; }
std::int32_t bot_task_base_step_result_007b4140() { return 1; }
std::int32_t bot_task_base_slot5c_007b4150() { return 0; }
void bot_task_base_slot64_007b4160(void*) {}

// 007B41E0, __thiscall(ECX = approach; arg, target), RET 8, EAX = approach, SEH frame.
//   009F9CE0(approach, arg, tuning+668h); vptr 00D0577C;
//   a = U(0.9, 1.05) * [approach+14h]+FCh             (007B4240, stream 1)
//   b = (float)(tuning+664h * 0.800000011920929)       (double constant)
//   +34h = (a > b ? b : a) * +24h       (FCOMI/JBE: a NaN `a` keeps a)
//   size = 007B3C00(target); v = +24h; r = U(0.9, 1.1) (007B42CB, stream 1)
//   +38h = (r * (double)[[approach+14h] + size*4 + 114h]) * v
//   +44h = 3600.0f; +4Ch = 0; +3Ch = v * [approach+14h]+F8h
//   009FB200(approach+58h, target); +CCh = 1.0f; +D0h = -U(0, 1); +D4h = 0; 007B3EA0(approach).
// The tuning object (0042E740) is fetched twice, as in the image. The +38h
// product is formed in x87 extended in the image and in double here
// (r * size is exact in a double; the multiply by v may round differently in
// the last bit before the float store).
Raw rocket_approach_construct_base_007b41e0(Seg47Host& host, Raw approach, void* arg, void* target) {
    host.approach_head_construct_009f9ce0(approach, arg, host.ai_tuning_0042e740().at<float>(0x668));
    approach.at<std::uint32_t>(0) = 0x00D0577Cu;
    const float a = host.random_uniform_00bd2f10(1, kRunLo_00ce3860, kGoAwayHi_00ce780c) *
                    approach.deref(0x14).at<float>(0xFC);
    const float b = static_cast<float>(host.ai_tuning_0042e740().at<float>(0x664) * kTuning664Scale_00ce3d40);
    const float chosen = (a > b) ? b : a;
    approach.at<float>(0x34) = chosen * approach.at<float>(0x24);
    const std::int32_t size = host.rocket_target_size_007b3c00(target);
    const double size_scale = approach.deref(0x14).at<float>(0x114 + 4 * static_cast<std::size_t>(size));
    const float v = approach.at<float>(0x24);
    const float r = host.random_uniform_00bd2f10(1, kRunLo_00ce3860, kFireHi_00ce6448);
    approach.at<float>(0x38) = static_cast<float>((static_cast<double>(r) * size_scale) * v);
    approach.at<float>(0x44) = kApproach44_00cfdeb0;
    approach.at<std::uint8_t>(0x4C) = 0;
    approach.at<float>(0x3C) = v * approach.deref(0x14).at<float>(0xF8);
    host.approach_target_ref_construct_009fb200(approach.sub(0x58), target);
    approach.at<float>(0xCC) = kOne_00d7a24c;
    approach.at<float>(0xD0) = -host.random_uniform_00bd2f10(1, 0.0f, 1.0f);
    approach.at<std::uint32_t>(0xD4) = 0;
    host.rocket_glide_seed_007b3ea0(approach);
    return approach;
}

// 007B45A0, __thiscall(ECX = vector): 007B4500(vector, 0), then _free([vector]).
void land_state_observer_vector_destruct_007b45a0(Seg47Host& host, Raw vector) {
    host.observer_vector_tidy_007b4500(vector, 0);
    host.free_00bf6989(vector.ptr(0));
}

// 007B45E0: XOR AL,AL; RET.
bool bot_task_state_base_predicate18_007b45e0() { return false; }

// 007B4890, __thiscall(ECX = state): +48h = U(0.8, 1.1) * [[state+8]+18Ch]
// (00BD2F10 at 007B48B2, stream 1). Unreferenced in the image.
void rocket_approach_redraw_run_speed_007b4890(Seg47Host& host, Raw state) {
    const Raw owner = state.deref(8);
    state.at<float>(0x48) = host.random_uniform_00bd2f10(1, kRunSpeedLo_00ce74f8, kFireHi_00ce6448) *
                            owner.at<float>(0x18C);
}

// 007B4D90, __thiscall(ECX = state; owner), RET 4, EAX = state, SEH frame.
// +4 = owner, +0Ch/+10h/+14h/+8 = 0, vptr 00D057E8, +18h = 1.0f,
// +1Ch = -U(0, 1) (stream 1, 007B4DEA), +20h = 0.0f.
Raw rocket_attack_run_state_construct_007b4d90(Seg47Host& host, Raw state, void* owner) {
    state.ptr(4) = owner;
    state.at<std::uint32_t>(0xC) = 0;
    state.at<std::uint32_t>(0x10) = 0;
    state.at<std::uint32_t>(0x14) = 0;
    state.at<std::uint32_t>(8) = 0;
    state.at<std::uint32_t>(0) = 0x00D057E8u;
    state.at<float>(0x18) = kOne_00d7a24c;
    state.at<float>(0x1C) = -host.random_uniform_00bd2f10(1, 0.0f, 1.0f);
    state.at<float>(0x20) = 0.0f;
    return state;
}

// 007B5400, __thiscall(ECX = approach; float* out), RET 4, EAX = out: copies +80h..+88h.
float* rocket_approach_body_aim_offset_007b5400(Raw approach, float* out) {
    out[0] = approach.at<float>(0x80);
    out[1] = approach.at<float>(0x84);
    out[2] = approach.at<float>(0x88);
    return out;
}

// 007B5980, __thiscall(ECX = timer; float dt), RET 4, AL. t = +8 - dt (float),
// stored; when 0 > t (FCOMI/JBE: a NaN t does not fire) +8 = U(+0, +4)
// (stream 1, 007B59B4) + t and AL = 1, else AL = 0. Unreferenced in the image.
bool random_timer_step_007b5980(Seg47Host& host, Raw timer, float dt) {
    const float t = timer.at<float>(8) - dt;
    timer.at<float>(8) = t;
    if (0.0f > t) {
        const float draw = host.random_uniform_00bd2f10(1, timer.at<float>(0), timer.at<float>(4));
        timer.at<float>(8) = draw + timer.at<float>(8);
        return true;
    }
    return false;
}

// 007B5BE0, __thiscall(ECX = unit), ST0. m = (+448h > +444h) ? +448h : +444h
// (FCOMI/JA: a NaN pair takes +444h); result = (m > +434h) ? m : +434h
// (JBE: a NaN takes +434h).
float unit_largest_extent_007b5be0(Raw unit) {
    const float a = unit.at<float>(0x444);
    const float b = unit.at<float>(0x448);
    const float m = (b > a) ? b : a;
    const float c = unit.at<float>(0x434);
    return (m > c) ? m : c;
}

// 007B5C20 / 007B5D70 / 007B5E60 (three identical bodies), __thiscall(ECX =
// condition), SEH frame: 0042BCA0(condition+4), then vptr 00D05840.
void bot_condition_destruct_base_007b5c20(Seg47Host& host, Raw condition) {
    host.handle_destruct_0042bca0(condition.sub(4));
    condition.at<std::uint32_t>(0) = 0x00D05840u;
}
void bot_condition_destruct_base_007b5d70(Seg47Host& host, Raw condition) {
    host.handle_destruct_0042bca0(condition.sub(4));
    condition.at<std::uint32_t>(0) = 0x00D05840u;
}
void bot_condition_destruct_base_007b5e60(Seg47Host& host, Raw condition) {
    host.handle_destruct_0042bca0(condition.sub(4));
    condition.at<std::uint32_t>(0) = 0x00D05840u;
}

// 007B5C70, __thiscall(ECX = condition; value), RET 4, EAX = condition, SEH frame.
// vptr 00D05860; 004BEF00(condition+4, value); then +4 = 00CE3CF0.
Raw bot_condition_construct_base_007b5c70(Seg47Host& host, Raw condition, void* value) {
    condition.at<std::uint32_t>(0) = 0x00D05860u;
    host.handle_construct_004bef00(condition.sub(4), value);
    condition.at<std::uint32_t>(4) = 0x00CE3CF0u;
    return condition;
}

// 007B5CD0 / 007B5DC0 / 007B5EB0 (identical), __thiscall(ECX = condition;
// flags), RET 4, EAX = condition: the 007B5C20 body, then operator delete
// (00BF65AC) when flags & 1.
namespace {
Raw condition_deleting_dtor(Seg47Host& host, Raw condition, std::uint8_t flags) {
    host.handle_destruct_0042bca0(condition.sub(4));
    condition.at<std::uint32_t>(0) = 0x00D05840u;
    if (flags & 1) host.operator_delete_00bf65ac(condition.p);
    return condition;
}
}  // namespace
Raw bot_condition_deleting_dtor_007b5cd0(Seg47Host& host, Raw condition, std::uint8_t flags) {
    return condition_deleting_dtor(host, condition, flags);
}
Raw bot_condition_point_off_nose_deleting_dtor_007b5dc0(Seg47Host& host, Raw condition, std::uint8_t flags) {
    return condition_deleting_dtor(host, condition, flags);
}
Raw bot_condition_point_distance_deleting_dtor_007b5eb0(Seg47Host& host, Raw condition, std::uint8_t flags) {
    return condition_deleting_dtor(host, condition, flags);
}

// 007B5D30, __thiscall(ECX = condition; value, float a, b, c), RET 10h, EAX.
// 007B5C70(condition, value); vptr 00D05870; +1Ch/+20h/+24h = a, b, c.
Raw bot_condition_point_off_nose_construct_007b5d30(Seg47Host& host, Raw condition, void* value, float a,
                                                    float b, float c) {
    bot_condition_construct_base_007b5c70(host, condition, value);
    condition.at<std::uint32_t>(0) = 0x00D05870u;
    condition.at<float>(0x1C) = a;
    condition.at<float>(0x20) = b;
    condition.at<float>(0x24) = c;
    return condition;
}

// 007B5E20, __thiscall(ECX = condition; value, const float[3]*, float d), RET 0Ch, EAX.
// 007B5C70(condition, value); vptr 00D05880; +1Ch..+24h = point; +28h = d * d
// (one x87 product rounded to float; exact in double first).
Raw bot_condition_point_distance_construct_007b5e20(Seg47Host& host, Raw condition, void* value,
                                                   const float* point, float distance) {
    bot_condition_construct_base_007b5c70(host, condition, value);
    condition.at<std::uint32_t>(0) = 0x00D05880u;
    condition.at<float>(0x1C) = point[0];
    condition.at<float>(0x20) = point[1];
    condition.at<float>(0x24) = point[2];
    condition.at<float>(0x28) = static_cast<float>(static_cast<double>(distance) * distance);
    return condition;
}

// 007B6680, __thiscall(ECX = approach), SEH frame: the member destructors in
// reverse construction order: 007B45F0 on +2ECh, +29Ch, +27Ch, +258h;
// 006CDD70(+1C0h+18h), 007B45F0(+1C0h); 006CDD70(+128h+18h), 007B45F0(+128h);
// 007B65E0(+ECh); 004116D0(approach ? approach+D8h : null); 00695870(+58h).
void rocket_approach_destroy_states_007b6680(Seg47Host& host, Raw approach) {
    host.bot_state_destruct_007b45f0(approach.sub(0x2EC));
    host.bot_state_destruct_007b45f0(approach.sub(0x29C));
    host.bot_state_destruct_007b45f0(approach.sub(0x27C));
    host.bot_state_destruct_007b45f0(approach.sub(0x258));
    host.state_member_destruct_006cdd70(approach.sub(0x1C0 + 0x18));
    host.bot_state_destruct_007b45f0(approach.sub(0x1C0));
    host.state_member_destruct_006cdd70(approach.sub(0x128 + 0x18));
    host.bot_state_destruct_007b45f0(approach.sub(0x128));
    host.state_destruct_007b65e0(approach.sub(0xEC));
    host.state_machine_destruct_004116d0(approach ? approach.field(0xD8) : nullptr);
    host.callback_owner_destroy_00695870(approach.sub(0x58));
}

// 007B6830, __thiscall(ECX = approach; task, target), RET 8, EAX = approach, SEH frame.
//   007B41E0(approach, [task+50h], target); 00411E20(approach+D8h);
//   vptr 00D05908, +D8h vptr 00D05904;
//   009C2AC0(approach+ECh; approach, target, +40h, +40h, +38h);
//   009C2980(approach+128h; approach, 100.0f); 009C2980(approach+1C0h; approach, 100.0f);
//   +258h inline: +4 = approach, +0Ch/+10h/+14h/+8 = 0, vptr 00D057B8,
//     +18h = +1Ch = 1400.0f, +20h = 0;
//   +27Ch inline: +4 = approach, +0Ch/+10h/+14h/+8 = 0, vptr 00D05808, +18h = 0.0f;
//   007B5780(approach+29Ch; approach); 007B4D90(approach+2ECh; approach);
//   seven 00411E70 registrations on +D8h, in the image's order and names.
Raw rocket_approach_construct_007b6830(Seg47Host& host, Raw approach, Raw task, void* target) {
    rocket_approach_construct_base_007b41e0(host, approach, task.ptr(0x50), target);
    const Raw machine = approach.sub(0xD8);
    host.state_machine_construct_00411e20(machine);
    const float f38 = approach.at<float>(0x38);
    const float f40 = approach.at<float>(0x40);
    approach.at<std::uint32_t>(0) = 0x00D05908u;
    machine.at<std::uint32_t>(0) = 0x00D05904u;
    host.moveto_state_construct_009c2ac0(approach.sub(0xEC), approach, target, f40, f40, f38);
    host.follow_state_construct_009c2980(approach.sub(0x128), approach, kFollowArg_00ce3d08);
    host.follow_state_construct_009c2980(approach.sub(0x1C0), approach, kFollowArg_00ce3d08);
    const Raw gotowards = approach.sub(0x258);
    gotowards.ptr(4) = approach.p;
    gotowards.at<std::uint32_t>(0xC) = 0;
    gotowards.at<std::uint32_t>(0x10) = 0;
    gotowards.at<std::uint32_t>(0x14) = 0;
    gotowards.at<std::uint32_t>(8) = 0;
    gotowards.at<std::uint32_t>(0) = 0x00D057B8u;
    gotowards.at<float>(0x18) = kAimWindow_00ced560;
    gotowards.at<float>(0x1C) = kAimWindow_00ced560;
    gotowards.at<std::uint8_t>(0x20) = 0;
    const Raw aim = approach.sub(0x27C);
    aim.ptr(4) = approach.p;
    aim.at<std::uint32_t>(0xC) = 0;
    aim.at<std::uint32_t>(0x10) = 0;
    aim.at<std::uint32_t>(0x14) = 0;
    aim.at<std::uint32_t>(8) = 0;
    aim.at<std::uint32_t>(0) = 0x00D05808u;
    aim.at<float>(0x18) = 0.0f;
    host.rocket_goaway_construct_007b5780(approach.sub(0x29C), approach);
    rocket_attack_run_state_construct_007b4d90(host, approach.sub(0x2EC), approach.p);
    host.state_machine_register_00411e70(machine, kMoveTo_00d058f4, approach.sub(0xEC));
    host.state_machine_register_00411e70(machine, kFollow_00d058e4, approach.sub(0x128));
    host.state_machine_register_00411e70(machine, kPrepare_00d058d4, approach.sub(0x1C0));
    host.state_machine_register_00411e70(machine, kGoTowards_00d058c0, gotowards);
    host.state_machine_register_00411e70(machine, kAim_00d058b4, aim);
    host.state_machine_register_00411e70(machine, kGoAway_00d058a4, approach.sub(0x29C));
    host.state_machine_register_00411e70(machine, kAttackRun_00d05890, approach.sub(0x2EC));
    return approach;
}

// 007B6CD0, EAX = 00E08F48 (the rocket command class); 007B6CF0, EAX = 00CFA6B4 "Rocket".
std::uint32_t bot_task_rocket_command_class_007b6cd0() { return 0x00E08F48u; }
const char* bot_task_rocket_name_007b6cf0() { return kRocket_00cfa6b4; }

// 007B6EB0, __thiscall(ECX = holder; state), RET 4: exit the current state
// (+4, vtable +08h) when set, store the new one and enter it (vtable +04h,
// no null test and no same-state test).
void ref_holder_assign_007b6eb0(Seg47Host& host, Raw holder, Raw state) {
    if (holder.ptr(4) != nullptr) host.state_vcall_exit_08(holder.deref(4));
    holder.ptr(4) = state.p;
    host.state_vcall_enter_04(state);
}

// 007B6EE0, __thiscall(ECX = machine; state), RET 4: when state != +4: exit the
// current one (when set), store, enter the new one (no null test).
void state_machine_set_current_state_007b6ee0(Seg47Host& host, Raw machine, Raw state) {
    if (state.p == machine.ptr(4)) return;
    if (machine.ptr(4) != nullptr) host.state_vcall_exit_08(machine.deref(4));
    machine.ptr(4) = state.p;
    host.state_vcall_enter_04(state);
}

// 007B6F50, __thiscall(ECX = task; state), RET 4: the 007B6EE0 shape on task+310h.
void bot_task_rocket_set_state_007b6f50(Seg47Host& host, Raw task, Raw state) {
    if (state.p == task.ptr(0x310)) return;
    if (task.ptr(0x310) != nullptr) host.state_vcall_exit_08(task.deref(0x310));
    task.ptr(0x310) = state.p;
    host.state_vcall_enter_04(state);
}

// 007B7BA0, __thiscall(ECX = task): exit the current state (+310h, vtable +08h;
// +310h is not cleared); copy the global vector [00F87574..7Ch]; when +3C4h is
// set, 006952A0(ECX = +3C4h, EDX = task+3B0h) and +3C4h = 0; +3A4h..+3ACh =
// the copied vector; +3C8h = 0.
void bot_task_rocket_stop_007b7ba0(Seg47Host& host, Raw task) {
    if (task.ptr(0x310) != nullptr) host.state_vcall_exit_08(task.deref(0x310));
    const auto* const zero = static_cast<const float*>(host.global_address(0x00F87574u));
    const float v0 = zero[0], v1 = zero[1], v2 = zero[2];
    if (task.ptr(0x3C4) != nullptr) {
        host.observer_unregister_pair_006952a0(task.ptr(0x3C4), task.sub(0x3B0));
        task.at<std::uint32_t>(0x3C4) = 0;
    }
    task.at<float>(0x3A4) = v0;
    task.at<float>(0x3A8) = v1;
    task.at<float>(0x3AC) = v2;
    task.at<std::uint8_t>(0x3C8) = 0;
}

// 007B7E10, __thiscall(ECX = task), SEH frame: 007B6680(task ? task+3F8h : null),
// then 007B4030(task).
void bot_task_rocket_destruct_007b7e10(Seg47Host& host, Raw task) {
    rocket_approach_destroy_states_007b6680(host, task ? task.sub(0x3F8) : Raw{nullptr});
    host.bot_task_base_destruct_007b4030(task);
}

// 007B7E70, __cdecl(string* out, const string& base, const char* suffix), EAX = out.
// A stack string (capacity 0Fh, size 0, buffer[0] = 0) = base (00408120 with
// 0, npos); append(suffix, strlen) (007B7CC0, the inline strlen loop); out is
// initialised the same way and assigned from it; the stack string is released
// (capacity >= 10h -> operator delete).
Raw bot_task_build_debug_string_007b7e70(Seg47Host& host, Raw out, Raw base, const char* suffix) {
    alignas(4) std::uint8_t storage[0x1C] = {};
    Raw temp{storage};
    temp.at<std::uint32_t>(0x18) = 0xF;
    temp.at<std::uint32_t>(0x14) = 0;
    temp.at<std::uint8_t>(4) = 0;
    host.string_assign_00408120(temp, base, 0, 0xFFFFFFFFu);
    const Raw appended = host.string_append_007b7cc0(temp, suffix, static_cast<std::uint32_t>(std::strlen(suffix)));
    out.at<std::uint32_t>(0x18) = 0xF;
    out.at<std::uint32_t>(0x14) = 0;
    out.at<std::uint8_t>(4) = 0;
    host.string_assign_00408120(out, appended, 0, 0xFFFFFFFFu);
    string_release_inline(host, temp);
    return out;
}

// 007B7F20, __thiscall(ECX = task; string* out), RET 4, EAX = out, SEH frame.
// 007B7E70(out, 009A1A90(task, &temp), 00411430(task+4D0h, [task+310h])),
// then the temporary is released.
Raw bot_task_rocket_debug_name_007b7f20(Seg47Host& host, Raw task, Raw out) {
    alignas(4) std::uint8_t storage[0x1C];
    Raw temp{storage};
    const Raw base = host.bot_task_debug_name_009a1a90(task, temp);
    const char* const state_name = host.state_machine_state_name_00411430(task.sub(0x4D0), task.ptr(0x310));
    bot_task_build_debug_string_007b7e70(host, out, base, state_name);
    string_release_inline(host, temp);
    return out;
}

// 007B7FD0, __fastcall(ECX = a, EDX = b), EAX, SEH frame: operator new(708h)
// (00BF681B); non-null -> 007B6A40(memory, a, b), else 0.
void* bot_task_make_rocket_007b7fd0(Seg47Host& host, void* a, void* b) {
    void* const memory = host.operator_new_00bf681b(0x708);
    if (memory == nullptr) return nullptr;
    return host.rocket_task_construct_007b6a40(memory, a, b);
}

// ============================================================================
// Plane state and weapon-controller band 007B80A0-007B9510
// ============================================================================

// 007B80A0, __thiscall(ECX = target), EAX: 1 when +E4h is set or [00F874FD] is set.
std::int32_t target_flag_e4_set_or_global_007b80a0(Seg47Host& host, Raw target) {
    if (target.at<std::uint8_t>(0xE4) != 0) return 1;
    if (*static_cast<const std::uint8_t*>(host.global_address(0x00F874FDu)) != 0) return 1;
    return 0;
}

// 007B80C0, __thiscall(ECX = target), EAX: 1 when +E4h is clear or [00F874FD] is set.
std::int32_t target_flag_e4_clear_or_global_007b80c0(Seg47Host& host, Raw target) {
    if (target.at<std::uint8_t>(0xE4) == 0) return 1;
    if (*static_cast<const std::uint8_t*>(host.global_address(0x00F874FDu)) != 0) return 1;
    return 0;
}

// 007B8140, __thiscall(ECX = window; index, count, float sample), RET 0Ch.
// window+0Ch+index*4 = sample; sum = the float running sum (each FADD stored
// to a float) of the count samples from +0Ch; +8 = sum / count (FIDIV: an
// x87 extended quotient rounded once to float; double here, which can differ
// only by a double rounding). count <= 0 gives 0/0 (NaN), as in the image.
void float_window_set_sample_and_sum_007b8140(Raw window, std::int32_t index, std::int32_t count, float sample) {
    window.at<float>(0xC + 4 * static_cast<std::size_t>(index)) = sample;
    float sum = 0.0f;
    for (std::int32_t i = 0; i < count; ++i) sum = sum + window.at<float>(0xC + 4 * static_cast<std::size_t>(i));
    window.at<float>(8) = static_cast<float>(static_cast<double>(sum) / static_cast<double>(count));
}

// 007B8450, __thiscall(ECX = plane): when +904h is set, clear it.
void plane_clear_flag_904_007b8450(Raw plane) {
    if (plane.at<std::uint8_t>(0x904) != 0) plane.at<std::uint8_t>(0x904) = 0;
}

// 007B8550, __thiscall(ECX = block): decrements the fourteen dwords +18h, +1Ch,
// +20h, +24h, +70h, +28h, +3Ch, +40h, +44h, +54h, +58h, +5Ch, +6Ch, +74h, then
// rewinds each one that fell below -2 (signed JGE). The image's rewinds for
// +1Ch, +20h, +24h and +28h store 14h into +18h, not into the field itself
// (kept: that is what the code does). +3Ch/+40h/+44h/+54h/+58h/+5Ch rewind to
// 10h; +6Ch/+70h/+74h to 0Ch; +18h to 14h.
void index_block_decrement_all_007b8550(Raw block) {
    for (std::size_t off : {std::size_t{0x18}, std::size_t{0x1C}, std::size_t{0x20}, std::size_t{0x24},
                            std::size_t{0x70}, std::size_t{0x28}, std::size_t{0x3C}, std::size_t{0x40},
                            std::size_t{0x44}, std::size_t{0x54}, std::size_t{0x58}, std::size_t{0x5C},
                            std::size_t{0x6C}, std::size_t{0x74}})
        block.at<std::int32_t>(off) -= 1;
    const std::int32_t v18 = block.at<std::int32_t>(0x18);
    const std::int32_t v1c = block.at<std::int32_t>(0x1C);
    const std::int32_t v20 = block.at<std::int32_t>(0x20);
    const std::int32_t v24 = block.at<std::int32_t>(0x24);
    const std::int32_t v70 = block.at<std::int32_t>(0x70);
    if (v18 < -2) block.at<std::int32_t>(0x18) = 0x14;
    if (v1c < -2) block.at<std::int32_t>(0x18) = 0x14;
    if (v20 < -2) block.at<std::int32_t>(0x18) = 0x14;
    if (v24 < -2) block.at<std::int32_t>(0x18) = 0x14;
    if (block.at<std::int32_t>(0x28) < -2) block.at<std::int32_t>(0x18) = 0x14;
    if (block.at<std::int32_t>(0x3C) < -2) block.at<std::int32_t>(0x3C) = 0x10;
    if (block.at<std::int32_t>(0x40) < -2) block.at<std::int32_t>(0x40) = 0x10;
    if (block.at<std::int32_t>(0x44) < -2) block.at<std::int32_t>(0x44) = 0x10;
    if (block.at<std::int32_t>(0x6C) < -2) block.at<std::int32_t>(0x6C) = 0xC;
    if (v70 < -2) block.at<std::int32_t>(0x70) = 0xC;
    if (block.at<std::int32_t>(0x74) < -2) block.at<std::int32_t>(0x74) = 0xC;
    if (block.at<std::int32_t>(0x54) < -2) block.at<std::int32_t>(0x54) = 0x10;
    if (block.at<std::int32_t>(0x58) < -2) block.at<std::int32_t>(0x58) = 0x10;
    if (block.at<std::int32_t>(0x5C) < -2) block.at<std::int32_t>(0x5C) = 0x10;
}

// 007B8AA0, __thiscall(ECX = plane; char), RET 4: +9F9h = the byte.
void plane_set_turbo_byte_007b8aa0(Raw plane, std::uint8_t value) { plane.at<std::uint8_t>(0x9F9) = value; }

// 007B8C20, __thiscall(ECX = plane), AL = +A9Dh.
bool plane_is_cockpit_view_active_007b8c20(Raw plane) { return plane.at<std::uint8_t>(0xA9D) != 0; }

// 007B8C30, __thiscall(ECX = plane; bool enabled), RET 4. When enabled and
// +C0Ch is clear: ok = (plane+72Ch)->vtable[38h](); +C0Ch = enabled;
// +9F0h = ok ? 1.0f : 0.0f. Otherwise only +C0Ch = enabled.
void plane_set_control_enabled_007b8c30(Seg47Host& host, Raw plane, bool enabled) {
    if (enabled && plane.at<std::uint8_t>(0xC0C) == 0) {
        const bool ok = host.control_mode_vcall_38(plane.sub(0x72C));
        plane.at<std::uint8_t>(0xC0C) = static_cast<std::uint8_t>(enabled);
        plane.at<float>(0x9F0) = ok ? kOne_00d7a24c : 0.0f;
        return;
    }
    plane.at<std::uint8_t>(0xC0C) = static_cast<std::uint8_t>(enabled);
}

// 007B8CE0, __thiscall(ECX = plane; mode, flag), RET 8. +C08h = mode; when the
// flag byte and mode are both non-zero: +C1Ch = (mode == 1) and
// 007B8860([plane+DECh], mode == 1).
void plane_set_gear_mode_007b8ce0(Seg47Host& host, Raw plane, std::int32_t mode, std::uint8_t flag) {
    plane.at<std::int32_t>(0xC08) = mode;
    if (flag == 0 || mode == 0) return;
    const std::uint8_t down = mode == 1 ? 1 : 0;
    plane.at<std::uint8_t>(0xC1C) = down;
    host.actuator_set_gear_instant_007b8860(plane.deref(0xDEC), down);
}

// 007B8D70, __thiscall(ECX = plane), EAX: 1 unless the gear channel
// ([plane+DECh]+28h) is enabled and its value is not 1.0f (UCOMISS: a NaN
// value answers 0 when enabled).
std::int32_t plane_gear_is_down_007b8d70(Raw plane) {
    const Raw channel = plane.deref(0xDEC).sub(0x28);
    if (channel.at<std::uint8_t>(0) == 0) return 1;
    return channel.at<float>(4) == kOne_00d7a24c ? 1 : 0;
}

// 007B8E70, __thiscall(ECX = plane), ST0 = +B2Ch.
float plane_get_float_b2c_007b8e70(Raw plane) { return plane.at<float>(0xB2C); }

// 007B8EF0, __thiscall(ECX = plane): 007DE600(plane+C98h); +C39h = 1.
void plane_lose_power_007b8ef0(Seg47Host& host, Raw plane) {
    host.plane_power_block_007de600(plane.sub(0xC98));
    plane.at<std::uint8_t>(0xC39) = 1;
}

// 007B9080, __thiscall(ECX = control mode at plane+72Ch), EAX: 1 when
// plane->vtable[38h]() (the speed) > [plane+538h]+19Ch (FCOMI/JBE: a NaN
// answers 0).
std::int32_t plane_control_mode_speed_above_class_19c_007b9080(Seg47Host& host, Raw mode) {
    const Raw plane{mode.p - 0x72C};
    const Raw desc = plane.deref(0x538);
    const float speed = host.plane_vcall_speed_38(plane);
    return speed > desc.at<float>(0x19C) ? 1 : 0;
}

namespace {
// The weapon controller is the plane: +994h racks at +974h.
std::int32_t rack_count(Raw plane) { return plane.at<std::int32_t>(0x994); }
Raw rack(Raw plane, std::int32_t i) { return plane.deref(0x974 + 4 * static_cast<std::size_t>(i)); }

void* first_rack_answering_218(Seg47Host& host, Raw plane) {
    for (std::int32_t i = 0; i < rack_count(plane); ++i) {
        void* const r = host.rack_vcall_218(rack(plane, i), 0x2A);
        if (r != nullptr) return r;
    }
    return nullptr;
}
}  // namespace

// 007B9100 and 007B9510 (identical bodies), __thiscall(ECX = plane), EAX: the
// first non-null rack->vtable[218h](2Ah), else 0.
void* weapon_controller_first_bomb_class_007b9100(Seg47Host& host, Raw plane) {
    return first_rack_answering_218(host, plane);
}
void* weapon_controller_first_bomb_class_007b9510(Seg47Host& host, Raw plane) {
    return first_rack_answering_218(host, plane);
}

// 007B91C0, __thiscall(ECX = plane; kind, loadout), RET 8, AL: some rack whose
// descriptor rack->vtable[220h](loadout) is non-null answers
// descriptor->vtable[08h](kind).
bool weapon_controller_has_ordnance_of_kind_007b91c0(Seg47Host& host, Raw plane, std::int32_t kind,
                                                     std::int32_t loadout) {
    for (std::int32_t i = 0; i < rack_count(plane); ++i) {
        const Raw d = host.rack_vcall_descriptor_220(rack(plane, i), loadout);
        if (d && host.descriptor_vcall_is_kind_08(d, kind)) return true;
    }
    return false;
}

// 007B9230, __thiscall(ECX = plane), EAX: racks > 0 ? rack0->vtable[220h](1) : 0.
void* weapon_controller_first_slot_descriptor_007b9230(Seg47Host& host, Raw plane) {
    if (rack_count(plane) <= 0) return nullptr;
    return host.rack_vcall_descriptor_220(rack(plane, 0), 1).p;
}

// 007B92E0, __thiscall(ECX = plane), AL: some rack->vtable[20Ch]() answers.
bool weapon_controller_any_slot_answers_20c_007b92e0(Seg47Host& host, Raw plane) {
    for (std::int32_t i = 0; i < rack_count(plane); ++i)
        if (host.rack_vcall_20c(rack(plane, i))) return true;
    return false;
}

// 007B9320, __thiscall(ECX = plane; loadout), RET 4, AL: some rack descriptor
// is kind 2Ah and none of 2Ch, 31h, 2Bh, 33h, 2Dh (asked in that order, each
// a vtable[08h] call; the first answer that rules the rack out moves on).
bool weapon_controller_has_general_bomb_ordnance_007b9320(Seg47Host& host, Raw plane, std::int32_t loadout) {
    for (std::int32_t i = 0; i < rack_count(plane); ++i) {
        const Raw d = host.rack_vcall_descriptor_220(rack(plane, i), loadout);
        if (!d) continue;
        if (!host.descriptor_vcall_is_kind_08(d, 0x2A)) continue;
        if (host.descriptor_vcall_is_kind_08(d, 0x2C)) continue;
        if (host.descriptor_vcall_is_kind_08(d, 0x31)) continue;
        if (host.descriptor_vcall_is_kind_08(d, 0x2B)) continue;
        if (host.descriptor_vcall_is_kind_08(d, 0x33)) continue;
        if (host.descriptor_vcall_is_kind_08(d, 0x2D)) continue;
        return true;
    }
    return false;
}

// 007B93E0 / 007B93F0 / 007B9400 / 007B94F0 / 007B9500, __thiscall(ECX =
// plane; loadout), RET 4, AL: 007B91C0(plane; 2Fh / 2Bh / 33h / 2Ch / 31h, loadout).
bool weapon_controller_has_drop_kamikaze_ordnance_007b93e0(Seg47Host& host, Raw plane, std::int32_t loadout) {
    return weapon_controller_has_ordnance_of_kind_007b91c0(host, plane, 0x2F, loadout);
}
bool weapon_controller_has_torpedo_ordnance_007b93f0(Seg47Host& host, Raw plane, std::int32_t loadout) {
    return weapon_controller_has_ordnance_of_kind_007b91c0(host, plane, 0x2B, loadout);
}
bool weapon_controller_has_rocket_kind33_007b9400(Seg47Host& host, Raw plane, std::int32_t loadout) {
    return weapon_controller_has_ordnance_of_kind_007b91c0(host, plane, 0x33, loadout);
}
bool weapon_controller_has_depth_charge_ordnance_007b94f0(Seg47Host& host, Raw plane, std::int32_t loadout) {
    return weapon_controller_has_ordnance_of_kind_007b91c0(host, plane, 0x2C, loadout);
}
bool weapon_controller_has_level_bomb_ordnance_007b9500(Seg47Host& host, Raw plane, std::int32_t loadout) {
    return weapon_controller_has_ordnance_of_kind_007b91c0(host, plane, 0x31, loadout);
}

// 007B9410, __thiscall(ECX = plane; loadout), RET 4, AL: some rack descriptor
// is kind 33h with 0.0f >= descriptor+E0h (COMISS/JAE: a NaN does not answer).
bool weapon_controller_find_ready_rocket_007b9410(Seg47Host& host, Raw plane, std::int32_t loadout) {
    for (std::int32_t i = 0; i < rack_count(plane); ++i) {
        const Raw d = host.rack_vcall_descriptor_220(rack(plane, i), loadout);
        if (d && host.descriptor_vcall_is_kind_08(d, 0x33) && 0.0f >= d.at<float>(0xE0)) return true;
    }
    return false;
}

// 007B9480, __thiscall(ECX = plane; loadout), RET 4, AL: some rack descriptor
// is kind 33h with !(0.0f >= descriptor+E0h) (COMISS/JB: a NaN answers).
bool weapon_controller_has_rocket_ordnance_007b9480(Seg47Host& host, Raw plane, std::int32_t loadout) {
    for (std::int32_t i = 0; i < rack_count(plane); ++i) {
        const Raw d = host.rack_vcall_descriptor_220(rack(plane, i), loadout);
        if (d && host.descriptor_vcall_is_kind_08(d, 0x33) && !(0.0f >= d.at<float>(0xE0))) return true;
    }
    return false;
}

// ============================================================================
// Plane helpers 007B9590-007BB110
// ============================================================================

namespace {
constexpr float kSinkAltitude_00ce38b8 = 10.0f;      // 41200000
constexpr float kSinkScaleDefault_00ce3854 = 3.0f;   // 40400000
constexpr double kSinkScaleBase_00d7a2b0 = 3.0;      // double 4008000000000000
constexpr double kSinkLimit_00ce3dc0 = 10.0;         // double 4024000000000000
constexpr double kHalfPi_00ce3830 = 1.5707963705062866;  // double 3FF921FB60000000
constexpr double kTwoPi_00ce3828 = 6.2831854820251465;   // double 401921FB60000000
constexpr double kNoHeight_00ce6658 = -1000.0;       // double C08F400000000000
constexpr float kNoHeight_00d7a240 = -1000.0f;       // C47A0000
constexpr float kHideScale_00ce4bc8 = 100000.0f;     // 47C35000
constexpr float kRammed_00cf58ec = 10000000.0f;      // 4B189680
constexpr double kTinyLength2_00ce3820 = 1e-10;      // double 3DDB7CDFD9D7BDBB
constexpr double kMinLength_00ce6638 = 0.01;         // double 3F847AE147AE147B
constexpr double kMinDt_00ce3d10 = 0.20000000298023224;  // double 3FC99999A0000000
constexpr float kMinDt_00ce54a0 = 0.2f;              // 3E4CCCCD
constexpr float kMinusOne_00d7a260 = -1.0f;          // BF800000
constexpr float kTenth_00d7a2f0 = 0.1f;              // 3DCCCCCD
}  // namespace

// 007B9590, __thiscall(ECX = plane): +520h = 0; when +DF4h is set, its
// vtable[10h]() and +DF4h = 0; the same for +6DCh; then tail-jumps to
// 00951FB0(plane).
void unit_instance_retire_ai_sub_nodes_007b9590(Seg47Host& host, Raw plane) {
    void* const first = plane.ptr(0xDF4);
    plane.at<std::uint8_t>(0x520) = 0;
    if (first != nullptr) {
        host.ai_node_vcall_retire_10(raw(first));
        plane.at<std::uint32_t>(0xDF4) = 0;
    }
    if (plane.ptr(0x6DC) != nullptr) {
        host.ai_node_vcall_retire_10(plane.deref(0x6DC));
        plane.at<std::uint32_t>(0x6DC) = 0;
    }
    host.unit_instance_retire_00951fb0(plane);
}

// 007B9620, __thiscall(ECX = plane), AL. Unreferenced in the image.
//   a = +9B4h: 10.0f > a (COMISS/JA, ordered) -> 1.
//   b = +9B8h: !(0 > b) (JBE: NaN included) -> 0.
//   f = 3.0f; c = +C64h: when 0 > c, f = (float)(3.0 - c * 0.800000011920929 / [+538h]+1ACh).
//   v = b * f + a (x87 extended, not stored); 10.0 > v (FCOMI/JBE: NaN -> 0) -> 1, else 0.
// Here v is formed in double (b * f is exact; the add can differ by a double
// rounding in the last bit).
bool plane_altitude_sink_predicate_007b9620(Raw plane) {
    const float a = plane.at<float>(0x9B4);
    if (kSinkAltitude_00ce38b8 > a) return true;
    const float b = plane.at<float>(0x9B8);
    if (!(0.0f > b)) return false;
    float f = kSinkScaleDefault_00ce3854;
    const float c = plane.at<float>(0xC64);
    if (0.0f > c) {
        f = static_cast<float>(kSinkScaleBase_00d7a2b0 -
                               (static_cast<double>(c) * kTuning664Scale_00ce3d40) /
                                   static_cast<double>(plane.deref(0x538).at<float>(0x1AC)));
    }
    const double v = static_cast<double>(b) * f + a;
    return kSinkLimit_00ce3dc0 > v;
}

// 007B96C0, __thiscall(ECX = plane): +C00h = 1; 00951F40(plane, 0).
void plane_detach_into_hangar_007b96c0(Seg47Host& host, Raw plane) {
    plane.at<std::uint8_t>(0xC00) = 1;
    host.unit_instance_detach_00951f40(plane, 0);
}

// 007B96F0, __thiscall(ECX = unit; f0, f1, f2, f3, f4, int n, float f6), RET 1Ch,
// EAX: [unit+C50h] null -> 0, else 007E2090(finder; the seven arguments, in order).
void* unit_find_gun_target_007b96f0(Seg47Host& host, Raw unit, float f0, float f1, float f2, float f3, float f4,
                                    std::int32_t n, float f6) {
    if (unit.ptr(0xC50) == nullptr) return nullptr;
    return host.gun_target_finder_scan_007e2090(unit.deref(0xC50), f0, f1, f2, f3, f4, n, f6);
}

// 007B9740, __thiscall(ECX = unit): when [unit+C50h] is set, tail-jumps to 007E2070(finder).
void plane_neighbours_clear_target_007b9740(Seg47Host& host, Raw unit) {
    if (unit.ptr(0xC50) != nullptr) host.gun_target_finder_clear_007e2070(unit.deref(0xC50));
}

// 007B97E0, __thiscall(ECX = plane), EAX = +9D4h (the squadron).
void* plane_get_squadron_007b97e0(Raw plane) { return plane.ptr(0x9D4); }

// 007B98A0, __thiscall(ECX = registry): [00F87294] = 0; vptr 00CE3818.
void firing_gun_registry_destruct_unwind_007b98a0(Seg47Host& host, Raw registry) {
    *static_cast<std::uint32_t*>(host.global_address(0x00F87294u)) = 0;
    registry.at<std::uint32_t>(0) = 0x00CE3818u;
}

// 007BA020, __thiscall(ECX = matrix; const matrix*), RET 4, EAX = matrix:
// product = 00413920(matrix; &stack 40h, other); 004134F0(matrix; product).
Raw matrix_multiply_in_place_007ba020(Seg47Host& host, Raw matrix, const void* other) {
    alignas(4) std::uint8_t product[0x40];
    void* const result = host.matrix_multiply_00413920(matrix, product, other);
    host.matrix_assign_004134f0(matrix, result);
    return matrix;
}

namespace {
// The session-message head every constructor here writes after 0075B430(type):
// +18h word, +1Ah and +1Ch bytes = 0, +4 = 1, then the vptr.
void message_head(Seg47Host& host, Raw message, std::uint32_t type) {
    host.session_message_construct_0075b430(message, type);
    message.at<std::uint16_t>(0x18) = 0;
    message.at<std::uint8_t>(0x1A) = 0;
    message.at<std::uint8_t>(0x1C) = 0;
}
}  // namespace

// 007BA170 ENGINEFIREFAILURE, __thiscall(ECX = message; int index, float value),
// RET 8, EAX: type 65h; +24h = index; +4 = 1; vptr 00D02CF4; +20h = value.
Raw session_message_enginefire_construct_007ba170(Seg47Host& host, Raw message, std::int32_t index, float value) {
    message_head(host, message, 0x65);
    message.at<std::int32_t>(0x24) = index;
    message.at<std::uint32_t>(4) = 1;
    message.at<std::uint32_t>(0) = 0x00D02CF4u;
    message.at<float>(0x20) = value;
    return message;
}

// 007BA1C0 ENGINESLOSTFAILURE, __thiscall(ECX = message; float), RET 4, EAX:
// type 66h; +4 = 1; vptr 00D02F94; +20h = value.
Raw session_message_engineslost_construct_007ba1c0(Seg47Host& host, Raw message, float value) {
    message_head(host, message, 0x66);
    message.at<std::uint32_t>(4) = 1;
    message.at<std::uint32_t>(0) = 0x00D02F94u;
    message.at<float>(0x20) = value;
    return message;
}

// 007BA200 SPINNINGFAILURE, __thiscall(ECX = message; char side, float), RET 8,
// EAX: type 67h; +24h = side; +4 = 1; vptr 00D02D08; +20h = value.
Raw session_message_spinning_construct_007ba200(Seg47Host& host, Raw message, std::uint8_t side, float value) {
    message_head(host, message, 0x67);
    message.at<std::uint8_t>(0x24) = side;
    message.at<std::uint32_t>(4) = 1;
    message.at<std::uint32_t>(0) = 0x00D02D08u;
    message.at<float>(0x20) = value;
    return message;
}

// 007BA340, __thiscall(ECX = game; float x, float z), RET 8, ST0. Walks the
// list [[game+19CCh]+34Ch] (node +4 next, +8 entity); for each entity that
// answers vtable[5Ch](44h), h = 0087FA50(entity; x, z) stored to a float;
// h > -1000.0 (FCOMI/JA, ordered) returns h. The end returns -1000.0f.
float landscape_height_at_007ba340(Seg47Host& host, Raw game, float x, float z) {
    for (Raw node = game.deref(0x19CC).deref(0x34C); node; node = node.deref(4)) {
        const Raw entity = node.deref(8);
        if (!host.entity_vcall_is_kind_5c(entity, 0x44)) continue;
        const float h = host.landscape_height_0087fa50(entity, x, z);
        if (static_cast<double>(h) > kNoHeight_00ce6658) return h;
    }
    return kNoHeight_00d7a240;
}

// 007BA3E0, __thiscall(ECX = unit), EAX: unit->vtable[5Ch](20h) and
// [[unit+3F4h]+80h] is 1, 5 or 6.
std::int32_t unit_is_gun_of_subtype156_007ba3e0(Seg47Host& host, Raw unit) {
    if (!host.entity_vcall_is_kind_5c(unit, 0x20)) return 0;
    const std::int32_t subtype = unit.deref(0x3F4).at<std::int32_t>(0x80);
    return (subtype == 1 || subtype == 5 || subtype == 6) ? 1 : 0;
}

// 007BA450, __fastcall(ECX = const float* a, EDX = const float* b; const float* c),
// RET 4, ST0. m = (*a < *b or unordered) ? *b : *a; result = (m < *c or
// unordered) ? *c : m (FCOMI/JB on both). Equal operands keep the earlier one, so
// max3(+0, -0, ..) is +0. The selections index a pair: MSVC folds the plain ternary
// into MAXSS, which keeps the later operand on a tie (cc13-fixture7).
float max3_007ba450(const float* a, const float* b, const float* c) {
    const float ab[2] = {*b, *a};
    const float m = ab[*a >= *b];
    const float mc[2] = {*c, m};
    return mc[m >= *c];
}

// 007BA600, __thiscall(ECX = plane; const float[3]*), RET 4: +F0h..+F8h = the vector.
void plane_store_message_vector_f0_007ba600(Raw plane, const float* v) {
    plane.at<float>(0xF0) = v[0];
    plane.at<float>(0xF4) = v[1];
    plane.at<float>(0xF8) = v[2];
}

// 007BA760, __thiscall(ECX = unit), EAX: 1 when +C3Ah or +5Dh is set.
std::int32_t unit_gun_fire_blocked_007ba760(Raw unit) {
    if (unit.at<std::uint8_t>(0xC3A) != 0) return 1;
    if (unit.at<std::uint8_t>(0x5D) != 0) return 1;
    return 0;
}

// 007BA7B0, __thiscall(ECX = record): +2Ch = 10, +38h = 0, +30h = 3, +34h = 1;
// -1 into +28h, +24h, +20h, +1Ch, +18h, +44h, +40h, +3Ch, +5Ch, +58h, +54h,
// +74h, +70h, +6Ch; +78h..+80h = [00F87574..7Ch]; +60h..+68h = +78h..+80h;
// +48h..+50h = +60h..+68h.
void plane_state_record_init_defaults_007ba7b0(Seg47Host& host, Raw record) {
    record.at<std::int32_t>(0x2C) = 0xA;
    record.at<std::int32_t>(0x38) = 0;
    record.at<std::int32_t>(0x30) = 3;
    record.at<std::int32_t>(0x34) = 1;
    for (std::size_t off : {std::size_t{0x28}, std::size_t{0x24}, std::size_t{0x20}, std::size_t{0x1C},
                            std::size_t{0x18}, std::size_t{0x44}, std::size_t{0x40}, std::size_t{0x3C},
                            std::size_t{0x5C}, std::size_t{0x58}, std::size_t{0x54}, std::size_t{0x74},
                            std::size_t{0x70}, std::size_t{0x6C}})
        record.at<std::int32_t>(off) = -1;
    const auto* const zero = static_cast<const float*>(host.global_address(0x00F87574u));
    record.at<float>(0x78) = zero[0];
    record.at<float>(0x7C) = zero[1];
    record.at<float>(0x80) = zero[2];
    record.at<float>(0x60) = record.at<float>(0x78);
    record.at<float>(0x64) = record.at<float>(0x7C);
    record.at<float>(0x68) = record.at<float>(0x80);
    record.at<float>(0x48) = record.at<float>(0x60);
    record.at<float>(0x4C) = record.at<float>(0x64);
    record.at<float>(0x50) = record.at<float>(0x68);
}

// 007BA890, __thiscall(ECX = message), EAX: id = (uint16)+20h; id < [00F89A10]
// (signed) -> [[00F89A54] + (id - [00F89A0C])*10h + 0Ch], else
// [[00F89AA8] + (id - [00F89A60])*10h + 0Ch].
void* session_message_resolve_entity_by_net_id_007ba890(Seg47Host& host, Raw message) {
    const std::int32_t id = message.at<std::uint16_t>(0x20);
    const auto g = [&](std::uint32_t address) { return *static_cast<std::int32_t*>(host.global_address(address)); };
    if (id < g(0x00F89A10u)) {
        const Raw table = raw(host.global_pointer(0x00F89A54u));
        return table.ptr(static_cast<std::size_t>(static_cast<std::uint32_t>(id - g(0x00F89A0Cu)) << 4) + 0xC);
    }
    const Raw table = raw(host.global_pointer(0x00F89AA8u));
    return table.ptr(static_cast<std::size_t>(static_cast<std::uint32_t>(id - g(0x00F89A60u)) << 4) + 0xC);
}

// 007BA8D0 PLANE_DEADMEATROT, __thiscall(ECX = message; plane), RET 4, EAX:
// type C8h; +4 = 1; vptr 00D03540; +20h..+28h = plane+BA0h..+BA8h.
Raw session_message_plane_deadmeatrot_construct_007ba8d0(Seg47Host& host, Raw message, Raw plane) {
    message_head(host, message, 0xC8);
    message.at<std::uint32_t>(4) = 1;
    message.at<std::uint32_t>(0) = 0x00D03540u;
    message.at<float>(0x20) = plane.at<float>(0xBA0);
    message.at<float>(0x24) = plane.at<float>(0xBA4);
    message.at<float>(0x28) = plane.at<float>(0xBA8);
    return message;
}

// 007BA950 PLANE_CONTROLFACTOR, __thiscall(ECX = message; seven dwords by
// value), RET 1Ch, EAX: 0075B430(C9h); REP MOVSD of the seven argument dwords
// to +20h..+38h; then the head bytes, +4 = 1, vptr 00D02E20.
Raw session_message_plane_controlfactor_construct_007ba950(Seg47Host& host, Raw message,
                                                           const std::uint32_t (&values)[7]) {
    host.session_message_construct_0075b430(message, 0xC9);
    std::memcpy(message.field(0x20), values, sizeof(values));
    message.at<std::uint16_t>(0x18) = 0;
    message.at<std::uint8_t>(0x1A) = 0;
    message.at<std::uint8_t>(0x1C) = 0;
    message.at<std::uint32_t>(4) = 1;
    message.at<std::uint32_t>(0) = 0x00D02E20u;
    return message;
}

// 007BA990 PLANE_HIT, __thiscall(ECX = message; int a, float b, c, d, const
// float[3]*), RET 14h, EAX: type CAh; +20h = a; +4 = 1; vptr 00D02E34;
// +24h/+28h/+2Ch = b, c, d; +30h..+38h = the vector.
Raw session_message_plane_hit_construct_007ba990(Seg47Host& host, Raw message, std::int32_t a, float b, float c,
                                                 float d, const float* v) {
    message_head(host, message, 0xCA);
    message.at<std::int32_t>(0x20) = a;
    message.at<std::uint32_t>(4) = 1;
    message.at<std::uint32_t>(0) = 0x00D02E34u;
    message.at<float>(0x24) = b;
    message.at<float>(0x28) = c;
    message.at<float>(0x2C) = d;
    message.at<float>(0x30) = v[0];
    message.at<float>(0x34) = v[1];
    message.at<float>(0x38) = v[2];
    return message;
}

namespace {
// The 3Ch-byte plane state record copy (FLD/FSTP per float, so a signalling
// NaN would be quieted; MOVZX/MOV per byte): +0..+20h floats, +24h..+26h
// bytes, +28h..+38h floats. +27h is not copied.
void copy_state_record(Raw destination, Raw source) {
    // The FLD/FSTP copies are modelled bit for bit (cc13-fixture10).
    const auto fld_fstp = [&](std::size_t off) {
        destination.at<std::uint32_t>(off) = cc13x87::x87_fld_fstp_bits(source.at<std::uint32_t>(off));
    };
    for (std::size_t off = 0; off <= 0x20; off += 4) fld_fstp(off);
    destination.at<std::uint8_t>(0x24) = source.at<std::uint8_t>(0x24);
    destination.at<std::uint8_t>(0x25) = source.at<std::uint8_t>(0x25);
    destination.at<std::uint8_t>(0x26) = source.at<std::uint8_t>(0x26);
    for (std::size_t off = 0x28; off <= 0x38; off += 4) fld_fstp(off);
}
}  // namespace

// 007BAA80 and 007BAAF0 (identical), __thiscall(ECX = destination; const
// record*), RET 4, EAX = destination.
Raw plane_state_record_copy_007baa80(Raw destination, Raw source) {
    copy_state_record(destination, source);
    return destination;
}
Raw plane_control_state_copy_007baaf0(Raw destination, Raw source) {
    copy_state_record(destination, source);
    return destination;
}

// 007BAC50, RET 8 (ECX unused; unreferenced): v = {r, r};
// 0093E9A0(overlay, &v) (__cdecl, two arguments).
void map_overlay_draw_ellipse_thunk_007bac50(Seg47Host& host, void* overlay, float r) {
    float v[2] = {r, r};
    host.map_overlay_draw_ellipse_0093e9a0(overlay, v);
}

// 007BAC80, __thiscall(ECX = node): 00B6DA70(node; 0.0f, 0); then
// node->vtable[2Ch](&{100000.0f, 100000.0f, 100000.0f}).
void plane_hide_model_007bac80(Seg47Host& host, Raw node) {
    host.scene_node_set_visibility_factor_00b6da70(node, 0.0f, 0);
    float scale[3] = {kHideScale_00ce4bc8, kHideScale_00ce4bc8, kHideScale_00ce4bc8};
    host.scene_node_vcall_set_scale_2c(node, scale);
}

// 007BAD00, RET 4 (ECX unused): the contact pair callback. a =
// 00C32490([pair])+6Ch, b = 00C32490([pair+4])+6Ch (planes). When
// (a+72Ch)->vtable[38h]() and (b+72Ch)->vtable[38h]() and a+DFCh is clear:
// a+DFCh = 1 and a->vtable[1ACh](10000000.0f).
void plane_contact_pair_ram_007bad00(Seg47Host& host, void* const* pair) {
    const Raw a = host.physics_body_owner_00c32490(pair[0]).deref(0x6C);
    const Raw b = host.physics_body_owner_00c32490(pair[1]).deref(0x6C);
    if (!host.control_mode_vcall_38(a.sub(0x72C))) return;
    if (!host.control_mode_vcall_38(b.sub(0x72C))) return;
    if (a.at<std::uint8_t>(0xDFC) != 0) return;
    a.at<std::uint8_t>(0xDFC) = 1;
    host.plane_vcall_damage_1ac(a, kRammed_00cf58ec);
}

// 007BAD70, __fastcall(ECX = float a[3] (in/out), EDX = const float b[3]).
// d = a - b (each stored float); l2 = d.y*d.y + d.x*d.x + d.z*d.z (x87
// extended, one rounding to float; double here); len = l2 > 1e-10 ?
// sqrt(l2) (float) : 0.0f (FCOMI/JBE). When 0.01 > len (JBE: a NaN length
// normalises) d = 00439950(stack vector) (a random unit direction), else
// d *= (float)(1/len). a = b + d (each component stored float).
void vector3_place_at_unit_distance_007bad70(Seg47Host& host, float* a, const float* b) {
    float d[3] = {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
    const float l2 = static_cast<float>(static_cast<double>(d[1]) * d[1] + static_cast<double>(d[0]) * d[0] +
                                        static_cast<double>(d[2]) * d[2]);
    float len;
    if (static_cast<double>(l2) > kTinyLength2_00ce3820)
        len = static_cast<float>(std::sqrt(static_cast<double>(l2)));
    else
        len = 0.0f;
    if (kMinLength_00ce6638 > static_cast<double>(len)) {
        float random_direction[3];
        const float* const r = host.vector3_set_random_unit_direction_00439950(random_direction);
        d[0] = r[0];
        d[1] = r[1];
        d[2] = r[2];
    } else {
        const float inv = static_cast<float>(1.0 / static_cast<double>(len));
        d[0] = d[0] * inv;
        d[1] = d[1] * inv;
        d[2] = d[2] * inv;
    }
    const float r0 = b[0] + d[0];
    const float r1 = d[1] + b[1];
    const float r2 = b[2] + d[2];
    a[0] = r0;
    a[1] = r1;
    a[2] = r2;
}

// 007BAE90, __thiscall(ECX = limiter; float target, float dt), RET 8.
// Layout: +0 value, +4 low, +8 high, +0Ch rate, +10h max rate, +14h accel.
//   dt <= 0 (COMISS/JAE, ordered) -> return.
//   dt' = (0.20000000298023224 > dt) ? 0.2f : dt   (FCOMI/JBE)
//   want = (float)((target - value) / dt')
//   rate: want >= rate (FCOMI/JB; NaN takes the low arm):
//     hi = (float)(rate + accel*dt); rate = (want > hi) ? hi : want
//   else lo = (float)(rate - accel*dt); rate = (lo > want) ? lo : want
//   (both JBE: a NaN keeps want). The accel step uses the original dt.
//   rate = clamp: (-max > rate) ? -max : ((rate > max) ? max : rate)  (JBE).
//   value = (float)(value + dt * rate); 00415690(&value, &low, &high).
void rate_limiter_step_007bae90(Seg47Host& host, Raw limiter, float target, float dt) {
    if (0.0f >= dt) return;
    const float step_dt = (kMinDt_00ce3d10 > static_cast<double>(dt)) ? kMinDt_00ce54a0 : dt;
    const float want =
        static_cast<float>((static_cast<double>(target) - limiter.at<float>(0)) / static_cast<double>(step_dt));
    const float rate = limiter.at<float>(0xC);
    const double accel_step = static_cast<double>(limiter.at<float>(0x14)) * dt;
    float next;
    if (!(want < rate || want != want || rate != rate)) {
        const float hi = static_cast<float>(rate + accel_step);
        next = (want > hi) ? hi : want;
    } else {
        const float lo = static_cast<float>(rate - accel_step);
        next = (lo > want) ? lo : want;
    }
    limiter.at<float>(0xC) = next;
    const float negative_max = -limiter.at<float>(0x10);
    const float current = limiter.at<float>(0xC);
    float clamped;
    if (negative_max > current) {
        clamped = negative_max;
    } else {
        const float max = limiter.at<float>(0x10);
        clamped = (current > max) ? max : current;
    }
    limiter.at<float>(0xC) = clamped;
    limiter.at<float>(0) =
        static_cast<float>(static_cast<double>(dt) * limiter.at<float>(0xC) + limiter.at<float>(0));
    host.math_clamp_in_place_00415690(limiter.field(0), limiter.field(4), limiter.field(8));
}

// 007BAF90, __thiscall(ECX = block; arg), RET 4, EAX = block. The control
// block defaults: +40h = 1, +ACh = arg, +A8h = -1.0f, +94h = 0.1f, and the
// float pairs below, in the image's store order.
Raw plane_unit_instance_init_control_block_007baf90(Raw block, std::uint32_t arg) {
    block.at<float>(0x54) = 0.0f;
    block.at<float>(0x48) = 0.0f;
    block.at<float>(0x5C) = kOne_00d7a24c;
    block.at<float>(0x58) = kOne_00d7a24c;
    block.at<float>(0x6C) = 0.0f;
    block.at<float>(0x60) = 0.0f;
    block.at<float>(0x74) = kOne_00d7a24c;
    block.at<float>(0x70) = kOne_00d7a24c;
    block.at<float>(0x84) = 0.0f;
    block.at<float>(0x78) = 0.0f;
    block.at<float>(0x8C) = kOne_00d7a24c;
    block.at<float>(0x88) = kOne_00d7a24c;
    block.at<float>(0x9C) = 0.0f;
    block.at<float>(0x90) = 0.0f;
    block.at<float>(0xA4) = kOne_00d7a24c;
    block.at<float>(0xA0) = kOne_00d7a24c;
    block.at<std::uint32_t>(0xAC) = arg;
    block.at<float>(0xA8) = kMinusOne_00d7a260;
    block.at<std::uint8_t>(0x40) = 1;
    block.at<float>(0x94) = kTenth_00d7a2f0;
    block.at<float>(0x98) = kOne_00d7a24c;
    block.at<float>(0x4C) = kMinusOne_00d7a260;
    block.at<float>(0x50) = kOne_00d7a24c;
    block.at<float>(0x7C) = kMinusOne_00d7a260;
    block.at<float>(0x80) = kOne_00d7a24c;
    block.at<float>(0x64) = kMinusOne_00d7a260;
    block.at<float>(0x68) = kOne_00d7a24c;
    return block;
}

// 007BB060, __thiscall(ECX = class; char force), RET 4, SEH frame.
// keep = 1; when 0 <= [[00E188A8]+18ECh] <= 7, keep = !0095BAB0([class+70h]).
// A scope object 0043AB50(&local, keep == 0) / 0043AB80(&local) brackets
// 009598D0(class, force || !keep) (the image clears only AL for the false
// case) and, when [class+21Ch] is set, 004B3F10([class+21Ch]+4).
void plane_class_activate_resources_007bb060(Seg47Host& host, Raw cls, std::uint8_t force) {
    bool keep = true;
    const std::int32_t mode = raw(host.global_pointer(0x00E188A8u)).at<std::int32_t>(0x18EC);
    if (mode >= 0 && mode <= 7) keep = !host.resource_query_0095bab0(cls.ptr(0x70));
    alignas(4) std::uint8_t scope[8];
    host.resource_scope_construct_0043ab50(scope, keep ? 0 : 1);
    host.scene_class_activate_009598d0(cls, (force != 0 || !keep) ? 1 : 0);
    if (cls.ptr(0x21C) != nullptr) host.resource_holder_activate_004b3f10(cls.deref(0x21C).sub(4));
    host.resource_scope_destruct_0043ab80(scope);
}

// 007BB110, __thiscall(ECX = unit), AL. channel = [unit+DECh]+60h: disabled,
// or its value == 1.0f (UCOMISS, ordered) -> 1; otherwise
// [unit + (uint16)[00F876B8]*8 + 9C3h] == 0.
bool unit_can_release_ordnance_007bb110(Seg47Host& host, Raw unit) {
    const Raw channel = unit.deref(0xDEC).sub(0x60);
    if (channel.at<std::uint8_t>(0) == 0) return true;
    if (channel.at<float>(4) == kOne_00d7a24c) return true;
    const std::uint16_t slot = *static_cast<const std::uint16_t*>(host.global_address(0x00F876B8u));
    return unit.at<std::uint8_t>(static_cast<std::size_t>(slot) * 8 + 0x9C3) == 0;
}

// ============================================================================
// Plane helpers 007BB150-007BBC50
// ============================================================================

namespace {
constexpr float kMinusZero_00d7a208 = -0.0f;           // 80000000
constexpr float kCockpitTurnEpsilon_00d7a23c = 0.001f;  // 3A83126F
constexpr float kExplosionMark_00d059a0 = 99.0f;        // 42C60000
constexpr float kSiteHeight_00ce5444 = 80.0f;           // 42A00000
constexpr float kSiteHeightKind6_00ce685c = 40.0f;      // 42200000

std::uint16_t local_slot(Seg47Host& host) {
    return *static_cast<const std::uint16_t*>(host.global_address(0x00F876B8u));
}
}  // namespace

// 007BB150, __thiscall(ECX = plane; char flag), RET 4: +DF0h = flag ?
// [plane + (uint16)[00F876B8]*8 + 9C3h] : 0.
void plane_publish_slot_byte_df0_007bb150(Seg47Host& host, Raw plane, std::uint8_t flag) {
    std::uint8_t value = flag;
    if (flag != 0) value = plane.at<std::uint8_t>(static_cast<std::size_t>(local_slot(host)) * 8 + 0x9C3);
    plane.at<std::uint8_t>(0xDF0) = value;
}

// 007BB170, __thiscall(ECX = plane; int slot, float time), RET 8, SEH frame.
// Session mode [[00E188A8]+1FE4h] 0 or 1: when the squadron +9D4h is set and
// time != +E8Ch (FUCOMIP: a NaN counts as different): +E8Ch = time, +E90h =
// slot; then, when the plane is the leader (+9D8h == 0), recurses into each
// other member i < [squadron+3CCh] (member = i <= 4 ? [squadron+3D0h+i*4] :
// null, no null test). Any other mode: a stack message (0075B430(CCh), the
// head bytes, +4 = 1, vptr 00D02E48, +20h = time, +24h = slot) goes to
// 00770AF0([00E188A8]+1EF0h, &message).
void plane_set_pilot_command_last_time_007bb170(Seg47Host& host, Raw plane, std::int32_t slot, float time) {
    const Raw game = raw(host.global_pointer(0x00E188A8u));
    const std::int32_t mode = game.at<std::int32_t>(0x1FE4);
    if (mode == 0 || mode == 1) {
        if (plane.ptr(0x9D4) == nullptr) return;
        if (time == plane.at<float>(0xE8C)) return;
        plane.at<float>(0xE8C) = time;
        plane.at<std::int32_t>(0xE90) = slot;
        if (plane.at<std::int32_t>(0x9D8) != 0) return;
        if (plane.deref(0x9D4).at<std::int32_t>(0x3CC) <= 0) return;
        for (std::uint32_t i = 0; static_cast<std::int32_t>(i) < plane.deref(0x9D4).at<std::int32_t>(0x3CC); ++i) {
            if (static_cast<std::int32_t>(i) == plane.at<std::int32_t>(0x9D8)) continue;
            const Raw member = i <= 4 ? plane.deref(0x9D4).deref(0x3D0 + 4 * static_cast<std::size_t>(i)) : Raw{nullptr};
            plane_set_pilot_command_last_time_007bb170(host, member, slot, time);
        }
        return;
    }
    alignas(4) std::uint8_t storage[0x28];
    Raw message{storage};
    host.session_message_construct_0075b430(message, 0xCC);
    message.at<std::uint16_t>(0x18) = 0;
    message.at<std::uint8_t>(0x1A) = 0;
    message.at<std::uint32_t>(4) = 1;
    message.at<std::uint8_t>(0x1C) = 0;
    message.at<std::uint32_t>(0) = 0x00D02E48u;
    message.at<float>(0x20) = time;
    message.at<std::int32_t>(0x24) = slot;
    host.session_send_to_secondary_first_peer_00770af0(game.sub(0x1EF0), storage);
}

// 007BB2C0, __thiscall(ECX = plane): when +C02h and +BF4h are set: the
// world-pose refresh 00414DB0(plane) when +C8h is clear, then +BF8h =
// 006BC530([plane+BF4h]; plane+FCh, plane+BFCh).
void plane_refresh_ground_contact_007bb2c0(Seg47Host& host, Raw plane) {
    if (plane.at<std::uint8_t>(0xC02) == 0) return;
    if (plane.ptr(0xBF4) == nullptr) return;
    if (plane.at<std::uint8_t>(0xC8) == 0) host.entity_pose_refresh_world_00414db0(plane);
    plane.at<std::uint8_t>(0xBF8) =
        host.air_ops_holder_over_runway_006bc530(plane.deref(0xBF4), plane.field(0xFC), plane.field(0xBFC)) ? 1 : 0;
}

// 007BB310, __thiscall(ECX = plane). When +A9Dh is set and 004BFED0(plane):
// +E00h = identity (004134F0 from a stack identity); +E40h = +E00h;
// for the yaw +A1Ch and then the pitch +A28h:
//   m = v > 0.0f (COMISS/JBE) ? v : -0.0f - v; when m > 0.001f:
//   yaw angle = -0.0f - v -> 00B646E0 (rotation Y); pitch angle =
//   -0.0f - (-0.0f - v) -> 00B64640 (rotation X); stack = that matrix
//   (004134F0), +E00h = 00413920(stack; other stack, +E00h).
// Then 00B63D50(ECX = +E40h, EDX = +E00h). Always tail-jumps to 00955970(plane).
void plane_update_cockpit_transform_007bb310(Seg47Host& host, Raw plane) {
    if (plane.at<std::uint8_t>(0xA9D) != 0 && host.plane_cockpit_ready_004bfed0(plane)) {
        const float identity[16] = {kOne_00d7a24c, 0, 0, 0, 0, kOne_00d7a24c, 0, 0,
                                    0, 0, kOne_00d7a24c, 0, 0, 0, 0, kOne_00d7a24c};
        const Raw view = plane.sub(0xE00);
        const Raw base = plane.sub(0xE40);
        alignas(4) std::uint8_t work[0x40];
        std::memcpy(work, identity, sizeof(identity));
        alignas(4) std::uint8_t first[0x40];
        alignas(4) std::uint8_t second[0x40];
        void* const assigned = host.matrix_assign_004134f0(view, work);
        host.matrix_assign_004134f0(base, assigned);
        const float yaw = plane.at<float>(0xA1C);
        const float yaw_magnitude = yaw > 0.0f ? yaw : kMinusZero_00d7a208 - yaw;
        if (yaw_magnitude > kCockpitTurnEpsilon_00d7a23c) {
            float angle = kMinusZero_00d7a208 - plane.at<float>(0xA1C);
            void* const rotation = host.matrix_build_rotation_y_00b646e0(first, &angle);
            host.matrix_assign_004134f0(Raw{work}, rotation);
            void* const product = host.matrix_multiply_00413920(Raw{work}, second, view.p);
            host.matrix_assign_004134f0(view, product);
        }
        const float pitch = plane.at<float>(0xA28);
        const float pitch_magnitude = pitch > 0.0f ? pitch : kMinusZero_00d7a208 - pitch;
        if (pitch_magnitude > kCockpitTurnEpsilon_00d7a23c) {
            const float negated = kMinusZero_00d7a208 - plane.at<float>(0xA28);
            float angle = kMinusZero_00d7a208 - negated;
            void* const rotation = host.matrix_build_rotation_x_00b64640(second, &angle);
            host.matrix_assign_004134f0(Raw{work}, rotation);
            void* const product = host.matrix_multiply_00413920(Raw{work}, first, view.p);
            host.matrix_assign_004134f0(view, product);
        }
        host.matrix_build_affine_inverse_00b63d50(base, view);
    }
    host.entity_refresh_scene_node_matrix_00955970(plane);
}

// 007BB4C0, __fastcall(ECX = const float a[], EDX = const float b[]), ST0.
// For the three 10h-strided rows: d = a_i - b_i (each stored float), l2 =
// d.y^2 + d.x^2 + d.z^2 (on the x87 stack, rounded to float at the FSTP),
// dist = l2 > 1e-10 ? sqrt(l2) (float) : 0.0f; sum = dist + sum (float).
// The sum of the three vertex distances (the ledger name "MinVertexDistance"
// does not match the body).
float triangle_vertex_distance_sum_007bb4c0(const float* a, const float* b) {
    float sum = 0.0f;
    for (int row = 0; row < 3; ++row) {
        const float* const p = a + 4 * row;
        const float* const q = b + 4 * row;
        const float dx = p[0] - q[0];
        const float dy = p[1] - q[1];
        const float dz = p[2] - q[2];
        // cc13-fixture2 follow-up: no reference in the image (rel32 and absolute scans are
        // empty); modelled at the main thread's 24-bit precision control (x87_*24).
        const float l2 = static_cast<float>(cc13x87::x87_add24(
            cc13x87::x87_add24(cc13x87::x87_mul24(dy, dy), cc13x87::x87_mul24(dx, dx)), cc13x87::x87_mul24(dz, dz)));
        float dist;
        if (static_cast<double>(l2) > kTinyLength2_00ce3820)
            dist = static_cast<float>(std::sqrt(static_cast<double>(l2)));
        else
            dist = 0.0f;
        sum = dist + sum;
    }
    return sum;
}

// 007BBA10, __thiscall(ECX = plane; const message*), RET 4. kind = +20h:
//   3: 0084B6F0(ECX = [plane+730h]+38h, EDX = &{msg+24h..2Ch}; &{msg+30h..38h}),
//      then 00926D90(plane, 1).
//   2: 007D8180(plane+AB0h, 0); when +5Dh is clear, plane->vtable[70h](1)
//      and +C10h = 99.0f.
//   else: the kind-3 call with the effect table at +28h, then 00926D90(plane, 1).
void plane_on_impact_message_007bba10(Seg47Host& host, Raw plane, Raw message) {
    const std::int32_t kind = message.at<std::int32_t>(0x20);
    if (kind == 2) {
        host.plane_flight_set_gravity_term_007d8180(plane.sub(0xAB0), 0);
        if (plane.at<std::uint8_t>(0x5D) == 0) {
            host.plane_vcall_70(plane, 1);
            plane.at<float>(0xC10) = kExplosionMark_00d059a0;
        }
        return;
    }
    const float position[3] = {message.at<float>(0x24), message.at<float>(0x28), message.at<float>(0x2C)};
    const float direction[3] = {message.at<float>(0x30), message.at<float>(0x34), message.at<float>(0x38)};
    const Raw effects = plane.deref(0x730).sub(kind == 3 ? 0x38 : 0x28);
    host.effect_spawn_oriented_point_0084b6f0(effects, position, direction);
    host.mission_entity_kill_00926d90(plane, 1);
}

// 007BBB70, __thiscall(ECX = plane; float* out), RET 4: out = +AC8h..+AD0h.
void unit_copy_vector_ac8_007bbb70(Raw plane, float* out) {
    out[0] = plane.at<float>(0xAC8);
    out[1] = plane.at<float>(0xACC);
    out[2] = plane.at<float>(0xAD0);
}

// 007BBC10, __thiscall(ECX = plane; target), RET 4, EAX: +DF4h set and
// [[+DF4h] + (uint16)[00F876B8]*1Ch + 98h] == target.
std::int32_t unit_pilot_target_is_007bbc10(Seg47Host& host, Raw plane, void* target) {
    if (plane.ptr(0xDF4) == nullptr) return 0;
    return plane.deref(0xDF4).ptr(static_cast<std::size_t>(local_slot(host)) * 0x1C + 0x98) == target ? 1 : 0;
}

// 007BBC50, __thiscall(ECX = plane), AL. Requires +900h == 4, +BF8h, holder =
// +BF4h, [holder+4] and [[holder+4]+7Ch]. p = 00427EB0(plane) (no stack
// argument; the pushed holder+48h stays for the next call); local =
// 00414D10(ECX = &local, EDX = p; holder+48h). limit = 80.0f, or 40.0f when
// [[holder+4]+7Ch]->vtable[5Ch](6). AL = local.y > limit (FCOMI/JBE: NaN -> 0).
bool plane_is_grounded_on_owned_site_007bbc50(Seg47Host& host, Raw plane) {
    if (plane.at<std::int32_t>(0x900) != 4) return false;
    if (plane.at<std::uint8_t>(0xBF8) == 0) return false;
    const Raw holder = plane.deref(0xBF4);
    if (!holder) return false;
    if (holder.ptr(4) == nullptr) return false;
    if (holder.deref(4).ptr(0x7C) == nullptr) return false;
    void* const pose = host.entity_pose_world_position_refreshed_00427eb0(plane);
    float local[3];
    host.vector3_transform_affine_point_00414d10(local, pose, holder.field(0x48));
    float limit = kSiteHeight_00ce5444;
    if (host.entity_vcall_is_kind_5c(holder.deref(4).deref(0x7C), 6)) limit = kSiteHeightKind6_00ce685c;
    return local[1] > limit;
}

// ============================================================================
// Plane helpers 007BC550-007BD3E0 and the level-bomb gate 007C7750
// ============================================================================

namespace {
constexpr double kReleaseHeightDivisor_00d05a38 = 2.799999952316284;  // double 4006666660000000
constexpr float kFltMax_00d7a248 = 3.4028234663852886e+38f;           // 7F7FFFFF
constexpr float kSync400_00cfd710 = 400.0f;
constexpr float kSync800_00ce3950 = 800.0f;
constexpr float kSync1200_00cfd714 = 1200.0f;
constexpr float kSync1600_00d05a40 = 1600.0f;
constexpr float kSync3000_00cfa424 = 3000.0f;

// The plane's child list: head +48h, next +44h.
template <class F>
void for_each_child(Raw plane, F&& f) {
    for (Raw child = plane.deref(0x48); child; child = child.deref(0x44))
        if (f(child)) return;
}

bool gun_subtype_156(Seg47Host& host, Raw child) {
    if (!host.entity_vcall_is_kind_5c(child, 0x20)) return false;
    const std::int32_t subtype = child.deref(0x3F4).at<std::int32_t>(0x80);
    return subtype == 1 || subtype == 5 || subtype == 6;
}
}  // namespace

// 007BC550, __thiscall(ECX = plane). Only when +4A4h (the Lua self) is set:
// state +900h 0 or 1 -> 00922F80(plane, 0) and 00951F40(plane, 0); any other
// state -> 00922F30(plane, 0) when +5Ch is clear, 00951F40(plane, 1) and
// 00779C60(plane, 1.0f).
void plane_sync_elevator_attachment_for_state_007bc550(Seg47Host& host, Raw plane) {
    if (plane.ptr(0x4A4) == nullptr) return;
    const std::int32_t state = plane.at<std::int32_t>(0x900);
    if (state == 1 || state == 0) {
        host.scene_node_disable_00922f80(plane, 0);
        host.unit_instance_detach_00951f40(plane, 0);
        return;
    }
    if (plane.at<std::uint8_t>(0x5C) == 0) host.scene_node_enable_00922f30(plane, 0);
    host.unit_instance_detach_00951f40(plane, 1);
    host.unit_update_owner_hud_state_00779c60(plane, kOne_00d7a24c);
}

// 007BC610, __thiscall(ECX = plane). When +808h, +80Ch and [[00F8D39C]+0Ch]
// are set: +808h->vtable[2Ch](&{0, 0, 0}); 007BAC80(+80Ch); +A9Dh = 1;
// 00B3C650([[00F8D39C]+0Ch]; +808h).
void plane_enter_cockpit_view_007bc610(Seg47Host& host, Raw plane) {
    if (plane.ptr(0x808) == nullptr) return;
    if (plane.ptr(0x80C) == nullptr) return;
    if (raw(host.global_pointer(0x00F8D39Cu)).ptr(0xC) == nullptr) return;
    const float zero[3] = {0.0f, 0.0f, 0.0f};
    host.scene_node_vcall_set_scale_2c(plane.deref(0x808), zero);
    plane_hide_model_007bac80(host, plane.deref(0x80C));
    plane.at<std::uint8_t>(0xA9D) = 1;
    host.cockpit_helper_set_attached_node_00b3c650(raw(host.global_pointer(0x00F8D39Cu)).deref(0xC),
                                                   plane.ptr(0x808));
}

// 007BC690, __thiscall(ECX = plane). When +808h, +80Ch and [[00F8D39C]+0Ch]
// are set: 007BAC80(+808h); 00B6DA70(+80Ch; 1.0f, 0); +80Ch->vtable[2Ch](&{0,
// 0, 0}); +A9Dh = 0; 00B3C650([[00F8D39C]+0Ch]; 0). Always: +E40h = identity
// (004134F0 from a stack identity) and +E00h = that (004134F0 on its result).
void plane_leave_cockpit_view_007bc690(Seg47Host& host, Raw plane) {
    if (plane.ptr(0x808) != nullptr && plane.ptr(0x80C) != nullptr &&
        raw(host.global_pointer(0x00F8D39Cu)).ptr(0xC) != nullptr) {
        plane_hide_model_007bac80(host, plane.deref(0x808));
        host.scene_node_set_visibility_factor_00b6da70(plane.deref(0x80C), kOne_00d7a24c, 0);
        const float zero[3] = {0.0f, 0.0f, 0.0f};
        host.scene_node_vcall_set_scale_2c(plane.deref(0x80C), zero);
        plane.at<std::uint8_t>(0xA9D) = 0;
        host.cockpit_helper_set_attached_node_00b3c650(raw(host.global_pointer(0x00F8D39Cu)).deref(0xC), nullptr);
    }
    const float identity[16] = {kOne_00d7a24c, 0, 0, 0, 0, kOne_00d7a24c, 0, 0,
                                0, 0, kOne_00d7a24c, 0, 0, 0, 0, kOne_00d7a24c};
    void* const assigned = host.matrix_assign_004134f0(plane.sub(0xE40), identity);
    host.matrix_assign_004134f0(plane.sub(0xE00), assigned);
}

// 007BC8C0, __thiscall(ECX = plane), ST0: a = sum of 006E3500(rack), b = sum
// of 006E3520(rack) over the +994h racks; result = (float)(a / (b > 1 ? b : 1))
// (FILD/FIDIV; with no racks the divisor is 1 and a is 0).
float weapon_controller_total_bomb_rounds_007bc8c0(Seg47Host& host, Raw plane) {
    std::int32_t a = 0;
    std::int32_t b = 0;
    std::int32_t divisor = 1;
    if (rack_count(plane) > 0) {
        for (std::int32_t i = 0; i < rack_count(plane); ++i) {
            a += host.bomb_platform_round_count_006e3500(rack(plane, i));
            b += host.bomb_platform_capacity_006e3520(rack(plane, i));
        }
        divisor = b > 1 ? b : 1;
    }
    return static_cast<float>(static_cast<double>(a) / static_cast<double>(divisor));
}

// 007BCA50, __thiscall(ECX = plane), EAX: +96Ch when set; otherwise the first
// child answering vtable[5Ch](20h) with [[child+3F4h]+80h] == 0Ah gives
// [[[child+3F8h]+34h]+8]; none -> 0.
std::int32_t plane_ordnance_icon_kind_007bca50(Seg47Host& host, Raw plane) {
    if (plane.at<std::int32_t>(0x96C) != 0) return plane.at<std::int32_t>(0x96C);
    std::int32_t kind = 0;
    for_each_child(plane, [&](Raw child) {
        if (host.entity_vcall_is_kind_5c(child, 0x20) && child.deref(0x3F4).at<std::int32_t>(0x80) == 0xA) {
            kind = child.deref(0x3F8).deref(0x34).at<std::int32_t>(8);
            return true;
        }
        return false;
    });
    return kind;
}

// 007BCB30, __thiscall(ECX = plane; class id), RET 4. +8FDh = 1; 00926D90(rack,
// 1) for each rack; +C54h = id; 0095A880(plane; 009552E0([plane+538h]; id),
// 0, 0); +994h = 0; then each child answering vtable[5Ch](25h) with +5Fh clear
// is appended to the racks.
void plane_re_equip_007bcb30(Seg47Host& host, Raw plane, std::int32_t id) {
    plane.at<std::uint8_t>(0x8FD) = 1;
    for (std::int32_t i = 0; i < rack_count(plane); ++i) host.mission_entity_kill_00926d90(rack(plane, i), 1);
    plane.at<std::int32_t>(0xC54) = id;
    void* const devices = host.plane_class_device_set_009552e0(plane.deref(0x538), id);
    host.unit_instantiate_class_devices_0095a880(plane, devices, 0, 0);
    plane.at<std::int32_t>(0x994) = 0;
    for_each_child(plane, [&](Raw child) {
        if (host.entity_vcall_is_kind_5c(child, 0x25) && child.at<std::uint8_t>(0x5F) == 0) {
            plane.ptr(0x974 + 4 * static_cast<std::size_t>(plane.at<std::int32_t>(0x994))) = child.p;
            plane.at<std::int32_t>(0x994) += 1;
        }
        return false;
    });
}

// 007BCC20, __thiscall(ECX = plane), ST0: -1.0f when +5Eh or +5Dh is set or
// +9D4h, +9D0h or +DF4h is null; 0.0f when (plane+72Ch)->vtable[38h]() is
// false; otherwise tail-jumps to 00999AE0([plane+DF4h]).
float plane_formation_wait_value_007bcc20(Seg47Host& host, Raw plane) {
    if (plane.at<std::uint8_t>(0x5E) != 0 || plane.at<std::uint8_t>(0x5D) != 0 || plane.ptr(0x9D4) == nullptr ||
        plane.ptr(0x9D0) == nullptr || plane.ptr(0xDF4) == nullptr)
        return kMinusOne_00d7a260;
    if (!host.control_mode_vcall_38(plane.sub(0x72C))) return 0.0f;
    return host.pilot_bot_first_task_wait_value_00999ae0(plane.deref(0xDF4));
}

// 007BCCF0, __thiscall(ECX = plane; float* out), RET 4, EAX = out (the
// level-bomb task's paratrooper branch, 009B7D7D). The pose refresh when +C8h
// is clear; t = (float)(y / 2.799999952316284) with y = +100h; a = t * the
// forward row +94h..+9Ch (row 2 of the +74h local matrix; each float); out = (x + a.x, y + a.y, z + a.z) with
// (x, y, z) = +FCh..+104h (each stored float; the y sum in x87 extended is
// stored once); out.y = 0078CF20([[00E188A8]+19F0h]; out.x, out.z); then for
// every node of [[00E188A8]+19CCh]+34Ch (no kind test here) h =
// 0087FA50([node+8]; out.x, out.z) and out.y = h when h > out.y (FCOMI/JBE).
float* plane_ordnance_release_point_world_007bccf0(Seg47Host& host, Raw plane, float* out) {
    if (plane.at<std::uint8_t>(0xC8) == 0) host.entity_pose_refresh_world_00414db0(plane);
    const float y = plane.at<float>(0x100);
    const float x = plane.at<float>(0xFC);
    const float z = plane.at<float>(0x104);
    const float t = static_cast<float>(static_cast<double>(y) / kReleaseHeightDivisor_00d05a38);
    const float ax = plane.at<float>(0x94) * t;
    const float ay = plane.at<float>(0x98) * t;
    const float az = t * plane.at<float>(0x9C);
    const Raw game = raw(host.global_pointer(0x00E188A8u));
    out[0] = x + ax;
    out[1] = y + ay;
    out[2] = z + az;
    out[1] = host.game_world_sample_water_height_0078cf20(game.deref(0x19F0), out[0], out[2]);
    for (Raw node = game.deref(0x19CC).deref(0x34C); node; node = node.deref(4)) {
        const float h = host.landscape_height_0087fa50(node.deref(8), out[0], out[2]);
        if (h > out[1]) out[1] = h;
    }
    return out;
}

// 007BD120, __thiscall(ECX = plane), ST0: the first rack whose bullet class
// [[rack+3F8h]+34h] has +8 == 12h tail-jumps to 00809DE0(class); none -> 0.0.
float weapon_controller_rocket_ignition_lift_007bd120(Seg47Host& host, Raw plane) {
    for (std::int32_t i = 0; i < rack_count(plane); ++i) {
        const Raw cls = rack(plane, i).deref(0x3F8).deref(0x34);
        if (cls.at<std::int32_t>(8) == 0x12) return host.rocket_class_ignition_lift_00809de0(cls);
    }
    return 0.0f;
}

// 007BD1A0, __thiscall(ECX = plane): 0072C6A0(child) for every child gun of
// subtype 1, 5 or 6 (vtable[5Ch](20h), [[child+3F4h]+80h]).
void plane_create_child_gun_ai_bots_007bd1a0(Seg47Host& host, Raw plane) {
    for_each_child(plane, [&](Raw child) {
        if (gun_subtype_156(host, child)) host.gun_create_ai_bots_0072c6a0(child);
        return false;
    });
}

// 007BD1F0, __thiscall(ECX = plane), AL: some child is such a gun.
bool plane_has_child_gun_of_subtype156_007bd1f0(Seg47Host& host, Raw plane) {
    bool found = false;
    for_each_child(plane, [&](Raw child) {
        found = gun_subtype_156(host, child);
        return found;
    });
    return found;
}

// 007BD240, __thiscall(ECX = plane; record** arg), RET 4. Only for the flight
// states +900h 4..7: key = *arg ? [*arg+20h] : 8; b = 00927850(plane; key)
// (AL kept in a stack byte); then 0077B280(plane, arg, b, 0.0f, 1, 400.0f, 3,
// 800.0f, 6, 1200.0f, 0Ah, 1600.0f, 0Fh, 3000.0f, 14h, FLT_MAX, 23h), __cdecl.
// The image passes b as the whole stack dword whose upper three bytes are
// uninitialised; here they are zero.
void plane_build_replication_delta_007bd240(Seg47Host& host, Raw plane, void** arg) {
    const std::int32_t state = plane.at<std::int32_t>(0x900);
    if (state != 7 && state != 6 && state != 4 && state != 5) return;
    const Raw record = raw(*arg);
    const std::int32_t key = record ? record.at<std::int32_t>(0x20) : 8;
    const std::uint32_t b = host.entity_replication_query_00927850(plane, key) ? 1u : 0u;
    host.replication_build_sync_delta_0077b280(plane, arg, b, 0.0f, 1, kSync400_00cfd710, 3, kSync800_00ce3950, 6,
                                               kSync1200_00cfd714, 0xA, kSync1600_00d05a40, 0xF,
                                               kSync3000_00cfa424, 0x14, kFltMax_00d7a248, 0x23);
}

// 007BD3E0 PLANE_CREATE, __thiscall(ECX = message; a1, plane, reference
// entity, a4, a5, float f6, f7, f8, char c9, a10), RET 28h, EAX, SEH frame.
// 00766FC0(message; C0h, a1, plane, 1); vptr 00D0379C; +110h = reference ?
// (uint16)[reference+174h] : 0; +114h..+11Ch = f6..f8; +120h = a4, +124h = a5;
// +128h = (plane+170h)->vtable[0](); +12Ah = c9; +129h = (byte)[plane+900h];
// +12Ch = a10.
Raw session_message_plane_create_construct_007bd3e0(Seg47Host& host, Raw message, std::uint32_t a1, Raw plane,
                                                    Raw reference, std::uint32_t a4, std::uint32_t a5, float f6,
                                                    float f7, float f8, std::uint8_t c9, std::uint32_t a10) {
    host.net_entity_create_message_construct_00766fc0(message, 0xC0, a1, plane, 1);
    message.at<std::uint32_t>(0) = 0x00D0379Cu;
    message.at<std::uint16_t>(0x110) = reference ? reference.at<std::uint16_t>(0x174) : 0;
    message.at<float>(0x114) = f6;
    message.at<float>(0x118) = f7;
    message.at<float>(0x11C) = f8;
    message.at<std::uint32_t>(0x120) = a4;
    message.at<std::uint32_t>(0x124) = a5;
    message.at<std::uint8_t>(0x128) = host.replicator_vcall_00(plane.sub(0x170));
    message.at<std::uint8_t>(0x12A) = c9;
    message.at<std::uint8_t>(0x129) = plane.at<std::uint8_t>(0x900);
    message.at<std::uint32_t>(0x12C) = a10;
    return message;
}

// 007C7750, __thiscall(ECX = plane; char check_doors), RET 4, AL: the
// level-bomb attitude gate. With check_doors: channel [plane+DECh]+60h enabled,
// its value not 1.0f (UCOMISS: NaN counts as not at 1.0) and
// [plane + (uint16)[00F876B8]*8 + 9C3h] set -> 0. Then |+C68h| (bank) >
// tuning+550h -> 0 and |+C64h| (pitch) > tuning+550h -> 0 (FCOMI/JA: NaN
// passes; 0042E740 fetched for each test); else 1.
bool plane_level_bomb_attitude_gate_007c7750(Seg47Host& host, Raw plane, std::uint8_t check_doors) {
    if (check_doors != 0) {
        const Raw channel = plane.deref(0xDEC).sub(0x60);
        if (channel.at<std::uint8_t>(0) != 0 && !(channel.at<float>(4) == kOne_00d7a24c) &&
            plane.at<std::uint8_t>(static_cast<std::size_t>(local_slot(host)) * 8 + 0x9C3) != 0)
            return false;
    }
    const float bank = std::fabs(plane.at<float>(0xC68));
    if (bank > host.ai_tuning_0042e740().at<float>(0x550)) return false;
    const float pitch = std::fabs(plane.at<float>(0xC64));
    if (pitch > host.ai_tuning_0042e740().at<float>(0x550)) return false;
    return true;
}

// ============================================================================
// Engine pod arrays {+0 data, +4 size, +8 capacity} and small plane helpers
// ============================================================================

// 007BD600, __thiscall(ECX = array; count), RET 4: 40h-byte elements. count
// = max(count, 1); when capacity < count: operator new(count*40h) (00BF55BE),
// copy each of the size elements (16 dwords) to a non-null destination slot,
// _free(old data), data = new, capacity = count.
void pod_array64_reserve_007bd600(Seg47Host& host, Raw array, std::int32_t count) {
    if (count < 1) count = 1;
    if (array.at<std::int32_t>(8) >= count) return;
    auto* const fresh = static_cast<std::uint8_t*>(host.operator_new_00bf55be(static_cast<std::uint32_t>(count) << 6));
    std::uint8_t* destination = fresh;
    for (std::int32_t i = 0; i < array.at<std::int32_t>(4); ++i, destination += 0x40)
        if (destination != nullptr)
            std::memcpy(destination, static_cast<std::uint8_t*>(array.ptr(0)) + static_cast<std::size_t>(i) * 0x40, 0x40);
    host.free_00bf6989(array.ptr(0));
    array.ptr(0) = fresh;
    array.at<std::int32_t>(8) = count;
}

// 007BD940, __thiscall(ECX = array; count), RET 4: the 007BD600 shape with
// 4-byte elements.
void pod_array4_reserve_007bd940(Seg47Host& host, Raw array, std::int32_t count) {
    if (count < 1) count = 1;
    if (array.at<std::int32_t>(8) >= count) return;
    auto* const fresh = static_cast<std::uint32_t*>(host.operator_new_00bf55be(static_cast<std::uint32_t>(count) * 4));
    std::uint32_t* destination = fresh;
    for (std::int32_t i = 0; i < array.at<std::int32_t>(4); ++i, ++destination)
        if (destination != nullptr) *destination = static_cast<std::uint32_t*>(array.ptr(0))[i];
    host.free_00bf6989(array.ptr(0));
    array.ptr(0) = fresh;
    array.at<std::int32_t>(8) = count;
}

// 007C2DB0, __thiscall(ECX = array; const element*), RET 4: when size ==
// capacity, 007BD600(array, max(capacity*2, 1)) (JG 1); the 40h bytes go to
// data + size*40h unless that address is null; size += 1.
void pod_array64_push_back_007c2db0(Seg47Host& host, Raw array, const void* element) {
    if (array.at<std::int32_t>(4) == array.at<std::int32_t>(8)) {
        std::int32_t grown = array.at<std::int32_t>(8) * 2;
        if (grown <= 1) grown = 1;
        pod_array64_reserve_007bd600(host, array, grown);
    }
    std::uint8_t* const destination =
        static_cast<std::uint8_t*>(array.ptr(0)) + (static_cast<std::size_t>(array.at<std::int32_t>(4)) << 6);
    if (destination != nullptr) std::memcpy(destination, element, 0x40);
    array.at<std::int32_t>(4) += 1;
}

// 007C2F80, __thiscall(ECX = array): capacity < 0 -> 007BD600(array, 0); the
// size counts down to 0 one step at a time (no element work), then size = 0.
void pod_array64_clear_007c2f80(Seg47Host& host, Raw array) {
    if (array.at<std::int32_t>(8) < 0) pod_array64_reserve_007bd600(host, array, 0);
    while (array.at<std::int32_t>(4) > 0) array.at<std::int32_t>(4) -= 1;
    array.at<std::int32_t>(4) = 0;
}

// 007C3FC0, __thiscall(ECX = array): the 007C2F80 body, then size = 0 and
// _free(data).
void pod_array64_destruct_007c3fc0(Seg47Host& host, Raw array) {
    if (array.at<std::int32_t>(8) < 0) pod_array64_reserve_007bd600(host, array, 0);
    while (array.at<std::int32_t>(4) > 0) array.at<std::int32_t>(4) -= 1;
    void* const data = array.ptr(0);
    array.at<std::int32_t>(4) = 0;
    host.free_00bf6989(data);
}

// 007C90D0, __thiscall(ECX = array of refcounted pointers): when size is
// non-zero, the last pointer p: when p is set, InterlockedDecrement(p+4) == 0
// -> p->vtable[0]() and the slot = 0; then size -= 1.
void ref_ptr_vector_pop_back_007c90d0(Seg47Host& host, Raw array) {
    const std::int32_t size = array.at<std::int32_t>(4);
    if (size == 0) return;
    void** const slot = static_cast<void**>(array.ptr(0)) + (size - 1);
    if (*slot != nullptr) {
        const Raw p = raw(*slot);
        if (host.interlocked_decrement_ce2220(reinterpret_cast<long*>(p.field(4))) == 0)
            host.refcounted_vcall_destroy_00(p);
        *slot = nullptr;
    }
    array.at<std::int32_t>(4) -= 1;
}

// 007C9150, __thiscall(ECX = array): 007C3000(array, 0), then _free(data).
void pod_array4_destruct_007c9150(Seg47Host& host, Raw array) {
    host.pod_array4_resize_007c3000(array, 0);
    host.free_00bf6989(array.ptr(0));
}

// 007CD2C0, __thiscall(ECX = vector): 007C9290(vector, 0), then _free(data).
void plane_engine_effect_vector_destruct_007cd2c0(Seg47Host& host, Raw vector) {
    plane_engine_effect_vector_resize_007c9290(host, vector, 0);
    host.free_00bf6989(vector.ptr(0));
}

// 007BDB80, __thiscall(ECX = entity), EAX: entity when non-null and
// vtable[5Ch](21h) answers, else 0.
void* entity_cast_to_mrfs_gun_007bdb80(Seg47Host& host, Raw entity) {
    if (!entity) return nullptr;
    return host.entity_vcall_is_kind_5c(entity, 0x21) ? entity.p : nullptr;
}

// 007BDBA0, __thiscall(ECX = entity), EAX = entity+CCh after the pose refresh
// 00414DB0 when +C8h is clear.
void* entity_world_matrix_refreshed_007bdba0(Seg47Host& host, Raw entity) {
    if (entity.at<std::uint8_t>(0xC8) == 0) host.entity_pose_refresh_world_00414db0(entity);
    return entity.field(0xCC);
}

// 007BDCB0, __thiscall(ECX = registry), EAX, SEH frame: vptr 00D05A80;
// +4 = 00BD1860() (BSP_CriticalSection_Create).
Raw firing_gun_registry_construct_007bdcb0(Seg47Host& host, Raw registry) {
    registry.at<std::uint32_t>(0) = 0x00D05A80u;
    registry.ptr(4) = host.critical_section_create_00bd1860();
    return registry;
}

// 007BDF50, __thiscall(ECX = state; float out[16]), RET 4, EAX = out: out =
// identity; 00467050(out; +20h, +24h, +28h) (the rotation from three
// angles); out+30h..+38h = state+2Ch..+34h.
float* plane_control_state_init_identity_007bdf50(Seg47Host& host, Raw state, float* out) {
    for (int i = 0; i < 16; ++i) out[i] = (i % 5 == 0) ? kOne_00d7a24c : 0.0f;
    host.matrix_set_rotation_00467050(out, state.at<float>(0x20), state.at<float>(0x24), state.at<float>(0x28));
    out[12] = state.at<float>(0x2C);
    out[13] = state.at<float>(0x30);
    out[14] = state.at<float>(0x34);
    return out;
}

// ============================================================================
// Rack queries, registries and refcounted holders 007C1D80-007C9510
// ============================================================================

namespace {
// The intrusive release the holders inline: p = [holder]; when set,
// InterlockedDecrement(p+4) == 0 -> p->vtable[0](); then [holder] = 0.
void release_ref_holder(Seg47Host& host, Raw holder) {
    const Raw p = holder.deref(0);
    if (!p) return;
    if (host.interlocked_decrement_ce2220(reinterpret_cast<long*>(p.field(4))) == 0) host.refcounted_vcall_destroy_00(p);
    holder.at<std::uint32_t>(0) = 0;
}

bool is_rack_child(Seg47Host& host, Raw child) { return host.entity_vcall_is_kind_5c(child, 0x25); }
}  // namespace

// 007C1D80, __thiscall(ECX = plane): child->vtable[200h]() for every child
// answering vtable[5Ch](25h) (the bomb platforms).
void plane_reload_bomb_platforms_007c1d80(Seg47Host& host, Raw plane) {
    for_each_child(plane, [&](Raw child) {
        if (is_rack_child(host, child)) host.rack_vcall_reload_200(child);
        return false;
    });
}

// 007C1DB0, __thiscall(ECX = plane), EAX: the sum of 006E3500(child) over the
// kind-25h children.
std::int32_t unit_count_remaining_ordnance_rounds_007c1db0(Seg47Host& host, Raw plane) {
    std::int32_t sum = 0;
    for_each_child(plane, [&](Raw child) {
        if (is_rack_child(host, child)) sum += host.bomb_platform_round_count_006e3500(child);
        return false;
    });
    return sum;
}

// 007C1DE0, __thiscall(ECX = plane), EAX: the sum of child->vtable[21Ch](33h)
// over the kind-25h children.
std::int32_t plane_rocket_round_count_007c1de0(Seg47Host& host, Raw plane) {
    std::int32_t sum = 0;
    for_each_child(plane, [&](Raw child) {
        if (is_rack_child(host, child)) sum += host.rack_vcall_round_count_21c(child, 0x33);
        return false;
    });
    return sum;
}

// 007C1F60, __thiscall(ECX = plane), EAX: the sum of 006E3500(child) over the
// kind-25h children answering child->vtable[210h](2Ah, 0).
std::int32_t unit_count_droppable_rounds_007c1f60(Seg47Host& host, Raw plane) {
    std::int32_t sum = 0;
    for_each_child(plane, [&](Raw child) {
        if (is_rack_child(host, child) && host.rack_vcall_holds_kind_210(child, 0x2A, 0))
            sum += host.bomb_platform_round_count_006e3500(child);
        return false;
    });
    return sum;
}

// 007C2050, __thiscall(ECX = plane; record* out), RET 4, EAX = out. When
// +DF4h is set and its task count +5Ch is non-zero: the first task
// ([[+DF4h]+58h][0])->vtable[0Ch](out); otherwise out+0Ch = 0.
void* plane_slot_d4_forward_to_df4_007c2050(Seg47Host& host, Raw plane, Raw out) {
    const Raw bot = plane.deref(0xDF4);
    if (bot && bot.at<std::uint32_t>(0x5C) != 0) {
        host.bot_task_vcall_0c(bot.deref(0x58).deref(0), out);
        return out.p;
    }
    out.at<std::uint8_t>(0xC) = 0;
    return out.p;
}

// 007C2810, RET 4 (ECX unused), EAX: holder null -> 0. When [holder+0Ch] is
// null: operator new(84h) (00BF681B); non-null -> +0/+4/+8/+10h/+14h = 0,
// +0Ch = 1.0f, 007BA7B0(record); [holder+0Ch] = the record (or 0). Returns
// [holder+0Ch].
void* plane_state_record_get_or_create_007c2810(Seg47Host& host, Raw holder) {
    if (!holder) return nullptr;
    if (holder.ptr(0xC) == nullptr) {
        void* const memory = host.operator_new_00bf681b(0x84);
        if (memory != nullptr) {
            const Raw record = raw(memory);
            record.at<std::uint32_t>(0) = 0;
            record.at<std::uint32_t>(4) = 0;
            record.at<std::uint32_t>(8) = 0;
            record.at<std::uint32_t>(0x10) = 0;
            record.at<std::uint32_t>(0x14) = 0;
            record.at<float>(0xC) = kOne_00d7a24c;
            plane_state_record_init_defaults_007ba7b0(host, record);
            holder.ptr(0xC) = memory;
            return holder.ptr(0xC);
        }
        holder.ptr(0xC) = nullptr;
    }
    return holder.ptr(0xC);
}

// 007C2AA0, __thiscall(ECX = plane), ST0: the pose refresh when +C8h is clear;
// -(float)atan2([plane+ECh], [plane+F4h]) (00BF701A _CIatan2 on the two
// stored floats; the CRT result is rounded to float before the FCHS).
float plane_get_heading_angle_007c2aa0(Seg47Host& host, Raw plane) {
    if (plane.at<std::uint8_t>(0xC8) == 0) host.entity_pose_refresh_world_00414db0(plane);
    const float a = plane.at<float>(0xEC);
    const float b = plane.at<float>(0xCC + 0x28);
    const float angle = static_cast<float>(std::atan2(static_cast<double>(a), static_cast<double>(b)));
    return -angle;
}

// 007C2EC0, EAX: the firing-gun registry singleton [00F87294]. When it is
// null: m = 00415350(); the lock cs = [m+10h] (a stack guard {vptr 00CE37FC,
// cs}); when cs is set EnterCriticalSection(cs) ([00CE2218]) and cs+18h += 1;
// when [00F87294] is still null: operator new(8) (00BF681B), 007BDCB0 on it
// (or null), [00F87294] = it, and 00BD0C30(00415350(); it) registers it;
// then, with cs, cs+18h -= 1 and LeaveCriticalSection(cs) ([00CE2210]).
void* firing_gun_registry_instance_007c2ec0(Seg47Host& host) {
    auto* const slot = static_cast<void**>(host.global_address(0x00F87294u));
    if (*slot != nullptr) return *slot;
    const Raw manager = raw(host.lock_manager_00415350());
    void* const cs = manager.ptr(0x10);
    if (cs != nullptr) {
        host.enter_critical_section_ce2218(cs);
        raw(cs).at<std::int32_t>(0x18) += 1;
    }
    if (*slot == nullptr) {
        void* const memory = host.operator_new_00bf681b(8);
        void* const registry = memory != nullptr ? firing_gun_registry_construct_007bdcb0(host, raw(memory)).p : nullptr;
        *slot = registry;
        host.lock_manager_register_00bd0c30(raw(host.lock_manager_00415350()), *slot);
    }
    if (cs != nullptr) {
        raw(cs).at<std::int32_t>(0x18) -= 1;
        host.leave_critical_section_ce2210(cs);
    }
    return *slot;
}

// 007C33B0, __thiscall(ECX = registry): vptr 00D05A80; 0041CC80(registry+4);
// [00F87294] = 0; vptr 00CE3818.
void firing_gun_registry_destruct_007c33b0(Seg47Host& host, Raw registry) {
    registry.at<std::uint32_t>(0) = 0x00D05A80u;
    host.critical_section_destroy_0041cc80(registry.sub(4));
    *static_cast<std::uint32_t*>(host.global_address(0x00F87294u)) = 0;
    registry.at<std::uint32_t>(0) = 0x00CE3818u;
}

// 007C3420, __thiscall(ECX = record), EAX = record: +0 = 0; 004134F0(record+4,
// stack identity).
Raw transform_record_init_identity_007c3420(Seg47Host& host, Raw record) {
    const float identity[16] = {kOne_00d7a24c, 0, 0, 0, 0, kOne_00d7a24c, 0, 0,
                                0, 0, kOne_00d7a24c, 0, 0, 0, 0, kOne_00d7a24c};
    record.at<std::uint32_t>(0) = 0;
    host.matrix_assign_004134f0(record.sub(4), identity);
    return record;
}

// 007C34B0, __thiscall(ECX = record; const record*), RET 4, EAX, SEH frame:
// +0 = 0; when source+0 is set, +0 = it and InterlockedIncrement(it+4);
// 004134F0(record+4, source+4).
Raw plane_engine_effect_record_copy_construct_007c34b0(Seg47Host& host, Raw record, Raw source) {
    record.at<std::uint32_t>(0) = 0;
    if (source.ptr(0) != nullptr) {
        record.ptr(0) = source.ptr(0);
        host.interlocked_increment_ce221c(reinterpret_cast<long*>(source.deref(0).field(4)));
    }
    host.matrix_assign_004134f0(record.sub(4), source.field(4));
    return record;
}

// 007C3510, __thiscall(ECX = record; RefPtr effect by value), RET 4, EAX, SEH
// frame: +0 = 0; when effect is set, +0 = effect and InterlockedIncrement;
// 004134F0(record+4, stack identity); then the by-value parameter is released
// (InterlockedDecrement == 0 -> effect->vtable[0]()).
Raw plane_engine_effect_construct_007c3510(Seg47Host& host, Raw record, void* effect) {
    record.at<std::uint32_t>(0) = 0;
    if (effect != nullptr) {
        record.ptr(0) = effect;
        host.interlocked_increment_ce221c(reinterpret_cast<long*>(static_cast<std::uint8_t*>(effect) + 4));
    }
    const float identity[16] = {kOne_00d7a24c, 0, 0, 0, 0, kOne_00d7a24c, 0, 0,
                                0, 0, kOne_00d7a24c, 0, 0, 0, 0, kOne_00d7a24c};
    host.matrix_assign_004134f0(record.sub(4), identity);
    if (effect != nullptr &&
        host.interlocked_decrement_ce2220(reinterpret_cast<long*>(static_cast<std::uint8_t*>(effect) + 4)) == 0)
        host.refcounted_vcall_destroy_00(raw(effect));
    return record;
}

// 007C3600 / 007C3640 / 007C3680 / 007C94E0 / 007C9510 (identical),
// __thiscall(ECX = holder): the intrusive release of [holder], then [holder] = 0.
void ref_ptr_release_007c3600(Seg47Host& host, Raw holder) { release_ref_holder(host, holder); }
void ref_ptr_release_007c3640(Seg47Host& host, Raw holder) { release_ref_holder(host, holder); }
void ref_ptr_release_007c3680(Seg47Host& host, Raw holder) { release_ref_holder(host, holder); }
void ref_ptr_release_007c94e0(Seg47Host& host, Raw holder) { release_ref_holder(host, holder); }
void ref_ptr_release_007c9510(Seg47Host& host, Raw holder) { release_ref_holder(host, holder); }

// 007C3F60, __thiscall(ECX = plane), EAX: the first rack whose
// vtable[220h](0) descriptor is non-null decides: its +14h, or 00F872D5 when
// that is null; no such rack -> 0.
void* weapon_controller_first_slot_ordnance_007c3f60(Seg47Host& host, Raw plane) {
    for (std::int32_t i = 0; i < rack_count(plane); ++i) {
        const Raw d = host.rack_vcall_descriptor_220(rack(plane, i), 0);
        if (!d) continue;
        if (d.ptr(0x14) != nullptr) return d.ptr(0x14);
        return host.global_address(0x00F872D5u);
    }
    return nullptr;
}

// 007C4470, __thiscall(ECX = vector), EAX, SEH frame: vptr 00D05AB8, +4..+10h =
// 0, 007DDDE0(+14h), 007DDDE0(+20h).
Raw effect_vector_construct_007c4470(Seg47Host& host, Raw vector) {
    vector.at<std::uint32_t>(0) = 0x00D05AB8u;
    vector.at<std::uint32_t>(4) = 0;
    vector.at<std::uint32_t>(8) = 0;
    vector.at<std::uint32_t>(0xC) = 0;
    vector.at<std::uint32_t>(0x10) = 0;
    host.vector3_fill_one_007ddde0(vector.sub(0x14));
    host.vector3_fill_one_007ddde0(vector.sub(0x20));
    return vector;
}

// 007C6F20, __thiscall(ECX = record): the inline std::string tidy of the
// string at +38h (+3Ch buffer/pointer, +4Ch size, +50h capacity): capacity
// >= 10h -> operator delete(+3Ch); capacity = 0Fh, size = 0, buffer[0] = 0.
void record_destruct_string_3c_007c6f20(Seg47Host& host, Raw record) {
    if (record.at<std::uint32_t>(0x50) >= 0x10) host.operator_delete_00bf65ac(record.ptr(0x3C));
    record.at<std::uint32_t>(0x50) = 0xF;
    record.at<std::uint32_t>(0x4C) = 0;
    record.at<std::uint8_t>(0x3C) = 0;
}

// ============================================================================
// Plane flight-state helpers 007C0C60-007CD250
// ============================================================================

namespace {
constexpr float kSputterX0_00ce3850 = 5.0f;          // 40A00000
constexpr float kSputterX1_00ce89cc = 25.0f;         // 41C80000
constexpr float kSputterHeight_00ce3918 = 8.0f;      // 41000000
constexpr float kSputterProbeY_00ce3930 = 20.0f;     // 41A00000
constexpr float kSputterLo_00ced070 = 0.07f;         // 3D8F5C29
constexpr float kSputterHi_00ce8198 = 0.18f;         // 3E3851EC
}  // namespace

// 007C0C60, plane vtable +4Ch, __thiscall(ECX = plane; float a[3], b[3], c[3],
// float d by value), RET 28h: 007BE9B0(plane+84Ch; the ten words), then
// 00924610(plane; the same ten words).
void plane_slot4c_point_event_007c0c60(Seg47Host& host, Raw plane, const float* a, const float* b, const float* c,
                                       float d) {
    host.plane_apply_point_event_local_007be9b0(plane.sub(0x84C), a, b, c, d);
    host.entity_point_event_00924610(plane, a, b, c, d);
}

// 007C3CB0, __thiscall(ECX = plane; float dt), RET 4. prev = +964h;
// r = 00419010(5.0, 0.0, 25.0, 1.0, 007D99C0(plane+AB0h)) (RET 14h);
// t = (float)(prev - r * dt) (x87; double here) -> +964h. When
// [[00E188A8]+1ED4h] is set and 0.0f > t (COMISS/JBE):
//   unless +900h == 6: (plane+72Ch)->vtable[38h]() must answer; +BF8h set
//   with +BF4h set returns; 8.0f > 00427EB0(plane).y (COMISS/JBE) required.
//   m = [[00E188A8]+1ED4h]->vtable[114h](&local) (EAX = matrix); P = (m+30h,
//   0, m+38h), Q = (m+30h, 20.0f, m+38h); unless +900h == 6,
//   00903BC0([[00E188A8]+19CCh]; &Q, &P) answering returns.
//   +964h = U(0.07, 0.18) on stream 0 (ECX = 0, shared generator 00BD2F10);
//   P.y = 1.0f; 008685E0(ECX = &result, EDX = [[00E188A8]+19ECh]; RefPtr
//   [[plane+538h]+218h] by value (incremented when set), &P, 0, 0);
//   handle = *result; 00440A30(&result); handle+9 = 1.
void plane_step_countdown_964_007c3cb0(Seg47Host& host, Raw plane, float dt) {
    const double prev = plane.at<float>(0x964);
    const float speed = host.plane_flight_forward_speed_007d99c0(plane.sub(0xAB0));
    const float r = host.math_interpolate_clamped_00419010(kSputterX0_00ce3850, 0.0f, kSputterX1_00ce89cc,
                                                            kOne_00d7a24c, speed);
    const float t = static_cast<float>(prev - static_cast<double>(r) * dt);
    plane.at<float>(0x964) = t;
    const Raw game = raw(host.global_pointer(0x00E188A8u));
    if (game.ptr(0x1ED4) == nullptr) return;
    if (!(0.0f > t)) return;
    const bool state6 = plane.at<std::int32_t>(0x900) == 6;
    if (!state6) {
        if (!host.control_mode_vcall_38(plane.sub(0x72C))) return;
        if (plane.at<std::uint8_t>(0xBF8) != 0 && plane.ptr(0xBF4) != nullptr) return;
        const float* const position = static_cast<const float*>(host.entity_pose_world_position_refreshed_00427eb0(plane));
        if (!(kSputterHeight_00ce3918 > position[1])) return;
    }
    alignas(4) std::uint8_t matrix_storage[0x40];
    const Raw m = host.camera_vcall_matrix_114(game.deref(0x1ED4), matrix_storage);
    float p[3] = {m.at<float>(0x30), 0.0f, m.at<float>(0x38)};
    const float q[3] = {m.at<float>(0x30), kSputterProbeY_00ce3930, m.at<float>(0x38)};
    if (!state6 && host.world_segment_blocked_00903bc0(game.deref(0x19CC), q, p)) return;
    plane.at<float>(0x964) = host.random_uniform_00bd2f10(0, kSputterLo_00ced070, kSputterHi_00ce8198);
    p[1] = kOne_00d7a24c;
    void* const effect = plane.deref(0x538).ptr(0x218);
    if (effect != nullptr)
        host.interlocked_increment_ce221c(reinterpret_cast<long*>(static_cast<std::uint8_t*>(effect) + 4));
    void* result = nullptr;
    host.point_effect_create_from_position_008685e0(&result, game.deref(0x19EC), effect, p, 0, 0);
    const Raw handle = raw(result);
    host.ref_ptr_slot_release_00440a30(&result);
    handle.at<std::uint8_t>(9) = 1;
}

// 007C3EA0, __thiscall(ECX = plane), SEH frame. When +DF4h and +9D4h are set:
// desc = 00465080(&local; [[[plane+BF4h]+4]+7Ch], 0.0f) (RET 8; the pushed 1
// stays as the next call's third word); 0077D600([plane+9D4h]; 00E08FA0, desc,
// 1); 0099A600([plane+DF4h]). Always: 0077C470(plane; 2, 0); a stack message
// 0075B430(57h) with the head bytes, +4 = 1, vptr 00CECCB4, routed through
// 0077C2A0(plane; &message, 0, 0).
void plane_issue_land_on_touchdown_007c3ea0(Seg47Host& host, Raw plane) {
    if (plane.ptr(0xDF4) != nullptr && plane.ptr(0x9D4) != nullptr) {
        alignas(4) std::uint8_t descriptor[0x18];
        void* const target = plane.deref(0xBF4).deref(4).ptr(0x7C);
        void* const desc = host.command_target_from_entity_00465080(descriptor, target, 0.0f);
        host.entity_issue_command_0077d600(plane.deref(0x9D4), 0x00E08FA0u, desc, 1);
        host.pilot_bot_retire_finished_tasks_0099a600(plane.deref(0xDF4));
    }
    host.entity_set_state_0077c470(plane, 2, 0);
    alignas(4) std::uint8_t storage[0x20];
    Raw message{storage};
    host.session_message_construct_0075b430(message, 0x57);
    message.at<std::uint16_t>(0x18) = 0;
    message.at<std::uint8_t>(0x1A) = 0;
    message.at<std::uint32_t>(4) = 1;
    message.at<std::uint8_t>(0x1C) = 0;
    message.at<std::uint32_t>(0) = 0x00CECCB4u;
    host.session_route_message_0077c2a0(plane, storage, 0, 0);
}

// 007C7430, __thiscall(ECX = plane). Session mode 2 with +900h not 4/5 ->
// 007C71E0(plane). Then +904h cleared when set; +910h = 0; +908h = 0.0f; when
// +BF8h is clear and +900h == 5, +900h = 4.
void plane_remote_touchdown_to_state4_007c7430(Seg47Host& host, Raw plane) {
    const Raw game = raw(host.global_pointer(0x00E188A8u));
    if (game.at<std::int32_t>(0x1FE4) == 2) {
        const std::int32_t state = plane.at<std::int32_t>(0x900);
        if (state != 4 && state != 5) host.plane_touchdown_007c71e0(plane);
    }
    if (plane.at<std::uint8_t>(0x904) != 0) plane.at<std::uint8_t>(0x904) = 0;
    plane.at<std::uint8_t>(0x910) = 0;
    plane.at<float>(0x908) = 0.0f;
    if (plane.at<std::uint8_t>(0xBF8) == 0 && plane.at<std::int32_t>(0x900) == 5) plane.at<std::int32_t>(0x900) = 4;
}

// 007C7800, __thiscall(ECX = plane; camera), RET 4. The image copies 13 dwords
// of the tuning object from +84h to a stack block and reads its +10h (the
// tuning +94h): camera+3ACh = -(float)(+9ECh * tuning+94h). s = (+9FAh and
// +C24h) ? 1.0f : -1.0f; on camera+428h: when s > 0 and 0.0f >= +2Ch
// (COMISS/JB: NaN skips) 0042C1B0(camera+428h); then +2Ch = s.
void plane_sync_cockpit_camera_gun_shake_007c7800(Seg47Host& host, Raw plane, Raw camera) {
    const Raw tuning = host.ai_tuning_0042e740();
    std::uint32_t block[13];
    std::memcpy(block, tuning.field(0x84), sizeof(block));
    float scale;
    std::memcpy(&scale, &block[4], sizeof(scale));
    const float v = plane.at<float>(0x9EC) * scale;
    camera.at<float>(0x3AC) = -v;
    const float s = (plane.at<std::uint8_t>(0x9FA) != 0 && plane.at<std::uint8_t>(0xC24) != 0) ? kOne_00d7a24c
                                                                                          : kMinusOne_00d7a260;
    const Raw shake = camera.sub(0x428);
    if (s > 0.0f && 0.0f >= shake.at<float>(0x2C)) host.camera_shake_start_0042c1b0(shake);
    shake.at<float>(0x2C) = s;
}

// 007CB9E0, __thiscall(ECX = plane). Session mode 2 -> 007C78A0(plane; 6, 0).
// When +900h != 6: +900h = 6, +C04h = -1.0f, +910h = 0, +904h cleared when
// set, +90Ch = 0.0f, 007C11E0(plane; 0).
void plane_remote_enter_flight_state6_007cb9e0(Seg47Host& host, Raw plane) {
    const Raw game = raw(host.global_pointer(0x00E188A8u));
    if (game.at<std::int32_t>(0x1FE4) == 2) host.plane_set_flight_state_unguarded_007c78a0(plane, 6, 0);
    if (plane.at<std::int32_t>(0x900) == 6) return;
    plane.at<std::int32_t>(0x900) = 6;
    plane.at<float>(0xC04) = kMinusOne_00d7a260;
    plane.at<std::uint8_t>(0x910) = 0;
    if (plane.at<std::uint8_t>(0x904) != 0) plane.at<std::uint8_t>(0x904) = 0;
    plane.at<float>(0x90C) = 0.0f;
    host.plane_flight_state_entered_007c11e0(plane, 0);
}

// 007CC2D0, __thiscall(ECX = control mode at plane+72Ch; arg), RET 4:
// 006E6450(mode; arg), then 007CB7F0(plane).
void plane_control_mode_on_water_contact_007cc2d0(Seg47Host& host, Raw mode, void* arg) {
    host.control_mode_water_contact_006e6450(mode, arg);
    host.plane_on_water_contact_007cb7f0(Raw{mode.p - 0x72C});
}

// 007CD250, __thiscall(ECX = string), EAX = string, SEH frame: a stack
// NativeString {length, data} from 007CCB30(&local); 00BD1A20(string; it)
// (append); the local's data returned to the sized pool with length + 1.
Raw plane_append_flight_state_debug_007cd250(Seg47Host& host, Raw string) {
    alignas(4) std::uint8_t storage[8] = {};
    Raw local{storage};
    void* const built = host.flight_state_debug_text_007ccb30(local);
    host.native_string_append_00bd1a20(string, built);
    if (local.ptr(4) != nullptr) {
        void* const pool = host.sized_pool_00419cc0();
        host.sized_pool_return_block_00bd1510(pool, local.ptr(4), local.at<std::uint32_t>(0) + 1, 1);
    }
    return string;
}

// ============================================================================
// Plane class and entity-creation helpers 007CC9B0-007D1C90
// ============================================================================

// 007CC9B0 PLANE_CREATE create entity, __thiscall(ECX = message), EAX.
// class = 00964790(ECX = (uint16)+1Eh, DL = 1); entity =
// class->vtable[28h]((uint16)+1Ch); the name +C0h (strlen, 0 when null) is
// copied into the NativeString at entity+154h (0041DD40(len, 0), then memcpy
// 00BF7680 of [entity+154h] bytes when its data is set); then
// entity->vtable[88h](0075B500(message; &local)).
void* session_message_plane_create_create_entity_007cc9b0(Seg47Host& host, Raw message) {
    void* const cls = host.entity_class_by_id_00964790(message.at<std::uint16_t>(0x1E), 1);
    const Raw entity = host.entity_class_vcall_create_28(raw(cls), message.at<std::uint16_t>(0x1C));
    const char* const name = static_cast<const char*>(message.ptr(0xC0));
    const std::uint32_t length = name != nullptr ? static_cast<std::uint32_t>(std::strlen(name)) : 0;
    const Raw string = entity.sub(0x154);
    host.native_string_resize_0041dd40(string, length, 0);
    if (string.ptr(4) != nullptr) host.memcpy_00bf7680(string.ptr(4), name, string.at<std::uint32_t>(0));
    alignas(4) std::uint8_t local[0x30];
    void* const descriptor = host.session_message_entity_descriptor_0075b500(message, local);
    host.entity_vcall_apply_create_88(entity, descriptor);
    return entity.p;
}

// 007CD9A0, __thiscall(ECX = plane), AL. For each kind-25h child c: idx =
// c+38Ch; the class vector [plane+538h]+94h {data, size} grows through
// 005471B0(vector; idx + 1) when idx >= size; when [data[idx]+0Dh] is set the
// answer is c->vtable[210h](2Ah, 0) != 0 (NEG/SBB). No such child -> 0.
bool plane_bay_door_rack_holds_bomb_007cd9a0(Seg47Host& host, Raw plane) {
    bool answer = false;
    for_each_child(plane, [&](Raw child) {
        if (!is_rack_child(host, child)) return false;
        const Raw vector = plane.deref(0x538).sub(0x94);
        const std::int32_t idx = child.at<std::int32_t>(0x38C);
        if (idx >= vector.at<std::int32_t>(4)) host.class_vector_grow_005471b0(vector, idx + 1);
        const Raw entry = raw(static_cast<void**>(vector.ptr(0))[idx]);
        if (entry.at<std::uint8_t>(0xD) == 0) return false;
        answer = host.rack_vcall_holds_kind_210(child, 0x2A, 0);
        return true;
    });
    return answer;
}

// 007CFCF0, __thiscall, EAX = 0Fh (the plane class id).
std::int32_t plane_class_slot1c_007cfcf0() { return 0xF; }

// 007D0350, __thiscall(ECX = plane), EAX = +93Ch.
std::uint32_t plane_get_field_93c_007d0350(Raw plane) { return plane.at<std::uint32_t>(0x93C); }

// 007D1C90, __thiscall(ECX = class; arg), RET 4, EAX, SEH frame: operator new
// (E94h) (00BF55BE), memset(0, E94h) before the null test, then
// 007CFD20(memory; arg, class); null -> 0.
void* plane_unit_instance_create_007d1c90(Seg47Host& host, Raw cls, void* arg) {
    void* const memory = host.operator_new_00bf55be(0xE94);
    host.memset_00bf79f0(memory, 0, 0xE94);
    if (memory == nullptr) return nullptr;
    return host.plane_unit_instance_construct_007cfd20(memory, arg, cls);
}

// ============================================================================
// Plane class speed and altitude gates 007C4830, 007C4D90
// ============================================================================

// 007C4830, __thiscall(ECX = class), ST0: (float)(tuning+230h * class+184h)
// (0042E740; one x87 product stored to a float).
float plane_class_stall_range_speed_007c4830(Seg47Host& host, Raw cls) {
    return host.ai_tuning_0042e740().at<float>(0x230) * cls.at<float>(0x184);
}

// 007C4D90, __thiscall(ECX = plane), AL: alt = +9B4h; tuning+354h > alt ->
// 0; alt > tuning+358h -> 0 (FCOMI/JA both: a NaN passes); else 1.
// 0042E740 is fetched for each test.
bool plane_in_paratrooper_drop_altitude_007c4d90(Seg47Host& host, Raw plane) {
    const float alt = plane.at<float>(0x9B4);
    if (host.ai_tuning_0042e740().at<float>(0x354) > alt) return false;
    if (alt > host.ai_tuning_0042e740().at<float>(0x358)) return false;
    return true;
}

// ============================================================================
// The plane engine-effect vector {+0 data, +4 size, +8 capacity} of 44h-byte
// records {RefPtr effect +0, matrix +4..+43h}
// ============================================================================

// 007C9170, __thiscall(ECX = vector; count), RET 4, SEH frame. count =
// max(count, 1); when capacity < count: operator new(count*44h) (00BF55BE);
// each old record is copy-constructed (007C34B0) into a non-null destination;
// then each old record's effect is released (InterlockedDecrement == 0 ->
// vtable[0]()) and zeroed; _free(old); data = new; capacity = count.
void plane_engine_effect_vector_reserve_007c9170(Seg47Host& host, Raw vector, std::int32_t count) {
    if (count < 1) count = 1;
    if (vector.at<std::int32_t>(8) >= count) return;
    auto* const fresh = static_cast<std::uint8_t*>(host.operator_new_00bf55be(static_cast<std::uint32_t>(count) * 0x44));
    auto* const old = static_cast<std::uint8_t*>(vector.ptr(0));
    for (std::int32_t i = 0; i < vector.at<std::int32_t>(4); ++i) {
        std::uint8_t* const destination = fresh + static_cast<std::size_t>(i) * 0x44;
        if (destination != nullptr)
            plane_engine_effect_record_copy_construct_007c34b0(host, Raw{destination},
                                                               Raw{old + static_cast<std::size_t>(i) * 0x44});
    }
    for (std::int32_t i = 0; i < vector.at<std::int32_t>(4); ++i)
        release_ref_holder(host, Raw{old + static_cast<std::size_t>(i) * 0x44});
    host.free_00bf6989(old);
    vector.ptr(0) = fresh;
    vector.at<std::int32_t>(8) = count;
}

// 007C9290, __thiscall(ECX = vector; count), RET 4. count > capacity ->
// 007C9170(count). Growing: each new non-null slot gets effect 0 and the
// identity matrix. Shrinking: size steps down to count releasing each effect
// and zeroing it. Then size = count.
void plane_engine_effect_vector_resize_007c9290(Seg47Host& host, Raw vector, std::int32_t count) {
    if (count > vector.at<std::int32_t>(8)) plane_engine_effect_vector_reserve_007c9170(host, vector, count);
    const std::int32_t size = vector.at<std::int32_t>(4);
    if (size < count) {
        for (std::int32_t i = size; i < count; ++i) {
            std::uint8_t* const slot = static_cast<std::uint8_t*>(vector.ptr(0)) + static_cast<std::size_t>(i) * 0x44;
            if (slot == nullptr) continue;
            Raw record{slot};
            record.at<std::uint32_t>(0) = 0;
            for (int k = 0; k < 16; ++k) record.at<float>(4 + 4 * static_cast<std::size_t>(k)) = (k % 5 == 0) ? kOne_00d7a24c : 0.0f;
        }
    }
    while (count < vector.at<std::int32_t>(4)) {
        vector.at<std::int32_t>(4) -= 1;
        release_ref_holder(host, Raw{static_cast<std::uint8_t*>(vector.ptr(0)) +
                                     static_cast<std::size_t>(vector.at<std::int32_t>(4)) * 0x44});
    }
    vector.at<std::int32_t>(4) = count;
}

// 007CCCA0, __thiscall(ECX = vector; const record*), RET 4, SEH frame: when
// size == capacity, 007C9170(max(capacity*2, 1)); the record is
// copy-constructed (007C34B0) at data + size*44h unless null; size += 1.
void plane_engine_effect_vector_push_back_007ccca0(Seg47Host& host, Raw vector, Raw record) {
    if (vector.at<std::int32_t>(4) == vector.at<std::int32_t>(8)) {
        std::int32_t grown = vector.at<std::int32_t>(8) * 2;
        if (grown <= 1) grown = 1;
        plane_engine_effect_vector_reserve_007c9170(host, vector, grown);
    }
    std::uint8_t* const destination =
        static_cast<std::uint8_t*>(vector.ptr(0)) + static_cast<std::size_t>(vector.at<std::int32_t>(4)) * 0x44;
    if (destination != nullptr) plane_engine_effect_record_copy_construct_007c34b0(host, Raw{destination}, record);
    vector.at<std::int32_t>(4) += 1;
}

// 007CD520, __thiscall(ECX = component), EAX, SEH frame: vptr 00D05E94;
// +8/+0Ch/+10h = 0; 007DDDE0(+18h); 007DDE00(+24h); 007C4470 on +48h, +74h,
// +A0h and +CCh; +F8h/+FCh/+100h = 0. +4 and +14h are not written.
Raw plane_fx_component_construct_007cd520(Seg47Host& host, Raw component) {
    component.at<std::uint32_t>(0) = 0x00D05E94u;
    component.at<std::uint32_t>(8) = 0;
    component.at<std::uint32_t>(0xC) = 0;
    component.at<std::uint32_t>(0x10) = 0;
    host.vector3_fill_one_007ddde0(component.sub(0x18));
    host.vector3_construct_007dde00(component.sub(0x24));
    effect_vector_construct_007c4470(host, component.sub(0x48));
    effect_vector_construct_007c4470(host, component.sub(0x74));
    effect_vector_construct_007c4470(host, component.sub(0xA0));
    effect_vector_construct_007c4470(host, component.sub(0xCC));
    component.at<std::uint32_t>(0xF8) = 0;
    component.at<std::uint32_t>(0xFC) = 0;
    component.at<std::uint32_t>(0x100) = 0;
    return component;
}

// 007CDC70, __thiscall(ECX = plane): the per-slot step record at plane +
// (uint16)[00E0B6CCh]*8 + 9C0h (the slot word is re-read before each store):
//   +9C1h = [plane+DECh] set and (channel +60h disabled or its value == 1.0f,
//           UCOMISS: NaN -> 0); +9C0h = 007B9140(plane; 0);
//   +9C2h = plane+520h; +9C4h = 007BCC20(plane); +9C3h = 007CD9A0(plane).
void plane_publish_step_record_007cdc70(Seg47Host& host, Raw plane) {
    const auto slot = [&]() {
        return static_cast<std::size_t>(*static_cast<const std::uint16_t*>(host.global_address(0x00E0B6CCu))) * 8;
    };
    bool doors = false;
    const Raw block = plane.deref(0xDEC);
    if (block) doors = block.at<std::uint8_t>(0x60) == 0 || block.at<float>(0x64) == kOne_00d7a24c;
    plane.at<std::uint8_t>(slot() + 0x9C1) = doors ? 1 : 0;
    std::uint8_t* const first = plane.field(slot() + 0x9C0);
    *first = host.plane_query_007b9140(plane, 0) ? 1 : 0;
    plane.at<std::uint8_t>(slot() + 0x9C2) = plane.at<std::uint8_t>(0x520);
    const Raw wait = plane.sub(slot() + 0x9C0);
    wait.at<float>(4) = plane_formation_wait_value_007bcc20(host, plane);
    const Raw bay = plane.sub(slot() + 0x9C0);
    bay.at<std::uint8_t>(3) = plane_bay_door_rack_holds_bomb_007cd9a0(host, plane) ? 1 : 0;
}

}  // namespace bsp::cc13_seg47

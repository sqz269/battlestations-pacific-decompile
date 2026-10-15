// Segment 69, reconstruction pass (packet cc13-seg69). See include/bsp/cc13_seg69.hpp and
// docs/CC13_SEG69.md. Every routine names its address, its original ABI and what was
// simplified.

#include "bsp/cc13_seg69.hpp"

#include <cstring>

namespace bsp::cc13_seg69 {
namespace {

Raw raw(void* p) { return Raw{static_cast<std::uint8_t*>(p)}; }

// A std::list / std::map object as the image lays it out: {allocator byte, head, size}.
struct ListObject {
    std::uint32_t allocator;
    void* head;
    std::uint32_t size;
};

// A std::vector object as the image lays it out: {allocator, first, last, end}.
struct VectorObject {
    std::uint32_t allocator;
    void** first;
    void** last;
    void** end;
};

void* node_next(void* node) { return raw(node).ptr(0x0); }
void* node_value(void* node) { return raw(node).ptr(0x8); }

// The inlined ~NativeStr: (data, length + 1) back to the sized pool when data is set.
void release(Seg69Host& host, NativeStr& s)
{
    if (s.data != nullptr) host.pool_release_00419cc0_00bd1510(s.data, s.length + 1);
}

// The inlined NativeStr(literal): 0041DD40(length, 1) then memcpy(data, text, length + 1).
void construct_literal(Seg69Host& host, NativeStr& s, const char* text, std::uint32_t length)
{
    s.length = 0;
    s.data = nullptr;
    host.native_string_resize_0041dd40(&s, length, true);
    if (s.data != nullptr) std::memcpy(s.data, text, s.length + 1);
}

// The leader-point shape every command body inlines: 00F87574 for an empty group, else the
// first member's +FCh after 00414DB0 when its +C8h byte is clear. The checked-iterator test
// (first node == head) calls 00BF6713 and carries on, as the image does.
const float* group_leader_point(Seg69Host& host, Raw group)
{
    if (group.at<std::int32_t>(kGroupMemberCount) == 0) {
        return &host.image_global(kZeroVector).at<float>(0);
    }
    void* head = group.ptr(kGroupMembersHead);
    void* first = node_next(head);
    if (first == head) host.invalid_parameter_00bf6713();
    Raw entity = raw(node_value(first));
    if (entity.at<std::uint8_t>(kEntityPoseFresh) == 0) host.entity_pose_refresh_00414db0(entity.p);
    return &entity.at<float>(kEntityWorldPosition);
}

}  // namespace

// 009FE130, ABI: __fastcall(ECX = unit) RET, float in ST0. No callers found.
// Kind 6 (vtable +5Ch): [[unit+538h]+500h]; kind 18h: [[unit+35Ch]+188h]; else 0.
float unit_ship_or_squadron_class_value_009fe130(Seg69Host& host, Raw unit)
{
    if (host.unit_is_kind_5c(unit.p, 6)) return raw(unit.ptr(0x538)).at<float>(0x500);
    if (host.unit_is_kind_5c(unit.p, 0x18)) return raw(unit.ptr(0x35C)).at<float>(0x188);
    return 0.0f;
}

// 009FE170, ABI: __fastcall(ECX = object) RET, float in ST0. No callers found.
// Type tests through vtable +18h (not +5Ch): type 6 answers the float at +500h, type 0Fh the
// float at +188h, else 0.
float class_ship_or_plane_value_009fe170(Seg69Host& host, Raw object)
{
    if (host.object_is_type_18(object.p, 6)) return object.at<float>(0x500);
    if (host.object_is_type_18(object.p, 0xF)) return object.at<float>(0x188);
    return 0.0f;
}

// 009FE1B0, ABI: __fastcall(ECX = unit, EDX = arg) RET. A squadron (kind 18h) is replaced by
// its +3D0h plane; a plane (kind 0Fh) stands as itself; anything else answers 0. A non-null
// subject that passes 007B9140(ECX = subject)(arg) answers its +C54h count.
std::int32_t unit_droppable_ordnance_count_009fe1b0(Seg69Host& host, Raw unit, std::uint32_t arg)
{
    void* subject = unit.p;
    if (host.unit_is_kind_5c(unit.p, 0x18)) {
        subject = unit.ptr(0x3D0);
    } else if (!host.unit_is_kind_5c(unit.p, 0xF)) {
        return 0;
    }
    if (subject != nullptr && host.can_drop_ordnance_007b9140(subject, arg)) {
        return raw(subject).at<std::int32_t>(0xC54);
    }
    return 0;
}

// 009FFE00, ABI: __fastcall(ECX = unit) RET, AL. True when byte +5Ch is set and bytes +5Dh,
// +60h and +5Eh are clear (tested in that order), unless the unit is a squadron (kind 18h)
// whose [+3D0h]+72Ch sub-object answers false from its vtable +38h.
bool unit_is_available_for_ai_009ffe00(Seg69Host& host, Raw unit)
{
    if (unit.at<std::uint8_t>(0x5C) == 0 || unit.at<std::uint8_t>(0x5D) != 0 ||
        unit.at<std::uint8_t>(0x60) != 0 || unit.at<std::uint8_t>(0x5E) != 0) {
        return false;
    }
    if (host.unit_is_kind_5c(unit.p, 0x18)) {
        Raw plane = raw(unit.ptr(0x3D0));
        if (!host.object_virtual_38(plane.p + 0x72C)) return false;
    }
    return true;
}

// 009FFF20, ABI: __fastcall(ECX = unit) RET. Kind 5: [unit+538h]; kind 18h: [unit+35Ch];
// else null.
void* unit_class_data_kind5_or_18_009fff20(Seg69Host& host, Raw unit)
{
    if (host.unit_is_kind_5c(unit.p, 5)) return unit.ptr(0x538);
    if (host.unit_is_kind_5c(unit.p, 0x18)) return unit.ptr(0x35C);
    return nullptr;
}

// 00A00020, ABI: __thiscall(ECX = record)(subject, value10, float class_weight, value1c,
// float random_factor, byte flag, const float* point), RET 1Ch, answers the record.
// +0 subject, +4/+8 point[0]/point[1], +14h weight, +0Ch flag, +10h, +1Ch. A random_factor
// below 0 (COMISS 0 against it, JBE stores when 0 <= x, so a NaN is stored) is replaced by
// 00BD2F10(stream 1, tuning+50h, tuning+54h); 00A371A0 is called twice, +54h read first.
// This draw moves the process-wide RNG.
Raw ai_candidate_record_construct_00a00020(Seg69Host& host, Raw record, void* subject,
    std::uint32_t value10, float class_weight, std::uint32_t value1c, float random_factor,
    std::uint8_t flag, const float* point)
{
    record.ptr(0x0) = subject;
    record.at<float>(0x4) = point[0];
    record.at<float>(0x8) = point[1];
    record.at<float>(0x14) = class_weight;
    record.at<std::uint8_t>(0xC) = flag;
    record.at<std::uint32_t>(0x10) = value10;
    record.at<std::uint32_t>(0x1C) = value1c;
    if (0.0f > random_factor) {
        const float high = host.active_mode_tuning_00a371a0().at<float>(kTuningRandomHigh);
        const float low = host.active_mode_tuning_00a371a0().at<float>(kTuningRandomLow);
        record.at<float>(0x18) = host.random_uniform_00bd2f10(1, low, high);
        return record;
    }
    record.at<float>(0x18) = random_factor;
    return record;
}

// 00A02010, ABI: __fastcall(ECX = object) RET. [object+5Ch], or the empty name 00F8A728.
const char* ai_name_or_empty_00a02010(Seg69Host& host, Raw object)
{
    const char* name = static_cast<const char*>(object.ptr(0x5C));
    if (name == nullptr) name = reinterpret_cast<const char*>(host.image_global(kEmptyName).p);
    return name;
}

// 00A038C0, ABI: __cdecl() RET. map::clear on the forced-target-weight map 00F8A734:
// 00A01340(ECX = map)(root), then head->parent = head, size = 0, head->left = head,
// head->right = head, in that order.
void ai_forced_target_weight_clear_00a038c0(Seg69Host& host)
{
    Raw map = host.image_global(kForcedTargetWeightMap);
    host.tree_erase_00a01340(map.p, raw(map.ptr(0x4)).ptr(0x4));
    raw(map.ptr(0x4)).ptr(0x4) = map.ptr(0x4);
    map.at<std::uint32_t>(0x8) = 0;
    raw(map.ptr(0x4)).ptr(0x0) = map.ptr(0x4);
    raw(map.ptr(0x4)).ptr(0x8) = map.ptr(0x4);
}

// 00A03A60, ABI: __cdecl() RET. Clears the global hint-weight map 00F8A740 as 00A038C0 does
// (through 0096BD60), then the eight per-party maps 00F8A750 + i*0Ch with the erase loop
// inlined: from the root, while its +15h isnil byte is clear, 0096BD60(ECX = map)(right),
// step to the left child and free the old node through 00BF65AC; then reset head and size.
void ai_hint_weight_maps_clear_00a03a60(Seg69Host& host)
{
    Raw global = host.image_global(kHintWeightGlobalMap);
    host.tree_erase_0096bd60(global.p, raw(global.ptr(0x4)).ptr(0x4));
    raw(global.ptr(0x4)).ptr(0x4) = global.ptr(0x4);
    global.at<std::uint32_t>(0x8) = 0;
    raw(global.ptr(0x4)).ptr(0x0) = global.ptr(0x4);
    raw(global.ptr(0x4)).ptr(0x8) = global.ptr(0x4);
    for (std::uint32_t address = kHintWeightPartyMaps; address < kHintWeightPartyMapsEnd;
         address += 0xC) {
        Raw map = host.image_global(address);
        void* node = raw(map.ptr(0x4)).ptr(0x4);
        while (raw(node).at<std::uint8_t>(0x15) == 0) {
            host.tree_erase_0096bd60(map.p, raw(node).ptr(0x8));
            void* left = raw(node).ptr(0x0);
            host.operator_delete_00bf65ac(node);
            node = left;
        }
        raw(map.ptr(0x4)).ptr(0x4) = map.ptr(0x4);
        map.at<std::uint32_t>(0x8) = 0;
        raw(map.ptr(0x4)).ptr(0x0) = map.ptr(0x4);
        raw(map.ptr(0x4)).ptr(0x8) = map.ptr(0x4);
    }
}

// 00A04050, ABI: __fastcall(ECX = out NativeStr*, EDX = vehicle class)(int equipment), RET 4,
// answers out. out = class name (+54h) followed, when equipment is non-zero and
// 00443490(ECX = [[[009552E0(class, equipment)+4]+8]+4], DL = 0) answers a device class, by
// " - " (00D21D00) and that device's +54h name. The temporaries are released in the image's
// order (name, then the empty right side or " - "+name, " - ", device name).
NativeStr* ai_debug_describe_class_00a04050(Seg69Host& host, NativeStr* out, Raw vehicle_class,
    int equipment_index)
{
    NativeStr device_name{0, nullptr};
    NativeStr separator{0, nullptr};
    NativeStr right{0, nullptr};
    bool have_device = false;
    if (equipment_index != 0) {
        Raw equipment = raw(host.class_equipment_009552e0(vehicle_class.p, equipment_index));
        void* device = raw(raw(equipment.ptr(0x4)).ptr(0x8)).ptr(0x4);
        Raw resolved = raw(host.device_class_resolve_00443490(device, false));
        if (resolved.p != nullptr) {
            host.native_string_assign_0041e870(&device_name, static_cast<const char*>(resolved.ptr(0x54)));
            host.native_string_assign_0041e870(&separator, " - ");
            host.native_string_concat_004261a0(&separator, &right, &device_name);
            have_device = true;
        }
    }
    NativeStr class_name{0, nullptr};
    host.native_string_assign_0041e870(&class_name, static_cast<const char*>(vehicle_class.ptr(0x54)));
    // Without a device the right side is the zeroed device_name slot itself (ESI = &A).
    host.native_string_concat_004261a0(&class_name, out, have_device ? &right : &device_name);
    release(host, class_name);
    if (have_device) {
        release(host, right);
        release(host, separator);
        release(host, device_name);
    }
    return out;
}

// 00A07EB0, ABI: __fastcall(ECX = key0, EDX = key1)(attacker, key3, float weight), RET 0Ch.
// The 16-byte key {key0, key1, attacker, key3} is looked up in the forced-target-weight map
// 00F8A734 (00A03B90); a found entry is erased (00A04CB0) after the checked-iterator test that
// the iterator belongs to the map (00BF6713 otherwise). The stored value is
// weight / 009FDF30(attacker->vtable[+1Ch]()), divided in double (FLD float, FSTP double,
// FDIVR double) and rounded to float into 00A079B0's cell.
void ai_forced_target_weight_set_00a07eb0(Seg69Host& host, std::uint32_t key0, std::uint32_t key1,
    void* attacker, std::uint32_t key3, float weight)
{
    std::uint32_t key[4] = {key0, key1, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(attacker)), key3};
    Raw map = host.image_global(kForcedTargetWeightMap);
    void* iterator[2] = {nullptr, nullptr};
    host.forced_weight_find_00a03b90(map.p, iterator, key);
    void* head = map.ptr(0x4);
    if (iterator[0] == nullptr || iterator[0] != map.p) host.invalid_parameter_00bf6713();
    if (iterator[1] != head) {
        void* erased[2] = {nullptr, nullptr};
        host.forced_weight_erase_00a04cb0(map.p, erased, iterator[0], iterator[1]);
    }
    const double numerator = static_cast<double>(weight);
    const double divisor = static_cast<double>(host.class_weight_009fdf30(host.object_class_1c(attacker)));
    const float value = static_cast<float>(numerator / divisor);
    *host.forced_weight_subscript_00a079b0(map.p, key) = value;
}

// 00A07F60, ABI: __fastcall(ECX = hint)(float weight), RET 4. The Lua SetHintWeight global
// arm: 00975C40(ECX = 00F8A740)(&hint) answers the cell; the weight is stored there.
void ai_hint_weight_set_global_00a07f60(Seg69Host& host, std::uint32_t hint, float weight)
{
    const std::uint32_t key = hint;
    *host.hint_weight_subscript_00975c40(host.image_global(kHintWeightGlobalMap).p, &key) = weight;
}

// 00A07F80, ABI: __fastcall(ECX = hint, EDX = party)(float weight), RET 4. The same store in
// the per-party map 00F8A750 + party*0Ch. No bound check on party, as in the image.
void ai_hint_weight_set_for_party_00a07f80(Seg69Host& host, std::uint32_t hint, int party,
    float weight)
{
    const std::uint32_t key = hint;
    const std::uint32_t map = kHintWeightPartyMaps + static_cast<std::uint32_t>(party) * 0xC;
    *host.hint_weight_subscript_00975c40(host.image_global(map).p, &key) = weight;
}

// 00A07FB0, ABI: __fastcall(ECX = group) RET, float in ST0 (rounded through a float slot).
// A local record vector, 00A07E40(ECX = group, EDX = &vector)(0), 00A07C10(ECX = &vector) for
// the slowest record, then operator delete of the vector's first pointer when set. Ghidra
// reported the delete as unreachable; the listing shows it is reached whenever 00A07E40 filled
// the vector.
float ai_group_min_speed_00a07fb0(Seg69Host& host, void* group)
{
    VectorObject records{0, nullptr, nullptr, nullptr};
    host.group_entity_records_00a07e40(group, &records, 0);
    const float speed = host.records_min_speed_00a07c10(&records);
    if (records.first != nullptr) host.operator_delete_00bf65ac(records.first);
    return speed;
}

// 00A08020, ABI: __fastcall(ECX = owner, EDX = out record vector)(filter, int kind), RET 8.
// 00A08420 is the only caller. A local list of (key, unit list) pairs is filled by
// 0066E590(ECX = owner, EDX = &list)(-1). For every unit of every pair: with a filter, the
// unit's world pose is refreshed inline when its +C8h byte is clear (00414DB0 on the parent
// +3Ch when set, 00413920 local*parent into a temporary, else the local +74h matrix itself;
// 16 floats to +CCh, +C8h = 1, +10Ch = 0) and a unit the filter's vtable +0h accepts at +FCh
// is skipped. A unit whose +340h byte is clear is skipped. Otherwise 0084D560 lists its
// supply classes; each class 00964790(index, 1) that passes the kind filter is pushed as a
// 20h record {class, 0, 0, 0, class+134h, 009FDF30 weight, 00BD2F10(1, tuning+50h, +54h), 0}
// through 00A078F0. The class list is freed inline; the pair list goes to 006654C0 and its
// head to operator delete. The vacuous checked-iterator tests (container null or foreign)
// cannot fire on a local list and are omitted.
void ai_collect_spawn_point_class_records_00a08020(Seg69Host& host, void* owner, void* out_vector,
    void* filter, int kind_filter)
{
    ListObject pairs{0, nullptr, 0};
    pairs.head = host.list_buy_head_node_006599c0(&pairs);
    pairs.size = 0;
    host.collect_party_units_0066e590(owner, &pairs, -1);
    for (void* pair = node_next(pairs.head); pair != pairs.head; pair = node_next(pair)) {
        void* units_head = raw(pair).ptr(0x10);
        for (void* node = node_next(units_head); node != raw(pair).ptr(0x10); ) {
            Raw unit = raw(node_value(node));
            bool skip = false;
            if (filter != nullptr) {
                if (unit.at<std::uint8_t>(kEntityPoseFresh) == 0) {
                    if (unit.ptr(kEntityParent) != nullptr) {
                        host.entity_pose_refresh_00414db0(unit.ptr(kEntityParent));
                    }
                    float product[16];
                    const float* source = &unit.at<float>(kEntityLocalMatrix);
                    if (unit.ptr(kEntityParent) != nullptr) {
                        source = host.matrix_multiply_00413920(&unit.at<float>(kEntityLocalMatrix),
                            product, &raw(unit.ptr(kEntityParent)).at<float>(kEntityWorldMatrix));
                    }
                    for (int i = 0; i < 16; ++i) unit.at<float>(kEntityWorldMatrix + 4 * i) = source[i];
                    unit.at<std::uint8_t>(kEntityPoseFresh) = 1;
                    unit.at<std::uint8_t>(kEntityPoseFlag10C) = 0;
                }
                skip = host.filter_rejects_0(filter, &unit.at<float>(kEntityWorldPosition));
            }
            if (!skip && unit.at<std::uint8_t>(0x340) != 0) {
                ListObject classes{0, nullptr, 0};
                host.support_site_class_list_0084d560(unit.p, &classes, owner, 0);
                for (void* entry = node_next(classes.head); entry != classes.head;
                     entry = node_next(entry)) {
                    const int index = static_cast<int>(reinterpret_cast<std::uintptr_t>(node_value(entry)));
                    Raw cls = raw(host.vehicle_class_by_index_00964790(index, true));
                    bool accept = true;
                    if (kind_filter >= 0) {
                        switch (kind_filter) {
                        case 0: accept = host.object_is_type_18(cls.p, 7); break;
                        case 1:
                        case 2:
                            accept = host.object_is_type_18(cls.p, 0xA) &&
                                (cls.at<std::uint8_t>(0x808) != 0 ? kind_filter == 2 : kind_filter == 1);
                            break;
                        case 3: accept = host.object_is_type_18(cls.p, 0xD); break;
                        case 4: accept = host.object_is_type_18(cls.p, 0xE); break;
                        case 5: accept = host.object_is_type_18(cls.p, 8); break;
                        case 6: accept = host.object_is_type_18(cls.p, 0xF); break;
                        default: break;  // above 6: every class
                        }
                    }
                    if (!accept) continue;
                    CandidateRecord record{};
                    record.class_weight = host.class_weight_009fdf30(host.object_class_1c(cls.p));
                    record.subject = cls.p;
                    record.point_x = 0.0f;
                    record.point_z = 0.0f;
                    record.flag = 0;
                    record.value10 = cls.at<std::uint32_t>(0x134);
                    record.value1c = 0;
                    const float high = host.active_mode_tuning_00a371a0().at<float>(kTuningRandomHigh);
                    const float low = host.active_mode_tuning_00a371a0().at<float>(kTuningRandomLow);
                    record.random_factor = host.random_uniform_00bd2f10(1, low, high);
                    host.record_vector_push_back_00a078f0(out_vector, &record);
                }
                // The inlined list destructor at 00A0836C-00A083B2.
                void* entry = node_next(classes.head);
                raw(classes.head).ptr(0x0) = classes.head;
                raw(classes.head).ptr(0x4) = classes.head;
                classes.size = 0;
                while (entry != classes.head) {
                    void* next = node_next(entry);
                    host.operator_delete_00bf65ac(entry);
                    entry = next;
                }
                host.operator_delete_00bf65ac(classes.head);
                classes.head = nullptr;
            }
            node = node_next(node);
        }
    }
    host.list_clear_nested_006654c0(&pairs);
    host.operator_delete_00bf65ac(pairs.head);
}

// 00A08420, ABI: __fastcall(ECX = owner, DL = spawn_points)(byte team_entities, out vector,
// filter, int kind), RET 10h. 00A08020(owner, out)(filter, kind) when DL is set, then
// 00A07D40(owner, out)(filter, 0) when the stack byte is set.
void ai_collect_candidate_records_00a08420(Seg69Host& host, void* owner, bool spawn_points,
    bool team_entities, void* out_vector, void* filter, int kind_filter)
{
    if (spawn_points) ai_collect_spawn_point_class_records_00a08020(host, owner, out_vector, filter, kind_filter);
    if (team_entities) host.team_entity_records_00a07d40(owner, out_vector, filter, 0);
}

// 00A0C1F0, ABI: __cdecl() RET. Unreferenced developer routine. Collects 00964790(i, 1) for
// every registry index i whose cache slot ([reg+4])[[reg+10h+4i]] is non-null (the cache is
// first grown through 00437BD0(id + 1) when the id is not below [reg+8]); note the index
// passed is the loop counter, not the id. Then for each class A, for level 0..00951F10(A)
// inclusive, for each class B: 00A08460(A, level)(B, 0), and (B, 1) when B's vtable
// +18h(1Ch) answers true. Every weight is discarded (FSTP ST0). The vector's storage is
// deleted at the end; with no class collected the routine returns without the delete.
void ai_debug_target_weight_all_classes_00a0c1f0(Seg69Host& host)
{
    VectorObject classes{0, nullptr, nullptr, nullptr};
    Raw registry = host.vehicle_class_registry_00437f50();
    for (int i = 0; i < registry.at<std::int32_t>(0x8); ++i) {
        const int id = registry.at<std::int32_t>(0x10 + 4 * i);
        if (id >= registry.at<std::int32_t>(0x8)) host.registry_cache_resize_00437bd0(registry.p + 0x4, id + 1);
        if (static_cast<void**>(registry.ptr(0x4))[id] != nullptr) {
            void* cls = host.vehicle_class_by_index_00964790(i, true);
            host.pointer_vector_push_back_00947fd0(&classes, &cls);
        }
    }
    for (std::uint32_t s = 0;; ++s) {
        if (classes.first == nullptr) return;
        if (s >= static_cast<std::uint32_t>(classes.last - classes.first)) break;
        void* attacker = classes.first[s];
        if (host.class_upgrade_levels_00951f10(attacker) >= 0) {
            int level = 0;
            do {
                for (std::uint32_t t = 0; classes.first != nullptr &&
                     t < static_cast<std::uint32_t>(classes.last - classes.first); ++t) {
                    void* target = classes.first[t];
                    (void)host.target_weight_00a08460(attacker, level, target, 0);
                    if (host.object_is_type_18(target, 0x1C)) {
                        (void)host.target_weight_00a08460(attacker, level, target, 1);
                    }
                }
                ++level;
            } while (level <= host.class_upgrade_levels_00951f10(attacker));
        }
    }
    host.operator_delete_00bf65ac(classes.first);
}

// 00A0FA70, ABI: __fastcall(ECX = argument text) RET. Unreferenced developer command. The text
// is copied unbounded into a 100h-byte stack buffer (the 10000h-byte report buffer follows
// it), '(' ')' ',' become spaces, and two space-separated tokens are cut out. The routine
// returns silently when the text is all spaces, when the first token runs to the end, or when
// nothing follows it; the second token may run to the end. atol on both, 00A2C950 on both ids;
// with both groups: fopen("aiGroupsTargetValue.txt", "wt"), [00E0E344] is saved and set to
// the first group's +5634h id clamped at 0, 00A0F970(ECX = g1, EDX = g2)(0, -1.0f, report, 0,
// 1.0f) writes the report (its ST0 discarded), [00E0E344] is restored, then fprintf("%s") and
// fclose. The file pointer is not checked, as in the image.
void ai_debug_write_groups_target_value_00a0fa70(Seg69Host& host, const char* arguments)
{
    char frame[0x10100];  // the image's stack frame (00BF9410 probe for 10100h bytes)
    char* text = frame;
    char* report = frame + 0x100;
    for (std::size_t k = 0;; ++k) {  // the inlined unbounded strcpy at 00A0FA80
        text[k] = arguments[k];
        if (arguments[k] == '\0') break;
    }
    const int length = static_cast<int>(std::strlen(text));
    for (int i = 0; i < length; ++i) {
        if (text[i] == '(' || text[i] == ')' || text[i] == ',') text[i] = ' ';
    }
    if (length <= 0) return;
    int i = 0;
    while (text[i] == ' ') {
        if (++i >= length) return;
    }
    const char* first = text + i;
    while (text[i] != ' ') {
        if (++i >= length) return;
    }
    text[i] = '\0';
    if (++i >= length) return;
    while (text[i] == ' ') {
        if (++i >= length) return;
    }
    const char* second = text + i;
    for (; i < length; ++i) {
        if (text[i] == ' ') {
            text[i] = '\0';
            break;
        }
    }
    const long first_id = host.atol_00bf8417(first);
    const long second_id = host.atol_00bf8417(second);
    void* g1 = host.group_by_id_00a2c950(static_cast<int>(first_id));
    void* g2 = host.group_by_id_00a2c950(static_cast<int>(second_id));
    if (g1 == nullptr || g2 == nullptr) return;
    void* file = host.fopen_00bf838e("aiGroupsTargetValue.txt", "wt");
    std::int32_t& party = host.image_global(kDebugPartyOverride).at<std::int32_t>(0);
    const std::int32_t saved = party;
    const std::int32_t id = raw(g1).at<std::int32_t>(kGroupId);
    party = id < 0 ? 0 : id;
    (void)host.group_target_value_00a0f970(g1, g2, 0, kTargetValueNoLimit, report, 0, 1.0f);
    party = saved;
    host.fprintf_00bf80fd(file, "%s", report);
    host.fclose_00bf8081(file);
}

// 00A0FDE0, 00A108F0, 00A10A20 and 00A10B40 (byte-identical but for the EH handler), ABI:
// __fastcall(ECX = command) RET. The destructor bodies of the four classes that carry the
// observer sub-object at +8h: 00695870(ECX = command ? command + 8 : 0), then the base
// command vtable 00D22968 is written back.
void ai_command_destruct_observer_body(Seg69Host& host, Raw command)
{
    host.callback_owner_destroy_00695870(command.p != nullptr ? command.p + 0x8 : nullptr);
    command.at<std::uint32_t>(0x0) = kCommandBaseVtable;
}

// 00A0FF60, ABI: __thiscall(ECX = command)(lua object), RET 4. 00B66790(ECX = object)
// ("commandType", 00E0E308[command->vtable[+4h]()]).
void ai_command_describe_type_00a0ff60(Seg69Host& host, Raw command, void* lua_object)
{
    const int type = host.command_type_4(command.p);
    const char* name = static_cast<const char**>(static_cast<void*>(host.image_global(kCommandTypeNames).p))[type];
    host.lua_set_cstring_00b66790(lua_object, "commandType", name);
}

// 00A10240, ABI: __thiscall(ECX = command)(group, const float* point), RET 8, answers command.
// MOVETO: +4h group, vtable 00D22ABC, +8h..+10h the point (FLD/FSTP float copies).
Raw ai_command_moveto_construct_00a10240(Raw command, void* group, const float* point)
{
    command.ptr(0x4) = group;
    command.at<std::uint32_t>(0x0) = kMoveToVtable;
    command.at<float>(0x8) = point[0];
    command.at<float>(0xC) = point[1];
    command.at<float>(0x10) = point[2];
    return command;
}

// 00A102D0, ABI as 00A10240. CAUTIOUSMOVE: +4h group, +8h..+10h point, the approach base at
// +14h (vtable 00D22A08 then 00D22AE4), byte +18h = 0, route counter +1Ch = 4, vtable 00D22AEC.
Raw ai_command_cautious_move_construct_00a102d0(Raw command, void* group, const float* point)
{
    command.ptr(0x4) = group;
    command.at<float>(0x8) = point[0];
    command.at<float>(0xC) = point[1];
    command.at<float>(0x10) = point[2];
    command.at<std::uint32_t>(0x14) = kCautiousApproachBaseVtable;
    command.at<std::uint8_t>(0x18) = 0;
    command.at<std::int32_t>(0x1C) = kCautiousRouteCounterInitial;
    command.at<std::uint32_t>(0x0) = kCautiousMoveVtable;
    command.at<std::uint32_t>(0x14) = kCautiousMoveApproachVtable;
    return command;
}

// 00A10370, ABI as 00A10240. REGROUPINGMOVE: as MOVETO with vtable 00D22B14.
Raw ai_command_regrouping_move_construct_00a10370(Raw command, void* group, const float* point)
{
    command.ptr(0x4) = group;
    command.at<std::uint32_t>(0x0) = kRegroupingMoveVtable;
    command.at<float>(0x8) = point[0];
    command.at<float>(0xC) = point[1];
    command.at<float>(0x10) = point[2];
    return command;
}

// 00A10710, ABI: __thiscall(ECX = command)(group, target group), RET 8, answers command.
// +4h group; the observer sub-object at +8h is built as the base (vtable 00CE3CD4, +0Ch..+14h
// zero, byte +18h zero), then the ATTACK vtables 00D22B7C / 00D22B64 go in, +1Ch = target,
// and 00694A60(ECX = target, EDX = command + 8) registers the pair.
Raw ai_command_attack_construct_base_00a10710(Seg69Host& host, Raw command, void* group,
    void* target)
{
    command.ptr(0x4) = group;
    command.at<std::uint32_t>(0x8) = kObserverBaseVtable;
    command.at<std::uint32_t>(0xC) = 0;
    command.at<std::uint32_t>(0x10) = 0;
    command.at<std::uint32_t>(0x14) = 0;
    command.at<std::uint8_t>(0x18) = 0;
    command.at<std::uint32_t>(0x0) = kAttackVtable;
    command.at<std::uint32_t>(0x8) = kAttackObserverVtable;
    command.ptr(0x1C) = target;
    host.observer_register_pair_00694a60(target, command.p + 0x8);
    return command;
}

// 00A10890, ABI as 00A10710. MOVETOATTACK: the base, then vtables 00D22BBC / 00D22BA4.
Raw ai_command_movetoattack_construct_00a10890(Seg69Host& host, Raw command, void* group,
    void* target)
{
    ai_command_attack_construct_base_00a10710(host, command, group, target);
    command.at<std::uint32_t>(0x0) = kMoveToAttackVtable;
    command.at<std::uint32_t>(0x8) = kMoveToAttackObserverVtable;
    return command;
}

// 00A109B0, ABI as 00A10710 (2Ch-byte object). CAUTIOUSATTACK: the base, the approach base at
// +20h (00D22A08), byte +24h = 0, route counter +28h = 4, vtables 00D22C04 / 00D22BEC, then
// +20h = 00D22BE4.
Raw ai_command_cautious_attack_construct_00a109b0(Seg69Host& host, Raw command, void* group,
    void* target)
{
    ai_command_attack_construct_base_00a10710(host, command, group, target);
    command.at<std::uint32_t>(0x20) = kCautiousApproachBaseVtable;
    command.at<std::uint8_t>(0x24) = 0;
    command.at<std::int32_t>(0x28) = kCautiousRouteCounterInitial;
    command.at<std::uint32_t>(0x0) = kCautiousAttackVtable;
    command.at<std::uint32_t>(0x8) = kCautiousAttackObserverVtable;
    command.at<std::uint32_t>(0x20) = kCautiousAttackApproachVtable;
    return command;
}

// 00A10AE0, ABI as 00A10710. CLOSEATTACK: the base, then vtables 00D22C44 / 00D22C2C.
Raw ai_command_close_attack_construct_00a10ae0(Seg69Host& host, Raw command, void* group,
    void* target)
{
    ai_command_attack_construct_base_00a10710(host, command, group, target);
    command.at<std::uint32_t>(0x0) = kCloseAttackVtable;
    command.at<std::uint32_t>(0x8) = kCloseAttackObserverVtable;
    return command;
}

// The shape the debug-draw bodies inline: for a non-empty group, the first member's pose is
// refreshed (00414DB0) when its +C8h byte is clear. Whatever was drawn with it is stripped.
void ai_group_refresh_leader_pose(Seg69Host& host, Raw group)
{
    if (group.at<std::int32_t>(kGroupMemberCount) == 0) return;
    void* head = group.ptr(kGroupMembersHead);
    void* first = node_next(head);
    if (first == head) host.invalid_parameter_00bf6713();
    Raw entity = raw(node_value(first));
    if (entity.at<std::uint8_t>(kEntityPoseFresh) == 0) host.entity_pose_refresh_00414db0(entity.p);
}

// 00A12540, ABI: __fastcall(ECX = command) RET. MOVETO debug draw, stripped: the owner group's
// (+4h) leader pose refresh only (the 00414DB0 call is a tail JMP).
void ai_command_moveto_debug_draw_00a12540(Seg69Host& host, Raw command)
{
    ai_group_refresh_leader_pose(host, raw(command.ptr(0x4)));
}

// 00A12580, ABI: __thiscall(ECX = approach base)(group, unused), RET 8. With the approach's
// +4h byte set: the group's first member (no empty-group test; the checked iterator calls
// 00BF6713 and the head's +8h is used), its director route 00778860; when the route's
// vtable +0Ch answers true, its vtable +8h point set is copied out through 0059CAE0(out, 0),
// 00A10C20(ECX = group) runs (its answer unused), and the point indices 0..count-2 are walked
// with the checked-iterator bounds tests on i+1 and i against the 0Ch-byte element vector at
// points+4h/+8h, re-reading 0059CAC0 every step. The drawing itself is stripped.
void ai_cautious_approach_visit_route_points_00a12580(Seg69Host& host, Raw approach, Raw group,
    void* unused)
{
    (void)unused;
    if (approach.at<std::uint8_t>(0x4) == 0) return;
    void* head = group.ptr(kGroupMembersHead);
    void* first = node_next(head);
    if (first == head) host.invalid_parameter_00bf6713();
    void* route = host.director_route_00778860(node_value(first));
    if (!host.route_virtual_c(route)) return;
    Raw points = raw(host.route_virtual_8(route));
    std::uint8_t copied[0xC];
    host.route_points_0059cae0(points.p, copied, 0);
    host.group_target_00a10c20(group.p);
    if (host.route_points_count_0059cac0(points.p) - 1 <= 0) return;
    int i = 0;
    do {
        const auto size = [&]() -> std::uint32_t {
            auto* begin = static_cast<std::uint8_t*>(points.ptr(0x4));
            auto* end = static_cast<std::uint8_t*>(points.ptr(0x8));
            return static_cast<std::uint32_t>((end - begin) / 0xC);
        };
        if (points.ptr(0x4) == nullptr || static_cast<std::uint32_t>(i + 1) >= size()) {
            host.invalid_parameter_00bf6713();
        }
        if (points.ptr(0x4) == nullptr || static_cast<std::uint32_t>(i) >= size()) {
            host.invalid_parameter_00bf6713();
        }
        ++i;
    } while (i < host.route_points_count_0059cac0(points.p) - 1);
}

// 00A12A50, ABI as 00A12540. REGROUPINGMOVE debug draw, stripped: owner leader refresh.
void ai_command_regrouping_move_debug_draw_00a12a50(Seg69Host& host, Raw command)
{
    ai_group_refresh_leader_pose(host, raw(command.ptr(0x4)));
}

// 00A12CD0, ABI as 00A12540. MOVETOATTACK debug draw, stripped: target (+1Ch) then owner.
void ai_command_movetoattack_debug_draw_00a12cd0(Seg69Host& host, Raw command)
{
    ai_group_refresh_leader_pose(host, raw(command.ptr(0x1C)));
    ai_group_refresh_leader_pose(host, raw(command.ptr(0x4)));
}

// 00A12DB0, ABI as 00A12540. CAUTIOUSATTACK debug draw, stripped: 00A12580(ECX = +20h)(owner,
// &local), then, with the +24h byte clear, target and owner leader refreshes.
void ai_command_cautious_attack_debug_draw_00a12db0(Seg69Host& host, Raw command)
{
    std::uint8_t local[0x10];
    ai_cautious_approach_visit_route_points_00a12580(host, Raw{command.p + 0x20},
        raw(command.ptr(0x4)), local);
    if (command.at<std::uint8_t>(0x24) != 0) return;
    ai_group_refresh_leader_pose(host, raw(command.ptr(0x1C)));
    ai_group_refresh_leader_pose(host, raw(command.ptr(0x4)));
}

// 00A12EA0, ABI as 00A12540. CLOSEATTACK debug draw, stripped: target then owner.
void ai_command_close_attack_debug_draw_00a12ea0(Seg69Host& host, Raw command)
{
    ai_group_refresh_leader_pose(host, raw(command.ptr(0x1C)));
    ai_group_refresh_leader_pose(host, raw(command.ptr(0x4)));
}

// 00A12F10, ABI as 00A12540. DEFENDPOSITION debug draw, stripped: the owner refresh twice,
// then 00A113D0(ECX = command)(&local) (the danger point it would have drawn).
void ai_command_defend_position_debug_draw_00a12f10(Seg69Host& host, Raw command)
{
    ai_group_refresh_leader_pose(host, raw(command.ptr(0x4)));
    ai_group_refresh_leader_pose(host, raw(command.ptr(0x4)));
    float point[3];
    host.defend_danger_point_00a113d0(command.p, point);
}

// 00A12F90, ABI as 00A12540. PATROLTO debug draw, stripped: owner leader refresh.
void ai_command_patrol_to_debug_draw_00a12f90(Seg69Host& host, Raw command)
{
    ai_group_refresh_leader_pose(host, raw(command.ptr(0x4)));
}

// 00A12FD0, ABI as 00A12540. RETREAT debug draw, stripped: owner refresh, 00A11530(ECX =
// command)(&local), owner refresh again.
void ai_command_retreat_debug_draw_00a12fd0(Seg69Host& host, Raw command)
{
    ai_group_refresh_leader_pose(host, raw(command.ptr(0x4)));
    float point[3];
    host.retreat_point_00a11530(command.p, point);
    ai_group_refresh_leader_pose(host, raw(command.ptr(0x4)));
}

// 00A13080, ABI: __thiscall(ECX = command)(float* out_score, const float* other_point), RET 8,
// answers the group. The owner (+4h) leader's x/z (00F87574 when empty); with other_point the
// midpoint, ((p + o) rounded to float) * 0.5 (double 00D7A280) rounded to float. Over the
// other team's group list (00F8AA48 + 0Ch * (team == 0), so team 0 searches list 1 and any
// other team list 0), a group whose 00A2C3D0 ratio is not <= 0 (FLDZ/FCOMIP/JNC: a NaN is
// taken) scores (dx^2 + dz^2 rounded to float) / (own +5644h + its +5644h) (FIDIV, rounded to
// float) against its leader point; a score below the best (initially 1e10f, 00CE4970; JBE
// keeps ties and NaN out) is kept. *out_score receives the best when out_score is set. The
// list's vacuous checked-iterator tests are omitted.
void* ai_command_find_nearest_enemy_group_00a13080(Seg69Host& host, Raw command, float* out_score,
    const float* other_point)
{
    Raw own = raw(command.ptr(0x4));
    const std::uint32_t list_index = own.at<std::int32_t>(kGroupTeam) == 0 ? 1u : 0u;
    const float* own_point = group_leader_point(host, own);
    float x = own_point[0];
    float z = own_point[2];
    if (other_point != nullptr) {
        const float sx = other_point[0] + x;
        const float sz = other_point[2] + z;
        x = static_cast<float>(static_cast<double>(sx) * 0.5);
        z = static_cast<float>(0.5 * static_cast<double>(sz));
    }
    float best = kNearestInitialBest;
    void* best_group = nullptr;
    Raw list = host.image_global(kGroupListsByTeam + list_index * 0xC);
    for (void* node = node_next(list.ptr(0x4)); node != list.ptr(0x4); node = node_next(node)) {
        Raw group = raw(node_value(node));
        const float ratio = host.group_recon_ratio_00a2c3d0(group.p);
        if (0.0f >= ratio) continue;
        const float* point = group_leader_point(host, group);
        const float dx = x - point[0];
        const float dz = z - point[2];
        const float sum = static_cast<float>(static_cast<double>(dx) * dx + static_cast<double>(dz) * dz);
        const std::int32_t count = raw(command.ptr(0x4)).at<std::int32_t>(kGroupMemberCount) +
            group.at<std::int32_t>(kGroupMemberCount);
        const float score = static_cast<float>(static_cast<double>(sum) / static_cast<double>(count));
        if (best > score) {
            best = score;
            best_group = group.p;
        }
    }
    if (out_score != nullptr) *out_score = best;
    return best_group;
}

// 00A13280, ABI: __fastcall(ECX = command) RET. r = tuning+1F4h; the nearest enemy group
// (00A13080(&score, null)); when one is found and score < r*r (r*r rounded to float; JBE
// skips ties and NaN), a 20h-byte CLOSEATTACK is built inline (00BF681B, 00A10710(owner,
// target), vtables 00D22C44 / 00D22C2C; null when the allocation fails) and installed through
// 00A2BD00(ECX = owner).
void ai_command_auto_close_attack_nearest_00a13280(Seg69Host& host, Raw command)
{
    const float reach = host.active_mode_tuning_00a371a0().at<float>(kTuningCloseAttackDist);
    float score = 0.0f;
    void* target = ai_command_find_nearest_enemy_group_00a13080(host, command, &score, nullptr);
    if (target == nullptr) return;
    const float reach2 = reach * reach;
    if (!(reach2 > score)) return;
    void* memory = host.operator_new_00bf681b(0x20);
    void* attack = nullptr;
    if (memory != nullptr) {
        attack = ai_command_close_attack_construct_00a10ae0(host, raw(memory), command.ptr(0x4), target).p;
    }
    host.group_set_command_00a2bd00(command.ptr(0x4), attack);
}

// 00A137D0, ABI: __thiscall(ECX = command)(out NativeStr*), RET 4, answers out.
// out = "command: " + 00E0E308[vtable[+4h]()] + "\n" (00CE4390). The newline is built first,
// then the name, then "command: "; the two concatenations go through a temporary.
NativeStr* ai_command_describe_string_00a137d0(Seg69Host& host, Raw command, NativeStr* out)
{
    NativeStr newline;
    construct_literal(host, newline, "\n", 1);
    const int type = host.command_type_4(command.p);
    const char* name = static_cast<const char**>(static_cast<void*>(host.image_global(kCommandTypeNames).p))[type];
    NativeStr type_name{0, nullptr};
    host.native_string_assign_0041e870(&type_name, name);
    NativeStr prefix;
    construct_literal(host, prefix, "command: ", 9);
    NativeStr joined{0, nullptr};
    NativeStr* left = host.native_string_concat_004261a0(&prefix, &joined, &type_name);
    host.native_string_concat_004261a0(left, out, &newline);
    release(host, joined);
    release(host, prefix);
    release(host, type_name);
    release(host, newline);
    return out;
}

// 00A13960, ABI: __thiscall(ECX = command)(lua object), RET 4. The MOVE-family Lua describe:
// "commandType" as 00A0FF60, then a new table under "targetPos" (00B67580 with a NativeStr
// key, released after), 00B67800 fetches it into a temporary and 0088BA30(EDX = +8h) writes
// the command's point into it; the temporary is destructed (00B67700).
void ai_command_move_describe_lua_00a13960(Seg69Host& host, Raw command, void* lua_object)
{
    ai_command_describe_type_00a0ff60(host, command, lua_object);
    NativeStr key;
    construct_literal(host, key, "targetPos", 9);
    host.lua_set_new_table_00b67580(lua_object, &key);
    release(host, key);
    std::uint8_t table[0x14];
    void* fetched = host.lua_get_by_name_00b67800(lua_object, table, "targetPos");
    host.lua_set_vector3_0088ba30(fetched, &command.at<float>(0x8));
    host.lua_object_destruct_00b67700(table);
}

// 00A13A60, ABI as 00A13960. The ATTACK-family Lua describe: "commandType", then key "target"
// set (00B675D0) to the target group's (+1Ch) first member's Lua self object (00927B40 into a
// temporary, destructed after). No empty-group test: the checked iterator calls 00BF6713 and
// the head's +8h is used.
void ai_command_attack_describe_lua_00a13a60(Seg69Host& host, Raw command, void* lua_object)
{
    ai_command_describe_type_00a0ff60(host, command, lua_object);
    NativeStr key;
    construct_literal(host, key, "target", 6);
    Raw target = raw(command.ptr(0x1C));
    void* head = target.ptr(kGroupMembersHead);
    void* first = node_next(head);
    if (first == head) host.invalid_parameter_00bf6713();
    std::uint8_t self[0x14];
    void* value = host.entity_lua_self_00927b40(node_value(first), self);
    host.lua_set_object_00b675d0(lua_object, &key, value);
    host.lua_object_destruct_00b67700(self);
    release(host, key);
}

// 00A15A70, ABI: __thiscall(ECX = brain)(party slot), RET 4, answers brain. +20h = slot,
// +24h = 009FFD60(slot) (the slot's team), +0h..+1Ch zeroed, then by 004BCA50: mode 4 Duel
// (+10h, 00A1F500), 5 Escort (+14h, 00A1F630), 6 Siege (+18h, 00A1F760), 7 Competitive (+1Ch,
// 00A1F890), each 38h bytes and alone; any other mode Defend (+0h, 00A1EFC0, 38h), Attack
// (+4h, 00A1F0F0, 38h), Sell (+8h, 00A1F220, 38h) and Capture (+0Ch, 00A1F390, 44h). Each
// constructor is __thiscall(ECX = memory)(brain); a failed allocation stores null.
Raw ai_party_brain_construct_00a15a70(Seg69Host& host, Raw brain, int party_slot)
{
    brain.at<std::int32_t>(0x20) = party_slot;
    brain.at<std::int32_t>(0x24) = host.player_slot_team_009ffd60(party_slot);
    for (std::size_t off = 0; off < 0x20; off += 4) brain.at<std::uint32_t>(off) = 0;
    const auto build = [&](std::size_t slot, std::uint32_t bytes, std::uint32_t constructor) {
        void* memory = host.operator_new_00bf681b(bytes);
        brain.ptr(slot) = memory != nullptr ? host.planner_construct(constructor, memory, brain.p) : nullptr;
    };
    switch (host.game_mode_004bca50()) {
    case 4: build(0x10, 0x38, 0x00A1F500); return brain;
    case 5: build(0x14, 0x38, 0x00A1F630); return brain;
    case 6: build(0x18, 0x38, 0x00A1F760); return brain;
    case 7: build(0x1C, 0x38, 0x00A1F890); return brain;
    default: break;
    }
    build(0x0, 0x38, 0x00A1EFC0);
    build(0x4, 0x38, 0x00A1F0F0);
    build(0x8, 0x38, 0x00A1F220);
    build(0xC, 0x44, 0x00A1F390);
    return brain;
}

// 00A15D00, ABI: __fastcall(ECX = brain) RET. By 004BCA50: mode 4..7 tail-jumps to the one
// mode planner's (+10h..+1Ch) vtable +28h; any other mode calls +0h, +4h, +8h and tail-jumps
// to +0Ch. No null tests, as in the image.
void ai_party_brain_debug_draw_planners_00a15d00(Seg69Host& host, Raw brain)
{
    const int mode = host.game_mode_004bca50();
    if (mode == 4) return host.planner_debug_draw_28(brain.ptr(0x10));
    if (mode == 5) return host.planner_debug_draw_28(brain.ptr(0x14));
    if (mode == 6) return host.planner_debug_draw_28(brain.ptr(0x18));
    if (mode == 7) return host.planner_debug_draw_28(brain.ptr(0x1C));
    host.planner_debug_draw_28(brain.ptr(0x0));
    host.planner_debug_draw_28(brain.ptr(0x4));
    host.planner_debug_draw_28(brain.ptr(0x8));
    host.planner_debug_draw_28(brain.ptr(0xC));
}

}  // namespace bsp::cc13_seg69

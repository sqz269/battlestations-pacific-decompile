// Packet cc_lua_core. See include/bsp/lua_binding_spawn.hpp for the evidence per routine.
// Every name is a hypothesis, not a recovered symbol. Nothing here invents a global or stubs
// an unresolved call: every native step is a host method the caller supplies.

#include "bsp/lua_binding_spawn.hpp"
#include "bsp/lua_spawn_new.hpp"

#include <stdexcept>
#include <string>

namespace bsp {
const char* spawn_new_field_name(SpawnNewField field) noexcept
{
    switch (field) {
    case SpawnNewField::Party: return "party";
    case SpawnNewField::Player: return "player";
    case SpawnNewField::Callback: return "callback";
    case SpawnNewField::ExcludeRadiusOverride: return "excludeRadiusOverride";
    case SpawnNewField::Id: return "id";
    case SpawnNewField::GroupMembers: return "groupMembers";
    case SpawnNewField::CamoColor: return "CamoColor";
    case SpawnNewField::Area: return "area";
    case SpawnNewField::RefPos: return "refPos";
    case SpawnNewField::AngleRange: return "angleRange";
    case SpawnNewField::DistRange: return "distRange";
    case SpawnNewField::LookAt: return "lookAt";
    }
    return "";
}

int spawn_flag_slot(int argument_count, bool slot8_is_string_or_nil) noexcept
{
    // 009449E7 seeds the next slot with 8. When the count exceeds 8 and slot 8 is a string it
    // is consumed as the instance name (00944AF8) and the next slot becomes 9 (00944B4E MOV
    // EBP,9); when slot 8 is nil (00944B32) it is skipped with the same effect. Anything else leaves the slot at 8.
    int slot = 8;
    if (argument_count > 8 && slot8_is_string_or_nil) {
        slot = 9;
    }
    // 00944B61 CMP EAX,EBP with a JLE: the flag is read only when the count exceeds the slot.
    if (argument_count <= slot) {
        return -1;
    }
    return slot;
}

// ---------------------------------------------------------------------------
// 00944680 BSP_LuaBinding_Spawn (573 insns). cc13-P1 completed it from the listing.
// ABI: lua_CFunction, ECX = lua_State, plain RET, EAX = 00B66400's result count.
// Frame (ESP after the four pushes): +18h/+1Ch class NativeString, +3Ch/+40h instance
// NativeString, +44h..+4Ch placed position, +50h hidden record (then RingPoints' far point,
// then the heading and the id string), +70h..+78h origin, +90h/+94h/+98h slots 5/3/4,
// +38h/+20h the two angles, +12h/+13h the search's found / crossed bytes.
// EH: handler 00CA7F4B, states 0..18h over the LuaObject temporaries and the NativeStrings;
// the unwind is std::string / reader lifetime here, not reproduced state by state.
// Precision: the two degree conversions are FMUL m64 00CE3D28 (float pi) then FDIV m64
// 00CE3D20 (180.0) then FSTP float; modelled float per operation (main-thread 24-bit
// control word); under 53-bit the product would not round before the divide.
// ---------------------------------------------------------------------------
int lua_binding_spawn(LuaBindingArgumentReader& args,
                      LuaBindingResultWriter& results,
                      LuaBindingSpawnHost& host,
                      SpawnPlacementHost& placement,
                      void** out_object,
                      SpawnBindingArguments* read)
{
    if (out_object != nullptr) {
        *out_object = nullptr;
    }
    if (host.static_guard_test_and_set_00f89b48()) {      // 0094469B..009446AA
        host.static_first_call_009446b3();                 // 009446B3..009446E8
    }

    SpawnBindingArguments in;
    in.class_name = args.get_string(0);                    // 00944737..00944750
    void* record = host.find_hidden_entity_record_004691b0(in.class_name);   // 0094477D
    in.placement_mode = args.get_integer(1);               // 009447A0

    void* entity = nullptr;
    in.origin_is_entity = args.is_entity_table(2);         // 009447D4
    if (in.origin_is_entity) {
        entity = args.entity_at(2);                        // 0094480A
        if (!host.entity_byte(entity, kSpawnOriginEntityPoseDirtyOffset)) {
            host.entity_refresh_world_pose(entity);        // 0094482D
        }
        for (int i = 0; i < 3; ++i) {                      // 00944832..00944856
            in.origin[i] = host.entity_float(entity,
                static_cast<std::uint32_t>(kSpawnOriginEntityPositionOffset + 4u * i));
        }
    } else {
        host.read_vector3_00888760(2, in.origin);          // 0094487A
    }
    in.span_x = static_cast<float>(args.get_number(3));    // 009448CD -> +94h
    in.span_y = static_cast<float>(args.get_number(4));    // 00944907 -> +98h
    in.span_z = static_cast<float>(args.get_number(5));    // 00944941 -> +90h
    const auto radians = [](double degrees) {              // 00944980..00944998
        const float scaled = static_cast<float>(degrees * 3.1415927410125732);   // 00CE3D28
        return static_cast<float>(static_cast<double>(scaled) / 180.0);          // 00CE3D20
    };
    in.angle_a_radians = radians(args.get_number(6));      // 0094497B -> +38h
    in.angle_b_radians = radians(args.get_number(7));      // 009449BE -> +20h

    if (in.placement_mode == 1) {                          // 009449E4
        if (entity == nullptr) {
            // 009449EE reads [ESI+C8h] with ESI = 0 when slot 2 was a position: the image
            // faults. Stop rather than reproduce it.
            throw std::logic_error("Spawn mode 1 needs an entity origin (00944680 faults)");
        }
        if (!host.entity_byte(entity, kSpawnOriginEntityPoseDirtyOffset)) {
            host.entity_refresh_world_pose(entity);        // 009449F9
        }
        if (!host.entity_byte(entity, kSpawnOriginEntityPoseDirtyOffset)) {
            host.entity_refresh_world_pose(entity);        // 00944A11
        }
        const float yaw = spawn_relative_yaw_00944a2c(     // 00944A16..00944A3F
            host.entity_float(entity, kSpawnOriginEntityHeadingOffsetA),
            host.entity_float(entity, kSpawnOriginEntityHeadingOffsetB));
        in.angle_a_radians = yaw + in.angle_a_radians;     // 00944A49..00944A4D
        in.angle_b_radians = yaw + in.angle_b_radians;     // 00944A51..00944A55
    }

    in.instance_name = in.class_name;                      // 00944A59..00944A86 resize + memcpy
    int flag_slot = 8;                                     // 009449E7 MOV EBP,8
    if (args.count() > 8) {                                // 00944A9A..00944AA2
        if (args.is_string(8)) {                           // 00944AC2
            in.instance_name = args.get_string(8);         // 00944AF8, 00944B02 0041E350
            in.has_instance_name = true;
            flag_slot = 9;
        } else if (args.is_nil(8)) {                       // 00944B32
            flag_slot = 9;
        }
    }
    in.direct_placement = false;                           // 00944B57
    if (args.count() > flag_slot) {                        // 00944B5C..00944B63
        if (host.is_boolean_00b66000(flag_slot)) {
            in.direct_placement = args.get_boolean(flag_slot);   // 00944BB7
        } else {
            static_cast<void>(args.is_nil(flag_slot));     // 00944BD6, result unused
        }
    }

    SpawnPlacementRequest request;
    for (int i = 0; i < 3; ++i) request.origin[i] = in.origin[i];
    request.near_distance = in.span_x;
    request.far_distance = in.span_y;
    request.height = in.span_z;
    request.angle_low = in.angle_a_radians;
    request.angle_high = in.angle_b_radians;
    request.party = host.record_party_10(record);          // 00944BF1..00944BF5
    request.direct = in.direct_placement;
    if (read != nullptr) {
        *read = in;
    }

    float position[3] = {0.0f, 0.0f, 0.0f};
    if (!in.direct_placement) {                            // 00944BF8
        const SpawnWorldBox box = host.world_box_711c();
        const std::vector<SpawnPlacementEntity> entities = host.placement_entities();
        const SpawnSearchResult found =
            spawn_find_position_00944c44(request, box, entities, placement);   // 00944BFE..00944D60
        if (!found.ok) {
            // 00944F20: the only failure exit. 00944F24 pushes nil and the frame still reports
            // one result, because 00B66400 counts what is on the stack rather than a literal.
            results.push_nil();
            return 1;
        }
        for (int i = 0; i < 3; ++i) position[i] = found.position[i];
    } else {
        // 00944EA5..00944EB8: 00438E10(record+0Ch, "PlaneSquadronGen") == 0.
        request.aircraft = spawn_class_is_aircraft_00944ead(host.record_class_0c(record));
        const SpawnWorldBox box = host.world_box_711c();
        const std::vector<SpawnPlacementEntity> entities = host.placement_entities();
        const SpawnDirectResult placed =
            spawn_place_directly_00942cb0(request, box, entities, placement);  // 00944EF6
        for (int i = 0; i < 3; ++i) position[i] = placed.position[i];
    }

    // 00944D66..00944D8A: an empty name is passed as the empty data 00F89B40.
    void* object = host.create_scene_object(in.class_name, in.instance_name);
    host.after_scene_object_created();                     // 00944D93 00874D00, CL = 1
    if (object == nullptr) {
        // 00944D8F does not test the creator's result before 00944DC5 dereferences it. The
        // reconstruction stops instead of reproducing the fault, and says so rather than
        // pretending the native checks.
        return 0;
    }

    const float heading = spawn_heading_00944db8(in.origin, position);   // 00944D98..00944DDA
    host.object_vcall_118(object, position);               // 00944DDE
    host.object_vcall_11c(object, heading);                // 00944DF2

    // The entity tail: 00944DF4 takes word +174h, 00944E03 formats it, 00944E23 takes the
    // `thisTable` global and 00944E48 pushes `thisTable[key]`.
    const std::uint16_t entity_id = host.object_entity_id(object);
    host.push_this_table_value(std::to_string(entity_id));

    host.scene_resolve_deferred_references();              // 00944E9B
    if (out_object != nullptr) {
        *out_object = object;
    }
    return 1;                                              // 00944F31 00B66400
}

int lua_binding_spawn_new_id_is_requested(LuaBindingArgumentReader& args,
                                          LuaBindingResultWriter& results,
                                          LuaBindingSpawnHost& host)
{
    const std::string wanted = args.get_string(0);
    bool found = false;
    const std::vector<std::string> queued = host.queued_request_ids();
    for (std::size_t i = 0; i < queued.size() && !found; ++i) {
        // 009459C0's loop exits as soon as the accumulated flag is set, so the scan short-
        // circuits on the first match rather than counting them.
        found = spawn_request_id_matches(queued[i], wanted);
    }
    results.push_boolean(found);
    return 1;
}

int lua_binding_spawn_new_id_remove(LuaBindingArgumentReader& args, LuaBindingSpawnHost& host)
{
    const std::size_t removed = host.remove_requests_with_id(args.get_string(0));
    static_cast<void>(removed);
    // The result count comes from the frame, and nothing was pushed.
    return 0;
}

int lua_binding_spawn_new(LuaBindingSpawnHost& host, const std::string& request_id)
{
    host.enqueue_request(request_id, host.next_request_serial());
    return 0;
}

} // namespace bsp

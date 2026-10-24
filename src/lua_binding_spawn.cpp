// Packet cc_lua_core. See include/bsp/lua_binding_spawn.hpp for the evidence per routine.
// Every name is a hypothesis, not a recovered symbol. Nothing here invents a global or stubs
// an unresolved call: every native step is a host method the caller supplies.

#include "bsp/lua_binding_spawn.hpp"
#include "bsp/lua_spawn_new.hpp"

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

int lua_binding_spawn(const SpawnBindingArguments& request,
                      LuaBindingResultWriter& results,
                      LuaBindingSpawnHost& host,
                      void** out_object)
{
    if (out_object != nullptr) {
        *out_object = nullptr;
    }

    float position[3] = {0.0f, 0.0f, 0.0f};
    if (request.direct_placement) {
        host.place_spawn_directly(request, position);
    } else if (!host.find_spawn_position(request, position)) {
        // 00944F20: the only failure exit. 00944F24 pushes nil and the frame still reports one
        // result, because 00B66400 counts what is on the stack rather than a literal.
        results.push_nil();
        return 1;
    }

    void* object = host.create_scene_object(request.class_name, request.instance_name);
    host.after_scene_object_created();
    if (object == nullptr) {
        // 00944D8F does not test the creator's result before 00944DCD dereferences it. The
        // reconstruction stops instead of reproducing the fault, and says so rather than
        // pretending the native checks.
        return 0;
    }

    host.object_vcall_118(object, position);
    host.object_vcall_11c(object, position[2]);

    // The entity tail: 00944E03 formats the id, 00944E23 takes the `thisTable` global and
    // 00944E48 pushes `thisTable[key]`. include/bsp/mission_lua_bindings.hpp owns the key rule
    // and the field set; this routine only supplies the id.
    const std::uint16_t entity_id = host.object_entity_id(object);
    static_cast<void>(entity_id);

    host.scene_resolve_deferred_references();
    if (out_object != nullptr) {
        *out_object = object;
    }
    return 1;
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

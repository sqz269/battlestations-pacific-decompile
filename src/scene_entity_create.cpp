#include "bsp/scene_entity_create.hpp"

// Entity creation by name: 0046D930 and the spawn wrapper 004C6BA0.
// Evidence: docs/SCENE_ENTITY_CREATE.md, reports/scene_entity_create.json.

namespace bsp {

void scene_create_identity_frame(float out[16]) noexcept {
    // 0046DA5E XORPS XMM0,XMM0 and 0046DA61 MOVSS XMM1,[00D7A24C]; the sixteen
    // stores at 0046DA76..0046DAEE put XMM1 at +0h, +14h, +28h and +3Ch and
    // XMM0 everywhere else, and 0046DB00 REP MOVSD copies all sixteen dwords
    // into the by-value argument slot.
    for (int i = 0; i < 16; ++i) {
        out[i] = 0.0f;
    }
    out[0] = 1.0f;
    out[5] = 1.0f;
    out[10] = 1.0f;
    out[15] = 1.0f;
}

bool scene_create_record_has_parent(std::uint32_t parent_name_length) noexcept {
    // 0046DA2D CMP dword ptr [EBX + 0x54],0x0 / 0046DA31 JNZ.
    return parent_name_length != 0;
}

const char* scene_create_parent_name(const char* parent_name_data,
                                     const char* empty_string) noexcept {
    // 0046DA40 TEST EAX,EAX / 0046DA44 MOV EAX,0xE18560.
    return parent_name_data != nullptr ? parent_name_data : empty_string;
}

SceneCreatePropertyArm scene_create_property_arm(const void* overrides) noexcept {
    // 0046DB51 TEST EDI,EDI / 0046DB55 JZ 0046DBAF, where EDI is the third
    // stack argument reloaded at 0046DB4D.
    return overrides != nullptr ? SceneCreatePropertyArm::RecordBagWithOverrides
                                : SceneCreatePropertyArm::RecordBag;
}

bool scene_spawn_assigns_party_slots(std::uint32_t game_field_1fe4) noexcept {
    // 004C6BBE CMP dword ptr [ESI + 0x1fe4],0x0 / 004C6BC7 JZ.
    return game_field_1fe4 != 0;
}

namespace {

// The property-bag branch at 0046DB55, both arms. Returns the holder to store
// at entity+C0h, which is null when operator new(0Ch) failed.
void* build_property_bag_ref(SceneEntityCreateHost& host,
                             void* record_properties,
                             void* overrides) {
    if (scene_create_property_arm(overrides) == SceneCreatePropertyArm::RecordBag) {
        // 0046DBAF..0046DBE0: wrap the record's own bag. 00922E20 clones it, so
        // the record's bag is not shared with the entity either way.
        return host.make_property_bag_ref(record_properties);
    }

    // 0046DB57..0046DBAD: clone, merge, wrap, then destroy the temporary.
    void* clone = host.clone_property_bag(record_properties);
    host.apply_property_overrides(clone, overrides);
    void* ref = host.make_property_bag_ref(clone);
    if (clone != nullptr) {
        // 0046DBA1 TEST ESI,ESI, then the vtable slot 0 call at 0046DBAB with
        // the deleting flag. The holder already took its own copy.
        host.destroy_property_bag(clone);
    }
    return ref;
}

} // namespace

SceneCreateResult scene_entity_create_0046d930(SceneEntityCreateHost& host,
                                               void* scene_database,
                                               const SceneCreateRequest& request) {
    SceneCreateResult result;

    // 0046D95B..0046D9AB: key the map at SceneDatabase+18h with class_key and
    // stop when the node is the map head.
    void* record = host.find_scene_record(scene_database, request.class_key);
    if (record == nullptr) {
        result.outcome = SceneCreateOutcome::RecordNotFound;
        return result; // 0046D9AF XOR EAX,EAX ... 0046D9C3 RET 0xC
    }

    const SceneCreateRecordFields fields = host.read_record(record); // 0046D9D9

    // 0046D9E4..0046DA03: the record's class name selects the class descriptor.
    void* class_row = host.find_class_row(scene_database, fields.class_name);
    const int class_id = host.read_class_id(class_row); // 0046DB17, 0046DB3E

    // 0046DA2D..0046DA5A: resolve the parent by name, or leave it null.
    void* parent = nullptr;
    if (scene_create_record_has_parent(fields.parent_name_length)) {
        // The native substitute is the global empty NativeString data at
        // kSceneEmptyStringData; an empty literal stands for it here because no
        // address of the original image is meaningful in this projection.
        parent = host.find_parent_entity(
            scene_create_parent_name(fields.parent_name_data, ""));
    }

    // 0046DA5E..0046DB27: the generation gate. The parent frame is a literal
    // identity matrix rebuilt here on every call.
    SceneCreateGateArgs gate;
    gate.class_name = host.class_id_to_name(class_id); // 0046DB1D
    gate.entity_name = request.instance_name;
    gate.parent = parent;
    gate.local_frame = fields.local_frame;
    gate.properties = fields.properties;
    gate.unused_a6 = 0;
    scene_create_identity_frame(gate.parent_frame);
    gate.out_deferred_record = nullptr;

    if (!host.should_generate(scene_database, gate)) {
        result.outcome = SceneCreateOutcome::GateRejected;
        return result; // 0046DB2E JZ 0046D9AF, the same return-null tail
    }

    // 0046DB34..0046DB4B: the class descriptor's creator builds the instance.
    SceneCreateCreatorArgs creator;
    creator.class_id = class_id;
    creator.entity_name = request.instance_name;
    creator.parent = parent;
    creator.local_frame = fields.local_frame;
    creator.properties = fields.properties;
    creator.trailing_zero = 0;

    void* entity = host.run_class_creator(class_row, creator);

    // 0046DB55..0046DBE0: the entity's only field written outside the creator.
    void* ref = build_property_bag_ref(host, fields.properties, request.overrides);
    host.store_property_bag_ref(entity, ref);

    // 0046DBE6 XOR CL,CL / 0046DBE8 CALL 00925F20, on every arm that got here.
    host.init_all_entities(false);

    result.entity = entity; // 0046DBF3 MOV EAX,EBP
    result.outcome = SceneCreateOutcome::Created;
    result.property_bag_ref_attached = ref != nullptr;
    return result;
}

SceneCreateResult scene_entity_spawn_004c6ba0(SceneEntityCreateHost& host,
                                              void* game,
                                              const SceneCreateRequest& request) {
    // 004C6BB2 substitutes the scene-database singleton for the wrapper's own
    // `this`, and 004C6BB9 forwards the three arguments unchanged.
    SceneCreateResult result =
        scene_entity_create_0046d930(host, host.scene_database_singleton(), request);

    // 004C6BBE..004C6BCD. The gate is on the game object, not on the entity, so
    // this runs even when the creation returned null; the entity is preserved
    // in EDI across the call and returned either way.
    if (scene_spawn_assigns_party_slots(host.spawn_party_gate_field(game))) {
        host.assign_party_player_slots(game, 0);
    }

    return result;
}

} // namespace bsp

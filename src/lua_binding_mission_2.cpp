// Packet cc_lua_find_entity. See include/bsp/lua_binding_mission_2.hpp for the
// address list, the ABI and what is read rather than assumed.

#include "bsp/lua_binding_mission_2.hpp"

namespace bsp {

std::int32_t sentity_init_progress_denominator(std::int32_t pending_count) noexcept {
    // 00925FB3 LEA EAX,[EAX + EAX*2]: three passes report progress.
    return pending_count * 3;
}

float sentity_init_progress_fraction(std::int32_t step,
    std::int32_t denominator) noexcept {
    // 00926000 FILD dword ptr [ESP+0x10] / 0092600A FIDIV dword ptr [ESP+0x1C] /
    // 0092600E FSTP float ptr: an integer divide on the x87 stack, narrowed to
    // float32 once, at the store.
    if (denominator == 0) return 0.0f;
    return static_cast<float>(static_cast<double>(step)
        / static_cast<double>(denominator));
}

namespace {

// 0092601E..0092603B. The reciprocal multiply divides the denominator by three,
// which is the pending count again, and the counter saturates there.
std::int32_t advance_clamped(std::int32_t step, std::int32_t ceiling) noexcept {
    const std::int32_t next = step + 1;
    return (next < ceiling) ? next : ceiling;
}

void enable_child_chain(void* entity, SEntityInitAllHost& host) {
    // 009261D6..009261EE: from +48h along +44h, each node to 00922F30 with 1.
    void* child = host.entity_first_child_48(entity);
    while (child != nullptr) {
        host.scene_node_enable_00922f30(child);
        child = host.entity_next_child_44(child);
    }
}

void disable_child_chain(void* entity, SEntityInitAllHost& host) {
    // 0092620D..00926222, the mirror with 00922F80.
    void* child = host.entity_first_child_48(entity);
    while (child != nullptr) {
        host.scene_node_disable_00922f80(child);
        child = host.entity_next_child_44(child);
    }
}

}  // namespace

void sentity_init_all_00925f20(bool force_recon_refresh, SEntityInitAllHost& host) {
    const std::int32_t count = host.pending_count_00f899d4();
    // 00925F43 CMP EAX,EDI / 00925F49 JZ 00926375: an empty list skips every
    // pass and the tail, the char argument included.
    if (count == 0) return;

    const std::int32_t denominator = sentity_init_progress_denominator(count);
    std::int32_t step = 0;  // 00925FB6 MOV dword ptr [ESP+0xC],EDI, EDI = 0

    // Pass A, 00925FC4..00926062. The attach pass: every pending entity reaches
    // vtable +9Ch here, which is what gives it a `thisTable` slot.
    for (void* entity : host.pending_entities_00f899d0()) {
        host.entity_name_vcall_10(entity);                              // 00925FFB
        host.loading_progress_report_0057c1a0(kSEntityInitProgressPhase,
            sentity_init_progress_fraction(step, denominator));         // 00926019
        step = advance_clamped(step, count);                            // 0092602C
        host.entity_attach_lua_self_vcall_9c(entity);                   // 0092604E
    }

    host.set_init_active_flag_00f899a5(true);           // 00926082
    host.log_enum_count_004b8490(count, denominator);   // 00926089

    // Pass B, 009260A0..0092611E.
    for (void* entity : host.pending_entities_00f899d0()) {
        host.entity_name_vcall_10(entity);                              // 009260D3
        host.loading_progress_report_0057c1a0(kSEntityInitProgressPhase,
            sentity_init_progress_fraction(step, denominator));         // 009260F1
        ++step;                                                         // 009260F6
        host.entity_init_second_vcall_a0(entity);                       // 00926110
    }

    // Pass C, 00926136..00926230: the third virtual, then the spawn descriptor's
    // start-enabled byte decides whether the entity and its children come up
    // enabled or disabled.
    for (void* entity : host.pending_entities_00f899d0()) {
        host.entity_name_vcall_10(entity);                              // 00926172
        host.loading_progress_report_0057c1a0(kSEntityInitProgressPhase,
            sentity_init_progress_fraction(step, denominator));         // 0092618D
        ++step;                                                         // 0092619A
        host.entity_init_third_vcall_a4(entity);                        // 009261A1
        if (!host.entity_descriptor_kind_is_initial_state(entity)) continue;
        if (host.entity_descriptor_start_enabled_3c(entity)) {
            // 009261BB..009261F0
            if (host.entity_flag_5e(entity) || host.entity_flag_5c(entity)) continue;
            host.entity_set_flag_5c(entity, true);   // 009261D0, before the call
            host.entity_enable_vcall_68(entity);     // 009261D4
            enable_child_chain(entity, host);
        } else {
            // 009261F2..00926222
            if (host.entity_flag_5e(entity) || !host.entity_flag_5c(entity)) continue;
            host.entity_set_flag_5c(entity, false);  // 00926207
            host.entity_disable_vcall_6c(entity);    // 0092620B
            disable_child_chain(entity, host);
        }
    }

    host.set_init_active_flag_00f899a5(false);  // 0092623B

    // Pass D, 00926250..009262BF. No progress report on this pass.
    for (void* entity : host.pending_entities_00f899d0()) {
        if (!host.entity_session_gate_vcall_5c(entity, kEntitySessionGateArgument)) {
            continue;  // 00926289 TEST AL,AL / JZ
        }
        host.entity_name_vcall_10(entity);          // 0092629C
        host.session_register_0077f090(entity);     // 009262AE
    }

    // Pass E, 009262D0..0092632B: the spawn descriptor is destroyed and the
    // field nulled, so nothing downstream sees a stale kind.
    for (void* entity : host.pending_entities_00f899d0()) {
        host.entity_release_spawn_descriptor(entity);
    }

    host.clear_pending_list_00926335();
    if (force_recon_refresh) host.recon_force_refresh_00807a50();  // 00926370
}

namespace {

// Read from the shipped image: for each row of 004F2800's class table, the
// `.rdata` immediates the creator stores (itself and one call deep), keeping the
// ones whose `+9Ch` is one of the ten attach functions of
// docs/MISSION_ENTITY_LUA_ATTACH.md. The fifteen rows of the class table with no
// such immediate are the `UnitClassFactory` and `TypedResource` creators, which
// build through a factory and install the vtable deeper than the scan reaches;
// they are absent from this table rather than recorded as "no attach".
constexpr SceneClassLuaIdentityRow kRows[kSceneClassLuaIdentityCount] = {
    {0x18, "PlaneSquadronGen", 0x00D087C0u, 0x007F4580u, 0x12, true},
    {0x1A, "LandConvoy", 0x00CEA570u, 0x00743450u, 0x14, true},
    {0x1D, "LandingPoint", 0x00CE90E0u, 0x00928A00u, -1, false},
    {0x41, "NavPoint", 0x00CE8550u, 0x00928A00u, 0x3B, true},
    {0x42, "MovieCamPos", 0x00CE86D8u, 0x00928A00u, 0x3C, false},
    {0x43, "MovieCamLookat", 0x00CE8860u, 0x00928A00u, 0x3D, false},
    {0x47, "Path", 0x00CE6290u, 0x00928A00u, 0x41, true},
    {0x4A, "CameraPath", 0x00CE8390u, 0x00928A00u, 0x44, true},
    {0x4D, "SpawnPoint", 0x00CEA218u, 0x0077E830u, 0x47, true},
    {0x5B, "SimpleEffect", 0x00CE8BD0u, 0x00928A00u, -1, false},
    {0x5C, "PeriodicEffect", 0x00CE8D68u, 0x00928A00u, -1, false},
};

}  // namespace

const SceneClassLuaIdentityRow* scene_class_lua_identity_table() noexcept {
    return kRows;
}

const SceneClassLuaIdentityRow* find_scene_class_lua_identity(int class_id) noexcept {
    for (const SceneClassLuaIdentityRow& row : kRows) {
        if (row.class_id == class_id) return &row;
    }
    return nullptr;
}

}  // namespace bsp

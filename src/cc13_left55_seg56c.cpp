// Packet cc13-left55, segment 56 part c. See include/bsp/cc13_left55_seg56c.hpp for the shared
// shape and local/left55_doc_seg56c.md for the table. Comments cite the instruction each
// statement stands for; every function states its address, Lua name, ABI and uncertainties.
#include "bsp/cc13_left55_seg56c.hpp"

#include <string.h> // _stricmp

namespace bsp::cc13_left55::seg56c {
namespace {

void* game(ImageAccess& image)
{
    return image.global_ptr(kGameGlobal);
}

void* interface_object(ImageAccess& image)
{
    return image.global_ptr(kInterfaceGlobal);
}

void* entity_world(ImageAccess& image)
{
    return image.read_ptr(game(image), 0x19CCu); // [game+19CCh]
}

// [game+18CCh + [game+18ECh]*4]: the local side's player record.
void* local_player_record(ImageAccess& image)
{
    void* g = game(image);
    const std::int32_t side = image.read_i32(g, 0x18ECu);
    return image.read_ptr(g, 0x18CCu + 4u * static_cast<std::uint32_t>(side));
}

// The squadron plane slot as the bindings read it: CMP reg,4 / JA -> 0 (XOR), else [sq+3D0h+4*i].
void* squadron_plane_slot(ImageAccess& image, void* squadron, std::int32_t slot)
{
    return static_cast<std::uint32_t>(slot) <= 4u
        ? image.read_ptr(squadron, 0x3D0u + 4u * static_cast<std::uint32_t>(slot))
        : nullptr;
}

// The breadth-first walk GetPayloads / GetPayload / GetGuns share. The work vector is seeded
// with the entity, or with every plane slot of a squadron (vtable+5Ch(18h)); each element then
// appends its child list ([e+48h], next +44h). 004323D0 / 004322D0 and the inline fast path
// are std::vector<void*>::push_back (VC9 instantiation); their checked-iterator test
// (end < begin -> 00BF6713) cannot fire on a well-formed vector and is not modelled.
std::vector<void*> seed_walk(ImageAccess& image, Left55Seg56cHost& host, void* entity)
{
    std::vector<void*> work;
    if (!host.entity_vcall_5c(entity, 0x18)) { // PUSH 18h, CALL [EDX+5Ch]
        work.push_back(entity);
    } else {
        // CMP [ESI+3CCh] before the first pass and at the bottom of each.
        for (std::int32_t slot = 0; slot < image.read_i32(entity, 0x3CCu); ++slot) {
            work.push_back(squadron_plane_slot(image, entity, slot));
        }
    }
    return work;
}

void append_children(ImageAccess& image, std::vector<void*>& work, void* element)
{
    for (void* child = image.read_ptr(element, 0x48u); child != nullptr;
         child = image.read_ptr(child, 0x44u)) {
        work.push_back(child);
    }
}

} // namespace

// 008BDEC0 Objectives_ClientRefresh. No arguments read. When byte [game+218Ch] is zero
// (008BDFAC) and [game+1FE4h] == 1 (CMP ...,EBX with EBX = 1), calls 00627960 then 00626C50
// (both void(void) per Ghidra; ECX is not set up for them). Nothing pushed: returns 0.
int lua_objectives_client_refresh(ImageAccess& image, Left55Seg56cHost& host)
{
    void* g = game(image);
    if (image.read_u8(g, 0x218Cu) == 0 && image.read_i32(g, 0x1FE4u) == 1) {
        host.objectives_refresh_00627960();
        host.objectives_refresh_00626c50();
    }
    return 0;
}

// 008BE010 Hack_DisableBoatFormations: byte [game+19D0h] = get_boolean(0). Returns 0.
int lua_hack_disable_boat_formations(LuaArgs& args, ImageAccess& image)
{
    const bool value = args.get_boolean(0);
    image.write_u8(game(image), 0x19D0u, value ? 1u : 0u);
    return 0;
}

// 008BE180 Hack_ForceColumnFormationNearIslands: byte [game+19D1h] = get_boolean(0). Returns 0.
int lua_hack_force_column_formation_near_islands(LuaArgs& args, ImageAccess& image)
{
    const bool value = args.get_boolean(0);
    image.write_u8(game(image), 0x19D1u, value ? 1u : 0u);
    return 0;
}

// 008BE2F0 GetPlayerActWeapon. No arguments. 008BE3D4..008BE3DA: index = [[interface+50h]+44h],
// pushes the C string at [00E0CFC4 + index*4] through 00B66710. No bounds test. Returns 1.
int lua_get_player_act_weapon(ImageAccess& image, Left55Seg56cHost& host)
{
    void* weapons = image.read_ptr(interface_object(image), 0x50u);
    const std::int32_t index = image.read_i32(weapons, 0x44u);
    const auto* name = static_cast<const char*>(
        image.global_ptr(kWeaponNameTable + 4u * static_cast<std::uint32_t>(index)));
    host.lua_push_c_string_00b66710(name);
    return 1;
}

// 008BEA50 GetPlaneEquipment. entity_at(0); when vtable+5Ch(0Fh) answers, the entity itself is
// the plane (no null test, 008BEB7F); otherwise the plane is [entity+3D0h] and nothing is pushed
// when it is null (CMP ESI,EBP / JZ 008BEB8F). Pushes the integer [plane+C54h] (00B664B0).
int lua_get_plane_equipment(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56cHost& host)
{
    void* entity = args.entity_at(0);
    void* plane = entity;
    if (!host.entity_vcall_5c(entity, 0x0F)) {
        plane = image.read_ptr(entity, 0x3D0u);
        if (plane == nullptr) {
            return 0;
        }
    }
    results.push_number(image.read_i32(plane, 0xC54u));
    return 1;
}

// 008BEBE0 GetPlaneIsReloading. entity_at(0); the result byte starts 0 (008BECFC). Kind 0Fh:
// 007B92E0 on the entity (no null test); otherwise on [entity+3D0h] only when that is non-null
// (008BED0E..008BED16). Pushes the boolean (00B66450). Returns 1.
int lua_get_plane_is_reloading(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56cHost& host)
{
    void* entity = args.entity_at(0);
    bool reloading = false;
    if (host.entity_vcall_5c(entity, 0x0F)) {
        reloading = host.weapon_controller_any_slot_answers_007b92e0(entity);
    } else {
        void* plane = image.read_ptr(entity, 0x3D0u);
        if (plane != nullptr) {
            reloading = host.weapon_controller_any_slot_answers_007b92e0(plane);
        }
    }
    results.push_boolean(reloading);
    return 1;
}

// 008BEF80 GetSquadTorpedoes(squadron [, all]). entity_at(0); the flag is get_boolean(1) only
// when count() == 2 (008BF0A7), else 0. NewTable on the frame (008BF0E9). Walks the list
// [game+19CCh]: count [list+21Ch] read once, node [list+224h], next = [node]; element [node+8].
// An element qualifies when byte [e+5Eh] is 0 and ((e+310h)->vtable+38h() or the flag); it is
// added (00927B40 self table, 00B666C0 at index 1, 2, ...; EBX starts at 1, 008BF114) when one
// of the squadron's plane slots (slot <= 4, else 0; count [sq+3CCh] cached in ESI) equals
// [e+3BCh], then the slot scan stops. Returns 00B66400 (the frame holds the table).
int lua_get_squad_torpedoes(LuaArgs& args, LuaFrame& frame, ImageAccess& image, Left55Seg56cHost& host)
{
    void* squadron = args.entity_at(0);
    bool include_all = false;
    if (args.count() == 2) {
        include_all = args.get_boolean(1);
    }
    void* table = host.lua_new_table_00b67930(frame);
    void* list = entity_world(image);
    std::int32_t remaining = image.read_i32(list, 0x21Cu);
    void* node = image.read_ptr(list, 0x224u);
    std::int32_t next_index = 1;
    if (remaining > 0) {
        do {
            void* element = image.read_ptr(node, 0x8u);
            if (image.read_u8(element, 0x5Eu) == 0
                && (host.entity_subobject_310_vcall_38(element) || include_all)) {
                const std::int32_t slots = image.read_i32(squadron, 0x3CCu);
                for (std::int32_t slot = 0; slot < slots; ++slot) {
                    if (squadron_plane_slot(image, squadron, slot) == image.read_ptr(element, 0x3BCu)) {
                        host.lua_table_set_object_at_index_00b666c0(
                            table, next_index, host.entity_lua_self_object_00927b40(element));
                        ++next_index;
                        break;
                    }
                }
            }
            --remaining;
            node = image.read_ptr(node, 0x0u);
        } while (remaining != 0);
    }
    return frame.result_count();
}

// 008C09A0 SetRoleHack(entity, role, take). entity_at(0); get_integer(2) is read first
// (008C0AEF), then get_integer(1) (008C0AF7); 0077C470(ECX = entity, arg1, arg2). Returns 0.
int lua_set_role_hack(LuaArgs& args, Left55Seg56cHost& host)
{
    void* entity = args.entity_at(0);
    const std::int32_t take = args.get_integer(2);
    const std::int32_t role_mask = args.get_integer(1);
    host.unit_send_role_transfer_0077c470(entity, role_mask, take);
    return 0;
}

// 008C0B70 SetMouseExclusiveRepairCategory: the frame is opened and the count returned
// (008C0C5A); no argument read, no other call. Returns 0.
int lua_set_mouse_exclusive_repair_category()
{
    return 0;
}

// 008C0E10 ActivateObjectives(flag): 005ED5D0(ECX = [interface+60h], get_boolean(0)). Returns 0.
int lua_activate_objectives(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host)
{
    const bool expanded = args.get_boolean(0);
    host.objectives_screen_set_expanded_005ed5d0(image.read_ptr(interface_object(image), 0x60u), expanded);
    return 0;
}

// 008C1620 SetRocketAirGroundTypeDifferent(flag): byte 00F874FD = (flag == 0) (SETZ). Returns 0.
int lua_set_rocket_air_ground_type_different(LuaArgs& args, ImageAccess& image)
{
    const bool different = args.get_boolean(0);
    image.write_global_u8(kRocketAirGroundSameFlag, different ? 0u : 1u);
    return 0;
}

// 008C1790 SetHP(unit, value): 00877B90(ECX = entity_at(0), (float)get_number(1)) (FSTP float
// at 008C18C5). Returns 0.
int lua_set_hp(LuaArgs& args, Left55Seg56cHost& host)
{
    void* unit = args.entity_at(0);
    const float health = static_cast<float>(args.get_number(1));
    host.unit_set_health_00877b90(unit, health);
    return 0;
}

// 008C2AC0 WipeEntityType(name). The kind is 0 (EBP) unless the copied name is non-empty and
// _stricmp(name, "cloud" 00D14D18) == 0, then 3Dh (008C2BA7). Walks [[[game+19CCh]+4]] by +38h;
// every element with byte +5Eh == 0 and [+C4h] == kind gets 00926D90(element, 2). Returns 0.
// The NativeString's null buffer for an empty name is the std::string's empty() here.
int lua_wipe_entity_type(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host)
{
    const std::string name = args.get_string(0);
    std::int32_t kind = 0;
    if (!name.empty() && _stricmp(name.c_str(), "cloud") == 0) {
        kind = 0x3D;
    }
    void* head = image.read_ptr(entity_world(image), 0x4u);
    for (void* entity = image.read_ptr(head, 0x0u); entity != nullptr;
         entity = image.read_ptr(entity, 0x38u)) {
        if (image.read_u8(entity, 0x5Eu) == 0 && image.read_i32(entity, 0xC4u) == kind) {
            host.mission_entity_kill_00926d90(entity, 2); // PUSH 2, MOV ECX,ESI
        }
    }
    return 0;
}

// 008C2C50 GetTargetInfoTarget. No arguments. The target is [[interface+B4h]+8]; null pushes nil
// (00B66430 at 008C2DA4), otherwise the thisTable entity of its u16 id at +174h (MOVZX,
// 008C2D05). Returns 1.
int lua_get_target_info_target(LuaResults& results, ImageAccess& image, Left55Seg56cHost& host)
{
    void* target_info = image.read_ptr(interface_object(image), 0xB4u);
    if (image.read_ptr(target_info, 0x8u) == nullptr) {
        results.push_nil();
    } else {
        void* target = image.read_ptr(target_info, 0x8u); // re-read at 008C2D02
        host.push_this_table_entity_004260b0_00b663d0(
            static_cast<std::uint16_t>(image.read_i32(target, 0x174u) & 0xFFFF));
    }
    return 1;
}

// 008C3170 SetVisibilityTerrainNode(name, visible). get_boolean(1) first (008C326F), then the
// name get_string(0). 00B72300(ECX = [[interface+54h]+0Ch], &name, &"Terrain Visibility Group"
// (00CF00D8, length 18h)). On a node: factor = (float)(int)visible (FILD, 008C334C), then
// 00B6DA70(node, factor, 0); parent = [node+30h]; child = 00B6E750(parent, 00B6D800(parent), 1);
// when child is non-null, 00B6DA70(00B6E750(child, &name, 1), factor, 0) with no null test on
// the second lookup (008C3388). Returns 0.
int lua_set_visibility_terrain_node(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host)
{
    const bool visible = args.get_boolean(1);
    const std::string name = args.get_string(0);
    const std::string excluded_root = "Terrain Visibility Group";
    void* scene = image.read_ptr(image.read_ptr(interface_object(image), 0x54u), 0x0Cu);
    void* node = host.scene_find_node_by_name_excluding_00b72300(scene, name, excluded_root);
    if (node != nullptr) {
        const float factor = static_cast<float>(visible ? 1 : 0);
        host.scene_node_set_visibility_factor_00b6da70(node, factor, false);
        void* parent = image.read_ptr(node, 0x30u);
        void* group = host.node_find_child_by_header_00b6e750(parent, host.node_name_header_00b6d800(parent), true);
        if (group != nullptr) {
            void* twin = host.node_find_child_by_name_00b6e750(group, name, true);
            host.scene_node_set_visibility_factor_00b6da70(twin, factor, false);
        }
    }
    return 0;
}

// 008C3610 GetDevice(entity, index). entity_at(0), get_integer(1). Walks [entity+48h] by +44h;
// the first element whose vtable+5Ch(1Eh) answers and whose [+38Ch] equals the index is pushed
// as its thisTable entity (u16 +174h, MOVZX at 008C37D9). The end of the list pushes nil
// (00B66430 at 008C3782). Returns 1.
int lua_get_device(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56cHost& host)
{
    void* entity = args.entity_at(0);
    const std::int32_t index = args.get_integer(1);
    for (void* device = image.read_ptr(entity, 0x48u); device != nullptr;
         device = image.read_ptr(device, 0x44u)) {
        if (host.entity_vcall_5c(device, 0x1E) && image.read_i32(device, 0x38Cu) == index) {
            host.push_this_table_entity_004260b0_00b663d0(
                static_cast<std::uint16_t>(image.read_i32(device, 0x174u) & 0xFFFF));
            return 1;
        }
    }
    results.push_nil();
    return 1;
}

// 008C3880 GetKamikazeByDummy(id). get_integer(0). Walks [[game+19CCh]+130h] by +4; the first
// node whose [[node+8]+6B8h] equals the id pushes [node+8] as its thisTable entity (u16 +174h).
// No match pushes nothing: returns 0 then, 1 after the push.
int lua_get_kamikaze_by_dummy(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host)
{
    const std::int32_t dummy = args.get_integer(0);
    for (void* node = image.read_ptr(entity_world(image), 0x130u); node != nullptr;
         node = image.read_ptr(node, 0x4u)) {
        void* plane = image.read_ptr(node, 0x8u);
        if (image.read_i32(plane, 0x6B8u) == dummy) {
            host.push_this_table_entity_004260b0_00b663d0(
                static_cast<std::uint16_t>(image.read_i32(plane, 0x174u) & 0xFFFF));
            return 1;
        }
    }
    return 0;
}

// 008C3AB0 GetFormationPositions(unit). group = [entity_at(0)+284h]; NewTable on the frame;
// for i in 0..[group+4F8h) (count read once into EBX): table[i+1] = {} (00B66670), sub =
// table[i+1] (00B67720), record = 0070D070(group, i), k = [group+500h] (re-read each pass,
// 008C3C2C); 0088BA30(sub, {c - rec[10h+4k], 0.0f, c - rec[20h+4k]}) with c the float at
// 00D7A208 = 0x80000000 (-0.0f), SUBSS in single precision. Returns 00B66400.
int lua_get_formation_positions(LuaArgs& args, LuaFrame& frame, ImageAccess& image, Left55Seg56cHost& host)
{
    void* unit = args.entity_at(0);
    void* group = image.read_ptr(unit, 0x284u);
    void* table = host.lua_new_table_00b67930(frame);
    const std::int32_t members = image.read_i32(group, 0x4F8u);
    constexpr float kNegativeZero = -0.0f; // 00D7A208: 00 00 00 80
    for (std::int32_t i = 0; i < members; ++i) {
        host.lua_table_set_new_table_at_index_00b66670(table, i + 1);
        void* sub = host.lua_table_get_by_index_00b67720(table, i + 1);
        void* record = host.unit_group_member_record_at_0070d070(group, i);
        const auto k = static_cast<std::uint32_t>(image.read_i32(group, 0x500u));
        Vec3 position{};
        position.x = kNegativeZero - image.read_f32(record, 0x10u + 4u * k);
        position.y = 0.0f; // XORPS / MOVSS [ESP+14h] at 008C3BF4
        position.z = kNegativeZero - image.read_f32(record, 0x20u + 4u * k);
        host.lua_set_vector3_fields_0088ba30(sub, position);
    }
    return frame.result_count();
}

// 008C3CE0 ReloadTorpedoes(unit). Walks [entity_at(0)+48h] by +44h; for each element whose
// vtable+5Ch(20h) answers and whose [[+3F4h]+80h] == 7, every barrel 0..[+448h) gets
// 0072CF00(element, barrel, 0.0f (FLDZ), 0 (EBP)). Returns 0.
int lua_reload_torpedoes(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host)
{
    void* unit = args.entity_at(0);
    for (void* gun = image.read_ptr(unit, 0x48u); gun != nullptr; gun = image.read_ptr(gun, 0x44u)) {
        if (host.entity_vcall_5c(gun, 0x20) && image.read_i32(image.read_ptr(gun, 0x3F4u), 0x80u) == 7) {
            for (std::int32_t barrel = 0; barrel < image.read_i32(gun, 0x448u); ++barrel) {
                host.gun_set_barrel_reload_timer_0072cf00(gun, barrel, 0.0f, 0);
            }
        }
    }
    return 0;
}

// 008C3EA0 Scoring_IgnorePlayersPartyLosses: byte [[game+21A0h]+1484h] = 1. Returns 0.
int lua_scoring_ignore_players_party_losses(ImageAccess& image)
{
    image.write_u8(image.read_ptr(game(image), 0x21A0u), 0x1484u, 1u);
    return 0;
}

// 008C3FE0 Scoring_AddPartyLoss(party, name [, amount]). amount defaults to 1 (MOV ESI,1 at
// 008C40A8) and is get_integer(2) only when count() >= 3 (CMP EAX,3 / JL). Then the name
// get_string(1), then get_integer(0); 0090ED90(ECX = [game+21A0h], party, &name, amount). The
// callee body was not read; the parameter names are hypotheses. Returns 0.
int lua_scoring_add_party_loss(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host)
{
    std::int32_t amount = 1;
    if (args.count() >= 3) {
        amount = args.get_integer(2);
    }
    const std::string name = args.get_string(1);
    const std::int32_t party = args.get_integer(0);
    host.scoring_add_party_loss_0090ed90(image.read_ptr(game(image), 0x21A0u), party, name, amount);
    return 0;
}

// 008C4200 SetCounterTimer(value): 00528BE0(ECX = [interface+C8h], (float)get_number(0)). The
// argument is read before the counter pointer is loaded (008C430B). Returns 0.
int lua_set_counter_timer(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host)
{
    const float value = static_cast<float>(args.get_number(0));
    host.hud_counter_set_value_00528be0(image.read_ptr(interface_object(image), 0xC8u), value);
    return 0;
}

// 008C4B50 SetShallowWater(name). object = [[[game+19CCh]+34Ch]+8] (008C4C33..008C4C3F); only
// when non-null is the name get_string(0) read and 00880220(object, &name) called. The callee
// body was not read. Returns 0.
int lua_set_shallow_water(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host)
{
    void* object = image.read_ptr(image.read_ptr(entity_world(image), 0x34Cu), 0x8u);
    if (object != nullptr) {
        const std::string name = args.get_string(0);
        host.shallow_water_apply_00880220(object, name);
    }
    return 0;
}

// 008C5FD0 RemoveTrigger(entity, name). list = entity_at(0)->vtable+60h() (008C60ED), then the
// name get_string(1); pushes the boolean 0074FBB0(ECX = list, EDX = &name). Returns 1.
int lua_remove_trigger(LuaArgs& args, LuaResults& results, Left55Seg56cHost& host)
{
    void* entity = args.entity_at(0);
    void* list = host.entity_vcall_60(entity);
    const std::string name = args.get_string(1);
    results.push_boolean(host.trigger_list_invoke_by_name_0074fbb0(list, name));
    return 1;
}

// 008C7010 CheatRemoveWater(entity): when vtable+5Ch(6) answers, vtable+23Ch(). Returns 0.
int lua_cheat_remove_water(LuaArgs& args, Left55Seg56cHost& host)
{
    void* entity = args.entity_at(0);
    if (host.entity_vcall_5c(entity, 6)) {
        host.entity_vcall_23c(entity);
    }
    return 0;
}

// 008C74B0 GameOver. No arguments. 00A933F0(004BEC00(), 1Eh, 5); byte [[00F8A0C4]+D0h] = 1;
// 0096BF70([00F8A0C4]); then for the lists at recon+DDCh, +DE8h, +DF4h (008053C0(0) fetched
// again before each), node next +4: obj = [[node+8]+4]; when byte [obj+5Dh] == 0,
// obj->vtable+F4h(1.0f) (FLD1). Returns 0.
int lua_game_over(ImageAccess& image, Left55Seg56cHost& host)
{
    host.input_set_context_level_00a933f0(host.input_manager_singleton_004bec00(), 0x1E, 5);
    void* warnings = image.global_ptr(kWarningManagerGlobal);
    image.write_u8(warnings, 0xD0u, 1u);
    host.warning_manager_clear_waiting_queue_0096bf70(warnings);
    for (std::uint32_t list_offset : {0xDDCu, 0xDE8u, 0xDF4u}) {
        void* recon = host.recon_ensure_slot_008053c0(0); // XOR ECX,ECX
        for (void* node = image.read_ptr(recon, list_offset); node != nullptr;
             node = image.read_ptr(node, 0x4u)) {
            void* owner = image.read_ptr(node, 0x8u);
            if (image.read_u8(image.read_ptr(owner, 0x4u), 0x5Du) == 0) {
                host.entity_vcall_f4(image.read_ptr(owner, 0x4u), 1.0f);
            }
        }
    }
    return 0;
}

// 008C8560 class_enumeration([type]). 004CAC30(game), 004D2B10(game), then
// VehicleClass = globals["VehicleClass" 00CE5880]. With no argument (count() == 0, 008C8618)
// the table is iterated (key/value objects 00B65F50, 00B67080 / 00B67190, end by 00B66420):
// key k = get_integer(key); for k >= 0: k == 15Eh calls 004254B0("kukac!" 00D153D8); the
// value's "Type" (00CE4780) string is copied; the row is skipped (only 004CAC30(game)) when
// the type is "LandVehicle" (00CE5858) or "LandFort" (00CE5864) by _stricmp, or equals
// "CommandBuilding" (00CE5870) by 00425850. Otherwise player record +28h = 0, 004CAC30,
// d = 00964790(k, 1), b = d->vtable+18h(0Fh), 004D2B10; when b: record +28h = 1, 004CAC30,
// 00964790(k, 1), 004D2B10. With an argument: get_integer(0), 004CAC30, 00964790(n, 1),
// 004D2B10. Both paths end with 004CAC30(game). The native returns a literal 0 (not 00B66400).
// The type copy is a NativeString whose buffer is null when empty: an empty type skips the two
// _stricmp tests exactly as std::string::empty() does here.
int lua_class_enumeration(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host)
{
    host.game_pump_front_end_frame_004cac30(game(image));
    host.game_destroy_world_containers_004d2b10(game(image));
    void* vehicle_class = host.lua_object_get_by_name_00b67800(host.lua_state_globals_00b67980(), "VehicleClass");
    if (args.count() == 0) {
        void* key = host.lua_object_construct_00b65f50();
        void* value = host.lua_object_construct_00b65f50();
        host.lua_iterate_first_00b67080(vehicle_class, key, value);
        while (!host.lua_object_is_unbound_00b66420(vehicle_class, key)) {
            const std::int32_t type_id = host.lua_object_get_integer_00b66290(key);
            if (type_id >= 0) { // CMP EAX,EBX / JL 008C8878
                if (type_id == 0x15E) {
                    host.debug_trace_004254b0("kukac!");
                }
                const std::string type = host.lua_object_get_string_00b662b0(
                    host.lua_object_get_by_name_00b67800(value, "Type"));
                const bool skip =
                    (!type.empty()
                        && (_stricmp(type.c_str(), "LandVehicle") == 0 || _stricmp(type.c_str(), "LandFort") == 0))
                    || host.native_string_equals_insensitive_00425850(type, "CommandBuilding");
                if (skip) {
                    host.game_pump_front_end_frame_004cac30(game(image));
                } else {
                    image.write_i32(local_player_record(image), 0x28u, 0);
                    host.game_pump_front_end_frame_004cac30(game(image));
                    void* descriptor = host.vehicle_class_get_or_create_00964790(type_id, true);
                    const bool again = host.vehicle_class_vcall_18(descriptor, 0x0F);
                    host.game_destroy_world_containers_004d2b10(game(image));
                    if (again) {
                        image.write_i32(local_player_record(image), 0x28u, 1);
                        host.game_pump_front_end_frame_004cac30(game(image));
                        host.vehicle_class_get_or_create_00964790(type_id, true);
                        host.game_destroy_world_containers_004d2b10(game(image));
                    }
                }
            }
            host.lua_iterate_next_00b67190(vehicle_class, key, value);
        }
    } else {
        const std::int32_t type_id = args.get_integer(0);
        host.game_pump_front_end_frame_004cac30(game(image));
        host.vehicle_class_get_or_create_00964790(type_id, true);
        host.game_destroy_world_containers_004d2b10(game(image));
    }
    host.game_pump_front_end_frame_004cac30(game(image));
    return 0;
}

// 008C9460 AddLineOfSightTrigger(entity, name?, callback, target, range). target = entity_at(3),
// range = (float)get_number(4); trigger = 00750550(ECX = target, range). The optional string
// is read from argument 1 by 008C9350 (nil leaves it disengaged), the callback get_string(2);
// 00750790(trigger, &optional, &callback); list = entity_at(0)->vtable+60h();
// 0074FAB0(ECX = list, EDX = trigger). Returns 0.
int lua_add_line_of_sight_trigger(LuaArgs& args, Left55Seg56cHost& host)
{
    void* target = args.entity_at(3);
    const float range = static_cast<float>(args.get_number(4));
    void* trigger = host.line_of_sight_trigger_create_00750550(target, range);
    std::optional<std::string> optional_name;
    bsp::cc13_seg56::lua_object_get_optional_string(args, 1, optional_name);
    const std::string callback = args.get_string(2);
    host.trigger_set_callback_name_00750790(trigger, optional_name, callback);
    void* list = host.entity_vcall_60(args.entity_at(0));
    host.trigger_list_add_0074fab0(list, trigger);
    return 0;
}

// 008C9730 AddProximityTrigger(entity, name?, callback, target, range). As AddLineOfSightTrigger
// but the target is a SceneCommandTarget read from argument 3 by 0088A810 (entity or vector)
// and the trigger comes from 007505C0(&target, range). Returns 0.
int lua_add_proximity_trigger(LuaArgs& args, Left55Seg56cHost& host)
{
    const SceneCommandTarget target = host.read_command_target_0088a810(3);
    const float range = static_cast<float>(args.get_number(4));
    void* trigger = host.proximity_trigger_create_007505c0(target, range);
    std::optional<std::string> optional_name;
    bsp::cc13_seg56::lua_object_get_optional_string(args, 1, optional_name);
    const std::string callback = args.get_string(2);
    host.trigger_set_callback_name_00750790(trigger, optional_name, callback);
    void* list = host.entity_vcall_60(args.entity_at(0));
    host.trigger_list_add_0074fab0(list, trigger);
    return 0;
}

// 008C9A10 AddSquadOrdnanceDroppedTrigger(entity, name?, callback). trigger = 00750660() first
// (008C9AF6, before any argument read); then as the two above. Returns 0.
int lua_add_squad_ordnance_dropped_trigger(LuaArgs& args, Left55Seg56cHost& host)
{
    void* trigger = host.squad_ordnance_trigger_create_00750660();
    std::optional<std::string> optional_name;
    bsp::cc13_seg56::lua_object_get_optional_string(args, 1, optional_name);
    const std::string callback = args.get_string(2);
    host.trigger_set_callback_name_00750790(trigger, optional_name, callback);
    void* list = host.entity_vcall_60(args.entity_at(0));
    host.trigger_list_add_0074fab0(list, trigger);
    return 0;
}

// 008CBEB0 HackSensitiveUnit(entity). The raw "Ptr" field of argument 0 (00888D20, not
// 00888AA0) is inserted into the set at [00F8A0C4]+1A4h (008CAEE0). Returns 0.
int lua_hack_sensitive_unit(ImageAccess& image, Left55Seg56cHost& host)
{
    void* unit = host.lua_table_ptr_field_00888d20(0);
    auto* set = static_cast<std::uint8_t*>(image.global_ptr(kWarningManagerGlobal)) + 0x1A4; // ADD ECX,1A4h
    host.sensitive_unit_set_insert_008caee0(set, unit);
    return 0;
}

// 008CC330 Scoring_IgnoreEntityKill(entity): entity_at(0) inserted into the tree at
// [game+21A0h]+1488h (0063F1E0, LEA ECX,[EAX+1488h]). Returns 0.
int lua_scoring_ignore_entity_kill(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host)
{
    void* entity = args.entity_at(0);
    auto* tree = static_cast<std::uint8_t*>(image.read_ptr(game(image), 0x21A0u)) + 0x1488;
    host.scoring_ignored_kill_set_insert_0063f1e0(tree, entity);
    return 0;
}

// 008CC850 Loading_Start. No arguments. Only when [game+644h] <= 0 (CMP ...,EBP(0) / JG):
// 0096BF70([00F8A0C4]); 004CD0F0(game, 1, 0, 1) (PUSH EBX, EBP, EBX); 0068A670(interface,
// &matrix, &velocity); 004D1100()->00867EE0(0.0f (FLDZ), [game+19FCh]); [00F8BBD8]->vtable+4(
// &matrix, velocity by value) (008CC99B..008CC9D4); 0060D6E0([00E19698], 1). Returns 0.
int lua_loading_start(ImageAccess& image, Left55Seg56cHost& host)
{
    if (image.read_i32(game(image), 0x644u) <= 0) {
        host.warning_manager_clear_waiting_queue_0096bf70(image.global_ptr(kWarningManagerGlobal));
        host.game_set_cinematic_mode_004cd0f0(game(image), true, false, true);
        ListenerMatrix matrix{};
        Vec3 velocity{};
        host.interface_get_sound_listener_0068a670(interface_object(image), matrix, velocity);
        void* effects = host.effect_manager_singleton_004d1100();
        host.effect_manager_update_00867ee0(effects, 0.0f, image.read_ptr(game(image), 0x19FCu));
        host.listener_sink_vcall_04(image.global_ptr(kListenerSinkGlobal), matrix, velocity);
        host.please_wait_screen_show_with_worker_0060d6e0(image.global_ptr(kPleaseWaitScreenGlobal), true);
    }
    return 0;
}

// 008CCEB0 SortByDistance(entity, list). origin = entity_at(0) world position +FCh/+100h/+104h
// after 00414DB0 when byte +C8h is 0. The list (argument 1, re-fetched by 00B677E0 before each
// iteration call) is iterated; each value is copied into a local LuaObject vector (00442220)
// and scored: a vector3 table (0088B840) by 00888760, otherwise the "Ptr" userdata (00CFAD08)
// whose pose is refreshed the same way. Score (x87, 008CD0FB..008CD1FE): dx, dy, dz each
// rounded to float; dy*dy and dx*dx each rounded to float; their sum is kept in the 80-bit
// register, dz*dz rounded to float is added and the total rounded to float. Here the sum is
// done in double (exact for two float squares except at exponent gaps beyond 29 bits; an
// uncertainty of the last float ulp in that corner). {index, score} pairs go into a vector
// (008CCA30 push_back), are sorted by 008C9E10 (first, last, count), and a new table on the
// frame gets table[i+1] = objects[pair[i].index] (00B666C0) with the checked-index test
// (index >= size or no storage -> 00BF6713) kept. 00BF6713 is _invalid_parameter_noinfo,
// which does not return in the shipped CRT; the native would go on to pass the out-of-range
// element address, here a null handle stands in on that path only. The two pre-sort checks
// (begin > end -> 00BF6713) are kept too. Returns 00B66400.
int lua_sort_by_distance(LuaArgs& args, LuaFrame& frame, ImageAccess& image, Left55Seg56cHost& host)
{
    void* origin_entity = args.entity_at(0);
    if (image.read_u8(origin_entity, 0xC8u) == 0) {
        host.entity_pose_refresh_world_00414db0(origin_entity);
    }
    const float ox = image.read_f32(origin_entity, 0xFCu);
    const float oy = image.read_f32(origin_entity, 0x100u);
    const float oz = image.read_f32(origin_entity, 0x104u);

    std::vector<ScoredEntry> scored; // [ESP+24h]
    std::vector<void*> objects;      // [ESP+34h], 14h-byte LuaObjects in the native
    void* key = host.lua_object_construct_00b65f50();
    void* value = host.lua_object_construct_00b65f50();
    std::uint32_t index = 0; // ESI
    host.lua_iterate_first_00b67080(host.lua_argument_object_00b677e0(1), key, value);
    while (!host.lua_object_is_unbound_00b66420(host.lua_argument_object_00b677e0(1), key)) {
        host.lua_object_vector_push_back_00442220(objects, value);
        float dx = 0.0f;
        float dy = 0.0f;
        float dz = 0.0f;
        if (host.lua_object_is_vector3_table_0088b840(value)) {
            const Vec3 point = host.lua_object_read_vector3_00888760(value);
            dx = point.x - ox;
            dy = point.y - oy;
            dz = point.z - oz;
        } else {
            void* entity = host.lua_object_to_userdata_00b662d0(host.lua_object_get_by_name_00b67800(value, "Ptr"));
            if (image.read_u8(entity, 0xC8u) == 0) {
                host.entity_pose_refresh_world_00414db0(entity);
            }
            dx = image.read_f32(entity, 0xFCu) - ox;
            dy = image.read_f32(entity, 0x100u) - oy;
            dz = image.read_f32(entity, 0x104u) - oz;
        }
        const float dy2 = dy * dy;
        const float dx2 = dx * dx;
        const float dz2 = dz * dz;
        const float score = static_cast<float>((static_cast<double>(dx2) + static_cast<double>(dy2))
            + static_cast<double>(dz2));
        scored.push_back(ScoredEntry{index, score});
        ++index;
        host.lua_iterate_next_00b67190(host.lua_argument_object_00b677e0(1), key, value);
    }

    ScoredEntry* first = scored.data();
    ScoredEntry* last = scored.data() + scored.size();
    if (first > last) { // 008CD2B3
        host.crt_invalid_parameter_00bf6713();
        if (first > last) { // 008CD2C4
            host.crt_invalid_parameter_00bf6713();
        }
    }
    host.sort_scored_entries_008c9e10(first, last, static_cast<std::int32_t>(last - first));

    void* table = host.lua_new_table_00b67930(frame);
    for (std::uint32_t i = 0; !scored.empty() && i < scored.size(); ++i) {
        const std::uint32_t object_index = scored[i].key;
        if (objects.empty() || object_index >= objects.size()) { // 008CD310..008CD330
            host.crt_invalid_parameter_00bf6713();
        }
        host.lua_table_set_object_at_index_00b666c0(
            table, static_cast<int>(i + 1), object_index < objects.size() ? objects[object_index] : nullptr);
    }
    return frame.result_count();
}

// 008CECC0 GetPayloads(entity). entity_at(0); NewTable on the frame; the shared walk (seed_walk,
// append_children); every walked element whose vtable+5Ch(2Ah) answers (asked after its
// children are appended, 008CEEFB) is added as its self table (00927B40) at index 1, 2, ...
// Returns 00B66400.
int lua_get_payloads(LuaArgs& args, LuaFrame& frame, ImageAccess& image, Left55Seg56cHost& host)
{
    void* entity = args.entity_at(0);
    void* table = host.lua_new_table_00b67930(frame);
    std::vector<void*> work = seed_walk(image, host, entity);
    std::int32_t next_index = 1; // [ESP+10h] = 1 at 008CEE5E
    for (std::size_t i = 0; i < work.size(); ++i) {
        void* element = work[i];
        append_children(image, work, element);
        if (host.entity_vcall_5c(element, 0x2A)) {
            host.lua_table_set_object_at_index_00b666c0(table, next_index, host.entity_lua_self_object_00927b40(element));
            ++next_index;
        }
    }
    return frame.result_count();
}

// 008CEFC0 GetPayload(entity, n). entity_at(0); the walk is seeded first, then n =
// get_integer(1) (008CF14B). Each walked element appends its children; when vtable+5Ch(2Ah)
// answers, n is decremented and the element is pushed as its thisTable entity (u16 +174h,
// MOVZX at 008CF23E) once n < 1. An exhausted walk pushes nothing. Returns 1 / 0.
int lua_get_payload(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host)
{
    void* entity = args.entity_at(0);
    std::vector<void*> work = seed_walk(image, host, entity);
    std::int32_t remaining = args.get_integer(1);
    for (std::size_t i = 0; i < work.size(); ++i) {
        void* element = work[i];
        append_children(image, work, element);
        if (host.entity_vcall_5c(element, 0x2A)) {
            --remaining;
            if (remaining < 1) {
                host.push_this_table_entity_004260b0_00b663d0(
                    static_cast<std::uint16_t>(image.read_i32(element, 0x174u) & 0xFFFF));
                return 1;
            }
        }
    }
    return 0;
}

// 008CF7E0 GetGuns(entity). GetPayloads with vtable+5Ch(20h) as the test (008CFA17). Returns
// 00B66400.
int lua_get_guns(LuaArgs& args, LuaFrame& frame, ImageAccess& image, Left55Seg56cHost& host)
{
    void* entity = args.entity_at(0);
    void* table = host.lua_new_table_00b67930(frame);
    std::vector<void*> work = seed_walk(image, host, entity);
    std::int32_t next_index = 1; // [ESP+10h] = 1 at 008CF97E
    for (std::size_t i = 0; i < work.size(); ++i) {
        void* element = work[i];
        append_children(image, work, element);
        if (host.entity_vcall_5c(element, 0x20)) {
            host.lua_table_set_object_at_index_00b666c0(table, next_index, host.entity_lua_self_object_00927b40(element));
            ++next_index;
        }
    }
    return frame.result_count();
}

} // namespace bsp::cc13_left55::seg56c

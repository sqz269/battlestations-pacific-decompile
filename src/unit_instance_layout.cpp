// docs/UNIT_INSTANCE_LAYOUT.md. Every sequence below is a projection of one
// native routine over an injected host; the host methods are the native call
// sites. Nothing here allocates, links or copies real memory.
#include "bsp/unit_instance_layout.hpp"

namespace bsp {

int unit_constructor_class_id(int level) noexcept
{
    if (level < 0 || level > kUnitConstructorLevels) {
        return -1;
    }
    return kUnitConstructorChain[level].class_id;
}

std::uint32_t unit_entity_id_registry(int level_argument) noexcept
{
    // 009286B8: CMP byte ptr [ESP+28h], BL with BL = 0, then JZ to the
    // 00F89A5C arm. 0077EED0 pushes the literal 1 at 0077EEF2.
    return level_argument != 0 ? kUnitEntityIdRegistryPrimary
                               : kUnitEntityIdRegistryAlternate;
}

UnitSlotCounterStep unit_slot_counter_step(int global_counter) noexcept
{
    // 0081F1A2  MOV DL, [00F87151]        ; the stored value is the pre-increment one
    // 0081F1A8  MOV [ESI+109Ch], DL
    // 0081F1AE  ADD AL, 1 / MOV [00F87151], AL
    // 0081F1BD  CMP [00F87151], 0Bh / JLE ; else MOV byte [00F87151], 0
    UnitSlotCounterStep step{};
    step.stored = global_counter;
    const int incremented = global_counter + 1;
    step.next = incremented > kUnitSlotCounterLimit ? 0 : incremented;
    return step;
}

UnitPlacementDecision unit_placement_decision(std::uint32_t current_node,
                                              std::uint32_t new_node) noexcept
{
    // 009288A1  MOV EAX, [ESI+30h]
    // 009288A8  CMP EBP, EAX / SETNZ BL   ; one predicate, reused at 009288DC
    UnitPlacementDecision decision{};
    decision.node_changed = new_node != current_node;
    decision.leave_old_parent = decision.node_changed && current_node != 0;
    decision.enter_new_parent = decision.node_changed && new_node != 0;
    return decision;
}

UnitInstanceCreationResult create_unit_instance(UnitInstanceCreationHost& host,
                                                std::uint32_t descriptor,
                                                std::uint32_t flag,
                                                int global_slot_counter) noexcept
{
    UnitInstanceCreationResult result{};

    // 006FE5AF then 006FE5BE. The memset runs on the raw return value, before
    // the null test at 006FE5CC.
    const std::uint32_t block = host.allocate(kUnitInstanceAllocationSize);
    host.zero_fill(block, kUnitInstanceAllocationSize);
    result.zeroed = true;

    if (block != 0) {
        // 006FE5DD: 006FE460(instance, flag), which is the whole chain.
        // Level 0, 00925CE0.
        host.construct_weak_owner(block + kUnitLayoutOffWeakOwner);
        host.copy_identity_matrix(block + kUnitLayoutOffLocalMatrix); // 00925DCA
        host.copy_identity_matrix(block + kUnitLayoutOffLocalMatrix); // 00925EA0
        host.reset_name_string(block + 0x154);

        // Level 1, 00928630, under the world lock.
        const std::uint32_t registry = host.entity_registry();
        host.enter_world_lock(registry);
        const std::uint32_t id_registry = unit_entity_id_registry(1);
        result.entity_id = host.allocate_entity_id(id_registry, flag, block);
        result.role_slot = kUnitRoleSlotValue;
        host.initialise_role_slot(block, kUnitRoleSlotMask, kUnitRoleSlotValue);
        host.entity_registered_hook(block);
        host.leave_world_lock(registry);

        // Level 3, 0087B670: the fixed-step node, group 0 at construction.
        host.construct_tick_node(block + kUnitLayoutOffTickNode, block, 0);
        result.tick_payload = block;

        // Level 4, 0095CC90: +538h takes the constructor's third argument,
        // which 0081ED40 passes as 0.
        result.descriptor_owning = 0;

        // Level 5, 0081ED40: the sub-objects, in call order, then the group
        // index overwrite and the global slot counter.
        host.construct_base_sub_object(block + 0x72c, 0x00809270);
        host.construct_base_sub_object(block + kUnitLayoutOffOrderRing, 0x00812d40);
        host.construct_base_sub_object(block + 0xa20, 0x0093bcc0);
        host.construct_base_sub_object(block + 0xbd0, 0x00815600);
        host.construct_base_sub_object(block + 0x10d4, 0x0074e7b0);
        const std::uint32_t side_block = host.allocate_side_block(0x2c);
        static_cast<void>(side_block); // stored at +73Ch
        result.tick_group = kUnitTickGroupIndex;
        const UnitSlotCounterStep slot = unit_slot_counter_step(global_slot_counter);
        result.slot_counter = slot.stored;

        // Level 6, 006FE460: the eight vptrs and the class id.
        for (int i = 0; i < kUnitVptrSlotCount; ++i) {
            result.vptrs[i] = kUnitVptrSlotValues[i];
        }
        result.class_id = unit_constructor_class_id(kUnitConstructorLevels);
        result.instance = block;
    }

    // 006FE5E8..006FE5FC run unconditionally on the native side; when the
    // allocation failed ESI is 0 and the setter receives a null this.
    host.set_vehicle_class(result.instance, descriptor);
    if (result.instance != 0) {
        result.descriptor_owning = descriptor;
        result.descriptor_back = descriptor;
    }
    return result;
}

UnitAttachOutcome attach_unit_instance(UnitInstancePlacementHost& host,
                                       UnitAttachState& state,
                                       std::uint32_t instance,
                                       std::uint32_t hierarchy_parent,
                                       std::uint32_t world_node,
                                       std::uint32_t matrix) noexcept
{
    UnitAttachOutcome outcome{};
    // 009258F6: CMP byte ptr [ESI+BCh], BL / JNZ to the epilogue.
    if (state.attached) {
        return outcome;
    }
    outcome.ran = true;

    state.world_node = world_node;       // 00925906
    state.hierarchy_parent = hierarchy_parent; // 00925935
    outcome.linked_to_hierarchy_parent = hierarchy_parent != 0;
    // 00925938: the null arm at 0092596A uses the list object at worldNode+8h.
    outcome.hierarchy_list_owner = hierarchy_parent != 0 ? hierarchy_parent : world_node;

    // 0092598E: both fields are cleared before the call, and the arguments are
    // the values they held.
    if (state.deferred_a != 0 || state.deferred_b != 0) {
        const std::uint32_t a = state.deferred_a;
        const std::uint32_t b = state.deferred_b;
        state.deferred_a = 0;
        state.deferred_b = 0;
        host.flush_deferred(instance, b, a);
    }

    host.copy_local_matrix(instance + kUnitLayoutOffLocalMatrix, matrix); // 009259C4
    state.pose_valid = false;      // 009259CE
    state.attach_flag_10c = false; // 009259D4

    state.attached = true; // 009259EE
    return outcome;
}

UnitPlacementDecision place_unit_instance(UnitInstancePlacementHost& host,
                                          UnitAttachState& state,
                                          std::uint32_t instance,
                                          std::uint32_t hierarchy_parent,
                                          std::uint32_t world_node,
                                          std::uint32_t matrix) noexcept
{
    const std::uint32_t registry = host.entity_registry(); // 0092887C
    host.enter_world_lock(registry);                       // 00928895

    const UnitPlacementDecision decision =
        unit_placement_decision(state.world_node, world_node);
    if (decision.leave_old_parent) {
        host.leaving_parent(instance); // 009288C7, vtable[134h]
    }

    attach_unit_instance(host, state, instance, hierarchy_parent, world_node, matrix);

    // 009288E1 re-reads [ESI+30h] rather than reusing the argument, so a body
    // that did not run still gates on whatever the field holds now.
    if (decision.node_changed && state.world_node != 0) {
        host.entered_parent(instance); // 009288F1, vtable[130h]
    }

    host.leave_world_lock(registry); // 009288FC
    return decision;
}

void register_unit_instance(UnitInstancePlacementHost& host,
                            std::uint32_t instance,
                            std::uint32_t world_node) noexcept
{
    // 006FE623 calls 00928560 first, which pushes onto the +24h list; the five
    // that follow are 006FE62F, 006FE63B, 006FE647, 006FE653 and 006FE65F.
    for (int i = 0; i < kUnitParentListCount; ++i) {
        host.push_parent_list(world_node + kUnitParentListOffsets[i], instance);
    }
}

const UnitClassCreatorRow kUnitClassCreators[kUnitClassCreatorCount] = {
    {"Destroyer", 0x00d1acf8, 0x006fe590, 0x1188, 0x006fe460, true},
    {"Cruiser", 0x00d1ad38, 0x006fb430, 0x1188, 0x006fb300, true},
    {"LandingShip", 0x00d1ad78, 0x0074be00, 0x122c, 0x0074bb00, true},
    {"Cargo", 0x00d1adbc, 0x006eb290, 0x118c, 0x006eb160, true},
    {"BattleShip", 0x00d1adf8, 0x006dfef0, 0x118c, 0x006dfc90, true},
    {"Submarine", 0x00d1ae38, 0x008531a0, 0x1288, 0x00852f10, true},
    {"TorpedoBoat", 0x00d1ae78, 0x00857e20, 0x1190, 0x00857cd0, true},
    {"MotherShip", 0x00d1aebc, 0x00758d30, 0x12c8, 0x00758550, true},
    {"ReconPlane", 0x00d19bf4, 0x008091d0, 0x0e94, 0x0074e0f0, true},
    {"SmallReconPlane", 0x00d1a5a8, 0x0084ca50, 0x0e94, 0x0084c920, true},
    {"LargeReconPlane", 0x00d1a5ec, 0x0074e540, 0x0e94, 0x0074e2d0, true},
    {"Fighter", 0x00d19c30, 0x007ddae0, 0x0e94, 0x007dd9b0, true},
    {"DiveBomber", 0x00d19c70, 0x00956390, 0x0e94, 0x00951b60, false},
    {"TorpedoBomber", 0x00d19f4c, 0x009564e0, 0x0e94, 0x00000000, false},
    {"Kamikaze", 0x00d1a224, 0x00956240, 0x0e94, 0x00000000, false},
    {"LevelBomber", 0x00d1a4f8, 0x007d7850, 0x0e94, 0x007d7720, true},
    {"AirField", 0x00d1a9a0, 0x006d3110, 0x08e4, 0x006d1c20, true},
    {"Shipyard", 0x00d1a9dc, 0x00848380, 0x07a4, 0x00848080, true},
    {"LandVehicle", 0x00d1aa18, 0x0074df10, 0x0740, 0x0074dcc0, true},
    {"LandFort", 0x00cff790, 0x00747000, 0x0758, 0x00745940, true},
    {"CommandBuilding", 0x00d1a538, 0x006f5c10, 0x07e8, 0x006f5610, true},
    // 00749150 is XOR EAX,EAX ; RET 4: this class never allocates an instance.
    {"DummyTargetVehicle", 0x00d1aa58, 0x00749150, 0x0000, 0x00000000, false},
};

bool unit_class_uses_vehicle_base(std::uint32_t instance_ctor) noexcept
{
    // The eight callers of 0081ED40, from the live call graph.
    switch (instance_ctor) {
    case 0x006dfc90:
    case 0x006eb160:
    case 0x006fb300:
    case 0x006fe460:
    case 0x0074bb00:
    case 0x00758550:
    case 0x00852f10:
    case 0x00857cd0:
        return true;
    default:
        return false;
    }
}

} // namespace bsp

#include "bsp/entity_identity.hpp"

#include <cstring>
#include <string.h>

// Reconstruction of the entity identity path: the two u16 handle tables
// (00951660 / 00951560 / 009517C0 / 009516D0 / 00951720 / 00927940 / 00521E30),
// the backslash-path lookup 009251F0, and the Cloud creators 004E9E40 /
// 004E9920. Evidence per claim in docs/ENTITY_IDENTITY.md.
//
// These are not drop-in binary replacements. The tables are index-based (the
// native links are raw addresses), and where the native computes an address with
// no bounds check this file returns a failure instead; every such divergence is
// commented with the instruction that differs.
namespace bsp {
namespace {

// 00438E10 BSP_CString_CompareInsensitive: equal pointers are equal, a null left
// sorts first, a null right sorts last, otherwise the CRT _stricmp.
int compare_insensitive_00438e10(const char* left, const char* right)
{
    if (left == right) {
        return 0;
    }
    if (left == nullptr) {
        return -1;
    }
    if (right == nullptr) {
        return 1;
    }
    return _stricmp(left, right);
}

bool slot_index_for_id(const EntityIdTable& table, std::uint16_t id, std::uint32_t& index)
{
    // 009517E0 / 009516D5 / 00521E3B: (id - first_id) * 10h + slots. The native
    // does not range-check; the caller of this helper turns an out-of-range
    // index into a failure.
    const int biased = static_cast<int>(id) - static_cast<int>(table.first_id);
    if (biased < 0) {
        return false;
    }
    index = static_cast<std::uint32_t>(biased);
    return index < static_cast<std::uint32_t>(table.slots.size());
}

// 009517F8 / 00951800: node->next->prev = node->prev; *(node->prev) = node->next.
// In the index model the owning list has to be named, because the native's "the
// address of the link that points at me" carries that for free.
void unlink_slot(EntityIdTable& table, std::uint32_t index)
{
    EntityIdSlot& node = table.slots[index];
    EntityIdList& list = node.in_live_list ? table.live_list : table.free_list;
    if (node.prev == kEntityIdNoSlot) {
        list.head = node.next;
    } else {
        table.slots[node.prev].next = node.next;
    }
    if (node.next == kEntityIdNoSlot) {
        list.tail = node.prev;
    } else {
        table.slots[node.next].prev = node.prev;
    }
    node.next = kEntityIdNoSlot;
    node.prev = kEntityIdNoSlot;
}

// 00951802..00951813 (live list) and 00951761..00951771 (free list): insert at
// the head.
void push_front(EntityIdTable& table, std::uint32_t index, bool live)
{
    EntityIdList& list = live ? table.live_list : table.free_list;
    EntityIdSlot& node = table.slots[index];
    node.prev = kEntityIdNoSlot;
    node.next = list.head;
    node.in_live_list = live;
    if (list.head == kEntityIdNoSlot) {
        list.tail = index;
    } else {
        table.slots[list.head].prev = index;
    }
    list.head = index;
}

} // namespace

void entity_id_table_reset_00951560(EntityIdTable& table)
{
    table.free_list = EntityIdList{};
    table.live_list = EntityIdList{};
    table.free_count = 0;
    if (table.slots.empty()) {
        return;
    }

    // Slot 0 is stored with the id 0 and left out of every list, whatever the
    // table's first id is: 00951560 writes the literal 0 into slots+8 and the
    // build loop starts at index 1. 00951720 skips index 0 as well.
    table.slots[0] = EntityIdSlot{};

    const std::uint32_t count = static_cast<std::uint32_t>(table.slots.size());
    for (std::uint32_t i = 1; i < count; ++i) {
        EntityIdSlot& slot = table.slots[i];
        slot = EntityIdSlot{};
        slot.id = static_cast<std::uint16_t>(table.first_id + static_cast<std::uint16_t>(i));
        // next runs toward the terminator, i.e. toward the lower indices.
        slot.next = (i == 1) ? kEntityIdNoSlot : (i - 1);
        slot.prev = (i + 1 < count) ? (i + 1) : kEntityIdNoSlot;
    }
    if (count >= 2) {
        table.free_list.head = count - 1;  // 00951560 *(this+1Ch)
        table.free_list.tail = 1;          // 00951560 *(this+10h)
        table.free_count = static_cast<std::int32_t>(count) - 1;
    }
}

EntityIdTable entity_id_table_construct_00951660(std::uint16_t first_id, std::uint32_t id_count)
{
    EntityIdTable table;
    table.first_id = first_id;
    table.id_count = id_count;
    table.slots.resize(static_cast<std::size_t>(id_count));
    entity_id_table_reset_00951560(table);
    return table;
}

EntityIdSweepResult entity_id_table_sweep_00951720(EntityIdTable& table)
{
    EntityIdSweepResult result;
    const std::uint32_t count = static_cast<std::uint32_t>(table.slots.size());
    if (count < 2) {
        return result;
    }
    for (std::uint32_t i = count - 1; i >= 1; --i) {
        // 00951737 tests slot i unbiased ...
        if (table.slots[i].entity != nullptr) {
            continue;
        }
        // ... while 00951741 relinks slot i - first_id. Identical only for the
        // table whose first id is 0; see docs/ENTITY_IDENTITY.md.
        const int biased = static_cast<int>(i) - static_cast<int>(table.first_id);
        if (biased < 0 || static_cast<std::uint32_t>(biased) >= count) {
            result.out_of_range_skipped += 1;
            continue;
        }
        const std::uint32_t index = static_cast<std::uint32_t>(biased);
        table.slots[index].entity = nullptr;
        unlink_slot(table, index);
        push_front(table, index, false);
        table.free_count += 1;
        result.reclaimed += 1;
    }
    return result;
}

std::uint16_t entity_id_table_allocate_009517c0(EntityIdTable& table,
                                                std::uint16_t requested_id,
                                                const void* entity)
{
    if (table.free_count == 0) {
        entity_id_table_sweep_00951720(table);  // 009517C6
    }

    std::uint32_t index = kEntityIdNoSlot;
    std::uint16_t id = 0;
    if (requested_id == 0) {
        // 009517D4: the free list's tail, and the id already in that slot.
        index = table.free_list.tail;
        if (index == kEntityIdNoSlot) {
            return 0;  // the native would dereference the terminator here
        }
        id = table.slots[index].id;
    } else {
        // 009517DD: the caller's id addresses its slot directly, unchecked.
        if (!slot_index_for_id(table, requested_id, index)) {
            return 0;
        }
        id = requested_id;
    }

    table.slots[index].entity = entity;  // 009517F2
    unlink_slot(table, index);
    push_front(table, index, true);  // the live list at +3Ch
    table.free_count -= 1;           // 00951815
    return id;
}

void entity_id_table_release_009516d0(EntityIdTable& table, std::uint16_t id)
{
    std::uint32_t index = kEntityIdNoSlot;
    if (!slot_index_for_id(table, id, index)) {
        return;  // the native has no such check
    }
    table.slots[index].entity = nullptr;  // 009516DB
    unlink_slot(table, index);
    push_front(table, index, false);  // the free list at +1Ch
    table.free_count += 1;            // 009516FD
}

const void* entity_id_table_lookup(const EntityIdTable& table, std::uint16_t id)
{
    std::uint32_t index = kEntityIdNoSlot;
    if (!slot_index_for_id(table, id, index)) {
        return nullptr;
    }
    return table.slots[index].entity;
}

EntityHandleTablePair entity_handle_table_construct_00927940(std::uint32_t low_count,
                                                             std::uint32_t high_count)
{
    EntityHandleTablePair pair;
    pair.primary = entity_id_table_construct_00951660(0, low_count);
    // 0092796A..00927970: the low table's count is the high table's first id.
    pair.alternate =
        entity_id_table_construct_00951660(static_cast<std::uint16_t>(low_count), high_count);
    return pair;
}

EntityIdTableKind entity_handle_table_kind_for_id_009287eb(const EntityHandleTablePair& pair,
                                                           std::uint16_t id)
{
    // 009287F5 CMP ECX,[00F89A10], the low table's id count.
    return (static_cast<std::uint32_t>(id) < pair.primary.id_count) ? EntityIdTableKind::Primary
                                                                   : EntityIdTableKind::Alternate;
}

const void* entity_handle_table_resolve_00521e30(const EntityHandleTablePair& pair,
                                                 std::uint16_t id)
{
    return (entity_handle_table_kind_for_id_009287eb(pair, id) == EntityIdTableKind::Primary)
               ? entity_id_table_lookup(pair.primary, id)
               : entity_id_table_lookup(pair.alternate, id);
}

EntityIdTableKind entity_construct_table_kind_009286b8(int construct_flag)
{
    // 009286B8 CMP byte ptr [ESP+0x28],BL with BL zeroed at 00928660: the zero
    // arm at 009286DA loads 00F89A5C, the other 00F89A08.
    return (construct_flag != 0) ? EntityIdTableKind::Primary : EntityIdTableKind::Alternate;
}

const EntityNameNode* entity_find_by_qualified_name_009251f0(const EntityNameNode& node,
                                                             const char* name)
{
    const char* const node_name = node.node_name();
    if (node_name == nullptr) {
        return nullptr;  // 00925203
    }
    if (compare_insensitive_00438e10(node_name, name) == 0) {
        return &node;  // 00925228, the whole remaining string matched
    }

    const int children = node.child_count();  // node+50h
    if (children == 0 || name == nullptr) {
        return nullptr;
    }
    const char* const separator = std::strchr(name, kEntityQualifiedNameSeparator);
    if (separator == nullptr) {
        return nullptr;  // 00925246: an unqualified miss never descends
    }

    // 00925254..00925265: the component before the separator must equal this
    // node's name in full, length first, then __strnicmp.
    const std::size_t component = static_cast<std::size_t>(separator - name);
    if (std::strlen(node_name) != component) {
        return nullptr;
    }
    if (_strnicmp(name, node_name, component) != 0) {
        return nullptr;  // 00925273
    }

    const char* const remainder = separator + 1;  // 00925286 ADD EBX,1
    for (int index = 0; index < children; ++index) {
        const EntityNameNode* const child = node.child_at(index);  // 00467F30
        if (child == nullptr) {
            continue;  // 00467F30 returns null past the end of the chain
        }
        const EntityNameNode* const hit =
            entity_find_by_qualified_name_009251f0(*child, remainder);
        if (hit != nullptr) {
            return hit;  // 009252A5, the first non-null wins
        }
    }
    return nullptr;
}

void* scene_cloud_instantiate_004e9e40(SceneCloudCreatorHost& host,
                                       const SceneCloudCreatorArgs& args)
{
    const void* const property = host.find_property(args.property_bag, kSceneCloudTypePropertyKey);
    if (property == nullptr) {
        return nullptr;  // 004E9E6E
    }
    const int tag = host.property_type_tag(property);
    if (tag != kScenePropertyTagStringA && tag != kScenePropertyTagStringB) {
        return nullptr;  // 004E9E7F, the 004E9F6A XOR EAX,EAX exit
    }

    // The class name is fetched and resolved twice; the first result is dropped
    // (004E9EA2), the second is the one that is used (004E9EEB).
    host.acquire_entity_class(host.property_string_value(property));
    const void* const entity_class = host.acquire_entity_class(host.property_string_value(property));

    void* const entity = host.create_entity_from_class(entity_class);  // 004E9EFC
    // 004E9F4B: the pushes at 004E9F3C / 004E9F47 / 004E9F48 are in reverse
    // argument order, so parent is first and the frame block last.
    host.attach_entity_to_scene(entity, args.parent, args.entity_registry, args.frame_block);
    host.set_entity_name(entity, args.entity_name);  // 004E9F50
    return entity;
}

bool scene_cloud_register_004e9920(SceneCloudCreatorHost& host, const void* property_bag)
{
    const void* const property = host.find_property(property_bag, kSceneCloudTypePropertyKey);
    if (property == nullptr) {
        return false;  // 004E9944
    }
    const int tag = host.property_type_tag(property);
    if (tag != kScenePropertyTagStringA && tag != kScenePropertyTagStringB) {
        return false;  // 004E9951
    }
    host.acquire_entity_class(host.property_string_value(property));  // 004E996F
    return true;
}

} // namespace bsp

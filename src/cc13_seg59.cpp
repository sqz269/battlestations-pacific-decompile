// Segment 59, packet cc13-seg59 (marker cc13_seg59). docs/CC13_SEG59.md.
#include "bsp/cc13_seg59.hpp"

namespace bsp::cc13_seg59 {
namespace {

template <class T>
T& at(void* base, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + off);
}
void* add(void* base, std::size_t off) noexcept {
    return static_cast<std::uint8_t*>(base) + off;
}

// 00904CCF-00904D03 (and the two copies at 00904D0A / 00904D45): every entity on the
// world+4h chain whose byte +5Ch is set gets vtable[DCh](0.0f); then 00904600(world, 0.0f)
// and 00874D00(0).
void drain_entities_once(Seg59Host& host, void* world) {
    void* e = at<void*>(at<void*>(world, kWorldEntityHeader), 0);
    while (e != nullptr) {
        if (at<std::uint8_t>(e, kEntityAlive5C) != 0)
            host.entity_vcall_dc(e, 0.0f);              // FLDZ, CALL [EDX+DCh]
        e = at<void*>(e, kEntityNext);
    }
    host.world_update_matrix_interpolators_00904600(world, 0.0f);
    host.game_run_extra_fixed_step_00874d00(0);
}

}  // namespace

void world_destruct_00904c40(Seg59Host& host, void* world) {
    at<std::uint32_t>(world, 0) = kWorldVtable;                     // 00904C5F
    at<std::uint8_t>(world, kWorldActiveByte) = 0;                  // 00904C67
    host.clear_byte_00e0af20();                                     // 00904C6D

    // 00904C73-00904C92: kill every mission entity, cause 7; the next link (+44h) is read
    // after the kill.
    for (void* e = at<void*>(at<void*>(world, kWorldMissionHeader), 0); e != nullptr;
         e = at<void*>(e, kMissionEntityNext))
        host.mission_entity_kill_00926d90(e, kMissionKillCause);
    host.game_run_extra_fixed_step_00874d00(0);                     // 00904C96

    // 00904C9B-00904CC6: kill each entity that has +5Eh set, +6Ch clear and either no
    // parent (+3Ch) or a parent whose +5Eh is clear. The next link (+38h) is read after.
    for (void* e = at<void*>(at<void*>(world, kWorldEntityHeader), 0); e != nullptr;
         e = at<void*>(e, kEntityNext)) {
        if (at<std::uint8_t>(e, kEntityByte5E) == 0) continue;
        if (at<std::uint32_t>(e, kEntityKilled6C) != 0) continue;
        void* parent = at<void*>(e, kEntityParent);
        if (parent != nullptr && at<std::uint8_t>(parent, kEntityByte5E) != 0) continue;
        host.scene_node_kill_00922fd0(e);
    }
    host.game_run_extra_fixed_step_00874d00(0);                     // 00904CCA

    drain_entities_once(host, world);                               // 00904CCF-00904D05
    drain_entities_once(host, world);                               // 00904D0A-00904D40
    drain_entities_once(host, world);                               // 00904D45-00904D7D

    // 00904D82-00904DA5: free both chain headers, mission first.
    if (void* h = at<void*>(world, kWorldMissionHeader)) {
        host.crt_free_00bf65ac(h);
        at<void*>(world, kWorldMissionHeader) = nullptr;
    }
    if (void* h = at<void*>(world, kWorldEntityHeader)) {
        host.crt_free_00bf65ac(h);
        at<void*>(world, kWorldEntityHeader) = nullptr;
    }

    // 00904DA8-00904DC8: release the refcounted object; at zero, its vtable slot 0.
    if (void* obj = at<void*>(world, kWorldRefObject)) {
        if (host.interlocked_decrement_ce2220(
                reinterpret_cast<volatile std::int32_t*>(add(obj, 4))) == 0)
            host.object_vcall_0(obj);
        at<void*>(world, kWorldRefObject) = nullptr;
    }

    // 00904DCE-00904E11: std::list clear at +4B4h (sentinel next = prev = self, size 0,
    // free each node), then free the sentinel itself.
    void* head = at<void*>(world, kWorldListHead);
    void* node = at<void*>(head, 0);
    at<void*>(head, 0) = head;
    at<void*>(at<void*>(world, kWorldListHead), 4) = at<void*>(world, kWorldListHead);
    at<std::uint32_t>(world, kWorldListSize) = 0;
    while (node != at<void*>(world, kWorldListHead)) {
        void* next = at<void*>(node, 0);
        host.crt_free_00bf65ac(node);
        node = next;
    }
    host.crt_free_00bf65ac(at<void*>(world, kWorldListHead));
    at<void*>(world, kWorldListHead) = nullptr;                     // 00904E21

    // 00904E2B: the 97 raw headers at +18h; 00904E33: the parent header at +0Ch.
    host.eh_vector_destructor_iterator_00bf7c6e(add(world, kWorldHeaderArray),
                                                kWorldHeaderArrayStride, kWorldHeaderArrayCount);
    host.parent_list_clear_raw_header_storage_004bf8e0(add(world, kWorldParentList));
}

void scene_property_record_map_destruct_008f4160(Seg59MapHost& host, void* object) {
    at<std::uint32_t>(object, 0) = kScenePropertyRecordMapVtable;   // 008F4160
    host.scene_property_record_map_clear_008f3f30(object);          // 008F4166 JMP
}

void enum_symbol_map_destruct_008f4d50(Seg59MapHost& host, void* object) {
    at<std::uint32_t>(object, 0) = kEnumSymbolMapVtable;            // 008F4D50
    host.enum_symbol_map_clear_008f4b60(object);                    // 008F4D56 JMP
}

void property_library_map_destruct_008f4d60(Seg59MapHost& host, void* object) {
    at<std::uint32_t>(object, 0) = kPropertyLibraryMapVtable;       // 008F4D60
    host.property_library_map_clear_008f4c50(object);               // 008F4D66 JMP
}

namespace {

// The inlined _Tree constructor: head = buy(); head->isnil byte = 1; head->parent (+4h),
// ->left (+0h), ->right (+8h) = head, in that order; size (+8h of the tree) = 0.
template <class Buy>
void tree_construct(void* tree, std::size_t isnil_offset, Buy buy) {
    void* head = buy(tree);
    at<void*>(tree, 4) = head;
    at<std::uint8_t>(head, isnil_offset) = 1;
    at<void*>(at<void*>(tree, 4), 4) = at<void*>(tree, 4);
    at<void*>(at<void*>(tree, 4), 0) = at<void*>(tree, 4);
    at<void*>(at<void*>(tree, 4), 8) = at<void*>(tree, 4);
    at<std::uint32_t>(tree, 8) = 0;
}

// The inlined _Tree destructor: erase(begin, end), free the head, zero head and size.
template <class Erase>
void tree_destruct(Seg59ScoringHost& host, void* tree, Erase erase) {
    void* head = at<void*>(tree, 4);
    erase(tree, at<void*>(head, 0), head);
    host.crt_free_00bf65ac(at<void*>(tree, 4));
    at<void*>(tree, 4) = nullptr;
    at<std::uint32_t>(tree, 8) = 0;
}

}  // namespace

void* scoring_manager_construct_0091d640(Seg59ScoringHost& host, void* m) {
    at<std::uint32_t>(m, 0) = kScoringManagerVtable;                // 0091D675
    host.eh_vector_constructor_iterator_00bf7cd1(add(m, kScoringRecords),  // 0091D67B
                                                 kScoringRecordStride, kScoringRecordCount);
    tree_construct(add(m, kScoringTree1428), 0x15,                  // 0091D68E-0091D6AC
                   [&](void* t) { return host.tree_buy_head_node_00582840(t); });
    tree_construct(add(m, kScoringTree1434), 0x1D,                  // 0091D6BB-0091D6D7
                   [&](void* t) { return host.tree_buy_head_node_00907cf0(t); });
    tree_construct(add(m, kScoringTree1440), 0x1D,                  // 0091D6E7-0091D703
                   [&](void* t) { return host.tree_buy_head_node_00907cf0(t); });
    at<std::uint32_t>(m, kScoringPoolLength) = 0;                   // 0091D706
    at<std::uint32_t>(m, kScoringPoolBlock) = 0;                    // 0091D70C
    tree_construct(add(m, kScoringTree1488), 0x11,                  // 0091D71F-0091D73F
                   [&](void* t) { return host.tree_buy_head_node_0063b110(t); });
    return m;
}

void scoring_manager_destruct_0091ccd0(Seg59ScoringHost& host, void* m) {
    at<std::uint32_t>(m, 0) = kScoringManagerVtable;                // 0091CCF1
    tree_destruct(host, add(m, kScoringTree1488),                   // 0091CCF7-0091CD2B
                  [&](void* t, void* f, void* l) { host.tree_erase_range_0063fb30(t, f, l); });
    if (void* block = at<void*>(m, kScoringPoolBlock)) {            // 0091CD2E-0091CD54
        void* pool = host.sized_storage_pool_get_singleton_00419cc0();
        host.sized_storage_pool_return_block_00bd1510(
            pool, block, at<std::uint32_t>(m, kScoringPoolLength) + 1u, 1u);
    }
    tree_destruct(host, add(m, kScoringTree1440),                   // 0091CD59-0091CD88
                  [&](void* t, void* f, void* l) { host.tree_erase_range_00911b30(t, f, l); });
    tree_destruct(host, add(m, kScoringTree1434),                   // 0091CD8B-0091CDBD
                  [&](void* t, void* f, void* l) { host.tree_erase_range_00911b30(t, f, l); });
    tree_destruct(host, add(m, kScoringTree1428),                   // 0091CDC0-0091CE04
                  [&](void* t, void* f, void* l) { host.tree_erase_range_0058ae10(t, f, l); });
    host.eh_vector_destructor_iterator_00bf7c6e(add(m, kScoringRecords),  // 0091CE0F
                                                kScoringRecordStride, kScoringRecordCount);
}

}  // namespace bsp::cc13_seg59

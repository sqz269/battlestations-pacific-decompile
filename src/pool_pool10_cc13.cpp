// cc13 pool10 (segment 6): the BrowserGroup entry reader, three visual-class CreateEntity slots
// and the AnimClass / Bullets / BuoyClass registry constructors and destructor bodies.
// Descriptive names are hypotheses, not recovered symbols.
//
// Evidence common to every body below (cc13-reltimer, 2026-10-24):
// - Listings read from the disk image with `bsp.py disasm-raw`; each body runs from its entry to
//   the RET named on it (CC padding follows, except 004694F0, which the next function abuts). No register inputs besides ECX, no x87, no
//   overlapping globals; every callee returns.
// - EH: each body has a C++ EH frame (push -1 / push handler / fs:[0]). The unwind maps were
//   decoded from the FuncInfo the handler loads (local/p10_eh.py in the cc13-pool10 tree); the
//   state actions are quoted on each body and modelled as catch-and-rethrow around the calls a
//   host might throw from.
// - The ledger rows were metadata_only_no_function_body_verified_oct24 (no body credited).
//   engine_seg4_6_9_cc13.cpp describes the same functions as table rows (kClassCreateKinds,
//   the registry table); that table is not a body and is left as it is.

#include "bsp/pool_pool10_cc13.hpp"


namespace bsp::cc13_pool10 {
namespace {

// The inline NativeString release the image emits after each local string: when data (+4) is
// non-null, 00419CC0() then 00BD1510(ECX = pool, data, size + 1, 1). The three arguments are
// pushed before the 00419CC0 call, which leaves them for 00BD1510.
void release_native_string(Pool10Host& host, Object string) {
    void* data = at<void*>(string, 4);
    if (data != nullptr) {
        const std::uint32_t bytes = at<std::uint32_t>(string, 0) + 1;
        Object pool = host.sized_pool_get_00419cc0();
        host.sized_pool_return_block_00bd1510(pool, data, bytes, 1);
    }
}

// The CreateEntity shape shared by 00470C90, 004728F0 and 00474160: operator new and _memset
// before the null test (_memset 00BF79F0 runs on the result untested; 00BF55BE is JMP 00BF681B), the
// base constructor 00489F30 and the three vtable stores and class id under EH state 0 (unwind:
// MOV ECX,[EBP-10h] / JMP 0042B100, operator delete of the new block), then, also on the null
// path, 0047B9E0(ECX = entity, class) and entity+188h = class.
struct CreateKind {
    std::uint32_t bytes;
    std::uint32_t vtable_0;
    std::uint32_t vtable_10;
    std::uint32_t vtable_24;
    std::uint32_t class_id;
};

Object create_class_entity(Pool10Host& host, Object entity_class, const CreateKind& kind) {
    Object p = host.operator_new_00bf55be(kind.bytes);
    // A host call, not std::memset: memset on a null result would be UB in C++ and could let
    // the optimiser drop the null test below.
    host.memset_00bf79f0(p, 0, kind.bytes);
    if (p != nullptr) {
        try {
            host.class_entity_construct_base_00489f30(p);
        } catch (...) {
            host.operator_delete_0042b100(p);
            throw;
        }
        at<std::uint32_t>(p, 0x0) = kind.vtable_0;
        at<std::uint32_t>(p, 0x10) = kind.vtable_10;
        at<std::uint32_t>(p, 0x24) = kind.vtable_24;
        at<std::uint32_t>(p, 0xc4) = kind.class_id;
    }
    host.class_entity_set_class_0047b9e0(p, entity_class);
    at<Object>(p, 0x188) = entity_class;
    return p;
}

// The registry constructor shape shared by 00472100, 004739F0 and 00475540. The registry is
// {vtable +0, std::map +4 = {allocator +4, head +8, size +0Ch}}. EH state 0 is entered before
// the vtable store (unwind: MOV ECX,[EBP-10h] / JMP to the registry's base destructor). The
// head node gets isnil = 1 and left = parent = right = itself, re-read from +8 for each store.
Object construct_registry(Pool10Host& host, Object registry, std::uint32_t vtable,
                          Object (Pool10Host::*buy_head)(Object), std::size_t isnil_offset,
                          void (Pool10Host::*base_destructor)(Object)) {
    Object tree = registry + 4;
    at<std::uint32_t>(registry, 0) = vtable;
    Object head = nullptr;
    try {
        head = (host.*buy_head)(tree);
    } catch (...) {
        (host.*base_destructor)(registry);
        throw;
    }
    at<Object>(tree, 4) = head;
    at<std::uint8_t>(head, isnil_offset) = 1;
    at<Object>(at<Object>(tree, 4), 4) = at<Object>(tree, 4);
    at<Object>(at<Object>(tree, 4), 0) = at<Object>(tree, 4);
    at<Object>(at<Object>(tree, 4), 8) = at<Object>(tree, 4);
    at<std::uint32_t>(tree, 8) = 0;
    return registry;
}

// The registry destructor-body shape shared by 00472170 and 00473A60: erase(begin, end) with
// begin = {tree, [head]} (leftmost) and end = {tree, head} under EH state 0 (unwind:
// MOV ECX,[EBP-18h] / JMP to the base destructor), _free(head), head = size = 0, then the base
// destructor inlined: the singleton pointer = 0 and vtable 00CE3818. The mapped class objects are
// not deleted.
void destruct_registry(Pool10Host& host, Object registry,
                       void (Pool10Host::*erase)(Object, TreeIterator*, TreeIterator, TreeIterator),
                       std::uint32_t singleton, void (Pool10Host::*base_destructor)(Object)) {
    Object tree = registry + 4;
    Object head = at<Object>(registry, 8);
    TreeIterator result{};
    try {
        (host.*erase)(tree, &result, TreeIterator{tree, at<Object>(head, 0)}, TreeIterator{tree, head});
    } catch (...) {
        (host.*base_destructor)(registry);
        throw;
    }
    host.free_00bf65ac(at<Object>(tree, 4));
    at<Object>(tree, 4) = nullptr;
    at<std::uint32_t>(tree, 8) = 0;
    host.global_u32(singleton) = 0;
    at<std::uint32_t>(registry, 0) = 0x00ce3818;
}

}  // namespace

// 004694F0 BSP_SceneFile_ReadBrowserGroupEntry, __thiscall(ECX never read, SceneTokenizer*),
// RET 4 at 0046960D (80 instructions); sole caller 00469E40 at 00469E7A. Every tokenizer call
// has ECX = the tokenizer (ESI, loaded from [ESP+24h] at 00469509). Literals: 00CE5654
// "BrowserGroup", 00CE4D38 "{", 00CE55C0 "GroupName", 00CE55BC "=", 00CE564C "GroupID",
// 00CE5644 "Parent", 00CE4CD4 "}". Two 8-byte NativeStrings {size, data}: the group name at
// entry-14h (0041E870 at 00469549, EH state 0) and the parent at entry-1Ch (0041E870 at
// 0046959E, state 1). 008D9AD0's destination is LEA EAX,[ESP+24h] at 0046956E = entry+4, the
// tokenizer argument slot itself (ESI already holds the pointer), so the integer is dropped;
// the C++ uses a local. Nothing is stored: after the closing "}" the parent string is released
// (state back to 0), then the name (state -1), each through the inline pool release.
// Unwind map (FuncInfo 00D89154): state 1 -> 0 runs 0041DD20 on the parent string, state 0 -> -1
// runs 0041DD20 on the name string.
// Host overlap: src/scene_traffic_groups.cpp (read_browser_groups_after_keyword, the loop body
// at "004694F0: eleven ExpectToken calls") parses the same grammar over SceneLexer in the live
// path under kSceneBrowserGroupsBound, and keeps the entries. This body is not wired in.
void scene_file_read_browser_group_entry_004694f0(Pool10Host& host, Object tokenizer) {
    alignas(4) std::uint8_t name[8] = {};
    alignas(4) std::uint8_t parent[8] = {};
    std::int32_t group_id = 0;
    host.scene_tokenizer_expect_token_008d9930(tokenizer, "BrowserGroup");
    host.scene_tokenizer_expect_token_008d9930(tokenizer, "{");
    host.scene_tokenizer_expect_token_008d9930(tokenizer, "GroupName");
    host.scene_tokenizer_expect_token_008d9930(tokenizer, "=");
    host.native_string_construct_0041e870(name, host.scene_tokenizer_read_token_008d9980(tokenizer));
    try {
        host.scene_tokenizer_expect_token_008d9930(tokenizer, "GroupID");
        host.scene_tokenizer_expect_token_008d9930(tokenizer, "=");
        host.scene_tokenizer_read_int_008d9ad0(tokenizer, &group_id);
        host.scene_tokenizer_expect_token_008d9930(tokenizer, "Parent");
        host.scene_tokenizer_expect_token_008d9930(tokenizer, "=");
        host.native_string_construct_0041e870(parent, host.scene_tokenizer_read_token_008d9980(tokenizer));
    } catch (...) {
        host.native_string_destroy_storage_0041dd20(name);
        throw;
    }
    try {
        host.scene_tokenizer_expect_token_008d9930(tokenizer, "}");
    } catch (...) {
        host.native_string_destroy_storage_0041dd20(parent);
        host.native_string_destroy_storage_0041dd20(name);
        throw;
    }
    release_native_string(host, parent);
    release_native_string(host, name);
}

// 00470C90 BSP_AnimClass_CreateEntity, AnimClass vtable slot +8h, __thiscall(class), RET at
// 00470D27 (41 instructions), EAX = entity. operator new(194h), vtables 00CE5A20 / 00CE5A08
// (+10h) / 00CE5A00 (+24h), class id +C4h = 3Eh. No 00926BE0 call (unlike Bullets).
// FuncInfo 00D89840.
Object anim_class_create_entity_00470c90(Pool10Host& host, Object anim_class) {
    return create_class_entity(host, anim_class, {0x194, 0x00ce5a20, 0x00ce5a08, 0x00ce5a00, 0x3e});
}

// 004728F0 BSP_BulletVisualClass_CreateEntity, Bullets visual class vtable slot +8h,
// __thiscall(class), RET at 0047298E (43 instructions), EAX = entity. operator new(18Ch),
// vtables 00CE5B90 / 00CE5B78 / 00CE5B70, class id 39h; after +188h = class it calls
// 00926BE0(ECX = entity) at 00472977. FuncInfo 00D89A20. Like 0047B9E0 and the +188h store,
// the 00926BE0 call also runs on the null path (ESI = 0 from 0047295D), as the image does.
Object bullet_visual_class_create_entity_004728f0(Pool10Host& host, Object bullet_class) {
    Object p = create_class_entity(host, bullet_class, {0x18c, 0x00ce5b90, 0x00ce5b78, 0x00ce5b70, 0x39});
    host.sentity_push_pending_00926be0(p);
    return p;
}

// 00474160 BSP_BuoyClass_CreateEntity, BuoyClass vtable slot +8h, __thiscall(class), RET at
// 004741F7 (41 instructions), EAX = entity. operator new(18Ch), vtables 00CE5CF0 / 00CE5CD8 /
// 00CE5CD0, class id 3Ch. No 00926BE0 call. FuncInfo 00D89BA0.
Object buoy_class_create_entity_00474160(Pool10Host& host, Object buoy_class) {
    return create_class_entity(host, buoy_class, {0x18c, 0x00ce5cf0, 0x00ce5cd8, 0x00ce5cd0, 0x3c});
}

// 00472100 BSP_AnimClassRegistry_Construct, __thiscall(registry), RET at 00472167
// (31 instructions), EAX = registry; only caller 00472200 (GetSingleton, which stores
// 00E18688). Vtable 00CE5B60, head node 00471210 (1Ch bytes), isnil +19h. Unwind (FuncInfo
// 00D89950): 00470D30.
Object anim_class_registry_construct_00472100(Pool10Host& host, Object registry) {
    return construct_registry(host, registry, 0x00ce5b60, &Pool10Host::tree_buy_head_node_00471210,
                              0x19, &Pool10Host::anim_class_registry_clear_singleton_00470d30);
}

// 00472170 BSP_AnimClassRegistry_DestructBody, __thiscall(registry), RET at 004721DE
// (39 instructions); called by the scalar deleting destructor 004721E0 (vtable 00CE5B60).
// Erase 00471C50 at 004721AA, _free at 004721B3, singleton 00E18688 cleared at 004721C5.
// Unwind (FuncInfo 00D8997C): 00470D30.
void anim_class_registry_destruct_body_00472170(Pool10Host& host, Object registry) {
    destruct_registry(host, registry, &Pool10Host::tree_erase_range_00471c50, 0x00e18688,
                      &Pool10Host::anim_class_registry_clear_singleton_00470d30);
}

// 004739F0 BSP_BulletVisualClassRegistry_Construct, __thiscall(registry), RET at 00473A57
// (31 instructions), EAX = registry; only caller 00473AF0 (singleton 00E18690). Vtable 00CE5CC4,
// head node 00472E10 (18h bytes, so isnil is +15h). Unwind (FuncInfo 00D89AD0): 00472990.
Object bullet_visual_class_registry_construct_004739f0(Pool10Host& host, Object registry) {
    return construct_registry(host, registry, 0x00ce5cc4, &Pool10Host::tree_buy_head_node_00472e10,
                              0x15, &Pool10Host::bullet_visual_class_registry_clear_singleton_00472990);
}

// 00473A60 BSP_BulletVisualClassRegistry_DestructBody, __thiscall(registry), RET at 00473ACE
// (39 instructions); called by the scalar deleting destructor 00473AD0 (vtable 00CE5CC4).
// Erase 004735F0 at 00473A9A, _free at 00473AA3, singleton 00E18690 cleared at 00473AB5.
// Unwind (FuncInfo 00D89AFC): 00472990.
void bullet_visual_class_registry_destruct_body_00473a60(Pool10Host& host, Object registry) {
    destruct_registry(host, registry, &Pool10Host::tree_erase_range_004735f0, 0x00e18690,
                      &Pool10Host::bullet_visual_class_registry_clear_singleton_00472990);
}

// 00475540 BSP_BuoyClassRegistry_Construct, __thiscall(registry), RET at 004755A7
// (31 instructions), EAX = registry; only caller 00475640 (singleton 00E18698). Vtable 00CE5E38,
// head node 00474650 (1Ch bytes), isnil +19h. Unwind (FuncInfo 00D89CB0): 00474220.
Object buoy_class_registry_construct_00475540(Pool10Host& host, Object registry) {
    return construct_registry(host, registry, 0x00ce5e38, &Pool10Host::tree_buy_head_node_00474650,
                              0x19, &Pool10Host::buoy_class_registry_clear_singleton_00474220);
}

}  // namespace bsp::cc13_pool10

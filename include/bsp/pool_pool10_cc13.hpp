#pragma once

// cc13 pool10 (segment 6, 004694F0..004755A7): the BrowserGroup entry reader, the CreateEntity
// slots of the AnimClass, Bullets and BuoyClass visual classes, and the constructors and
// destructor bodies of their class registries. Descriptive names are hypotheses, not recovered
// symbols.
//
// Objects are reached through byte offsets in their Win32 layout (Object plus at<T>). Every call
// the image makes goes through a Pool10Host method named for its target address, at the same
// point and with the same arguments; library code (operator new, _free, the std::map head-node
// and erase instantiations) is bound there with no credit. Build only: no host path calls these.

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_pool10 {

static_assert(sizeof(void*) == 4, "segment 6 objects are modelled in their Win32 layout");

using Object = std::uint8_t*;

template <class T>
inline T& at(Object object, std::size_t offset) noexcept {
    return *reinterpret_cast<T*>(object + offset);
}

// A checked std::map iterator as the image passes it by value: {container, node}.
struct TreeIterator {
    Object container;
    Object node;
};

struct Pool10Host {
    virtual ~Pool10Host() = default;

    // Class entities.
    virtual Object operator_new_00bf55be(std::uint32_t bytes) = 0;                         // cdecl
    virtual void class_entity_construct_base_00489f30(Object entity) = 0;                   // ECX = entity
    virtual void class_entity_set_class_0047b9e0(Object entity, Object entity_class) = 0;    // RET 4
    virtual void sentity_push_pending_00926be0(Object entity) = 0;                           // ECX = entity
    virtual void operator_delete_0042b100(Object entity) = 0;  // EH unwind only, ECX = entity

    // Registry base destructors (stores 0 to the singleton pointer, vtable 00CE3818), reached
    // from these bodies only through EH unwind funclets. ECX = registry.
    virtual void anim_class_registry_clear_singleton_00470d30(Object registry) = 0;
    virtual void bullet_visual_class_registry_clear_singleton_00472990(Object registry) = 0;
    virtual void buoy_class_registry_clear_singleton_00474220(Object registry) = 0;

    // Scene tokenizer and NativeString {size +0, data +4}.
    virtual void scene_tokenizer_expect_token_008d9930(Object tokenizer, const char* literal) = 0;  // RET 4
    virtual const char* scene_tokenizer_read_token_008d9980(Object tokenizer) = 0;                 // RET
    virtual void scene_tokenizer_read_int_008d9ad0(Object tokenizer, std::int32_t* out) = 0;       // RET 4
    virtual void native_string_construct_0041e870(Object string, const char* text) = 0;            // RET 4
    virtual void native_string_destroy_storage_0041dd20(Object string) = 0;  // EH unwind only
    virtual Object sized_pool_get_00419cc0() = 0;
    virtual void sized_pool_return_block_00bd1510(Object pool, void* data, std::uint32_t bytes,
                                                  std::uint32_t flag) = 0;                         // RET 0Ch

    // std::map instantiations of the three class registries (library, bound).
    virtual Object tree_buy_head_node_00471210(Object tree) = 0;  // AnimClass, 1Ch-byte node
    virtual Object tree_buy_head_node_00472e10(Object tree) = 0;  // Bullets, 18h-byte node
    virtual Object tree_buy_head_node_00474650(Object tree) = 0;  // BuoyClass, 1Ch-byte node
    virtual void tree_erase_range_00471c50(Object tree, TreeIterator* result, TreeIterator first,
                                           TreeIterator last) = 0;  // RET 14h
    virtual void tree_erase_range_004735f0(Object tree, TreeIterator* result, TreeIterator first,
                                           TreeIterator last) = 0;  // RET 14h
    virtual void free_00bf65ac(void* block) = 0;                    // cdecl

    // A dword global of the image, by address.
    virtual std::uint32_t& global_u32(std::uint32_t address) = 0;
};

// 004694F0 __thiscall(ECX unused, SceneTokenizer*), RET 4: parses and discards one
// BrowserGroup { GroupName = TOKEN GroupID = INT Parent = TOKEN } entry.
void scene_file_read_browser_group_entry_004694f0(Pool10Host& host, Object tokenizer);

// CreateEntity, visual-class vtable slot +8h: __thiscall(class), RET, EAX = new entity.
Object anim_class_create_entity_00470c90(Pool10Host& host, Object anim_class);
Object bullet_visual_class_create_entity_004728f0(Pool10Host& host, Object bullet_class);
Object buoy_class_create_entity_00474160(Pool10Host& host, Object buoy_class);

// Registry constructors __thiscall(registry), RET, EAX = registry; destructor bodies
// __thiscall(registry), RET.
Object anim_class_registry_construct_00472100(Pool10Host& host, Object registry);
void anim_class_registry_destruct_body_00472170(Pool10Host& host, Object registry);
Object bullet_visual_class_registry_construct_004739f0(Pool10Host& host, Object registry);
void bullet_visual_class_registry_destruct_body_00473a60(Pool10Host& host, Object registry);
Object buoy_class_registry_construct_00475540(Pool10Host& host, Object registry);

}  // namespace bsp::cc13_pool10

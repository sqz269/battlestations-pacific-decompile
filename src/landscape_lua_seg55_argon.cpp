#include "bsp/landscape_lua_seg55_argon.hpp"
#include "bsp/native_damageable_fake_effect_vector.hpp"

#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif

namespace bsp::landscape_lua_seg55_argon {

namespace {

inline std::int32_t atomic_decrement_ref(std::int32_t& ref_count) noexcept {
#ifdef _WIN32
    return static_cast<std::int32_t>(
        InterlockedDecrement(reinterpret_cast< volatile LONG* >(&ref_count)));
#else
    return --ref_count;
#endif
}

inline int ascii_stricmp(const char* a, const char* b) noexcept {
    while (*a != '\0' && *b != '\0') {
        unsigned char ca = static_cast<unsigned char>(*a);
        unsigned char cb = static_cast<unsigned char>(*b);
        if (ca >= 'A' && ca <= 'Z') {
            ca = static_cast<unsigned char>(ca + ('a' - 'A'));
        }
        if (cb >= 'A' && cb <= 'Z') {
            cb = static_cast<unsigned char>(cb + ('a' - 'A'));
        }
        if (ca != cb) {
            return static_cast<int>(ca) - static_cast<int>(cb);
        }
        ++a;
        ++b;
    }
    unsigned char ca = static_cast<unsigned char>(*a);
    unsigned char cb = static_cast<unsigned char>(*b);
    if (ca >= 'A' && ca <= 'Z') {
        ca = static_cast<unsigned char>(ca + ('a' - 'A'));
    }
    if (cb >= 'A' && cb <= 'Z') {
        cb = static_cast<unsigned char>(cb + ('a' - 'A'));
    }
    return static_cast<int>(ca) - static_cast<int>(cb);
}

} // namespace

// ============================================================================
// 1. STL Tree & Vector Helpers + DamageableSection / DamageableClass /
//    UnitTickableEntity (00878530..0087C700)
// ============================================================================

// 00878530 STL_Tree_Inc_00878530
// __thiscall(ECX = iterator*), RET.
// Calls 008772B0 (00878533) and returns `it` in EAX.
DamageableTreeIterator* stl_tree_inc_00878530(
    DamageableTreeIterator* it,
    DamageableTreeHost& host) {
    host.advance_iterator_008772b0(*it);
    return it;
}

// 00878610 STL_Tree_InitHead_00878610
// __thiscall(ECX = tree*), RET.
// Calls 00877FA0 std_Tree_Buynode (00878613), stores head at +04h, sets
// head->is_nil (+55h) = 1, self-links parent (+04h), left (+00h), and right (+08h),
// and zeroes size (+08h).
void stl_tree_init_head_00878610(
    DamageableTree& tree,
    DamageableTreeHost& host) {
    DamageableTreeNode* head = host.buy_head_node_00877fa0();
    tree.head = head;
    head->is_nil = true;
    tree.head->parent = tree.head;
    tree.head->left = tree.head;
    tree.head->right = tree.head;
    tree.size = 0;
}

// 00878A90 STL_Tree_Construct_00878A90
// __thiscall(ECX = tree*, comp, alloc), RET 8.
// Calls 00877FA0 (00878A93), initializes head and size, and returns `&tree`.
DamageableTree* stl_tree_construct_00878a90(
    DamageableTree& tree,
    DamageableTreeHost& host) {
    stl_tree_init_head_00878610(tree, host);
    return &tree;
}

// 0087C2A0 STL_Tree_Construct_0087C2A0
// __thiscall(ECX = tree*), RET.
// Calls 00877FA0 (0087C2A3), initializes head and size, and returns `&tree`.
DamageableTree* stl_tree_construct_0087c2a0(
    DamageableTree& tree,
    DamageableTreeHost& host) {
    stl_tree_init_head_00878610(tree, host);
    return &tree;
}

// 008786C0 STL_UnitPartPointerVector_CopyBackward
// __cdecl(first, last, dest_end), RET.
// Copies dwords backward from `[first, last)` into `[dest_begin, dest_end)` and
// returns `dest_end - ((last - first) >> 2)`.
std::uint32_t* stl_unit_part_pointer_vector_copy_backward_008786c0(
    const std::uint32_t* first,
    const std::uint32_t* last,
    std::uint32_t* dest_end) noexcept {
    const std::ptrdiff_t count = last - first;
    std::uint32_t* result = dest_end - count;
    const std::uint32_t* cur = last;
    std::uint32_t* out = dest_end;
    while (cur != first) {
        --cur;
        --out;
        *out = *cur;
    }
    return result;
}

// 00878BB0 STL_UnitPartPointerVector_FillConstructCount
// __thiscall(ECX = vec, dest, count, val_slot), RET 0Ch.
// Calls 00878190 (00878BD4) with ECX = dest, EDX = count, and returns `dest + count`.
std::uint32_t* stl_unit_part_pointer_vector_fill_construct_count_00878bb0(
    std::uint32_t* dest,
    std::size_t count,
    const std::uint32_t* val_slot,
    UnitPartPointerVectorHost& host) {
    host.fill_construct_loop_00878190(dest, count, val_slot);
    return dest + count;
}

// 008793C0 STL_UnitPartPointerVector_CopyConstructWrapper
// __thiscall(ECX = vec, first, last, dest), RET 0Ch.
// Calls 008776B0 (008793DC) with ECX = first, EDX = last, dest on stack.
std::uint32_t* stl_unit_part_pointer_vector_copy_construct_wrapper_008793c0(
    const std::uint32_t* first,
    const std::uint32_t* last,
    std::uint32_t* dest,
    UnitPartPointerVectorHost& host) {
    return host.copy_construct_range_008776b0(first, last, dest);
}

// 0087AF40 STL_UnitPartPointerVector_InsertCopies
// __thiscall(ECX = vec, unused_iter_proxy, where_ptr, count, val_ptr), RET 10h.
// Captures `*val_ptr` at 0087AF62, checks `0x3FFFFFFF - size < count` (-> 0087A7E0 at
// 0087AFA4), grows capacity by 1.5x via 00876A80 (0087B005) when `cap < size + count`,
// or inserts in place via 008793C0 / 00878BB0 / 008786C0 / 008780F0.
void stl_unit_part_pointer_vector_insert_copies_0087af40(
    UnitPartPointerVector& vec,
    std::uint32_t* where_ptr,
    std::size_t count,
    const std::uint32_t* val_ptr,
    UnitPartPointerVectorHost& host) {
    const std::uint32_t captured_val = (val_ptr != nullptr) ? *val_ptr : 0u;
    const std::size_t cap =
        (vec.first != nullptr) ? static_cast<std::size_t>(vec.end_cap - vec.first) : 0u;
    if (count == 0u) {
        return;
    }
    const std::size_t cur_size =
        (vec.first != nullptr) ? static_cast<std::size_t>(vec.last - vec.first) : 0u;
    if (0x3FFFFFFFu - cur_size < count) {
        host.on_xlen_0087a7e0();
    }
    if (cap < cur_size + count) {
        std::size_t new_cap = (0x3FFFFFFFu - (cap >> 1) < cap) ? 0u : (cap + (cap >> 1));
        if (new_cap < cur_size + count) {
            new_cap = cur_size + count;
        }
        std::uint32_t* new_buf = host.allocate_00876a80(new_cap);
        std::uint32_t* mid = host.copy_construct_range_008776b0(vec.first, where_ptr, new_buf);
        mid = stl_unit_part_pointer_vector_fill_construct_count_00878bb0(
            mid, count, &captured_val, host);
        host.copy_construct_range_008776b0(where_ptr, vec.last, mid);
        if (vec.first != nullptr) {
            host.free_00bf65ac(vec.first);
        }
        vec.end_cap = new_buf + new_cap;
        vec.last = new_buf + cur_size + count;
        vec.first = new_buf;
        return;
    }
    const std::size_t tail_elems = static_cast<std::size_t>(vec.last - where_ptr);
    if (tail_elems < count) {
        stl_unit_part_pointer_vector_copy_construct_wrapper_008793c0(
            where_ptr, vec.last, where_ptr + count, host);
        stl_unit_part_pointer_vector_fill_construct_count_00878bb0(
            vec.last, count - tail_elems, &captured_val, host);
        vec.last += count;
        host.assign_range_008780f0(where_ptr, vec.last - count, &captured_val);
        return;
    }
    std::uint32_t* split = vec.last - count;
    std::uint32_t* new_last = stl_unit_part_pointer_vector_copy_construct_wrapper_008793c0(
        split, vec.last, vec.last, host);
    vec.last = new_last;
    stl_unit_part_pointer_vector_copy_backward_008786c0(where_ptr, split, new_last - count);
    host.assign_range_008780f0(where_ptr, where_ptr + count, &captured_val);
}

// 00878BF0 STL_Copy16_00878BF0
// __cdecl(first, last, dest), RET.
// Calls 00878820 (00878C1E) and returns `dest + ((last - first) >> 4) * 16`.
// 00878820 is the per-row assignment loop already reconstructed in
// native_damageable_fake_effect_vector.cpp: FLD/FSTP +00h, copies +04h/+08h, and when
// the +0Ch owner differs stores it, InterlockedIncrement(new+4) [00CE221C], then
// InterlockedDecrement(old+4) [00CE2220] and on zero old->vslot 0 (00878820..00878877).
// COMDAT twin of 00879330.
FakeEffectRow16* stl_copy16_00878bf0(
    const FakeEffectRow16* first,
    const FakeEffectRow16* last,
    FakeEffectRow16* dest) {
    bsp::copy_native_damageable_fake_effect_rows_00878820(first, last, dest);
    return dest + (last - first);
}

// 00878D10 STL_CopyBackward16_00878D10
// __cdecl(first, last, dest_end), RET.
// Calls 008788D0 (00878D3E) and returns `dest_end - ((last - first) >> 4) * 16`.
// 008788D0 is the backward form of the same refcounting row assignment as 00878820
// (008788F0..00878936: identical +0Ch increment/decrement/vslot-0 release), reused from
// native_damageable_fake_effect_vector.cpp. COMDAT twin of 00879870.
FakeEffectRow16* stl_copy_backward16_00878d10(
    const FakeEffectRow16* first,
    const FakeEffectRow16* last,
    FakeEffectRow16* dest_end) {
    bsp::copy_backward_native_damageable_fake_effect_rows_008788d0(first, last, dest_end);
    return dest_end - (last - first);
}

// 008793F0 STL_CopyBackward16_008793F0
// __cdecl(first, last, dest_end), RET.
// Calls 008788D0 (0087941E); identical COMDAT twin of 00878D10.
FakeEffectRow16* stl_copy_backward16_008793f0(
    const FakeEffectRow16* first,
    const FakeEffectRow16* last,
    FakeEffectRow16* dest_end) {
    return stl_copy_backward16_00878d10(first, last, dest_end);
}

// 00878E10 STL_Construct_DamageableSection_00878E10
// __fastcall(ECX = place, EDX = src), RET.
// If `place != nullptr`, calls 00878B40 BSP_DamageableSection_CopyConstruct (00878E3C).
void stl_construct_damageable_section_00878e10(
    DamageableSection* place,
    const DamageableSection* src,
    DamageableSectionHost& host) {
    if (place != nullptr && src != nullptr) {
        host.copy_construct_00878b40(*place, *src);
    }
}

// 00879470 STL_Construct_DamageableSection_00879470
// __stdcall(place, src), RET 8.
// If `place != nullptr`, calls 00878B40 BSP_DamageableSection_CopyConstruct (008794A2).
void stl_construct_damageable_section_00879470(
    DamageableSection* place,
    const DamageableSection* src,
    DamageableSectionHost& host) {
    if (place != nullptr && src != nullptr) {
        host.copy_construct_00878b40(*place, *src);
    }
}

// 00878EB0 BSP_DamageableSection_ConstructDefault
// __fastcall(ECX = section), RET.
// Stores vptr 00D0DF04 (00878EB7), zeroes +24h (00878EBE), calls 00BF79F0 _memset(this, 0, 0x30)
// (00878EC5), and writes 10.0f (`[00CE38B8]`) to +28h and +2Ch.
DamageableSection* damageable_section_construct_default_00878eb0(
    DamageableSection& section) noexcept {
    section.vptr = 0x00D0DF04u;
    section.field_24 = 0u;
    std::memset(&section, 0, sizeof(DamageableSection));
    section.threshold_28 = 10.0f;
    section.threshold_2c = 10.0f;
    return &section;
}

// 008794C0 TMPL_RefPtr_ReleaseAt0C_008794C0
// __fastcall(ECX = owner), RET.
// If `[owner + 0Ch]` is non-null, decrements `[ptr + 04h]`, invokes vslot 0 at zero,
// and zeroes `[owner + 0Ch]`.
void tmpl_ref_ptr_release_at_0c_008794c0(RefCountedObject*& slot_at_0c) {
    if (slot_at_0c != nullptr) {
        RefCountedObject* obj = slot_at_0c;
        if (atomic_decrement_ref(obj->ref_count) == 0) {
            obj->destroy_vslot0();
        }
        slot_at_0c = nullptr;
    }
}

// 0087B8B0 STL_Vector_Tidy_0087B8B0
// __fastcall(ECX = vec), RET.
// If `vec.first != nullptr`, calls 0087AB30 (0087B8C4) and 00BF65AC `_free` (0087B8CD),
// then zeroes +04h, +08h, +0Ch.
void stl_vector_tidy_0087b8b0(
    DamageableOwnerVector& vec,
    DamageableClassHost& host) {
    if (vec.first != nullptr) {
        host.release_owner_rows_0087ab30(vec.first, vec.last, vec);
        host.free_mem(vec.first);
    }
    vec.first = nullptr;
    vec.last = nullptr;
    vec.end_cap = nullptr;
}

// 0087C700 BSP_DamageableClass_Destruct
// __thiscall(ECX = self), RET.
// Restores vptr 00D0E13C, frees +54h (0087C739), releases refcounted +50h, erases
// frame tree +60h via 0087AE70 (0087C77C) and frees head (0087C785), returns pool
// strings at +58h/+5Ch (0087C7A9/0087C7B0) and +38h/+3Ch (0087C7CB/0087C7D2),
// reserves 0 on +28h when +30h < 0 (0087C7E7), decrements +2Ch to 0 and frees +28h
// (0087C800), calls 00879240 on +18h (0087C810), releases owner rows at +08h..+14h
// (0087C82C / 0087C835), and calls 00BD30F0 (0087C850).
void damageable_class_destruct_0087c700(
    DamageableClassState& self,
    DamageableClassHost& host) {
    self.vptr = 0x00D0E13Cu;
    if (self.buf_54 != nullptr) {
        host.free_mem(self.buf_54);
        self.buf_54 = nullptr;
    }
    if (self.ref_50 != nullptr) {
        if (atomic_decrement_ref(self.ref_50->ref_count) == 0) {
            self.ref_50->destroy_vslot0();
        }
        self.ref_50 = nullptr;
    }
    host.erase_frame_tree_0087ae70(self.frame_tree);
    host.free_mem(self.frame_tree.head);
    self.frame_tree.head = nullptr;
    self.frame_tree.size = 0u;
    if (self.str_5c_data != nullptr) {
        host.return_pool_block_00bd1510(self.str_5c_data, self.str_58_len + 1u);
    }
    if (self.str_3c_data != nullptr) {
        host.return_pool_block_00bd1510(self.str_3c_data, self.str_38_len + 1u);
    }
    if (self.points_capacity_sign < 0) {
        host.reserve_points_capacity_0074d190(0);
    }
    while (self.points_count > 0) {
        --self.points_count;
    }
    self.points_count = 0;
    host.free_mem(self.points_buf);
    host.destroy_effect_vector_00879240();
    stl_vector_tidy_0087b8b0(self.owner_rows, host);
    host.destroy_refcounted_base_00bd30f0();
}

// 00878990 BSP_UnitTickableEntity_OnKilled
// __thiscall(ECX = ent), RET.
// Calls 008781F0(0) (00878995) and tail-jumps 0077D270 (0087899D).
void unit_tickable_entity_on_killed_00878990(
    UnitTickableEntityState& ent,
    UnitTickableEntityHost& host) {
    host.set_scene_nodes_visible_008781f0(ent, false);
    host.on_killed_lua_0077d270(ent);
}

// 00878F80 BSP_UnitTickableEntity_ReleaseSceneNodes
// __fastcall(ECX = ent), RET.
// Deletes each non-null node in `ent.scene_nodes` (+380h..+384h) via vslot +04h(1),
// erases the vector, and recurses on children at +48h (walking +44h) with vslot +5Ch(4) != 0.
void unit_tickable_entity_release_scene_nodes_00878f80(
    UnitTickableEntityState& ent) {
    for (std::size_t i = 0; i < ent.scene_nodes.size(); ++i) {
        if (ent.scene_nodes[i] != nullptr) {
            ent.scene_nodes[i]->scalar_delete_vslot4(1);
            ent.scene_nodes[i] = nullptr;
        }
    }
    ent.scene_nodes.clear();
    for (UnitTickableEntityState* child = ent.first_child_48;
         child != nullptr;
         child = child->next_sibling_44) {
        if (child->is_kind_4) {
            unit_tickable_entity_release_scene_nodes_00878f80(*child);
        }
    }
}

// 008797B0 BSP_UnitInstance_ReleaseDamageStateInstance
// __thiscall(ECX = ent), RET.
// Calls vslot +18h() twice and compares with `[00E188DC]`; when equal calls
// 004BCA80 (008797D5); calls 00878F80 (008797DC); deletes `[ent+360h]` via vslot +04h(1)
// and zeroes it; tail-jumps 009277F0 (00879801).
void unit_instance_release_damage_state_instance_008797b0(
    UnitTickableEntityState& ent,
    const void* controlled_render_node_00e188dc,
    UnitTickableEntityHost& host) {
    if (ent.primary_render_node != nullptr &&
        ent.primary_render_node == controlled_render_node_00e188dc) {
        host.controlled_listener_publish_004bca80();
    }
    unit_tickable_entity_release_scene_nodes_00878f80(ent);
    if (ent.damage_state_instance != nullptr) {
        ent.damage_state_instance->scalar_delete_vslot4(1);
        ent.damage_state_instance = nullptr;
    }
    host.base_release_009277f0(ent);
}

// 0087B670 BSP_UnitTickableEntity_Construct
// __thiscall(ECX = ent, flag), RET 4.
// Calls 0077EED0(flag) (0087B694) and 00875890(ECX = ent+310h, ent, 0) (0087B6A9),
// installs the six vtables, zeroes +348h..+350h and +380h..+388h, sets +35Ch = -1,
// +364h = 99.9f (00D0E12C), +374h = 0x3F, and +C4h = 4.
UnitTickableEntityState* unit_tickable_entity_construct_0087b670(
    UnitTickableEntityState& ent,
    std::uint32_t flag,
    UnitTickableEntityHost& host) {
    host.construct_owner_entity_0077eed0(ent, flag);
    host.construct_tick_registration_00875890(ent, 0);
    ent.vptr_00 = 0x00D0DF70u;
    ent.vptr_10 = 0x00D0DF54u;
    ent.vptr_24 = 0x00D0DF4Cu;
    ent.vptr_170 = 0x00D0DF48u;
    ent.vptr_1e4 = 0x00D0DF40u;
    ent.vptr_310 = 0x00D0DF28u;
    ent.part_descriptors.clear();
    ent.field_35c = 0xFFFFFFFFu;
    ent.field_364 = 99.9f;
    ent.field_374 = 0x3Fu;
    ent.scene_nodes.clear();
    ent.field_c4 = 4u;
    return &ent;
}

// 0087A410 BSP_UnitTickableEntity_Destruct
// __thiscall(ECX = ent), RET.
// Reinstalls the six vtables, frees +380h..+388h (0087A474) and +348h..+350h (0087A499),
// calls 00875490(ent+310h) (0087A4B5), and calls 0077E380(ent) (0087A4C4).
void unit_tickable_entity_destruct_0087a410(
    UnitTickableEntityState& ent,
    UnitTickableEntityHost& host) {
    ent.vptr_00 = 0x00D0DF70u;
    ent.vptr_10 = 0x00D0DF54u;
    ent.vptr_24 = 0x00D0DF4Cu;
    ent.vptr_170 = 0x00D0DF48u;
    ent.vptr_1e4 = 0x00D0DF40u;
    ent.vptr_310 = 0x00D0DF28u;
    ent.scene_nodes.clear();
    ent.scene_nodes.shrink_to_fit();
    ent.part_descriptors.clear();
    ent.part_descriptors.shrink_to_fit();
    host.destruct_tick_element_00875490(ent);
    host.destruct_owner_entity_0077e380(ent);
}

// 0087BF80 BSP_UnitTickableEntity_SEntityInitSlotA4
// __thiscall(ECX = ent), RET.
// Calls 0077FAD0 (0087BF9E); when `[ent+C0h]` has mode 3, opens saved Lua reader
// (`009238A0` / `004425C0`), enters `"_fragile"` (`00D0DEF8`), reads `"damagedGFXLevel"`
// (`00D0DEE8` -> `00876EC0` when non-zero), enters `"damage"` (`00CFD7C8`) and reads
// each part's `"fire"` (`00CE6798`) into a local float, then tidies reader (`00441210`).
// Next, if `damageable_class->frame_tree.size != 0` and neither `ent` nor `ent->parent_3c`
// reports `vslot_5C(8)`, iterates `frame_tree` via `008772B0`, clones each section node
// with `(0, 1.0f)`, pushes to `ent.scene_nodes` (`0087B840`), attaches to `ent->vslot_18()`
// (`00B6E680`), and applies `node->local_matrix` (`+10h`) via `vslot_38`.
void unit_tickable_entity_sentity_init_slot_a4_0087bf80(
    UnitTickableEntityState& ent,
    UnitTickableEntityHost& host,
    DamageableTreeHost& tree_host) {
    host.read_replication_state_0077fad0(ent);
    if (ent.has_saved_lua_holder && ent.saved_lua_mode == 3) {
        const int gfx_level = host.read_saved_damaged_gfx_level(ent);
        if (gfx_level != 0) {
            host.set_model_group_state_00876ec0(ent, gfx_level);
        }
        for (std::size_t i = 0; i < ent.part_descriptors.size(); ++i) {
            if (host.has_saved_damage_entry(ent, i)) {
                (void)host.read_saved_damage_fire(ent, i);
            }
        }
    }
    if (ent.damageable_class != nullptr && ent.damageable_class->frame_tree.size != 0u) {
        if (!ent.is_kind_8) {
            if (ent.parent_3c != nullptr && ent.parent_3c->is_kind_8) {
                return;
            }
            void* parent_render_node = ent.primary_render_node;
            DamageableTree& tree = ent.damageable_class->frame_tree;
            if (tree.head != nullptr) {
                DamageableTreeIterator it{&tree, tree.head->left};
                while (it.node != nullptr && it.node != tree.head) {
                    UnitSceneNode* cloned = host.clone_section_scene_node(
                        it.node->section_model_owner, 0, 1.0f);
                    ent.scene_nodes.push_back(cloned);
                    if (cloned != nullptr) {
                        host.set_node_parent_00b6e680(cloned, parent_render_node);
                        cloned->set_local_matrix_vslot38(it.node->local_matrix);
                    }
                    tree_host.advance_iterator_008772b0(it);
                }
            }
        }
    }
}

// ============================================================================
// 2. Landscape Terrain, HeightField Forwarders & Collision Subobject
//    (0087F9A0..00883BB0)
// ============================================================================

// 0087F9A0 BSP_Landscape_OnHit
// __thiscall(ECX = landscape, param_2, param_3, param_4), RET 0Ch.
// `[+3D0h]->vslot_40()` takes no arguments: the three pushes at 0087F9A9..0087F9B9 stay on the
// stack for `007407C0(ECX = [+168h], root, param_3, param_4, param_2)` (0087F9C6). No null test
// on +3D0h (cc13-fixes2; the earlier version passed the three words to vslot_40 and guarded it).
void landscape_on_hit_0087f9a0(
    LandscapeState& self,
    std::uint32_t param_2,
    std::uint32_t param_3,
    std::uint32_t param_4,
    LandscapeHost& host) {
    void* target = self.height_field_3d0->get_terrain_render_root_vslot40();
    host.add_decal_007407c0(self.decal_owner_id, target, param_3, param_4, param_2);
}

// 0087F9F0 BSP_Landscape_SetMatrixNoOp
// Bare RET (vslot +120h of 00CEA050).
void landscape_set_matrix_no_op_0087f9f0() noexcept {}

// 0087FA20 BSP_Landscape_ForwardLocalHeight
// __thiscall(ECX = landscape, a, b), RET 8.
// Forwards to `[+3D0h]->vslot_24(a, b)` (`00ADA160`).
float landscape_forward_local_height_0087fa20(
    const LandscapeState& self,
    float a,
    float b) {
    return self.height_field_3d0->local_height_vslot24(a, b);
}

// 0087FA50 BSP_Landscape_ForwardHeightAt
// __thiscall(ECX = landscape, x, z), RET 8.
// Forwards to `[+3D0h]->vslot_28(x, z)`.
float landscape_forward_height_at_0087fa50(
    const LandscapeState& self,
    float x,
    float z) {
    return self.height_field_3d0->height_at_vslot28(x, z);
}

// 0087FB90 BSP_Landscape_ForwardLocalNormal
// __thiscall(ECX = landscape, out, x, z), RET 0Ch.
// Forwards to `[+3D0h]->vslot_34(out, x, z)` (`00ADA1C0`) and returns `out`.
Vec3* landscape_forward_local_normal_0087fb90(
    const LandscapeState& self,
    Vec3* out,
    float x,
    float z) {
    self.height_field_3d0->local_normal_vslot34(out, x, z);
    return out;
}

// 0087FC30 BSP_Landscape_ForwardLocalGridToWorldXZ
// __thiscall(ECX = landscape, out, a, b), RET 0Ch.
// Forwards to `[+3D0h]->vslot_1c(out, a, b)` (`00ADA0D0`) and returns `out`.
Vec2* landscape_forward_local_grid_to_world_xz_0087fc30(
    const LandscapeState& self,
    Vec2* out,
    float a,
    float b) {
    self.height_field_3d0->local_grid_to_world_xz_vslot1c(out, a, b);
    return out;
}

// 0087FC60 BSP_Landscape_HeightAtWorldOrDefault
// __thiscall(ECX = landscape, const float* xz), RET 4.
// Calls `[+3D0h]->vslot_2c(xz[0], xz[1])` when `[+3D0h] != nullptr`, otherwise `-1.0f` (`00D7A260`).
float landscape_height_at_world_or_default_0087fc60(
    const LandscapeState& self,
    const float* xz) {
    if (self.height_field_3d0 != nullptr) {
        return self.height_field_3d0->height_at_world_vslot2c(xz[0], xz[1]);
    }
    return -1.0f;
}

// 0087FE80 BSP_Landscape_VSlot198_NoOp
// Bare RET (vslot +198h of 00CEA050).
void landscape_vslot198_no_op_0087fe80() noexcept {}

// 0087FF80 BSP_CollisionShape_TraceSegment_Subobject
// __thiscall(ECX = landscape + 344h, ray, filter, hit_record), RET 0Ch.
// Tests `[shape + 8Ch]` (`height_field_3d0`) `vslot_3c(ray, filter, &local_hit)`;
// on hit copies `local_hit` to `out_record.hit_pos`, calls `00470370(&out_record, shape - 344h)`
// (0087FFD9), sets `material_kind = 10`, `part_index = -1`, and returns true.
bool collision_shape_trace_segment_subobject_0087ff80(
    LandscapeState& parent_landscape,
    const void* ray,
    const void* filter,
    HitRecordState& out_record,
    LandscapeHost& host) {
    if (parent_landscape.height_field_3d0 != nullptr) {
        Vec3 local_hit{};
        if (parent_landscape.height_field_3d0->trace_segment_vslot3c(ray, filter, local_hit)) {
            out_record.hit_pos = local_hit;
            host.hit_record_set_entity_00470370(out_record, &parent_landscape);
            out_record.material_kind = 10u;
            out_record.part_index = -1;
            return true;
        }
    }
    return false;
}

// 00880000 BSP_Landscape_TickTerrain
// __thiscall(ECX = landscape, float dt), RET 4.
// Advances `terrain_time_3dc += dt`, calls 00BB0A90 on `coast_mesh_3d4` (0088002B),
// 00BAEC50 on `coastline_3d8` (00880048), 00ADA0C0 on `height_field_3d0` (0088005F),
// and `terrain_render_node_3cc->vslot_38(this + 74h)`.
void landscape_tick_terrain_00880000(
    LandscapeState& self,
    float dt,
    std::uint32_t game_terrain_tick_arg,
    LandscapeHost& host) {
    self.terrain_time_3dc += dt;
    if (self.coast_mesh_3d4 != nullptr) {
        host.coast_mesh_set_time_00bb0a90(self.coast_mesh_3d4, self.terrain_time_3dc);
    }
    if (self.coastline_3d8 != nullptr) {
        host.coastline_set_time_00baec50(self.coastline_3d8, self.terrain_time_3dc);
    }
    host.tick_terrain_noop_00ada0c0(self.height_field_3d0, game_terrain_tick_arg);
    if (self.terrain_render_node_3cc != nullptr) {
        host.update_render_node_pose_vslot38(self.terrain_render_node_3cc, self.entity_matrix_74);
    }
}

// 00880AF0 BSP_RefCountedPtr_Release_00880af0
// __fastcall(ECX = RefCountedObject**), RET.
RefCountedObject** ref_counted_ptr_release_00880af0(RefCountedObject** slot) {
    if (slot != nullptr && *slot != nullptr) {
        RefCountedObject* obj = *slot;
        if (atomic_decrement_ref(obj->ref_count) == 0) {
            obj->destroy_vslot0();
        }
        *slot = nullptr;
    }
    return slot;
}

// 00880B20 BSP_RefCountedPtr_Release_00880b20
// __fastcall(ECX = RefCountedObject**), RET. Same body as 00880AF0.
RefCountedObject** ref_counted_ptr_release_00880b20(RefCountedObject** slot) {
    return ref_counted_ptr_release_00880af0(slot);
}

// 00880B50 BSP_RefCountedPtr_Release_00880b50
// __fastcall(ECX = RefCountedObject**), RET. Same body as 00880AF0.
RefCountedObject** ref_counted_ptr_release_00880b50(RefCountedObject** slot) {
    return ref_counted_ptr_release_00880af0(slot);
}

// 00880B80 BSP_RefCountedPtr_Release_00880b80
// __fastcall(ECX = RefCountedObject**), RET. Same body as 00880AF0.
RefCountedObject** ref_counted_ptr_release_00880b80(RefCountedObject** slot) {
    return ref_counted_ptr_release_00880af0(slot);
}

// 00880BB0 BSP_Landscape_ReleaseVertexStreams
// __fastcall(ECX = landscape), RET.
// Walks convex hulls of `generated_model_420` (`+50h..+54h`), refreshes world pose via
// 00414DB0 (00880C21) when `world_pose_valid` (`+C8h`) is false, copies `world_matrix_cc`
// (`+CCh`), translates `m[12..14]` by `hull->local_offset` (`+14h..+1Ch`), and calls
// 006FB030 `BSP_ConvexHull_DebugDrawEdges` (00880C5F).
void landscape_release_vertex_streams_00880bb0(
    LandscapeState& self,
    LandscapeHost& host) {
    if (self.generated_model_420 == nullptr) {
        return;
    }
    for (LandscapeConvexHull* hull : self.generated_model_420->convex_hulls) {
        if (!self.world_pose_valid) {
            host.refresh_world_pose_00414db0(self);
        }
        Mat4x4 local_mat = self.world_matrix_cc;
        local_mat.m[12] += hull->local_offset.x;
        local_mat.m[13] += hull->local_offset.y;
        local_mat.m[14] += hull->local_offset.z;
        host.debug_draw_hull_edges_006fb030(hull, local_mat);
    }
}

// 00880CD0 BSP_Landscape_Teardown
// __fastcall(ECX = landscape), RET.
// Detaches `this + 1E4h` from SpatialIndex (0042E630 / 0098A500 at 00880CE2 / 00880CE9),
// resets AvoidZoneRegistry layers (004C17D0 / 00423AF0 at 00880CEE / 00880CF5), zeroes
// `spatial_shape_count_2dc` (+2DCh), and tail-jumps 009277F0 (00880D07).
void landscape_teardown_00880cd0(
    LandscapeState* self,
    LandscapeHost& host) {
    void* spatial_node = (self != nullptr)
        ? reinterpret_cast<std::uint8_t*>(self) + 0x1E4
        : nullptr;
    host.spatial_index_detach_0098a500(spatial_node);
    host.avoid_zone_reset_layers_00423af0();
    if (self != nullptr) {
        self->spatial_shape_count_2dc = 0u;
        host.base_teardown_009277f0(*self);
    }
}

// 00881600 BSP_Landscape_BindTerrainMaterialParams
// __fastcall(ECX = model_node), RET.
// Checks `node->vslot_0c(BSP_GeneratedModel_GetCurrentType())`; when true iterates
// geometries (00B74650 / 00B74640) and mesh elements (00B72B40 / 00B732C0), registering
// the 8 terrain material shader parameters, then recurses on child nodes (`+34h`, `+3Ch`).
void landscape_bind_terrain_material_params_00881600(
    GeneratedModelNode* node) {
    if (node == nullptr) {
        return;
    }
    static constexpr const char* kParamNames[8] = {
        "cStaticShot_Size_OffsetX_OffsetY",
        "cBasic___ShoreH_DirtH_SimpleDist_SimpleFog",
        "cSnoise___Div_Max_UV",
        "cHnoise___Div_Max",
        "cGround___slope_UV_maskUV",
        "cUW___slope_UV",
        "cUWColor0",
        "cUWColor1",
    };
    if (node->matches_current_type) {
        for (auto& geom : node->geometries) {
            for (auto& elem : geom) {
                for (const char* name : kParamNames) {
                    elem.registered_params.emplace_back(name);
                }
            }
        }
    }
    for (GeneratedModelNode* child : node->children) {
        landscape_bind_terrain_material_params_00881600(child);
    }
}

// 00882420 BSP_Landscape_ApplyOceanWaveDamp
// __fastcall(ECX = model_node), RET.
// Checks `node->vslot_0c(BSP_GeneratedModel_GetCurrentType())`; for each mesh element
// reads effect name (00B172D0), strips any directory prefix up to the last `/` (004BCB80 /
// 00469840), and when `strstr(basename, "oceanwavedamp") != nullptr` calls
// `00B6DA70` (`BSP_SceneNode_SetVisibilityFactor`) on the node, once per matching element.
// Call bytes 0088258D..00882599: FLDZ; PUSH 0; PUSH ECX; FSTP [ESP]; MOV ECX,[ESP+18h]
// (= EDI = node, saved at 00882446); CALL 00B6DA70, i.e. thiscall(node, 0.0f, false).
// Recurses on children (+34h list, +3Ch sibling, 008825F1..00882602). The binary has no
// null check on `node` (00882444 MOV ESI,[EDI]); `host` is not an original argument.
// The type test (cc13-fix8): the vtable slot address is taken first (00882444..0088244A),
// then 00B74310 is called and its result pushed for [vtable+0Ch](type) with ECX = node.
void landscape_apply_ocean_wave_damp_00882420(
    GeneratedModelNode* node,
    LandscapeHost& host) {
    const std::int32_t current_type = host.generated_model_current_type_00b74310();
    if (host.node_is_type_vslot0c(node, current_type)) {             // 0088245B JE
        for (auto& geom : node->geometries) {
            for (auto& elem : geom) {
                std::string name = elem.effect_name;
                const std::size_t slash_pos = name.find_last_of('/');
                if (slash_pos != std::string::npos) {
                    name = name.substr(slash_pos + 1u);
                }
                if (name.find("oceanwavedamp") != std::string::npos) {
                    host.scene_node_set_visibility_factor_00b6da70(node, 0.0f, false);
                }
            }
        }
    }
    for (GeneratedModelNode* child : node->children) {
        landscape_apply_ocean_wave_damp_00882420(child, host);
    }
}

// 008828E0 BSP_Landscape_Destruct
// __fastcall(ECX = landscape), RET.
// Reinstalls the six Landscape vtables (`00CEA090`, `00CEA078`, `00CEA070`, `00CEA06C`,
// `00CEA05C`, `00CEA050`), calls 008826F0 `BSP_Landscape_ReleaseTerrain` (0088292E),
// releases `model_path_424`, `file_path_3c4`, `ref_3e4`, `vec_378`, `vec_368`, `buf_2e0`,
// and calls 009287B0 `BSP_GameEntity_Destruct` (00882AA3).
void landscape_destruct_008828e0(
    LandscapeState& self,
    LandscapeHost& host) {
    self.vptr_00 = 0x00CEA090u;
    self.vptr_10 = 0x00CEA078u;
    self.vptr_24 = 0x00CEA070u;
    self.vptr_170 = 0x00CEA06Cu;
    self.vptr_1e4 = 0x00CEA05Cu;
    self.vptr_344 = 0x00CEA050u;
    host.release_terrain_008826f0(self);
    self.model_path_424.clear();
    if (self.ref_3e4 != nullptr) {
        if (atomic_decrement_ref(self.ref_3e4->ref_count) == 0) {
            self.ref_3e4->destroy_vslot0();
        }
        self.ref_3e4 = nullptr;
    }
    self.file_path_3c4.clear();
    self.vec_378.clear();
    self.vec_368.clear();
    self.vptr_1e4 = 0x00CE89E8u;
    self.buf_2e0 = nullptr;
    host.destruct_game_entity_009287b0(self);
}

// 00882AC0 BSP_Landscape_LoadTerrain
// __thiscall(ECX = landscape, const NativeString* file_path), RET 4.
// Releases previous terrain (008826F0), extracts basename `F` after the last `\` or `/`
// (unused), builds `terrain/<file_path>_heightmap.tdt`, `"NULL"`, `terrain/<file_path>_colormap.dds`,
// constructs heightfield at `+3D0h` (00ADD290), builds `models/terrain/<file_path>.mmod` and
// branches on its VFS existence (00882EFB).
// Corrected 2026-10-16 (cc13-fix7, audit fixes2 XF5): +41Ch is now written (007188A0) and
// +420h comes from [+41Ch]->vslot8(0, 1.0f); the attach is 00ADE820 with file_path and
// &self+74h; 007135C0 gets [+420h]; hull actors (00C5D580) are pushed into +364h; the
// tdt load is 00ADDA60 with its last two floats; foliage is 00ADAC90(file_path); 00881600
// gets [+3D0h]->vslot40(); the world extents are read here, not passed in (00882AC0 has
// one stack argument, RET 4). Simplifications: the stack strings are std::string; the
// NxActorDesc for a hull is built by the host from the values listed at
// create_hull_actor_00c5d580. The grid-cell loop (00883507..008839E2, cc13-fix8) reads the
// tile from [hf+40h], calls 00ADEA70 on the tile, fills the inline shape descriptor
// (TerrainTileShapeDesc) and creates the actor through 00C5D580 (the host builds the
// NxActorDesc default stores and runs 00C336C0, as for the hulls); no 00C31AA0 call exists.
void landscape_load_terrain_00882ac0(
    LandscapeState& self,
    const std::string& file_path,
    LandscapeHost& host) {
    host.release_terrain_008826f0(self);
    self.has_model_3e1 = false;

    const std::size_t last_backslash = file_path.find_last_of('\\');
    const std::size_t last_slash = file_path.find_last_of('/');
    int cut_index = -1;
    if (last_backslash != std::string::npos) {
        cut_index = static_cast<int>(last_backslash);
    }
    if (last_slash != std::string::npos && static_cast<int>(last_slash) > cut_index) {
        cut_index = static_cast<int>(last_slash);
    }
    // 00882BEE: 00469840 copies the text after the separator into a temporary ([ESP+140h]),
    // but every path below is built from the full argument: the 0043C130 prefix calls push
    // EDI = [ESP+208h] (the argument, 00882BD4) at 00882C31 and 00882D21, and
    // [ESP+208h] again at 00882E5D. The temporary is only released (checked cc13-fixes2).
    const std::string base_name = file_path.substr(static_cast<std::size_t>(cut_index + 1));
    static_cast<void>(base_name);
    const std::string tdt_path = "terrain/" + file_path + "_heightmap.tdt";
    const std::string null_tag = "NULL";
    const std::string dds_path = "terrain/" + file_path + "_colormap.dds";
    self.height_field_3d0 =
        host.create_height_field_00add290(file_path, host.game_terrain_context_19ec());
    const std::string mmod_path = "models/terrain/" + file_path + ".mmod";

    if (!host.vfs_file_exists(mmod_path)) {
        // 00883A1B..00883A7D: x87 FSUB then FSTP float of each extent; 255.0f is the float
        // at 00CEDD44 (bytes 00 00 7F 43).
        const LandscapeWorldBox box = host.world_box_e188a8();
        host.load_tdt_00adda60(
            self.height_field_3d0,
            tdt_path,
            null_tag,
            dds_path,
            box.f_7124 - box.f_7130,
            box.f_7128 - box.f_711c,
            0.0f,
            255.0f);
    } else {
        // 00882F03..00882F49 builds the same path again into a second string, and
        // 00882FA2..00882FB7 copies its text into a third (null data reads 00F878E8) for
        // 007188A0. All three hold `models/terrain/<file_path>.mmod`.
        const std::string mmod_again = "models/terrain/" + file_path + ".mmod";
        self.model_resource_41c = host.load_model_resource_007188a0(mmod_again);
        // 00882FFA..0088300E: FLD1 / FSTP [ESP] / PUSH 0 / CALL [vtable+8], no null test.
        self.generated_model_420 =
            host.model_resource_instantiate_vslot8(self.model_resource_41c, 0, 1.0f);
        const LandscapeWorldBox box = host.world_box_e188a8();
        host.attach_model_00ade820(
            self.height_field_3d0,
            file_path,
            self.generated_model_420,
            box.f_7124 - box.f_7130,
            box.f_7128 - box.f_711c,
            &self.entity_matrix_74);
        // 00883062..008830BE (cc13-fixes2): only with an ocean scene at [[00E188A8]+19F0h]:
        // the render node root->vslot10(6, 0) into +3CCh, 00B6D890(node, 0), the scene's wave
        // field ([scene+A8h]) 00BA1370(node), then 00882420 on the render root (vslot40()
        // fetched again), not on the model +420h. No null test on the node.
        if (host.game_ocean_scene_19f0() != nullptr) {
            GeneratedModelNode* root = self.height_field_3d0->get_terrain_render_root_vslot40();
            self.terrain_render_node_3cc = host.render_root_create_node_vslot10(root, 6, 0);
            host.node_propagate_root_registration_00b6d890(self.terrain_render_node_3cc, 0);
            host.ocean_wave_field_add_terrain_coverage_00ba1370(host.game_ocean_scene_19f0(),
                self.terrain_render_node_3cc);
            landscape_apply_ocean_wave_damp_00882420(
                self.height_field_3d0->get_terrain_render_root_vslot40(), host);
        }
        self.has_model_3e1 = true;                                    // 008830C8
        self.unit_part_instance_418 =
            host.create_unit_part_instance_007135c0(self, self.generated_model_420);
        // 008830FA..00883502: walks [+420h]+50h..+54h with no null test on the model or on
        // an element; only hulls with [hull+8] == 0 get an actor.
        for (LandscapeConvexHull* hull : self.generated_model_420->convex_hulls) {
            if (hull->kind != 0) {
                continue;                                             // 00883252 JNZ
            }
            if (!self.world_pose_valid) {
                host.refresh_world_pose_00414db0(self);               // 00883378
            }
            // 00883388 00C336C0(&self+CCh), then 0088338D..008833F1 adds hull+14h..+1Ch to
            // the descriptor's translation.
            Mat4x4 pose = self.world_matrix_cc;
            pose.m[12] += hull->local_offset.x;
            pose.m[13] += hull->local_offset.y;
            pose.m[14] += hull->local_offset.z;
            void* actor = host.create_hull_actor_00c5d580(self, hull, pose);
            host.pointer_vector_push_back_004216b0(self.vec_368, actor);  // 008834B4
        }
        // 00883507..008839E2 (cc13-fix8): EBP = [+3D0h] for the whole loop. The outer
        // counter [ESP+2Ch] runs to 00ADAC30 ([hf+38h]) with [ESP+30h] += 12Ch; the inner
        // [ESP+18h] runs to 00ADAC40 ([hf+3Ch]) with [ESP+3Ch] reset to 0 per column and
        // += 12Ch. The tile is [hf+40h][[hf+38h] * gz + gx]; a null tile skips the cell.
        TerrainHeightFieldInterface* const hf = self.height_field_3d0;
        const int cells_x = host.get_grid_cells_x_00adac30(hf);
        const int cells_z = host.get_grid_cells_z_00adac40(hf);
        // [ESP+9Ch] / [ESP+0A0h] are stored for kind 2 only, so other kinds pass whatever
        // the slots last held (an earlier kind-2 tile, else stack left by the hull loop,
        // whose actor descriptor also starts at [ESP+48h]); the start value is unknown.
        TerrainTileShapeDesc shape{};
        for (int gx = 0, step_x = 0; gx < cells_x; ++gx, step_x += 300) {
            for (int gz = 0, step_z = 0; gz < cells_z; ++gz, step_z += 300) {
                const int index = host.height_field_tiles_across_38(hf) * gz + gx;
                void* const tile = host.height_field_tile_40(hf, index);
                if (tile == nullptr) {
                    continue;                                         // 00883564 JE
                }
                const TerrainTileBlock* block = host.tile_block_00adea70(tile);
                shape.f_00 = 0.0f;
                shape.f_04 = 0.0f;
                shape.i_08 = 8;
                shape.i_0c = 0;
                shape.type_10 = 5;
                const float identity[12] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f};
                for (int i = 0; i < 12; ++i) {
                    shape.local_pose_14[i] = identity[i];
                }
                shape.rows_44 = 0x21;
                shape.columns_48 = 0x21;
                shape.row_spacing_4c = 9.375f;                        // 00D0E658
                shape.column_spacing_50 = 9.375f;
                shape.kind2_60 = 0;
                void* data = nullptr;                                 // 00883622 XOR ECX
                if (block->kind_0c == 0) {
                    data = host.tile_block_00adea70(tile)->data_34;   // 00883660..00883667
                } else if (block->kind_0c == 2) {
                    const TerrainTileBlock* again = host.tile_block_00adea70(tile); // 0088362F
                    data = again->data_40;
                    shape.kind2_60 = 1;
                    shape.block_2c_54 = again->f_2c;
                    shape.block_30_58 = again->f_30;
                }
                shape.data_5c = data;                                 // 0088372B

                // 0088384F..00883905: x = float([hf+80h] + float(step_x)), y = 0.0f + 0.0f,
                // z = float(float(step_z) + [hf+84h]), added to a copy of +74h (not +CCh).
                const float x = host.height_field_origin_x_80(hf) + static_cast<float>(step_x);
                const float z = static_cast<float>(step_z) + host.height_field_origin_z_84(hf);
                const float y = 0.0f + 0.0f;                          // FLDZ; FADD ST0,ST0
                Mat4x4 pose = self.entity_matrix_74;                  // REP MOVSD 10h dwords
                pose.m[12] += x;
                pose.m[13] += y;
                pose.m[14] += z;
                void* actor = host.create_grid_tile_actor_00c5d580(shape, pose);
                // 0088392A..00883984: inline push_back into the +374h vector (00420830
                // when full).
                self.vec_378.push_back(actor);
            }
        }
    }

    // 00883A82..00883AA8, both branches: 00ADAC90(ECX = hf, &file_path), then 00881600 on
    // the render root [+3D0h]->vslot40(), not on +420h.
    host.load_foliage_default_00adac90(self.height_field_3d0, file_path);
    landscape_bind_terrain_material_params_00881600(
        self.height_field_3d0->get_terrain_render_root_vslot40());
}

// 00883BB0 BSP_Landscape_InitAttachTerrain
// __fastcall(ECX = landscape), RET.
// Zeroes terrain pointers (+3CCh, +3D4h, +3D8h, +3DCh, +418h, +41Ch, +420h, +58h),
// calls 00928A00 and 0087FE90, clears `file_path_3c4`, reads `FilePath` and optional
// `ModelPath` (`models/terrain/<ModelPath>.mmod` -> `model_path_424`), calls 00882AC0
// (00883DD3), creates optional `ShallowWater` decal (+42Ch), registers collision
// subobject `+344h` into `+1E4h`, computes local bounds (`max.y += 42.0` from 00CE6840),
// attaches to SpatialIndex (0098BA10), and registers decal owner (00740FE0).
void landscape_init_attach_terrain_00883bb0(
    LandscapeState& self,
    LandscapeHost& host) {
    self.terrain_render_node_3cc = nullptr;
    self.coast_mesh_3d4 = nullptr;
    self.coastline_3d8 = nullptr;
    self.terrain_time_3dc = 0.0f;
    self.unit_part_instance_418 = nullptr;
    self.model_resource_41c = nullptr;
    self.generated_model_420 = nullptr;
    self.field_58 = 0u;

    host.create_lua_self_table_00928a00(self);
    host.reset_terrain_state_0087fe90(self);
    self.file_path_3c4.clear();

    if (self.scene_load_mode == 1) {
        self.file_path_3c4 = host.get_scene_property_string("FilePath");
        if (host.has_scene_property("ModelPath")) {
            const std::string model_prop = host.get_scene_property_string("ModelPath");
            self.model_path_424 = "models/terrain/" + model_prop + ".mmod";
        }
    }

    landscape_load_terrain_00882ac0(self, self.file_path_3c4, host);

    if (self.scene_load_mode == 1) {
        if (self.shallow_water_handle_42c != 0u) {
            host.release_shallow_water_decal(self.shallow_water_handle_42c);
            self.shallow_water_handle_42c = 0u;
        }
        if (host.has_shallow_water_property()) {
            const std::string tex = host.get_shallow_water_texture();
            if (!tex.empty()) {
                if (!self.world_pose_valid) {
                    host.refresh_world_pose_00414db0(self);
                }
                const Vec3 sw_pos = host.get_shallow_water_pos();
                const Vec2 world_xz{
                    sw_pos.x + self.world_matrix_cc.m[12],
                    sw_pos.z + self.world_matrix_cc.m[14],
                };
                const float sw_size = host.get_shallow_water_size();
                self.shallow_water_handle_42c = host.create_shallow_water_decal(
                    tex, world_xz, Vec2{sw_size, sw_size});
            }
        }
    }

    self.spatial_shapes.push_back(reinterpret_cast<std::uint8_t*>(&self) + 0x344);
    ++self.spatial_shape_count_2dc;
    self.spatial_owner_230 = &self;

    Vec3 world_min{};
    Vec3 world_max{};
    if (self.height_field_3d0 != nullptr) {
        self.height_field_3d0->get_world_bounds_vslot0c(world_min, world_max);
    }
    world_max.y = static_cast<float>(static_cast<double>(world_max.y) + 42.0);
    if (!self.world_pose_valid) {
        host.refresh_world_pose_00414db0(self);
    }
    self.spatial_matrix_384 = self.world_matrix_cc;
    const Vec3 local_max{
        world_max.x - self.world_matrix_cc.m[12],
        world_max.y - self.world_matrix_cc.m[13],
        world_max.z - self.world_matrix_cc.m[14],
    };
    const Vec3 local_min{
        world_min.x - self.world_matrix_cc.m[12],
        world_min.y - self.world_matrix_cc.m[13],
        world_min.z - self.world_matrix_cc.m[14],
    };
    host.set_spatial_local_bounds_0098a920(local_min, local_max);
    host.spatial_index_attach_0098ba10(self.spatial_matrix_384);
    self.decal_owner_id = host.register_decal_owner_00740fe0();
}

// ============================================================================
// 3. MissionLuaHost, MissionLuaResultSink, MissionLuaDeferredCall,
//    MissionLuaCallQueue & LuaObject / MissionEntity Helpers (008844F0..0088B3D0)
// ============================================================================

// 008844F0 BSP_MissionLuaHost_ReleaseMachine
// __fastcall(ECX = host), RET.
// If `[host + 04h]` is non-null, calls its vslot 0(1) (`006B87C0`) and nulls `[host + 04h]`.
void mission_lua_host_release_machine_008844f0(
    MissionLuaHostState& host,
    MissionLuaEnvironmentHost& env) {
    if (host.lua_machine != nullptr) {
        env.delete_lua_machine(host.lua_machine);
        host.lua_machine = nullptr;
    }
}

// 00885110 BSP_MissionLuaHost_RunScriptFile
// __thiscall(ECX = host, const char* path), RET 4.
// Opens `path` via VFS mode 2; if stream is valid (`vslot_18()`), allocates buffer of
// `vslot_30()` bytes, reads via `vslot_24()`, and runs chunk via `006B89F0` (008852E7)
// with `results = nullptr` and `mode = 2`.
void mission_lua_host_run_script_file_00885110(
    MissionLuaHostState& host,
    const char* path,
    MissionLuaEnvironmentHost& env) {
    std::vector<char> bytes{};
    const std::string safe_path = (path != nullptr) ? path : "";
    if (env.read_vfs_file(safe_path, bytes)) {
        std::string ignored_error{};
        env.run_lua_chunk_006b89f0(
            host.lua_machine,
            bytes.data(),
            bytes.size(),
            safe_path.c_str(),
            nullptr,
            ignored_error,
            2);
    }
}

// 00885350 BSP_VfsFileBlockScope_Destruct
// __fastcall(ECX = scope), RET.
// Sets vptr 00D0E6EC; when `scope.saved_depth < vfs_depth`, calls 00BDC9B0(`""`) (008853A0).
void vfs_file_block_scope_destruct_00885350(
    VfsFileBlockScope& scope,
    MissionLuaEnvironmentHost& env) {
    scope.vptr = 0x00D0E6ECu;
    if (scope.saved_depth < env.current_vfs_block_depth()) {
        env.leave_vfs_file_block_00bdc9b0("");
    }
}

// 00885A30 BSP_MissionLuaResultSink_AppendAllAsText
// __thiscall(ECX = sink, NativeString* out), RET 4.
// Walks `sink.items` (`[this+08h]` for `[this+0Ch]` 20-byte records) and calls
// 008855B0 (00885A53) on each.
void mission_lua_result_sink_append_all_as_text_00885a30(
    const MissionLuaResultSink& sink,
    std::string& out_text,
    MissionLuaEnvironmentHost& env) {
    for (const LuaVariantRecord& rec : sink.items) {
        env.append_variant_as_text_008855b0(rec, out_text);
    }
}

// 00885DA0 BSP_MissionLua_PushArgumentRecord
// __thiscall(ECX = record, LuaMachine* machine), RET 4.
// Switches on `record.tag` (`[record + 04h]`, 0..6):
//   0 -> 006B80E0 PushNumber
//   1 -> 006B8120 PushString (substituting 00F87904 `""` when null)
//   2 -> 006B80C0 PushBoolean
//   3 -> if short != 0: 00884240("%d") + GetGlobal("thisTable") + GetTable + Remove; else PushNil
//   4 -> no-op
//   5 -> 006B84B0 CreateTable + 00885A70 on each pair
//   6 -> 006B8130 PushNil
void mission_lua_push_argument_record_00885da0(
    const LuaVariantRecord& record,
    MissionLuaEnvironmentHost& env) {
    switch (record.tag) {
    case 0:
        env.lua_push_number_006b80e0(record.num_val);
        return;
    case 1:
        env.lua_push_string_006b8120(record.str_val.c_str());
        return;
    case 2:
        env.lua_push_boolean_006b80c0(record.bool_val);
        return;
    case 3:
        if (record.entity_id != 0) {
            env.lua_push_entity_from_this_table(record.entity_id);
        } else {
            env.lua_push_nil_006b8130();
        }
        return;
    case 4:
        return;
    case 5:
        env.lua_create_table_006b84b0();
        for (const LuaVariantPair& pair : record.table_entries) {
            env.lua_push_table_pair_00885a70(pair);
        }
        return;
    case 6:
        env.lua_push_nil_006b8130();
        return;
    default:
        return;
    }
}

// 00885FB0 BSP_MissionLuaHost_RunScriptWithVariants
// __thiscall(ECX = host, const char* path, char run_variants), RET 8.
// Calls 00885110(path) (00885FD2), and when `run_variants != 0` queries 00BDEF90
// (00886012) and calls 00885110 (0088605E) on each override path.
void mission_lua_host_run_script_with_variants_00885fb0(
    MissionLuaHostState& host,
    const char* path,
    bool run_variants,
    MissionLuaEnvironmentHost& env) {
    mission_lua_host_run_script_file_00885110(host, path, env);
    if (run_variants) {
        std::vector<std::string> overrides{};
        env.append_lua_script_overrides_00bdef90(
            (path != nullptr) ? path : "", overrides);
        for (const std::string& ov : overrides) {
            mission_lua_host_run_script_file_00885110(host, ov.c_str(), env);
        }
    }
}

// 00886E00 BSP_MissionLua_BuildVariantFromStack
// __thiscall(ECX = sink, LuaVariantRecord* out, int stack_index, int depth, int max_depth), RET 10h.
// Guarded by `depth < max_depth`; switches on `006B7EC0(stack_index)` (`lua_type`):
//   0, 7 (nil / userdata) -> tag 6
//   1 (boolean)           -> 008869F0 (tag 0, num_val = 1.0f or 0.0f)
//   2 (lightuserdata)     -> 00886B30 (tag 4, userdata_val)
//   3 (number)            -> 008869D0 (tag 0, num_val)
//   4 (string)            -> 00886A10 (tag 1, str_val)
//   5 (table)             -> 00886B50 (tag 5, recurses with depth + 1, max_depth)
//   6 (function)          -> 00886A10 (tag 1, "<Function>")
void mission_lua_build_variant_from_stack_00886e00(
    MissionLuaResultSink& sink,
    LuaVariantRecord& out,
    int stack_index,
    int depth,
    int max_depth,
    MissionLuaEnvironmentHost& env) {
    (void)sink;
    if (depth >= max_depth) {
        return;
    }
    const int ltype = env.lua_stack_type_006b7ec0(stack_index);
    switch (ltype) {
    case 0:
    case 7:
        out.tag = 6;
        break;
    case 1:
        out.tag = 0;
        out.num_val = env.lua_to_boolean_006b8000(stack_index) ? 1.0f : 0.0f;
        break;
    case 2:
        out.tag = 4;
        out.userdata_val = env.lua_to_userdata_006b80a0(stack_index);
        break;
    case 3:
        out.tag = 0;
        out.num_val = env.lua_to_float_006b8020(stack_index);
        break;
    case 4: {
        const char* s = env.lua_to_string_006b8060(stack_index);
        out.tag = 1;
        out.str_val = (s != nullptr) ? s : "";
        break;
    }
    case 5:
        out.tag = 5;
        out.table_entries =
            env.lua_read_table_entries(stack_index, depth + 1, max_depth);
        break;
    case 6:
        out.tag = 1;
        out.str_val = "<Function>";
        break;
    default:
        env.lua_get_caller_info_006b86b0(1);
        break;
    }
}

// 00887220 BSP_MissionLua_CollectStackResults
// __thiscall(ECX = sink, int count, int max_depth), RET 8.
// Queries `top = 006B7E70()`, starts at `first = (count < 1) ? 1 : (top - count + 1)`,
// builds each variant via 00886E00 (008872BF), and pushes to `sink.items` (006EDF00 at 008872CB).
void mission_lua_collect_stack_results_00887220(
    MissionLuaResultSink& sink,
    int count,
    int max_depth,
    MissionLuaEnvironmentHost& env) {
    const int top = env.lua_get_top_006b7e70();
    const int first = (count < 1) ? 1 : (top - count + 1);
    for (int idx = first; idx <= top; ++idx) {
        LuaVariantRecord rec{};
        mission_lua_build_variant_from_stack_00886e00(
            sink, rec, idx, 0, max_depth, env);
        sink.items.push_back(rec);
    }
}

// 008874C0 BSP_MissionLuaPendingResults_Destruct
// __fastcall(ECX = pending), RET.
// Sets vptr 00D0E6F0; when `armed` (+0Ch) is non-zero, calls
// `00887220(sink, -1 - neg_count, 4)` (008874EA) and `006B85F0(-neg_count)` (008874F7).
void mission_lua_pending_results_destruct_008874c0(
    MissionLuaPendingResults& pending,
    MissionLuaEnvironmentHost& env) {
    pending.vptr = 0x00D0E6F0u;
    if (pending.armed && pending.sink != nullptr) {
        mission_lua_collect_stack_results_00887220(
            *pending.sink, -1 - pending.neg_count, 4, env);
        env.lua_pop_006b85f0(-pending.neg_count);
    }
}

// 00887560 BSP_MissionLuaDeferredCall_CopyArguments
// __thiscall(ECX = dst_vec, const src_vec*), RET 4.
// Calls 006B88E0(0) (0088756E), 006B87F0(src.size) (0088757A), and 006EDF00 (00887594)
// for each element, returning `dst_vec`.
std::vector<LuaVariantRecord>* mission_lua_deferred_call_copy_arguments_00887560(
    std::vector<LuaVariantRecord>& dst_vec,
    const std::vector<LuaVariantRecord>& src_vec) {
    dst_vec.clear();
    dst_vec.reserve(src_vec.size());
    for (const LuaVariantRecord& item : src_vec) {
        dst_vec.push_back(item);
    }
    return &dst_vec;
}

// 008875D0 BSP_MissionLuaResultSink_Construct
// __fastcall(ECX = sink), RET.
// Sets vptr 00D0E798 and zeroes +04h..+10h.
void mission_lua_result_sink_construct_008875d0(MissionLuaResultSink& sink) noexcept {
    sink.vptr = 0x00D0E798u;
    sink.lua_machine = nullptr;
    sink.items.clear();
}

// 008875F0 BSP_LuaScriptOverrideList_Construct
// __thiscall(ECX = sink, void* lua_machine), RET 4.
// Sets vptr 00D0E798, zeroes +04h..+10h, stores `lua_machine` at +04h, returns `&sink`.
MissionLuaResultSink* lua_script_override_list_construct_008875f0(
    MissionLuaResultSink& sink,
    void* lua_machine) noexcept {
    sink.vptr = 0x00D0E798u;
    sink.lua_machine = nullptr;
    sink.items.clear();
    sink.lua_machine = lua_machine;
    return &sink;
}

// 00887670 BSP_LuaVariantVector_Tidy_00887670
// __fastcall(ECX = sink), RET.
// Sets vptr 00D0E798, calls 006B88E0(0) (0088767E), and frees buffer (00887687).
void lua_variant_vector_tidy_00887670(MissionLuaResultSink& sink) noexcept {
    sink.vptr = 0x00D0E798u;
    sink.items.clear();
    sink.items.shrink_to_fit();
}

// 008876B0 BSP_MissionLuaDeferredCall_Destruct
// __fastcall(ECX = call), RET.
// Calls 006B88E0(0) and frees `args` buffer (+10h), then returns pool storage for
// `func_name` (+08h) and `self_key` (+00h).
void mission_lua_deferred_call_destruct_008876b0(MissionLuaDeferredCall& call) noexcept {
    call.args.clear();
    call.args.shrink_to_fit();
    call.func_name.clear();
    call.self_key.clear();
}

// 00887B30 BSP_MissionLuaHost_CallNamedForced
// __thiscall(ECX = host, self_key, name, args, stack_first, stack_last), RET 14h.
// Calls 00887750 (00887B4D) with `results = nullptr` and `force = 1`.
void mission_lua_host_call_named_forced_00887b30(
    MissionLuaHostState& host,
    const char* self_key,
    const char* func_name,
    const void* args,
    std::uint32_t stack_first,
    std::uint32_t stack_last,
    MissionLuaEnvironmentHost& env) {
    env.call_named_00887750(
        host, self_key, func_name, args, stack_first, stack_last, nullptr, true);
}

// 00887C30 BSP_MissionLuaCallQueue_InsertNode
// __thiscall(ECX = queue, next, prev, value), RET 0Ch.
// Allocates 2Ch-byte node via 00BF681B (00887C53), stores `next` (+00h) and `prev` (+04h),
// and copy-constructs `value` at `+08h` via 00887A10 (00887C91).
MissionLuaCallQueueNode* mission_lua_call_queue_insert_node_00887c30(
    MissionLuaCallQueueNode* next_node,
    MissionLuaCallQueueNode* prev_node,
    const MissionLuaDeferredCall& value) {
    auto* node = new MissionLuaCallQueueNode{};
    node->next = next_node;
    node->prev = prev_node;
    node->payload = value;
    return node;
}

// 00887D90 BSP_MissionLuaCallQueue_Clear
// __fastcall(ECX = queue), RET.
// Reseats sentinel `head` links to itself, sets `size = 0`, and walks the circular list
// calling 008876B0 (00887DB6) and `_free` (00887DBC) on each node.
void mission_lua_call_queue_clear_00887d90(MissionLuaCallQueue& queue) noexcept {
    if (queue.head == nullptr) {
        queue.size = 0u;
        return;
    }
    MissionLuaCallQueueNode* sentinel = queue.head;
    MissionLuaCallQueueNode* cur = sentinel->next;
    sentinel->next = sentinel;
    sentinel->prev = sentinel;
    queue.size = 0u;
    while (cur != nullptr && cur != sentinel) {
        MissionLuaCallQueueNode* next = cur->next;
        mission_lua_deferred_call_destruct_008876b0(cur->payload);
        delete cur;
        cur = next;
    }
}

// 00888300 BSP_MissionLuaHost_Destruct
// __fastcall(ECX = host), RET.
// Sets vptr 00D0E7A8, deletes `lua_machine` (+04h) via vslot 0(1) if non-null, calls
// 00887D90 (00888354) on `deferred_calls` (+08h), and frees sentinel `head` (+0Ch).
void mission_lua_host_destruct_00888300(
    MissionLuaHostState& host,
    MissionLuaEnvironmentHost& env) {
    host.vptr = 0x00D0E7A8u;
    if (host.lua_machine != nullptr) {
        env.delete_lua_machine(host.lua_machine);
        host.lua_machine = nullptr;
    }
    mission_lua_call_queue_clear_00887d90(host.deferred_calls);
    delete host.deferred_calls.head;
    host.deferred_calls.head = nullptr;
}

// 00888760 BSP_LuaObject_ReadVector3
// __fastcall(ECX = out float[3], EDX = LuaObject* table), RET.
// Iterates table keys via 00B67080 / 00B67190, matching `"x"` (00CEB488), `"y"` (00D045F8),
// and `"z"` (00CFD718), writes `out->x/y/z`, and returns `out`.
Vec3* lua_object_read_vector3_00888760(
    Vec3* out,
    const std::vector<LuaTableEntry>& table_entries) noexcept {
    float x = out->x;
    float y = out->y;
    float z = out->z;
    for (const LuaTableEntry& entry : table_entries) {
        if (entry.key == "x") {
            x = entry.num_value;
        } else if (entry.key == "y") {
            y = entry.num_value;
        } else if (entry.key == "z") {
            z = entry.num_value;
        }
    }
    out->x = x;
    out->y = y;
    out->z = z;
    return out;
}

// 008889C0 BSP_LuaValue_IsEntityHandle
// __fastcall(ECX = LuaObject* val), RET.
// Checks `IsTable` (00B661B0 at 008889DB), fetches `"Ptr"` (00CFAD08) via 00B67800
// (00888A03), checks `IsLightUserdata` (00B66160 at 00888A12), converts via `ToUserdata`
// (00B662D0 at 00888A4D), and tests `entity->vslot_5C(1) != 0` (00888A72).
bool lua_value_is_entity_handle_008889c0(
    bool is_table,
    const std::vector<LuaTableEntry>& table_entries) noexcept {
    if (!is_table) {
        return false;
    }
    for (const LuaTableEntry& entry : table_entries) {
        if (entry.key == "Ptr") {
            if (!entry.is_light_userdata || entry.userdata_ptr == nullptr) {
                return false;
            }
            const auto* ent =
                static_cast<const MissionEntityCandidate*>(entry.userdata_ptr);
            return ent->is_entity_kind_1;
        }
    }
    return false;
}

// 00888D20 BSP_LuaTable_GetPtrField
// __fastcall(ECX = LuaObject* table), RET.
// Fetches `"Ptr"` (00CFAD08) via 00B67800 (00888D43) and converts via 00B662D0 (00888D52).
void* lua_table_get_ptr_field_00888d20(
    const std::vector<LuaTableEntry>& table_entries) noexcept {
    for (const LuaTableEntry& entry : table_entries) {
        if (entry.key == "Ptr") {
            return entry.userdata_ptr;
        }
    }
    return nullptr;
}

// 008890F0 BSP_Game_IsScriptLoadActive
// __fastcall(ECX = game), RET.
// `XOR EAX,EAX; CMP [ECX+644h], EAX; SETG AL; RET`.
bool game_is_script_load_active_008890f0(int script_load_depth_644) noexcept {
    return script_load_depth_644 > 0;
}

// 0088B1B0 BSP_MissionEntity_FindByName
// __stdcall(int len, char* data), RET 8.
// Walks `kind_index` from -6 to 0x5A (stride 0Ch in `*(game+19CCh)+1Ch`), checking only
// the 14 active cases in the switch table at 0088B310:
// `{0, 9, 18, 19, 20, 21, 22, 59, 62, 63, 64, 65, 68, 71}`.
// Filters candidates by `flag_5c && !flag_5d && !flag_60 && !flag_5e`, and matches
// `candidate->vslot_10()` against `query_name` via `_stricmp` (00BF7FBF at 0088B2A8).
MissionEntityCandidate* mission_entity_find_by_name_0088b1b0(
    const MissionEntityBucketTable& table,
    const char* query_name) noexcept {
    for (int kind_index = -6; kind_index <= 0x5A; ++kind_index) {
        switch (kind_index) {
        case 0:
        case 9:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x3B:
        case 0x3E:
        case 0x3F:
        case 0x40:
        case 0x41:
        case 0x44:
        case 0x47: {
            const auto& bucket = table.buckets[kind_index + 6];
            for (MissionEntityCandidate* cand : bucket) {
                if (cand == nullptr) {
                    continue;
                }
                if (cand->flag_5c && !cand->flag_5d && !cand->flag_60 && !cand->flag_5e) {
                    const char* cname = cand->name;
                    if (query_name == nullptr) {
                        if (cname == nullptr || cname[0] == '\0') {
                            return cand;
                        }
                    } else if (cname != nullptr) {
                        if (ascii_stricmp(query_name, cname) == 0) {
                            return cand;
                        }
                    }
                }
            }
            break;
        }
        default:
            break;
        }
    }
    return nullptr;
}

// 0088B3D0 BSP_NativeString_ReleaseStorage_0088b3d0
// __fastcall(ECX = NativeString*), RET.
// If `data (+04h) != nullptr`, calls 00419CC0 (0088B3E0) and 00BD1510 (0088B3E7)
// with `(data, len + 1, 1)`.
void native_string_release_storage_0088b3d0(
    std::uint32_t len,
    void* data,
    DamageableClassHost& pool_host) {
    if (data != nullptr) {
        pool_host.return_pool_block_00bd1510(data, len + 1u);
    }
}

} // namespace bsp::landscape_lua_seg55_argon

// Segment 60, reconstruction pass (packet cc13-seg49). See include/bsp/cc13_seg49.hpp and
// docs/CC13_SEG49.md. Every routine names its address, its original ABI and what was
// simplified.

#include "bsp/cc13_seg49.hpp"

#include <cmath>
#include <cstring>

#include "bsp/attack_target_classify.hpp"

namespace bsp::cc13_seg49 {

// 009222E0, ABI: __thiscall(ECX = registry, NativeString name by value) RET 8. Sole caller
// BSP_LuaBinding_RemoveWatch (0088D560): 005FE8B0(ECX = registry+4h, &name), then the by-value
// string's storage returns to the pool when it has data.
void watch_registry_remove_009222e0(Seg60Host& host, Raw registry, NativeStringRaw name)
{
    host.string_set_erase_005fe8b0(registry.p + 0x4, &name);
    if (name.data != nullptr) {
        host.pool_return_00bd1510(name.data, name.length + 1);
    }
}

// 00922A90, ABI: __fastcall(ECX = entity) RET -> EAX. Class id +C4h in {19h, 1Ah, 1Bh, 36h,
// 45h, 46h, 1Ch} (the CMP order). Callers 0097D650, 009813A0.
bool class_id_is_land_family_00922a90(Raw entity) noexcept
{
    const int id = entity.at<int>(0xC4);
    return id == 0x19 || id == 0x1A || id == 0x1B || id == 0x36 || id == 0x45 || id == 0x46 || id == 0x1C;
}

// 00922AD0, ABI: __fastcall(ECX = entity) RET -> EAX. Non-null, +5Dh clear, 009229F0(entity,
// 6) and not IsKindOf(8). No caller found.
bool entity_is_surface_ship_target_00922ad0(Seg60Host& host, void* entity)
{
    if (entity == nullptr || Raw{static_cast<std::uint8_t*>(entity)}.at<std::uint8_t>(0x5D) != 0) {
        return false;
    }
    // 009229F0 inline (bsp/attack_target_classify.hpp): kind 6 or a LandFort faked as one.
    const bool is_kind = host.is_kind_of_5c(entity, 6);
    bool attackable = is_kind;
    if (!is_kind && host.is_kind_of_5c(entity, 0x1B)) {
        Raw cls{static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(entity)}.ptr(0x538))};
        attackable = bsp::faked_family_00922990(cls.at<int>(0x178), 6);
    }
    if (!attackable) {
        return false;
    }
    return !host.is_kind_of_5c(entity, 8);
}

// 00922DE0, ABI: __thiscall(ECX = self, source) RET 4 -> EAX. The spawn descriptor's copy
// constructor (vtable 00D03D94): kind +4h copied; kind 1 clones the property bag (008F41F0,
// ECX = source+8h); kind 3 copies +8h; kind 2 and every other kind leave +8h unwritten.
void* spawn_descriptor_copy_00922de0(Seg60Host& host, Raw self, Raw source)
{
    self.at<std::uint32_t>(0x0) = kSpawnDescriptorVtable;
    const int kind = source.at<int>(0x4);
    self.at<int>(0x4) = kind;
    if (kind == 1) {
        self.ptr(0x8) = host.property_bag_clone_008f41f0(source.ptr(0x8));
    } else if (kind == 3) {
        self.ptr(0x8) = source.ptr(0x8);
    }
    return self.p;
}

// 00922E50, ABI: __fastcall(ECX = self) RET. Kinds 1 and 2 own +8h and give it its scalar
// deleting destructor (vtable[0](1)) when non-null.
void spawn_descriptor_destruct_00922e50(Seg60Host& host, Raw self)
{
    self.at<std::uint32_t>(0x0) = kSpawnDescriptorVtable;
    if (self.at<int>(0x4) == 1 && self.ptr(0x8) != nullptr) {
        host.scalar_deleting_destructor_0(self.ptr(0x8), 1);
    }
    if (self.at<int>(0x4) == 2 && self.ptr(0x8) != nullptr) {
        host.scalar_deleting_destructor_0(self.ptr(0x8), 1);
    }
}

// 00922EC0, ABI: __fastcall(ECX = node) RET. A negative +58h is resolved from the parent
// (+3Ch) recursively, then copied; no null check on the parent, as the image.
void resolve_inherited_58_00922ec0(Raw node) noexcept
{
    if (node.at<int>(0x58) < 0) {
        Raw parent{static_cast<std::uint8_t*>(node.ptr(0x3C))};
        resolve_inherited_58_00922ec0(parent);
        node.at<int>(0x58) = Raw{static_cast<std::uint8_t*>(node.ptr(0x3C))}.at<int>(0x58);
    }
}

// 00923230, ABI: __fastcall(ECX = vector {data, count, capacity}) RET. No caller found. The
// unconditional growth step: capacity = capacity * 2 + 2, new block (00BF55BE), per-slot copy
// skipped for a null block, the old data freed (00BF6989) when non-null.
void pointer_vector_grow_00923230(Seg60Host& host, Raw vector)
{
    const std::uint32_t capacity = vector.at<std::uint32_t>(0x8) * 2 + 2;
    vector.at<std::uint32_t>(0x8) = capacity;
    auto* block = static_cast<std::uint32_t*>(host.operator_new_00bf55be(capacity * 4));
    auto* old = static_cast<std::uint32_t*>(vector.ptr(0x0));
    for (std::uint32_t i = 0; i < vector.at<std::uint32_t>(0x4); ++i) {
        if (block + i != nullptr) {
            block[i] = old[i];
        }
    }
    if (old != nullptr) {
        host.free_00bf6989(old);
    }
    vector.ptr(0x0) = block;
}

// 00923420, ABI: __thiscall(ECX = self, source) RET 4 -> EAX. Copy constructor of a {data,
// count, capacity} array of 10h elements: empty source -> all zero (+0h, +8h, +4h); else
// capacity = count = source count, a block of capacity * 10h (00BF55BE), each element copied
// unless its slot address is null.
void* vec16_copy_construct_00923420(Seg60Host& host, Raw self, Raw source)
{
    const std::uint32_t count = source.at<std::uint32_t>(0x4);
    if (count == 0) {
        self.at<std::uint32_t>(0x0) = 0;
        self.at<std::uint32_t>(0x8) = 0;
        self.at<std::uint32_t>(0x4) = 0;
        return self.p;
    }
    self.at<std::uint32_t>(0x8) = count;
    self.ptr(0x0) = host.operator_new_00bf55be(count << 4);
    self.at<std::uint32_t>(0x4) = self.at<std::uint32_t>(0x8);
    for (std::uint32_t i = 0; i < self.at<std::uint32_t>(0x4); ++i) {
        auto* slot = static_cast<std::uint8_t*>(self.ptr(0x0)) + i * 0x10;
        if (slot != nullptr) {
            std::memcpy(slot, static_cast<std::uint8_t*>(source.ptr(0x0)) + i * 0x10, 0x10);
        }
    }
    return self.p;
}

// 009234D0, ABI: __thiscall(ECX = self, capacity) RET 4. Unconditional reallocation of the 10h
// array: capacity stored first, block (00BF55BE), per-element copy skipped for a null block,
// old data freed (00BF6989) when non-null.
void vec16_reserve_009234d0(Seg60Host& host, Raw self, std::uint32_t capacity)
{
    self.at<std::uint32_t>(0x8) = capacity;
    auto* block = static_cast<std::uint8_t*>(host.operator_new_00bf55be(capacity << 4));
    for (std::uint32_t i = 0; i < self.at<std::uint32_t>(0x4); ++i) {
        if (block + i * 0x10 != nullptr) {
            std::memcpy(block + i * 0x10, static_cast<std::uint8_t*>(self.ptr(0x0)) + i * 0x10, 0x10);
        }
    }
    if (self.ptr(0x0) != nullptr) {
        host.free_00bf6989(self.ptr(0x0));
    }
    self.ptr(0x0) = block;
}

// 00923620, ABI: __fastcall(ECX = singleton) RET. Unwind target (00CA6AA0): clears 0109CE90 and
// reinstalls the singleton-base vtable 00CE3818.
void singleton_reset_base_0109ce90_00923620(Seg60Host& host, Raw singleton)
{
    host.global_pointer(0x0109CE90) = nullptr;
    singleton.at<std::uint32_t>(0x0) = kSingletonBaseVtable;
}

// 00923680, ABI: __fastcall(ECX = singleton) RET. As 00923620 for 00F899EC.
void singleton_reset_base_00f899ec_00923680(Seg60Host& host, Raw singleton)
{
    host.global_pointer(0x00F899EC) = nullptr;
    singleton.at<std::uint32_t>(0x0) = kSingletonBaseVtable;
}

// 00923760, ABI: __thiscall(ECX = vector {data, count, capacity}, const dword*) RET 4. A full
// vector grows to capacity * 2 + 2 (00BF55BE, per-slot copy, free 00BF6989 when non-null);
// the value is stored unless the slot address is null; the count always advances.
void pointer_vector_push_back_00923760(Seg60Host& host, Raw vector, const std::uint32_t* value)
{
    if (vector.at<std::uint32_t>(0x4) == vector.at<std::uint32_t>(0x8)) {
        pointer_vector_grow_00923230(host, vector);  // the same inlined sequence
    }
    auto* slot = static_cast<std::uint32_t*>(vector.ptr(0x0)) + vector.at<std::uint32_t>(0x4);
    if (slot != nullptr) {
        *slot = *value;
    }
    vector.at<std::uint32_t>(0x4) += 1;
}

// 00923810, ABI: __thiscall(ECX = entity, levels) RET 4 -> EAX. The parent chain (+3Ch) walked
// up `levels` steps (at least one), stopping at null.
void* entity_ancestor_00923810(Raw entity, int levels) noexcept
{
    void* node = entity.ptr(0x3C);
    int remaining = levels - 1;
    while (node != nullptr && remaining > 0) {
        node = Raw{static_cast<std::uint8_t*>(node)}.ptr(0x3C);
        --remaining;
    }
    return node;
}

// 00923870, ABI: __thiscall(ECX = entity, a, b, c) RET 0Ch. vtable +98h(a, b, c), then +A0h().
void entity_place_and_commit_00923870(Seg60Host& host, void* entity, std::uint32_t a, std::uint32_t b, std::uint32_t c)
{
    host.entity_virtual_98(entity, a, b, c);
    host.entity_virtual_a0(entity);
}

// 00923990, ABI: __fastcall(ECX = record) RET -> EAX = record. 00470470 on the record itself.
void* hit_record_reset_self_00923990(Seg60Host& host, void* record)
{
    host.hit_record_reset_00470470(record);
    return record;
}

// 00923ED0, ABI: __thiscall(ECX = self, source) RET 4 -> EAX. Copy constructor of a 54h record:
// dwords +0h..+28h, byte +2Ch, dwords +30h..+38h, the 10h-element array at +3Ch (the
// 00923420 sequence inline), then +48h..+50h.
void* think_record_copy_construct_00923ed0(Seg60Host& host, Raw self, Raw source)
{
    for (std::size_t off = 0; off <= 0x28; off += 4) {
        self.at<std::uint32_t>(off) = source.at<std::uint32_t>(off);
    }
    self.at<std::uint8_t>(0x2C) = source.at<std::uint8_t>(0x2C);
    self.at<std::uint32_t>(0x30) = source.at<std::uint32_t>(0x30);
    self.at<std::uint32_t>(0x34) = source.at<std::uint32_t>(0x34);
    self.at<std::uint32_t>(0x38) = source.at<std::uint32_t>(0x38);
    vec16_copy_construct_00923420(host, Raw{self.p + 0x3C}, Raw{source.p + 0x3C});
    self.at<std::uint32_t>(0x48) = source.at<std::uint32_t>(0x48);
    self.at<std::uint32_t>(0x4C) = source.at<std::uint32_t>(0x4C);
    self.at<std::uint32_t>(0x50) = source.at<std::uint32_t>(0x50);
    return self.p;
}

// 00923FD0, ABI: __thiscall(ECX = self, capacity) RET 4. 009234D0's reallocation, only when the
// request exceeds the current capacity (unsigned compare).
void vec16_reserve_at_least_00923fd0(Seg60Host& host, Raw self, std::uint32_t capacity)
{
    if (self.at<std::uint32_t>(0x8) < capacity) {
        vec16_reserve_009234d0(host, self, capacity);
    }
}

// 00924200, ABI: __fastcall(ECX = self) RET -> EAX. vtable 00D190C8 and +4h = 00BD1860() (a
// tracked critical section).
void* deferred_entity_event_lock_construct_00924200(Seg60Host& host, Raw self)
{
    self.at<std::uint32_t>(0x0) = kDeferredEventLockVtable;
    self.ptr(0x4) = host.critical_section_create_00bd1860();
    return self.p;
}

// 009245A0, ABI: __thiscall(ECX = entity, delta_b8, delta_b4) RET 8. Every entity up the parent
// chain (+3Ch) adds delta_b4 to +B4h and delta_b8 to +B8h. At the root, with a world (+30h),
// the root is removed from the world list +0Ch (004845A0) when its +B8h went from non-zero to
// zero, and pushed (00484540) when it went from zero to non-zero.
void entity_adjust_world_list_counters_009245a0(Seg60Host& host, Raw entity, int delta_b8, int delta_b4)
{
    Raw node = entity;
    int before = 0;
    int after = 0;
    for (;;) {
        before = node.at<int>(0xB8);
        node.at<int>(0xB4) += delta_b4;
        after = before + delta_b8;
        node.at<int>(0xB8) = after;
        if (node.ptr(0x3C) == nullptr) {
            break;
        }
        node = Raw{static_cast<std::uint8_t*>(node.ptr(0x3C))};
    }
    auto* world = static_cast<std::uint8_t*>(node.ptr(0x30));
    if (world == nullptr) {
        return;
    }
    if (before != 0) {
        if (after == 0) {
            host.unit_list_remove_004845a0(world + 0xC, node.p);
        }
    } else if (after != 0) {
        host.unit_list_push_back_00484540(world + 0xC, node.p);
    }
}

// 00924610, ABI: __thiscall(ECX = entity, nine floats by value, one unused dword) RET 28h.
// Translation +A4h, row 2 +94h, row 1 +84h, then 0085DC80 on the local matrix +74h; the
// pose-valid bytes are not touched.
void entity_set_local_frame_00924610(Seg60Host& host, Raw entity, const float* translation, const float* row2,
                                     const float* row1)
{
    for (int i = 0; i < 3; ++i) {
        entity.at<float>(0xA4 + i * 4) = translation[i];
    }
    for (int i = 0; i < 3; ++i) {
        entity.at<float>(0x94 + i * 4) = row2[i];
    }
    for (int i = 0; i < 3; ++i) {
        entity.at<float>(0x84 + i * 4) = row1[i];
    }
    host.orthonormalize_basis_rows_0085dc80(&entity.at<float>(0x74));
}

// 00924690, ABI: __thiscall(ECX = entity, a, b, c) RET 0Ch. With a decal list (+168h) and a
// non-null vtable +20h result (called twice), 007407C0(ECX = list, [[r+160h]+0Ch], b, c, a).
void entity_add_decal_00924690(Seg60Host& host, Raw entity, std::uint32_t a, std::uint32_t b, std::uint32_t c)
{
    void* list = entity.ptr(0x168);
    if (list == nullptr || host.entity_virtual_20(entity.p) == nullptr) {
        return;
    }
    Raw r{static_cast<std::uint8_t*>(host.entity_virtual_20(entity.p))};
    const std::uint32_t x = Raw{static_cast<std::uint8_t*>(r.ptr(0x160))}.at<std::uint32_t>(0xC);
    host.decal_list_add_007407c0(list, x, b, c, a);
}

// 00924770, ABI: __fastcall(ECX = list {?, head +4h, size +8h}) RET. MSVC list clear: the head
// sentinel is relinked to itself, the size zeroed, then each node freed (00BF65AC) walking
// +0h (next) until the sentinel.
void sentinel_list_clear_00924770(Seg60Host& host, Raw list)
{
    auto** head = static_cast<void**>(list.ptr(0x4));
    void* node = head[0];
    head[0] = head;
    static_cast<void**>(list.ptr(0x4))[1] = list.ptr(0x4);
    list.at<std::uint32_t>(0x8) = 0;
    while (node != list.ptr(0x4)) {
        void* next = *static_cast<void**>(node);
        host.free_00bf65ac(node);
        node = next;
    }
}

// 00924AC0, ABI: __cdecl() -> EAX. A 68h head node (00BF681B) linked to itself through +0h
// and +4h (each store guarded as the image: the first by the node, the second by node+4h).
void* sentinel_list_buy_head_00924ac0(Seg60Host& host)
{
    auto* node = static_cast<std::uint8_t*>(host.operator_new_00bf681b(0x68));
    if (node != nullptr) {
        *reinterpret_cast<void**>(node) = node;
    }
    if (node + 4 != nullptr) {
        *reinterpret_cast<void**>(node + 4) = node;
    }
    return node;
}

// 00924BE0, ABI: __thiscall(ECX = self, source) RET 4. Assignment of the 10h array: the count is
// zeroed when non-zero, the block reallocated (009234D0's sequence, copying nothing because the
// count is already zero) when source count > capacity, then count = source count and each
// element copied unless its slot address is null.
void vec16_assign_00924be0(Seg60Host& host, Raw self, Raw source)
{
    if (self.at<std::uint32_t>(0x4) != 0) {
        self.at<std::uint32_t>(0x4) = 0;
    }
    const std::uint32_t count = source.at<std::uint32_t>(0x4);
    if (self.at<std::uint32_t>(0x8) < count) {
        vec16_reserve_009234d0(host, self, count);
    }
    self.at<std::uint32_t>(0x4) = source.at<std::uint32_t>(0x4);
    for (std::uint32_t i = 0; i < source.at<std::uint32_t>(0x4); ++i) {
        auto* slot = static_cast<std::uint8_t*>(self.ptr(0x0)) + i * 0x10;
        if (slot != nullptr) {
            std::memcpy(slot, static_cast<std::uint8_t*>(source.ptr(0x0)) + i * 0x10, 0x10);
        }
    }
}

// 00924CF0, ABI: __fastcall(ECX = object) RET. No caller found. +8h = 0; InterlockedDecrement
// on +4h; at zero, a tail jump to vtable[0] with ECX = object and no stack argument.
void refcounted_release_00924cf0(Seg60Host& host, Raw object)
{
    object.at<std::uint32_t>(0x8) = 0;
    if (host.interlocked_decrement(reinterpret_cast<volatile long*>(object.p + 0x4)) == 0) {
        host.refcounted_virtual_0(object.p);
    }
}

namespace {

ArchiveArg key60(const char* text) noexcept { return {0, reinterpret_cast<std::uintptr_t>(text)}; }
ArchiveArg int60(std::uint32_t value) noexcept { return {1, value}; }

}  // namespace

// 00924D90, ABI: __thiscall(ECX = entity, archive) RET 4. The base entity's archive writer:
// "type" (+C4h), "name" (vtable +10h, only when non-null; the virtual is called twice), "race"
// (+58h), "party" (+54h), "parentID" ({4, +3Ch}), "localFrame" ({7, &+74h}); then into
// "_entity": "active" ({3, byte +5Ch}), "deadMeat" ({3, 1}) when +5Dh is set, and
// "invincible" ({2, float +150h}) unless 00BD5680 finds it equal to {2, 0.0f}; then leave.
// Key strings: type 00CE4AD0, name 00CE49B8, race 00D190EC, party 00D14754, parentID 00CF82B0,
// localFrame 00CF82A4, _entity 00D190E4, active 00CE4A08, deadMeat 00D190D8, invincible 00D190CC.
void entity_write_archive_00924d90(Seg60Host& host, Raw entity, void* archive)
{
    host.archive_write_0c(archive, key60("type"), int60(entity.at<std::uint32_t>(0xC4)));
    if (host.entity_name_10(entity.p) != 0) {
        host.archive_write_0c(archive, key60("name"), {0, host.entity_name_10(entity.p)});
    }
    host.archive_write_0c(archive, key60("race"), int60(entity.at<std::uint32_t>(0x58)));
    host.archive_write_0c(archive, key60("party"), int60(entity.at<std::uint32_t>(0x54)));
    host.archive_write_0c(archive, key60("parentID"), {4, reinterpret_cast<std::uintptr_t>(entity.ptr(0x3C))});
    host.archive_write_0c(archive, key60("localFrame"), {7, reinterpret_cast<std::uintptr_t>(entity.p + 0x74)});
    host.archive_enter_04(archive, key60("_entity"));
    host.archive_write_0c(archive, key60("active"), {3, entity.at<std::uint8_t>(0x5C)});
    if (entity.at<std::uint8_t>(0x5D) != 0) {
        host.archive_write_0c(archive, key60("deadMeat"), {3, 1});
    }
    std::uint32_t bits = 0;
    std::memcpy(&bits, &entity.at<float>(0x150), sizeof bits);
    const ArchiveArg value{2, bits};
    const ArchiveArg zero{2, 0};
    if (!host.tagged_value_equals_00bd5680(value, zero)) {
        host.archive_write_0c(archive, key60("invincible"), value);
    }
    host.archive_leave_08(archive);
}

// 00924F90, ABI: __thiscall(ECX = entity, parent) RET 4. Re-parenting: unless +3Ch is already
// the parent, the counters +B8h/+B4h are withdrawn up the old chain (009245A0(-b8, -b4)) when
// either is non-zero; the entity leaves its old chain (00924710 with ECX = old parent+48h, or
// [world+8h] for a root), +3Ch = parent, and it is appended to the parent's child chain {head
// +48h, tail +4Ch, count +50h} or, for a null parent, the world roots [[+30h]+8h] {head, tail
// +4h, count +8h} (links: previous +40h, next +44h); then the counters are re-applied.
void entity_set_parent_00924f90(Seg60Host& host, Raw entity, void* parent)
{
    if (entity.ptr(0x3C) == parent) {
        return;
    }
    const int b8 = entity.at<int>(0xB8);
    const int b4 = entity.at<int>(0xB4);
    if (b8 != 0 || b4 != 0) {
        entity_adjust_world_list_counters_009245a0(host, entity, -b8, -b4);
    }
    void* old_chain = entity.ptr(0x3C) != nullptr
                          ? static_cast<void*>(static_cast<std::uint8_t*>(entity.ptr(0x3C)) + 0x48)
                          : Raw{static_cast<std::uint8_t*>(entity.ptr(0x30))}.ptr(0x8);
    host.entity_unlink_from_sibling_chain_00924710(old_chain, entity.p);
    entity.ptr(0x3C) = parent;
    Raw chain = parent != nullptr ? Raw{static_cast<std::uint8_t*>(parent) + 0x48}
                                  : Raw{static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(entity.ptr(0x30))}.ptr(0x8))};
    if (chain.at<std::uint32_t>(0x8) != 0) {
        Raw{static_cast<std::uint8_t*>(chain.ptr(0x4))}.ptr(0x44) = entity.p;
    } else {
        chain.ptr(0x0) = entity.p;
    }
    entity.ptr(0x40) = chain.ptr(0x4);
    chain.ptr(0x4) = entity.p;
    entity.ptr(0x44) = nullptr;
    chain.at<std::uint32_t>(0x8) += 1;
    if (b8 != 0 || b4 != 0) {
        entity_adjust_world_list_counters_009245a0(host, entity, b8, b4);
    }
}

// 00925180, ABI: __thiscall(ECX = registry, const NativeString* name) RET 4 -> EAX. Walks the
// entities from [[registry+4h]] through +38h. With no name data, the first entity whose
// vtable +10h name is null or empty; otherwise the first whose name compares equal by
// _stricmp (00BF7FBF), where a null entity name compares as the name's length.
void* entity_registry_find_by_name_00925180(Seg60Host& host, Raw registry, const NativeStringRaw& name)
{
    void* entity = *static_cast<void**>(registry.ptr(0x4));
    while (entity != nullptr) {
        const auto* own = reinterpret_cast<const char*>(host.entity_name_10(entity));
        if (name.data == nullptr) {
            if (own == nullptr || own[0] == '\0') {
                return entity;
            }
        } else {
            const int result = own == nullptr ? static_cast<int>(name.length) : host.stricmp_00bf7fbf(name.data, own);
            if (result == 0) {
                return entity;
            }
        }
        entity = Raw{static_cast<std::uint8_t*>(entity)}.ptr(0x38);
    }
    return nullptr;
}

// 009253A0, ABI: __fastcall(ECX = destination, EDX = source) RET. Placement copy of a 60h
// record when the destination is non-null: 00923ED0, then +54h/+58h/+5Ch as floats (the image
// moves them through FLD/FSTP, which would quiet a signalling NaN; a plain float copy here).
void think_record_placement_copy_009253a0(Seg60Host& host, void* destination, Raw source)
{
    if (destination == nullptr) {
        return;
    }
    Raw self{static_cast<std::uint8_t*>(destination)};
    think_record_copy_construct_00923ed0(host, self, source);
    self.at<float>(0x54) = source.at<float>(0x54);
    self.at<float>(0x58) = source.at<float>(0x58);
    self.at<float>(0x5C) = source.at<float>(0x5C);
}

// 00925400, ABI: __fastcall(ECX = self) RET. vtable 00D190B4, 0041CC80 on self+4h, 0109CE90
// cleared, singleton-base vtable.
void weak_owner_mutex_destruct_00925400(Seg60Host& host, Raw self)
{
    self.at<std::uint32_t>(0x0) = kWeakOwnerMutexVtable;
    host.critical_section_destroy_owned_0041cc80(self.p + 0x4);
    host.global_pointer(0x0109CE90) = nullptr;
    self.at<std::uint32_t>(0x0) = kSingletonBaseVtable;
}

// 009255F0, ABI: __fastcall(ECX = link) RET -> AL. Under the observer lock (00694280's
// section, when present), whether link+8h is non-null.
bool observer_link_bound_009255f0(Seg60Host& host, Raw link)
{
    host.observer_lock_enter_00694280();
    const bool bound = link.ptr(0x8) != nullptr;
    host.observer_lock_leave_00694280();
    return bound;
}

// 00925630, ABI: __fastcall(ECX = self) RET. As 00925400 with vtable 00D190C0 and 00F899E4.
void pending_init_lock_destruct_00925630(Seg60Host& host, Raw self)
{
    self.at<std::uint32_t>(0x0) = kPendingInitLockVtable;
    host.critical_section_destroy_owned_0041cc80(self.p + 0x4);
    host.global_pointer(0x00F899E4) = nullptr;
    self.at<std::uint32_t>(0x0) = kSingletonBaseVtable;
}

// 00925710, ABI: __fastcall(ECX = self) RET. As 00925400 with vtable 00D190C8 and 00F899EC
// (the destructor of 00924200's object).
void deferred_entity_event_lock_destruct_00925710(Seg60Host& host, Raw self)
{
    self.at<std::uint32_t>(0x0) = kDeferredEventLockVtable;
    host.critical_section_destroy_owned_0041cc80(self.p + 0x4);
    host.global_pointer(0x00F899EC) = nullptr;
    self.at<std::uint32_t>(0x0) = kSingletonBaseVtable;
}

// 00925AF0, ABI: __fastcall(ECX = list) RET -> EAX. +4h = 00924A50(), +8h = 0.
void* stl_list_construct_00925af0(Seg60Host& host, Raw list)
{
    list.ptr(0x4) = host.stl_list_buy_sentinel_00924a50();
    list.at<std::uint32_t>(0x8) = 0;
    return list.p;
}

// 00925B10, ABI: __fastcall(ECX = list) RET -> EAX. +4h = 00924AC0(), +8h = 0.
void* sentinel_list_construct_00925b10(Seg60Host& host, Raw list)
{
    list.ptr(0x4) = sentinel_list_buy_head_00924ac0(host);
    list.at<std::uint32_t>(0x8) = 0;
    return list.p;
}

// 00925BD0, ABI: __fastcall(ECX = list) RET. 00924770 with 004704B0 on each node's +8h before
// the free.
void hit_record_list_clear_00925bd0(Seg60Host& host, Raw list)
{
    auto** head = static_cast<void**>(list.ptr(0x4));
    void* node = head[0];
    head[0] = head;
    static_cast<void**>(list.ptr(0x4))[1] = list.ptr(0x4);
    list.at<std::uint32_t>(0x8) = 0;
    while (node != list.ptr(0x4)) {
        void* next = *static_cast<void**>(node);
        host.hit_record_destroy_004704b0(static_cast<std::uint8_t*>(node) + 0x8);
        host.free_00bf65ac(node);
        node = next;
    }
}

namespace {

void release60(Seg60Host& host, const NativeStringRaw& s)
{
    if (s.data != nullptr) {
        host.pool_return_00bd1510(s.data, s.length + 1);
    }
}

// The inlined NativeString-from-C-string of 00926486..00926500: zeroed, resized (0041DD40,
// keep 1), and the text copied with its terminator when the resize produced data.
void string_from(Seg60Host& host, NativeStringRaw& s, const char* text)
{
    s.length = 0;
    s.data = nullptr;
    host.native_string_resize_0041dd40(s, static_cast<std::uint32_t>(std::strlen(text)), 1);
    if (s.data != nullptr) {
        std::memcpy(s.data, text, s.length + 1);
    }
}

}  // namespace

// 00926420, ABI: __fastcall(ECX = out NativeString, EDX = entity) RET -> EAX = out. The
// backslash-joined name path: out = the entity's vtable +10h name (0041E870), then for each
// ancestor (+3Ch) out = name + "\" (00CE7894) + out through two 004261A0 temporaries copied back
// with 0041DD40 and memcpy; the temporaries return to the pool in reverse order of creation.
NativeStringRaw* entity_full_path_name_00926420(Seg60Host& host, NativeStringRaw& out, void* entity)
{
    host.native_string_construct_0041e870(out, reinterpret_cast<const char*>(host.entity_name_10(entity)));
    for (void* parent = Raw{static_cast<std::uint8_t*>(entity)}.ptr(0x3C); parent != nullptr;
         parent = Raw{static_cast<std::uint8_t*>(parent)}.ptr(0x3C)) {
        NativeStringRaw separator{};
        string_from(host, separator, "\\");
        NativeStringRaw name{};
        string_from(host, name, reinterpret_cast<const char*>(host.entity_name_10(parent)));
        NativeStringRaw joined{};
        NativeStringRaw* left = host.native_string_concat_004261a0(name, joined, separator);
        NativeStringRaw path{};
        NativeStringRaw* result = host.native_string_concat_004261a0(*left, path, out);
        if (&out != result) {
            host.native_string_resize_0041dd40(out, result->length, 1);
            if (result->length != 0) {
                std::memcpy(out.data, result->data, out.length);
            }
        }
        release60(host, path);
        release60(host, joined);
        release60(host, name);
        release60(host, separator);
    }
    return &out;
}

// 00926B80, ABI: __thiscall(ECX = list, a, b, c, d, unused) RET 14h. assign(first {a, b}, last
// {c, d}): the 00924770 clear inline, then 009269B0(list, head->next, a, b, c, d, c) with the
// arguments exactly as 00926BC0..00926BD2 push them.
void stl_list_assign_range_00926b80(Seg60Host& host, Raw list, std::uint32_t a, std::uint32_t b, std::uint32_t c,
                                    std::uint32_t d, std::uint32_t unused)
{
    static_cast<void>(unused);
    sentinel_list_clear_00924770(host, list);
    void* where = *static_cast<void**>(list.ptr(0x4));
    host.stl_list_insert_range_009269b0(list.p, list.p, where, a, b, c, d, c);
}

// 00927790, ABI: __fastcall(ECX = registry) RET. 009516B0 on registry+54h, then on the
// registry itself (the second runs as the EH-protected member's destructor).
void entity_registry_destroy_id_tables_00927790(Seg60Host& host, Raw registry)
{
    host.entity_registry_destroy_id_table_slots_009516b0(registry.p + 0x54);
    host.entity_registry_destroy_id_table_slots_009516b0(registry.p);
}

// 00927810, ABI: __thiscall(ECX = unit, mask, argument) RET 8. vtable +154h(i, argument) for each
// bit i of mask, i = 0..8.
void unit_apply_role_mask_00927810(Seg60Host& host, void* unit, std::uint32_t mask, std::uint32_t argument)
{
    std::uint32_t bit = 1;
    for (int i = 0; i < 9; ++i) {
        if ((mask & bit) != 0) {
            host.unit_virtual_154(unit, i, argument);
        }
        bit *= 2;
    }
}

// 00927850, ABI: __thiscall(ECX = unit, value) RET 4 -> AL. Whether one of the nine dwords at
// +1ACh equals value.
bool unit_has_role_slot_value_00927850(Raw unit, int value) noexcept
{
    for (int i = 0; i < 9; ++i) {
        if (unit.at<int>(0x1AC + i * 4) == value) {
            return true;
        }
    }
    return false;
}

// 00927880, ABI: __fastcall(ECX = entity) RET -> EAX. o = vtable +114h(); a tail jump to
// o vtable +18h when o is non-null, else 0.
std::uint32_t controlled_target_00927880(Seg60Host& host, void* entity)
{
    void* object = host.entity_virtual_114(entity);
    if (object == nullptr) {
        return 0;
    }
    return host.object_virtual_18(object);
}

// 009278A0, ABI: __thiscall(ECX = unit, value) RET 4. +180h = value, then vtable +148h(1FFh,
// value).
void unit_set_role_009278a0(Seg60Host& host, Raw unit, std::uint32_t value)
{
    unit.at<std::uint32_t>(0x180) = value;
    host.unit_virtual_148(unit.p, 0x1FF, value);
}

// 00927C90, ABI: __thiscall(ECX = unit, index) RET 4 -> AL. The role word +188h + index * 4 is 9,
// or one of 0..7 (the counter is compared before it passes 7; unsigned word).
bool unit_role_word_is_player_or_open_00927c90(Raw unit, int index) noexcept
{
    const std::uint32_t word = unit.at<std::uint32_t>(0x188 + index * 4);
    for (std::uint32_t i = 0;; ++i) {
        if (word == 9 || word == i) {
            return true;
        }
        if (static_cast<int>(i + 1) > 7) {
            return false;
        }
    }
}

// 00927CC0, ABI: __thiscall(ECX = unit, value) RET 4 -> EAX. Bit i set for each of the nine role
// words (+188h) that is 9 or equals value.
std::uint32_t unit_role_mask_for_00927cc0(Raw unit, int value) noexcept
{
    std::uint32_t mask = 0;
    for (int i = 0; i < 9; ++i) {
        const int word = unit.at<int>(0x188 + i * 4);
        if (word == 9 || word == value) {
            mask |= 1u << i;
        }
    }
    return mask;
}

// 00927E50, ABI: __thiscall(ECX = unit, mask, from, to) RET 0Ch. For each role i in mask whose
// word (+188h) equals from: word = to; when the unit is IsKindOf(2), the owner word (+1ACh) is
// not 8 and the new word is 8, a ROLEOWNER message (007796F0(bit, owner, 0, 0) on the stack) is
// routed (0077C2A0(ECX = unit, message, 0, 0)) and its vtable reset to the base 00CE4974. Then
// 00696350(ECX = unit, EDX = 5, 0).
void unit_reassign_roles_00927e50(Seg60Host& host, Raw unit, std::uint32_t mask, int from, int to)
{
    std::uint32_t bit = 1;
    for (int i = 0; i < 9; ++i, bit += bit) {
        int& word = unit.at<int>(0x188 + i * 4);
        if ((mask & bit) == 0 || word != from) {
            continue;
        }
        word = to;
        if (!host.is_kind_of_5c(unit.p, 2)) {
            continue;
        }
        const int owner = unit.at<int>(0x1AC + i * 4);
        if (owner == 8 || word != 8) {
            continue;
        }
        std::uint8_t bytes[0x30] = {};
        Raw message{bytes};
        void* built = host.session_message_roleowner_construct_007796f0(message, bit, owner, 0, 0);
        host.session_route_message_0077c2a0(unit.p, built, 0, 0);
        message.at<std::uint32_t>(0x0) = kSessionMessageBaseVtable;
    }
    host.observer_notify_00696350(unit.p, 5, 0);
}

namespace {

// The checked-iterator dereference the image inlines: begin == head reports 00BF6713, the head is
// re-read, and an unchanged equality reports again.
void check_not_end(Seg60Host& host, Raw list, void* node)
{
    if (node == list.ptr(0x4)) {
        host.invalid_iterator_00bf6713();
        if (node == list.ptr(0x4)) {
            host.invalid_iterator_00bf6713();
        }
    }
}

// push_back of 00927610: node = 00924B10(list, head, head->prev, &value), 009267F0(list, 1),
// head->prev = node, node->prev->next = node.
void list_push_back(Seg60Host& host, Raw list, void* const* value)
{
    auto** head = static_cast<void**>(list.ptr(0x4));
    void* node = host.stl_list_buy_node_00924b10(list.p, head, head[1], value);
    host.stl_list_incsize_009267f0(list.p, 1);
    head[1] = node;
    void* prev = static_cast<void**>(node)[1];
    static_cast<void**>(prev)[0] = node;
}

}  // namespace

// 00927610, ABI: __fastcall(ECX = entity, DL = flag) RET. Called at 00898932 with DL = 0. Nodes
// are {next +0h, prev +4h, value +8h}. While the pending-init list at 00F899CC is not empty, its
// back node ([head+4h], checked each time it is read) is copied to the back of a local list
// unless it holds this entity, then erased (unlinked, freed by 00BF65AC, size decremented); so
// the others land in the local list in reverse order. The entity is then pushed onto the
// pending list, 00925F20 runs with CL = flag, the local list is assigned back to the pending
// list (00926B80 with {local, begin}, {local, head} and the entity as the unused fifth
// argument), and the local list is destroyed (00924A70).
void sentity_init_now_00927610(Seg60Host& host, void* entity, std::uint8_t flag)
{
    std::uint8_t local_bytes[0xC] = {};
    Raw local{local_bytes};
    local.ptr(0x4) = host.stl_list_buy_sentinel_00924a50();
    local.at<std::uint32_t>(0x8) = 0;
    Raw pending = host.global_object(kPendingInitList);
    while (pending.at<std::uint32_t>(0x8) != 0) {
        void* back = static_cast<void**>(pending.ptr(0x4))[1];
        check_not_end(host, pending, back);
        if (static_cast<void**>(back)[2] != entity) {
            back = static_cast<void**>(pending.ptr(0x4))[1];
            check_not_end(host, pending, back);
            list_push_back(host, local, &static_cast<void**>(back)[2]);
        }
        back = static_cast<void**>(pending.ptr(0x4))[1];
        check_not_end(host, pending, back);
        if (back == pending.ptr(0x4)) {
            continue;  // 009276F0: nothing erased when the back is still the head
        }
        void* next = static_cast<void**>(back)[0];
        void* prev = static_cast<void**>(back)[1];
        static_cast<void**>(prev)[0] = next;
        static_cast<void**>(next)[1] = prev;
        host.free_00bf65ac(back);
        pending.at<std::uint32_t>(0x8) -= 1;
    }
    void* const value = entity;
    list_push_back(host, pending, &value);
    host.sentity_init_all_00925f20(flag);
    void* begin = static_cast<void**>(local.ptr(0x4))[0];
    stl_list_assign_range_00926b80(host, pending, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(local.p)),
                                   static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(begin)),
                                   static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(local.p)),
                                   static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(local.ptr(0x4))),
                                   static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(entity)));
    host.stl_list_destroy_00924a70(local.p);
}

// 00928450, ABI: __thiscall(ECX = entity, archive) RET 4. The game entity's archive writer:
// 00924D90, then into "_gameEntity" (00D192B0): "thinkFunction" (00D192A0, {0, +1D8h}) when
// set, "timing" (00D19298, {2, float +1E0h}) when byte +1DCh is set, and into "roleAvailable"
// (00D19288) each role word (+188h, nine) that is not 9 as {1, i} -> {1, word}; leave twice.
void game_entity_write_archive_00928450(Seg60Host& host, Raw entity, void* archive)
{
    entity_write_archive_00924d90(host, entity, archive);
    host.archive_enter_04(archive, key60("_gameEntity"));
    if (entity.ptr(0x1D8) != nullptr) {
        host.archive_write_0c(archive, key60("thinkFunction"), {0, reinterpret_cast<std::uintptr_t>(entity.ptr(0x1D8))});
    }
    if (entity.at<std::uint8_t>(0x1DC) != 0) {
        std::uint32_t bits = 0;
        std::memcpy(&bits, &entity.at<float>(0x1E0), sizeof bits);
        host.archive_write_0c(archive, key60("timing"), {2, bits});
    }
    host.archive_enter_04(archive, key60("roleAvailable"));
    for (std::uint32_t i = 0; i < 9; ++i) {
        const std::uint32_t word = entity.at<std::uint32_t>(0x188 + i * 4);
        if (word != 9) {
            host.archive_write_0c(archive, int60(i), int60(word));
        }
    }
    host.archive_leave_08(archive);
    host.archive_leave_08(archive);
}

// 009285C0, ABI: __fastcall(ECX = self) RET. vtable 00D19284, 0041CC80 on self+4h, 00F899FC
// cleared, singleton-base vtable.
void scene_lock_destruct_009285c0(Seg60Host& host, Raw self)
{
    self.at<std::uint32_t>(0x0) = kSceneLockVtable;
    host.critical_section_destroy_owned_0041cc80(self.p + 0x4);
    host.global_pointer(0x00F899FC) = nullptr;
    self.at<std::uint32_t>(0x0) = kSingletonBaseVtable;
}

// 009291D0, ABI: __thiscall(ECX = entity, argument) RET 4. IsKindOf(2) is asked first (both arms
// differ only in the stack slot of the temporary); with a think function name (+1D8h) the
// temporary NativeString (0041E870) goes to 009290A0(&name, argument, 0, -1) and returns to the
// pool afterwards.
void entity_invoke_think_once_009291d0(Seg60Host& host, Raw entity, std::uint32_t argument)
{
    static_cast<void>(host.is_kind_of_5c(entity.p, 2));
    const char* function = static_cast<const char*>(entity.ptr(0x1D8));
    if (function == nullptr) {
        return;
    }
    NativeStringRaw name{};
    host.native_string_construct_0041e870(name, function);
    host.mission_entity_register_lua_script_009290a0(entity.p, &name, argument, 0, -1);
    release60(host, name);
}

// 00929C80, ABI: __thiscall(ECX = entity, a, b, c) RET 0Ch. 009258F0(a, b, c), then the
// entity joins the world list [+30h]+438h (00484540).
void entity_attach_and_register_00929c80(Seg60Host& host, Raw entity, std::uint32_t a, std::uint32_t b, std::uint32_t c)
{
    host.scene_node_attach_to_parents_009258f0(entity.p, a, b, c);
    host.unit_list_push_back_00484540(static_cast<std::uint8_t*>(entity.ptr(0x30)) + 0x438, entity.p);
}

// 00929D70, ABI: __fastcall(ECX = entity) RET. The spatial node (+1C4h; null for a null entity,
// though the routine then dereferences it anyway) leaves the index (0042E630 then 0098A500);
// the first node of the world list +438h (head +43Ch, {+4h next, +8h entity}) holding the entity
// is erased (004837D0); a physics body (+354h) is removed through [game+18h] (00C34F70); +354h
// and +2BCh are cleared. With a partner (+33Ch) the partner's back link is cleared; otherwise a
// node at +34Ch is unlinked and released (00B6DFA0) and cleared. Then 00926D90(1) and +384h = 1.
void entity_detach_and_kill_00929d70(Seg60Host& host, Raw entity)
{
    void* spatial = entity.p != nullptr ? static_cast<void*>(entity.p + 0x1C4) : nullptr;
    void* index = host.spatial_index_get_singleton_0042e630();
    host.spatial_index_detach_node_0098a500(index, spatial);
    auto* world = static_cast<std::uint8_t*>(entity.ptr(0x30));
    for (void* node = Raw{world}.ptr(0x43C); node != nullptr; node = Raw{static_cast<std::uint8_t*>(node)}.ptr(0x4)) {
        if (Raw{static_cast<std::uint8_t*>(node)}.ptr(0x8) == entity.p) {
            host.unit_list_erase_node_004837d0(world + 0x438, node);
            break;
        }
    }
    if (entity.ptr(0x354) != nullptr) {
        host.physics_remove_body_00c34f70(host.game_e188a8().ptr(0x18), entity.ptr(0x354));
    }
    void* partner = entity.ptr(0x33C);
    entity.ptr(0x354) = nullptr;
    entity.ptr(0x2BC) = nullptr;
    if (partner != nullptr) {
        Raw{static_cast<std::uint8_t*>(partner)}.ptr(0x33C) = nullptr;
    } else if (entity.ptr(0x34C) != nullptr) {
        host.node_unlink_and_release_00b6dfa0(entity.ptr(0x34C));
        entity.ptr(0x34C) = nullptr;
    }
    host.mission_entity_kill_00926d90(entity.p, 1);
    entity.at<std::uint8_t>(0x384) = 1;
}

// 00929E30, ABI: __fastcall(ECX = entity) RET via JMP 00923060. 00929D70 unless +384h is set.
void entity_shutdown_once_00929e30(Seg60Host& host, Raw entity)
{
    if (entity.at<std::uint8_t>(0x384) == 0) {
        entity_detach_and_kill_00929d70(host, entity);
    }
    host.trivial_body_00923060(entity.p);
}

namespace {

// The refcounted release the destructors inline: InterlockedDecrement on +4h, vtable[0]() at
// zero (ECX = object, no stack argument).
void release_refcounted(Seg60Host& host, void* object)
{
    if (host.interlocked_decrement(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(object) + 0x4)) == 0) {
        host.refcounted_virtual_0(object);
    }
}

}  // namespace

// 0092A040, ABI: __fastcall(ECX = entity) RET. The destructor behind vtable 00D19500: the six
// vtables; each non-null effect of the array {+330h data, +334h count} is stopped (00867B10),
// its byte +9h set, released and its slot cleared (twice, as the image); the entity leaves the
// list headed by 00F89AD8 (+340h next, +344h previous); the array is walked again releasing any
// remaining slot, then freed (00BF6989); +324h and +2C0h are freed when set (the +1C4h vtable
// reset to 00CE89E8 between them); 00875490 on +170h and 00925780 finish.
void tickable_game_entity_destruct_0092a040(Seg60Host& host, Raw entity)
{
    entity.at<std::uint32_t>(0x0) = kTickableEntityVtable;
    for (const auto& pair : kTickableEntityVtables) {
        entity.at<std::uint32_t>(pair[0]) = pair[1];
    }
    for (std::uint32_t i = 0; i < entity.at<std::uint32_t>(0x334); ++i) {
        auto** slots = static_cast<void**>(entity.ptr(0x330));
        if (slots[i] == nullptr) {
            continue;
        }
        Raw effect{static_cast<std::uint8_t*>(slots[i])};
        host.point_effect_stop_children_00867b10(effect.p);
        effect.at<std::uint8_t>(0x9) = 1;
        slots = static_cast<void**>(entity.ptr(0x330));
        if (slots[i] != nullptr) {
            release_refcounted(host, slots[i]);
            slots[i] = nullptr;
        }
        slots[i] = nullptr;
    }
    void*& head = host.global_pointer(0x00F89AD8);
    if (head == entity.p) {
        head = entity.ptr(0x340);
    }
    if (entity.ptr(0x340) != nullptr) {
        Raw{static_cast<std::uint8_t*>(entity.ptr(0x340))}.ptr(0x344) = entity.ptr(0x344);
    }
    if (entity.ptr(0x344) != nullptr) {
        Raw{static_cast<std::uint8_t*>(entity.ptr(0x344))}.ptr(0x340) = entity.ptr(0x340);
    }
    if (entity.ptr(0x330) != nullptr) {
        for (std::uint32_t i = 0; i < entity.at<std::uint32_t>(0x334); ++i) {
            void*& slot = static_cast<void**>(entity.ptr(0x330))[i];
            if (slot != nullptr) {
                release_refcounted(host, slot);
                slot = nullptr;
            }
        }
        host.free_00bf6989(entity.ptr(0x330));
    }
    if (entity.ptr(0x324) != nullptr) {
        host.free_00bf6989(entity.ptr(0x324));
    }
    entity.at<std::uint32_t>(0x1C4) = kCollisionBaseVtable1C4;
    if (entity.ptr(0x2C0) != nullptr) {
        host.free_00bf6989(entity.ptr(0x2C0));
    }
    host.unit_tick_element_destruct_00875490(entity.p + 0x170);
    host.entity_destruct_base_00925780(entity.p);
}

// 0092CE70, ABI: __fastcall(ECX = unit) RET -> EAX. True when +6B8h is not negative and
// 00779AD0's age is below 3.0 (FCOMIP/JBE: NaN gives false).
bool unit_contact_is_recent_0092ce70(Seg60Host& host, Raw unit)
{
    if (unit.at<int>(0x6B8) < 0) {
        return false;
    }
    const float age = host.unit_contact_age_seconds_00779ad0(unit.p);
    return kRecentContactSeconds > age;
}

// 0092CFD0, ABI: __fastcall(ECX = proxy) RET. The body +2Ch is removed through [game+18h]
// (00C34F70) when set; +2Ch is cleared either way.
void motion_proxy_release_body_0092cfd0(Seg60Host& host, Raw proxy)
{
    if (proxy.ptr(0x2C) != nullptr) {
        host.physics_remove_body_00c34f70(host.game_e188a8().ptr(0x18), proxy.ptr(0x2C));
    }
    proxy.ptr(0x2C) = nullptr;
}

// 0092D620, ABI: __thiscall(ECX = proxy, matrix) RET 4. A 30h stack transform from the matrix
// (00C336C0) is applied to the body +2Ch (00C56CF0); no null check, as the image.
void motion_proxy_sync_transform_0092d620(Seg60Host& host, Raw proxy, const void* matrix)
{
    std::uint8_t transform[0x30];
    host.physics_transform_from_matrix_00c336c0(transform, matrix);
    host.physics_body_set_transform_00c56cf0(proxy.ptr(0x2C), transform);
}

// 0092D650, ABI: __thiscall(ECX = proxy, const float* xz) RET 4. The body's current 30h
// transform (00C32000) copied to the stack, its +24h / +2Ch replaced by xz[0] / xz[1], then
// applied (00C56CF0).
void motion_proxy_set_xz_0092d650(Seg60Host& host, Raw proxy, const float* xz)
{
    std::uint8_t transform[0x30];
    std::memcpy(transform, host.physics_body_transform_00c32000(proxy.ptr(0x2C)), sizeof transform);
    Raw t{transform};
    t.at<float>(0x24) = xz[0];
    t.at<float>(0x2C) = xz[1];
    host.physics_body_set_transform_00c56cf0(proxy.ptr(0x2C), transform);
}

// 0092D6A0, ABI: __thiscall(ECX = proxy, float* out) RET 4 -> EAX = out. The body's linear
// velocity (00C31F40 into a stack vector) copied out as floats.
float* motion_proxy_linear_velocity_0092d6a0(Seg60Host& host, Raw proxy, float* out)
{
    float local[3];
    const float* v = host.physics_body_linear_velocity_00c31f40(proxy.ptr(0x2C), local);
    out[0] = v[0];
    out[1] = v[1];
    out[2] = v[2];
    return out;
}

// 0092D700, ABI: __thiscall(ECX = proxy, float* out) RET 4 -> EAX = out. As 0092D6A0 with the
// angular velocity (00C31F20).
float* motion_proxy_angular_velocity_0092d700(Seg60Host& host, Raw proxy, float* out)
{
    float local[3];
    const float* v = host.physics_body_angular_velocity_00c31f20(proxy.ptr(0x2C), local);
    out[0] = v[0];
    out[1] = v[1];
    out[2] = v[2];
    return out;
}

// 0092D000, ABI: __thiscall(ECX = entity, const float* step, const float* origin, float* out)
// RET 0Ch. No caller found. The world is [[entity+1Ch]+30h]. out = origin + step; while
// 00903860 finds ground under out and out.y is not above it, prev = out and out += step. Then
// five halvings: mid = float(float(out + prev) * 0.5); the ground is queried at OUT (not at mid,
// as the image does); no ground or mid.y above it moves out to mid, otherwise prev to mid.
// Finally out = prev.
void ground_ray_march_0092d000(Seg60Host& host, Raw entity, const float* step, const float* origin, float* out)
{
    const auto world = [&entity]() {
        return Raw{static_cast<std::uint8_t*>(entity.ptr(0x1C))}.ptr(0x30);
    };
    float prev[3] = {origin[0], origin[1], origin[2]};
    out[0] = static_cast<float>(static_cast<double>(step[0]) + origin[0]);
    out[1] = static_cast<float>(static_cast<double>(step[1]) + origin[1]);
    out[2] = static_cast<float>(static_cast<double>(origin[2]) + step[2]);
    float height = origin[0];  // the slot is the routine's origin argument, overwritten at 0092D018
    bool ground = host.world_ground_height_at_00903860(world(), out, &height);
    while (ground) {
        const float y = out[1];
        if (y > height) {
            break;
        }
        prev[0] = out[0];
        prev[1] = y;
        prev[2] = out[2];
        out[0] = static_cast<float>(static_cast<double>(step[0]) + out[0]);
        out[1] = static_cast<float>(static_cast<double>(y) + step[1]);
        out[2] = static_cast<float>(static_cast<double>(prev[2]) + step[2]);
        ground = host.world_ground_height_at_00903860(world(), out, &height);
    }
    for (int i = 0; i < 5; ++i) {
        float mid[3];
        for (int k = 0; k < 3; ++k) {
            const auto sum = static_cast<float>(static_cast<double>(out[k]) + prev[k]);
            mid[k] = static_cast<float>(static_cast<double>(sum) * kRepairZoneHalf);
        }
        ground = host.world_ground_height_at_00903860(world(), out, &height);
        if (!ground || mid[1] > height) {
            out[0] = mid[0];
            out[1] = mid[1];
            out[2] = mid[2];
        } else {
            prev[0] = mid[0];
            prev[1] = mid[1];
            prev[2] = mid[2];
        }
    }
    out[0] = prev[0];
    out[1] = prev[1];
    out[2] = prev[2];
}

// 0092A6D0, ABI: __fastcall(ECX = entity) RET. One in ten calls (00BD2FC0 stream 1, remainder
// 0): a random point (u, 0, v) inside the body's (+354h) bounds (00C31F90, u and v drawn by
// 00BD2F10 stream 1 between the two returned corners) is moved by the body transform (00C32000,
// x = ((u*m0 + 0*m3) + v*m6) + m9 and likewise for y and z, each stored as float). When it lies
// below zero height, a point effect of the settings' definition (+72Ch, its reference count
// raised for the by-value handle) is created in group [game+19ECh] at (x, 0, z) with an identity
// basis (00868420), its returned handle is released, and the effect's byte +9h is set.
void hull_surface_spray_0092a6d0(Seg60Host& host, Raw entity)
{
    if (host.random_next_u32_00bd2fc0(1) % 10 != 0) {
        return;
    }
    Raw settings = host.game_settings_00424c40();
    float first[3];
    float second[3];
    host.physics_body_bounds_00c31f90(entity.ptr(0x354), first, second);
    const float u = host.random_uniform_float_range_00bd2f10(1, first[0], second[0]);
    const float v = host.random_uniform_float_range_00bd2f10(1, first[2], second[2]);
    const auto* m = static_cast<const float*>(host.physics_body_transform_00c32000(entity.ptr(0x354)));
    const double ud = u;
    const double vd = v;
    const auto x = static_cast<float>(((ud * m[0] + 0.0 * m[3]) + vd * m[6]) + m[9]);
    const auto y = static_cast<float>(((m[1] * ud + m[4] * 0.0) + m[7] * vd) + m[10]);
    const auto z = static_cast<float>(((ud * m[2] + 0.0 * m[5]) + vd * m[8]) + m[11]);
    if (!(0.0f > y)) {
        return;
    }
    const float matrix[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                              0.0f, 0.0f, 1.0f, 0.0f, x,    0.0f, z,    1.0f};
    void* definition = settings.ptr(0x72C);
    if (definition != nullptr) {
        host.interlocked_increment(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(definition) + 4));
    }
    void* handle = nullptr;
    void** result = host.point_effect_create_from_matrix_00868420(&handle, host.game_e188a8().at<std::uint32_t>(0x19EC),
                                                                  definition, matrix, 0, 0);
    auto* effect = static_cast<std::uint8_t*>(*result);
    if (handle != nullptr) {
        release_refcounted(host, handle);
    }
    if (effect != nullptr) {
        effect[9] = 1;
    }
}

// 0092A8C0, ABI: __fastcall(ECX = entity) RET. One in a hundred calls (00BD2FC0 stream 1): w =
// float(second.x - first.x) of the body bounds (00C31F90); r = 00BD2F10(1, -1.0, 1.0); the side
// is -1 below zero, 1 above 0.0 (COMISS), else 0 (NaN gives 0); u = float(side * w * 0.5);
// v = 00BD2F10(1, first.z, second.z). (u, 0, v) through the body transform gives x and z (the
// 0092A6D0 schedule; no height is computed or tested), and a point effect of the settings'
// definition +730h is created at (x, 0, z) in group [game+19ECh] with an identity basis; the
// handle is released and the effect's byte +9h set.
void hull_side_spray_0092a8c0(Seg60Host& host, Raw entity)
{
    if (host.random_next_u32_00bd2fc0(1) % 100 != 0) {
        return;
    }
    Raw settings = host.game_settings_00424c40();
    float first[3];
    float second[3];
    host.physics_body_bounds_00c31f90(entity.ptr(0x354), first, second);
    const auto width = static_cast<float>(static_cast<double>(second[0]) - first[0]);
    const float r = host.random_uniform_float_range_00bd2f10(1, kSprayRandomLow, 1.0f);
    int side = 0;
    if (0.0f > r) {
        side = -1;
    } else if (r > kDetectionDefaultValue) {
        side = 1;
    }
    const auto u = static_cast<float>(static_cast<double>(side) * width * kRepairZoneHalf);
    const float v = host.random_uniform_float_range_00bd2f10(1, first[2], second[2]);
    const auto* m = static_cast<const float*>(host.physics_body_transform_00c32000(entity.ptr(0x354)));
    const double ud = u;
    const double vd = v;
    const auto x = static_cast<float>(((ud * m[0] + 0.0 * m[3]) + vd * m[6]) + m[9]);
    const auto z = static_cast<float>(((ud * m[2] + 0.0 * m[5]) + vd * m[8]) + m[11]);
    const float matrix[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                              0.0f, 0.0f, 1.0f, 0.0f, x,    0.0f, z,    1.0f};
    void* definition = settings.ptr(0x730);
    if (definition != nullptr) {
        host.interlocked_increment(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(definition) + 4));
    }
    void* handle = nullptr;
    void** result = host.point_effect_create_from_matrix_00868420(&handle, host.game_e188a8().at<std::uint32_t>(0x19EC),
                                                                  definition, matrix, 0, 0);
    auto* effect = static_cast<std::uint8_t*>(*result);
    if (handle != nullptr) {
        release_refcounted(host, handle);
    }
    if (effect != nullptr) {
        effect[9] = 1;
    }
}

// 0092A240, ABI: __thiscall(ECX = entity, parent node) RET 4. +350h = parent. With a definition
// ([+348h]+55Ch) and [game+19FCh] set: w = float(class+A0h * 0.2), height = float(class+A8h *
// 0.6), n = float(w / 10) clamped to [1, 2] (FLD1/FCOMIP then COMISS; NaN kept), count = n
// truncated (FISTP under round-toward-zero); count == 0 returns. Per effect: offset =
// 00BD2F10(1, -w, w) (-w as -0.0 - w); the parent's world position (+120h, refreshed through
// 00B6DB70 unless byte +5Ch bit 1 is set) plus (0.0, height, offset) is appended to the point
// array {+324h data, +328h count, +32Ch capacity} (growth capacity * 2 + 2 with a per-slot float
// copy; old data freed when set); an effect of the definition (reference raised for the
// by-value handle) is created under the parent with the local matrix identity + (0, height,
// offset) (008687C0(parent, handle, matrix, 1, 0, 0)); the handle array {+330h data, +334h
// count, +338h capacity} grows the same way, moving each handle with a reference raise and a
// release of the old slot; the new handle is stored with a reference raise and the created
// handle released.
void tickable_entity_attach_fires_0092a240(Seg60Host& host, Raw entity, void* parent_node)
{
    entity.ptr(0x350) = parent_node;
    Raw cls{static_cast<std::uint8_t*>(entity.ptr(0x348))};
    if (cls.ptr(0x55C) == nullptr || host.game_e188a8().at<std::uint32_t>(0x19FC) == 0) {
        return;
    }
    const auto width = static_cast<float>(cls.at<float>(0xA0) * kFireWidthScale);
    const auto height = static_cast<float>(cls.at<float>(0xA8) * kFireHeightScale);
    auto n = static_cast<float>(width / kFireCountDivisor);
    if (1.0f > n) {
        n = 1.0f;
    } else if (n > kFireCountMax) {
        n = kFireCountMax;
    }
    // FISTP of a NaN stores the integer indefinite 8000000000000000h, whose low dword is 0.
    const auto count = std::isnan(n) ? 0u : static_cast<std::uint32_t>(static_cast<std::int64_t>(n));
    if (count == 0) {
        return;
    }
    const float low = -0.0f - width;
    for (std::uint32_t remaining = count; remaining != 0; --remaining) {
        const float offset = host.random_uniform_float_range_00bd2f10(1, low, width);
        Raw node{static_cast<std::uint8_t*>(entity.ptr(0x350))};
        if ((node.at<std::uint8_t>(0x5C) & 2) == 0) {
            host.transform_refresh_world_matrix_00b6db70(node.p);
        }
        const float px = node.at<float>(0x120);
        const float py = node.at<float>(0x124);
        const float pz = node.at<float>(0x128);
        const float point[3] = {static_cast<float>(px + 0.0), static_cast<float>(static_cast<double>(py) + height),
                                static_cast<float>(static_cast<double>(pz) + offset)};
        // Point array growth (0092A3B6..0092A423).
        if (entity.at<std::uint32_t>(0x328) == entity.at<std::uint32_t>(0x32C)) {
            const std::uint32_t capacity = entity.at<std::uint32_t>(0x32C) * 2 + 2;
            entity.at<std::uint32_t>(0x32C) = capacity;
            auto* block = static_cast<float*>(host.operator_new_00bf55be(capacity * 12));
            for (std::uint32_t i = 0; i < entity.at<std::uint32_t>(0x328); ++i) {
                if (block + i * 3 != nullptr) {
                    const auto* old = static_cast<const float*>(entity.ptr(0x324)) + i * 3;
                    block[i * 3 + 0] = old[0];
                    block[i * 3 + 1] = old[1];
                    block[i * 3 + 2] = old[2];
                }
            }
            if (entity.ptr(0x324) != nullptr) {
                host.free_00bf6989(entity.ptr(0x324));
            }
            entity.ptr(0x324) = block;
        }
        float* slot = static_cast<float*>(entity.ptr(0x324)) + entity.at<std::uint32_t>(0x328) * 3;
        if (slot != nullptr) {
            slot[0] = point[0];
            slot[1] = point[1];
            slot[2] = point[2];
        }
        entity.at<std::uint32_t>(0x328) += 1;
        const float matrix[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, height, offset, 1.0f};
        void* definition = cls.ptr(0x55C);
        if (definition != nullptr) {
            host.interlocked_increment(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(definition) + 4));
        }
        void* created = nullptr;
        void** result = host.point_effect_create_with_parent_matrix_008687c0(&created, node.p, definition, matrix, 1, 0, 0);
        // Handle array growth (0092A53E..0092A610).
        if (entity.at<std::uint32_t>(0x334) == entity.at<std::uint32_t>(0x338)) {
            const std::uint32_t capacity = entity.at<std::uint32_t>(0x338) * 2 + 2;
            entity.at<std::uint32_t>(0x338) = capacity;
            auto** block = static_cast<void**>(host.operator_new_00bf55be(capacity * 4));
            for (std::uint32_t i = 0; i < entity.at<std::uint32_t>(0x334); ++i) {
                auto** old = static_cast<void**>(entity.ptr(0x330));
                if (block + i != nullptr) {
                    block[i] = nullptr;
                    if (old[i] != nullptr) {
                        block[i] = old[i];
                        host.interlocked_increment(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(old[i]) + 4));
                    }
                }
                if (old[i] != nullptr) {
                    release_refcounted(host, old[i]);
                    old[i] = nullptr;
                }
            }
            if (entity.ptr(0x330) != nullptr) {
                host.free_00bf6989(entity.ptr(0x330));
            }
            entity.ptr(0x330) = block;
        }
        void** handle_slot = static_cast<void**>(entity.ptr(0x330)) + entity.at<std::uint32_t>(0x334);
        if (handle_slot != nullptr) {
            *handle_slot = nullptr;
            if (*result != nullptr) {
                *handle_slot = *result;
                host.interlocked_increment(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(*result) + 4));
            }
        }
        entity.at<std::uint32_t>(0x334) += 1;
        if (created != nullptr) {
            release_refcounted(host, created);
            created = nullptr;
        }
    }
}

// 0092DFE0, ABI: __cdecl(first, last, destination) -> EAX. Copies 48h-byte records forward
// (REP MOVSD of 12h dwords) and returns destination + count * 48h (count by signed division).
std::uint8_t* copy_48h_records_0092dfe0(const std::uint8_t* first, const std::uint8_t* last, std::uint8_t* destination) noexcept
{
    const auto count = static_cast<std::ptrdiff_t>(last - first) / 0x48;
    std::uint8_t* out = destination;
    for (const std::uint8_t* it = first; it != last; it += 0x48, out += 0x48) {
        std::memcpy(out, it, 0x48);
    }
    return destination + count * 0x48;
}

}  // namespace bsp::cc13_seg49

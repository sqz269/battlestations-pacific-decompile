#include "bsp/terrain_particles_cc13.hpp"

#include <intrin.h>

#include <algorithm>
#include <cmath>
#include <new>
#include <stdexcept>

// Segments 80-82 (packet cc13-terrain1). Evidence and ABIs: docs/CC13_TERRAIN_1.md and the
// declarations in include/bsp/terrain_particles_cc13.hpp.

namespace bsp::cc13_terrain {
namespace {

template <typename T>
T rd(const void* base, std::uint32_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + offset, sizeof value);
    return value;
}

template <typename T>
void wr(void* base, std::uint32_t offset, T value) {
    std::memcpy(static_cast<std::uint8_t*>(base) + offset, &value, sizeof value);
}

std::uint8_t* at(void* base, std::uint32_t offset) {
    return static_cast<std::uint8_t*>(base) + offset;
}

float f(double v) noexcept { return static_cast<float>(v); }

bool contains(const std::uint32_t* list, std::size_t n, std::uint32_t address) noexcept {
    for (std::size_t i = 0; i < n; ++i)
        if (list[i] == address) return true;
    return false;
}

template <std::size_t N>
bool in(const std::uint32_t (&list)[N], std::uint32_t address) noexcept {
    return contains(list, N, address);
}

// Pointer-array shapes (data/count/capacity at +00/+04/+08).
constexpr std::uint32_t kReserve[] = {0x00AD62A0, 0x00AD65C0, 0x00AD6620, 0x00AFEB60, 0x00ADB050};
constexpr std::uint32_t kResize[] = {0x00AD64D0, 0x00AD6DB0, 0x00AD6E00, 0x00AFEC20, 0x00ADBE60};
constexpr std::uint32_t kDestroy[] = {0x00AD7500, 0x00AD76E0, 0x00AD7700, 0x00AFEE50, 0x00ADD130};
constexpr std::uint32_t kPushBack[] = {0x00AD6A90, 0x00AF6820, 0x00AF88F0, 0x00ADBDF0};

// vector<void*> push_back through the 00AD8A00/00AD8A90 InsertAt path: _Insert_n(last, 1).
void vector4_push_back(NativeVector4& v, std::uint32_t value, TerrainHost& host) {
    if (v.first == nullptr || v.end - v.first <= v.last - v.first)
        vector4_insert_n(v, v.last, 1, value, host);
    else
        *v.last++ = value;
}

NativeVector4& vec_at(void* base, std::uint32_t offset) {
    return *reinterpret_cast<NativeVector4*>(at(base, offset));
}

std::uint32_t vector4_size(const NativeVector4& v) noexcept {
    return v.first == nullptr ? 0u : static_cast<std::uint32_t>(v.last - v.first);
}

// The landscape-name compare of 00AD7B90: equal lengths, then __stricmp; empty == empty.
bool names_equal(const void* a, const void* b, TerrainHost& host) {
    const std::uint32_t la = rd<std::uint32_t>(a, 0);
    const std::uint32_t lb = rd<std::uint32_t>(b, 0);
    if (la != lb) return false;
    if (la == 0) return true;
    return host.stricmp_00bf7fbf(rd<const char*>(a, 4), rd<const char*>(b, 4)) == 0;
}

}  // namespace

// ---------------------------------------------------------------------------
// Shared primitives.

void release_native_string(void* string, TerrainHost& host) {
    void* chars = rd<void*>(string, 4);
    if (chars != nullptr) host.return_string_block_00bd1510(chars, rd<std::uint32_t>(string, 0) + 1);
}

void release_ref_slot(void** slot, TerrainHost& host) {
    void* object = *slot;
    if (object == nullptr) return;
    if (_InterlockedDecrement(reinterpret_cast<volatile long*>(at(object, 4))) == 0)
        host.call_virtual(object, 0);
    *slot = nullptr;
}

void** ref_ptr_release_00ad6680(void** slot, TerrainHost& host) {
    release_ref_slot(slot, host);
    return slot;
}

void** ref_ptr_add_ref_00b17d50(void** slot) {
    if (*slot != nullptr) _InterlockedIncrement(reinterpret_cast<volatile long*>(at(*slot, 4)));
    return slot;
}

const FieldSetter kFieldSetters[5] = {
    {0x00AD56C0, 0x3C, 4}, {0x00AD56F0, 0x08, 4}, {0x00AD5710, 0x0C, 4}, {0x00AD5730, 0x11, 1},
    {0x00AF0450, 0x2C, 4},
};

bool apply_field_setter(std::uint32_t address, void* object, std::uint32_t value) {
    for (const FieldSetter& s : kFieldSetters) {
        if (s.address != address) continue;
        std::memcpy(at(object, s.offset), &value, s.width);
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// STL instantiations.

void allocator_deallocate(void* p, TerrainHost& host) { host.free(p); }

void* allocator_allocate(std::uint32_t count, std::uint32_t element_size, TerrainHost& host) {
    if (count != 0 && 0xFFFFFFFFu / count < element_size) throw std::bad_alloc();
    return host.operator_new(count * element_size);
}

void fill_dwords(std::uint32_t* first, std::uint32_t* last, const std::uint32_t* value) {
    for (; first != last; ++first) *first = *value;
}

std::uint32_t* fill_n_dwords(std::uint32_t* first, std::uint32_t n, const std::uint32_t* value) {
    for (std::uint32_t i = 0; i < n; ++i) first[i] = *value;
    return first + n;
}

std::uint32_t* copy_dwords(const std::uint32_t* first, const std::uint32_t* last, std::uint32_t* dest) {
    const std::ptrdiff_t n = last - first;
    if (n != 0) std::memmove(dest, first, static_cast<std::size_t>(n) * 4);
    return dest + n;
}

std::uint32_t* copy_backward_dwords(const std::uint32_t* first, const std::uint32_t* last,
    std::uint32_t* dest_end) {
    const std::ptrdiff_t n = last - first;
    std::uint32_t* dest = dest_end - n;
    if (n > 0) std::memmove(dest, first, static_cast<std::size_t>(n) * 4);
    return dest;
}

void vector4_insert_n(NativeVector4& v, std::uint32_t* where, std::uint32_t count, std::uint32_t value,
    TerrainHost& host) {
    std::uint32_t capacity = v.first == nullptr ? 0u : static_cast<std::uint32_t>(v.end - v.first);
    if (count == 0) return;
    const std::uint32_t size = vector4_size(v);
    if (0x3FFFFFFFu - size < count) throw std::length_error("vector<T> too long");
    if (capacity < size + count) {
        capacity = 0x3FFFFFFFu - capacity / 2 < capacity ? 0u : capacity + capacity / 2;
        if (capacity < size + count) capacity = size + count;
        auto* fresh = static_cast<std::uint32_t*>(allocator_allocate(capacity, 4, host));
        std::uint32_t* p = copy_dwords(v.first, where, fresh);
        p = fill_n_dwords(p, count, &value);
        copy_dwords(where, v.last, p);
        if (v.first != nullptr) host.free(v.first);
        v.first = fresh;
        v.end = fresh + capacity;
        v.last = fresh + size + count;
        return;
    }
    const std::uint32_t tail = static_cast<std::uint32_t>(v.last - where);
    if (tail < count) {
        // Room at the end: move the tail up by count, fill the gap and the vacated tail.
        copy_dwords(where, v.last, where + count);
        fill_n_dwords(v.last, count - tail, &value);
        v.last += count;
        fill_dwords(where, v.last - count, &value);
    } else {
        std::uint32_t* old_last = v.last;
        v.last = copy_dwords(old_last - count, old_last, old_last);
        copy_backward_dwords(where, old_last - count, old_last);
        fill_dwords(where, where + count, &value);
    }
}

void vector4_tidy(NativeVector4& v, TerrainHost& host) {
    if (v.first != nullptr) host.free(v.first);
    v.first = v.last = v.end = nullptr;
}

// ---------------------------------------------------------------------------
// Pointer arrays.

bool is_pointer_array_routine(std::uint32_t address) noexcept {
    return in(kReserve, address) || in(kResize, address) || in(kDestroy, address) || in(kPushBack, address);
}

void pointer_array_reserve(std::uint32_t address, PtrArray& a, std::int32_t n) {
    if (!in(kReserve, address)) throw std::invalid_argument("not a reserve routine");
    bsp::cc13_scene2::array_reserve_min_one(a, n);
}

void pointer_array_resize(std::uint32_t address, PtrArray& a, std::int32_t n) {
    if (!in(kResize, address)) throw std::invalid_argument("not a resize routine");
    bsp::cc13_scene2::array_resize_zero_fill<void*>(a, n,
        [](PtrArray& x, std::int32_t m) { bsp::cc13_scene2::array_reserve_min_one(x, m); });
}

void pointer_array_destroy(std::uint32_t address, PtrArray& a) {
    if (!in(kDestroy, address)) throw std::invalid_argument("not a destroy routine");
    pointer_array_resize(0x00AD64D0, a, 0);
    ::operator delete(a.data);
}

void pointer_array_push_back(std::uint32_t address, PtrArray& a, void* value, const PtrReserve& reserve) {
    if (!in(kPushBack, address)) throw std::invalid_argument("not a push-back routine");
    if (a.count == a.capacity) {
        std::int32_t n = a.capacity * 2;
        if (n < 2) n = 1;
        reserve(a, n);
    }
    a.data[a.count++] = value;
}

void dword_array_reserve_00b0a440(NativeArray<std::uint32_t>& a, std::uint32_t n, TerrainHost& host) {
    if (static_cast<std::uint32_t>(a.capacity) >= n) return;
    a.capacity = static_cast<std::int32_t>(n);
    auto* fresh = static_cast<std::uint32_t*>(allocator_allocate(n, 4, host));
    if (a.data != nullptr) {
        for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(a.count); ++i) fresh[i] = a.data[i];
        host.free(a.data);
    }
    a.data = fresh;
}

// ---------------------------------------------------------------------------
// Singleton bases.

const SingletonBase kSingletonBases[4] = {
    {0x00AD6810, 0x00AD68B0, 0x00F8C264, 0x00D5D21C, true},
    {0x00AD6950, 0x00AD69F0, 0x00F8C210, 0x00D5D220, true},
    {0x00AD9D00, 0x00AD9DA0, 0x00F8C218, 0x00D5D284, true},
    {0x00AF1140, 0x00AF11E0, 0x00F8C280, 0x00D5D840, false},
};

const SingletonBase* find_singleton_base(std::uint32_t address) noexcept {
    for (const SingletonBase& b : kSingletonBases)
        if (b.construct_address == address || b.destroy_address == address) return &b;
    if (address == 0x00AD9E90) return &kSingletonBases[2];  // second copy of 00AD9DA0
    return nullptr;
}

void construct_singleton_base(void* object, const SingletonBase& base, TerrainHost& host) {
    wr<std::uint32_t>(object, 0, base.base_vtable);
    host.lifetime_lock(base.bump_depth);
    host.write_global(base.instance_global, object);
    host.lifetime_register_00bd0c30(host.read_global(base.instance_global));
    host.lifetime_unlock(base.bump_depth);
}

void destroy_singleton_base(void* object, const SingletonBase& base, TerrainHost& host) {
    wr<std::uint32_t>(object, 0, base.base_vtable);
    host.lifetime_lock(base.bump_depth);
    host.lifetime_unregister_00bcfca0(host.read_global(base.instance_global));
    host.write_global(base.instance_global, nullptr);
    host.lifetime_unlock(base.bump_depth);
    wr<std::uint32_t>(object, 0, kSingletonBaseVTable);
}

// ---------------------------------------------------------------------------
// Segment 80: foliage.

void foliage_type_record_destroy_00ad77e0(void* record, TerrainHost& host) {
    release_native_string(at(record, 0x1C), host);
    release_native_string(at(record, 0x14), host);
    release_native_string(at(record, 0x0C), host);
}

void foliage_type_record_destroy_range_00ad8c30(std::uint8_t* first, std::uint8_t* last, TerrainHost& host) {
    for (; first != last; first += 0x30) foliage_type_record_destroy_00ad77e0(first, host);
}

void foliage_type_record_vector_tidy_00ad9520(void* vector, TerrainHost& host) {
    auto* first = rd<std::uint8_t*>(vector, 4);
    if (first != nullptr) {
        foliage_type_record_destroy_range_00ad8c30(first, rd<std::uint8_t*>(vector, 8), host);
        host.free(first);
    }
    wr<std::uint32_t>(vector, 4, 0);
    wr<std::uint32_t>(vector, 8, 0);
    wr<std::uint32_t>(vector, 0xC, 0);
}

void foliage_type_group_destroy_00ad97d0(void* group, TerrainHost& host) {
    foliage_type_record_vector_tidy_00ad9520(at(group, 8), host);
    release_native_string(group, host);
}

void foliage_type_group_destroy_range_00ad9880(std::uint8_t* first, std::uint8_t* last, TerrainHost& host) {
    for (; first != last; first += 0x18) foliage_type_group_destroy_00ad97d0(first, host);
}

void foliage_type_group_vector_tidy_00ad9970(void* vector, TerrainHost& host) {
    auto* first = rd<std::uint8_t*>(vector, 4);
    if (first != nullptr) {
        foliage_type_group_destroy_range_00ad9880(first, rd<std::uint8_t*>(vector, 8), host);
        host.free(first);
    }
    wr<std::uint32_t>(vector, 4, 0);
    wr<std::uint32_t>(vector, 8, 0);
    wr<std::uint32_t>(vector, 0xC, 0);
}

void* foliage_type_table_construct_00ad99f0(void* table, TerrainHost& host) {
    construct_singleton_base(table, kSingletonBases[0], host);
    wr<std::uint32_t>(table, 0, 0x00D5D274);
    wr<std::uint32_t>(table, 8, 0);
    wr<std::uint32_t>(table, 0xC, 0);
    wr<std::uint32_t>(table, 0x10, 0);
    return table;
}

void foliage_type_table_destroy_00ad9a40(void* table, TerrainHost& host) {
    foliage_type_group_vector_tidy_00ad9970(at(table, 4), host);
    destroy_singleton_base(table, kSingletonBases[0], host);
}

void foliage_ref_object_destroy_00ad7880(void* object, TerrainHost& host) {
    release_ref_slot(reinterpret_cast<void**>(at(object, 4)), host);
    release_ref_slot(reinterpret_cast<void**>(at(object, 0)), host);
    release_ref_slot(reinterpret_cast<void**>(at(object, 0x40)), host);
    release_native_string(at(object, 0x38), host);
    release_native_string(at(object, 0x30), host);
    release_native_string(at(object, 0x28), host);
}

void foliage_ref_object_delete_00ad7b50(void** slot, TerrainHost& host) {
    void* object = *slot;
    if (object == nullptr) return;
    foliage_ref_object_destroy_00ad7880(object, host);
    host.free(object);
    *slot = nullptr;
}

void foliage_system_remove_landscape_00ad7b90(void* system, void* landscape, TerrainHost& host) {
    if (landscape != nullptr) {
        // Bush groups (+54h vector): group +00 is the landscape; erase on a name match when
        // the group's +14h is set.
        NativeVector4& bushes = vec_at(system, 0x54);
        for (std::uint32_t i = 0; bushes.first != nullptr && i < vector4_size(bushes); ++i) {
            void* group = reinterpret_cast<void*>(bushes.first[i]);
            if (!names_equal(at(rd<void*>(group, 0), 0x20), at(landscape, 0x20), host)) continue;
            if (rd<std::uint32_t>(group, 0x14) == 0) continue;
            host.bush_group_detach_00ae1e00(group, at(system, 0x30));
            if (group != nullptr) {
                host.bush_group_destruct_00ae1c80(group);
                host.free(group);
            }
            std::memmove(&bushes.first[i], &bushes.first[i + 1], (bushes.last - &bushes.first[i + 1]) * 4);
            --bushes.last;
            --i;
        }
        // Tree groups (+64h vector): the same with the group's +08h and the +24h list.
        NativeVector4& trees = vec_at(system, 0x64);
        for (std::uint32_t i = 0; trees.first != nullptr && i < vector4_size(trees); ++i) {
            void* group = reinterpret_cast<void*>(trees.first[i]);
            if (!names_equal(at(rd<void*>(group, 0), 0x20), at(landscape, 0x20), host)) continue;
            if (rd<std::uint32_t>(group, 0x08) == 0) continue;
            host.tree_group_detach_00ae4f30(group, at(system, 0x24));
            if (group != nullptr) {
                host.tree_group_destruct_00ae4c20(group);
                host.free(group);
            }
            std::memmove(&trees.first[i], &trees.first[i + 1], (trees.last - &trees.first[i + 1]) * 4);
            --trees.last;
            --i;
        }
        // Ref objects (+48h pointer array) whose +44h is zero are deleted and removed.
        auto& objects = *reinterpret_cast<PtrArray*>(at(system, 0x48));
        for (std::int32_t i = 0; i < objects.count; ++i) {
            void* object = objects.data[i];
            if (rd<std::uint32_t>(object, 0x44) != 0) continue;
            foliage_ref_object_delete_00ad7b50(&objects.data[i], host);
            for (std::int32_t k = i; k < objects.count - 1; ++k) objects.data[k] = objects.data[k + 1];
            --objects.count;
            --i;
        }
    }
    // Loaded atlases: unload the atlas named by +8Ch / +94h, clear the texture name +7Ch /
    // +84h and copy the (now empty) name into the atlas name.
    for (std::uint32_t k = 0; k < 2; ++k) {
        const std::uint32_t flag = 0x9C + 4 * k, atlas = 0x8C + 8 * k, name = 0x7C + 8 * k;
        if (rd<std::uint32_t>(system, flag) == 0) continue;
        wr<std::uint32_t>(system, flag, 0);
        host.texture_atlas_unload_00aefa30(at(system, atlas));
        if (rd<std::uint32_t>(system, name) != 0) {
            release_native_string(at(system, name), host);
            wr<std::uint32_t>(system, name + 4, 0);
            wr<std::uint32_t>(system, name, 0);
        }
        // _memcpy(chars, "" (00CE3A0C), 0) when chars is set: no effect.
        host.native_string_assign_0041dd40(at(system, atlas), rd<const void*>(system, name + 4),
            rd<std::uint32_t>(system, name));
    }
}

void* foliage_system_find_or_create_tree_group_00ad8c60(void* system, void* landscape, std::uint32_t type,
    std::uint32_t extra, TerrainHost& host) {
    NativeVector4& trees = vec_at(system, 0x64);
    for (std::uint32_t i = 0; trees.first != nullptr && i < vector4_size(trees); ++i) {
        void* group = reinterpret_cast<void*>(trees.first[i]);
        if (rd<void*>(group, 0) == landscape && rd<std::uint32_t>(group, 4) == type) return group;
    }
    void* memory = host.operator_new(0x3C);
    void* group = memory == nullptr ? nullptr
        : host.tree_group_instance_construct_00ae5070(memory, landscape, type, rd<std::uint32_t>(system, 0x3C),
            extra);
    vector4_push_back(trees, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(group)), host);
    return group;
}

void* foliage_system_find_or_create_bush_group_00ad8dd0(void* system, void* landscape, void* string_a,
    void* string_b, TerrainHost& host) {
    NativeVector4& bushes = vec_at(system, 0x54);
    void* group = nullptr;
    bool found = false;
    for (std::uint32_t i = 0; bushes.first != nullptr && i < vector4_size(bushes); ++i) {
        group = reinterpret_cast<void*>(bushes.first[i]);
        if (rd<void*>(group, 0) == landscape) { found = true; break; }
    }
    if (!found) {
        void* memory = host.operator_new(0x70);
        group = nullptr;
        if (memory != nullptr) {
            // The two strings are copied (0041DD40 + _memcpy) into by-value arguments that
            // the constructor owns: b then a are built, a is the first string argument.
            group = host.bush_group_type_construct_00ae1e10(memory, landscape, string_a, string_b,
                rd<std::uint32_t>(system, 0x3C));
        }
        vector4_push_back(bushes, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(group)), host);
    }
    release_native_string(string_a, host);
    release_native_string(string_b, host);
    return group;
}

void foliage_system_destroy_00ad9570(void* system, TerrainHost& host) {
    wr<std::uint32_t>(system, 0, 0x00D5D270);
    release_ref_slot(reinterpret_cast<void**>(at(system, 0xA4)), host);
    auto& objects = *reinterpret_cast<PtrArray*>(at(system, 0x48));
    for (std::int32_t i = 0; i < objects.count; ++i) foliage_ref_object_delete_00ad7b50(&objects.data[i], host);
    if (rd<void*>(system, 0x40) != nullptr) {
        host.node_unlink_and_release_00b6d8f0(rd<void*>(system, 0x40));
        wr<std::uint32_t>(system, 0x40, 0);
    }
    if (void* table = host.read_global(0x00F8C264)) host.call_virtual_u32(table, 0, 1);
    release_native_string(at(system, 0x94), host);
    release_native_string(at(system, 0x8C), host);
    release_native_string(at(system, 0x84), host);
    release_native_string(at(system, 0x7C), host);
    vector4_tidy(vec_at(system, 0x64), host);
    vector4_tidy(vec_at(system, 0x54), host);
    pointer_array_destroy(0x00AD7500, objects);
    pointer_array_destroy(0x00AD7700, *reinterpret_cast<PtrArray*>(at(system, 0x30)));
    pointer_array_destroy(0x00AD76E0, *reinterpret_cast<PtrArray*>(at(system, 0x24)));
    if (void* first = rd<void*>(system, 0x18)) {
        host.destroy_native_string_range_00432050(first, rd<void*>(system, 0x1C));
        host.free(first);
    }
    wr<std::uint32_t>(system, 0x18, 0);
    wr<std::uint32_t>(system, 0x1C, 0);
    wr<std::uint32_t>(system, 0x20, 0);
    destroy_singleton_base(system, kSingletonBases[1], host);
}

void* terrain_render_object_create_block_00ad9c40(std::uint32_t a, std::uint32_t b, std::uint32_t c,
    TerrainHost& host) {
    void* memory = host.operator_new(0x38);
    if (memory == nullptr) return nullptr;
    host.terrain_render_block_construct_00adf6b0(memory, a, b, c);
    wr<std::uint32_t>(memory, 0, 0x00D5D278);
    return memory;
}

void* terrain_render_object_construct(std::uint32_t address, void* object, TerrainHost& host) {
    construct_singleton_base(object, kSingletonBases[2], host);
    if (address == 0x00AD9E60) {
        wr<std::uint32_t>(object, 0, 0x00D5D288);
    } else if (address == 0x00AD9F90) {
        for (std::uint32_t off = 4; off <= 0x10; off += 4) wr<std::uint32_t>(object, off, 0);
        wr<std::uint32_t>(object, 0, 0x00D5D2A0);
    } else {
        throw std::invalid_argument("not a terrain render object constructor");
    }
    return object;
}

void terrain_render_object_destroy_00ad9ec0(void* object, TerrainHost& host) {
    wr<std::uint32_t>(object, 0, 0x00D5D2A0);
    for (std::uint32_t off : {0x04u, 0x10u, 0x08u, 0x0Cu}) release_ref_slot(reinterpret_cast<void**>(at(object, off)), host);
    destroy_singleton_base(object, kSingletonBases[2], host);
}

void* terrain_get_shader_name_00ad9fd0(void* out, TerrainHost& host) {
    host.native_string_assign_literal_0041e870(out, "Terrain.mshd");
    return out;
}

namespace {
// 00426060 BSP_NativeString_CopyConstruct into a by-value argument {len, chars}.
struct NativeStr { std::uint32_t len; void* chars; };
NativeStr copy_string(const NativeStr& s, TerrainHost& host) {
    NativeStr out{0, nullptr};
    host.native_string_assign_0041dd40(&out, s.chars, s.len);
    return out;
}
}  // namespace

void foliage_system_load_bushes_00ad9050(void* system, void* landscape, const void* path, TerrainHost& host,
    FoliageLoadHost& load) {
    void* stream = load.vfs_open_0109ceec(path, 2);
    const std::int32_t count = load.stream_read_int(stream);
    if (count != 0) {
        NativeStr a{0, nullptr}, b{0, nullptr};
        load.stream_read_string(stream, &a);
        load.stream_read_string(stream, &b);
        NativeStr init_b = copy_string(b, host), init_a = copy_string(a, host);
        NativeStr group_b = copy_string(b, host), group_a = copy_string(a, host);
        void* group = foliage_system_find_or_create_bush_group_00ad8dd0(system, landscape, &group_a, &group_b, host);
        load.bush_group_initialise_00ae2450(group, stream, count, &init_a, &init_b);
        release_native_string(&b, host);
        release_native_string(&a, host);
    }
    release_ref_slot(&stream, host);
}

void foliage_system_load_trees_00ad91c0(void* system, void* landscape, const void* path, TerrainHost& host,
    FoliageLoadHost& load) {
    void* stream = load.vfs_open_0109ceec(path, 2);
    for (std::int32_t v = load.stream_read_int(stream); v != 0; v = load.stream_read_int(stream)) {
        const std::int32_t x = load.stream_read_int(stream);
        NativeStr model{0, nullptr};
        load.stream_read_string(stream, &model);
        const std::uint32_t type = load.find_tree_group_type_00ad7030(system, &model);
        void* group = foliage_system_find_or_create_tree_group_00ad8c60(system, landscape, type,
            static_cast<std::uint32_t>(x), host);
        load.tree_group_initialise_00ae5870(group, stream, v);
        release_native_string(&model, host);
    }
    release_ref_slot(&stream, host);
}

void* foliage_system_construct_00ad9ac0(void* system, TerrainHost& host, FoliageLoadHost& load) {
    construct_singleton_base(system, kSingletonBases[1], host);
    wr<std::uint32_t>(system, 0, 0x00D5D270);
    for (std::uint32_t off = 0x18; off <= 0x40; off += 4) wr<std::uint32_t>(system, off, 0);
    for (std::uint32_t off : {0x48u, 0x4Cu, 0x50u, 0x58u, 0x5Cu, 0x60u, 0x68u, 0x6Cu, 0x70u})
        wr<std::uint32_t>(system, off, 0);
    for (std::uint32_t off = 0x7C; off <= 0x98; off += 4) wr<std::uint32_t>(system, off, 0);
    wr<std::uint8_t>(system, 5, 0);
    wr<std::uint8_t>(system, 6, 1);
    if (void* table = host.operator_new(0x14)) foliage_type_table_construct_00ad99f0(table, host);
    load.parse_foliage_types_00ae84e0(host.read_global(0x00F8C264));
    wr<std::uint32_t>(system, 0x9C, 0);
    wr<std::uint32_t>(system, 0xA0, 0);
    wr<std::uint32_t>(system, 0xA4, 0);
    wr<std::uint8_t>(system, 4, 0);
    return system;
}

void* tree_construct_00ad5590(void* tree, FoliageLoadHost& load) {
    void* head = load.allocate_tree_head_00ad42c0();
    wr<void*>(tree, 4, head);
    wr<std::uint8_t>(head, 0x15, 1);
    wr<void*>(head, 4, head);
    wr<void*>(head, 0, head);
    wr<void*>(head, 8, head);
    wr<std::uint32_t>(tree, 8, 0);
    return tree;
}

void* font_descriptor_construct_00ad55c0(void* object, const void* name, std::uint32_t a, std::uint32_t b,
    std::uint8_t c, TerrainHost& host, FoliageLoadHost& load) {
    tree_construct_00ad5590(object, load);
    wr<std::uint32_t>(object, 0xC, 0);
    wr<std::uint32_t>(object, 0x10, 0);
    if (at(object, 0xC) != name)
        host.native_string_assign_0041dd40(at(object, 0xC), rd<const void*>(name, 4), rd<std::uint32_t>(name, 0));
    wr<std::uint32_t>(object, 0x18, a);
    wr<std::uint16_t>(object, 0x14, 0);
    wr<std::uint32_t>(object, 0x1C, b);
    for (std::uint32_t off = 0x20; off <= 0x44; off += 4) wr<std::uint32_t>(object, off, 0);
    wr<std::uint8_t>(object, 0x48, c);
    return object;
}

// ---------------------------------------------------------------------------
// Segment 81: height field.

namespace {
void* fresh_node(void* field, HeightFieldHost& host) {
    void* node = rd<void*>(field, 0x2C);
    if ((rd<std::uint8_t>(node, 0x5C) & 2) == 0) host.refresh_world_matrix_00b6db70(node);
    return node;
}

// _ftol2 truncates toward zero.
std::int32_t ftol(double v) { return static_cast<std::int32_t>(v); }
}  // namespace

void height_field_local_grid_to_world_xz_00ada0d0(void* field, float out[2], float u, float v,
    HeightFieldHost& host) {
    const double cell = rd<float>(field, 0x1C);
    const float p[3] = {f(cell * u + rd<float>(field, 0x80)), 0.0f, f(cell * v + rd<float>(field, 0x84))};
    void* node = fresh_node(field, host);
    float world[3];
    host.transform_affine_point_004142e0(p, world, at(node, 0xF0));
    out[0] = world[0];
    out[1] = world[2];
}

void height_field_world_bounds_00ada720(void* field, float out[6], HeightFieldHost& host) {
    void* node = fresh_node(field, host);
    const float x0 = rd<float>(node, 0x120), z0 = rd<float>(node, 0x128);
    const double tile = rd<float>(field, 0x18);
    const std::int32_t columns = rd<std::int32_t>(field, 0x38), rows = rd<std::int32_t>(field, 0x3C);
    node = fresh_node(field, host);
    const float x1 = rd<float>(node, 0x120), z1 = rd<float>(node, 0x128);
    out[0] = f(static_cast<double>(rd<float>(field, 0x80)) + x0);
    out[1] = kNoDataHeight_00D7A240;
    out[2] = f(static_cast<double>(rd<float>(field, 0x84)) + z0);
    out[4] = rd<float>(field, 0x14);
    out[3] = f(x1 + tile * columns);
    out[5] = f(z1 + tile * rows);
}

std::int32_t* height_field_world_to_cell_00ada810(void* field, std::int32_t out[2], float x, float z,
    HeightFieldHost& host) {
    const double inv = f(1.0 / rd<float>(field, 0x1C));
    void* node = fresh_node(field, host);
    out[0] = ftol(((static_cast<double>(x) - rd<float>(node, 0x120)) - rd<float>(field, 0x80)) * inv);
    node = fresh_node(field, host);
    out[1] = ftol(((static_cast<double>(z) - rd<float>(node, 0x128)) - rd<float>(field, 0x84)) * inv);
    return out;
}

float height_field_height_at_world_00ada990(void* field, float x, float z, HeightFieldHost& host) {
    const double inv = f(1.0 / rd<float>(field, 0x1C));
    void* node = fresh_node(field, host);
    const float u = f(((static_cast<double>(x) - rd<float>(node, 0x120)) - rd<float>(field, 0x80)) * inv);
    node = fresh_node(field, host);
    const float v = f(((static_cast<double>(z) - rd<float>(node, 0x128)) - rd<float>(field, 0x84)) * inv);
    const double bilinear = host.slot50_sample_bilinear(field, u, v);
    return f(host.slot48_grid_height(field, u, v) + bilinear);
}

std::int32_t height_field_samples_00adac50(const void* field, bool deep) {
    const std::int32_t n = rd<std::int32_t>(field, deep ? 0x3C : 0x38);
    return n == 0 ? 0 : n * kSamplesPerTile_00D5D658 + 1;
}

void height_field_compute_height_range(std::uint32_t address, void* field) {
    if (address == 0x00ADB280) {
        wr<float>(field, 0x10, 100000.0f);
        wr<float>(field, 0x14, -100000.0f);
    } else if (address == 0x00ADBD00) {
        wr<float>(field, 0x10, 1e10f);
        wr<float>(field, 0x14, kNoDataHeight_00D7A240);
    } else {
        throw std::invalid_argument("not a height-range scan");
    }
    const std::uint32_t columns = rd<std::uint32_t>(field, 0x38), rows = rd<std::uint32_t>(field, 0x3C);
    auto* const* tiles = rd<void* const*>(field, 0x40);
    for (std::uint32_t r = 0; r < rows; ++r) {
        for (std::uint32_t c = 0; c < columns; ++c) {
            const void* tile = tiles[columns * r + c];
            if (tile == nullptr) continue;
            const float lo = rd<float>(tile, 0x14), mn = rd<float>(field, 0x10);
            wr<float>(field, 0x10, lo > mn ? mn : lo);
            const float hi = rd<float>(tile, 0x18), mx = rd<float>(field, 0x14);
            wr<float>(field, 0x14, mx > hi ? mx : hi);
        }
    }
}

float height_field_sample_at_00adb550(void* field, std::int32_t i, std::int32_t j, HeightFieldHost& host) {
    if (i < 0 || i >= host.slot10_samples_wide(field)) return 0.0f;
    if (j < 0 || j >= host.slot14_samples_deep(field)) return 0.0f;
    std::int32_t tx = 0, tz = 0, lx = 0, lz = 0;
    host.slot44_split_index(field, &tx, &tz, &lx, &lz, i, j);
    const void* tile = rd<void* const*>(field, 0x40)[rd<std::int32_t>(field, 0x38) * tz + tx];
    if (tile == nullptr) return 0.0f;
    const std::uint8_t b = block_byte_layer_sample_00adb1d0(tile_block_00adea70(tile), lx, lz);
    return f(static_cast<double>(b) / 255.0 * 60.0);
}

float height_field_sample_bilinear_00adb620(void* field, float u, float v, HeightFieldHost& host) {
    const std::int32_t j = static_cast<std::int32_t>(v);
    const std::int32_t i = static_cast<std::int32_t>(u);
    const float fu = f(static_cast<double>(u) - i);
    const float h11 = host.slot4c_sample(field, i + 1, j + 1);
    const float h01 = host.slot4c_sample(field, i, j + 1);
    const float h10 = host.slot4c_sample(field, i + 1, j);
    const float h00 = host.slot4c_sample(field, i, j);
    const float a = f(h00 + (static_cast<double>(h10) - h00) * fu);
    const float b = f(h01 + static_cast<double>(fu) * (static_cast<double>(h11) - h01));
    const float fv = f(static_cast<double>(v) - j);
    return f(a + (static_cast<double>(b) - a) * fv);
}

void tile_grid_load_tile_from_node_00adbc90(void* field, void* node, HeightFieldHost& host) {
    const std::int32_t x = host.node_read_dword_00be99f0(node);
    const std::int32_t y = host.node_read_dword_00be99f0(node);
    fresh_node(field, host);
    fresh_node(field, host);
    void* tile = host.create_tile(field, x, y);
    rd<void**>(field, 0x40)[rd<std::int32_t>(field, 0x38) * y + x] = tile;
    host.tile_read_chunks_00adfeb0(tile, node);
}

void landscape_release_tiles_00adbf80(void* field, TerrainHost& host) {
    for (std::int32_t r = 0; r < rd<std::int32_t>(field, 0x3C); ++r)
        for (std::int32_t c = 0; c < rd<std::int32_t>(field, 0x38); ++c)
            release_ref_slot(&rd<void**>(field, 0x40)[rd<std::int32_t>(field, 0x38) * r + c], host);
    void* foliage = rd<void*>(field, 0x5C);
    foliage_system_remove_landscape_00ad7b90(foliage, field, host);
    apply_field_setter(0x00AD56C0, foliage, 0);
    for (std::uint32_t off : {0x30u, 0x34u, 0x64u, 0x60u}) release_ref_slot(reinterpret_cast<void**>(at(field, off)), host);
}

void tile_grid_for_each_tile_00adb230(void* field, std::uint32_t argument, HeightFieldHost& host) {
    for (std::int32_t r = 0; r < rd<std::int32_t>(field, 0x3C); ++r)
        for (std::int32_t c = 0; c < rd<std::int32_t>(field, 0x38); ++c)
            tile_forward_slot50_00adead0(rd<void**>(field, 0x40)[rd<std::int32_t>(field, 0x38) * r + c], argument, host);
}

void tile_forward_slot50_00adead0(void* tile, std::uint32_t argument, HeightFieldHost& host) {
    host.tile_member_slot50(rd<void*>(tile, 0x24), argument);
}

void* tile_block_00adea70(const void* tile) { return rd<void*>(tile, 0x1C); }

void byte_layer_stamp_max_00adb160(void* layer, std::int32_t x, std::int32_t y, std::uint8_t value) {
    const std::int32_t w = rd<std::int32_t>(layer, 0x20), h = rd<std::int32_t>(layer, 0x24);
    if (x < 1) x = 0;
    if (x >= w - 1) x = w - 1;
    if (y < 1) y = 0;
    if (y >= h - 1) y = h - 1;
    std::uint8_t* cell = rd<std::uint8_t*>(layer, 0x28) + w * y + x;
    if (*cell < value) *cell = value;
}

std::uint8_t block_byte_layer_sample_00adb1d0(const void* layer, std::int32_t x, std::int32_t y) {
    if (x < 1) x = 0;
    else if (x > 0x20) x = 0x21;
    if (y < 1) y = 0;
    else if (y > 0x20) y = 0x21;
    return rd<const std::uint8_t*>(layer, 0x28)[rd<std::int32_t>(layer, 0x20) * y + x];
}

const GridShape kGridResizes[7] = {
    {0x00ADACF0, 4, false}, {0x00ADAF50, 2, false}, {0x00ADF5E0, 1, false}, {0x00AED2D0, 4, false},
    {0x00AED480, 0xC, false}, {0x00ADADC0, 4, true}, {0x00AED210, 1, true},
};

void grid_resize(std::uint32_t address, void* grid, std::int32_t w, std::int32_t h, const void* fill,
    TerrainHost& host) {
    const GridShape* shape = nullptr;
    for (const GridShape& s : kGridResizes)
        if (s.address == address) shape = &s;
    if (shape == nullptr) throw std::invalid_argument("not a grid resize");
    const std::uint64_t bytes = static_cast<std::uint64_t>(static_cast<std::uint32_t>(w * h)) * shape->element_size;
    auto* fresh = static_cast<std::uint8_t*>(host.operator_new(bytes >> 32 ? 0xFFFFFFFFu : static_cast<std::uint32_t>(bytes)));
    const std::int32_t old_w = rd<std::int32_t>(grid, 0), old_h = rd<std::int32_t>(grid, 4);
    const auto* old = rd<const std::uint8_t*>(grid, 8);
    const std::int32_t cw = std::min(w, old_w), ch = std::min(h, old_h);
    const std::uint32_t size = shape->element_size;
    for (std::int32_t y = 0; y < (shape->fill ? h : ch); ++y) {
        for (std::int32_t x = 0; x < (shape->fill ? w : cw); ++x) {
            std::uint8_t* dst = fresh + (static_cast<std::size_t>(y) * w + x) * size;
            if (x < cw && y < ch)
                std::memcpy(dst, old + (static_cast<std::size_t>(old_w) * y + x) * size, size);
            else
                std::memcpy(dst, fill, size);
        }
    }
    if (old != nullptr) {
        host.free(const_cast<std::uint8_t*>(old));
        wr<std::uint32_t>(grid, 8, 0);
    }
    wr<std::int32_t>(grid, 0, w);
    wr<std::int32_t>(grid, 4, h);
    wr<void*>(grid, 8, fresh);
}

void grid_clear(void* grid, TerrainHost& host) {
    if (void* cells = rd<void*>(grid, 8)) {
        host.free(cells);
        wr<std::uint32_t>(grid, 8, 0);
    }
    wr<std::uint32_t>(grid, 4, 0);
    wr<std::uint32_t>(grid, 0, 0);
}

// ---------------------------------------------------------------------------
// Blocks and tiles.

namespace {
void ref_counted_base(void* object) {
    wr<std::uint32_t>(object, 0, 0x00CEB130);
    wr<std::uint32_t>(object, 4, 1);
}

// The free-and-zero tail shared by the block destructors: free cells, zero w/h words.
void free_grid_words(void* object, std::uint32_t cells, std::uint32_t w, std::uint32_t h, TerrainHost& host) {
    if (void* p = rd<void*>(object, cells)) {
        host.free(p);
        wr<std::uint32_t>(object, cells, 0);
    }
    wr<std::uint32_t>(object, w, 0);
    wr<std::uint32_t>(object, h, 0);
}

void block_base_level_destruct(void* block, TerrainHost& host, BlockHost& blocks) {  // 00AEDA80
    wr<std::uint32_t>(block, 0, 0x00D5D6EC);
    free_grid_words(block, 0x28, 0x20, 0x24, host);
    wr<std::uint32_t>(block, 0, 0x00D5D6B0);
    blocks.ref_counted_destroy_base(block);
}
}  // namespace

void* terrain_block_construct_base_00aed9a0(void* block, std::int32_t w, std::int32_t h, TerrainHost& host) {
    ref_counted_base(block);
    wr<std::uint32_t>(block, 8, 0);
    wr<std::int32_t>(block, 0xC, -1);
    wr<std::int32_t>(block, 0x10, w);
    wr<std::int32_t>(block, 0x14, h);
    wr<std::uint32_t>(block, 0, 0x00D5D6EC);
    for (std::uint32_t off : {0x20u, 0x24u, 0x28u}) wr<std::uint32_t>(block, off, 0);
    wr<float>(block, 0x18, 1e10f);
    wr<float>(block, 0x1C, -1e10f);
    const std::uint8_t zero = 0;
    grid_resize(0x00AED210, at(block, 0x20), w, h, &zero, host);
    return block;
}

void* terrain_block_construct_base_00aedbc0(void* block, std::int32_t w, std::int32_t h, TerrainHost& host) {
    ref_counted_base(block);
    wr<std::uint32_t>(block, 8, 0);
    wr<std::int32_t>(block, 0xC, -1);
    wr<std::int32_t>(block, 0x10, w);
    wr<std::int32_t>(block, 0x14, h);
    wr<std::uint32_t>(block, 0, 0x00D5D75C);
    for (std::uint32_t off : {0x18u, 0x1Cu, 0x20u}) wr<std::uint32_t>(block, off, 0);
    grid_resize(0x00AED480, at(block, 0x18), w, h, nullptr, host);
    return block;
}

void* terrain_block_construct(std::uint32_t address, void* block, std::int32_t w, std::int32_t h,
    TerrainHost& host) {
    terrain_block_construct_base_00aed9a0(block, w, h, host);
    switch (address) {
    case 0x00AEDAC0:
        wr<std::uint32_t>(block, 0, 0x00D5D724);
        for (std::uint32_t off : {0x2Cu, 0x30u, 0x34u}) wr<std::uint32_t>(block, off, 0);
        grid_resize(0x00AED2D0, at(block, 0x2C), w, h, nullptr, host);
        wr<std::uint32_t>(block, 0xC, 0);
        break;
    case 0x00ADC420:
        wr<std::uint32_t>(block, 0, 0x00D5D2DC);
        for (std::uint32_t off : {0x38u, 0x3Cu, 0x40u}) wr<std::uint32_t>(block, off, 0);
        grid_resize(0x00ADAF50, at(block, 0x38), w, h, nullptr, host);
        break;
    case 0x00ADF9F0:
        wr<std::uint32_t>(block, 0, 0x00D5D424);
        for (std::uint32_t off : {0x38u, 0x3Cu, 0x40u}) wr<std::uint32_t>(block, off, 0);
        grid_resize(0x00ADF5E0, at(block, 0x38), w, h, nullptr, host);
        break;
    default:
        throw std::invalid_argument("not a terrain block constructor");
    }
    return block;
}

void terrain_block_destruct(std::uint32_t address, void* block, TerrainHost& host, BlockHost& blocks) {
    switch (address) {
    case 0x00AEDA80:
        block_base_level_destruct(block, host, blocks);
        return;
    case 0x00AEDC40:
        wr<std::uint32_t>(block, 0, 0x00D5D75C);
        free_grid_words(block, 0x20, 0x18, 0x1C, host);
        wr<std::uint32_t>(block, 0, 0x00D5D6B0);
        blocks.ref_counted_destroy_base(block);
        return;
    case 0x00AEDB60:
        wr<std::uint32_t>(block, 0, 0x00D5D724);
        free_grid_words(block, 0x34, 0x2C, 0x30, host);
        block_base_level_destruct(block, host, blocks);
        return;
    case 0x00ADC680:
    case 0x00ADC8A0:
        wr<std::uint32_t>(block, 0, 0x00D5D2DC);
        free_grid_words(block, 0x40, 0x38, 0x3C, host);
        block_base_level_destruct(block, host, blocks);
        return;
    case 0x00ADFC50:
    case 0x00ADFD10:
        wr<std::uint32_t>(block, 0, 0x00D5D424);
        free_grid_words(block, 0x40, 0x38, 0x3C, host);
        block_base_level_destruct(block, host, blocks);
        return;
    default:
        throw std::invalid_argument("not a terrain block destructor");
    }
}

float terrain_float_block_default_height_00aedb40() { return -1e10f; }

void terrain_u8_block_sample_00adfb80(const void* block, float* out, std::int32_t i, std::int32_t j) {
    const std::uint8_t b = rd<const std::uint8_t*>(block, 0x40)[rd<std::int32_t>(block, 0x38) * j + i];
    *out = b == 0xFF ? kNoDataHeight_00D7A240
                     : f(static_cast<double>(b) * rd<float>(block, 0x34) + rd<float>(block, 0x2C));
}

void terrain_float_layer_get_00aed710(const void* block, float* out, std::int32_t i, std::int32_t j) {
    *out = rd<const float*>(block, 0x34)[rd<std::int32_t>(block, 0x2C) * j + i];
}

void terrain_float_layer_fill_00aed890(void* block, float value) {
    for (std::int32_t r = 0; r < rd<std::int32_t>(block, 0x30); ++r)
        for (std::int32_t c = 0; c < rd<std::int32_t>(block, 0x2C); ++c)
            rd<float*>(block, 0x34)[rd<std::int32_t>(block, 0x2C) * r + c] = value;
}

void* terrain_tile_construct_00adf6b0(void* tile, std::uint32_t a, std::uint32_t b, std::uint32_t c) {
    wr<std::uint32_t>(tile, 8, a);
    wr<std::uint32_t>(tile, 0xC, b);
    wr<std::uint32_t>(tile, 0x10, c);
    ref_counted_base(tile);
    wr<std::uint32_t>(tile, 0, 0x00D5D418);
    for (std::uint32_t off = 0x1C; off < 0x38; off += 4) wr<std::uint32_t>(tile, off, 0);
    return tile;
}

void terrain_tile_destruct_00adf7e0(void* tile, TerrainHost& host, BlockHost& blocks) {
    wr<std::uint32_t>(tile, 0, 0x00D5D418);
    if (rd<void*>(tile, 0x24) != nullptr) host.node_unlink_and_release_00b6d8f0(rd<void*>(tile, 0x24));
    for (std::uint32_t off = 0x28; off < 0x38; off += 4) release_ref_slot(reinterpret_cast<void**>(at(tile, off)), host);
    release_ref_slot(reinterpret_cast<void**>(at(tile, 0x1C)), host);
    release_ref_slot(reinterpret_cast<void**>(at(tile, 0x20)), host);
    blocks.ref_counted_destroy_base(tile);
}

namespace {
// Retained assignment: no-op when unchanged, else store, AddRef new, release old.
bool assign_retained(void* object, std::uint32_t offset, void* value, TerrainHost& host) {
    void* old = rd<void*>(object, offset);
    if (old == value) return false;
    wr<void*>(object, offset, value);
    if (value != nullptr) _InterlockedIncrement(reinterpret_cast<volatile long*>(at(value, 4)));
    release_ref_slot(&old, host);
    return true;
}
}  // namespace

void terrain_tile_set_block_00adf8b0(void* tile, void* block, TerrainHost& host, BlockHost& blocks) {
    if (!assign_retained(tile, 0x1C, block, host)) return;
    terrain_tile_refresh_height_range_00adeb00(tile, blocks);
}

void terrain_tile_set_second_block_00adf920(void* tile, void* block, TerrainHost& host) {
    assign_retained(tile, 0x20, block, host);
}

void terrain_tile_create_block(std::uint32_t address, void* tile, void* chunk, TerrainHost& host,
    BlockHost& blocks) {
    void* block = nullptr;
    if (address == 0x00ADF960) {
        if (void* memory = host.operator_new(0x38)) block = terrain_block_construct(0x00AEDAC0, memory, 0x21, 0x21, host);
    } else if (address == 0x00ADFE10) {
        if (void* memory = host.operator_new(0x44)) {
            block = terrain_block_construct(0x00ADF9F0, memory, 0x21, 0x21, host);
            wr<std::uint32_t>(block, 0, 0x00D5D45C);
            wr<std::uint32_t>(block, 0xC, 1);
        }
    } else {
        throw std::invalid_argument("not a tile block factory");
    }
    blocks.block_slot10_read(block, chunk);
    terrain_tile_set_block_00adf8b0(tile, block, host, blocks);
    release_ref_slot(&block, host);
}

void terrain_tile_refresh_height_range_00adeb00(void* tile, BlockHost& blocks) {
    float lo = 0.0f, hi = 0.0f;
    blocks.block_slot0c_range(rd<void*>(tile, 0x1C), &lo, &hi);
    wr<float>(tile, 0x14, lo);
    wr<float>(tile, 0x18, hi);
}

void tile_list_release_00adc8e0(void* owner, TerrainHost& host, BlockHost& blocks) {
    if (rd<std::uint8_t>(owner, 0x50) == 0) return;
    for (std::int32_t i = 0; i < rd<std::int32_t>(owner, 0x48); ++i) {
        void* tile = rd<void**>(owner, 0x44)[i];
        terrain_tile_refresh_height_range_00adeb00(tile, blocks);
        host.call_virtual(tile, 8);
    }
    // 00ADBE60(0): shrinking only sets the count.
    wr<std::int32_t>(owner, 0x48, 0);
    wr<std::uint8_t>(owner, 0x50, 0);
}

void height_field_destroy_00add1b0(void* field, TerrainHost& host, HeightFieldDestroyHost& more) {
    wr<std::uint32_t>(field, 0, 0x00D5D350);
    landscape_release_tiles_00adbf80(field, host);
    more.quadtree_destroy_00aea8e0(at(field, 0x88));
    release_native_string(at(field, 0x54), host);
    wr<std::int32_t>(field, 0x48, 0);  // 00ADBE60(0)
    host.free(rd<void*>(field, 0x44));
    free_grid_words(field, 0x40, 0x38, 0x3C, host);
    release_native_string(at(field, 0x20), host);
    more.weak_link_destruct_00ae9930(field);
}

} // namespace bsp::cc13_terrain

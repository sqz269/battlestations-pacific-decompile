#include "bsp/terrain_particles_cc13.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/terrain_render.hpp"
#include "bsp/camera_affine.hpp"
#include "bsp/camera_multiply.hpp"
#include "bsp/camera_projection.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_string.hpp"

#include <intrin.h>

#include <algorithm>
#include <array>
#include <utility>
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
constexpr std::uint32_t kPushBack[] = {0x00AD6A90, 0x00AF6820, 0x00AF88F0, 0x00ADBDF0, 0x00AEE010, 0x00AEE080};

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
                bush_group_destruct_00ae1c80(group, host);
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
                tree_group_destruct_00ae4c20(group, host);
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
            // Both operands pass FLD / FSTP DWORD to stack slots first, which quiets a
            // signalling NaN (cc13-fixture6).
            const float lo = cc13x87::x87_fld_fstp(rd<float>(tile, 0x14));
            const float mn = cc13x87::x87_fld_fstp(rd<float>(field, 0x10));
            wr<float>(field, 0x10, lo > mn ? mn : lo);
            const float hi = cc13x87::x87_fld_fstp(rd<float>(tile, 0x18));
            const float mx = cc13x87::x87_fld_fstp(rd<float>(field, 0x14));
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

void* lazy_singleton_get(std::uint32_t address, TerrainHost& host) {
    std::uint32_t global = 0, size = 0;
    if (address == 0x00ADC7B0) {
        global = 0x0109DBB4;
        size = 4;
    } else if (address == 0x00AF0990) {
        global = 0x00F8C278;
        size = 8;
    } else {
        throw std::invalid_argument("not a lazy singleton getter");
    }
    if (host.read_global(global) == nullptr) {
        host.lifetime_lock(true);
        if (host.read_global(global) == nullptr) {
            void* object = host.operator_new(size);
            host.write_global(global, object);
            void* registered = object;
            if (object != nullptr) {
                if (size == 4) {
                    wr<std::uint32_t>(object, 0, 0x00D5D2D8);
                } else {
                    wr<std::uint32_t>(object, 4, 0x00D5D7E8);
                    wr<std::uint32_t>(object, 0, 0x00D5D7F4);
                    wr<std::uint32_t>(object, 4, 0x00D5D7F0);
                    registered = at(object, 4);
                }
            }
            host.lifetime_register_00bd0c30(size == 4 ? host.read_global(global) : registered);
        }
        host.lifetime_unlock(true);
    }
    return host.read_global(global);
}

void particle_system_create_lock_00af19b0(TerrainHost& host, ParticleHost& particles) {
    host.write_global(0x00F8C284, particles.critical_section_create());
}

void particle_system_destroy_lock_00af2650(TerrainHost& host, ParticleHost& particles) {
    if (void* cs = host.read_global(0x00F8C284)) {
        while (rd<std::int32_t>(cs, 0x18) > 0) {
            wr<std::int32_t>(cs, 0x18, rd<std::int32_t>(cs, 0x18) - 1);
            particles.critical_section_leave(cs);
        }
        particles.critical_section_delete(cs);
        host.free(cs);
    }
    host.write_global(0x00F8C284, nullptr);
}

bool byte_buffer_reset_00af55b0(void* buffer, std::uint32_t size, TerrainHost& host) {
    wr<std::uint32_t>(buffer, 8, size);
    wr<std::uint32_t>(buffer, 0x18, 0);
    wr<std::uint32_t>(buffer, 0, 0);
    if (void* old = rd<void*>(buffer, 0x14)) host.free(old);
    void* fresh = host.operator_new(rd<std::uint32_t>(buffer, 8));
    wr<void*>(buffer, 0x14, fresh);
    return fresh != nullptr;
}

float random_variation_factor_00af5a30(float spread, ParticleHost& particles) {
    if (spread == 0.0f) return 1.0f;
    const auto draw = static_cast<std::int32_t>(particles.draw_u32_stream0_00bd2fc0());
    return f(static_cast<double>(draw) * spread * 4.656612873077393e-10 + 1.0);
}

float random_fraction_times_00af7f70(float x, ParticleHost& particles) {
    if (x == 0.0f) return 0.0f;
    return f(static_cast<double>(particles.draw_fraction_stream0_00bd2f40()) * x);
}

float particle_emitter_random_sign_00af7fb0(const void* emitter, ParticleHost& particles) {
    if (rd<std::uint8_t>(emitter, 0x29) == 0) return 1.0f;
    return particles.draw_fraction_stream0_00bd2f40() >= 0.5f ? 1.0f : -1.0f;
}

void particle_range_set(std::uint32_t address, void* range, std::uint32_t a, std::uint32_t b) {
    std::uint32_t off = 0;
    if (address == 0x00AFB790) off = 8;
    else if (address == 0x00AFB7C0) off = 0x10;
    else throw std::invalid_argument("not a particle range setter");
    wr<std::uint32_t>(range, off, a);
    wr<std::uint32_t>(range, off + 4, b);
}

void particle_model_refresh_bounds_00af5b70(void* model, ParticleHost& particles) {
    std::uint8_t local[0x10] = {};
    void* bounds = particles.particle_definition_copy_bounds_00af3f50(rd<void*>(model, 0x18C), local);
    particles.generated_model_set_local_bounds_00b74390(model, bounds);
}

void foliage_group_manager_rebuild_particle_frame_00af0520(TerrainHost& host, ParticleHost& particles) {
    particles.particle_model_build_frame_geometry(rd<void*>(host.read_global(0x00F8C274), 0x1C));
}

void particle_model_advance_vertex_cursor_00af0540(void* model, std::int32_t count, ParticleHost& particles) {
    if (rd<std::uint32_t>(model, 0x24) == 0 || rd<std::uint32_t>(model, 0x28) == 0) return;
    wr<std::int32_t>(model, 0x28, rd<std::int32_t>(model, 0x28) + particles.particle_model_vertex_stride(model) * count);
}

void pointer_array_grow(std::uint32_t address, PtrArray& a) {
    if (address != 0x00AF1B90 && address != 0x00AF5D80) throw std::invalid_argument("not a grow routine");
    bsp::cc13_scene2::array_grow_double_plus_two(a);
}

void type_info_initialize_mw_particle_system_00af6b90(void* type_info, TerrainHost& host, ParticleHost& particles) {
    if (host.read_global_u8(0x00F8D2CC) != 0) return;
    host.write_global_u8(0x00F8D2CC, 1);
    wr<const char*>(type_info, 0x10, "MwParticleSystem");  // 00D5DA3C
    particles.type_info_initialize_3d_object_00b74f90(0x01090034);
    wr<std::uint32_t>(type_info, 4, host.read_global_u32(0x01090034));
    wr<std::uint32_t>(type_info, 8, host.read_global_u32(0x01090038));
    wr<std::uint32_t>(type_info, 0xC, host.read_global_u32(0x0109003C));
    void* counter = particles.type_id_counter_singleton();
    const std::uint32_t id = rd<std::uint32_t>(counter, 4);
    wr<std::uint32_t>(counter, 4, id + 1);
    wr<std::uint32_t>(type_info, 0, id);
}

void* type_info_construct_mw_particle_system_00af7490(void* type_info, TerrainHost& host, ParticleHost& particles) {
    type_info_initialize_mw_particle_system_00af6b90(type_info, host, particles);
    return type_info;
}

void* particle_parameter_scalar_deleting_dtor_00af81c0(void* parameter, std::uint8_t flags, ParticleHost& particles) {
    particles.particle_parameter_dispose_payload(parameter);
    if (flags & 1) particles.particle_parameter_return_pool_slot(parameter);
    return parameter;
}

void particle_parameter_ptr_delete_00af8400(void** slot, ParticleHost& particles) {
    void* parameter = *slot;
    if (parameter == nullptr) return;
    particles.particle_parameter_dispose_payload(parameter);
    particles.particle_parameter_return_pool_slot(parameter);
    *slot = nullptr;
}

void particle_shader_set_destroy_00af12a0(void* set, TerrainHost& host) {
    wr<std::uint32_t>(set, 0, 0x00D5D844);
    void* first = rd<void*>(set, 4);
    if (_InterlockedDecrement(reinterpret_cast<volatile long*>(at(first, 4))) == 0) host.call_virtual(first, 0);
    for (std::uint32_t off : {0x18u, 0x08u, 0x0Cu, 0x10u, 0x14u, 0x1Cu, 0x20u, 0x24u, 0x28u, 0x2Cu})
        release_ref_slot(reinterpret_cast<void**>(at(set, off)), host);
    destroy_singleton_base(set, kSingletonBases[3], host);
}

void particle_keys_push_00afbe90(void* keys, const void* key, const KeyReserve& reserve) {
    auto& a = *static_cast<NativeArray<std::uint8_t>*>(keys);
    if (a.count == a.capacity) reserve(a, a.capacity * 2);
    std::memcpy(a.data + a.count * 0x2C, key, 0x2C);
    ++a.count;
}

void* particle_keys_assign_00afc1d0(void* keys, const void* source, const KeyReserve& reserve, TerrainHost& host) {
    auto& a = *static_cast<NativeArray<std::uint8_t>*>(keys);
    const auto& s = *static_cast<const NativeArray<std::uint8_t>*>(source);
    host.free(a.data);
    a.data = nullptr;
    a.count = 0;
    a.capacity = 0;
    reserve(a, s.count);
    for (std::int32_t i = 0; i < s.count; ++i) particle_keys_push_00afbe90(keys, s.data + i * 0x2C, reserve);
    return keys;
}

void* vector_deleting_destructor(std::uint32_t address, void* object, std::uint8_t flags,
    const std::function<void(void*)>& element_destructor, TerrainHost& host) {
    std::uint32_t size = 0;
    bool scalar_destructs = true;
    switch (address) {
    case 0x00AFCE30: size = 1; scalar_destructs = false; break;
    case 0x00AFCE80: size = 0x108; break;
    case 0x00B04AB0: size = 8; break;
    default: throw std::invalid_argument("not a vector deleting destructor");
    }
    if (flags & 2) {
        std::uint8_t* base = static_cast<std::uint8_t*>(object) - 4;
        const std::uint32_t count = rd<std::uint32_t>(base, 0);
        for (std::uint32_t i = count; i-- > 0;) element_destructor(static_cast<std::uint8_t*>(object) + i * size);
        if (flags & 1) host.free(base);
        return base;
    }
    if (scalar_destructs) element_destructor(object);
    if (flags & 1) host.free(object);
    return object;
}

void particle_emitter_set_update_transforms_00afd820(void* set, std::uint32_t argument,
    const std::function<void(void* emitter, std::uint32_t argument)>& update_00afdcf0) {
    for (std::int32_t i = 0; i < rd<std::int32_t>(set, 0x14); ++i) {
        const std::uint8_t index = rd<const std::uint8_t*>(set, 4)[i];
        update_00afdcf0(rd<std::uint8_t*>(set, 0xC) + index * 0x108u, argument);
    }
}

void pointer_array_push_back_plus_one_00afebe0(PtrArray& a, void* value) {
    if (a.count == a.capacity) {
        std::int32_t n = a.capacity + 1;
        if (n < 2) n = 1;
        bsp::cc13_scene2::array_reserve_min_one(a, n);
    }
    a.data[a.count++] = value;
}

PtrArray& pointer_array_assign_00afece0(PtrArray& a, const PtrArray& source) {
    pointer_array_resize(0x00AFEC20, a, 0);
    bsp::cc13_scene2::array_reserve_min_one(a, source.count);
    for (std::int32_t i = 0; i < source.count; ++i) pointer_array_push_back_plus_one_00afebe0(a, source.data[i]);
    return a;
}

namespace {
// The image-memory pointer array {data, count, capacity} of a named list (+08).
void list_values_reserve(void* arr, std::int32_t n, TerrainHost& host) {  // 00AFEB60 over image memory
    if (n < 1) n = 1;
    if (rd<std::int32_t>(arr, 8) >= n) return;
    auto* fresh = static_cast<std::uint32_t*>(host.operator_new(static_cast<std::uint32_t>(n) * 4));
    for (std::int32_t i = 0; i < rd<std::int32_t>(arr, 4); ++i) fresh[i] = rd<std::uint32_t*>(arr, 0)[i];
    host.free(rd<void*>(arr, 0));
    wr<void*>(arr, 0, fresh);
    wr<std::int32_t>(arr, 8, n);
}

void list_values_push(void* arr, std::uint32_t value, TerrainHost& host) {  // 00AFEBE0 shape
    if (rd<std::int32_t>(arr, 4) == rd<std::int32_t>(arr, 8)) {
        std::int32_t n = rd<std::int32_t>(arr, 8) + 1;
        if (n < 2) n = 1;
        list_values_reserve(arr, n, host);
    }
    rd<std::uint32_t*>(arr, 0)[rd<std::int32_t>(arr, 4)] = value;
    wr<std::int32_t>(arr, 4, rd<std::int32_t>(arr, 4) + 1);
}
}  // namespace

void named_pointer_list_destroy_00afee90(void* list, TerrainHost& host) {
    wr<std::int32_t>(list, 0xC, 0);  // 00AFEC20(0) twice: shrinking only sets the count
    host.free(rd<void*>(list, 8));
    release_native_string(list, host);
}

void* named_pointer_list_construct_00afef30(void* list, void* name, void* value, TerrainHost& host) {
    for (std::uint32_t off = 0; off < 0x14; off += 4) wr<std::uint32_t>(list, off, 0);
    if (list != name) host.native_string_assign_0041dd40(list, rd<const void*>(name, 4), rd<std::uint32_t>(name, 0));
    list_values_push(at(list, 8), static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(value)), host);
    release_native_string(name, host);
    return list;
}

void* named_pointer_list_copy_00aff010(void* list, const void* source, TerrainHost& host) {
    wr<std::uint32_t>(list, 0, 0);
    wr<std::uint32_t>(list, 4, 0);
    if (list != source) host.native_string_assign_0041dd40(list, rd<const void*>(source, 4), rd<std::uint32_t>(source, 0));
    for (std::uint32_t off = 8; off < 0x14; off += 4) wr<std::uint32_t>(list, off, 0);
    // 00AFECE0: resize 0, reserve the source count, push each value.
    const void* src = at(const_cast<void*>(source), 8);
    wr<std::int32_t>(list, 0xC, 0);
    list_values_reserve(at(list, 8), rd<std::int32_t>(src, 4), host);
    for (std::int32_t i = 0; i < rd<std::int32_t>(src, 4); ++i)
        list_values_push(at(list, 8), rd<const std::uint32_t*>(src, 0)[i], host);
    return list;
}

void named_list_array_reserve_00aff0b0(void* array, std::int32_t n, TerrainHost& host) {
    if (n < 1) n = 1;
    if (rd<std::int32_t>(array, 8) >= n) return;
    auto* fresh = static_cast<std::uint8_t*>(host.operator_new(static_cast<std::uint32_t>(n) * 0x14));
    auto* old = rd<std::uint8_t*>(array, 0);
    const std::int32_t count = rd<std::int32_t>(array, 4);
    for (std::int32_t i = 0; i < count; ++i) named_pointer_list_copy_00aff010(fresh + i * 0x14, old + i * 0x14, host);
    for (std::int32_t i = 0; i < count; ++i) named_pointer_list_destroy_00afee90(old + i * 0x14, host);
    host.free(old);
    wr<void*>(array, 0, fresh);
    wr<std::int32_t>(array, 8, n);
}

void named_list_array_push_back_00aff190(void* array, const void* list, TerrainHost& host) {
    if (rd<std::int32_t>(array, 4) == rd<std::int32_t>(array, 8)) {
        std::int32_t n = rd<std::int32_t>(array, 8) + 1;
        if (n < 2) n = 1;
        named_list_array_reserve_00aff0b0(array, n, host);
    }
    named_pointer_list_copy_00aff010(rd<std::uint8_t*>(array, 0) + rd<std::int32_t>(array, 4) * 0x14, list, host);
    wr<std::int32_t>(array, 4, rd<std::int32_t>(array, 4) + 1);
}

void named_list_array_resize_00aff210(void* array, std::int32_t n, TerrainHost& host) {
    if (rd<std::int32_t>(array, 8) < n) named_list_array_reserve_00aff0b0(array, n, host);
    for (std::int32_t i = rd<std::int32_t>(array, 4); i < n; ++i)
        std::memset(rd<std::uint8_t*>(array, 0) + i * 0x14, 0, 0x14);
    while (n < rd<std::int32_t>(array, 4)) {
        wr<std::int32_t>(array, 4, rd<std::int32_t>(array, 4) - 1);
        named_pointer_list_destroy_00afee90(rd<std::uint8_t*>(array, 0) + rd<std::int32_t>(array, 4) * 0x14, host);
    }
    wr<std::int32_t>(array, 4, n);
}

void named_list_array_destroy(std::uint32_t address, void* array, TerrainHost& host) {
    if (address != 0x00AFF420 && address != 0x00AFF450) throw std::invalid_argument("not a named-list array destroy");
    named_list_array_resize_00aff210(array, 0, host);
    host.free(rd<void*>(array, 0));
}

void named_list_array_add_under_name_00aff290(void* array, void* name, void* value, TerrainHost& host) {
    bool found = false;
    for (std::int32_t i = 0; i < rd<std::int32_t>(array, 4); ++i) {
        std::uint8_t* list = rd<std::uint8_t*>(array, 0) + i * 0x14;
        if (!names_equal(list, name, host)) continue;
        list_values_push(list + 8, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(value)), host);
        found = true;
        break;
    }
    if (!found) {
        void* temporary = host.operator_new(0x14);
        if (temporary != nullptr) {
            NativeStr copy{0, nullptr};
            host.native_string_assign_0041dd40(&copy, rd<const void*>(name, 4), rd<std::uint32_t>(name, 0));
            named_pointer_list_construct_00afef30(temporary, &copy, value, host);
        }
        named_list_array_push_back_00aff190(array, temporary, host);  // the temporary leaks, as in the image
    }
    release_native_string(name, host);
}

void particle_emitter_destruct_00aff8c0(void* emitter, TerrainHost& host, BlockHost& blocks) {
    wr<std::uint32_t>(emitter, 0, 0x00D5DCB4);
    if (void* container = rd<void*>(emitter, 0x10)) {
        host.call_virtual_u32(container, 4, 1);
        wr<std::uint32_t>(emitter, 0x10, 0);
    }
    release_ref_slot(reinterpret_cast<void**>(at(emitter, 0xC)), host);
    blocks.ref_counted_destroy_base(emitter);
}

void array_reallocate_6c_00b04df0(void* array, std::uint32_t n, TerrainHost& host) {
    if (void* old = rd<void*>(array, 0)) host.free(old);
    const std::uint64_t bytes = static_cast<std::uint64_t>(n) * 0x6C;
    void* fresh = host.operator_new(bytes >> 32 ? 0xFFFFFFFFu : static_cast<std::uint32_t>(bytes));
    wr<std::uint32_t>(array, 4, n);
    wr<void*>(array, 0, fresh);
}

void dword_array_assign_00b0a710(NativeArray<std::uint32_t>& a, const NativeArray<std::uint32_t>& source,
    TerrainHost& host) {
    a.count = 0;
    dword_array_reserve_00b0a440(a, static_cast<std::uint32_t>(source.count), host);
    a.count = source.count;
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(source.count); ++i) a.data[i] = source.data[i];
}

void texture_premultiply_alpha_if_uncompressed_00b00950(void* texture, RenderMiscHost& render) {
    const std::int32_t format = render.texture_format_00b33e60(texture);
    if (format >= 0x31545844) return;
    if (format == 0x15) render.texture_premultiply_alpha_00b4c6a0(texture);
    else if (format == 0x16) render.texture_alpha_from_non_black_00b4c6d0(texture);
}

void light_set_position_00b094e0(void* light, const float xyz[3]) {
    wr<float>(light, 0x1EC, xyz[0]);
    wr<float>(light, 0x1F0, xyz[1]);
    wr<float>(light, 0x1F4, xyz[2]);
}

void tracer_resources_destruct_singleton_base_00b0a420(void* object, TerrainHost& host) {
    host.write_global(0x00F8D38C, nullptr);
    wr<std::uint32_t>(object, 0, kSingletonBaseVTable);
}

void tracer_resources_destruct_00b0aca0(void* object, TerrainHost& host) {
    wr<std::uint32_t>(object, 0, 0x00D5E074);
    release_ref_slot(reinterpret_cast<void**>(at(object, 8)), host);
    release_ref_slot(reinterpret_cast<void**>(at(object, 4)), host);
    tracer_resources_destruct_singleton_base_00b0a420(object, host);
}

void vector3_transform_projective_00b0d720(float v[3], const void* matrix, RenderMiscHost& render) {
    const float in[4] = {v[0], v[1], v[2], 1.0f};
    float out[4];
    const float* r = render.vector4_transform_00b62d10(in, out, matrix);
    const float x = r[0], y = r[1], z = r[2], w = r[3];
    const double inv = 1.0 / w;
    v[0] = f(x * inv);
    v[1] = f(y * inv);
    v[2] = f(inv * z);
}

void scene_renderer_run_batch_list_00b0e970(void* command, std::int32_t index, RenderMiscHost& render) {
    render.batch_list_slot0c(rd<void*>(command, 0xC + 4 * index), index);
    void* list = rd<void*>(command, 0xC + 4 * index);
    render.batch_list_slot08(list, index, render.render_command_camera_00b1bf40(command));
}

void render_batch_jobs_submit_00b10700(void* owner, std::int32_t index, RenderMiscHost& render) {
    wr<std::int32_t>(rd<void*>(owner, 0xC + 4 * index), 8, index);
    void* pool = render.frame_job_pool_004c1130();
    void* list = rd<void*>(owner, 0xC + 4 * index);
    render.job_queue_submit(rd<void*>(pool, 4), render.render_batch_preparation_job(), list);
}

void material_record_swap_00b13350(void* a, void* b, RenderMiscHost& render) {
    alignas(4) std::uint8_t tmp[0x12C];
    render.material_record_copy_00b13070(tmp, a);
    render.material_record_assign_00b13180(a, b);
    render.material_record_assign_00b13180(b, tmp);
    render.material_record_destroy_00b10740(tmp);
}

void scene_renderer_toggle_stats_filter_name_00b171c0(void* service, const void* name, RenderMiscHost& render) {
    const std::uint32_t element = render.stats_filter_find(service, name);
    if (element != rd<std::uint32_t>(service, 0x6A4)) render.stats_filter_erase_00589690(service, element);
    else render.stats_filter_append_00450540(service, name);
}

void* material_register_float4_array_00b18ac0(void* material, const void* name, const void* source,
    std::int32_t count, RenderMiscHost& render) {
    return render.material_register_parameter_00b17e10(material, name, source, count * 4, 0);
}

void* named_record88_scalar_deleting_dtor_00b191d0(void* record, std::uint8_t flags, TerrainHost& host,
    RenderMiscHost& render) {
    release_native_string(record, host);
    if (flags & 1) render.return_88_byte_pool_slot(record);
    return record;
}

bool scene_type_info_is_kind_of_00b199a0(std::uint32_t id, TerrainHost& host) {
    return host.read_global_u32(0x00F8D428) == id || host.read_global_u32(0x00F8D42C) == id;
}

void* registry_iterator_step(std::uint32_t address, void* iterator, RenderMiscHost& render) {
    if (address == 0x00B19AF0 || address == 0x00B19BF0) render.registry_iterator_increment(iterator);
    else if (address == 0x00B1ABE0 || address == 0x00B1ABF0) render.registry_iterator_decrement(iterator);
    else throw std::invalid_argument("not a registry iterator thunk");
    return iterator;
}

void* named_value_construct_00b1ac00(void* record, const void* name, const std::uint32_t* value, TerrainHost& host) {
    wr<std::uint32_t>(record, 0, 0);
    wr<std::uint32_t>(record, 4, 0);
    if (record != name) host.native_string_assign_0041dd40(record, rd<const void*>(name, 4), rd<std::uint32_t>(name, 0));
    wr<std::uint32_t>(record, 8, *value);
    return record;
}

void* named_value_copy_00b1ac50(void* record, const void* source, TerrainHost& host) {
    named_value_construct_00b1ac00(record, source, static_cast<const std::uint32_t*>(source) + 2, host);
    return record;
}

float* lua_global_float4_00b1b980(void* owner, float out[4], const void* name, RenderMiscHost& render) {
    void* globals = render.lua_get_globals_00b67980(owner);
    void* field = render.lua_get_by_name_00b68100(globals, name);
    render.lua_object_destruct_00b67700(globals);
    if (render.lua_is_table_00b661b0(field)) {
        render.lua_get_float4_00b67c40(field, out);
    } else {
        out[0] = out[1] = out[2] = out[3] = 1.0f;
    }
    render.lua_object_destruct_00b67700(field);
    return out;
}

void quad_nodes_reserve_00ae9ac0(NativeArray<QuadNode14>& a, std::int32_t n) {
    bsp::cc13_scene2::array_reserve_min_one(a, n);
}

void quad_nodes_push_back_00aea530(NativeArray<QuadNode14>& a, const QuadNode14& node) {
    if (a.count == a.capacity) {
        std::int32_t n = a.capacity * 2;
        if (n < 2) n = 1;
        quad_nodes_reserve_00ae9ac0(a, n);
    }
    a.data[a.count++] = node;
}

void quad_nodes_resize_00aea590(NativeArray<QuadNode14>& a, std::int32_t n) {
    bsp::cc13_scene2::array_resize_zero_fill<QuadNode14>(a, n,
        [](NativeArray<QuadNode14>& x, std::int32_t m) { quad_nodes_reserve_00ae9ac0(x, m); });
}

void quad_nodes_destroy_00aea8a0(NativeArray<QuadNode14>& a) {
    quad_nodes_resize_00aea590(a, 0);
    ::operator delete(a.data);
}

void* terrain_quadtree_construct_00aea8c0(void* tree) {
    wr<std::uint32_t>(tree, 0, 0x00D5D6A4);
    for (std::uint32_t off : {0x3Cu, 0x40u, 0x44u}) wr<std::uint32_t>(tree, off, 0);
    return tree;
}

void terrain_quadtree_destroy_00aea8e0(void* tree) {
    wr<std::uint32_t>(tree, 0, 0x00D5D6A4);
    quad_nodes_destroy_00aea8a0(*reinterpret_cast<NativeArray<QuadNode14>*>(at(tree, 0x3C)));
}

void terrain_quadtree_set_bounds_00aea900(void* tree, void* field, const float min[3], const float max[3],
    std::int32_t depth) {
    wr<void*>(tree, 0x48, field);
    for (std::uint32_t k = 0; k < 3; ++k) {
        wr<float>(tree, 4 + 4 * k, min[k]);
        wr<float>(tree, 0x10 + 4 * k, max[k]);
    }
    wr<std::int32_t>(tree, 0x34, depth);
    wr<std::int32_t>(tree, 0x38, 1 << (depth & 0x1F));
    for (std::uint32_t k = 0; k < 3; ++k)
        wr<float>(tree, 0x1C + 4 * k, f(static_cast<double>(rd<float>(tree, 0x10 + 4 * k)) - rd<float>(tree, 4 + 4 * k)));
    const double n = static_cast<float>(rd<std::int32_t>(tree, 0x38));
    for (std::uint32_t k = 0; k < 3; ++k) wr<float>(tree, 0x28 + 4 * k, f(rd<float>(tree, 0x1C + 4 * k) / n));
    quad_nodes_resize_00aea590(*reinterpret_cast<NativeArray<QuadNode14>*>(at(tree, 0x3C)), 0);
}

void terrain_quadtree_tile_range_00ae9b50(const void* tree, float out[2], std::int32_t x, std::int32_t y) {
    const void* field = rd<const void*>(tree, 0x48);
    if (x < rd<std::int32_t>(field, 0x38) && y < rd<std::int32_t>(field, 0x3C)) {
        const void* tile = rd<void* const*>(field, 0x40)[rd<std::int32_t>(field, 0x38) * y + x];
        if (tile != nullptr) {
            out[0] = rd<float>(tile, 0x14);
            out[1] = rd<float>(tile, 0x18);
            return;
        }
    }
    out[0] = kNoDataHeight_00D7A240;
    out[1] = 1000.0f;
}

void* terrain_weak_link_construct_00ae9870(void* link, void* ref, TerrainHost& host, WeakOwnerHost& weak) {
    weak.weak_owner_construct(link);
    wr<std::uint32_t>(link, 0, 0x00D5D660);
    wr<std::uint32_t>(link, 0xC, 0);
    assign_retained(link, 0xC, ref, host);
    wr<float>(link, 0x10, kNoDataHeight_00D7A240);
    wr<float>(link, 0x14, 1e10f);
    wr<float>(link, 0x18, 300.0f);
    wr<float>(link, 0x1C, 9.375f);
    return link;
}

void terrain_weak_link_destruct_00ae9930(void* link, TerrainHost& host, WeakOwnerHost& weak) {
    wr<std::uint32_t>(link, 0, 0x00D5D660);
    release_ref_slot(reinterpret_cast<void**>(at(link, 0xC)), host);
    weak.weak_owner_destroy(link);
}

void bush_quad_node_destroy_children_00ae09d0(void* node, TerrainHost& host) {
    for (std::uint32_t off = 0x1C; off <= 0x28; off += 4) {
        void* child = rd<void*>(node, off);
        if (child == nullptr) continue;
        bush_quad_node_destroy_children_00ae09d0(child, host);
        host.free(child);
        wr<std::uint32_t>(node, off, 0);
    }
}

void bush_quad_node_delete_owned_00ae08e0(void** slot, TerrainHost& host) {
    void* node = *slot;
    if (node == nullptr) return;
    bush_quad_node_destroy_children_00ae09d0(node, host);
    host.free(node);
    *slot = nullptr;
}

void node_ref_release_00ae0910(void** slot, TerrainHost& host) {
    if (*slot == nullptr) return;
    host.node_unlink_and_release_00b6d8f0(*slot);
    *slot = nullptr;
}

void tree_quad_node_destroy_children_00ae4f70(void* node, TerrainHost& host) {
    if (rd<std::int32_t>(node, 0x24) > 0) {
        for (std::uint32_t off = 0x2C; off <= 0x38; off += 4) {
            void* child = rd<void*>(node, off);
            if (child == nullptr) continue;
            tree_quad_node_destroy_children_00ae4f70(child, host);
            host.free(child);
            wr<std::uint32_t>(node, off, 0);
        }
    }
    release_ref_slot(reinterpret_cast<void**>(at(node, 0x54)), host);
    if (void* p = rd<void*>(node, 0x14)) host.free(p);
    for (std::uint32_t off : {0x14u, 0x18u, 0x1Cu}) wr<std::uint32_t>(node, off, 0);
}

void tree_quad_node_delete_00ae2ec0(void** slot, TerrainHost& host) {
    void* node = *slot;
    if (node == nullptr) return;
    tree_quad_node_destroy_children_00ae4f70(node, host);
    host.free(node);
    *slot = nullptr;
}

bool pointer_array_erase_value_swap_last_00ae4dd0(PtrArray& a, const void* value) {
    for (std::int32_t i = 0; i < a.count; ++i) {
        if (a.data[i] != value) continue;
        if (i != a.count - 1) a.data[i] = a.data[a.count - 1];
        --a.count;
        return true;
    }
    return false;
}

void bush_group_set_enabled_00ae0710(void* group, std::uint8_t enabled, TerrainHost& host) {
    wr<std::uint8_t>(group, 0x40, enabled);
    if (void* mesh = rd<void*>(group, 0x2C)) host.mesh_clear_draw_section_owners_00b73c10(mesh);
}

void tree_group_set_enabled_00ae2c80(void* group, std::uint8_t enabled, TerrainHost& host) {
    wr<std::uint8_t>(group, 0x38, enabled);
    host.mesh_clear_draw_section_owners_00b73c10(rd<void*>(group, 0x28));
}

bool generated_model_finder_visit_00ae2cb0(void* finder, void* node, TerrainHost& host) {
    if (host.node_is_generated_model(node)) {
        wr<void*>(finder, 8, node);
        return true;
    }
    for (void* child = rd<void*>(node, 0x34); child != nullptr; child = rd<void*>(child, 0x3C)) {
        if (host.node_is_generated_model(child)) {
            wr<void*>(finder, 8, child);
            return true;
        }
    }
    for (void* child = rd<void*>(node, 0x34); child != nullptr; child = rd<void*>(child, 0x3C))
        if (generated_model_finder_visit_00ae2cb0(finder, child, host)) return true;
    wr<void*>(finder, 8, nullptr);
    return false;
}

void tree_group_destruct_00ae4c20(void* group, TerrainHost& host) {
    tree_quad_node_delete_00ae2ec0(reinterpret_cast<void**>(at(group, 0x10)), host);
    node_ref_release_00ae0910(reinterpret_cast<void**>(at(group, 0x18)), host);
    void* type = rd<void*>(group, 4);
    wr<std::int32_t>(type, 0x44, rd<std::int32_t>(type, 0x44) - 1);
    for (std::uint32_t off : {0x1Cu, 0x20u, 0x24u, 0x28u, 0x34u}) release_ref_slot(reinterpret_cast<void**>(at(group, off)), host);
}

void bush_group_destruct_00ae1c80(void* group, TerrainHost& host) {
    for (std::uint32_t off : {0x50u, 0x28u, 0x2Cu, 0x3Cu, 0x38u}) release_ref_slot(reinterpret_cast<void**>(at(group, off)), host);
    node_ref_release_00ae0910(reinterpret_cast<void**>(at(group, 0x20)), host);
    bush_quad_node_delete_owned_00ae08e0(reinterpret_cast<void**>(at(group, 0x18)), host);
    while (rd<std::int32_t>(group, 0x6C) > 0) {
        wr<std::int32_t>(group, 0x6C, rd<std::int32_t>(group, 0x6C) - 1);
        host.cs_leave(at(group, 0x54));
    }
    host.cs_delete(at(group, 0x54));
    host.shadow_resize_pointer_vector_00ae19b0(at(group, 0x44), 0);
    host.free(rd<void*>(group, 0x44));
    release_native_string(at(group, 0x0C), host);
    release_native_string(at(group, 0x04), host);
}

namespace {
void copy_string_field(void* dst, const void* src, TerrainHost& host) {
    if (dst != src) host.native_string_assign_0041dd40(dst, rd<const void*>(src, 4), rd<std::uint32_t>(src, 0));
}
}  // namespace

void* foliage_type_record_construct_00ae6590(void* record, const FoliageTypeRecordArgs& args, TerrainHost& host) {
    for (std::uint32_t off = 0xC; off <= 0x20; off += 4) wr<std::uint32_t>(record, off, 0);
    wr<std::uint8_t>(record, 0, args.flag);
    wr<std::uint32_t>(record, 4, args.a);
    copy_string_field(at(record, 0xC), args.s1, host);
    wr<std::uint32_t>(record, 8, args.b);
    copy_string_field(at(record, 0x14), args.s2, host);
    copy_string_field(at(record, 0x1C), args.s3, host);
    wr<std::uint32_t>(record, 0x24, args.c);
    wr<std::uint32_t>(record, 0x28, args.d);
    wr<std::uint32_t>(record, 0x2C, args.e);
    release_native_string(args.s1, host);
    release_native_string(args.s2, host);
    release_native_string(args.s3, host);
    return record;
}

void* foliage_type_record_copy_00ae6720(void* record, const void* source, TerrainHost& host) {
    wr<std::uint8_t>(record, 0, rd<std::uint8_t>(source, 0));
    wr<std::uint32_t>(record, 4, rd<std::uint32_t>(source, 4));
    wr<std::uint32_t>(record, 8, rd<std::uint32_t>(source, 8));
    for (std::uint32_t off : {0xCu, 0x14u, 0x1Cu}) {
        wr<std::uint32_t>(record, off, 0);
        wr<std::uint32_t>(record, off + 4, 0);
        copy_string_field(at(record, off), static_cast<const std::uint8_t*>(source) + off, host);
    }
    for (std::uint32_t off : {0x24u, 0x28u, 0x2Cu}) wr<std::uint32_t>(record, off, rd<std::uint32_t>(source, off));
    return record;
}

void* foliage_type_record_assign_00ae6840(void* record, const void* source, TerrainHost& host) {
    wr<std::uint8_t>(record, 0, rd<std::uint8_t>(source, 0));
    wr<std::uint32_t>(record, 4, rd<std::uint32_t>(source, 4));
    wr<std::uint32_t>(record, 8, rd<std::uint32_t>(source, 8));
    for (std::uint32_t off : {0xCu, 0x14u, 0x1Cu})
        copy_string_field(at(record, off), static_cast<const std::uint8_t*>(source) + off, host);
    for (std::uint32_t off : {0x24u, 0x28u, 0x2Cu}) wr<std::uint32_t>(record, off, rd<std::uint32_t>(source, off));
    return record;
}

void* foliage_type_group_copy_00ae7470(void* group, const void* source, TerrainHost& host,
    const std::function<void(void* dst_vector, const void* src_vector)>& vector_copy_00ae7030) {
    wr<std::uint32_t>(group, 0, 0);
    wr<std::uint32_t>(group, 4, 0);
    copy_string_field(group, source, host);
    vector_copy_00ae7030(at(group, 8), static_cast<const std::uint8_t*>(source) + 8);
    return group;
}

void foliage_type_group_construct_empty_00ae76c0(void* group) {
    for (std::uint32_t off : {0x0u, 0x4u, 0xCu, 0x10u, 0x14u}) wr<std::uint32_t>(group, off, 0);
}

void terrain_float_layer_quantize16_00aed730(void* block, std::uint16_t* out, std::int32_t i, std::int32_t j,
    TerrainHost& host) {
    void* owner = rd<void*>(block, 8);
    _InterlockedIncrement(reinterpret_cast<volatile long*>(at(owner, 4)));
    const float v = rd<const float*>(block, 0x34)[rd<std::int32_t>(block, 0x2C) * j + i];
    const float lo = rd<float>(owner, 0x10), hi = rd<float>(owner, 0x14);
    float c;
    if (lo > v) c = lo;
    else if (v > hi) c = hi;
    else c = v;
    const float range = f(static_cast<double>(hi) - lo);
    const double scaled = (static_cast<double>(lo) + c) / range * 65535.0;
    *out = static_cast<std::uint16_t>(static_cast<std::int32_t>(scaled));
    if (_InterlockedDecrement(reinterpret_cast<volatile long*>(at(owner, 4))) == 0) host.call_virtual(owner, 0);
}

void terrain_float_layer_quantize8_00aed820(const void* block, std::uint8_t* out, std::int32_t i, std::int32_t j) {
    const float v = rd<const float*>(block, 0x34)[rd<std::int32_t>(block, 0x2C) * j + i];
    float c;
    if (0.0f > v) c = 0.0f;
    else if (v > 1.0f) c = 1.0f;
    else c = v;
    *out = static_cast<std::uint8_t>(static_cast<std::int32_t>(static_cast<double>(c) * 255.0));
}

void terrain_block_write_chunk(std::uint32_t address, void* block, void* writer, ChunkHost& chunks,
    BlockHost& blocks) {
    if (address == 0x00AEDD60) {
        chunks.begin_chunk(writer, "F32");
        for (std::int32_t r = 0; r < rd<std::int32_t>(block, 0x30); ++r) {
            for (std::int32_t c = 0; c < rd<std::int32_t>(block, 0x2C); ++c) {
                float v = 0.0f;
                blocks.block_slot08_sample(block, &v, c, r);
                chunks.write_float(writer, v);
            }
        }
        chunks.end_chunk(writer, "F32");
        return;
    }
    std::uint32_t size = 0;
    const char* tag = nullptr;
    if (address == 0x00ADE670) { size = 2; tag = "U16"; }
    else if (address == 0x00AE0390) { size = 1; tag = "U8"; }
    else throw std::invalid_argument("not a block chunk writer");
    chunks.begin_chunk(writer, tag);
    chunks.write_float(writer, rd<float>(block, 0x2C));
    chunks.write_float(writer, rd<float>(block, 0x30));
    for (std::int32_t r = 0; r < rd<std::int32_t>(block, 0x3C); ++r)
        for (std::int32_t c = 0; c < rd<std::int32_t>(block, 0x38); ++c)
            chunks.write_bytes(writer, rd<const std::uint8_t*>(block, 0x40) + (rd<std::int32_t>(block, 0x38) * r + c) * size, size);
    chunks.end_chunk(writer, tag);
}

void terrain_tile_write_node_chunk_00ae0250(void* tile, void* writer, ChunkHost& chunks) {
    chunks.begin_chunk(writer, "NODE");
    chunks.write_dword(writer, rd<std::uint32_t>(tile, 0xC));
    chunks.write_dword(writer, rd<std::uint32_t>(tile, 0x10));
    chunks.block_slot14_write(rd<void*>(tile, 0x1C), writer);
    chunks.end_chunk(writer, "NODE");
}

void terrain_block_read_chunk(std::uint32_t address, void* block, void* node, ChunkHost& chunks) {
    std::uint32_t size = 0;
    if (address == 0x00ADC6C0) size = 2;
    else if (address == 0x00ADFBD0) size = 1;
    else throw std::invalid_argument("not a block chunk reader");
    wr<float>(block, 0x2C, chunks.node_read_float_00be99d0(node));
    const float range = chunks.node_read_float_00be99d0(node);
    wr<float>(block, 0x30, range);
    wr<float>(block, 0x34, f(1.0 / range));
    for (std::int32_t r = 0; r < rd<std::int32_t>(block, 0x3C); ++r) {
        for (std::int32_t c = 0; c < rd<std::int32_t>(block, 0x38); ++c) {
            std::uint32_t v = 0;
            chunks.node_read_raw_bytes(node, &v, size);
            std::memcpy(rd<std::uint8_t*>(block, 0x40) + (rd<std::int32_t>(block, 0x38) * r + c) * size, &v, size);
        }
    }
}

void* texture_atlas_find_by_substring_00aee150(void* atlas, const void* needle, AtlasHost& atlases) {
    for (std::int32_t i = 0; i < rd<std::int32_t>(atlas, 0x14); ++i) {
        void* texture = rd<void**>(atlas, 0x10)[i];
        const void* name = atlases.texture_name_00b33e40(texture);
        const char* hay = rd<const char*>(name, 4);
        const char* pin = rd<const char*>(needle, 4);
        if (hay == nullptr || pin == nullptr) continue;
        const char* p = atlases.strstr_00bf9440(hay, pin);
        if (p != nullptr && static_cast<std::int32_t>(p - hay) > -1) return texture;
    }
    return nullptr;
}

void texture_atlas_item_destruct_00aee220(void* item, TerrainHost& host) {
    release_native_string(at(item, 0xC), host);
    release_native_string(item, host);
}

void* texture_atlas_item_construct_00aee4d0(void* item, const void* name, void* texture, const void* descriptor,
    TerrainHost& host, AtlasHost& atlases) {
    for (std::uint32_t off : {0x0u, 0x4u, 0xCu, 0x10u}) wr<std::uint32_t>(item, off, 0);
    copy_string_field(item, descriptor, host);
    copy_string_field(at(item, 0xC), name, host);
    NativeStr dot{0, nullptr};
    host.native_string_assign_0041dd40(&dot, ".", 1);  // 00CE3A70
    const std::uint32_t index = atlases.reverse_find_00467cf0(at(item, 0xC), &dot, 0x7FFFFFFF);
    release_native_string(&dot, host);
    if (index != 0xFFFFFFFFu) {
        const std::uint32_t old_len = rd<std::uint32_t>(item, 0xC);
        host.native_string_resize_0041dd40(at(item, 0xC), index);
        if (old_len < rd<std::uint32_t>(item, 0xC))
            std::memset(rd<std::uint8_t*>(item, 0x10) + old_len, 0x20, rd<std::uint32_t>(item, 0xC) - old_len);
    }
    wr<void*>(item, 8, texture);
    return item;
}

void pointer_vector_push_back_min4_00ae16b0(PtrArray& a, void* value, const PtrReserve& reserve) {
    if (a.count == a.capacity) {
        std::int32_t n = a.capacity * 2;
        if (n < 5) n = 4;
        reserve(a, n);
    }
    a.data[a.count++] = value;
}

void* bush_group_model_scalar_deleting_dtor_00ae1680(void* model, std::uint8_t flags, AtlasHost& atlases) {
    wr<std::uint32_t>(model, 0, 0x00D5D4A0);
    atlases.model_destroy(model);
    if (flags & 1) atlases.bush_model_pool_free_00ae13d0(model);
    return model;
}

void particle_container_unused_slot0_00b05400(std::uint32_t) {}

std::uint32_t height_field_attach_foliage_00adac90(void* field, TerrainHost& host, AtlasHost& atlases) {
    void* foliage = host.read_global(0x00F8C210);
    wr<void*>(field, 0x5C, foliage);
    apply_field_setter(0x00AD56C0, foliage, rd<std::uint32_t>(field, 0xC));
    atlases.foliage_load_landscape_00ad92c0(foliage, field);
    return 0;
}

void bush_group_set_diffuse_factor_00ae22d0(void* group, float factor, GroupModelHost& models) {
    wr<float>(group, 0x24, factor);
    const float* source = reinterpret_cast<const float*>(at(group, 0x24));
    models.material_register_float_00b18b20(rd<void*>(group, 0x3C), "cDiffuseFactor", source);
    models.material_register_float_00b18b20(rd<void*>(group, 0x38), "cDiffuseFactor", source);
}

void bush_group_model_render_filter_00ae23f0(void* model, void* ctx, std::uint32_t a, std::uint32_t b,
    std::uint32_t c, TerrainHost& host, GroupModelHost& models) {
    void* view = rd<void*>(ctx, 8);
    if (rd<std::int32_t>(view, 0x198) == 2 && !models.foliage_bush_shadow_allowed_00ad5760(host.read_global(0x00F8C210)))
        return;
    if (void* owner = rd<void*>(model, 0x184)) models.render_slot20(models.bush_build_object_00ae1fb0(owner, view), ctx, a, b, c);
}

void* tree_group_instance_construct_00ae5070(void* instance, void* landscape, void* type, void* root,
    std::uint32_t extra, GroupModelHost& models) {
    wr<void*>(instance, 0, landscape);
    wr<void*>(instance, 4, type);
    void* stream = models.geometry_vertex_stream_00b73260(models.generated_model_geometry_00b74640(rd<void*>(type, 8)));
    wr<void*>(instance, 0x20, stream);
    _InterlockedIncrement(reinterpret_cast<volatile long*>(at(stream, 4)));
    wr<std::int32_t>(type, 0x44, rd<std::int32_t>(type, 0x44) + 1);
    wr<void*>(instance, 0x14, root);
    wr<std::uint32_t>(instance, 8, 0);
    wr<std::uint32_t>(instance, 0xC, extra);
    wr<void*>(instance, 0x18, models.scene_find_node_by_name_00b72270(rd<void*>(instance, 0x14), "TreeGroup"));
    wr<std::uint32_t>(instance, 0x10, 0);
    return instance;
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

// ---------------------------------------------------------------------------
// cc13-terrain2: ballistic extrema. Extended-precision chains are rounded at each FSTP float.
// cc13-fixture2 follow-up: 00AFB7F0 and 00AFBA40 have no reference in the image (rel32 and
// absolute scans are empty); as particle helpers they are modelled at the main thread's
// 24-bit precision control, each operation rounded to 24 significand bits in listing order
// (00AFB80F..27 (4*b)*b - (a*12)*c; 00AFB86E..BF (s - 2b)/(6a) + x0, (-2b - s)/(6a) + x0;
// 00AFB8E4..FA 6a*u + 2b; 00AFB90C..1A ((a*u + b)*u + c)*u + d).

namespace {
float ballistic_extremum(const float* curve, const float* other, bool maximum) noexcept {
    using namespace cc13x87;
    const float a = curve[7], b = curve[8], c = curve[9], d = curve[10], x0 = curve[0];
    const float disc = static_cast<float>(x87_sub24(x87_mul24(x87_mul24(4.0, b), b), x87_mul24(x87_mul24(a, 12.0), c)));
    const bool fallback = 0.0f > disc || a == 0.0f;
    if (!fallback) {
        const float s = static_cast<float>(std::sqrt(static_cast<double>(disc)));
        const double six_a = x87_mul24(a, 6.0);
        const float t1 = static_cast<float>(x87_add24(x87_div24(x87_sub24(s, x87_mul24(b, 2.0)), six_a), x0));
        const float t2 = static_cast<float>(x87_add24(x87_div24(x87_sub24(x87_mul24(b, -2.0), s), six_a), x0));
        for (const float t : {t1, t2}) {
            if (!(t > x0) || !(other[0] > t)) continue;
            const float u = static_cast<float>(x87_sub24(t, x0));
            const double slope = x87_add24(x87_mul24(six_a, u), x87_mul24(b, 2.0));
            if (maximum ? !(0.0 > slope) : !(slope > 0.0)) continue;
            return static_cast<float>(
                x87_add24(x87_mul24(x87_add24(x87_mul24(x87_add24(x87_mul24(a, u), b), u), c), u), d));
        }
    }
    const float y0 = curve[1], y1 = other[1];
    if (maximum) return y0 > y1 ? y0 : y1;
    return y1 > y0 ? y0 : y1;
}
} // namespace

float ballistic_max_height_00afb7f0(const float* curve, const float* other) noexcept {
    return ballistic_extremum(curve, other, true);
}

float ballistic_min_height_00afba40(const float* curve, const float* other) noexcept {
    return ballistic_extremum(curve, other, false);
}

// ===========================================================================
// cc13-left1: bodies left open by Pass 2. docs/CC13_LEFT_1.md.
// ===========================================================================
namespace {

void left_release(void* object) {
    auto* const count = reinterpret_cast<volatile long*>(at(object, 4));
    if (_InterlockedDecrement(count) == 0) {
        using Slot0 = void(__thiscall*)(void*);
        reinterpret_cast<Slot0>(rd<void*>(rd<void*>(object, 0), 0))(object);
    }
}

// The copy-construct schedule these bodies inline: zero the header; unless it is the source,
// resize to the source length (keeping) and copy that many characters when nonzero.
void left_copy_string(void* destination, const void* source, TerrainHost& host) {
    wr<std::uint32_t>(destination, 0, 0);
    wr<void*>(destination, 4, nullptr);
    if (destination == source) return;
    const std::uint32_t length = rd<std::uint32_t>(source, 0);
    host.native_string_resize_0041dd40(destination, length);
    if (length != 0) std::memcpy(rd<void*>(destination, 4), rd<const void*>(source, 4), rd<std::uint32_t>(destination, 0));
}

void left_release_string(void* header, TerrainHost& host) {
    void* const chars = rd<void*>(header, 4);
    if (chars != nullptr) host.return_string_block_00bd1510(chars, rd<std::uint32_t>(header, 0) + 1u);
}

// The inline 4-byte {data, count, capacity} teardown of the registry destructor.
void left_teardown_dword_vector(void* vector, TerrainHost& host) {
    std::int32_t capacity = rd<std::int32_t>(vector, 8);
    if (capacity < 0 && capacity < 1) {
        auto* const fresh = static_cast<std::uint32_t*>(host.operator_new(4));
        const auto* const old = rd<std::uint32_t*>(vector, 0);
        for (std::int32_t i = 0; i < rd<std::int32_t>(vector, 4); ++i) {
            if (fresh + i != nullptr) fresh[i] = old[i];
        }
        host.free(rd<void*>(vector, 0));
        wr<void*>(vector, 0, fresh);
        wr<std::int32_t>(vector, 8, 1);
    }
    for (std::int32_t i = rd<std::int32_t>(vector, 4); i < 0; ++i) {
        auto* const slot = rd<std::uint32_t*>(vector, 0) + i;
        if (slot != nullptr) *slot = 0;
    }
    while (rd<std::int32_t>(vector, 4) > 0) wr<std::int32_t>(vector, 4, rd<std::int32_t>(vector, 4) - 1);
    wr<std::int32_t>(vector, 4, 0);
    host.free(rd<void*>(vector, 0));
}

}  // namespace

void texture_atlas_unload_00aefa30(void* registry, const void* name, TerrainHost& host, TerrainLeftHost& left) {
    auto** p = rd<void**>(registry, 4);
    while (p != rd<void**>(registry, 4) + rd<std::int32_t>(registry, 8)) {
        if (!left.atlas_split_name_matches_00aef3c0(name, *p)) {
            ++p;
            continue;
        }
        std::int32_t i = 0;
        while (i < rd<std::int32_t>(registry, 0x14)) {
            auto** const textures = rd<void**>(registry, 0x10);
            void* const texture = textures[i];
            if (texture == rd<void*>(*p, 8)) {
                left_release(texture);
                for (std::int32_t k = i; k < rd<std::int32_t>(registry, 0x14) - 1; ++k) {
                    rd<void**>(registry, 0x10)[k] = rd<void**>(registry, 0x10)[k + 1];
                }
                wr<std::int32_t>(registry, 0x14, rd<std::int32_t>(registry, 0x14) - 1);
            } else {
                ++i;
            }
        }
        void* const item = *p;
        if (item != nullptr) {
            texture_atlas_item_destruct_00aee220(item, host);
            host.free(item);
        }
        void** const last = rd<void**>(registry, 4) + rd<std::int32_t>(registry, 8) - 1;
        if (p != last) *p = *last;
        wr<std::int32_t>(registry, 8, rd<std::int32_t>(registry, 8) - 1);
    }
}

void texture_atlas_register_descriptor_00aef280(const void* name, void* vfs_0109ceec, void* manager_00f8c26c,
    const char* empty_00f8c270, TerrainHost& host, TerrainLeftHost& left) {
    void* const block = host.operator_new(0x1C);
    void* const buffer = block != nullptr ? left.text_buffer_construct_00af5600(block) : nullptr;
    alignas(4) std::uint8_t copy[8];
    left_copy_string(copy, name, host);
    const bool found = left.vfs_resolve_existing_name_00bdf4c0(vfs_0109ceec, copy);
    const char* path = rd<const char*>(name, 4);
    if (path == nullptr) path = empty_00f8c270;
    if (!found) {
        left.log_004254b0("Atlas file not found: %s", path);
    } else {
        left.text_buffer_load_file_00af5850(buffer, path);
        left.texture_atlas_parse_buffer_00aeeaf0(manager_00f8c26c, buffer);
    }
    if (buffer != nullptr) {
        left.text_buffer_destroy_00af5620(buffer);
        host.free(buffer);
    }
    left_release_string(copy, host);
}

void texture_atlas_load_directory_00aeff40(void* directory, void* vfs_0109ceec, void* manager_00f8c26c,
    const char* empty_00f8c270, TerrainHost& host, TerrainLeftHost& left) {
    alignas(4) std::uint8_t extension[8] = {};
    host.native_string_assign_0041dd40(extension, "ats", 3);  // resize(3, keep) + copy of length + 1
    alignas(4) std::uint8_t list[0x0C] = {};
    left.vfs_enumerate_resource_list_00886280(vfs_0109ceec, list, directory, extension, 1);
    left_release_string(extension, host);
    while (rd<std::uint32_t>(list, 8) != 0) {
        alignas(4) std::uint8_t entry[8] = {};
        void* const popped = left.pooled_string_list_pop_front_00557a90(list, entry);
        texture_atlas_register_descriptor_00aef280(popped, vfs_0109ceec, manager_00f8c26c, empty_00f8c270, host, left);
        left_release_string(entry, host);
    }
    left.pooled_string_list_clear_storage_004d05e0(list);
    host.free(rd<void*>(list, 4));
}

void texture_atlas_registry_destruct_00aef090(void* registry, TerrainHost& host, TerrainLeftHost& left) {
    wr<std::uint32_t>(registry, 0, 0x00CFEAE8u);
    for (std::int32_t i = 0; i < rd<std::int32_t>(registry, 8); ++i) {
        void* const item = rd<void**>(registry, 4)[i];
        if (item != nullptr) {
            texture_atlas_item_destruct_00aee220(item, host);
            host.free(item);
        }
    }
    for (std::int32_t i = 0; i < rd<std::int32_t>(registry, 0x14); ++i) left_release(rd<void**>(registry, 0x10)[i]);
    left_teardown_dword_vector(at(registry, 0x10), host);
    left_teardown_dword_vector(at(registry, 4), host);
    left.singleton_holder_destruct_007375e0(registry);
}

namespace {

void* left_geometry(void* model) { return rd<void*>(model, 0x180); }                       // 00B74640
void* left_element(void* geometry, std::uint32_t i) { return rd<void**>(geometry, 0x54)[i]; }  // 00B732C0

}  // namespace

void foliage_group_manager_create_floating_particles_00af0f20(void* manager, void* particle_resources_00f8c280,
    TerrainHost& host, TerrainLeftHost& left) {
    wr<void*>(manager, 0x24, nullptr);
    wr<std::uint32_t>(manager, 0x2C, 0x3F800000u);  // 00D7A24C 1.0f
    alignas(4) std::uint8_t shader[8], geometry[8], name[8];
    host.native_string_assign_literal_0041e870(shader, "ParticleFloating.mshd");
    host.native_string_assign_literal_0041e870(geometry, "ParticleFloating.mvfm");
    host.native_string_assign_literal_0041e870(name, "FloatingParticles");
    wr<void*>(manager, 0x24, left.create_generated_model_00b4c700(name, geometry, shader, 4, 0, 0, 0));
    left_release_string(name, host);
    left_release_string(geometry, host);
    left_release_string(shader, host);
    void* const resources = rd<void*>(particle_resources_00f8c280, 0x24);  // 00AF1130
    void* const texture = rd<std::int16_t>(resources, 0x34) > 0 ? rd<void*>(resources, 0x10) : nullptr;
    void* const element = left_element(left_geometry(rd<void*>(manager, 0x24)), 0);
    left.material_set_texture_slot_00b189f0(rd<void*>(element, 0x20), 0, texture);
    void* const indices = left.particle_index_stream_00af10a0(particle_resources_00f8c280);
    left.geometry_set_index_stream_00b73b70(left_geometry(rd<void*>(manager, 0x24)), indices);
}

void* bush_group_type_construct_00ae1e10(void* type, void* landscape, std::uint32_t name_a_length, void* name_a_data,
    std::uint32_t name_b_length, void* name_b_data, void* owner, TerrainHost& host, TerrainLeftHost& left) {
    for (std::uint32_t offset : {0x04u, 0x08u, 0x0Cu, 0x10u, 0x44u, 0x48u, 0x4Cu}) wr<std::uint32_t>(type, offset, 0);
    left.initialize_critical_section(at(type, 0x54));
    wr<std::uint32_t>(type, 0x6C, 0);
    wr<void*>(type, 0, landscape);
    wr<void*>(type, 0x1C, owner);
    // The by-value headers are never the destination, so both copies run (the self test is moot).
    host.native_string_resize_0041dd40(at(type, 4), name_a_length);
    if (name_a_length != 0) std::memcpy(rd<void*>(type, 8), name_a_data, rd<std::uint32_t>(type, 4));
    host.native_string_resize_0041dd40(at(type, 0x0C), name_b_length);
    if (name_b_length != 0) std::memcpy(rd<void*>(type, 0x10), name_b_data, rd<std::uint32_t>(type, 0x0C));
    wr<std::uint32_t>(type, 0x14, 0);
    alignas(4) std::uint8_t label[8] = {};
    host.native_string_assign_0041dd40(label, "BushGroup", 9);
    wr<void*>(type, 0x20, left.scene_find_node_by_name_00b72270(rd<void*>(type, 0x1C), label, 0));
    left_release_string(label, host);
    wr<std::uint32_t>(type, 0x2C, 0);
    wr<std::uint32_t>(type, 0x18, 0);
    wr<std::uint32_t>(type, 0x50, 0);
    if (name_a_data != nullptr) host.return_string_block_00bd1510(name_a_data, name_a_length + 1u);
    if (name_b_data != nullptr) host.return_string_block_00bd1510(name_b_data, name_b_length + 1u);
    return type;
}

std::int32_t bush_cell_cull_and_emit_lod_00ae1a10(void* cell, void* group, std::int32_t running,
                                                 void* foliage_system_00f8c210, TerrainLeftHost& left) {
    const TerrainVec3f centre{rd<float>(cell, 0), rd<float>(cell, 4), rd<float>(cell, 8)};
    const TerrainVec3f extent{rd<float>(cell, 0x0C), rd<float>(cell, 0x10), rd<float>(cell, 0x14)};
    std::vector<TerrainPlane> planes(rd<std::uint32_t>(group, 0));
    if (!planes.empty()) std::memcpy(planes.data(), at(group, 4), planes.size() * sizeof(TerrainPlane));
    const TerrainBoxVisibility visibility = terrain_box_classify_planes_00ae05e0(planes, centre, extent);
    if (visibility == TerrainBoxVisibility::Outside) return running;
    if (visibility != TerrainBoxVisibility::Inside && rd<std::uint32_t>(cell, 0x34) != 0) {
        // Each child's result already contains the running value it was given (the image adds it).
        std::int32_t sum = running;
        for (std::uint32_t offset : {0x1Cu, 0x20u, 0x24u, 0x28u}) {
            void* const child = rd<void*>(cell, offset);
            if (child != nullptr) {
                sum = static_cast<std::int32_t>(static_cast<std::uint32_t>(sum) + static_cast<std::uint32_t>(
                    bush_cell_cull_and_emit_lod_00ae1a10(child, group, sum, foliage_system_00f8c210, left)));
            }
        }
        return sum;
    }
    const std::uint32_t count = rd<std::uint32_t>(cell, 0x30) - rd<std::uint32_t>(cell, 0x2C) + 1u;
    void* const section = left.draw_section_create_00533fa0();
    const float d[3] = {f(static_cast<double>(centre.x) - rd<float>(group, 0x118)),
                        f(static_cast<double>(centre.y) - rd<float>(group, 0x11C)),
                        f(static_cast<double>(centre.z) - rd<float>(group, 0x120))};
    const float distance = f(left.vector3_length_00419440(d));
    const double threshold = left.foliage_system_scalar_0c_00ad5720(foliage_system_00f8c210);
    const bool far_away = static_cast<double>(distance) > threshold;  // FCOMIP; JBE takes unordered
    left.draw_section_set_material_00b864c0(section, rd<void*>(group, far_away ? 0x10C : 0x110));
    wr<std::uint32_t>(section, 0x08, 4);
    wr<std::uint32_t>(section, 0x0C, rd<std::uint32_t>(cell, 0x2C));
    wr<std::uint32_t>(section, 0x14, rd<std::uint32_t>(cell, 0x2C));
    wr<std::uint32_t>(section, 0x10, count);
    wr<std::uint32_t>(section, 0x18, count / 3u);
    wr<float>(section, 0x34, distance);
    left.draw_section_append_vertex_stream_00b85b80(section, rd<void*>(group, 0x104));
    left.draw_section_set_vertex_layout_00b86650(section, rd<void*>(group, 0x124));
    left.mesh_append_draw_section_00b73c60(rd<void*>(group, 0x108), section);
    left_release(section);
    if (rd<std::uint8_t>(group, 0x114) != 0) left.foliage_cell_draw_box_00ae0a50(cell, 0xFFFFFFFFu, 0.0f);
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(running) + count);
}

namespace {

std::uint32_t left_reload_bits(std::uint32_t bits) noexcept {  // FLD dword / FSTP dword
    if ((bits & 0x7F800000u) == 0x7F800000u && (bits & 0x007FFFFFu) != 0u) bits |= 0x00400000u;
    return bits;
}

bool left_view_keeps_far_plane(void* camera) {
    const std::int32_t view = rd<std::int32_t>(camera, 0x198);
    return view == 2 || view == 7;
}

}  // namespace

void* bush_group_model_build_bush_object_00ae1fb0(void* type, void* camera, void* foliage_system_00f8c210,
    TerrainHost& host, TerrainLeftHost& left) {
    void* const slot = left.model_allocate_raw_slot_00b74eb0(0x184);
    void* model = nullptr;
    if (slot != nullptr) {
        alignas(4) std::uint8_t name[8];
        host.native_string_assign_literal_0041e870(name, "BushOjjekt");
        model = left.generated_model_construct_00b75030(slot, name);
        left_release_string(name, host);
    }
    float saved_far = 0.0f;
    if (!left_view_keeps_far_plane(camera)) {
        saved_far = rd<float>(camera, 0x1D8);
        left.camera_set_far_plane_00b6fc10(camera, f(left.foliage_scalar_08_00ad5700(foliage_system_00f8c210)));
    }
    void* const mesh_slot = left.mesh_allocate_raw_slot_00b73b60(0xBC);
    void* const mesh = mesh_slot != nullptr ? left.mesh_construct_00b73d70(mesh_slot) : nullptr;
    const float minus_one = -1.0f;  // 00D7A260
    left.generated_model_set_geometry_00b75170(model, 0, mesh, minus_one, minus_one);
    left.mesh_set_vertex_stream_00b73bb0(mesh, 0, rd<void*>(type, 0x28));
    host.mesh_clear_draw_section_owners_00b73c10(mesh);
    alignas(4) std::uint8_t record[0x128] = {};
    const std::int32_t planes = left.camera_plane_count_00b6fdd0(camera);
    // The image has room for 16 planes before +104h and does not check; more would overwrite its frame.
    if (planes > 16) throw std::length_error("bush cull record holds at most 16 camera planes");
    wr<std::int32_t>(record, 0, planes);
    void* const set = left.camera_plane_set_00b70710(camera);
    for (std::int32_t i = 0; i < planes; ++i) {
        const float* const plane = left.plane_set_get_plane_00b656f0(set, i);
        std::uint32_t bits[4];
        std::memcpy(bits, plane, sizeof bits);
        for (std::uint32_t k = 0; k < 4; ++k) wr<std::uint32_t>(record, 4 + 16u * i + 4u * k, left_reload_bits(bits[k]));
        wr<float>(record, 4 + 16u * i + 12u, f(static_cast<double>(rd<float>(record, 4 + 16u * i + 12u)) - 50.0));  // 00CE3938
    }
    wr<void*>(record, 0x104, rd<void*>(type, 0x28));
    wr<void*>(record, 0x108, mesh);
    wr<void*>(record, 0x10C, rd<void*>(type, 0x3C));
    wr<void*>(record, 0x110, rd<void*>(type, 0x38));
    wr<std::uint8_t>(record, 0x114, 0);
    if ((rd<std::uint8_t>(camera, 0x5C) & 2u) == 0) left.node_refresh_world_matrix_00b6db70(camera);
    wr<std::uint32_t>(record, 0x118, rd<std::uint32_t>(camera, 0x120));
    wr<std::uint32_t>(record, 0x11C, rd<std::uint32_t>(camera, 0x124));
    wr<std::uint32_t>(record, 0x120, rd<std::uint32_t>(camera, 0x128));
    wr<void*>(record, 0x124, rd<void*>(type, 0x50));
    wr<std::int32_t>(type, 0x34, bush_cell_cull_and_emit_lod_00ae1a10(rd<void*>(type, 0x18), record, 0,
                                                                      foliage_system_00f8c210, left));
    if (rd<std::uint8_t>(type, 0x40) == 0) host.mesh_clear_draw_section_owners_00b73c10(mesh);
    if (mesh != nullptr) left_release(mesh);
    void* const node = rd<void*>(type, 0x20);
    if ((rd<std::uint8_t>(node, 0x5C) & 2u) == 0) left.node_refresh_world_matrix_00b6db70(node);
    using SetTransform = void(__thiscall*)(void*, void*);
    reinterpret_cast<SetTransform>(rd<void*>(rd<void*>(model, 0), 0x34))(model, at(node, 0xF0));
    void* const section = at(type, 0x54);
    left.enter_critical_section(section);
    wr<std::int32_t>(type, 0x6C, rd<std::int32_t>(type, 0x6C) + 1);
    if (rd<std::int32_t>(type, 0x48) == rd<std::int32_t>(type, 0x4C)) {
        std::int32_t capacity = rd<std::int32_t>(type, 0x4C) * 2;
        if (capacity <= 4) capacity = 4;
        left.shadow_reserve_pointer_vector_00ae1440(at(type, 0x44), capacity);
    }
    void** const end = rd<void**>(type, 0x44) + rd<std::int32_t>(type, 0x48);
    if (end != nullptr) *end = model;
    wr<std::int32_t>(type, 0x48, rd<std::int32_t>(type, 0x48) + 1);
    wr<std::int32_t>(type, 0x6C, rd<std::int32_t>(type, 0x6C) - 1);
    left.leave_critical_section(section);
    if (!left_view_keeps_far_plane(camera)) left.camera_set_far_plane_00b6fc10(camera, saved_far);
    return model;
}

void height_field_dump_height_tga_00adc1c0(void* field, const void* file, TerrainHost& host, TerrainLeftHost& left) {
    constexpr std::int32_t k = 32;  // 00D5D658
    void* const block = host.operator_new(0x1C);
    void* const image = block != nullptr
        ? left.ram_image_construct_00befbb0(block, k * rd<std::int32_t>(field, 0x38) + 1, k * rd<std::int32_t>(field, 0x3C) + 1, 1, 0x32)
        : nullptr;
    const std::int32_t stride = rd<std::int32_t>(field, 0x38) * k + 1;
    std::uint8_t* const data = left.ram_image_data_00befb60(image);
    left.height_field_compute_range_00adbd00(field);
    const auto clamp_low = [](float value) { return value > 0.0f ? value : 0.0f; };  // COMISS / JBE
    const float range = f(static_cast<double>(rd<float>(field, 0x14)) - clamp_low(rd<float>(field, 0x10)));
    for (std::uint32_t row = 0; row < rd<std::uint32_t>(field, 0x3C); ++row) {
        for (std::uint32_t column = 0; column < rd<std::uint32_t>(field, 0x38); ++column) {
            void* const tile = rd<void**>(field, 0x40)[rd<std::uint32_t>(field, 0x38) * row + column];
            const auto pixel = [&](std::int32_t u, std::int32_t v) -> std::uint8_t& {
                return data[k * static_cast<std::int32_t>(column) + (k * static_cast<std::int32_t>(row) + v) * stride + u];
            };
            if (tile == nullptr) {
                for (std::int32_t v = 0; v < 0x21; ++v)
                    for (std::int32_t u = 0; u < 0x21; ++u) pixel(u, v) = 0;
                continue;
            }
            void* const samples = rd<void*>(tile, 0x1C);  // 00ADEA70
            for (std::int32_t v = 0; v < 0x21; ++v) {
                for (std::int32_t u = 0; u < 0x21; ++u) {
                    float sample = 0.0f;
                    using Sample = void(__thiscall*)(void*, float*, std::int32_t, std::int32_t);
                    reinterpret_cast<Sample>(rd<void*>(rd<void*>(samples, 0), 8))(samples, &sample, u, v);
                    if (!(sample > 0.0f)) sample = 0.0f;  // COMISS / JA keeps
                    const float lo = clamp_low(rd<float>(field, 0x10));
                    const float scaled = f((static_cast<double>(sample) - lo) / range * 255.0);  // 00CE4B48
                    pixel(u, v) = static_cast<std::uint8_t>(static_cast<std::int32_t>(scaled));  // CVTTSS2SI
                }
            }
        }
    }
    left.ram_image_save_tga_00bf0050(file, image);
    left_release(image);
}

void height_field_stamp_byte_at_world_00adb6f0(void* field, float x, float z, float h, HeightFieldHost& fields) {
    const float inv = f(1.0 / rd<float>(field, 0x1C));
    std::int32_t i = static_cast<std::int32_t>(f(static_cast<double>(inv) * x));  // _ftol2 truncates
    std::int32_t j = static_cast<std::int32_t>(f(static_cast<double>(inv) * z));
    const std::int32_t wide = fields.slot10_samples_wide(field);
    if (i <= 0) i = 0;
    if (i >= wide) i = wide;
    const std::int32_t deep = fields.slot14_samples_deep(field);
    if (j <= 0) j = 0;
    if (j >= deep) j = deep;
    std::int32_t tx = 0, tz = 0, lx = 0, lz = 0;
    fields.slot44_split_index(field, &tx, &tz, &lx, &lz, i, j);
    void* const tile = rd<void**>(field, 0x40)[rd<std::int32_t>(field, 0x38) * tz + tx];
    if (tile == nullptr) return;
    const float height = h > 0.0f ? h : 0.0f;  // COMISS / JBE: NaN gives 0
    const float hf = f(static_cast<double>(height) / 60.0 * 255.0);  // 00CE3D68, 00CE4B48
    const float chosen = 255.0 > static_cast<double>(hf) ? hf : 255.0f;  // FCOMPI / JBE; 00CEDD44
    const auto value = static_cast<std::uint8_t>(static_cast<std::int32_t>(chosen));  // CVTTSS2SI, low byte
    void* const layer = rd<void*>(tile, 0x1C);  // 00ADEA70, re-read before each stamp in the image
    byte_layer_stamp_max_00adb160(layer, lx, lz, value);
    byte_layer_stamp_max_00adb160(rd<void*>(tile, 0x1C), lx + 1, lz, value);
    byte_layer_stamp_max_00adb160(rd<void*>(tile, 0x1C), lx + 1, lz + 1, value);
    byte_layer_stamp_max_00adb160(rd<void*>(tile, 0x1C), lx, lz + 1, value);
}

void* terrain_tile_grid_get_or_create_tile_00adb980(void* grid, std::int32_t x, std::int32_t y,
    HeightFieldHost& fields, TerrainHost& host, BlockHost& blocks, TerrainLeftHost& left) {
    void*& existing = rd<void**>(grid, 0x40)[rd<std::int32_t>(grid, 0x38) * y + x];
    if (existing != nullptr) return existing;
    void* const tile = fields.create_tile(grid, x, y);
    void* const memory_a = host.operator_new(0x38);
    void* const block_a = memory_a != nullptr ? terrain_block_construct(0x00AEDAC0, memory_a, 0x21, 0x21, host) : nullptr;
    terrain_tile_set_block_00adf8b0(tile, block_a, host, blocks);
    left_release(block_a);
    void* const memory_b = host.operator_new(0x38);
    void* const block_b = memory_b != nullptr ? terrain_block_construct(0x00AEDAC0, memory_b, 0x21, 0x21, host) : nullptr;
    terrain_tile_set_second_block_00adf920(tile, block_b, host);
    left_release(block_b);
    const float g = rd<float>(grid, 0x18);
    float low = 0.0f, high = 0.0f;
    std::uint32_t low_bits = 0x501502F9u;  // 00CE4970 1.0e10f
    std::memcpy(&low, &low_bits, 4);
    using SetSample = void(__thiscall*)(void*, float, std::int32_t, std::int32_t);
    for (std::int32_t j = 0; j < 0x21; ++j) {
        for (std::int32_t i = 0; i < 0x21; ++i) {
            reinterpret_cast<SetSample>(rd<void*>(rd<void*>(block_a, 0), 0x1C))(block_a, 0.0f, i, j);
            if (!(0.0f > low)) low = 0.0f;   // COMISS 0, m / JA
            high = low > 0.0f ? low : 0.0f;  // COMISS m, 0 / JBE
            reinterpret_cast<SetSample>(rd<void*>(rd<void*>(block_b, 0), 0x1C))(block_b, 0.0f, i, j);
        }
    }
    terrain_tile_refresh_height_range_00adeb00(tile, blocks);
    using Slot = void(__thiscall*)(void*);
    reinterpret_cast<Slot>(rd<void*>(rd<void*>(tile, 0), 8))(tile);
    void* const model = rd<void*>(tile, 0x24);  // 00ADEAF0
    const auto refresh = [&] {
        if ((rd<std::uint8_t>(model, 0x5C) & 2u) == 0) fields.refresh_world_matrix_00b6db70(model);
    };
    refresh();
    const float sx = rd<float>(model, 0xF0);
    refresh();
    const float sy = rd<float>(model, 0x104);
    refresh();
    const float sz = rd<float>(model, 0x118);
    const TerrainVec3f box_min{f(0.0 / sx), f(static_cast<double>(low) / sy), f(0.0 / sz)};
    const TerrainVec3f box_max{f(static_cast<double>(g) / sx), f(static_cast<double>(high) / sy), f(static_cast<double>(g) / sz)};
    const TerrainSphere sphere = terrain_box_bounding_sphere_00ada600(box_min, box_max);
    const float wider = sx > sy ? sx : sy;          // FCOMPI / JBE
    const float widest = wider > sz ? wider : sz;   // FCOMPI / JA
    float bounds[4] = {sphere.centre.x, sphere.centre.y, sphere.centre.z, f(static_cast<double>(rd<float>(grid, 0x18)) / widest)};
    left.generated_model_set_local_bounds_00b74390(model, bounds);
    reinterpret_cast<Slot>(rd<void*>(rd<void*>(model, 0), 0x48))(model);
    rd<void**>(grid, 0x40)[rd<std::int32_t>(grid, 0x38) * y + x] = tile;
    return tile;
}

// ===========================================================================
// cc13-left2: bodies left open by cc13-left1. docs/CC13_LEFT_2.md.
// ===========================================================================
namespace {

void l2_release(void* object) {
    auto* const count = reinterpret_cast<volatile long*>(at(object, 4));
    if (_InterlockedDecrement(count) == 0) {
        using Slot0 = void(__thiscall*)(void*);
        reinterpret_cast<Slot0>(rd<void*>(rd<void*>(object, 0), 0))(object);
    }
}

template <typename R, typename... A>
R l2_vcall(void* object, std::uint32_t slot, A... args) {
    using Fn = R(__thiscall*)(void*, A...);
    return reinterpret_cast<Fn>(rd<void*>(rd<void*>(object, 0), slot))(object, args...);
}

// A pooled temporary built by 0041E870 and released (length + 1) at scope end.
class L2Temporary {
public:
    L2Temporary(const char* text, TerrainHost& host) : host_(host) { host_.native_string_assign_literal_0041e870(header_, text); }
    ~L2Temporary() {
        if (header_[1] != 0) host_.return_string_block_00bd1510(reinterpret_cast<void*>(header_[1]), header_[0] + 1u);
    }
    L2Temporary(const L2Temporary&) = delete;
    L2Temporary& operator=(const L2Temporary&) = delete;
    void* get() noexcept { return header_; }

private:
    TerrainHost& host_;
    std::uint32_t header_[2]{};
};

// The inlined copy of a string header into another unless they are the same (resize keeping, then copy
// the source length when nonzero).
void l2_copy_into(void* destination, const void* source, TerrainHost& host) {
    if (destination == source) return;
    const std::uint32_t length = rd<std::uint32_t>(source, 0);
    host.native_string_resize_0041dd40(destination, length);
    if (rd<std::uint32_t>(source, 0) != 0) {
        std::memcpy(rd<void*>(destination, 4), rd<const void*>(source, 4), rd<std::uint32_t>(destination, 0));
    }
}

using L2Vec3 = std::array<float, 3>;
const CameraMatrix& l2_matrix(const void* p) { return *static_cast<const CameraMatrix*>(p); }

void* l2_refreshed_node(void* node, TerrainLeftHost& left) {
    if ((rd<std::uint8_t>(node, 0x5C) & 2u) == 0) left.node_refresh_world_matrix_00b6db70(node);
    return node;
}

void* l2_geometry(void* model) { return rd<void*>(model, 0x180); }  // 00B74640, its argument unused
void* l2_element(void* geometry, std::uint32_t i) { return rd<void**>(geometry, 0x54)[i]; }  // 00B732C0

}  // namespace

void particle_emitter_draw_debug_axes_00afdcf0(void* emitter, TerrainHost& host, TerrainLeft2Host& left) {
    if (rd<std::uint8_t>(rd<void*>(emitter, 0xA0), 0x7C) == 0) return;
    void* node = rd<void*>(emitter, 0xA4);
    L2Vec3 origin;
    if (rd<std::uint8_t>(node, 0x1B0) != 0) {
        l2_refreshed_node(node, left);
        transform_point_004142e0(*static_cast<const L2Vec3*>(emitter), l2_matrix(at(node, 0xF0)), origin);
    } else {
        origin = *static_cast<const L2Vec3*>(emitter);
    }
    const L2Vec3 unit_x{1.0f, 0.0f, 0.0f}, unit_y{0.0f, 1.0f, 0.0f}, unit_z{0.0f, 0.0f, 1.0f};
    CameraMatrix m;
    node = rd<void*>(emitter, 0xA4);
    if (rd<std::uint8_t>(node, 0x1B0) != 0) {
        l2_refreshed_node(node, left);
        CameraMatrix product;
        multiply_camera_matrices_00413920(product, l2_matrix(left.particle_record_matrix_00afda80(emitter)),
                                          l2_matrix(at(node, 0xF0)));
        copy_camera_matrix_004134f0(m, product);
    } else {
        copy_camera_matrix_004134f0(m, l2_matrix(left.particle_record_matrix_00afda80(emitter)));
    }
    const auto axis = [&m](const L2Vec3& unit) {
        L2Vec3 p;
        transform_point_004142e0(unit, m, p);
        return L2Vec3{f(static_cast<double>(p[0]) - m[12]), f(static_cast<double>(p[1]) - m[13]),
                      f(static_cast<double>(p[2]) - m[14])};
    };
    const L2Vec3 ax = axis(unit_x);
    const L2Vec3 ay = axis(unit_y);
    const L2Vec3 az = axis(unit_z);
    void* const device = host.read_global(0x00F8D394);
    const auto line = [&](const L2Vec3& direction, std::uint32_t argb) {
        L2Vec3 from = origin;
        L2Vec3 to{f(static_cast<double>(direction[0]) + from[0]), f(static_cast<double>(direction[1]) + from[1]),
                  f(static_cast<double>(direction[2]) + from[2])};
        l2_vcall<void>(device, 0xB4, from.data(), to.data(), argb, argb);
    };
    line(ax, 0xFFFF0000u);
    line(az, 0xFF0000FFu);
    line(ay, 0xFF00FF00u);
}

void* tree_group_type_construct_00ae5160(void* type, const void* name, TerrainHost& host, TerrainLeft2Host& left) {
    wr<void*>(type, 0x00, nullptr);
    wr<void*>(type, 0x04, nullptr);
    wr<float>(type, 0x18, -1.0e10f);  // 00CE4ADC
    wr<float>(type, 0x1C, 1.0e10f);   // 00CE4970
    for (std::uint32_t offset : {0x28u, 0x2Cu, 0x30u, 0x34u, 0x38u, 0x3Cu}) wr<std::uint32_t>(type, offset, 0);
    l2_copy_into(at(type, 0x38), name, host);
    void* const resource = left.resource_manager_load_00b80d70(left.resource_manager_singleton_004c1400(), name);
    wr<void*>(type, 0x00, resource);
    void* const instance = l2_vcall<void*>(resource, 0x08, 0u, 1.0f);
    wr<void*>(type, 0x04, instance);
    left.generated_model_finder_visit_00ae2cb0(type, rd<void*>(instance, 0x0C));
    const auto geometry = [type]() { return l2_geometry(rd<void*>(type, 0x08)); };
    if (rd<std::uint32_t>(geometry(), 0x50) != 0) {
        wr<std::uint32_t>(type, 0x20, rd<std::uint32_t>(at(geometry(), 0x10), 0x08));  // 00B732B0(0)
        wr<std::uint32_t>(type, 0x24, rd<std::uint32_t>(at(geometry(), 0x10), 0x0C));
    } else {
        wr<std::uint32_t>(type, 0x20, 0);
        wr<std::uint32_t>(type, 0x24, rd<std::uint32_t>(geometry(), 0x58) - 1u);  // 00B72B40
    }
    void* const material = rd<void*>(l2_element(geometry(), rd<std::uint32_t>(type, 0x20)), 0x20);
    void* const texture = rd<std::int16_t>(material, 0x34) > 0 ? rd<void*>(material, 0x10) : nullptr;
    l2_copy_into(at(type, 0x30), static_cast<std::uint8_t*>(texture) + 8, host);  // 00B33E40
    void* const effect = rd<void*>(rd<void*>(l2_element(geometry(), rd<std::uint32_t>(type, 0x20)), 0x20), 0x7C);
    l2_copy_into(at(type, 0x28), at(effect, 0xB8), host);  // 00B172D0
    wr<std::uint32_t>(type, 0x0C, 0);
    wr<std::uint32_t>(type, 0x10, 0);
    for (std::uint32_t i = rd<std::uint32_t>(type, 0x20); i <= rd<std::uint32_t>(type, 0x24); ++i) {
        wr<std::uint32_t>(type, 0x0C, rd<std::uint32_t>(type, 0x0C) + rd<std::uint32_t>(l2_element(geometry(), i), 0x10));
        wr<std::uint32_t>(type, 0x10, rd<std::uint32_t>(type, 0x10) + rd<std::uint32_t>(l2_element(geometry(), i), 0x18));
    }
    left.geometry_compute_bounds_00b732d0(geometry());
    void* const bounds = at(geometry(), 0x98);  // 00B726A0
    wr<float>(type, 0x14, f(static_cast<double>(rd<float>(bounds, 0x10)) - rd<float>(bounds, 0x04)));
    wr<void*>(type, 0x40, left.material_create_for_effect_name_00535320(at(type, 0x28)));
    const char* const texture_name = rd<const char*>(type, 0x34) != nullptr ? rd<const char*>(type, 0x34) : "";
    void* loaded;
    {
        L2Temporary temporary(texture_name, host);
        loaded = l2_vcall<void*>(host.read_global(0x00F8D394), 0x64, temporary.get(), 0u);
    }
    left.material_set_texture_slot_00b189f0(rd<void*>(type, 0x40), 0, loaded);
    wr<void*>(type, 0x44, nullptr);
    if (loaded != nullptr) l2_release(loaded);
    return type;
}

void tree_group_initialise_00ae5870(void* group, void* stream, std::uint32_t count, TerrainHost& host,
                                    TerrainLeft2Host& left) {
    wr<std::uint8_t>(group, 0x38, 1);
    wr<std::uint32_t>(group, 0x08, count);
    void* const device = host.read_global(0x00F8D394);
    void* declaration;
    {
        L2Temporary file("ccuf44uf43.mvfm", host);
        declaration = l2_vcall<void*>(device, 0x38, file.get());
    }
    wr<void*>(group, 0x1C, l2_vcall<void*>(device, 0x5C, count, 1u, declaration));
    void* const data = l2_vcall<void*>(rd<void*>(group, 0x1C), 0x10, count, 0u, 0u);
    l2_vcall<void>(stream, 0x24, data, count << 5, 0u);
    l2_vcall<void>(rd<void*>(group, 0x1C), 0x14);
    void* const instance_declaration = l2_vcall<void*>(rd<void*>(group, 0x20), 0x24);
    if (rd<void*>(group, 0x18) == nullptr) {
        void* model = nullptr;
        if (void* const slot = left.model_allocate_raw_slot_00b74eb0(0x184)) {
            L2Temporary temporary("TreeGroup", host);
            model = left.generated_model_construct_00b75030(slot, temporary.get());
        }
        wr<void*>(group, 0x18, model);
        wr<std::uint32_t>(model, 0x138, rd<std::uint32_t>(model, 0x138) & ~3u);
        left.node_set_parent_00b6e680(rd<void*>(group, 0x18), l2_vcall<void*>(rd<void*>(group, 0x00), 0x40));
        left.node_propagate_root_00b6d890(rd<void*>(group, 0x18),
                                          rd<void*>(l2_vcall<void*>(rd<void*>(group, 0x00), 0x40), 0xA4));
        void* const slot = left.mesh_allocate_raw_slot_00b73b60(0xBC);
        void* const mesh = slot != nullptr ? left.mesh_construct_00b73d70(slot) : nullptr;
        wr<void*>(group, 0x28, mesh);
        left.generated_model_set_geometry_00b75170(rd<void*>(group, 0x18), 0, mesh, -1.0f, -1.0f);  // 00D7A260
        const std::uint32_t kind = rd<std::uint32_t>(group, 0x0C);
        const char* const effects[] = {"instanced.mshd", "coral.mshd", "seaweed.mshd"};
        if (kind <= 2) {
            L2Temporary temporary(effects[kind], host);
            wr<void*>(group, 0x34, left.material_create_for_effect_name_00535320(temporary.get()));
        }
        struct {
            void* declarations[4];
            std::uint32_t count;
        } list{{instance_declaration, declaration, nullptr, nullptr}, 2};  // [2..3] stack garbage in the image
        wr<void*>(group, 0x24, l2_vcall<void*>(device, 0x40, &list));
        wr<std::uint32_t>(rd<void*>(group, 0x20), 0x54, 0x40000000u);
        wr<std::uint32_t>(rd<void*>(group, 0x1C), 0x54, 0x80000000u);
        left.mesh_set_vertex_stream_00b73bb0(rd<void*>(group, 0x28), 0, rd<void*>(group, 0x20));
        left.mesh_set_vertex_stream_00b73bb0(rd<void*>(group, 0x28), 1, rd<void*>(group, 0x1C));
        left.geometry_set_index_stream_00b73b70(rd<void*>(group, 0x28),
                                                rd<void*>(l2_geometry(rd<void*>(rd<void*>(group, 0x04), 0x08)), 0x60));
        void* const type_material = rd<void*>(rd<void*>(group, 0x04), 0x40);
        left.material_set_texture_slot_00b189f0(rd<void*>(group, 0x34), 0,
            rd<std::int16_t>(type_material, 0x34) > 0 ? rd<void*>(type_material, 0x10) : nullptr);
    }
    float box[6];
    l2_vcall<void>(rd<void*>(group, 0x00), 0x0C, box);
    const float max_z = box[5];
    const float node_z1 = rd<float>(l2_refreshed_node(l2_vcall<void*>(rd<void*>(group, 0x00), 0x40), left), 0x128);
    const float max_x = box[3];
    const float node_x1 = rd<float>(l2_refreshed_node(l2_vcall<void*>(rd<void*>(group, 0x00), 0x40), left), 0x120);
    const float min_z = box[2];
    const float node_z2 = rd<float>(l2_refreshed_node(l2_vcall<void*>(rd<void*>(group, 0x00), 0x40), left), 0x128);
    const float min_x = box[0];
    const float node_x2 = rd<float>(l2_refreshed_node(l2_vcall<void*>(rd<void*>(group, 0x00), 0x40), left), 0x120);
    const float rect[4] = {f(static_cast<double>(min_x) - node_x2), f(static_cast<double>(min_z) - node_z2),
                           f(static_cast<double>(max_x) - node_x1), f(static_cast<double>(max_z) - node_z1)};
    void* const block = host.operator_new(0x58);  // 00BF681B
    wr<void*>(group, 0x10, block != nullptr ? left.tree_visibility_construct_00ae5550(block, stream, rect) : nullptr);
    if (declaration != nullptr) l2_release(declaration);
}

void terrain_visibility_group_build_flat_00ade0e0(void* group, const void* name, HeightFieldHost& fields,
    TerrainHost& host, BlockHost& blocks, TerrainLeft2Host& left, const TerrainLeft2NodeCalls& nodes) {
    wr<std::uint32_t>(group, 0x28, 0);
    l2_copy_into(at(group, 0x54), name, host);
    void* node = nullptr;
    if (void* const slot = nodes.node_allocate_raw_slot_00b6ed70(0x174)) {
        L2Temporary temporary("Terrain Visibility Group", host);
        node = nodes.node_construct_00b6f5a0(slot, temporary.get());
    }
    wr<void*>(group, 0x2C, node);
    left.node_propagate_root_00b6d890(rd<void*>(group, 0x2C), rd<void*>(group, 0x0C));
    grid_resize(0x00ADACF0, at(group, 0x38), 4, 4, nullptr, host);
    for (std::int32_t y = 0; y < 4; ++y) {
        for (std::int32_t x = 0; x < 4; ++x) {
            void* const tile = fields.create_tile(group, x, y);
            void* const memory_a = host.operator_new(0x38);
            void* const block_a =
                memory_a != nullptr ? terrain_block_construct(0x00AEDAC0, memory_a, 0x21, 0x21, host) : nullptr;
            terrain_tile_set_block_00adf8b0(tile, block_a, host, blocks);
            l2_release(block_a);
            void* const memory_b = host.operator_new(0x38);
            void* const block_b =
                memory_b != nullptr ? terrain_block_construct(0x00AEDAC0, memory_b, 0x21, 0x21, host) : nullptr;
            terrain_tile_set_second_block_00adf920(tile, block_b, host);
            l2_release(block_b);
            const float g = rd<float>(group, 0x18);
            float low = 1.0e10f;  // 00CE4970
            float high = 0.0f;
            using SetSample = void(__thiscall*)(void*, float, std::int32_t, std::int32_t);
            for (std::int32_t j = 0; j < 0x21; ++j) {
                for (std::int32_t i = 0; i < 0x21; ++i) {
                    reinterpret_cast<SetSample>(rd<void*>(rd<void*>(block_a, 0), 0x1C))(block_a, 0.0f, i, j);
                    if (!(0.0f > low)) low = 0.0f;   // COMISS 0, m / JA
                    high = low > 0.0f ? low : 0.0f;  // COMISS m, 0 / JBE
                }
            }
            terrain_tile_refresh_height_range_00adeb00(tile, blocks);
            l2_vcall<void>(tile, 0x08);
            void* const model = rd<void*>(tile, 0x24);  // 00ADEAF0
            const float sx = rd<float>(l2_refreshed_node(model, left), 0xF0);
            const float sy = rd<float>(l2_refreshed_node(model, left), 0x104);
            const float sz = rd<float>(l2_refreshed_node(model, left), 0x118);
            const TerrainVec3f box_min{f(0.0 / sx), f(static_cast<double>(low) / sy), f(0.0 / sz)};
            const TerrainVec3f box_max{f(static_cast<double>(g) / sx), f(static_cast<double>(high) / sy),
                                       f(static_cast<double>(g) / sz)};
            const TerrainSphere sphere = terrain_box_bounding_sphere_00ada600(box_min, box_max);
            const float wider = sx > sy ? sx : sy;         // FCOMPI / JBE
            const float widest = wider > sz ? wider : sz;  // FCOMPI / JA
            float bounds[4] = {sphere.centre.x, sphere.centre.y, sphere.centre.z,
                               f(static_cast<double>(rd<float>(group, 0x18)) / widest)};
            left.generated_model_set_local_bounds_00b74390(model, bounds);
            l2_vcall<void*>(model, 0x48);  // its 16-byte result is copied to dead locals
            rd<void**>(group, 0x40)[rd<std::int32_t>(group, 0x38) * y + x] = tile;
        }
    }
}

void bush_group_initialise_00ae2450(void* group, void* stream, std::uint32_t count, void* name_a,
    void* name_b, NativeStringStorage& strings, TerrainHost& host, TerrainLeft2Host& left,
    const TerrainLeft2BushCalls& bush) {
    struct Header {
        std::uint32_t length;
        char* data;
    };
    const auto release = [&strings](Header& h) {
        if (h.data != nullptr) strings.release(h.data, h.length + 1u);
    };
    const auto literal = [&strings](Header& h, const char* text) {
        h = {0, nullptr};
        resize_native_string_header_0041dd40(&h, strings, static_cast<std::uint32_t>(std::strlen(text)), true);
        if (h.data != nullptr) std::memcpy(h.data, text, h.length + 1u);
    };
    const auto assign = [&strings](void* destination, const void* source) {
        if (destination == source) return;
        resize_native_string_header_0041dd40(destination, strings, rd<std::uint32_t>(source, 0), true);
        if (rd<std::uint32_t>(source, 0) != 0) {
            std::memcpy(rd<char*>(destination, 4), rd<const char*>(source, 4), rd<std::uint32_t>(destination, 0));
        }
    };
    wr<std::uint8_t>(group, 0x40, 1);
    wr<std::uint32_t>(group, 0x14, count);
    assign(at(group, 0x04), name_a);
    assign(at(group, 0x0C), name_b);
    Header normal_a{}, normal_b{};
    {
        Header suffix{}, stem{};
        literal(suffix, "_normal.tga");
        construct_native_string_substring_00469840(at(group, 0x04), &stem, 0, rd<std::uint32_t>(group, 0x04) - 4u, strings);
        concatenate_native_string_headers_004261a0(&stem, &normal_a, &suffix, strings);
        release(stem);
        release(suffix);
    }
    {
        Header suffix{}, stem{};
        literal(suffix, "_normal.tga");
        construct_native_string_substring_00469840(at(group, 0x0C), &stem, 0, rd<std::uint32_t>(group, 0x0C) - 4u, strings);
        concatenate_native_string_headers_004261a0(&stem, &normal_b, &suffix, strings);
        release(stem);
        release(suffix);
    }
    void* const device = host.read_global(0x00F8D394);
    void* declaration;
    {
        Header file{};
        literal(file, "pf44nss2uf42uf44cc.mvfm");
        declaration = l2_vcall<void*>(device, 0x38, &file);
        release(file);
    }
    wr<void*>(group, 0x28, l2_vcall<void*>(device, 0x5C, count * 6u, 1u, declaration));
    void* const data = l2_vcall<void*>(rd<void*>(group, 0x28), 0x10, count * 6u, 0u, 0u);
    l2_vcall<void>(stream, 0x24, data, count * 0x120u, 0u);
    l2_vcall<void>(rd<void*>(group, 0x28), 0x14);
    struct {
        void* declarations[4];
        std::uint32_t count;
    } list{{declaration, nullptr, nullptr, nullptr}, 1};  // [1..3] stack garbage in the image
    wr<void*>(group, 0x50, l2_vcall<void*>(device, 0x40, &list));
    if (declaration != nullptr) l2_release(declaration);
    if (rd<void*>(group, 0x20) == nullptr) {
        void* model = nullptr;
        if (void* const slot = bush.bush_model_pool_allocate_00ae1870(reinterpret_cast<void*>(0x00F8C228))) {
            Header name{};
            literal(name, "BushGroup");
            model = left.generated_model_construct_00b75030(slot, &name);
            wr<std::uint32_t>(model, 0, 0x00D5D4A0u);
            release(name);
        }
        wr<void*>(group, 0x20, model);
        wr<std::uint32_t>(model, 0x138, rd<std::uint32_t>(model, 0x138) & ~3u);
        left.node_set_parent_00b6e680(rd<void*>(group, 0x20), l2_vcall<void*>(rd<void*>(group, 0x00), 0x40));
        left.node_propagate_root_00b6d890(rd<void*>(group, 0x20),
                                          rd<void*>(l2_vcall<void*>(rd<void*>(group, 0x00), 0x40), 0xA4));
        wr<void*>(rd<void*>(group, 0x20), 0x184, group);
        for (const auto& [offset, effect] : {std::pair<std::uint32_t, const char*>{0x3C, "bush_lores.mshd"},
                                             std::pair<std::uint32_t, const char*>{0x38, "bush.mshd"}}) {
            Header name{};
            literal(name, effect);
            wr<void*>(group, offset, left.material_create_for_effect_name_00535320(&name));
            release(name);
        }
        void* const registry = host.read_global(0x00F8C26C);
        const auto text = [](const char* p) { return p != nullptr ? p : ""; };  // 00F8C224
        bush.atlas_find_item_00aefb20(registry, text(rd<const char*>(group, 0x10)));
        bush.atlas_find_item_00aefb20(registry, text(rd<const char*>(group, 0x08)));
        bush.atlas_find_item_00aefb20(registry, text(normal_b.data));
        bush.atlas_find_item_00aefb20(registry, text(normal_a.data));
        void* const foliage = host.read_global(0x00F8C210);
        left.material_set_texture_slot_00b189f0(rd<void*>(group, 0x3C), 0, bush.bush_texture_00ad57d0(foliage));
        left.material_set_texture_slot_00b189f0(rd<void*>(group, 0x3C), 1, bush.bush_normal_texture_00ad57e0(foliage));
        left.material_set_texture_slot_00b189f0(rd<void*>(group, 0x3C), 2,
                                                bush.shadow_map_texture_00b0d140(host.read_global(0x00F8D39C)));
        left.material_set_texture_slot_00b189f0(rd<void*>(group, 0x38), 0, bush.bush_texture_00ad57d0(foliage));
        left.material_set_texture_slot_00b189f0(rd<void*>(group, 0x38), 1, bush.bush_normal_texture_00ad57e0(foliage));
    }
    float box[6];
    l2_vcall<void>(rd<void*>(group, 0x00), 0x0C, box);
    float centre[3], half[3];
    for (int i = 0; i < 3; ++i) {
        centre[i] = f(static_cast<double>(f(static_cast<double>(box[i]) + box[i + 3])) * 0.5);  // 00D7A280
        half[i] = f(static_cast<double>(f(static_cast<double>(box[i + 3]) - box[i])) * 0.5);
    }
    void* node = nullptr;
    if (void* const block = host.operator_new(0x38)) {  // 00BF681B
        const float y = rd<float>(l2_refreshed_node(l2_vcall<void*>(rd<void*>(group, 0x00), 0x40), left), 0x124);
        node = bush.bush_quad_node_read_00ae0f10(block, stream, centre, half, y);
    }
    wr<void*>(group, 0x18, node);
    release(normal_b);
    release(normal_a);
    release(*static_cast<Header*>(name_a));  // the by-value arguments, owned by the callee (RET 18h)
    release(*static_cast<Header*>(name_b));
}

} // namespace bsp::cc13_terrain

namespace bsp::cc13_terrain {
namespace {
float nc_percentage(float numerator, float denominator) {
    const double hundred = 100.0; // verified qword 00D7A220
    float value;
    __asm {
        fld numerator
        fdiv denominator
        fmul hundred
        fstp value
    }
    return value;
}
void nc_add_percentage(void* destination, const void* name, float percentage, TerrainHost& host) {
    struct StringHeader { std::uint32_t length; void* chars; } copy{0, nullptr};
    host.native_string_assign_0041dd40(&copy, rd<const void*>(name, 4), rd<std::uint32_t>(name, 0));
    std::uint32_t word;
    std::memcpy(&word, &percentage, sizeof word);
    named_list_array_add_under_name_00aff290(destination, &copy,
        reinterpret_cast<void*>(static_cast<std::uintptr_t>(word)), host);
}
} // namespace
void emitter_set_collect_percentages_00afd860(void* set, void* destination, TerrainHost& host) {
    for (std::int32_t i = 0; i < rd<std::int32_t>(set, 0x14); ++i) {
        const auto index = rd<const std::uint8_t*>(set, 4)[i];
        const auto* record = rd<const std::uint8_t*>(set, 0xc) + index * 0x108u;
        const void* definition = rd<const void*>(record, 0xa0);
        if (rd<std::uint8_t>(definition, 0x7c)) {
            const float value = nc_percentage(rd<float>(record, 0x34), rd<float>(record, 0x38));
            nc_add_percentage(destination, static_cast<const std::uint8_t*>(definition) + 8, value, host);
        }
    }
}
void particle_group_collect_percentages_00b05210(void* group, void* destination, TerrainHost& host) {
    for (std::int32_t i = 0; i < rd<std::int32_t>(group, 0x1c); ++i) {
        const auto index = rd<const std::uint16_t*>(group, 0xc)[i * 4];
        const auto* record = rd<const std::uint8_t*>(group, 0x14) + index * 0x6cu;
        const float value = nc_percentage(rd<float>(record, 0x40), rd<float>(record, 0x44));
        const auto* definition = rd<const std::uint8_t*>(record, 0x64);
        nc_add_percentage(destination, definition + 8, value, host);
    }
}
void named_list_array_clear_00aff410(void* array, TerrainHost& host) {
    named_list_array_resize_00aff210(array, 0, host);
}
void emitter_collect_percentages_00aff580(void* emitter, void* destination, TerrainHost& host) {
    if (void* group = rd<void*>(emitter, 0x10))
        particle_group_collect_percentages_00b05210(group, destination, host);
}
std::uint32_t renderer_sum_resource_words_00b23d40(void* renderer, ResourceWord slot0c) {
    auto* item = rd<std::uint8_t*>(renderer, 0x1a78);
    const auto end = reinterpret_cast<std::uintptr_t>(item)
        + rd<std::uint32_t>(renderer, 0x1a7c) * 0x2cu;
    std::uint32_t result = 0;
    while (reinterpret_cast<std::uintptr_t>(item) != end) {
        result += slot0c(rd<void*>(item, 0x28));
        item += 0x2c;
    }
    return result;
}
__declspec(naked) void texture_tail_slot3c_00b33ea0() {
    __asm {
        mov eax, [ecx]
        mov edx, [eax + 0x3c]
        jmp edx
    }
}
__declspec(naked) void texture_tail_slot40_00b33eb0() {
    __asm {
        mov eax, [ecx]
        mov edx, [eax + 0x40]
        jmp edx
    }
}
} // namespace bsp::cc13_terrain

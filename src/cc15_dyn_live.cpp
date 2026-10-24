// cc15_dyn_live: the referenced Dyn/Mit bodies of segment 0. See include/bsp/cc15_dyn_live.hpp and
// docs/CC15_DYN_LIVE.md. Nothing here is wired. No x87 arithmetic except 00407000's FILD/FSTP,
// which is an exact load and one round-to-nearest store to double, independent of the precision
// control word. No function here draws from the shared RNG 00BD2F10.

#include "bsp/cc15_dyn_live.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <intrin.h>

namespace bsp::cc15_dyn_live {

using cc13_dyn1::Raw;
using cc13_dyn1::raw;

namespace {

// The two pools differ only in these constants.
struct PoolShape {
    std::uint32_t page_bytes;   // operator new size
    std::uint32_t slot_stride;  // 28h / 18h
    std::uint32_t stack_off;    // 1400h / C00h
    std::uint32_t depth_off;    // 1500h / D00h
};
constexpr PoolShape kPool36{0x1504, 0x28, 0x1400, 0x1500};
constexpr PoolShape kPool20{0xd04, 0x18, 0xc00, 0xd00};

static_assert(sizeof(void*) == 4 && sizeof(MitPoolAllocator) == 0x38);
constexpr std::uint32_t kPool36Vtable = 0x00d7a1dc;
constexpr std::uint32_t kPool20Vtable = 0x00d7a1e4;

void pool_unlink(MitPoolAllocator& pool, void*& registry_head) noexcept {
    pool.vtable_00 = 0x00d7a0c0;  // allocator_list_elem base, 00403970
    if (pool.field_04 != 0)
        raw(reinterpret_cast<void*>(pool.field_04)).at<std::uint32_t>(8) = pool.registry_next_08;
    else
        registry_head = reinterpret_cast<void*>(pool.registry_next_08);
    if (pool.registry_next_08 != 0)
        raw(reinterpret_cast<void*>(pool.registry_next_08)).at<std::uint32_t>(4) = pool.field_04;
    // Native unlink keeps this element's stale links.
}

void pool_destroy_lock(MitPoolAllocator& pool) noexcept {
    while (static_cast<std::int32_t>(pool.lock_depth_24) > 0) {
        --pool.lock_depth_24;
        ::LeaveCriticalSection(&pool.lock_0c);
    }
    ::DeleteCriticalSection(&pool.lock_0c);
}

MitPoolAllocator* pool_construct(MitPoolAllocator& pool, void*& registry_head,
    std::uint32_t vtable, const MitPoolLifecycleCrt& crt) {
    // Publication precedes both the concrete vtable store and lock creation.
    pool.field_04 = 0;
    pool.registry_next_08 = reinterpret_cast<std::uint32_t>(registry_head);
    if (registry_head != nullptr) raw(registry_head).ptr(4) = &pool;
    registry_head = &pool;
    unsigned unwind_state = 0;
    try {
        pool.vtable_00 = vtable;
        ::InitializeCriticalSection(&pool.lock_0c);
        pool.lock_depth_24 = 0;
        pool.pages_28 = nullptr;
        pool.page_count_2c = 0;
        // FH3 state 2: vector at +28h, section at +0Ch, base links.
        unwind_state = 2;
        pool.first_free_page_34 = 0xffffffffu;
        pool.page_capacity_30 = 0x20;
        auto* fresh = static_cast<std::uint8_t**>(crt.operator_new_00bf55be(0x80));
        auto cursor = reinterpret_cast<std::uintptr_t>(fresh);
        // Reload count/table after allocation. A provider can observe/change
        // the published pool; retain the image's zero-destination skip too.
        for (std::uint32_t i = 0; i < pool.page_count_2c; ++i, cursor += 4) {
            if (cursor != 0)
                *reinterpret_cast<std::uint8_t**>(cursor) = pool.pages_28[i];
        }
        if (pool.pages_28 != nullptr) crt.free_00bf6989(pool.pages_28);
        pool.pages_28 = fresh;
    } catch (...) {
        if (unwind_state == 2) {
            if (pool.pages_28 != nullptr) crt.free_00bf6989(pool.pages_28);
            pool_destroy_lock(pool);
        }
        pool_unlink(pool, registry_head);
        throw;
    }
    return &pool;
}

void pool_destroy(MitPoolAllocator& pool, void*& registry_head,
    std::uint32_t vtable, const MitPoolLifecycleCrt& crt) {
    pool.vtable_00 = vtable;
    // The image reloads both count and table after every page-delete callback.
    for (std::uint32_t i = 0; i < pool.page_count_2c; ++i)
        crt.delete_00bf65ac(pool.pages_28[i]);
    if (pool.pages_28 != nullptr) crt.free_00bf6989(pool.pages_28);
    pool_destroy_lock(pool);
    pool_unlink(pool, registry_head);
}

std::uint8_t* page_construct(const PoolShape& s, std::uint8_t* page, std::uint32_t page_index) noexcept {
    const Raw p = raw(page);
    p.at<std::uint16_t>(s.depth_off) = 0x80;
    for (std::int32_t i = 0; i < 0x80; ++i) {
        p.at<std::uint16_t>(s.stack_off + 2u * i) = static_cast<std::uint16_t>(0x7f - i);
        // the page index is the dword after each slot's payload
        p.at<std::uint32_t>(s.slot_stride * i + (s.slot_stride - 4)) = page_index;
    }
    return page;
}

// 004077F0 / 0040D860 share this body instruction for instruction.
void* pool_allocate(const PoolShape& s, MitPoolAllocator& pool, const DynLiveCrt& crt) {
    ::EnterCriticalSection(&pool.lock_0c);
    pool.lock_depth_24 += 1;
    if (pool.first_free_page_34 == 0xffffffffu) {
        // No page has a free slot: the new page's index is the current count.
        pool.first_free_page_34 = pool.page_count_2c;
        auto* mem = static_cast<std::uint8_t*>(crt.operator_new_00bf681b(s.page_bytes));
        std::uint8_t* const page = mem ? page_construct(s, mem, pool.first_free_page_34) : nullptr;
        if (pool.page_count_2c == pool.page_capacity_30) {
            // grow the page vector to (capacity + 1) * 2
            const std::uint32_t capacity = (pool.page_capacity_30 + 1) * 2;
            pool.page_capacity_30 = capacity;
            auto** pages = static_cast<std::uint8_t**>(crt.operator_new_00bf55be(capacity * 4u));
            for (std::uint32_t i = 0; i < pool.page_count_2c; ++i) {
                if (pages + i != nullptr) pages[i] = pool.pages_28[i];  // placement-new null test
            }
            if (pool.pages_28 != nullptr) crt.free_00bf6989(pool.pages_28);
            pool.pages_28 = pages;
        }
        std::uint8_t** const end = pool.pages_28 + pool.page_count_2c;
        if (end != nullptr) *end = page;
        pool.page_count_2c += 1;
    }
    // Pop the top of the hinted page's free stack. A null page (failed new) faults here as in the
    // image.
    const Raw p = raw(pool.pages_28[pool.first_free_page_34]);
    const std::uint16_t depth = static_cast<std::uint16_t>(p.at<std::uint16_t>(s.depth_off) - 1);
    p.at<std::uint16_t>(s.depth_off) = depth;
    const std::uint16_t index = p.at<std::uint16_t>(s.stack_off + 2u * depth);
    void* const slot = p.field(s.slot_stride * index);
    if (depth == 0) {
        // The page is now full: move the hint to the next page with a free slot, or -1. The scan
        // starts after the old hint; pages before it are not revisited.
        const std::uint32_t start = pool.first_free_page_34 + 1;
        pool.first_free_page_34 = 0xffffffffu;
        for (std::uint32_t i = start; i < pool.page_count_2c; ++i) {
            if (raw(pool.pages_28[i]).at<std::uint16_t>(s.depth_off) != 0) {
                pool.first_free_page_34 = i;
                break;
            }
        }
    }
    pool.lock_depth_24 -= 1;
    ::LeaveCriticalSection(&pool.lock_0c);
    return slot;
}

}  // namespace

std::uint8_t* pool36_page_construct_004084d0(std::uint8_t* page, std::uint32_t page_index) noexcept {
    return page_construct(kPool36, page, page_index);
}

std::uint8_t* pool20_page_construct_0040da40(std::uint8_t* page, std::uint32_t page_index) noexcept {
    return page_construct(kPool20, page, page_index);
}

void* pool36_allocate_004077f0(MitPoolAllocator& pool, const DynLiveCrt& crt) {
    return pool_allocate(kPool36, pool, crt);
}

void* pool20_allocate_0040d860(MitPoolAllocator& pool, const DynLiveCrt& crt) {
    return pool_allocate(kPool20, pool, crt);
}

void pool20_free_0040d9d0(MitPoolAllocator& pool, void* slot) {
    ::EnterCriticalSection(&pool.lock_0c);  // 0040D9D9 CALL [00CE2218]
    pool.lock_depth_24 += 1;
    const std::uint32_t page_index = raw(slot).at<std::uint32_t>(0x14);
    std::uint8_t* const page = pool.pages_28[page_index];
    // 0040D9F1..0040DA02: (slot - page) / 18h by IMUL 2AAAAAABh, SAR 2, plus the sign bit.
    const std::int32_t offset = static_cast<std::int32_t>(
        reinterpret_cast<std::uintptr_t>(slot) - reinterpret_cast<std::uintptr_t>(page));
    const Raw p = raw(page);
    const std::uint16_t depth = p.at<std::uint16_t>(0xd00);
    p.at<std::uint16_t>(0xc00 + 2u * depth) = static_cast<std::uint16_t>(offset / 0x18);
    p.at<std::uint16_t>(0xd00) = static_cast<std::uint16_t>(depth + 1);
    if (page_index < pool.first_free_page_34) pool.first_free_page_34 = page_index;  // JNC: unsigned
    pool.lock_depth_24 -= 1;
    ::LeaveCriticalSection(&pool.lock_0c);  // 0040DA35 CALL [00CE2210]
}

// ---------------------------------------------------------------- edge hash map

std::uint32_t edge_hash_bucket(std::uint16_t key_a, std::uint16_t key_b) noexcept {
    const std::uint32_t k = (static_cast<std::uint32_t>(key_a) << 16) + key_b;
    std::uint32_t h = (k + 0xe092c689u) ^ 0x4e67c6a7u;
    for (int round = 0; round < 4; ++round) h ^= (h << 5) + (h >> 2) + k;
    return h & 0x7fu;
}

namespace {
inline std::uint32_t composite_key(std::uint16_t a, std::uint16_t b) noexcept {
    return (static_cast<std::uint32_t>(a) << 16) + b;
}

// Shared tail of 004080D0/00408470: count shorts of the value array, re-reading the source count
// each iteration (signed compare), then the count itself.
void copy_values(EdgeNode* dst, const EdgeEntry* src) noexcept {
    for (std::int32_t i = 0; i < src->value_count_08; ++i) dst->values_04[i] = src->values_04[i];
    dst->value_count_08 = src->value_count_08;
}
}  // namespace

EdgeNode** edge_map_find_00407a50(EdgeNode** out, const EdgeEntry* key, const EdgeHashMap* map) noexcept {
    const std::uint32_t k = composite_key(key->key_a_00, key->key_b_02);
    for (EdgeNode* n = map->buckets_000[edge_hash_bucket(key->key_a_00, key->key_b_02)]; n; n = n->next_10) {
        if (composite_key(n->key_a_00, n->key_b_02) == k) {
            *out = n;
            return out;
        }
    }
    *out = nullptr;
    return out;
}

EdgeNode* edge_node_assign_004080d0(EdgeNode* dst, const EdgeEntry* src) noexcept {
    dst->key_a_00 = src->key_a_00;
    dst->key_b_02 = src->key_b_02;
    copy_values(dst, src);
    return dst;
}

EdgeNode* edge_node_construct_00408470(EdgeNode* dst, const EdgeEntry* src) noexcept {
    std::memcpy(dst, src, 4);  // 00408470 MOV EDX,[ECX] / MOV [EAX],EDX
    copy_values(dst, src);
    return dst;
}

EdgeNode* edge_map_find_or_insert_00407af0(const EdgeEntry* entry, EdgeHashMap* map,
                                           MitPoolAllocator& pool20, const DynLiveCrt& crt) {
    const std::uint32_t k = composite_key(entry->key_a_00, entry->key_b_02);
    const std::uint32_t bucket = edge_hash_bucket(entry->key_a_00, entry->key_b_02);
    for (EdgeNode* n = map->buckets_000[bucket]; n; n = n->next_10) {
        if (composite_key(n->key_a_00, n->key_b_02) == k) return edge_node_assign_004080d0(n, entry);
    }
    map->count_200 += 1;
    auto* node = static_cast<EdgeNode*>(pool_allocate(kPool20, pool20, crt));  // 00407B71 CALL 0040D860
    if (node != nullptr) edge_node_construct_00408470(node, entry);
    // A null node faults on the next store, as in the image.
    node->next_10 = map->buckets_000[bucket];
    node->prev_0c = nullptr;
    if (map->buckets_000[bucket] != nullptr) map->buckets_000[bucket]->prev_0c = node;
    map->buckets_000[bucket] = node;
    return node;
}

void edge_map_clear_00408340(EdgeHashMap* map, MitPoolAllocator& pool20) {
    for (std::int32_t b = 0; b < 0x80; ++b) {
        for (EdgeNode* n = map->buckets_000[b]; n;) {
            EdgeNode* const next = n->next_10;  // read before the free, as 00408360 does
            pool20_free_0040d9d0(pool20, n);     // inlined copy of 0040D9D0
            n = next;
        }
    }
    std::memset(map, 0, 0x200);  // 004083F2 CALL 00BF79F0 (memset)
    map->count_200 = 0;
}

// ---------------------------------------------------------------- 0040AF00

DynPool68* pool68_construct_0040af00(DynPool68* pool, const DynLiveCrt& crt) {
    pool->pages_00 = nullptr;
    pool->page_count_04 = 0;
    pool->page_capacity_08 = 0;
    pool->ptr_90 = pool->inline_10;
    pool->field_94 = 0;
    pool->field_4c = 0;
    pool->ptr_50 = reinterpret_cast<std::uint32_t*>(pool->inline_54);
    pool->free_head_0c = nullptr;
    pool->field_98 = 0;
    auto* page = static_cast<std::uint8_t*>(crt.malloc_00bf9f1a(0x109a0));
    // No null test on the malloc result (0040AF5B): a failed malloc faults here as in the image.
    for (std::uint32_t i = 0; i < 999; ++i) raw(page).ptr(0x44 * i + 0x40) = page + 0x44 * (i + 1);
    raw(page).at<std::uint32_t>(0x1099c) = 0;
    pool->free_head_0c = page;
    if (pool->page_count_04 == pool->page_capacity_08) {
        // capacity * 2 + 2
        const std::uint32_t capacity = pool->page_capacity_08 * 2 + 2;
        pool->page_capacity_08 = capacity;
        auto** pages = static_cast<std::uint8_t**>(crt.operator_new_00bf55be(capacity * 4u));
        for (std::uint32_t i = 0; i < pool->page_count_04; ++i) {
            if (pages + i != nullptr) pages[i] = pool->pages_00[i];
        }
        if (pool->pages_00 != nullptr) crt.free_00bf6989(pool->pages_00);
        pool->pages_00 = pages;
    }
    std::uint8_t** const end = pool->pages_00 + pool->page_count_04;
    if (end != nullptr) *end = page;
    pool->page_count_04 += 1;
    return pool;
}

// ---------------------------------------------------------------- vector of dwords

DynVector4* vector4_copy_construct_0040d7b0(DynVector4* dst, const DynVector4* src, const DynLiveCrt& crt) {
    const auto size = static_cast<std::uint32_t>(src->size_04);
    if (size == 0) {
        dst->data_00 = nullptr;
        dst->capacity_08 = 0;
        dst->size_04 = 0;
        return dst;
    }
    dst->capacity_08 = size;
    dst->data_00 = static_cast<std::uint32_t*>(crt.operator_new_00bf55be(size * 4u));
    dst->size_04 = static_cast<std::int32_t>(dst->capacity_08);
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(dst->size_04); ++i) {  // JC: unsigned
        if (dst->data_00 + i != nullptr) dst->data_00[i] = src->data_00[i];
    }
    return dst;
}

namespace {
void set_capacity(DynVector4* v, std::uint32_t capacity, const DynLiveCrt& crt) {
    v->capacity_08 = capacity;
    // MUL by 4, SETO / NEG / OR: an overflowing request becomes FFFFFFFFh.
    const std::uint64_t bytes = static_cast<std::uint64_t>(capacity) * 4u;
    const std::uint32_t request = (bytes >> 32) ? 0xffffffffu : static_cast<std::uint32_t>(bytes);
    std::uint32_t* const old = v->data_00;
    v->data_00 = static_cast<std::uint32_t*>(crt.operator_new_00bf55be(request));
    for (std::int32_t i = 0; i < v->size_04; ++i) v->data_00[i] = old[i];
    if (old != nullptr) crt.free_00bf6989(old);
}
}  // namespace

void vector4_set_capacity_0040ee00(DynVector4* v, std::uint32_t capacity, const DynLiveCrt& crt) {
    set_capacity(v, capacity, crt);
}

void vector4_set_capacity_0040eef0(DynVector4* v, std::uint32_t capacity, const DynLiveCrt& crt) {
    set_capacity(v, capacity, crt);
}

// ---------------------------------------------------------------- 00407000 / 0040CFC0

void* profile_timer_calibrate_00407000(void* timer, double& ticks_per_second_0109e9e8) {
    const std::uint64_t before = __rdtsc();
    ::Sleep(100);  // 0040700D CALL [00CE2230]
    const std::uint64_t after = __rdtsc();
    // 64-bit SUB/SBB, __allmul by 10 (low 64 bits), FILD qword (signed) / FSTP double.
    const auto ticks = static_cast<std::int64_t>((after - before) * 10u);
    ticks_per_second_0109e9e8 = static_cast<double>(ticks);
    return timer;
}

int format_string_1024_0040cfc0(char* buffer, const char* format, ...) {
    va_list args;
    va_start(args, format);
    const int result = ::vsprintf_s(buffer, 0x400, format, args);  // 0040CFD4 CALL 00BF7289
    va_end(args);
    return result;
}

// ------------------------------------------------------ oct24 pool lifecycle

MitPoolAllocator* pool36_construct_00407590(MitPoolAllocator& pool,
    void*& registry_head, const MitPoolLifecycleCrt& crt) {
    return pool_construct(pool, registry_head, kPool36Vtable, crt);
}

void pool36_destroy_global_00407690(MitPoolAllocator& pool,
    void*& registry_head, const MitPoolLifecycleCrt& crt) {
    pool_destroy(pool, registry_head, kPool36Vtable, crt);
}

MitPoolAllocator* pool20_construct_0040f910(MitPoolAllocator& pool,
    void*& registry_head, const MitPoolLifecycleCrt& crt) {
    return pool_construct(pool, registry_head, kPool20Vtable, crt);
}

void pool20_destroy_global_0040fa10(MitPoolAllocator& pool,
    void*& registry_head, const MitPoolLifecycleCrt& crt) {
    pool_destroy(pool, registry_head, kPool20Vtable, crt);
}

}  // namespace bsp::cc15_dyn_live

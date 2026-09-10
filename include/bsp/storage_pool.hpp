#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

// Host projection of the game's sized storage pool, recovered from 00419cc0
// (pool A singleton getter), 00bd1120 / 00bd1510 (pool A allocate / release),
// 00bd12a0 (free-ring push), 00bd0f50 / 00bd1480 (pool A construction) and the
// second instantiation 00bd1680 / 00bd11f0 / 00bd1620 / 00bd1000 / 00bd1570
// behind GameAlloc 00bd1780 and GameFree 00bd17a0.
//
// This is a semantic reconstruction, not a binary-compatible replacement. The
// native pools are lazily constructed process singletons behind DAT_01090aa8
// and DAT_01090aac; nothing here reads a global. Evidence, native ABI and the
// deviations listed below: docs/APP_INIT_ALLOC_STRINGS.md.
namespace bsp {

// Backing allocator for requests at or above the pool threshold. Native pool A
// forwards those to _malloc 00bf9f1a and _free 00bf9dc8; pool B does the same.
// allocate must not return null; it reports exhaustion by throwing.
class SystemAllocator {
public:
    virtual ~SystemAllocator() = default;
    virtual void* allocate(std::uint32_t size) = 0;
    virtual void release(void* block) noexcept = 0;
};

// std::malloc / std::free, matching the native large-block path.
SystemAllocator& crt_system_allocator() noexcept;

// Recovered geometry of one pool instantiation. class_count doubles as the
// size threshold: a request of exactly n bytes uses free ring n, so n and the
// block size are the same number and a block must be released with the size it
// was allocated with. region_stride is the per-class slice of the shared ring
// laid down by the constructor (head[i] = i * region_stride, tail[i] = head[i] - 1).
struct SizedStoragePoolConfig {
    std::uint32_t class_count{};    // 0x96 pool A, 0x81 pool B
    std::uint32_t ring_capacity{};  // 0x80000 both; must be a power of two
    std::uint32_t region_stride{};  // 0xda7 pool A, 0xfe0 pool B
    std::uint32_t arena_bytes{};    // 7,000,000 pool A; 10,000,000 pool B
    std::uint32_t bump_alignment{}; // 1 pool A (exact bytes), 4 pool B
};

// Pool A, reached through 00419cc0. Serves BSP_NativeString_Resize 0041dd40.
SizedStoragePoolConfig string_pool_config_00419cc0() noexcept;
// Pool B, reached through 00bd1680. Serves GameAlloc/GameFree and Lua l_alloc.
SizedStoragePoolConfig game_pool_config_00bd1680() noexcept;

class SizedStoragePool {
public:
    // Throws std::invalid_argument if the configuration cannot describe a
    // native pool: ring_capacity must be a non-zero power of two and the class
    // regions (class_count * region_stride) must fit inside it.
    explicit SizedStoragePool(const SizedStoragePoolConfig& config,
        SystemAllocator& system = crt_system_allocator());

    SizedStoragePool(const SizedStoragePool&) = delete;
    SizedStoragePool& operator=(const SizedStoragePool&) = delete;

    // 00bd1120 / 00bd11f0. size >= class_count goes to the system allocator.
    // Otherwise the free ring for that exact size is popped, and if that region
    // is empty a fresh block is carved off the bump arena. Never returns null:
    // the native carve has no bounds check at all, so this projection throws
    // std::bad_alloc where the native would walk past the arena.
    void* allocate_00bd1120(std::uint32_t size);

    // 00bd1510 / 00bd1620. size must be the size the block was allocated with.
    // size >= class_count is released to the system allocator; a smaller block
    // is pushed back onto the free ring for that exact size, unless small
    // returns are disabled, in which case the block is dropped and never freed.
    void release_00bd1510(void* block, std::uint32_t size) noexcept;

    bool is_pooled_size(std::uint32_t size) const noexcept { return size < config_.class_count; }

    // Projection of the native DAT_01090aa4 gate, which is read only on the
    // release path: when it is non-zero small blocks are dropped, not returned.
    void set_small_returns_enabled(bool enabled) noexcept { small_returns_enabled_ = enabled; }
    bool small_returns_enabled() const noexcept { return small_returns_enabled_; }

    // Ring bookkeeping mirroring the native counters at ring+0x2004b0 (live)
    // and ring+0x2004b4 (high-water mark). allocate decrements live and leaves
    // the high-water mark alone, exactly as the native does.
    std::uint32_t live_free_blocks() const noexcept { return live_free_blocks_; }
    std::uint32_t peak_free_blocks() const noexcept { return peak_free_blocks_; }
    std::uint32_t arena_used() const noexcept { return bump_offset_; }
    std::uint32_t arena_bytes() const noexcept { return config_.arena_bytes; }

private:
    void push_00bd12a0(void* block, std::uint32_t size_class) noexcept;

    SizedStoragePoolConfig config_;
    SystemAllocator* system_;
    std::unique_ptr<unsigned char[]> arena_;
    std::vector<void*> ring_;
    std::vector<std::uint32_t> region_head_;
    std::vector<std::uint32_t> region_tail_;
    std::uint32_t ring_mask_{};
    std::uint32_t bump_offset_{};
    std::uint32_t live_free_blocks_{};
    std::uint32_t peak_free_blocks_{};
    bool small_returns_enabled_{true};
};

// GameAlloc 00bd1780 and GameFree 00bd17a0: __cdecl wrappers that fetch the
// pool B singleton and forward. The free is sized, so every caller has to carry
// the allocation size; Lua's l_alloc 00a6a1d0 does exactly that with osize.
void* game_allocate_00bd1780(SizedStoragePool& pool, std::uint32_t size);
void game_release_00bd17a0(SizedStoragePool& pool, void* block, std::uint32_t size) noexcept;

}

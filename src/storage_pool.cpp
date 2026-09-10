#include "bsp/storage_pool.hpp"

#include <cstdlib>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {

class CrtSystemAllocator final : public SystemAllocator {
public:
    void* allocate(std::uint32_t size) override {
        void* block = std::malloc(size == 0 ? 1u : static_cast<std::size_t>(size));
        if (block == nullptr) throw std::bad_alloc();
        return block;
    }
    void release(void* block) noexcept override { std::free(block); }
};

bool is_power_of_two(std::uint32_t value) noexcept {
    return value != 0 && (value & (value - 1u)) == 0;
}

} // namespace

SystemAllocator& crt_system_allocator() noexcept {
    static CrtSystemAllocator allocator;
    return allocator;
}

// 00bd1480 plus 00bd0f50: 150 classes, ring slice 0xda7, arena this+8 through
// this+0x6acfc8 (7,000,000 bytes), carve advances by the exact byte count.
SizedStoragePoolConfig string_pool_config_00419cc0() noexcept {
    SizedStoragePoolConfig config;
    config.class_count = 0x96;
    config.ring_capacity = 0x80000;
    config.region_stride = 0xda7;
    config.arena_bytes = 7000000u;
    config.bump_alignment = 1;
    return config;
}

// 00bd1570 plus 00bd1000: 129 classes, ring slice 0xfe0, arena this+0x200414
// through this+0xb89a94 (10,000,000 bytes), carve advances by size rounded up
// to four (00bd1244: ADD EBX,3 / AND EBX,0xfffffffc).
SizedStoragePoolConfig game_pool_config_00bd1680() noexcept {
    SizedStoragePoolConfig config;
    config.class_count = 0x81;
    config.ring_capacity = 0x80000;
    config.region_stride = 0xfe0;
    config.arena_bytes = 10000000u;
    config.bump_alignment = 4;
    return config;
}

SizedStoragePool::SizedStoragePool(const SizedStoragePoolConfig& config, SystemAllocator& system)
    : config_(config), system_(&system) {
    if (config_.class_count == 0) throw std::invalid_argument("pool needs at least one size class");
    if (!is_power_of_two(config_.ring_capacity)) throw std::invalid_argument("ring capacity must be a power of two");
    if (config_.bump_alignment == 0 || !is_power_of_two(config_.bump_alignment)) {
        throw std::invalid_argument("bump alignment must be a power of two");
    }
    if (config_.region_stride == 0 ||
        static_cast<std::uint64_t>(config_.region_stride) * config_.class_count > config_.ring_capacity) {
        throw std::invalid_argument("class regions do not fit in the ring");
    }

    ring_mask_ = config_.ring_capacity - 1u;
    // The native arena is raw storage from operator new and is never zeroed.
    arena_.reset(new unsigned char[config_.arena_bytes]);
    ring_.assign(config_.ring_capacity, nullptr);
    region_head_.resize(config_.class_count);
    region_tail_.resize(config_.class_count);
    for (std::uint32_t index = 0; index < config_.class_count; ++index) {
        const std::uint32_t head = index * config_.region_stride;
        region_head_[index] = head;
        // 00bd0f50 stores head - 1 and then rewrites entry 0 with the mask, so
        // every empty region satisfies (tail + 1) & mask == head.
        region_tail_[index] = (head - 1u) & ring_mask_;
    }
}

void* SizedStoragePool::allocate_00bd1120(std::uint32_t size) {
    if (size >= config_.class_count) return system_->allocate(size);

    const std::uint32_t tail = region_tail_[size];
    if (region_head_[size] == ((tail + 1u) & ring_mask_)) {
        // Region empty: carve a fresh block off the bump arena. 00bd1171.
        const std::uint32_t offset = bump_offset_;
        const std::uint32_t step = (size + config_.bump_alignment - 1u) & ~(config_.bump_alignment - 1u);
        if (step > config_.arena_bytes - offset) {
            // The native has no such check and simply walks off the arena.
            throw std::bad_alloc();
        }
        bump_offset_ = offset + step;
        return arena_.get() + offset;
    }

    --live_free_blocks_; // 00bd1197, mirrored without touching the high-water mark.
    void* block = ring_[tail];
    region_tail_[size] = (tail - 1u) & ring_mask_;
    return block;
}

void SizedStoragePool::release_00bd1510(void* block, std::uint32_t size) noexcept {
    if (size >= config_.class_count) {
        system_->release(block);
        return;
    }
    if (!small_returns_enabled_) return; // DAT_01090aa4 non-zero: the block is dropped.
    push_00bd12a0(block, size);
}

// 00bd12a0. Every size class owns a circular slice of one shared ring. Pushing
// advances the class's own tail first; if that runs into the head of the next
// class, that class's whole region is rotated forward by one slot, which
// displaces its first element into the slot just vacated. The displaced element
// becomes the block looking for a home, and the cascade continues until a class
// with slack is found or the ring wraps back to the starting class.
void SizedStoragePool::push_00bd12a0(void* block, std::uint32_t size_class) noexcept {
    region_tail_[size_class] = (region_tail_[size_class] + 1u) & ring_mask_;

    std::uint32_t last = size_class;
    std::uint32_t next = (size_class + 1u) % config_.class_count;
    while (next != size_class) {
        const std::uint32_t boundary = region_head_[next];
        if (region_tail_[last] != boundary) break;
        if (boundary != ((region_tail_[next] + 1u) & ring_mask_)) {
            // Region `next` holds at least one block; swap ours into its head
            // slot and carry its first element onward.
            void* displaced = ring_[boundary];
            ring_[boundary] = block;
            block = displaced;
        }
        region_head_[next] = (boundary + 1u) & ring_mask_;
        region_tail_[next] = (region_tail_[next] + 1u) & ring_mask_;
        last = next;
        next = (next + 1u) % config_.class_count;
    }

    ring_[region_tail_[last]] = block;
    ++live_free_blocks_;
    if (live_free_blocks_ > peak_free_blocks_) peak_free_blocks_ = live_free_blocks_;
}

void* game_allocate_00bd1780(SizedStoragePool& pool, std::uint32_t size) {
    return pool.allocate_00bd1120(size);
}

void game_release_00bd17a0(SizedStoragePool& pool, void* block, std::uint32_t size) noexcept {
    pool.release_00bd1510(block, size);
}

}

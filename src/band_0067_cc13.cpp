// Leaf helpers from the 00670000-006CFFFF and 00700000-0073FFFF bands (cc13-sweep67).
// Reconstructed and build-tested only: not ABI-compatible, not fixture- or game-validated.
// See docs/CC13_BAND_0067_0073.md.

#include "bsp/band_0067_cc13.hpp"
#include "bsp/cc13_x87_model.hpp"

namespace bsp::band_0067_cc13 {

Vec3 vec3_add_00709a60(const Vec3& a, const Vec3& b) noexcept {
    // FLD [edx+i]; FADD [eax+i]; FSTP temp (00709A63..00709A7E), then copied to ECX.
    return {static_cast<float>(static_cast<double>(a.x) + b.x), static_cast<float>(static_cast<double>(a.y) + b.y),
            static_cast<float>(static_cast<double>(a.z) + b.z)};
}

Vec3 vec3_sub_00709ac0(const Vec3& a, const Vec3& b) noexcept {
    // FSUB [eax], [eax+4], [eax+8] (00709AC9..00709ADB) into temporaries, then copied.
    return {static_cast<float>(static_cast<double>(a.x) - b.x), static_cast<float>(static_cast<double>(a.y) - b.y),
            static_cast<float>(static_cast<double>(a.z) - b.z)};
}

void vec3_add_scaled_in_place_00709b60(Vec3& v, const Vec3& d, float s) noexcept {
    // Main thread, 24-bit precision control (cc13-fixture7): each FMUL/FADD rounds to 24
    // bits with the extended exponent before the FSTP DWORD, which matters for denormals.
    using cc13x87::x87_add24;
    using cc13x87::x87_mul24;
    // 00709B63..00709B7E: the products are stored to float temporaries first.
    const float px = static_cast<float>(x87_mul24(d.x, s));
    const float py = static_cast<float>(x87_mul24(d.y, s));
    const float pz = static_cast<float>(x87_mul24(d.z, s));
    // 00709B82..00709B9A: FLD/FADD/FSTP per component.
    v.x = static_cast<float>(x87_add24(v.x, px));
    v.y = static_cast<float>(x87_add24(py, v.y));
    v.z = static_cast<float>(x87_add24(v.z, pz));
}

Vec3 vec3_add_scaled_00709bb0(const Vec3& a, const Vec3& b, float s) noexcept {
    const float px = static_cast<float>(static_cast<double>(b.x) * s);
    const float py = static_cast<float>(static_cast<double>(b.y) * s);
    const float pz = static_cast<float>(static_cast<double>(b.z) * s);
    // 00709BD6..00709BF1: a + product into temporaries, then copied to ECX.
    return {static_cast<float>(static_cast<double>(a.x) + px), static_cast<float>(static_cast<double>(a.y) + py),
            static_cast<float>(static_cast<double>(a.z) + pz)};
}

void plane_from_normal_and_point_0073e780(Plane& plane, const Vec3& point, const Vec3& normal) noexcept {
    plane.normal = normal;  // 0073E78A..0073E797
    // 0073E79A..0073E7AE: (p.y*n.y + p.x*n.x) + p.z*n.z, one FSTP.
    const double dot = (static_cast<double>(point.y) * normal.y + static_cast<double>(point.x) * normal.x) +
                       static_cast<double>(point.z) * normal.z;
    plane.d = static_cast<float>(dot);
}

float plane_signed_distance_0073e7c0(const Plane& plane, const Vec3& point) noexcept {
    // 0073E7C4..0073E7D8: same order as 0073E780, stored to a float.
    const float dot = static_cast<float>(
        (static_cast<double>(point.y) * plane.normal.y + static_cast<double>(point.x) * plane.normal.x) +
        static_cast<double>(point.z) * plane.normal.z);
    // 0073E7DC..0073E7E3: FSUB [ecx+0Ch]; FSTP.
    return static_cast<float>(static_cast<double>(dot) - plane.d);
}

int digit_spinner_step_0067df40(std::int32_t& digit, int delta) noexcept {
    // LEA eax, [edx+eax+0Ah]; CDQ; IDIV 10 (0067DF58..0067DF63); the remainder is stored.
    const std::int32_t sum = digit + delta + 10;
    digit = sum % 10;
    return sum / 10;
}

std::uint8_t warning_remap_kind_007018a0(int kind) noexcept {
    if (kind == 0) {
        return 1;
    }
    return static_cast<std::uint8_t>((kind != 1) + 2);
}

bool listbox_row_activatable_0067c170(const std::uint8_t* enabled_bytes, const std::uint8_t* locked_bytes,
                                      int row) noexcept {
    // CMP byte [eax+ecx+0Ch], 0 / CMP byte [eax+ecx+10Ch], 0 (0067C174, 0067C17B).
    return enabled_bytes[row] != 0 && locked_bytes[row] == 0;
}

void radix_sort_records16_00722210(Record16* records, int count, Record16* scratch, int key_index) noexcept {
    // Interleaved histogram table int[256][4] (1000h bytes, memset first).
    std::array<std::array<std::int32_t, 4>, 256> table{};
    for (int i = 0; i < count; ++i) {
        const std::uint32_t key = records[i].words[static_cast<std::size_t>(key_index)];
        ++table[key & 0xFFu][0];
        ++table[(key >> 8) & 0xFFu][1];
        ++table[(key >> 16) & 0xFFu][2];
        ++table[key >> 24][3];
    }
    // Exclusive prefix sums for the four digit columns, 256 iterations.
    std::array<std::int32_t, 4> running{};
    for (auto& row : table) {
        for (std::size_t column = 0; column < 4; ++column) {
            const std::int32_t n = row[column];
            row[column] = running[column];
            running[column] += n;
        }
    }
    if (count <= 0) {
        return;
    }
    // Scatter by byte 0..3 of the key, alternating records -> scratch -> records.
    Record16* from = records;
    Record16* to = scratch;
    for (std::size_t pass = 0; pass < 4; ++pass) {
        for (int i = 0; i < count; ++i) {
            const std::uint32_t key = from[i].words[static_cast<std::size_t>(key_index)];
            const std::uint32_t digit = (key >> (8 * pass)) & 0xFFu;
            to[table[digit][pass]++] = from[i];
        }
        Record16* const swap = from;
        from = to;
        to = swap;
    }
}

SlotPool::SlotPool(std::size_t slot_size, std::uint16_t slots_per_block) noexcept
    : slot_size_(slot_size), slots_per_block_(slots_per_block) {}

SlotPool::Block SlotPool::make_block() const {
    Block block;
    block.free_stack.resize(slots_per_block_);
    for (std::uint16_t i = 0; i < slots_per_block_; ++i) {
        block.free_stack[i] = static_cast<std::uint16_t>(slots_per_block_ - 1 - i);  // 007075D0 loop
    }
    block.free_count = slots_per_block_;
    return block;
}

SlotPool::Slot SlotPool::allocate() {
    if (first_free_ == 0xFFFFFFFFu) {
        // 00708937..007089CB: the new block takes index = block count and is appended.
        first_free_ = static_cast<std::uint32_t>(blocks_.size());
        blocks_.push_back(make_block());
    }
    Block& block = blocks_[first_free_];
    // 007089D5..007089FC: pop the free stack (slot = block + index * slot size).
    --block.free_count;
    const Slot slot{first_free_, block.free_stack[block.free_count]};
    if (block.free_count == 0) {
        // 007089FE..: first_free = -1, then search the following blocks for a free slot.
        std::uint32_t next = first_free_ + 1;
        first_free_ = 0xFFFFFFFFu;
        for (; next < blocks_.size(); ++next) {
            if (blocks_[next].free_count != 0) {
                first_free_ = next;
                break;
            }
        }
    }
    return slot;
}

void SlotPool::free(const Slot& slot) noexcept {
    Block& block = blocks_[slot.block];
    // 007079B8..007079EA: block from slot +188h, stack[count] = (slot - block) / slot size; ++count.
    block.free_stack[block.free_count] = slot.index;
    ++block.free_count;
    if (static_cast<std::uint32_t>(slot.block) < first_free_) {  // 007079F2 unsigned compare
        first_free_ = static_cast<std::uint32_t>(slot.block);
    }
}

std::uint16_t SlotPool::free_count(std::size_t block) const noexcept {
    return blocks_[block].free_count;
}

}  // namespace bsp::band_0067_cc13

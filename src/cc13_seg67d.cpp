// cc13_seg67d: segment 67 upper half (packet cc13-seg67d). See
// include/bsp/cc13_seg67d.hpp and docs/CC13_SEG67D.md.
// Hypothesis names, build-tested only, nothing wired.

#include "bsp/cc13_seg67d.hpp"

namespace bsp {
namespace {

template <class T>
T& at(void* base, std::size_t offset) {
    return cc13_seg67d_at<T>(base, offset);
}

void* ptr(void* base, std::size_t offset) {
    return static_cast<std::uint8_t*>(base) + offset;
}

// Float constants, as stored (all are read with MOVSS, at float width).
constexpr float kOne_00d7a24c = 1.0f;              // 3F800000
constexpr float kMinusOne_00d7a260 = -1.0f;        // BF800000
constexpr float k1000_00ce3804 = 1000.0f;          // 447A0000
constexpr float k0_15_00ce7818 = 0.15f;            // 3E19999A
constexpr float k10_00ce38b8 = 10.0f;              // 41200000
constexpr float k100_00ce3d08 = 100.0f;            // 42C80000
constexpr float k400_00cfd710 = 400.0f;            // 43C80000
constexpr float k50_00ceb4d4 = 50.0f;              // 42480000
constexpr float k1e9_00d217e8 = 1000000000.0f;     // 4E6E6B28

}  // namespace

// 009DBDF0. Store order follows the image.
void* cc13_seg67d_path_plan_pair_ctor_009dbdf0(void* self, Cc13Seg67dHost& host) {
    host.path_plan_pair_construct_00bf7cd1(self);           // 009DBE02
    at<void*>(self, 0xD4) = ptr(self, 0x68);                // second plan
    at<void*>(self, 0x48) = ptr(self, 0x68);
    at<float>(self, 0xDC) = kMinusOne_00d7a260;
    at<float>(self, 0xF0) = 0.0f;
    at<std::uint8_t>(self, 0xD8) = 0;
    at<std::uint8_t>(self, 0xD9) = 0;
    at<std::uint8_t>(self, 0xDA) = 0;
    at<std::uint32_t>(self, 0xE8) = 0;
    at<std::uint32_t>(self, 0xE4) = 0;
    at<std::uint32_t>(self, 0xEC) = 0;
    at<std::uint32_t>(self, 0xE0) = 0;
    at<void*>(self, 0xD0) = self;                           // first plan
    at<void*>(self, 0xB0) = self;
    at<float>(self, 0xFC) = k1e9_00d217e8;
    at<float>(self, 0xF8) = k1e9_00d217e8;
    return self;
}

// 009DFCB0. Store order follows the image. Zero floats are XORPS stores (+0.0)
// and zero dwords and bytes come from EBX = 0. The same bits either way.
void* cc13_seg67d_nav_block_seed_defaults_009dfcb0(void* self, Cc13Seg67dHost& host) {
    at<float>(self, 0x14) = 0.0f;
    at<float>(self, 0x0C) = 0.0f;
    at<float>(self, 0x10) = 0.0f;
    at<std::uint32_t>(self, 0x08) = 0;
    at<std::uint32_t>(self, 0x00) = 0;
    at<std::uint32_t>(self, 0x04) = 0;
    at<std::uint8_t>(self, 0x20) = 1;
    at<float>(self, 0x44) = 0.0f;
    at<float>(self, 0x3C) = 0.0f;
    at<float>(self, 0x40) = 0.0f;
    at<std::uint32_t>(self, 0x38) = 0;
    at<std::uint32_t>(self, 0x30) = 0;
    at<std::uint32_t>(self, 0x34) = 0;
    at<std::uint8_t>(self, 0x50) = 1;
    cc13_seg67d_path_plan_pair_ctor_009dbdf0(ptr(self, 0x60), host);  // 009DFCF3

    at<float>(self, 0x164) = 0.0f;
    at<float>(self, 0x178) = 0.0f;
    at<float>(self, 0x16C) = 0.0f;
    at<float>(self, 0x168) = 0.0f;
    at<float>(self, 0x160) = 0.0f;
    at<float>(self, 0x170) = 0.0f;
    at<std::uint8_t>(self, 0x174) = 0;
    at<float>(self, 0x184) = kOne_00d7a24c;
    at<float>(self, 0x180) = kOne_00d7a24c;
    at<float>(self, 0x17C) = k1000_00ce3804;
    at<float>(self, 0x188) = 0.0f;
    at<float>(self, 0x18C) = 0.0f;
    at<float>(self, 0x190) = kMinusOne_00d7a260;
    at<float>(self, 0x1C0) = kMinusOne_00d7a260;
    at<float>(self, 0x1BC) = kMinusOne_00d7a260;
    at<float>(self, 0x1B0) = 0.0f;
    at<float>(self, 0x1A4) = 0.0f;
    at<float>(self, 0x19C) = 0.0f;
    at<float>(self, 0x1B8) = 0.0f;
    at<std::uint32_t>(self, 0x198) = 0;
    at<std::uint8_t>(self, 0x1A0) = 1;
    at<std::uint32_t>(self, 0x1AC) = 0;
    at<std::uint32_t>(self, 0x1B4) = 0;
    at<std::uint8_t>(self, 0x1A8) = 0;
    at<std::uint8_t>(self, 0x1C4) = 0;
    at<std::uint8_t>(self, 0x1C5) = 0;
    at<std::uint8_t>(self, 0x1C6) = 0;
    at<float>(self, 0x1D0) = 0.0f;
    at<float>(self, 0x1D4) = 0.0f;
    at<float>(self, 0x1D8) = 0.0f;
    at<float>(self, 0x1DC) = 0.0f;
    at<std::uint8_t>(self, 0x1E0) = 1;
    at<std::uint8_t>(self, 0x1E1) = 0;
    at<std::uint8_t>(self, 0x1E2) = 0;
    at<float>(self, 0x1EC) = 0.0f;
    at<float>(self, 0x1F0) = 0.0f;
    at<float>(self, 0x1F4) = 0.0f;
    at<float>(self, 0x1F8) = 0.0f;
    at<std::uint8_t>(self, 0x1FC) = 1;
    at<std::uint8_t>(self, 0x1FD) = 0;
    at<std::uint8_t>(self, 0x1FE) = 0;
    // blk+3C4h..blk+3E4h: the window also projected by
    // ship_ai_nav_block_steering_defaults_009dfcb0 (ship_ai_nav_block_ctor.cpp).
    at<float>(self, 0x20C) = k0_15_00ce7818;
    at<float>(self, 0x200) = k10_00ce38b8;
    at<float>(self, 0x214) = k100_00ce3d08;
    at<float>(self, 0x210) = k100_00ce3d08;
    at<float>(self, 0x204) = k400_00cfd710;
    at<float>(self, 0x208) = k400_00cfd710;
    at<float>(self, 0x220) = k50_00ceb4d4;
    at<float>(self, 0x218) = k400_00cfd710;
    at<float>(self, 0x21C) = k1000_00ce3804;
    return self;
}

}  // namespace bsp

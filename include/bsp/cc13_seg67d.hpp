#pragma once

// cc13_seg67d: segment 67 upper half (009C6000-009F6060), squadron and plane AI
// plus the ship AI navigation block (packet cc13-seg67d, docs/CC13_SEG67D.md).
// Standalone, nothing wired.
//
// Every name is a hypothesis, not a recovered symbol. These are new C++
// interfaces over the native records, not drop-in replacements of the original
// ABI. Each function's comment gives the native ABI. Native objects are raw
// byte blocks, touched only at the offsets the image writes. Every call the
// image makes goes through a named Cc13Seg67dHost method, at the same point
// and with the same arguments. Build-tested only.

#include <cstddef>
#include <cstdint>

namespace bsp {

template <class T>
inline T& cc13_seg67d_at(void* base, std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}

class Cc13Seg67dHost {
public:
    virtual ~Cc13Seg67dHost() = default;

    // 00BF7CD1 `eh vector constructor iterator'(array, 68h, 2,
    // 009D9CC0 BSP_ShipAiPathPlan_Init, 009DBDB0 BSP_ShipAiPathPlan_DestructBody).
    // It constructs the two 68h-byte path plans at array+0h and array+68h, and
    // destroys the ones already built if a constructor throws.
    virtual void path_plan_pair_construct_00bf7cd1(void* array) = 0;
};

// 009DBDF0 CG_array_ctor_helper_009dbdf0: the 100h-byte path-plan pair member
// at nav block +60h (blk+224h). __fastcall(ECX = this), returns EAX = this,
// plain RET. Body 009DBDF0-009DBE81.
void* cc13_seg67d_path_plan_pair_ctor_009dbdf0(void* self, Cc13Seg67dHost& host);

// 009DFCB0 BSP_ShipAi_NavigationBlockSeedDefaults: the full constructor of the
// 224h-byte steering sub-block at blk+1C4h. __thiscall(ECX = this), returns
// EAX = this, plain RET. Body 009DFCB0-009DFEB4. Sole caller 009E4330 at
// 009E43CC. It calls 009DBDF0 directly, so the reconstruction calls
// cc13_seg67d_path_plan_pair_ctor_009dbdf0 at the same point.
void* cc13_seg67d_nav_block_seed_defaults_009dfcb0(void* self, Cc13Seg67dHost& host);

}  // namespace bsp

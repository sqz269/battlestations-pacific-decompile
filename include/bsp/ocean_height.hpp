#pragma once

// The water height sampler the ship motion tick and the buoyancy model both call.
//
// docs/OCEAN_HEIGHT.md carries the addresses, the original ABI and the uncertainty. Names
// are hypotheses, not recovered symbols. This is a semantic interface for MSVC Win32, not
// a binary replacement.
//
// 0078CF20 is `float __thiscall(world, float x, float z)`, RET 8, body
// 0078CF20..0078CF76, on the world object at [00E188A8]+19F0h. It is the whole sampler:
// two calls on the sub-object at world+A8h, multiplied, rounded to float. Callers treat
// the result as a world-space y, and every one of them subtracts it from a point's y to
// get a height above water (00932D6B in the buoyancy loop, 004462D0 step 2, the ship
// motion tick's wave gate).

namespace bsp {

struct OceanHeightHost {
    virtual ~OceanHeightHost() = default;

    // 0078C890, `float __thiscall(field, float x, float z)`, RET 8, body
    // 0078C890..0078C93E. The wave field: it returns 0.0f immediately when the byte at
    // field+F9h is set (0078C896), otherwise it scales both coordinates by field+B4h,
    // takes the fractional part of each through the CRT helper 00BF85B0, samples the
    // object at field+BCh through 00B960C0 with the two fractions, and multiplies the
    // result by field+24h. Only that outer shape is transcribed here; 00B960C0's body was
    // not read, so `wave_sample` is `contract: unread` below that point.
    virtual float wave_height_0078c890(float x, float z) = 0;

    // 00B9CF50, `float __thiscall(field, float x, float z)`, RET 8, body
    // 00B9CF50..00B9D112. A coverage mask: it walks the 30h-byte region array at
    // field+620h (count at field+624h) for the first region whose (+10h,+14h)-(+18h,+1Ch)
    // rectangle contains (x, z), converts the point to two clamped integer bitmap indices
    // through the region's +20h/+24h scales and +28h/+2Ch dimensions, reads one byte from
    // the bitmap at region+4h, divides it by the float at 00CE4B48 and clamps the result
    // to [0, 1]. **When no region contains the point it returns exactly 1.0f**, which is
    // the open-sea case.
    virtual float coverage_mask_00b9cf50(float x, float z) = 0;
};

// 0078CF20. The product, rounded to float by the FSTP at 0078CF69.
float ocean_water_height_0078cf20(float x, float z, OceanHeightHost& host);

// A flat sea: the wave field disabled (field+F9h set) or its amplitude field+24h zero,
// with no coverage region over the point. Both make 0078CF20 return exactly 0.0f, which
// is the y of the water plane the rest of the motion path assumes.
struct FlatSeaOceanHost final : OceanHeightHost {
    float wave_height_0078c890(float, float) override { return 0.0f; }
    float coverage_mask_00b9cf50(float, float) override { return 1.0f; }
};

}  // namespace bsp

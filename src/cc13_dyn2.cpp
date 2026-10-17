// cc13_dyn2: Dyn::TerrainConvexIntersect slot 0 (00C58EB0) and the Mit pool <36,128> free
// (00407960, thunk 00C36990). See include/bsp/cc13_dyn2.hpp and docs/CC13_DYN2.md. Nothing
// here is wired.
//
// x87 note. 00C58EB0 is a collision-dispatcher method; the collision pass that calls these
// methods runs on the Dyn task workers at the 53-bit control word 027Fh
// (docs/CC13_FPU_PRECISION.md). The C++ below therefore computes each x87 chain in double, in
// the image's grouping, and narrows to float exactly where the image stores a float. Operand
// direction of every reg-reg op was taken from the bytes (tools/x87_sym.py decodes them; the
// one non-popping reg-reg form, 00C5A7DD FMUL ST1, is D8 C9: ST0 = ST0*ST1). Products of two
// floats, and of a float and a small integer, are exact in double, so only the grouping of
// the sums matters. The SSE parts (CVTSI2SS, CVTTSS2SI, MOVSS) do not depend on precision.
// No function here draws from the shared RNG 00BD2F10.

#include "bsp/cc13_dyn2.hpp"

#include "bsp/math.hpp"
#include "bsp/native_dyn_general_convex.hpp"

#include <cstring>
#include <xmmintrin.h>

namespace bsp::cc13_dyn2 {

using cc13_dyn1::Raw;
using cc13_dyn1::raw;

// ---------------------------------------------------------------- Mit pool <36,128>

void pool36_free_00407960(MitPoolAllocator36x128& pool, void* slot) {
    ::EnterCriticalSection(&pool.lock_0c);  // 00407969 CALL [00CE2218]
    pool.lock_depth_24 += 1;                // 0040796F ADD [0109EA30],1
    const std::uint32_t page_index = raw(slot).at<std::uint32_t>(0x24);
    std::uint8_t* const page = pool.pages_28[page_index];
    // 00407981..00407992: (slot - page) / 28h, IMUL by 66666667h, SAR 4, plus the sign bit:
    // signed division truncating toward zero.
    const std::int32_t offset = static_cast<std::int32_t>(
        reinterpret_cast<std::uintptr_t>(slot) - reinterpret_cast<std::uintptr_t>(page));
    const std::int32_t index = offset / 0x28;
    const Raw p = raw(page);
    const std::uint16_t depth = p.at<std::uint16_t>(0x1500);
    p.at<std::uint16_t>(0x1400 + 2u * depth) = static_cast<std::uint16_t>(index);
    p.at<std::uint16_t>(0x1500) = static_cast<std::uint16_t>(depth + 1);
    // 004079AB CMP EDI,[0109EA40] / JNC: unsigned below lowers the first-free hint.
    if (page_index < pool.first_free_page_34) pool.first_free_page_34 = page_index;
    pool.lock_depth_24 -= 1;                // 004079B9 SUB [0109EA30],1
    ::LeaveCriticalSection(&pool.lock_0c);  // 004079C5 CALL [00CE2210]
}

void pool36_free_thunk_00c36990(MitPoolAllocator36x128& pool, void* slot) {
    pool36_free_00407960(pool, slot);  // 00C36990 JMP 00407960
}

// ---------------------------------------------------------------- 00C58EB0

namespace {

inline double D(float v) noexcept { return static_cast<double>(v); }
inline float F(double v) noexcept { return static_cast<float>(v); }

// 00D7A280 0.5 (double), 00D7A2B0 3.0 (double), 00D7A250 -1.0 (double),
// 00D7A240 -1000.0f (the uint16 FFFFh height), 00D7A24C 1.0f.
constexpr double kHalf_00d7a280 = 0.5;
constexpr double kThree_00d7a2b0 = 3.0;
constexpr double kMinusOne_00d7a250 = -1.0;
constexpr float kNoHeight_00d7a240 = -1000.0f;
constexpr float kOne_00d7a24c = 1.0f;

// A shape's local 3x4 (S+34h: three rows, then the translation) composed with a body matrix
// (rows m[0..8], translation m[9..11]): every element ((p0 + p1) + p2), the translation then
// + m[9+c]. Both prologue paths (00C58EE1.., 00C593A8..) use this grouping for all 24 values.
void compose(const float* l, const float* m, float* out) noexcept {
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            out[r * 3 + c] =
                F((D(l[r * 3]) * m[c] + D(l[r * 3 + 1]) * m[3 + c]) + D(l[r * 3 + 2]) * m[6 + c]);
        }
    }
    for (int c = 0; c < 3; ++c) {
        out[9 + c] =
            F(((D(l[9]) * m[c] + D(l[10]) * m[3 + c]) + D(l[11]) * m[6 + c]) + m[9 + c]);
    }
}

// CVTTSS2SI: truncation; NaN and out-of-range give 80000000h.
inline std::int32_t cvttss2si(float v) noexcept { return _mm_cvtt_ss2si(_mm_set_ss(v)); }

// The heightfield read at sample `index` (32-bit wrapped row*width+column). Mode [T+22Ch]:
// 0 float samples, 1 uint16 decoded as FLD [T+224h] * FILD u + [T+228h] with FFFFh ->
// -1000.0f (XMM1, from 00D7A240), anything else stores XMM0, which is 0.0 at every one of
// the four sample sites (XORPS at 00C59F6B before the loops and 00C5A7A1 at the end of each
// inner iteration; the only in-iteration writer, 00C5A29E, is the mode-0 path of the last
// sample).
float sample(const Raw t, std::uint32_t index) noexcept {
    const std::int32_t mode = t.at<std::int32_t>(0x22c);
    if (mode == 0) return static_cast<const float*>(t.ptr(0x210))[static_cast<std::int32_t>(index)];
    if (mode == 1) {
        const std::uint16_t u =
            static_cast<const std::uint16_t*>(t.ptr(0x210))[static_cast<std::int32_t>(index)];
        if (u == 0xffffu) return kNoHeight_00d7a240;
        return F(D(t.at<float>(0x224)) * static_cast<double>(u) + t.at<float>(0x228));
    }
    return 0.0f;
}

// The inlined Dyn::Shape base constructor reads its descriptor (dyn1's construct_shape_base:
// +8 <- desc+10h, +24h..30h <- desc+0..0Ch). Here the descriptor is the constant 0, so the
// image loads the absolute addresses 10h, 0, 4, 8, 0Ch (00C59EDA..00C59F15; the PE has no
// relocation directory, so they stay absolute). Every one faults in the image; the same
// volatile loads are kept here.
inline std::uint32_t null_descriptor_dword(std::uintptr_t address) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(address);
}

constexpr std::size_t kTriangleShapeSize = 0x234;  // ESP+198h..3CBh in the frame
constexpr std::size_t kContactRecordSize = 0x128;  // ESP+3D0h..4F7h: count + 8 * 24h
constexpr std::size_t kWorkRecordSize = 0x1f8;     // ESP+4F8h..6EFh

}  // namespace

bool terrain_convex_intersect_00c58eb0(void* result, void* shape_a, const float* matrix_a,
                                       void* shape_b, const float* matrix_b,
                                       DynGeneralConvexIntersectStorage& general,
                                       const CameraAxesCrtAccess& crt,
                                       std::uint32_t shape_base_vtable,
                                       std::uint32_t triangle_vtable) {
    // 00C58ED4 CMP [shape_a+8],5 / JNZ 00C593A8. Terrain first: EBX = shape_a, swap flag
    // [ESP+18h] = 0 (00C5933D). Otherwise EBX = shape_b, flag = 1 (00C5980A).
    const bool swapped = raw(shape_a).at<std::int32_t>(0x8) != 5;
    void* const terrain = swapped ? shape_b : shape_a;
    void* const convex = swapped ? shape_a : shape_b;
    const float* const terrain_matrix = swapped ? matrix_b : matrix_a;
    const float* const convex_matrix = swapped ? matrix_a : matrix_b;
    const Raw t = raw(terrain);
    const Raw cv = raw(convex);

    // Frame ESP+D4h.. (terrain world) and ESP+98h.. (convex world).
    float tw[12];
    float cw[12];
    compose(reinterpret_cast<const float*>(t.field(0x34)), terrain_matrix, tw);
    compose(reinterpret_cast<const float*>(cv.field(0x34)), convex_matrix, cw);

    // 00C59880..00C59A1E: r[i][j] = convex row i . terrain row j.
    float r[3][3];
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            r[i][j] = F((D(cw[i * 3]) * tw[j * 3] + D(cw[i * 3 + 1]) * tw[j * 3 + 1]) +
                        D(cw[i * 3 + 2]) * tw[j * 3 + 2]);
        }
    }
    // 00C59A30..00C59ABC: the translation difference in terrain space.
    float d[3];
    for (int k = 0; k < 3; ++k) d[k] = F(D(cw[9 + k]) - tw[9 + k]);
    float dl[3];
    for (int j = 0; j < 3; ++j) {
        dl[j] = F((D(tw[j * 3]) * d[0] + D(tw[j * 3 + 1]) * d[1]) + D(tw[j * 3 + 2]) * d[2]);
    }
    // 00C59AC0..00C59B6D: the convex shape's body-space box (lo S+0Ch..14h, hi S+18h..20h)
    // as a center and half extents.
    float center[3];
    float half[3];
    for (int k = 0; k < 3; ++k) {
        const float lo = cv.at<float>(0x0c + 4 * k);
        const float hi = cv.at<float>(0x18 + 4 * k);
        center[k] = F(D(F(D(hi) + lo)) * kHalf_00d7a280);
        half[k] = F(D(F(D(hi) - lo)) * kHalf_00d7a280);
    }
    // 00C59B71..00C59BEF: the center in terrain space.
    float cl[3];
    for (int j = 0; j < 3; ++j) {
        cl[j] = F(((D(center[0]) * r[0][j] + D(center[1]) * r[1][j]) + D(center[2]) * r[2][j]) +
                  dl[j]);
    }
    // 00C59BF6..00C59C92: nine BSP_Math_AbsFloat calls in row order.
    float a[3][3];
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) a[i][j] = abs_00401170(r[i][j]);
    }
    float ext[3];
    for (int j = 0; j < 3; ++j) {
        ext[j] = F((D(half[0]) * a[0][j] + D(half[1]) * a[1][j]) + D(half[2]) * a[2][j]);
    }
    // 00C59D03..00C59D87: the x and z spans (the image also forms the y span into frame slots
    // that nothing reads).
    const float x_lo = F(D(cl[0]) - ext[0]);
    const float z_lo = F(D(cl[2]) - ext[2]);
    const float x_hi = F(D(cl[0]) + ext[0]);
    const float z_hi = F(D(cl[2]) + ext[2]);
    const float spacing_x = t.at<float>(0x21c);
    const float spacing_z = t.at<float>(0x220);
    const float qx_lo = F(D(x_lo) / spacing_x);
    const float qx_hi = F(D(x_hi) / spacing_x);
    const float qz_lo = F(D(z_lo) / spacing_z);
    const float qz_hi = F(D(z_hi) / spacing_z);

    // 00C59DE5..00C59E8A: grid range. The column start clamps to [0, width-1], the row start
    // to [0, height]; both ends are trunc+1 (JNS on the wrapped sum) clamped to [0, size].
    const std::int32_t width = t.at<std::int32_t>(0x214);
    const std::int32_t height = t.at<std::int32_t>(0x218);
    std::int32_t col_begin;
    {
        const std::int32_t v = cvttss2si(qx_lo);
        col_begin = v < 0 ? 0 : (v > width - 1 ? width - 1 : v);
    }
    std::int32_t col_end;
    {
        const std::int32_t v = static_cast<std::int32_t>(
            static_cast<std::uint32_t>(cvttss2si(qx_hi)) + 1u);
        col_end = v < 0 ? 0 : (v > width ? width : v);
    }
    std::int32_t row_begin;
    {
        const std::int32_t v = cvttss2si(qz_lo);
        row_begin = v < 0 ? 0 : (v > height ? height : v);
    }
    std::int32_t row_end;
    {
        const std::int32_t v = static_cast<std::int32_t>(
            static_cast<std::uint32_t>(cvttss2si(qz_hi)) + 1u);
        row_end = v < 0 ? 0 : (v > height ? height : v);
    }
    // 00C59E8E..00C59EB9: an empty product returns AL = 0 with the result untouched.
    if (static_cast<std::uint32_t>(row_end - row_begin) *
            static_cast<std::uint32_t>(col_end - col_begin) ==
        0u) {
        return false;
    }

    // 00C59EBC..00C59FC8: the stack TriangleShape at ESP+198h, in the image's store order.
    alignas(8) std::uint8_t triangle[kTriangleShapeSize];
    alignas(8) std::uint8_t contact[kContactRecordSize];
    alignas(8) std::uint8_t work[kWorkRecordSize];
    const Raw tri = raw(triangle);
    const Raw res = raw(result);
    tri.at<std::uint32_t>(0x0) = shape_base_vtable;  // 00D7A04C
    res.at<std::int32_t>(0x0) = 0;                    // 00C59ECA count
    tri.at<std::uint32_t>(0x204) = 0;
    tri.at<std::uint32_t>(0x4) = 0;
    tri.at<std::uint32_t>(0x8) = null_descriptor_dword(0x10);   // 00C59EDA, faults
    tri.at<std::uint32_t>(0x24) = null_descriptor_dword(0x0);
    tri.at<std::uint32_t>(0x28) = null_descriptor_dword(0x4);
    tri.at<std::uint32_t>(0x2c) = null_descriptor_dword(0x8);
    tri.at<std::uint32_t>(0x30) = null_descriptor_dword(0xc);
    tri.at<std::uint32_t>(0x208) = 0;
    tri.at<std::uint32_t>(0x20c) = 0;
    tri.at<std::uint32_t>(0x0) = triangle_vtable;  // 00D7A1B0; EH state 0 (not reproduced)
    tri.at<float>(0x54) = kOne_00d7a24c;
    tri.at<float>(0x44) = kOne_00d7a24c;
    tri.at<float>(0x34) = kOne_00d7a24c;
    tri.ptr(0x4) = t.ptr(0x4);  // the terrain's body
    for (const std::size_t off : {0x50u, 0x4cu, 0x48u, 0x40u, 0x3cu, 0x38u, 0x60u, 0x5cu, 0x58u})
        tri.at<float>(off) = 0.0f;

    float* const verts = reinterpret_cast<float*>(tri.field(0x210));
    const Raw w = raw(work);
    std::int32_t& count = res.at<std::int32_t>(0x0);

    // One 00C53010 call (ECX = terrain world, EDX = convex, EAX = convex world; stack work,
    // contact, triangle) and the append of its contact (REP MOVSD of nine dwords to
    // result+4+count*24h, then ADD [result],1). Returns true when the count reached 8.
    auto intersect_triangle = [&]() -> bool {
        w.at<const void*>(0x1f0) = general.support_directions_08;  // 0109EA50
        w.at<const void*>(0x1f4) = &general.critical_section_278;  // 0109ECC0
        if (intersect_native_dyn_convex_00c53010(work, contact, triangle, tw, convex, cw, crt)) {
            std::memcpy(res.field(0x4 + static_cast<std::size_t>(count) * 0x24),
                        raw(contact).field(0x4), 0x24);
            count += 1;
        }
        return count == 8;  // 00C5A527 / 00C5A786 CMP [ESI],8 / JZ 00C5A7C1
    };

    // 00C59FCC..00C5A7B9: columns outer, rows inner (signed JGE/JL compares).
    for (std::int32_t col = col_begin; col < col_end; ++col) {
        const float fcol = static_cast<float>(col);       // CVTSI2SS
        const float fcol1 = static_cast<float>(col + 1);  // CVTSI2SS
        for (std::int32_t row = row_begin; row < row_end; ++row) {
            const std::uint32_t wu = static_cast<std::uint32_t>(width);
            const std::uint32_t ru = static_cast<std::uint32_t>(row);
            const std::uint32_t cu = static_cast<std::uint32_t>(col);
            const float h00 = sample(t, wu * ru + cu);
            const float x0 = F(D(spacing_x) * fcol);
            const float z0 = F(D(spacing_z) * static_cast<double>(row));  // FILD row
            const float h10 = sample(t, wu * ru + (cu + 1u));
            const float x1 = F(D(spacing_x) * fcol1);
            const float h11 = sample(t, (ru + 1u) * wu + (cu + 1u));
            const float z1 = F(D(spacing_z) * static_cast<double>(row + 1));  // FIMUL row+1
            const float h01 = sample(t, wu * (ru + 1u) + cu);

            // Triangle (x0,h00,z0) (x1,h10,z0) (x1,h11,z1); its box is the centroid twice.
            verts[0] = x0; verts[1] = h00; verts[2] = z0;
            verts[3] = x1; verts[4] = h10; verts[5] = z0;
            verts[6] = x1; verts[7] = h11; verts[8] = z1;
            {
                const float sx = F(D(x1) + x0);
                const float sy = F(D(h10) + h00);
                const float sz = F(D(z0) + z0);
                tri.at<float>(0x0c) = F(D(F(D(sx) + x1)) / kThree_00d7a2b0);
                tri.at<float>(0x10) = F(D(F(D(h11) + sy)) / kThree_00d7a2b0);
                tri.at<float>(0x14) = F(D(F(D(sz) + z1)) / kThree_00d7a2b0);
                tri.at<float>(0x18) = F(D(F(D(x1) + sx)) / kThree_00d7a2b0);
                tri.at<float>(0x1c) = F(D(F(D(h11) + sy)) / kThree_00d7a2b0);
                tri.at<float>(0x20) = F(D(F(D(sz) + z1)) / kThree_00d7a2b0);
            }
            if (intersect_triangle()) goto done;

            // Triangle (x0,h00,z0) (x1,h11,z1) (x0,h01,z1).
            verts[0] = x0; verts[1] = h00; verts[2] = z0;
            verts[3] = x1; verts[4] = h11; verts[5] = z1;
            verts[6] = x0; verts[7] = h01; verts[8] = z1;
            {
                const float sx = F(D(x1) + x0);
                const float sy = F(D(h11) + h00);
                const float sz = F(D(z1) + z0);
                tri.at<float>(0x0c) = F(D(F(D(x0) + sx)) / kThree_00d7a2b0);
                tri.at<float>(0x10) = F(D(F(D(h01) + sy)) / kThree_00d7a2b0);
                tri.at<float>(0x14) = F(D(F(D(sz) + z1)) / kThree_00d7a2b0);
                tri.at<float>(0x18) = F(D(F(D(x0) + sx)) / kThree_00d7a2b0);
                tri.at<float>(0x1c) = F(D(F(D(h01) + sy)) / kThree_00d7a2b0);
                tri.at<float>(0x20) = F(D(F(D(z1) + sz)) / kThree_00d7a2b0);
            }
            if (intersect_triangle()) goto done;
        }
    }
done:
    // 00C5A7C1..00C5A825: terrain was second, so each contact's normal is multiplied by -1.0
    // and its two points are exchanged.
    if (swapped) {
        for (std::int32_t i = 0; i < count; ++i) {
            float* const c = reinterpret_cast<float*>(res.field(0x4 + static_cast<std::size_t>(i) * 0x24));
            c[6] = F(D(c[6]) * kMinusOne_00d7a250);
            c[7] = F(D(c[7]) * kMinusOne_00d7a250);
            c[8] = F(D(c[8]) * kMinusOne_00d7a250);
            std::uint32_t tmp[3];
            std::memcpy(tmp, c, 12);
            std::memcpy(c, c + 3, 12);
            std::memcpy(c + 3, tmp, 12);
        }
    }
    return count != 0;  // 00C5A827 CMP [ESI],0 / SETNZ AL
}

}  // namespace bsp::cc13_dyn2
